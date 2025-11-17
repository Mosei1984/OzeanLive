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
  
  // Hard-clip to screen bounds to prevent OOB access
  int16_t x0 = max<int16_t>(0, x);
  int16_t y0 = max<int16_t>(0, y);
  int16_t x1 = min<int16_t>(TFT_WIDTH,  x + w);
  int16_t y1 = min<int16_t>(TFT_HEIGHT, y + h);
  
  // Nothing to draw if clipped entirely
  if (x0 >= x1 || y0 >= y1) return;
  
  uint16_t* buf = bgCanvas->getBuffer();
  tft.startWrite();
  for (int16_t r = y0; r < y1; r++) {
    tft.setAddrWindow(x0, r, x1 - x0, 1);
    tft.writePixels(buf + r * TFT_WIDTH + x0, x1 - x0);
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
    int16_t sy = y + py;
    
    // Skip scanline if outside vertical bounds
    if (sy < 0 || sy >= TFT_HEIGHT) continue;
    
    uint16_t px=0;
    while(px<w) {
      // Skip transparent pixels
      while(px<w) {
        uint16_t srcX = flipX ? (w-1-px) : px;
        uint16_t c = pgm_read_word(&bmp[py*w + srcX]);
        if (c != TRANSPARENT_COLOR) break;
        ++px;
      }
      if(px>=w) break;
      
      // Find run of non-transparent pixels
      uint16_t start=px;
      while(px<w) {
        uint16_t srcX = flipX ? (w-1-px) : px;
        uint16_t c = pgm_read_word(&bmp[py*w + srcX]);
        if (c == TRANSPARENT_COLOR) break;
        ++px;
      }
      
      uint16_t len = px-start;
      int16_t runX0 = x + start;
      int16_t runX1 = x + start + len;
      
      // Clip run to horizontal screen bounds
      if (runX1 <= 0 || runX0 >= TFT_WIDTH) continue;
      
      int16_t clipStart = max<int16_t>(0, runX0);
      int16_t clipEnd = min<int16_t>(TFT_WIDTH, runX1);
      uint16_t clipLen = clipEnd - clipStart;
      
      // Calculate offset into sprite for clipped region
      uint16_t spriteOffset = clipStart - runX0;
      
      // Write clipped run in chunks
      uint16_t remaining = clipLen;
      uint16_t offset = 0;
      while(remaining) {
        uint16_t chunk = (remaining > 96) ? 96 : remaining;
        for(uint16_t i=0; i<chunk; i++) {
          uint16_t srcX = start + spriteOffset + offset + i;
          if (flipX) srcX = w-1-srcX;
          buf[i] = pgm_read_word(&bmp[py*w + srcX]);
        }
        tft.setAddrWindow(clipStart+offset, sy, chunk, 1);
        tft.writePixels(buf, chunk);
        remaining -= chunk;
        offset += chunk;
      }
    }
  }
  tft.endWrite();
}
