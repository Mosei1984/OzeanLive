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

void initShrimp() {
    // Calculate ground position (same as in main.cpp setup)
    int16_t groundH = 28;
    int16_t groundY = PLAY_AREA_Y + PLAY_AREA_H - groundH;
    
    shrimpX = random(PLAY_AREA_X, PLAY_AREA_X + PLAY_AREA_W - BEE_SHRIMP_WIDTH);
    shrimpY = groundY - 30;
    targetX = random(PLAY_AREA_X, PLAY_AREA_X + PLAY_AREA_W - BEE_SHRIMP_WIDTH);
    velocityX = 0;
    lastDrawX = -1;
    lastDrawY = -1;
}

void updateAndDrawShrimp(float deltaTime) {
    const float SHRIMP_SPEED = 15.0f;
    const float TARGET_THRESHOLD = 2.0f;

    if (abs(shrimpX - targetX) < TARGET_THRESHOLD) {
        targetX = random(PLAY_AREA_X, PLAY_AREA_X + PLAY_AREA_W - BEE_SHRIMP_WIDTH);
        velocityX = 0;
    }

    if (shrimpX < targetX) {
        velocityX = SHRIMP_SPEED;
        movingLeft = false;
    } else if (shrimpX > targetX) {
        velocityX = -SHRIMP_SPEED;
        movingLeft = true;
    }

    shrimpX += velocityX * deltaTime;

    shrimpX = constrain(shrimpX, (float)PLAY_AREA_X, (float)(PLAY_AREA_X + PLAY_AREA_W - BEE_SHRIMP_WIDTH));

    if (lastDrawX >= 0 && lastDrawY >= 0) {
        restoreRegion(lastDrawX, lastDrawY, BEE_SHRIMP_WIDTH, BEE_SHRIMP_HEIGHT);
    }

    int16_t drawX = (int16_t)shrimpX;
    int16_t drawY = (int16_t)shrimpY;

    drawSpriteOptimized(bee_shrimpBitmap, BEE_SHRIMP_WIDTH, BEE_SHRIMP_HEIGHT, drawX, drawY, movingLeft);

    lastDrawX = drawX;
    lastDrawY = drawY;
}
