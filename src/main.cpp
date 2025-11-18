#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "gfx.h"
#include "sprite_common.h"
#include "environment.h"
#include "bubbles.h"
#include "particles.h"
#include "pet.h"
#include "seahorse.h"
#include "menu.h"
#include "dirt.h"
#include "eeprom_store.h"
#include "shrimp.h"
#include "start_menu.h"
#include "pause_menu.h"

enum GameMode
{
    MODE_START,
    MODE_ALIVE,
    MODE_PAUSED,
    MODE_DEAD
};
static GameMode gMode = MODE_START;

// ---- Zustände für spätere Erweiterungen (z.B. Einstellungsmenü) ----
enum ScreenID
{
    SCREEN_MAIN = 0
};

ScreenID currentScreen = SCREEN_MAIN;

// Vorwärtsdeklaration
void drawStatusBar();

// Cached values für Status-Bar Optimierung
static int lastShownHunger = -1;
static int lastShownFun = -1;
static int lastShownEnergy = -1;

// Setup
void setup()
{
    pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
    pinMode(PIN_BTN_OK, INPUT_PULLUP);
    pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);

    Serial.begin(115200);
    delay(200);

    initDisplay();
    randomSeed(analogRead(0));

    // Play-Area berechnen (zwischen Status-Bar und Bottom-Bar)
    PLAY_AREA_X = 0;
    PLAY_AREA_Y = STATUS_BAR_H;
    PLAY_AREA_W = TFT_WIDTH;
    PLAY_AREA_H = TFT_HEIGHT - STATUS_BAR_H - BOTTOM_BAR_H;

    // Seepferdchen-Basis grob über dem Boden (exakter groundY wird in drawEnvironment verwendet)
    int16_t groundH = 28;
    int16_t groundY = PLAY_AREA_Y + PLAY_AREA_H - groundH;
    seahorseBaseX = PLAY_AREA_X + PLAY_AREA_W / 4;
    seahorseBaseY = groundY - 24; // etwas über dem Boden

    initBubbles();
    initParticles();
    initDirt();
    initShrimp();

    tft.fillScreen(COLOR_BG);

    // Background-Canvas initialisieren und Environment einmalig rendern
    initBackgroundCanvas();
    if (bgCanvas)
    {
        bgCanvas->fillScreen(COLOR_BG);
        drawEnvironmentToCanvas(bgCanvas);
        // Initial aufs TFT blitten
        tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
        drawStatusBar();
        drawBottomMenu();
    }

    while (true)
    {
        StartChoice choice = runStartMenu(hasSave());
        if (choice == START_NEW)
        {
            clearSave();
            initPet();
            gMode = MODE_ALIVE;
            break;
        }
        else if (choice == START_LOAD)
        {
            initPet();
            int16_t h, f, e, hp;
            uint32_t age;
            bool dead;
            if (loadFull(h, f, e, hp, age, dead))
            {
                pet.hunger = h;
                pet.fun = f;
                pet.energy = e;
                pet.hp = hp;
                pet.ageSec = age;
                pet.dead = dead;
                if (pet.hp > getMaxHP())
                    pet.hp = getMaxHP();
                gMode = pet.dead ? MODE_DEAD : MODE_ALIVE;
                break;
            }
        }
    }
}

// Zeichnet die Statuszeile oben (Hunger/Fun/Energie)
void drawStatusBar()
{
    // Nur neu zeichnen wenn sich Werte geändert haben
    if (pet.hunger != lastShownHunger || pet.fun != lastShownFun || pet.energy != lastShownEnergy)
    {
        tft.fillRect(0, 0, TFT_WIDTH, STATUS_BAR_H, COLOR_STATUS_BG);

        tft.setCursor(4, 4);
        tft.setTextSize(1);
        tft.setTextColor(0xFFFF);

        tft.print("H:");
        tft.print(pet.hunger);
        tft.print(" F:");
        tft.print(pet.fun);
        tft.print(" E:");
        tft.print(pet.energy);

        // Cache-Werte aktualisieren
        lastShownHunger = pet.hunger;
        lastShownFun = pet.fun;
        lastShownEnergy = pet.energy;
    }
}

// Frame-Timing-Variablen (60 FPS)
static unsigned long lastFrameUs = 0;
static unsigned long nextFrameUs = 0;
static float dtSecSmooth = 0.0167f;        // ~60 FPS initial
const unsigned long targetFrameUs = 16667; // ~60 FPS

// Pause-Erkennung durch langen OK-Druck
static uint32_t okPressStartMs = 0;
static const uint32_t LONG_PRESS_MS = 1000;
static bool okWasPressed = false;

