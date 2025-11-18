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

static uint16_t interpolateColor(uint16_t color1, uint16_t color2, float t) {
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;
  
  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;
  
  uint8_t r = r1 + (uint8_t)((r2 - r1) * t);
  uint8_t g = g1 + (uint8_t)((g2 - g1) * t);
  uint8_t b = b1 + (uint8_t)((b2 - b1) * t);
  
  return (r << 11) | (g << 5) | b;
}

static void drawSandTexture(GFXcanvas16* canvas, int16_t x, int16_t y, int16_t w, int16_t h) {
  for (int i = 0; i < (w * h) / 8; i++) {
    int16_t px = x + random(0, w);
    int16_t py = y + random(0, h);
    if (px >= 0 && px < TFT_WIDTH && py >= 0 && py < TFT_HEIGHT) {
      uint16_t color = (random(0, 100) < 70) ? COLOR_SAND_DARK : 0xDDB5;
      canvas->drawPixel(px, py, color);
    }
  }
  
  for (int i = 0; i < 8; i++) {
    int16_t px = x + random(5, w - 5);
    int16_t py = y + random(2, h - 2);
    uint16_t pebbleColor = 0xC618;
    canvas->fillCircle(px, py, 1, pebbleColor);
  }
}

// Helper: Sprite ins Canvas zeichnen
static void drawSpriteToCanvas(GFXcanvas16* canvas, const uint16_t *bitmap, uint16_t w, uint16_t h, int16_t x, int16_t y) {
  for (uint16_t py = 0; py < h; ++py) {
    for (uint16_t px = 0; px < w; ++px) {
      uint16_t color = pgm_read_word(&bitmap[py * w + px]);
      if (isTransparent16(color)) continue;
      
      int16_t sx = x + px;
      int16_t sy = y + py;
      if (sx < 0 || sy < 0 || sx >= TFT_WIDTH || sy >= TFT_HEIGHT) continue;
      
      canvas->drawPixel(sx, sy, color);
    }
  }
}

void drawEnvironmentToCanvas(GFXcanvas16* canvas) {
#ifdef DEBUG_GRAPHICS
  Serial.println("[ENV] Drawing environment to canvas...");
#endif
  
  // --- Wasser-Hintergrund mit sanftem Verlauf ---
  int16_t waterHeight = PLAY_AREA_H - 28;
  for (int16_t y = 0; y < waterHeight; y++) {
    float t = (float)y / (float)waterHeight;
    uint16_t color = interpolateColor(COLOR_WATER_TOP, COLOR_WATER, t);
    canvas->drawFastHLine(PLAY_AREA_X, PLAY_AREA_Y + y, PLAY_AREA_W, color);
  }
#ifdef DEBUG_GRAPHICS
  Serial.println("[ENV] Water gradient drawn");
#endif

  // --- Sandboden ---
  int16_t groundH = 28;
  int16_t groundY = PLAY_AREA_Y + PLAY_AREA_H - groundH;

  canvas->fillRect(PLAY_AREA_X, groundY, PLAY_AREA_W, groundH, COLOR_SAND_DARK);
  canvas->fillRect(PLAY_AREA_X, groundY, PLAY_AREA_W, 8, COLOR_SAND_LIGHT);

  for (int16_t y = 8; y < 18; y++) {
    float t = (float)(y - 8) / 10.0f;
    uint16_t color = interpolateColor(COLOR_SAND_LIGHT, COLOR_SAND_DARK, t);
    canvas->drawFastHLine(PLAY_AREA_X, groundY + y, PLAY_AREA_W, color);
  }

  drawSandTexture(canvas, PLAY_AREA_X, groundY, PLAY_AREA_W, groundH);

  for (int x = PLAY_AREA_X; x < PLAY_AREA_X + PLAY_AREA_W; x += 4) {
    int offset = random(0, 3);
    canvas->drawPixel(x + offset, groundY - 1, COLOR_SAND_LIGHT);
    if (random(0, 100) < 40) {
      canvas->drawPixel(x + offset, groundY - 2, 0xFFFF);
    }
  }

  // --- Deko: Korallen, Kelp, Steine ---
#ifdef DEBUG_SPRITES
  Serial.println("[ENV] Drawing sprites to canvas...");
#endif
  
  // Multiple kelp plants (5-7) distributed across bottom
  drawSpriteToCanvas(canvas, kelp2Bitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 1, groundY - KELP_HEIGHT + 4);
  drawSpriteToCanvas(canvas, kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 10, groundY - KELP_HEIGHT + 6);
  drawSpriteToCanvas(canvas, kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 140, groundY - KELP_HEIGHT + 5);
  drawSpriteToCanvas(canvas, kelp2Bitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 150, groundY - KELP_HEIGHT + 7);
  drawSpriteToCanvas(canvas, kelp2Bitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 160, groundY - KELP_HEIGHT + 4);
  drawSpriteToCanvas(canvas, kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 170, groundY - KELP_HEIGHT + 4);
  drawSpriteToCanvas(canvas, kelp2Bitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 180, groundY - KELP_HEIGHT + 6);
  drawSpriteToCanvas(canvas, kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 190, groundY - KELP_HEIGHT + 5);
#ifdef DEBUG_SPRITES
  Serial.println("[ENV] Kelp sprites drawn");
#endif

  // Seaweed plants (5 instances) clustered together
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 20, groundY - SEAWEED_HEIGHT + 8);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 32, groundY - SEAWEED_HEIGHT + 10);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 44, groundY - SEAWEED_HEIGHT + 7);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 56, groundY - SEAWEED_HEIGHT + 9);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 68, groundY - SEAWEED_HEIGHT + 11);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 80, groundY - SEAWEED_HEIGHT + 7);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 92, groundY - SEAWEED_HEIGHT + 6);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 104, groundY - SEAWEED_HEIGHT + 7);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 116, groundY - SEAWEED_HEIGHT + 5);
  drawSpriteToCanvas(canvas, seaweedBitmap, SEAWEED_WIDTH, SEAWEED_HEIGHT, PLAY_AREA_X + 128, groundY - SEAWEED_HEIGHT + 13);
