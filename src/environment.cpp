#include "environment.h"
#include "sprite_common.h"

// Sprite-Header
#include "sprites/stone.h"
#include "sprites/kelp.h"
#include "sprites/corals.h"
#include "sprites/anemone_green.h"

// Seepferdchen-Basisposition wird in main festgelegt
extern int16_t seahorseBaseX;
extern int16_t seahorseBaseY;

// Helper: Sprite ins Canvas zeichnen
static void drawSpriteToCanvas(GFXcanvas16* canvas, const uint16_t *bitmap, uint16_t w, uint16_t h, int16_t x, int16_t y) {
  for (uint16_t py = 0; py < h; ++py) {
    for (uint16_t px = 0; px < w; ++px) {
      uint16_t color = pgm_read_word(&bitmap[py * w + px]);
      if (color == TRANSPARENT_COLOR) continue;
      
      int16_t sx = x + px;
      int16_t sy = y + py;
      if (sx < 0 || sy < 0 || sx >= TFT_WIDTH || sy >= TFT_HEIGHT) continue;
      
      canvas->drawPixel(sx, sy, color);
    }
  }
}

void drawEnvironmentToCanvas(GFXcanvas16* canvas) {
  // --- Wasser-Hintergrund ---
  int16_t midY = PLAY_AREA_Y + PLAY_AREA_H / 3;
  canvas->fillRect(PLAY_AREA_X, PLAY_AREA_Y, PLAY_AREA_W,
               midY - PLAY_AREA_Y, COLOR_WATER_TOP);
  canvas->fillRect(PLAY_AREA_X, midY, PLAY_AREA_W,
               PLAY_AREA_H - (midY - PLAY_AREA_Y), COLOR_WATER);

  // --- Sandboden ---
  int16_t groundH = 28;
  int16_t groundY = PLAY_AREA_Y + PLAY_AREA_H - groundH;

  canvas->fillRect(PLAY_AREA_X, groundY, PLAY_AREA_W, groundH, COLOR_SAND_DARK);
  canvas->fillRect(PLAY_AREA_X, groundY, PLAY_AREA_W, 8, COLOR_SAND_LIGHT);

  // Kleine "Schaumkante"
  for (int x = PLAY_AREA_X; x < PLAY_AREA_X + PLAY_AREA_W; x += 6) {
    canvas->drawPixel(x,     groundY - 1, COLOR_SAND_LIGHT);
    canvas->drawPixel(x + 3, groundY - 2, COLOR_SAND_LIGHT);
  }

  // --- Deko: Korallen, Kelp, Steine ---
  // Linke Seite: Kelp + Brain Coral
  int16_t kelpY = groundY - KELP_HEIGHT + 4;
  drawSpriteToCanvas(canvas, kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 4, kelpY);

  int16_t brainX = PLAY_AREA_X + 30;
  int16_t brainY = groundY - BRAIN_CORAL_HEIGHT + 4;
  drawSpriteToCanvas(canvas, brain_coralBitmap, BRAIN_CORAL_WIDTH, BRAIN_CORAL_HEIGHT, brainX, brainY);

  // Mitte links: Fan Coral + Stein davor
  int16_t fanX = PLAY_AREA_X + 80;
  int16_t fanY = groundY - FAN_CORAL_HEIGHT + 6;
  drawSpriteToCanvas(canvas, fan_coralBitmap, FAN_CORAL_WIDTH, FAN_CORAL_HEIGHT, fanX, fanY);

  drawSpriteToCanvas(canvas, stoneBitmap, STONE_WIDTH, STONE_HEIGHT,
                  fanX + 6, groundY + 8 - STONE_HEIGHT / 2);

  // Mitte rechts: Staghorn + Tube Coral
  int16_t stagX = PLAY_AREA_X + 140;
  int16_t stagY = groundY - STAGHORN_CORAL_HEIGHT + 6;
  drawSpriteToCanvas(canvas, staghorn_coralBitmap, STAGHORN_CORAL_WIDTH, STAGHORN_CORAL_HEIGHT,
                  stagX, stagY);

  int16_t tubeX = stagX + 22;
  int16_t tubeY = groundY - TUBE_CORAL_HEIGHT + 10;
  drawSpriteToCanvas(canvas, tube_coralBitmap, TUBE_CORAL_WIDTH, TUBE_CORAL_HEIGHT,
                  tubeX, tubeY);

  // Bottom-right: Anemone
  int16_t anemoneX = PLAY_AREA_X + PLAY_AREA_W - ANEMONE_GREEN_WIDTH - 5;
  int16_t anemoneY = groundY - ANEMONE_GREEN_HEIGHT + 10;
  drawSpriteToCanvas(canvas, anemone_greenBitmap, ANEMONE_GREEN_WIDTH, ANEMONE_GREEN_HEIGHT, anemoneX, anemoneY);
}

