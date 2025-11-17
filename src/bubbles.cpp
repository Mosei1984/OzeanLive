#include "bubbles.h"
#include "gfx.h"
#include "sprites/small_bubble.h"
#include "sprites/medium_bubble.h"

struct Bubble {
  int16_t x;
  float   y;
  float   speed;
  bool    big;
  float   prevX;
  float   prevY;
};

constexpr uint8_t NUM_BUBBLES = 10;
static Bubble bubbles[NUM_BUBBLES];

static void resetBubble(Bubble &b) {
  b.x = PLAY_AREA_X + random(0, PLAY_AREA_W);
  b.y = PLAY_AREA_Y + PLAY_AREA_H + random(5, 40);
  b.speed = 30.0f + random(0, 31);
  b.big = (random(0, 100) < 40);
  b.prevX = -1;
  b.prevY = -1;
}

void initBubbles() {
  for (uint8_t i = 0; i < NUM_BUBBLES; ++i) {
    resetBubble(bubbles[i]);
    bubbles[i].y = PLAY_AREA_Y + random(0, PLAY_AREA_H);
  }
}

void updateAndDrawBubbles(float dtSec) {
  for (uint8_t i = 0; i < NUM_BUBBLES; ++i) {
    Bubble &b = bubbles[i];
    
    uint16_t w = b.big ? MEDIUM_BUBBLE_WIDTH : SMALL_BUBBLE_WIDTH;
    uint16_t h = b.big ? MEDIUM_BUBBLE_HEIGHT : SMALL_BUBBLE_HEIGHT;
    
    if (b.prevX >= 0) {
      restoreRegion((int16_t)b.prevX, (int16_t)b.prevY, w, h);
    }
    
    b.y -= b.speed * dtSec;

    if (b.y < PLAY_AREA_Y - 20) {
      resetBubble(b);
      continue;
    }

    if (b.big) {
      drawSpriteBasic(medium_bubbleBitmap, MEDIUM_BUBBLE_WIDTH, MEDIUM_BUBBLE_HEIGHT,
                      b.x, (int16_t)b.y);
    } else {
      drawSpriteBasic(small_bubbleBitmap, SMALL_BUBBLE_WIDTH, SMALL_BUBBLE_HEIGHT,
                      b.x, (int16_t)b.y);
    }
    
    b.prevX = b.x;
    b.prevY = b.y;
  }
}
