# 🐠 OzeanLive - Ultra-Smooth Aquarium Tamagotchi

Ein hochoptimiertes digitales Aquarium/Tamagotchi-Spiel für Teensy 4.1 mit ST7789 Display (320x170px).

![Platform](https://img.shields.io/badge/platform-Teensy%204.1-blue)
![Framework](https://img.shields.io/badge/framework-Arduino-00979D)
![FPS](https://img.shields.io/badge/FPS-60-brightgreen)
![License](https://img.shields.io/badge/license-MIT-green)

## ✨ Features

### 🎬 Animation-System
- **5 Pet-Animationen**: Idle, Moving, Eating, Playing, Sleeping
- **60 FPS** stabiles Frame-Timing mit micros()-basierter Clock
- **State-Machine** mit smooth Transitions (0.2-0.3s)
- **Multi-Frame-Sprites** mit automatischem FlipX

### 🏊 Natürliche Bewegungen
- **Steering-Verhalten** mit Wegpunkt-Navigation
- **Easing-Funktionen** für sanftes Abbremsen
- **swimPhase** - geschwindigkeitsabhängiger Schwanzschlag (1.6 Hz)
- **Movement-Noise** - organische laterale Schwankungen

### 🎨 Partikel-Effekte
- **30 Partikel gleichzeitig** (Fixed Pool)
- **4 Typen**: Futter-Krumen 🍞, Herzen ❤️, ZZZ 💤, Schmutz 💨
- **Stipple-basiertes Fade-Out** für Alpha-Blending ohne Performance-Hit
- Auto-Spawn bei allen Aktionen

### 🧹 Dirt/Cleaning-System
- **20 Schmutz-Spots** erscheinen über Zeit (45-90s Intervall)
- **4 Arten**: Algen, Schmutz, Kalk, Mixed
- **Stipple-Pattern** für realistische Opacity (25%/50%/75%/100%)
- **Reinigen-Action** kostet Energie, entfernt Schmutz

### ⚡ Performance-Optimierungen
- **Scanline-Run-Batching** - 10-30x schnelleres Sprite-Drawing
- **Background-Canvas** - Environment nur 1x gerendert (109KB)
- **Dirty-Region-Restore** - Nur geänderte Pixel neu zeichnen
- **EMA-Smoothing** für jitter-freie deltaTime

## 🎮 Steuerung

| Taste | Funktion |
|-------|----------|
| **Links** | Menü-Auswahl nach links |
| **Rechts** | Menü-Auswahl nach rechts |
| **OK** | Aktion bestätigen |

### Menü-Aktionen
- **Futter** → Hunger -20, Energie +5 | Spawnt Futter-Krümel
- **Spielen** → Fun +20, Energie -5 | Spawnt Herzen
- **Schlafen** → Energie +20, Hunger +5 | Spawnt ZZZ
- **Putzen** → Dirt -30~50, Energie -10 | Spawnt Schmutz-Partikel

## 🛠️ Hardware

### Benötigte Komponenten
- **Teensy 4.1** (600 MHz ARM Cortex-M7)
- **ST7789 Display** 1.9" 320x170 (Landscape)
- **3 Buttons** (Pullup, aktiv LOW)

### Pin-Belegung (Teensy 4.1)
```cpp
// Display
PIN_TFT_CS   = 10
PIN_TFT_DC   = 9
PIN_TFT_RST  = 8

// Buttons
PIN_BTN_LEFT  = 2
PIN_BTN_OK    = 3
PIN_BTN_RIGHT = 4
```

## 📦 Installation

### PlatformIO (empfohlen)
```bash
git clone https://github.com/Mosei1984/OzeanLive.git
cd OzeanLive
pio run --target upload
```

### Arduino IDE
1. Repository klonen
2. Bibliotheken installieren:
   - Adafruit GFX Library @ ^1.11.9
   - Adafruit ST7735 and ST7789 Library @ ^1.10.3
3. `src/main.cpp` öffnen
4. Board: "Teensy 4.1" auswählen
5. Upload

## 📊 Performance-Metriken

| Metrik | Wert |
|--------|------|
| **Target FPS** | 60 |
| **Actual FPS** | ~60 stable |
| **Frame Time** | ~16.67ms |
| **Sprite Draw** | 0.5-2ms |
| **Partikel (30x)** | ~3ms |
| **Headroom** | 5-10ms |

## 🎨 Sprite-Assets

### Aktueller Status
Das Projekt verwendet **procedural generierte Platzhalter-Sprites** (einfarbige Formen mit Variationen).

### Eigene Sprites erstellen
1. **Format**: RGB565 16-bit, Transparenz = `0xF81F` (Pink)
2. **Größen**:
   - Clownfish: 30x25px (5 States × 3-6 Frames)
   - Partikel: 8x8px (4 Typen)
   - Dirt: 12x12px (4 Arten)
3. **Integration**: Arrays in `src/sprites/*.cpp` ersetzen

Siehe [ANIMATIONS.md](ANIMATIONS.md) für Details!

## 📁 Projekt-Struktur

```
teensy_aquarium_project/
├── src/
│   ├── main.cpp           # Hauptprogramm + Loop
│   ├── animator.cpp/h     # State-Machine für Animationen
│   ├── pet.cpp/h          # Pet-Logik + Bewegung
│   ├── particles.cpp/h    # Partikel-System
│   ├── dirt.cpp/h         # Schmutz/Cleaning-System
│   ├── gfx.cpp/h          # Display + Sprite-Drawing
│   ├── environment.cpp/h  # Hintergrund/Deko
│   ├── bubbles.cpp/h      # Blasen-Animation
│   ├── menu.cpp/h         # Button-Input + Menü
│   └── sprites/           # Alle Sprite-Assets
├── platformio.ini         # PlatformIO-Konfiguration
├── ANIMATIONS.md          # Detaillierte Animations-Doku
└── README.md              # Diese Datei
```

## 🚀 Erweiterungsmöglichkeiten

### Geplante Features
- [ ] Mehr Pet-States (CLEANING, EXCITED, SAD)
- [ ] Sound-Effekte via Teensy Audio Library
- [ ] Mehrere Pets gleichzeitig
- [ ] Day/Night-Cycle mit Beleuchtungs-Wechsel
- [ ] Touch-Screen-Support
- [ ] Speichern von Stats (EEPROM/SD-Card)

### Advanced Optimizations
- [ ] RLE-Kompression für Sprites (30-60% Ersparnis)
- [ ] Palette-8 CLUT (8-bit Sprites + Lookup-Table)
- [ ] DMA SPI für asynchrones Display-Update

## 🧠 Technische Highlights

### Frame-Timing
```cpp
// micros()-basierte Frame-Clock
const unsigned long targetFrameUs = 16667; // ~60 FPS
float dtSec = constrain((micros() - lastFrameUs) / 1e6f, 0.004f, 0.05f);
dtSecSmooth = dtSecSmooth * 0.8f + dtSec * 0.2f; // EMA
```

### Scanline-Run-Batching
```cpp
// Nur opaque Pixel-Runs zeichnen → 10-30x schneller
static uint16_t buf[96];
tft.startWrite();
for (each scanline) {
  for (each opaque run) {
    tft.setAddrWindow(x, y, len, 1);
    tft.writePixels(buf, len);
  }
}
tft.endWrite();
```

### Dirty-Region-Restore
```cpp
// Nur geänderte Bereiche vom Background wiederherstellen
void restoreRegion(int16_t x, int16_t y, int16_t w, int16_t h) {
  uint16_t* buf = bgCanvas->getBuffer();
  for (int16_t r = 0; r < h; r++) {
    tft.writePixels(buf + (y+r)*TFT_WIDTH + x, w);
  }
}
```

## 📝 Lizenz

MIT License - siehe [LICENSE](LICENSE) für Details.

## 🙏 Credits

- **Oracle AI** - Architektur-Beratung für Timing & Performance
- **Adafruit** - GFX & ST7789 Libraries
- **PlatformIO** - Build-System

## 📧 Kontakt

Fragen, Feedback, Pull Requests? Gerne!

---

**Made with ❤️ and smooth animations on Teensy 4.1**
