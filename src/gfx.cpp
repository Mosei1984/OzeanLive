#include "gfx.h"
#include "sprite_common.h"
#include "sprites/clownfish.h"

// Frame phase tracking for debugging
static FramePhase currentPhase = PHASE_COLLECT;
volatile int gFrameId = 0;

FramePhase getFramePhase() {
  return currentPhase;
}

void setFramePhase(FramePhase phase) {
  if (phase == PHASE_COLLECT) {
    gFrameId++;
  }
  currentPhase = phase;
}

// Globale Display-Instanz
Adafruit_ST7789 tft = Adafruit_ST7789(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

// Play-Area
int16_t PLAY_AREA_X = 0;
int16_t PLAY_AREA_Y = 0;
int16_t PLAY_AREA_W = 0;
int16_t PLAY_AREA_H = 0;

// Background-Canvas
GFXcanvas16 *bgCanvas = nullptr;
bool gNoCanvas = false;

// Displayinitialisierung
void initDisplay()
{
#ifdef DEBUG_GRAPHICS
    Serial.println("[GFX] Initializing display...");
#endif

#ifdef ESP32
    // Initialize SPI with stable speed for ST7789V2
    SPI.begin();
    SPI.setFrequency(40000000); // 20 MHz - stable for most ESP32
#endif

    // Initialize display - ST7789V2 specific for 170x320 panel
    tft.init(170, 320, SPI_MODE0); // Explicit dimensions for 170x320 variant
    tft.setRotation(1); // Landscape mode (320x170)
    
    // Apply 35-pixel column offset for 170x320 panel on 240x320 controller
    tft.startWrite();
    tft.writeCommand(0x2A); // CASET - Column Address Set  
    tft.spiWrite(0x00);     // Start column high byte
    tft.spiWrite(35);       // Start at column 35
    tft.spiWrite(0x00);     // End column high byte
    tft.spiWrite(35 + TFT_HEIGHT - 1); // End column (35 + 170 - 1 = 204)
    tft.writeCommand(0x2B); // RASET - Row Address Set
    tft.spiWrite(0x00);     // Start row high byte
    tft.spiWrite(0);        // Start row 0
    tft.spiWrite((TFT_WIDTH - 1) >> 8); // End row high byte
    tft.spiWrite((TFT_WIDTH - 1) & 0xFF); // End row low byte (319)
    tft.endWrite();
    
    tft.fillScreen(COLOR_BG);
    tft.setTextWrap(false);
    
    // Initialize backlight pin (HIGH = ON)
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);  // Turn on backlight
    
#ifdef DEBUG_GRAPHICS
    Serial.print("[GFX] Display initialized: ");
    Serial.print(TFT_WIDTH);
    Serial.print("x");
    Serial.println(TFT_HEIGHT);
#endif
}

void initBackgroundCanvas()
{
    if (bgCanvas)
        return; // Already allocated, prevent leak

#ifdef DEBUG_GRAPHICS
    Serial.print("[GFX] Allocating background canvas (");
    Serial.print(TFT_WIDTH * TFT_HEIGHT * 2);
    Serial.println(" bytes)...");
#endif

#ifdef ESP32
    // ESP32: Try allocation, handle failure gracefully
    bgCanvas = new (std::nothrow) GFXcanvas16(TFT_WIDTH, TFT_HEIGHT);
    if (!bgCanvas)
    {
        Serial.println("[GFX] WARNING: Canvas allocation failed (low heap). Running without background cache.");
        gNoCanvas = true;
        return;
    }
#ifdef DEBUG_GRAPHICS
    Serial.print("[GFX] Canvas allocated successfully. Free heap: ");
    Serial.println(ESP.getFreeHeap());
#endif
#else
    // Teensy has enough RAM
    bgCanvas = new GFXcanvas16(TFT_WIDTH, TFT_HEIGHT);
    if (!bgCanvas)
    {
        Serial.println("[GFX] ERROR: Failed to allocate bgCanvas!");
        gNoCanvas = true;
    }
#ifdef DEBUG_GRAPHICS
    else
    {
        Serial.println("[GFX] Canvas allocated successfully (Teensy)");
    }
#endif
#endif
}

