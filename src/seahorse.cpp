#include <math.h>
#include "seahorse.h"
#include "gfx.h"
#include "pet.h"
#include "sprites/seahorse_sprite.h"

// Globale Variablen aus main / pet
extern float animPhase;
int16_t seahorseBaseX = 0;
int16_t seahorseBaseY = 0;

static int16_t prevSeahorseY = -1;

void updateAndDrawSeahorse() {
  constexpr int16_t SWAY_MARGIN = 6;
  
  if (prevSeahorseY >= 0) {
    int16_t oldY = prevSeahorseY - SWAY_MARGIN;
    int16_t oldH = SEAHORSE_HEIGHT + SWAY_MARGIN * 2;
    restoreRegion(seahorseBaseX, oldY, SEAHORSE_WIDTH, oldH);
  }
  
  float sway = sinf(animPhase * 0.7f) * 5.0f;
  int16_t y = static_cast<int16_t>(seahorseBaseY + sway);

  drawSpriteBasic(seahorseBitmap, SEAHORSE_WIDTH, SEAHORSE_HEIGHT,
                  seahorseBaseX, y);
  
  prevSeahorseY = seahorseBaseY;
}
