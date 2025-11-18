#include "dirt.h"
#include "gfx.h"
#include "sprite_common.h"
#include "sprites/dirt_spots.h"
#include "particles.h"
#include "environment.h"
#include <math.h>

extern GFXcanvas16* bgCanvas;
extern bool gNoCanvas;

DirtSpot gDirtSpots[MAX_DIRT_SPOTS];

static float spawnAccumulator = 0.0f;
constexpr float MIN_SPAWN_INTERVAL = 45.0f;
constexpr float MAX_SPAWN_INTERVAL = 90.0f;
static float nextSpawnTime = 60.0f;

static uint8_t getStippleLevel(uint8_t strength) {
  if (strength > 75) return 4;  // 100%
  if (strength > 50) return 3;  // 75%
  if (strength > 25) return 2;  // 50%
  return 1;  // 25%
}

static bool shouldDrawDirtPixel(int16_t x, int16_t y, uint8_t level) {
  uint8_t pat = (x ^ y) & 3;
  if (level == 4) return true;
  if (level == 3) return pat != 3;
  if (level == 2) return pat < 2;
  return pat == 0;
}

static void drawDirtToCanvas(uint8_t spotIndex) {
  if (!bgCanvas || gNoCanvas) return;
  if (!gDirtSpots[spotIndex].active) return;
  
  const uint16_t* sprite = getDirtSprite(gDirtSpots[spotIndex].kind);
  int16_t x = gDirtSpots[spotIndex].x;
  int16_t y = gDirtSpots[spotIndex].y;
  uint8_t stippleLevel = getStippleLevel(gDirtSpots[spotIndex].strength);
  
  for (int16_t dy = 0; dy < DIRT_SPOT_SIZE; dy++) {
    for (int16_t dx = 0; dx < DIRT_SPOT_SIZE; dx++) {
      int16_t px = x + dx;
      int16_t py = y + dy;
      
      if (px < 0 || px >= TFT_WIDTH || py < 0 || py >= TFT_HEIGHT) continue;
      
      uint16_t color = pgm_read_word(&sprite[dy * DIRT_SPOT_SIZE + dx]);
      
      if (isTransparent16(color)) continue;
      
      if (shouldDrawDirtPixel(px, py, stippleLevel)) {
        bgCanvas->drawPixel(px, py, color);
      }
    }
  }
}

void initDirt() {
  for (uint8_t i = 0; i < MAX_DIRT_SPOTS; i++) {
    gDirtSpots[i].active = false;
    gDirtSpots[i].timeAlive = 0.0f;
    gDirtSpots[i].strength = 0;
  }
  spawnAccumulator = 0.0f;
  nextSpawnTime = random(MIN_SPAWN_INTERVAL * 10, MAX_SPAWN_INTERVAL * 10) / 10.0f;
}

void updateDirt(float deltaTime) {
  // Update existing spots
  for (uint8_t i = 0; i < MAX_DIRT_SPOTS; i++) {
    if (!gDirtSpots[i].active) continue;
    
    gDirtSpots[i].timeAlive += deltaTime;
    
    // Gradually increase strength over time (max 100)
    if (gDirtSpots[i].strength < 100) {
      uint8_t oldStrength = gDirtSpots[i].strength;
      float growthRate = 2.0f; // units per second
      gDirtSpots[i].strength = min(100, gDirtSpots[i].strength + (uint8_t)(growthRate * deltaTime));
      
      // Redraw to canvas when strength changes
      if (gDirtSpots[i].strength != oldStrength) {
        drawDirtToCanvas(i);
      }
    }
  }
  
  // Spawn new dirt spots periodically
  spawnAccumulator += deltaTime;
  if (spawnAccumulator >= nextSpawnTime) {
    spawnAccumulator = 0.0f;
    nextSpawnTime = random(MIN_SPAWN_INTERVAL * 10, MAX_SPAWN_INTERVAL * 10) / 10.0f;
    
    // Find free slot
    for (uint8_t i = 0; i < MAX_DIRT_SPOTS; i++) {
      if (!gDirtSpots[i].active) {
        gDirtSpots[i].active = true;
        gDirtSpots[i].kind = random(0, 4);
        gDirtSpots[i].strength = 15; // Start with low strength
        gDirtSpots[i].timeAlive = 0.0f;
        
        // Random position on "glass" (not on ground)
        // Avoid bottom 30px (ground area)
        int16_t margin = 20;
        gDirtSpots[i].x = PLAY_AREA_X + margin + random(0, max(10, PLAY_AREA_W - margin * 2));
        gDirtSpots[i].y = PLAY_AREA_Y + margin + random(0, max(10, PLAY_AREA_H - 50 - margin));
        
        // Draw to canvas immediately
        drawDirtToCanvas(i);
        break;
      }
    }
  }
}

void drawDirt() {
  // Dirt is now drawn directly to bgCanvas when spawned/updated
  // This function is kept for compatibility but does nothing
}

void cleanDirt() {
  for (uint8_t i = 0; i < MAX_DIRT_SPOTS; i++) {
    if (!gDirtSpots[i].active) continue;
    
    // Remove all dirt spots completely
    gDirtSpots[i].active = false;
    spawnDirtPuff(gDirtSpots[i].x + DIRT_SPOT_SIZE / 2, 
                  gDirtSpots[i].y + DIRT_SPOT_SIZE / 2, 
                  6);
  }
  
  // Redraw background canvas to remove dirt
  if (bgCanvas && !gNoCanvas) {
    bgCanvas->fillScreen(COLOR_BG);
    drawEnvironmentToCanvas(bgCanvas);
  }
}

void spawnPoopSpot(int16_t x) {
  for (uint8_t i = 0; i < MAX_DIRT_SPOTS; i++) {
    if (!gDirtSpots[i].active) {
      gDirtSpots[i].active = true;
      gDirtSpots[i].kind = random(0, 4);
      gDirtSpots[i].strength = 40;  // Visible but not max
      gDirtSpots[i].timeAlive = 0.0f;
      
      // Place on ground near fish X position with random offset
      int16_t groundH = 28;
      int16_t groundY = PLAY_AREA_Y + PLAY_AREA_H - groundH;
      
      // Add random offset to X position (-20 to +20 pixels)
      int16_t randomOffsetX = random(-20, 21);
      int16_t poopX = x + randomOffsetX - DIRT_SPOT_SIZE / 2;
      
      // Add small random offset to Y position (0 to 5 pixels up from ground)
      int16_t randomOffsetY = random(0, 6);
      int16_t poopY = groundY - DIRT_SPOT_SIZE - 1 - randomOffsetY;
      
      gDirtSpots[i].x = constrain(poopX,
                                  (int16_t)PLAY_AREA_X,
                                  (int16_t)(PLAY_AREA_X + PLAY_AREA_W - DIRT_SPOT_SIZE));
      gDirtSpots[i].y = constrain(poopY,
                                  (int16_t)(groundY - DIRT_SPOT_SIZE - 10),
                                  (int16_t)(groundY - DIRT_SPOT_SIZE));
      
      // Draw to canvas immediately
      drawDirtToCanvas(i);
      break;
    }
  }
}

uint8_t getTotalDirtLevel() {
  uint16_t total = 0;
  for (uint8_t i = 0; i < MAX_DIRT_SPOTS; i++) {
    if (gDirtSpots[i].active) {
      total += gDirtSpots[i].strength;
    }
  }
  // Return clamped to 255
  return (total > 255) ? 255 : (uint8_t)total;
}