void restoreRegion(int16_t x, int16_t y, int16_t w, int16_t h)
{
#ifdef DEBUG_GRAPHICS
    if (currentPhase != PHASE_RESTORE) {
        Serial.print("[RESTORE] CRITICAL: Late restore at (");
        Serial.print(x);
        Serial.print(",");
        Serial.print(y);
        Serial.print(") size ");
        Serial.print(w);
        Serial.print("x");
        Serial.print(h);
        Serial.print(" in DRAW phase, frame ");
        Serial.println(gFrameId);
        return; // BLOCK late restores!
    }
    
    // Check for potential sprite overlap
    if (x < 200 && x + w > 100 && y < 110 && y + h > 50) {
        Serial.print("[RESTORE] Area: (");
        Serial.print(x);
        Serial.print(",");
        Serial.print(y);
        Serial.print(") size ");
        Serial.print(w);
        Serial.print("x");
        Serial.print(h);
        Serial.print(" - ");
        if (w * h > 1000) {
            Serial.println("LARGE!");
        } else {
            Serial.println("ok");
        }
    }
#endif

    if (gNoCanvas || !bgCanvas)
        return;

    // Hard-clip to screen bounds to prevent OOB access
    int16_t x0 = (x > 0) ? x : 0;
    int16_t y0 = (y > 0) ? y : 0;
    int16_t x1 = ((x + w) < TFT_WIDTH) ? (x + w) : TFT_WIDTH;
    int16_t y1 = ((y + h) < TFT_HEIGHT) ? (y + h) : TFT_HEIGHT;

    // Nothing to draw if clipped entirely
    if (x0 >= x1 || y0 >= y1)
        return;

    uint16_t *buf = bgCanvas->getBuffer();
    tft.startWrite();
    for (int16_t r = y0; r < y1; r++)
    {
        tft.setAddrWindow(x0, r, x1 - x0, 1);
        tft.writePixels(buf + r * TFT_WIDTH + x0, x1 - x0);
    }
    tft.endWrite();
}

// Einfache Sprite-Zeichenfunktion mit Transparenzfarbe
void drawSpriteBasic(const uint16_t *bitmap, uint16_t w, uint16_t h, int16_t x, int16_t y)
{
    for (uint16_t py = 0; py < h; ++py)
    {
        for (uint16_t px = 0; px < w; ++px)
        {
            uint16_t color = pgm_read_word(&bitmap[py * w + px]);
            if (isTransparent16(color))
                continue;

            int16_t sx = x + px;
            int16_t sy = y + py;
            if (sx < 0 || sy < 0 || sx >= TFT_WIDTH || sy >= TFT_HEIGHT)
                continue;

            tft.drawPixel(sx, sy, color);
        }
    }
}

void drawSpriteOptimized(const uint16_t *bmp, uint16_t w, uint16_t h, int16_t x, int16_t y, bool flipX)
{
    static uint16_t buf[96];
    tft.startWrite();

    for (uint16_t py = 0; py < h; ++py)
    {
        int16_t sy = y + py;

        // Skip scanline if outside vertical bounds
        if (sy < 0 || sy >= TFT_HEIGHT)
            continue;

        uint16_t px = 0;
        while (px < w)
        {
            // Skip transparent pixels
            while (px < w)
            {
                uint16_t srcX = flipX ? (w - 1 - px) : px;
                uint16_t c = pgm_read_word(&bmp[py * w + srcX]);
                if (!isTransparent16(c))
                    break;
                ++px;
            }
            if (px >= w)
                break;

            // Find run of non-transparent pixels
            uint16_t start = px;
            while (px < w)
            {
                uint16_t srcX = flipX ? (w - 1 - px) : px;
                uint16_t c = pgm_read_word(&bmp[py * w + srcX]);
                if (isTransparent16(c))
                    break;
                ++px;
            }

            uint16_t len = px - start;
            int16_t runX0 = x + start;
            int16_t runX1 = x + start + len;

            // Clip run to horizontal screen bounds
            if (runX1 <= 0 || runX0 >= TFT_WIDTH)
                continue;

            int16_t clipStart = (runX0 > 0) ? runX0 : 0;
            int16_t clipEnd = (runX1 < TFT_WIDTH) ? runX1 : TFT_WIDTH;
            uint16_t clipLen = clipEnd - clipStart;

            // Calculate offset into sprite for clipped region
            uint16_t spriteOffset = clipStart - runX0;

            // Write clipped run in chunks
            uint16_t remaining = clipLen;
            uint16_t offset = 0;
            while (remaining)
            {
                uint16_t chunk = (remaining > 96) ? 96 : remaining;
                for (uint16_t i = 0; i < chunk; i++)
                {
                    uint16_t srcX = start + spriteOffset + offset + i;
                    if (flipX)
                        srcX = w - 1 - srcX;
                    buf[i] = pgm_read_word(&bmp[py * w + srcX]);
                }
                tft.setAddrWindow(clipStart + offset, sy, chunk, 1);
                tft.writePixels(buf, chunk);
                remaining -= chunk;
                offset += chunk;
            }
        }
    }
    tft.endWrite();
}

