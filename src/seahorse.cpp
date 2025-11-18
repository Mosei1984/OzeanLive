#include <math.h>
#include "seahorse.h"
#include "gfx.h"
#include "pet.h"
#include "sprites/seahorse_sprite.h"

// Check if sprite intersects play area
static inline bool intersectsPlayArea(int16_t x, int16_t y, int16_t w, int16_t h) {
  return x + w > PLAY_AREA_X && x < PLAY_AREA_X + PLAY_AREA_W &&
         y + h > PLAY_AREA_Y && y < PLAY_AREA_Y + PLAY_AREA_H;
}

// Globale Variablen aus main / pet
extern float animPhase;
int16_t seahorseBaseX = 0;
int16_t seahorseBaseY = 0;

static int16_t prevSeahorseY = -1;

// Second seahorse
int16_t seahorse2BaseX = 0;
int16_t seahorse2BaseY = 0;
static int16_t prevSeahorse2Y = -1;

void restoreSeahorseRegion() {
  constexpr int16_t SWAY_MARGIN = 3;

  // First seahorse - clip to PLAY_AREA
  if (prevSeahorseY >= 0) {
#ifdef DEBUG_GRAPHICS
    Serial.print("[SEAHORSE1-RESTORE] ");
#endif
    int16_t oldY = prevSeahorseY - SWAY_MARGIN;
    int16_t oldH = SEAHORSE_HEIGHT + SWAY_MARGIN * 2;
    
    int16_t rx = max<int16_t>(PLAY_AREA_X, seahorseBaseX);
    int16_t ry = max<int16_t>(PLAY_AREA_Y, oldY);
    int16_t rw = min<int16_t>(SEAHORSE_WIDTH, (int16_t)(PLAY_AREA_X + PLAY_AREA_W - rx));
    int16_t rh = min<int16_t>(oldH, (int16_t)(PLAY_AREA_Y + PLAY_AREA_H - ry));
    
    if (rw > 0 && rh > 0) {
      restoreRegion(rx, ry, rw, rh);
    }
    
    // Invalidate after restore
    prevSeahorseY = -1;
  }

  // Second seahorse - clip to PLAY_AREA
  if (prevSeahorse2Y >= 0) {
#ifdef DEBUG_GRAPHICS
    Serial.print("[SEAHORSE2-RESTORE] ");
#endif
    int16_t oldY = prevSeahorse2Y - SWAY_MARGIN;
    int16_t oldH = SEAHORSE2_HEIGHT + SWAY_MARGIN * 2;
    
    int16_t rx = max<int16_t>(PLAY_AREA_X, seahorse2BaseX);
    int16_t ry = max<int16_t>(PLAY_AREA_Y, oldY);
    int16_t rw = min<int16_t>(SEAHORSE2_WIDTH, (int16_t)(PLAY_AREA_X + PLAY_AREA_W - rx));
    int16_t rh = min<int16_t>(oldH, (int16_t)(PLAY_AREA_Y + PLAY_AREA_H - ry));
    
    if (rw > 0 && rh > 0) {
      restoreRegion(rx, ry, rw, rh);
    }
    
    // Invalidate after restore
    prevSeahorse2Y = -1;
  }
}

void updateAndDrawSeahorse() {
  FramePhase phase = getFramePhase();
  
  // COLLECT phase: Register dirty rects
  if (phase == PHASE_COLLECT) {
    // First seahorse
    float sway = sinf(animPhase * 0.7f) * 5.0f;
    int16_t y = static_cast<int16_t>(seahorseBaseY + sway);
    
    if (intersectsPlayArea(seahorseBaseX, y, SEAHORSE_WIDTH, SEAHORSE_HEIGHT)) {
      addDirtyRectPair(seahorseBaseX, y, SEAHORSE_WIDTH, SEAHORSE_HEIGHT, seahorseBaseX, prevSeahorseY);
    } else {
      prevSeahorseY = -1;
    }
    
    // Second seahorse
    float sway2 = sinf(animPhase * 0.5f + 1.5f) * 4.0f;
    int16_t y2 = static_cast<int16_t>(seahorse2BaseY + sway2);
    
    if (intersectsPlayArea(seahorse2BaseX, y2, SEAHORSE2_WIDTH, SEAHORSE2_HEIGHT)) {
      addDirtyRectPair(seahorse2BaseX, y2, SEAHORSE2_WIDTH, SEAHORSE2_HEIGHT, seahorse2BaseX, prevSeahorse2Y);
    } else {
      prevSeahorse2Y = -1;
    }
  }
  // DRAW phase: Draw seahorses and commit positions
  else if (phase == PHASE_DRAW) {
    // First seahorse
    float sway = sinf(animPhase * 0.7f) * 5.0f;
    int16_t y = static_cast<int16_t>(seahorseBaseY + sway);
    
    if (intersectsPlayArea(seahorseBaseX, y, SEAHORSE_WIDTH, SEAHORSE_HEIGHT)) {
      drawSpriteBasic(seahorseBitmap, SEAHORSE_WIDTH, SEAHORSE_HEIGHT,
                      seahorseBaseX, y);
      prevSeahorseY = y;
    }
    
    // Second seahorse
    float sway2 = sinf(animPhase * 0.5f + 1.5f) * 4.0f;
    int16_t y2 = static_cast<int16_t>(seahorse2BaseY + sway2);
    
    if (intersectsPlayArea(seahorse2BaseX, y2, SEAHORSE2_WIDTH, SEAHORSE2_HEIGHT)) {
      drawSpriteBasic(seahorse2Bitmap, SEAHORSE2_WIDTH, SEAHORSE2_HEIGHT,
                      seahorse2BaseX, y2);
      prevSeahorse2Y = y2;
    }
  }
}
