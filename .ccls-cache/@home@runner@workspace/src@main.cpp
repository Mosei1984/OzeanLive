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

    initPet();
    initBubbles();
    initParticles();
    initDirt();

    tft.fillScreen(COLOR_BG);
    
    // Background-Canvas initialisieren und Environment einmalig rendern
    initBackgroundCanvas();
    if (bgCanvas) {
        bgCanvas->fillScreen(COLOR_BG);
        drawEnvironmentToCanvas(bgCanvas);
        // Initial aufs TFT blitten
        tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
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

// Loop
void loop()
{
    // deltaTime berechnen
    unsigned long nowUs = micros();
    float dtSec = (nowUs - lastFrameUs) / 1e6f;
    dtSec = constrain(dtSec, 0.004f, 0.05f);         // Clamping [4ms, 50ms]
    dtSecSmooth = dtSecSmooth * 0.8f + dtSec * 0.2f; // EMA smoothing
    lastFrameUs = nowUs;

    // Animationsphase fortschreiben (zeitbasiert)
    animPhase += ANIM_PHASE_RATE * dtSecSmooth;
    if (animPhase > 1000.0f)
        animPhase -= 1000.0f;

    // Menü-Logik aktualisieren (Buttons -> pendingAction)
    updateMenuLogic();

    // Pet-Logik aktualisieren (Hunger, Fun, Energie, Aktionen)
    updatePetStats();

    // --- Frame zeichnen ---
    drawStatusBar();
    updateAndDrawBubbles(dtSecSmooth);
    updateAndDrawSeahorse();
    updateDirt(dtSecSmooth);
    drawDirt();
    updateParticles(dtSecSmooth);
    drawPetAnimated(dtSecSmooth);
    drawParticles();
    drawBottomMenu();

    // Präzises Frame-Pacing
    if (nextFrameUs == 0)
        nextFrameUs = micros();
    nextFrameUs += targetFrameUs;
    long sleep = nextFrameUs - micros();
    if (sleep > 0)
    {
        delayMicroseconds(sleep);
    }
    else if (sleep < -(long)targetFrameUs)
    {
        nextFrameUs = micros(); // Reset bei großer Verzögerung
    }
}
