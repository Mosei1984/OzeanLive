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

// Pins Teensy 4.1 – bei Bedarf anpassen
constexpr uint8_t PIN_TFT_CS = 10;
constexpr uint8_t PIN_TFT_DC = 9;
constexpr uint8_t PIN_TFT_RST = 8;

// Buttons (Pullup, aktiv LOW)
constexpr uint8_t PIN_BTN_LEFT = 2;
constexpr uint8_t PIN_BTN_OK = 3;
constexpr uint8_t PIN_BTN_RIGHT = 4;

// Farben (kannst du später anpassen)
const uint16_t COLOR_BG = 0x0000;         // Schwarz
const uint16_t COLOR_WATER = 0x0010;      // dunkles Blau
const uint16_t COLOR_WATER_TOP = 0x07FF;  // Cyan
const uint16_t COLOR_SAND_LIGHT = 0xFEE5; // heller Sand
const uint16_t COLOR_SAND_DARK = 0xE69D;  // dunklerer Sand
const uint16_t COLOR_STATUS_BG = 0x4208;  // dunkles Grau

// Animationsgeschwindigkeit (zeitbasiert: Phase-Units pro Sekunde)
constexpr float ANIM_PHASE_RATE = 4.5f;
