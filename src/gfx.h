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

// Frame phase tracking
enum FramePhase {
  PHASE_COLLECT,   // Update physics and collect dirty rects
  PHASE_RESTORE,   // Restore dirty regions from bgCanvas
  PHASE_DRAW       // Draw sprites
};

extern volatile int gFrameId;

void setFramePhase(FramePhase phase);
FramePhase getFramePhase();

// Initialisierung
void initDisplay();
void initBackgroundCanvas();

// Sprite-Zeichenfunktion (mit Transparenz)
void drawSpriteBasic(const uint16_t *bitmap, uint16_t w, uint16_t h, int16_t x, int16_t y);

// Optimierte Sprite-Zeichenfunktion mit Scanline-Run-Batching und FlipX
void drawSpriteOptimized(const uint16_t* bitmap, uint16_t w, uint16_t h, int16_t x, int16_t y, bool flipX);

// Dirty-Region-Restore
void restoreRegion(int16_t x, int16_t y, int16_t w, int16_t h);

// Fast play-area restore from background canvas
void blitPlayAreaFromCanvas();

// Death screen
void drawDeathScreen();

// Backlight control
void setBacklight(bool on);

// ---- Dirty Rectangle System ----
struct DirtyRect {
  int16_t x, y, w, h;
  bool valid;
};

// Initialize dirty rect system
void initDirtyRects();

// Clear dirty rect list for new frame
void clearDirtyRects();

// Add a dirty rect (current + previous position of sprite)
void addDirtyRect(int16_t x, int16_t y, uint16_t w, uint16_t h);
void addDirtyRectPair(int16_t x, int16_t y, uint16_t w, uint16_t h, 
                      float prevX, float prevY);

// Merge overlapping/nearby dirty rects to reduce draw calls
void mergeDirtyRects();

// Restore and draw all dirty regions
void processDirtyRects();