// Loop
void loop()
{
    // Initialize frame timers on first run to avoid spike
    static bool inited = false;
    unsigned long nowUs = micros();
    if (!inited)
    {
        lastFrameUs = nowUs;
        nextFrameUs = nowUs;
        inited = true;
    }

    // deltaTime berechnen (wrap-safe for micros() overflow at ~71 minutes)
    uint32_t deltaUs = (uint32_t)(nowUs - lastFrameUs);
    float dtSec = deltaUs * 1e-6f;
    dtSec = constrain(dtSec, DT_MIN, DT_MAX);
    dtSecSmooth = dtSecSmooth * (1.0f - DT_EMA_ALPHA) + dtSec * DT_EMA_ALPHA;
    lastFrameUs = nowUs;

    // Animationsphase fortschreiben (zeitbasiert)
    animPhase += ANIM_PHASE_RATE * dtSecSmooth;
    if (animPhase > 1000.0f)
        animPhase -= 1000.0f;

    if (gMode == MODE_ALIVE)
    {
        // Long-press detection for pause
        int okState = digitalRead(PIN_BTN_OK);
        if (okState == LOW && !okWasPressed)
        {
            okPressStartMs = millis();
            okWasPressed = true;
        }
        else if (okState == LOW && okWasPressed)
        {
            if (millis() - okPressStartMs >= LONG_PRESS_MS)
            {
                gMode = MODE_PAUSED;
                while (digitalRead(PIN_BTN_OK) == LOW)
                    ;
                okWasPressed = false;
            }
        }
        else if (okState == HIGH)
        {
            okWasPressed = false;
        }

        // Menü-Logik aktualisieren (Buttons -> pendingAction)
        updateMenuLogic();

        // Pet-Logik aktualisieren (Hunger, Fun, Energie, Aktionen)
        updatePetStats();

        // Check if pet died
        if (pet.dead)
        {
            gMode = MODE_DEAD;
        }

        // --- Frame zeichnen ---
        // CRITICAL: Restore regions FIRST to avoid erasing other sprites
        restoreParticleRegions();
        restorePetRegion();

        drawStatusBar();
        updateAndDrawBubbles(dtSecSmooth);
        updateAndDrawSeahorse();
        updateAndDrawShrimp(dtSecSmooth);

        // Draw poop on ground BEFORE fish (fish swims over it)
        updateDirt(dtSecSmooth);
        drawDirt();

        // Draw fish on top of poop
        drawPetAnimated(dtSecSmooth);

        // Draw particles LAST so they appear on top
        updateParticles(dtSecSmooth);
        drawParticles();
        drawBottomMenu();
    }
    else if (gMode == MODE_PAUSED)
    {
        PauseChoice choice = runPauseMenu();

        if (choice == PAUSE_RESUME)
        {
            gMode = MODE_ALIVE;
            tft.fillScreen(COLOR_BG);
            if (bgCanvas)
            {
                tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
            }
            drawStatusBar();
            drawBottomMenu();
        }
        else if (choice == PAUSE_SAVE_EXIT)
        {
            while (true)
            {
                StartChoice startChoice = runStartMenu(hasSave());
                if (startChoice == START_NEW)
                {
                    clearSave();
                    initPet();
                    gMode = MODE_ALIVE;
                    tft.fillScreen(COLOR_BG);
                    if (bgCanvas)
                    {
                        tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
                    }
                    drawStatusBar();
                    drawBottomMenu();
                    break;
                }
                else if (startChoice == START_LOAD)
                {
                    initPet();
                    int16_t h, f, e, hp;
                    uint32_t age;
                    bool dead;
                    if (loadFull(h, f, e, hp, age, dead))
                    {
                        pet.hunger = h;
                        pet.fun = f;
                        pet.energy = e;
                        pet.hp = hp;
                        pet.ageSec = age;
                        pet.dead = dead;
                        if (pet.hp > getMaxHP())
                            pet.hp = getMaxHP();
                        gMode = pet.dead ? MODE_DEAD : MODE_ALIVE;
                        tft.fillScreen(COLOR_BG);
                        if (bgCanvas)
                        {
                            tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
                        }
                        drawStatusBar();
                        drawBottomMenu();
                        break;
                    }
                }
            }
        }
    }
    else if (gMode == MODE_DEAD)
    {
        if (bgCanvas)
        {
            tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
        }
        drawDeathScreen();

        if (digitalRead(PIN_BTN_OK) == LOW)
        {
            delay(200);
            while (digitalRead(PIN_BTN_OK) == LOW)
            {
            }

            while (true)
            {
                StartChoice choice = runStartMenu(hasSave());
                if (choice == START_NEW)
                {
                    clearSave();
                    initPet();
                    gMode = MODE_ALIVE;
                    break;
                }
                else if (choice == START_LOAD)
                {
                    initPet();
                    int16_t h, f, e, hp;
                    uint32_t age;
                    bool dead;
                    if (loadFull(h, f, e, hp, age, dead))
                    {
                        pet.hunger = h;
                        pet.fun = f;
                        pet.energy = e;
                        pet.hp = hp;
                        pet.ageSec = age;
                        pet.dead = dead;
                        if (pet.hp > getMaxHP())
                            pet.hp = getMaxHP();
                        gMode = pet.dead ? MODE_DEAD : MODE_ALIVE;
                        break;
                    }
                }
            }
        }
    }

    // Präzises Frame-Pacing (wrap-safe)
    if (nextFrameUs == 0)
        nextFrameUs = micros();
    nextFrameUs += targetFrameUs;
    int32_t sleep = (int32_t)(nextFrameUs - micros());
    if (sleep > 0)
    {
        delayMicroseconds(sleep);
    }
    else if (sleep < -(int32_t)targetFrameUs)
    {
        nextFrameUs = micros(); // Reset bei großer Verzögerung
    }
}