void blitPlayAreaFromCanvas() {
  if (gNoCanvas || !bgCanvas) return;
  
  const int16_t x = PLAY_AREA_X;
  const int16_t y = PLAY_AREA_Y;
  const int16_t w = PLAY_AREA_W;
  const int16_t h = PLAY_AREA_H;
  
  uint16_t* buf = bgCanvas->getBuffer();
  
  tft.startWrite();
  tft.setAddrWindow(x, y, w, h);
  for (int16_t row = 0; row < h; ++row) {
    tft.writePixels(buf + (y + row) * TFT_WIDTH + x, w);
  }
  tft.endWrite();
}

void drawDeathScreen()
{
    int16_t centerX = TFT_WIDTH / 2;
    int16_t centerY = TFT_HEIGHT / 2;

    tft.fillRect(0, 20, TFT_WIDTH, TFT_HEIGHT - 40, 0x0841);

    tft.setTextSize(2);
    tft.setTextColor(0xFFFF);
    const char *msg = "Your fish died!";
    int16_t msgW = strlen(msg) * 12;
    tft.setCursor(centerX - msgW / 2, centerY - 50);
    tft.print(msg);

    // Draw dead fish sprite (30x25)
    int16_t fishX = centerX - 15; // Center the 30px wide sprite
    int16_t fishY = centerY - 12; // Center the 25px tall sprite
    drawSpriteOptimized(nemodeadBitmap, 30, 25, fishX, fishY, false);

    tft.setTextSize(1);
    const char *hint = "Press OK to restart";
    int16_t hintW = strlen(hint) * 6;
    tft.setCursor(centerX - hintW / 2, centerY + 50);
    tft.print(hint);
}

void setBacklight(bool on) {
    digitalWrite(PIN_TFT_BL, on ? HIGH : LOW);
#ifdef DEBUG_GRAPHICS
    Serial.print("[GFX] Backlight ");
    Serial.println(on ? "ON" : "OFF");
#endif
}

// ---- Dirty Rectangle System Implementation ----
constexpr uint8_t MAX_DIRTY_RECTS = 32;
static DirtyRect dirtyRects[MAX_DIRTY_RECTS];
static uint8_t dirtyRectCount = 0;

void initDirtyRects() {
  dirtyRectCount = 0;
  for (uint8_t i = 0; i < MAX_DIRTY_RECTS; ++i) {
    dirtyRects[i].valid = false;
  }
}

void clearDirtyRects() {
  dirtyRectCount = 0;
}

void addDirtyRect(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  if (dirtyRectCount >= MAX_DIRTY_RECTS) {
#ifdef DEBUG_GRAPHICS
    Serial.println("[DIRTY] Warning: Max dirty rects reached, falling back to full play area");
#endif
    // Fallback: add full play area rect if we overflow
    if (dirtyRectCount == MAX_DIRTY_RECTS) {
      clearDirtyRects();
      dirtyRects[0].x = PLAY_AREA_X;
      dirtyRects[0].y = PLAY_AREA_Y;
      dirtyRects[0].w = PLAY_AREA_W;
      dirtyRects[0].h = PLAY_AREA_H;
      dirtyRects[0].valid = true;
      dirtyRectCount = 1;
    }
    return;
  }
  
  // Clip to play area
  int16_t x0 = max(x, PLAY_AREA_X);
  int16_t y0 = max(y, PLAY_AREA_Y);
  int16_t x1 = min((int16_t)(x + w), (int16_t)(PLAY_AREA_X + PLAY_AREA_W));
  int16_t y1 = min((int16_t)(y + h), (int16_t)(PLAY_AREA_Y + PLAY_AREA_H));
  
  if (x1 <= x0 || y1 <= y0) {
    return; // Fully outside play area
  }
  
  dirtyRects[dirtyRectCount].x = x0;
  dirtyRects[dirtyRectCount].y = y0;
  dirtyRects[dirtyRectCount].w = x1 - x0;
  dirtyRects[dirtyRectCount].h = y1 - y0;
  dirtyRects[dirtyRectCount].valid = true;
  dirtyRectCount++;
}

