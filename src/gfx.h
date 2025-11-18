#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"

// Globale Display-Instanz
extern Adafruit_ST7789 tft;

// Play-Area Variablen
extern int16_t PLAY_AREA_X;
extern int16_t PLAY_AREA_Y;
extern int16_t PLAY_AREA_W;
extern int16_t PLAY_AREA_H;

// Background-Canvas (320x170, 16-bit)
extern GFXcanvas16* bgCanvas;
extern bool gNoCanvas;

// Initialisierung
void initDisplay();
void initBackgroundCanvas();

// Sprite-Zeichenfunktion (mit Transparenz)
void drawSpriteBasic(const uint16_t *bitmap, uint16_t w, uint16_t h, int16_t x, int16_t y);

// Optimierte Sprite-Zeichenfunktion mit Scanline-Run-Batching und FlipX
void drawSpriteOptimized(const uint16_t* bitmap, uint16_t w, uint16_t h, int16_t x, int16_t y, bool flipX);

// Dirty-Region-Restore
void restoreRegion(int16_t x, int16_t y, int16_t w, int16_t h);

// Death screen
void drawDeathScreen();
