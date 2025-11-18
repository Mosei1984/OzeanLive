#include "bubbles.h"
#include "gfx.h"
#include "sprites/small_bubble.h"
#include "sprites/medium_bubble.h"

// Access to background canvas for restore
extern GFXcanvas16* bgCanvas;
extern bool gNoCanvas;

static inline void restorePlayAreaRegion(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  int16_t rx = max<int16_t>(x, PLAY_AREA_X);
  int16_t ry = max<int16_t>(y, PLAY_AREA_Y);
  int16_t rmaxx = min<int16_t>(x + w, PLAY_AREA_X + PLAY_AREA_W);
  int16_t rmaxy = min<int16_t>(y + h, PLAY_AREA_Y + PLAY_AREA_H);
  if (rmaxx > rx && rmaxy > ry) restoreRegion(rx, ry, rmaxx - rx, rmaxy - ry);
}

struct Bubble {
  int16_t x;
  float   y;
  float   speed;
  bool    big;
  float   prevX;
  float   prevY;
  bool    active;
};

constexpr uint8_t NUM_BUBBLES = 10;
static Bubble bubbles[NUM_BUBBLES];

// Two independent timers
static float fishTimerSec = 0.0f;
static float floorTimerSec = 0.0f;

// Fish origin for spawns (-1 disables)
static int16_t fishOriginX = -1;
static int16_t fishOriginY = -1;

// Random interval generator (30-60s)
static float nextSpawnIntervalSec() {
  return 30.0f + random(0, 31);
}

// Find inactive bubble in pool
static Bubble* allocBubble() {
  for (uint8_t i = 0; i < NUM_BUBBLES; ++i) {
    if (!bubbles[i].active) {
      return &bubbles[i];
    }
  }
  return nullptr;
}

// Spawn bubble at position
static void spawnBubbleAt(int16_t x, int16_t y, bool big) {
  Bubble* b = allocBubble();
  if (!b) return;
  
  b->x = x;
  b->y = y;
  b->speed = 30.0f + random(0, 31);
  b->big = big;
  b->prevX = -1;
  b->prevY = -1;
  b->active = true;
}

void bubblesSetFishOrigin(int16_t x, int16_t y) {
  fishOriginX = x;
  fishOriginY = y;
}

void initBubbles() {
  for (uint8_t i = 0; i < NUM_BUBBLES; ++i) {
    bubbles[i].active = false;
  }
  
  fishTimerSec = nextSpawnIntervalSec();
  floorTimerSec = nextSpawnIntervalSec();
}

void restoreBubblesRegion() {
  // No longer needed - full play-area redraw handles this
}

void updateAndDrawBubbles(float dtSec) {
  FramePhase phase = getFramePhase();
  
  // COLLECT phase: Update physics and register dirty rects
  if (phase == PHASE_COLLECT) {
    // Fish timer (spawn 1-2 small bubbles at fish position every 30-60s)
    if (fishOriginX >= 0 && dtSec > 0) {
      fishTimerSec -= dtSec;
      if (fishTimerSec <= 0.0f) {
        int count = 1 + random(0, 2);
        for (int i = 0; i < count; ++i) {
          int16_t xOffset = random(-8, 9);
          spawnBubbleAt(fishOriginX + xOffset, fishOriginY, false);
        }
        fishTimerSec = nextSpawnIntervalSec();
      }
    }
    
    // Floor timer
    if (dtSec > 0) {
      floorTimerSec -= dtSec;
      if (floorTimerSec <= 0.0f) {
        int16_t floorY = PLAY_AREA_Y + PLAY_AREA_H + 10;
        for (int i = 0; i < 2; ++i) {
          int16_t x = PLAY_AREA_X + random(0, PLAY_AREA_W - SMALL_BUBBLE_WIDTH);
          spawnBubbleAt(x, floorY, false);
        }
        int mediumCount = 1 + random(0, 2);
        for (int i = 0; i < mediumCount; ++i) {
          int16_t x = PLAY_AREA_X + random(0, PLAY_AREA_W - MEDIUM_BUBBLE_WIDTH);
          spawnBubbleAt(x, floorY, true);
        }
        floorTimerSec = nextSpawnIntervalSec();
      }
    }
    
    // Update positions and collect dirty rects
    for (uint8_t i = 0; i < NUM_BUBBLES; ++i) {
      Bubble &b = bubbles[i];
      if (!b.active) continue;
      
      uint16_t w = b.big ? MEDIUM_BUBBLE_WIDTH : SMALL_BUBBLE_WIDTH;
      uint16_t h = b.big ? MEDIUM_BUBBLE_HEIGHT : SMALL_BUBBLE_HEIGHT;
      
      if (dtSec > 0) {
        b.y -= b.speed * dtSec;
      }

      int16_t drawY = (int16_t)b.y;
      
      // Deactivate when bubble moves above play area
      if (drawY < PLAY_AREA_Y - h) {
        // Register both current and previous positions to fully erase
        if (b.prevX >= 0 && b.prevY >= 0) {
          addDirtyRect(b.prevX, b.prevY, w, h);
        }
        addDirtyRect(b.x, drawY, w, h);  // Current position too
        b.active = false;
        b.prevX = -1;
        b.prevY = -1;
        continue;
      }
      
      // Register dirty rect for visible bubbles
      if (drawY >= PLAY_AREA_Y && drawY <= PLAY_AREA_Y + PLAY_AREA_H - h) {
        addDirtyRectPair(b.x, drawY, w, h, b.prevX, b.prevY);
      } else {
        // Moving outside visible area but not deactivated yet
        if (b.prevX >= 0 && b.prevY >= 0) {
          addDirtyRect(b.prevX, b.prevY, w, h);
        }
        b.prevX = -1;
        b.prevY = -1;
      }
    }
  }
  // DRAW phase: Draw bubbles and commit positions
  else if (phase == PHASE_DRAW) {
    for (uint8_t i = 0; i < NUM_BUBBLES; ++i) {
      Bubble &b = bubbles[i];
      if (!b.active) continue;
      
      uint16_t w = b.big ? MEDIUM_BUBBLE_WIDTH : SMALL_BUBBLE_WIDTH;
      uint16_t h = b.big ? MEDIUM_BUBBLE_HEIGHT : SMALL_BUBBLE_HEIGHT;
      int16_t drawY = (int16_t)b.y;
      int16_t minY = PLAY_AREA_Y;
      int16_t maxY = PLAY_AREA_Y + PLAY_AREA_H - h;
      
      if (drawY >= minY && drawY <= maxY) {
        // Draw bubble
        if (b.big) {
          drawSpriteBasic(medium_bubbleBitmap, MEDIUM_BUBBLE_WIDTH, MEDIUM_BUBBLE_HEIGHT,
                          b.x, drawY);
        } else {
          drawSpriteBasic(small_bubbleBitmap, SMALL_BUBBLE_WIDTH, SMALL_BUBBLE_HEIGHT,
                          b.x, drawY);
        }
        
        // Commit current position for next frame
        b.prevX = b.x;
        b.prevY = drawY;
      }
    }
  }
}