void addDirtyRectPair(int16_t x, int16_t y, uint16_t w, uint16_t h, 
                       float prevX, float prevY) {
  // Add small margin to ensure complete sprite erasure (for sub-pixel movement)
  const int16_t margin = 1;
  
  // Add current position with margin
  addDirtyRect(x - margin, y - margin, w + margin * 2, h + margin * 2);
  
  // Add previous position if valid
  if (prevX >= 0 && prevY >= 0) {
    addDirtyRect((int16_t)prevX - margin, (int16_t)prevY - margin, w + margin * 2, h + margin * 2);
  }
}

void mergeDirtyRects() {
  if (dirtyRectCount <= 1) return;
  
  // Simple merge: combine overlapping or adjacent rects
  bool merged = true;
  while (merged && dirtyRectCount > 1) {
    merged = false;
    
    for (uint8_t i = 0; i < dirtyRectCount; ++i) {
      if (!dirtyRects[i].valid) continue;
      
      for (uint8_t j = i + 1; j < dirtyRectCount; ++j) {
        if (!dirtyRects[j].valid) continue;
        
        DirtyRect& a = dirtyRects[i];
        DirtyRect& b = dirtyRects[j];
        
        // Check if rects overlap or are adjacent (within 2 pixels)
        int16_t ax1 = a.x + a.w;
        int16_t ay1 = a.y + a.h;
        int16_t bx1 = b.x + b.w;
        int16_t by1 = b.y + b.h;
        
        bool xOverlap = (a.x - 2 <= bx1) && (b.x - 2 <= ax1);
        bool yOverlap = (a.y - 2 <= by1) && (b.y - 2 <= ay1);
        
        if (xOverlap && yOverlap) {
          // Merge b into a
          int16_t newX = min(a.x, b.x);
          int16_t newY = min(a.y, b.y);
          int16_t newX1 = max(ax1, bx1);
          int16_t newY1 = max(ay1, by1);
          
          a.x = newX;
          a.y = newY;
          a.w = newX1 - newX;
          a.h = newY1 - newY;
          
          // Invalidate b and compact array
          b.valid = false;
          
          // Move last rect to position j to compact
          if (j < dirtyRectCount - 1) {
            dirtyRects[j] = dirtyRects[dirtyRectCount - 1];
          }
          dirtyRectCount--;
          
          merged = true;
          break; // Restart merge after modification
        }
      }
      
      if (merged) break;
    }
  }
  
#ifdef DEBUG_GRAPHICS
  Serial.print("[DIRTY] After merge: ");
  Serial.print(dirtyRectCount);
  Serial.println(" rects");
#endif
}

void processDirtyRects() {
  if (gNoCanvas || !bgCanvas) {
    // Fallback to full redraw
    blitPlayAreaFromCanvas();
    return;
  }
  
  if (dirtyRectCount == 0) {
#ifdef DEBUG_GRAPHICS
    Serial.println("[DIRTY] No dirty rects, skipping restore");
#endif
    return;
  }
  
  // Restore all dirty regions from background canvas
  uint16_t* buf = bgCanvas->getBuffer();
  
  tft.startWrite();
  for (uint8_t i = 0; i < dirtyRectCount; ++i) {
    if (!dirtyRects[i].valid) continue;
    
    DirtyRect& r = dirtyRects[i];
    
#ifdef DEBUG_GRAPHICS
    Serial.print("[DIRTY] Restoring rect ");
    Serial.print(i);
    Serial.print(": (");
    Serial.print(r.x);
    Serial.print(",");
    Serial.print(r.y);
    Serial.print(") ");
    Serial.print(r.w);
    Serial.print("x");
    Serial.println(r.h);
#endif
    
    tft.setAddrWindow(r.x, r.y, r.w, r.h);
    for (int16_t row = 0; row < r.h; ++row) {
      tft.writePixels(buf + (r.y + row) * TFT_WIDTH + r.x, r.w);
    }
  }
  tft.endWrite();
}
