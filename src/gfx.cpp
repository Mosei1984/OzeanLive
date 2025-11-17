#include "gfx.h"
#include "sprite_common.h"

// Globale Display-Instanz
Adafruit_ST7789 tft = Adafruit_ST7789(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

// Play-Area
int16_t PLAY_AREA_X = 0;
int16_t PLAY_AREA_Y = 0;
int16_t PLAY_AREA_W = 0;
int16_t PLAY_AREA_H = 0;

// Background-Canvas
GFXcanvas16* bgCanvas = nullptr;

// Displayinitialisierung
void initDisplay() {
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(1); // Landscape: 320 (X) x 170 (Y)
  tft.fillScreen(COLOR_BG);
  tft.setTextWrap(false);
}

void initBackgroundCanvas() {
  bgCanvas = new GFXcanvas16(TFT_WIDTH, TFT_HEIGHT);
  if (!bgCanvas) {
    Serial.println("ERROR: Failed to allocate bgCanvas!");
  }
}

void restoreRegion(int16_t x, int16_t y, int16_t w, int16_t h) {
  if (!bgCanvas) return;
  
  uint16_t* buf = bgCanvas->getBuffer();
  tft.startWrite();
  for (int16_t r = 0; r < h; r++) {
    tft.setAddrWindow(x, y+r, w, 1);
    tft.writePixels(buf + (y+r)*TFT_WIDTH + x, w);
  }
  tft.endWrite();
}

// Einfache Sprite-Zeichenfunktion mit Transparenzfarbe
void drawSpriteBasic(const uint16_t *bitmap, uint16_t w, uint16_t h, int16_t x, int16_t y) {
  for (uint16_t py = 0; py < h; ++py) {
    for (uint16_t px = 0; px < w; ++px) {
      uint16_t color = pgm_read_word(&bitmap[py * w + px]);
      if (color == TRANSPARENT_COLOR) continue;

      int16_t sx = x + px;
      int16_t sy = y + py;
      if (sx < 0 || sy < 0 || sx >= TFT_WIDTH || sy >= TFT_HEIGHT) continue;

      tft.drawPixel(sx, sy, color);
    }
  }
}

void drawSpriteOptimized(const uint16_t* bmp, uint16_t w, uint16_t h, int16_t x, int16_t y, bool flipX) {
  static uint16_t buf[96];
  tft.startWrite();
  
  for (uint16_t py=0; py<h; ++py) {
    uint16_t px=0;
    while(px<w) {
      while(px<w) {
        uint16_t srcX = flipX ? (w-1-px) : px;
        uint16_t c = pgm_read_word(&bmp[py*w + srcX]);
        if (c != TRANSPARENT_COLOR) break;
        ++px;
      }
      if(px>=w) break;
      
      uint16_t start=px;
      while(px<w) {
        uint16_t srcX = flipX ? (w-1-px) : px;
        uint16_t c = pgm_read_word(&bmp[py*w + srcX]);
        if (c == TRANSPARENT_COLOR) break;
        ++px;
      }
      
      uint16_t len = px-start;
      uint16_t sx = x + start;
      uint16_t sy = y + py;
      
      if (sy < 0 || sy >= TFT_HEIGHT) continue;
      
      uint16_t remaining = len;
      uint16_t offset = 0;
      while(remaining) {
        uint16_t chunk = (remaining > 96) ? 96 : remaining;
        for(uint16_t i=0; i<chunk; i++) {
          uint16_t srcX = start+offset+i;
          if (flipX) srcX = w-1-srcX;
          buf[i] = pgm_read_word(&bmp[py*w + srcX]);
        }
        tft.setAddrWindow(sx+offset, sy, chunk, 1);
        tft.writePixels(buf, chunk);
        remaining -= chunk;
        offset += chunk;
      }
    }
  }
  tft.endWrite();
}
