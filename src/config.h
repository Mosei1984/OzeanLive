#pragma once
#include <Arduino.h>

// Displaygröße (ST7789V2 1.9" 170x320, Landscape)
constexpr int16_t TFT_WIDTH = 320;
constexpr int16_t TFT_HEIGHT = 170;

// Status- und Bottom-Bar Höhe
constexpr int16_t STATUS_BAR_H = 16;
constexpr int16_t BOTTOM_BAR_H = 20;

// Play-Area (wird in setup() gesetzt)
extern int16_t PLAY_AREA_X;
extern int16_t PLAY_AREA_Y;
extern int16_t PLAY_AREA_W;
extern int16_t PLAY_AREA_H;

#ifdef ESP32
// ESP32 Pin-Mapping
constexpr uint8_t PIN_TFT_CS = 5;
constexpr uint8_t PIN_TFT_DC = 16;
constexpr uint8_t PIN_TFT_RST = 17;
constexpr uint8_t PIN_TFT_BL = 4;   // Backlight control

// Buttons (Pullup, aktiv LOW)
constexpr uint8_t PIN_BTN_LEFT = 25;
constexpr uint8_t PIN_BTN_OK = 26;
constexpr uint8_t PIN_BTN_RIGHT = 27;
#else
// Pins Teensy 4.1
constexpr uint8_t PIN_TFT_CS = 10;
constexpr uint8_t PIN_TFT_DC = 9;
constexpr uint8_t PIN_TFT_RST = 8;
constexpr uint8_t PIN_TFT_BL = 7;   // Backlight control

// Buttons (Pullup, aktiv LOW)
constexpr uint8_t PIN_BTN_LEFT = 2;
constexpr uint8_t PIN_BTN_OK = 3;
constexpr uint8_t PIN_BTN_RIGHT = 4;
#endif

// Farben (kannst du später anpassen)
const uint16_t COLOR_BG = 0x0000;         // Schwarz
const uint16_t COLOR_WATER = 0x0010;      // dunkles Blau
const uint16_t COLOR_WATER_TOP = 0x07FF;  // Cyan
const uint16_t COLOR_SAND_LIGHT = 0xFEE5; // heller Sand
const uint16_t COLOR_SAND_DARK = 0xE69D;  // dunklerer Sand
const uint16_t COLOR_STATUS_BG = 0x4208;  // dunkles Grau

// Animationsgeschwindigkeit (zeitbasiert: Phase-Units pro Sekunde)
constexpr float ANIM_PHASE_RATE = 4.5f;

// Frame timing constants
constexpr float DT_MIN = 0.004f;      // 4ms min deltaTime
constexpr float DT_MAX = 0.05f;       // 50ms max deltaTime
constexpr float DT_EMA_ALPHA = 0.2f;  // EMA smoothing weight

// Debug settings  
//#define DEBUG_BUTTONS      // Comment out to disable button debug output
//#define DEBUG_GRAPHICS     // Comment out to disable graphics debug output
//#define DEBUG_SPRITES      // Comment out to disable sprite debug output
//#define DEBUG_GAME_LOGIC   // Comment out to disable game logic debug output

// A/B Test toggles
//#define DISABLE_BUBBLES  // Uncomment to test without bubbles