#ifdef DEBUG_SPRITES
  Serial.println("[ENV] Seaweed sprites drawn");
#endif

  // Multiple rocks at varying heights (3-5 rocks)
  drawSpriteToCanvas(canvas, stone2Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 25, groundY + 10);
  drawSpriteToCanvas(canvas, stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 65, groundY + 20);
  drawSpriteToCanvas(canvas, stone3Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 85, groundY + 8);
  drawSpriteToCanvas(canvas, stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 105, groundY + 8);
  drawSpriteToCanvas(canvas, stone3Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 125, groundY );
  drawSpriteToCanvas(canvas, stone2Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 135, groundY );
  drawSpriteToCanvas(canvas, stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 155, groundY + 20);
  drawSpriteToCanvas(canvas, stone3Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 165, groundY + 20);
  drawSpriteToCanvas(canvas, stone2Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 175, groundY + 16);
  drawSpriteToCanvas(canvas, stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 195, groundY +16);
  drawSpriteToCanvas(canvas, stone3Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 205, groundY + 8);
  drawSpriteToCanvas(canvas, stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 215, groundY + 8);
  drawSpriteToCanvas(canvas, stone3Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 225, groundY + 6);
  drawSpriteToCanvas(canvas, stone2Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 235, groundY );
  drawSpriteToCanvas(canvas, stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 255, groundY + 20);
  drawSpriteToCanvas(canvas, stone3Bitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 245, groundY + 2);

  drawSpriteToCanvas(canvas, tube_coralBitmap, TUBE_CORAL_WIDTH, TUBE_CORAL_HEIGHT, PLAY_AREA_X + 200, groundY - TUBE_CORAL_HEIGHT+20);
  drawSpriteToCanvas(canvas, brain_coralBitmap, BRAIN_CORAL_WIDTH, BRAIN_CORAL_HEIGHT, PLAY_AREA_X + 210, groundY - BRAIN_CORAL_HEIGHT + 15);
  drawSpriteToCanvas(canvas, staghorn_coralBitmap, STAGHORN_CORAL_WIDTH, STAGHORN_CORAL_HEIGHT, PLAY_AREA_X + 220, groundY - STAGHORN_CORAL_HEIGHT + 7);
  drawSpriteToCanvas(canvas, fan_coralBitmap, FAN_CORAL_WIDTH, FAN_CORAL_HEIGHT, PLAY_AREA_X + 230, groundY - FAN_CORAL_HEIGHT + 4);
  drawSpriteToCanvas(canvas, tube_coralBitmap, TUBE_CORAL_WIDTH, TUBE_CORAL_HEIGHT, PLAY_AREA_X + 240, groundY - TUBE_CORAL_HEIGHT + 11);

  // Distributed corals with proper spacing based on sprite widths
  const int16_t CORAL_GAP = 15;

  int16_t brainX = PLAY_AREA_X + 35;
  int16_t brainY = groundY - BRAIN_CORAL_HEIGHT + 4;
  drawSpriteToCanvas(canvas, brain_coralBitmap, BRAIN_CORAL_WIDTH, BRAIN_CORAL_HEIGHT, brainX, brainY);

  int16_t fanX = brainX + BRAIN_CORAL_WIDTH + CORAL_GAP;
  int16_t fanY = groundY - FAN_CORAL_HEIGHT + 6;
  drawSpriteToCanvas(canvas, fan_coralBitmap, FAN_CORAL_WIDTH, FAN_CORAL_HEIGHT, fanX, fanY);

  int16_t stagX = fanX + FAN_CORAL_WIDTH + CORAL_GAP;
  int16_t stagY = groundY - STAGHORN_CORAL_HEIGHT + 6;
  drawSpriteToCanvas(canvas, staghorn_coralBitmap, STAGHORN_CORAL_WIDTH, STAGHORN_CORAL_HEIGHT, stagX, stagY);

  int16_t tubeX = stagX + STAGHORN_CORAL_WIDTH + CORAL_GAP;
  int16_t tubeY = groundY - TUBE_CORAL_HEIGHT + 20;
  drawSpriteToCanvas(canvas, tube_coralBitmap, TUBE_CORAL_WIDTH, TUBE_CORAL_HEIGHT, tubeX, tubeY);

  int16_t anemoneX = PLAY_AREA_X + PLAY_AREA_W - ANEMONE_GREEN_WIDTH ;
  int16_t anemoneY = groundY - ANEMONE_GREEN_HEIGHT+25;
  drawSpriteToCanvas(canvas, anemone_greenBitmap, ANEMONE_GREEN_WIDTH, ANEMONE_GREEN_HEIGHT, anemoneX, anemoneY);
