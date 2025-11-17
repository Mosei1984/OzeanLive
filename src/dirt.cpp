#include "dirt.h"
#include "gfx.h"
#include "sprite_common.h"
#include "sprites/dirt_spots.h"
#include "particles.h"
#include <math.h>

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
      float growthRate = 2.0f; // units per second
      gDirtSpots[i].strength = min(100, gDirtSpots[i].strength + (uint8_t)(growthRate * deltaTime));
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
        break;
      }
    }
  }
}

void drawDirt() {
  for (uint8_t i = 0; i < MAX_DIRT_SPOTS; i++) {
    if (!gDirtSpots[i].active) continue;
    
    uint8_t stippleLevel = getStippleLevel(gDirtSpots[i].strength);
    const uint16_t* sprite = getDirtSprite(gDirtSpots[i].kind);
    
    int16_t x = gDirtSpots[i].x;
    int16_t y = gDirtSpots[i].y;
    
    // Draw dirt spot with stipple pattern based on strength
    for (int16_t dy = 0; dy < DIRT_SPOT_SIZE; dy++) {
      for (int16_t dx = 0; dx < DIRT_SPOT_SIZE; dx++) {
        int16_t px = x + dx;
        int16_t py = y + dy;
        
        // Check bounds
        if (px < 0 || px >= TFT_WIDTH || py < 0 || py >= TFT_HEIGHT) continue;
        
        // Get sprite pixel
        uint16_t color = sprite[dy * DIRT_SPOT_SIZE + dx];
        
        // Skip transparent pixels
        if (color == TRANSPARENT_COLOR) continue;
        
        // Apply stipple pattern
        if (shouldDrawDirtPixel(px, py, stippleLevel)) {
          tft.drawPixel(px, py, color);
        }
      }
    }
  }
}

void cleanDirt() {
  for (uint8_t i = 0; i < MAX_DIRT_SPOTS; i++) {
    if (!gDirtSpots[i].active) continue;
    
    // Restore region first to erase existing dirt
    restoreRegion(gDirtSpots[i].x, gDirtSpots[i].y, DIRT_SPOT_SIZE, DIRT_SPOT_SIZE);
    
    // Reduce strength by 30-50
    int16_t reduction = random(30, 51);
    int16_t newStrength = (int16_t)gDirtSpots[i].strength - reduction;
    
    if (newStrength <= 0) {
      // Deactivate spot and spawn particles
      gDirtSpots[i].active = false;
      spawnDirtPuff(gDirtSpots[i].x + DIRT_SPOT_SIZE / 2, 
                    gDirtSpots[i].y + DIRT_SPOT_SIZE / 2, 
                    6);
    } else {
      gDirtSpots[i].strength = (uint8_t)newStrength;
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