void drawEnvironment() {
  // --- Wasser-Hintergrund ---
  int16_t midY = PLAY_AREA_Y + PLAY_AREA_H / 3;
  tft.fillRect(PLAY_AREA_X, PLAY_AREA_Y, PLAY_AREA_W,
               midY - PLAY_AREA_Y, COLOR_WATER_TOP);
  tft.fillRect(PLAY_AREA_X, midY, PLAY_AREA_W,
               PLAY_AREA_H - (midY - PLAY_AREA_Y), COLOR_WATER);

  // --- Sandboden ---
  int16_t groundH = 28;
  int16_t groundY = PLAY_AREA_Y + PLAY_AREA_H - groundH;

  tft.fillRect(PLAY_AREA_X, groundY, PLAY_AREA_W, groundH, COLOR_SAND_DARK);
  tft.fillRect(PLAY_AREA_X, groundY, PLAY_AREA_W, 8, COLOR_SAND_LIGHT);

  // Kleine "Schaumkante"
  for (int x = PLAY_AREA_X; x < PLAY_AREA_X + PLAY_AREA_W; x += 6) {
    tft.drawPixel(x,     groundY - 1, COLOR_SAND_LIGHT);
    tft.drawPixel(x + 3, groundY - 2, COLOR_SAND_LIGHT);
  }

  // --- Deko: Korallen, Kelp, Steine ---
  // Linke Seite: Kelp + Brain Coral
  int16_t kelpY = groundY - KELP_HEIGHT + 4;
  drawSpriteBasic(kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 4, kelpY);

  int16_t brainX = PLAY_AREA_X + 30;
  int16_t brainY = groundY - BRAIN_CORAL_HEIGHT + 4;
  drawSpriteBasic(brain_coralBitmap, BRAIN_CORAL_WIDTH, BRAIN_CORAL_HEIGHT, brainX, brainY);

  // Mitte links: Fan Coral + Stein davor
  int16_t fanX = PLAY_AREA_X + 80;
  int16_t fanY = groundY - FAN_CORAL_HEIGHT + 6;
  drawSpriteBasic(fan_coralBitmap, FAN_CORAL_WIDTH, FAN_CORAL_HEIGHT, fanX, fanY);

  drawSpriteBasic(stoneBitmap, STONE_WIDTH, STONE_HEIGHT,
                  fanX + 6, groundY + 8 - STONE_HEIGHT / 2);

  // Mitte rechts: Staghorn + Tube Coral
  int16_t stagX = PLAY_AREA_X + 140;
  int16_t stagY = groundY - STAGHORN_CORAL_HEIGHT + 6;
  drawSpriteBasic(staghorn_coralBitmap, STAGHORN_CORAL_WIDTH, STAGHORN_CORAL_HEIGHT,
                  stagX, stagY);

  int16_t tubeX = stagX + 22;
  int16_t tubeY = groundY - TUBE_CORAL_HEIGHT + 10;
  drawSpriteBasic(tube_coralBitmap, TUBE_CORAL_WIDTH, TUBE_CORAL_HEIGHT,
                  tubeX, tubeY);

  // Bottom-right: Anemone
  int16_t anemoneX = PLAY_AREA_X + PLAY_AREA_W - ANEMONE_GREEN_WIDTH - 5;
  int16_t anemoneY = groundY - ANEMONE_GREEN_HEIGHT + 10;
  drawSpriteBasic(anemone_greenBitmap, ANEMONE_GREEN_WIDTH, ANEMONE_GREEN_HEIGHT, anemoneX, anemoneY);
}

void getAnemonePosition(int16_t& x, int16_t& y) {
  int16_t groundH = 28;
  int16_t groundY = PLAY_AREA_Y + PLAY_AREA_H - groundH;
  
  int16_t anemoneX = PLAY_AREA_X + PLAY_AREA_W - ANEMONE_GREEN_WIDTH - 5;
  int16_t anemoneY = groundY - ANEMONE_GREEN_HEIGHT + 10;
  
  x = anemoneX + ANEMONE_GREEN_WIDTH / 2;
  y = anemoneY + ANEMONE_GREEN_HEIGHT / 2;
}
