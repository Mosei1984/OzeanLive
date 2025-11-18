#include "shrimp.h"
#include "gfx.h"
#include "sprites/bee_shrimp.h"

static float shrimpX = 0;
static float shrimpY = 0;
static float targetX = 0;
static float velocityX = 0;
static int16_t lastDrawX = -1;
static int16_t lastDrawY = -1;
static bool movingLeft = false;
static float animTimer = 0.0f;
static int currentFrame = 0;

void initShrimp() {
    // Calculate ground position (same as in main.cpp setup)
    int16_t groundH = 28;
    int16_t groundY = PLAY_AREA_Y + PLAY_AREA_H - groundH;
    
    shrimpX = random(PLAY_AREA_X, PLAY_AREA_X + PLAY_AREA_W - BEE_SHRIMP_WIDTH);
    shrimpY = groundY - 0;
    targetX = random(PLAY_AREA_X, PLAY_AREA_X + PLAY_AREA_W - BEE_SHRIMP_WIDTH);
    velocityX = 0;
    lastDrawX = -1;
    lastDrawY = -1;
}

void updateAndDrawShrimp(float deltaTime) {
    FramePhase phase = getFramePhase();
    const float SHRIMP_SPEED = 15.0f;
    const float TARGET_THRESHOLD = 2.0f;

    // COLLECT phase: Update physics and register dirty rects
    if (phase == PHASE_COLLECT) {
        // Check if reached target and pick new one
        if (abs(shrimpX - targetX) < TARGET_THRESHOLD) {
            targetX = random(PLAY_AREA_X, PLAY_AREA_X + PLAY_AREA_W - BEE_SHRIMP_WIDTH);
        }

        // Always move toward target (no pauses)
        if (shrimpX < targetX) {
            velocityX = SHRIMP_SPEED;
            movingLeft = true;
        } else {
            velocityX = -SHRIMP_SPEED;
            movingLeft = false;
        }

        if (deltaTime > 0) {
            shrimpX += velocityX * deltaTime;
        }

        shrimpX = constrain(shrimpX, (float)PLAY_AREA_X, (float)(PLAY_AREA_X + PLAY_AREA_W - BEE_SHRIMP_WIDTH));

        if (abs(velocityX) > 0.1f && deltaTime > 0) {
            animTimer += deltaTime * 6.0f;
            if (animTimer >= 1.0f) {
                animTimer -= 1.0f;
                currentFrame = (currentFrame + 1) % 3;
            }
        } else {
            currentFrame = 0;
            animTimer = 0.0f;
        }

        int16_t drawX = (int16_t)shrimpX;
        int16_t drawY = (int16_t)shrimpY;

        // Register dirty rect for shrimp (current + previous position)
        addDirtyRectPair(drawX, drawY, BEE_SHRIMP_WIDTH, BEE_SHRIMP_HEIGHT, lastDrawX, lastDrawY);
    }
    // DRAW phase: Draw shrimp and commit position
    else if (phase == PHASE_DRAW) {
        const uint16_t* sprite;
        switch(currentFrame) {
            case 1: sprite = shrimp_move_Bitmap; break;
            case 2: sprite = shrimp_move01_Bitmap; break;
            default: sprite = bee_shrimpBitmap; break;
        }

        int16_t drawX = (int16_t)shrimpX;
        int16_t drawY = (int16_t)shrimpY;
        
        drawSpriteOptimized(sprite, BEE_SHRIMP_WIDTH, BEE_SHRIMP_HEIGHT, drawX, drawY, movingLeft);

        // Commit current position for next frame
        lastDrawX = drawX;
        lastDrawY = drawY;
    }
}

void restoreShrimpRegion() {
  if (lastDrawX >= 0 && lastDrawY >= 0) {
#ifdef DEBUG_GRAPHICS
    Serial.print("[SHRIMP-RESTORE] ");
#endif
    const int16_t margin = 2;
    
    int16_t rx = max<int16_t>(PLAY_AREA_X, (int16_t)(lastDrawX - margin));
    int16_t ry = max<int16_t>(PLAY_AREA_Y, (int16_t)(lastDrawY - margin));
    int16_t rw = min<int16_t>((int16_t)(BEE_SHRIMP_WIDTH + 2 * margin),
                              (int16_t)(PLAY_AREA_X + PLAY_AREA_W - rx));
    int16_t rh = min<int16_t>((int16_t)(BEE_SHRIMP_HEIGHT + 2 * margin),
                              (int16_t)(PLAY_AREA_Y + PLAY_AREA_H - ry));
    
    if (rw > 0 && rh > 0) {
      restoreRegion(rx, ry, rw, rh);
    }
    
    // Invalidate after restore to prevent double-restore
    lastDrawX = -1;
    lastDrawY = -1;
  }
}
