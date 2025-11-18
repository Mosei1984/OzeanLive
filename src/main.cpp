#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
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
#include "Buttons.h"
#ifdef ESP32
#include "ble_handler.h"
#include <esp_heap_caps.h>
#endif

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
#ifdef ESP32
    // Disable watchdog during initialization
    disableCore0WDT();
#endif

    Buttons.begin();

    Serial.begin(115200);
    delay(200);

#ifdef ESP32
    // Initialize EEPROM for ESP32 (required before use)
    EEPROM.begin(1024);
    Serial.println("EEPROM initialized (1024 bytes)");
#endif

    initDisplay();
    randomSeed(analogRead(0));

#ifdef ESP32
    initBLE();
    // Feed watchdog periodically
    yield();
#endif

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

    // Second seahorse position (right side, different height)
    extern int16_t seahorse2BaseX;
    extern int16_t seahorse2BaseY;
    seahorse2BaseX = PLAY_AREA_X + (PLAY_AREA_W * 3 / 4);
    seahorse2BaseY = groundY - 18;  // Slightly lower than first

    initBubbles();
    initParticles();
    initDirt();
    initShrimp();
    initDirtyRects();

#ifdef ESP32
    yield();
#endif

    tft.fillScreen(COLOR_BG);

    // Background-Canvas initialisieren und Environment einmalig rendern
    initBackgroundCanvas();
    
#ifdef ESP32
    yield();
#endif
    
    if (bgCanvas)
    {
#ifdef DEBUG_GRAPHICS
        Serial.println("[MAIN] Filling canvas background...");
#endif
        bgCanvas->fillScreen(COLOR_BG);
        drawEnvironmentToCanvas(bgCanvas);
        
#ifdef ESP32
        yield();
#endif
        
#ifdef DEBUG_GRAPHICS
        Serial.println("[MAIN] Blitting canvas to display...");
#endif
        // Initial aufs TFT blitten
        tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
        drawStatusBar();
        drawBottomMenu();
#ifdef DEBUG_GRAPHICS
        Serial.println("[MAIN] Initial screen rendered");
#endif
    }

    while (true)
    {
        StartChoice choice = runStartMenu(hasSave());
        if (choice == START_NEW)
        {
#ifdef DEBUG_GAME_LOGIC
            Serial.println("[MAIN] Starting NEW GAME");
#endif
            clearSave();
            initPet();
            gMode = MODE_ALIVE;
            
            // Clear start menu completely
            tft.fillScreen(COLOR_BG);
            if (bgCanvas)
            {
                bgCanvas->fillScreen(COLOR_BG);
                drawEnvironmentToCanvas(bgCanvas);
                tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
            }
            drawStatusBar();
            drawBottomMenu();
            
            break;
        }
        else if (choice == START_LOAD)
        {
#ifdef DEBUG_GAME_LOGIC
            Serial.println("[MAIN] LOADING saved game");
#endif
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
#ifdef DEBUG_GAME_LOGIC
                Serial.print("[MAIN] Game loaded - Age: ");
                Serial.print(age);
                Serial.print("s, HP: ");
                Serial.print(hp);
                Serial.print(", Dead: ");
                Serial.println(dead);
#endif
                
                // Clear start menu completely
                tft.fillScreen(COLOR_BG);
                if (bgCanvas)
                {
                    bgCanvas->fillScreen(COLOR_BG);
                    drawEnvironmentToCanvas(bgCanvas);
                    tft.drawRGBBitmap(0, 0, bgCanvas->getBuffer(), TFT_WIDTH, TFT_HEIGHT);
                }
                drawStatusBar();
                drawBottomMenu();
                
                break;
            }
        }
    }
    
#ifdef DEBUG_GAME_LOGIC
    Serial.println("[MAIN] Entering main game loop");
#endif
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
const unsigned long targetFrameUs = 33333; // ~30 FPS - realistic for full play-area redraw

// Pause-Erkennung durch langen OK-Druck
static uint32_t okPressStartMs = 0;
static const uint32_t LONG_PRESS_MS = 1000;
static bool okWasPressed = false;

// Loop
void loop()
{
    Buttons.poll();

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

    // Memory diagnostics (every 2 seconds)
#ifdef ESP32
    static unsigned long lastHeapLog = 0;
    if (millis() - lastHeapLog > 2000)
    {
        size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        size_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        Serial.print("[HEAP] Free: ");
        Serial.print(freeHeap);
        Serial.print(" bytes, Largest block: ");
        Serial.println(largestBlock);
        lastHeapLog = millis();
    }
#endif

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
#ifdef DEBUG_BUTTONS
                Serial.println("[BTN] OK long-press detected - PAUSE GAME");
#endif
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

        // --- Frame zeichnen (3-phase dirty rect system) ---
        
        // PHASE 1: COLLECT - Update physics and collect dirty rects
        clearDirtyRects();
        setFramePhase(PHASE_COLLECT);
        
#ifndef DISABLE_BUBBLES
        bubblesSetFishOrigin((int16_t)getFishX(), (int16_t)getFishY());
        updateAndDrawBubbles(dtSecSmooth);
#endif
        updateAndDrawSeahorse();
        updateAndDrawShrimp(dtSecSmooth);
        updateDirt(dtSecSmooth);
        drawPetAnimated(dtSecSmooth);
        updateParticles(dtSecSmooth);
        drawParticles();
        
        // Merge overlapping dirty rects to optimize
        mergeDirtyRects();
        
        // PHASE 2: RESTORE - Restore dirty regions from background canvas
        setFramePhase(PHASE_RESTORE);
        processDirtyRects();
        
        // PHASE 3: DRAW - Draw all sprites in Z-order (last = foreground)
        setFramePhase(PHASE_DRAW);
        
#ifndef DISABLE_BUBBLES
        updateAndDrawBubbles(0);
#endif
        updateAndDrawSeahorse();
        updateAndDrawShrimp(0);
        drawDirt();
        drawPetAnimated(0);
        drawParticles();
        
#ifdef DEBUG_SPRITES
        static int checkFrames = 0;
        if (checkFrames++ > 120) {  // Every 2 seconds
            float fishX = getFishX();
            float fishY = getFishY();
            Serial.print("[SPRITES] Fish visible at (");
            Serial.print((int)fishX);
            Serial.print(",");
            Serial.print((int)fishY);
            Serial.println(")");
            checkFrames = 0;
        }
#endif
        
        // Draw menus LAST to ensure they're on top
        drawBottomMenu();
        drawStatusBar();
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
#ifdef DEBUG_BUTTONS
            Serial.println("[MAIN] Exiting to start menu...");
#endif
            // Wait for button release and debounce delay
            delay(300);
            
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