#ifdef DEBUG_SPRITES
  Serial.println("[ENV] Corals and anemone sprite drawn");
  Serial.println("[ENV] All environment sprites loaded successfully!");
#endif
}

void drawEnvironment() {
  // --- Wasser-Hintergrund mit sanftem Verlauf ---
  int16_t waterHeight = PLAY_AREA_H - 28;
  for (int16_t y = 0; y < waterHeight; y++) {
    float t = (float)y / (float)waterHeight;
    uint16_t color = interpolateColor(COLOR_WATER_TOP, COLOR_WATER, t);
    tft.drawFastHLine(PLAY_AREA_X, PLAY_AREA_Y + y, PLAY_AREA_W, color);
  }

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
  // Multiple kelp plants (5-7) distributed across bottom
  drawSpriteBasic(kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 4, groundY - KELP_HEIGHT + 4);
  drawSpriteBasic(kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 50, groundY - KELP_HEIGHT + 6);
  drawSpriteBasic(kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 110, groundY - KELP_HEIGHT + 5);
  drawSpriteBasic(kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 165, groundY - KELP_HEIGHT + 7);
  drawSpriteBasic(kelpBitmap, KELP_WIDTH, KELP_HEIGHT, PLAY_AREA_X + 200, groundY - KELP_HEIGHT + 4);

  // Multiple rocks at varying heights (3-5 rocks)
  drawSpriteBasic(stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 25, groundY + 10);
  drawSpriteBasic(stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 95, groundY + 6);
  drawSpriteBasic(stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 145, groundY + 8);
  drawSpriteBasic(stoneBitmap, STONE_WIDTH, STONE_HEIGHT, PLAY_AREA_X + 190, groundY + 12);

  // Distributed corals with proper spacing based on sprite widths
  const int16_t CORAL_GAP = 15;

  int16_t brainX = PLAY_AREA_X + 35;
  int16_t brainY = groundY - BRAIN_CORAL_HEIGHT + 4;
  drawSpriteBasic(brain_coralBitmap, BRAIN_CORAL_WIDTH, BRAIN_CORAL_HEIGHT, brainX, brainY);

  int16_t fanX = brainX + BRAIN_CORAL_WIDTH + CORAL_GAP;
  int16_t fanY = groundY - FAN_CORAL_HEIGHT + 6;
  drawSpriteBasic(fan_coralBitmap, FAN_CORAL_WIDTH, FAN_CORAL_HEIGHT, fanX, fanY);

  int16_t stagX = fanX + FAN_CORAL_WIDTH + CORAL_GAP;
  int16_t stagY = groundY - STAGHORN_CORAL_HEIGHT + 6;
  drawSpriteBasic(staghorn_coralBitmap, STAGHORN_CORAL_WIDTH, STAGHORN_CORAL_HEIGHT, stagX, stagY);

  int16_t tubeX = stagX + STAGHORN_CORAL_WIDTH + CORAL_GAP;
  int16_t tubeY = groundY - TUBE_CORAL_HEIGHT + 10;
  drawSpriteBasic(tube_coralBitmap, TUBE_CORAL_WIDTH, TUBE_CORAL_HEIGHT, tubeX, tubeY);

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
