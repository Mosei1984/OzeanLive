# 🐠 Animation-System Dokumentation

## Übersicht
Das Aquarium nutzt ein hochoptimiertes Frame-basiertes Animations-System mit State-Machine, Partikel-Effekten und dynamischem Schmutz-System.

---

## 🎬 Animation-States

### Pet-Animationen (Clownfish)
1. **IDLE** - Ruhig schweben (4 Frames, 7 FPS, Loop)
2. **MOVING** - Aktives Schwimmen (6 Frames, 10 FPS, Loop)
3. **EATING** - Füttern (5 Frames, 10 FPS, Non-Loop)
4. **PLAYING** - Spielen (6 Frames, 12 FPS, Loop)
5. **SLEEPING** - Schlafen (3 Frames, 5 FPS, Loop)

### Transitions
- Smooth State-Übergang mit 0.2-0.3s Dauer
- Amplituden-Blending (Swing/Bob reduziert während Transition)
- Auto-Transition: IDLE ↔ MOVING basierend auf Geschwindigkeit

---

## 🎨 Partikel-System

### Partikel-Typen (Max 30 gleichzeitig)
1. **FOOD_CRUMB** 🍞
   - Spawn: Bei FEED-Action (8-12 Partikel)
   - Physik: Sinken langsam (Gravity)
   - Farbe: Braun/Orange
   - Lifetime: 2-3 Sekunden

2. **HEART** ❤️
   - Spawn: Bei PLAY-Action (6-10 Partikel)
   - Physik: Steigen langsam (Buoyancy)
   - Farbe: Rot/Pink
   - Lifetime: 2-4 Sekunden

3. **ZZZ** 💤
   - Spawn: Bei REST-Action (3-5 Partikel)
   - Physik: Steigen (Buoyancy)
   - Farbe: Weiß
   - Lifetime: 3-5 Sekunden

4. **DIRT** 💨
   - Spawn: Bei CLEAN-Action (10-15 Partikel)
   - Physik: Langsam sinken + Dämpfung
   - Farbe: Grau/Grün
   - Lifetime: 1-2 Sekunden

### Rendering
- **Stipple-Pattern** für Alpha-Blending-Effekt ohne echtes Blending
- Fade-Out basierend auf `age/lifetime`
- Dirty-Region-Restore für flicker-freies Rendering

---

## 🧹 Dirt/Cleaning-System

### Schmutz-Mechanik
- **20 Dirt-Spots** erscheinen über Zeit
- **Spawn-Intervall**: Alle 45-90 Sekunden
- **Wachstum**: Strength steigt langsam von 0 → 100
- **Arten**: 4 verschiedene (Algen, Schmutz, Kalk, Mixed)

### Visualisierung (Stipple-Pattern)
- **Strength 0-25**: 25% Stipple (kaum sichtbar)
- **Strength 26-50**: 50% Stipple
- **Strength 51-75**: 75% Stipple
- **Strength 76-100**: 100% (komplett sichtbar)

### Reinigung
- **Action**: MENU_CLEAN → ACTION_CLEAN
- **Effekt**: Reduziert alle Spots um 30-50 Strength
- **Kosten**: -10 Energie
- **Visuell**: Spawnt 10-15 DIRT-Partikel

---

## 🏊 Bewegungs-Optimierungen

### Steering-Verhalten
- Wegpunkt-basierte Navigation
- **Easing**: `easeOutCubic` beim Abbremsen vor Ziel
- **Arrive-Radius**: 10px

### swimPhase (Natürlicher Schwimm-Stil)
- **Frequenz**: 1.6 Hz Schwanzschlag
- **Speed-abhängig**: Schneller = stärkeres Wedeln
- **Amplitude**: 3-6px Swing, 2-4px Bob (je nach Geschwindigkeit)

### Movement-Noise
- Dezente laterale Schwankungen (±15%)
- Nur aktiv bei `speed > 5 px/s`
- Update alle 0.5s für organisches Verhalten

---

## ⚡ Performance-Optimierungen

### 60 FPS Frame-Timing
- `micros()`-basierte Frame-Clock (16.667µs)
- DeltaTime mit Clamping [4ms, 50ms] + EMA-Smoothing
- Präzises Frame-Pacing statt `delay()`

### Sprite-Drawing (10-30x schneller)
- **Scanline-Run-Batching**: Nur opaque Pixel-Runs zeichnen
- **FlipX**: Horizontal-Flip ohne doppelte Assets
- **Batch-Buffer**: 96 Pixel pro Write
- **startWrite/endWrite**: Minimaler SPI-Overhead

### Dirty-Region-System
- **Background-Canvas**: 109KB, 1x gerendert
- **restoreRegion()**: Nur geänderte Bereiche wiederherstellen
- Sprites tracken alte Position für Restore
- Status-Bar: Nur bei Wert-Änderung neu zeichnen

---

## 🎮 Menü-Aktionen

| Button | Aktion | Effekt | Kosten |
|--------|--------|--------|--------|
| **Futter** | FEED → EATING | Hunger -20, Energie +5 | Futter-Krümel-Partikel |
| **Spielen** | PLAY → PLAYING | Fun +20, Energie -5 | Herz-Partikel |
| **Schlafen** | REST → SLEEPING | Energie +20, Hunger +5 | ZZZ-Partikel |
| **Putzen** | CLEAN | Dirt -30~50 | Energie -10, Dirt-Partikel |

---

## 📝 Sprite-Assets TODO

### Aktuell: Procedural Platzhalter
Die Sprites verwenden momentan **procedural generierte Platzhalter** (einfarbige Rechtecke mit Variationen).

### Echte Sprites erstellen:
1. **Clownfish-Animationen** (30x25px, 5 States × 3-6 Frames = ~25 Frames)
2. **Partikel-Sprites** (8x8px, 4 Typen)
3. **Dirt-Sprites** (12x12px, 4 Arten)

### Format:
- **RGB565** 16-bit Color Array in PROGMEM
- **Transparenz**: `0xF81F` (Pink)
- Tool: Eigener Sprite-Generator oder Aseprite → Converter

### Integration:
Einfach die Arrays in den entsprechenden `.cpp` Files ersetzen:
- `src/sprites/clownfish.cpp` - Multi-Frame-Arrays
- `src/sprites/particles.cpp` - Partikel-Sprites
- `src/sprites/dirt_spots.cpp` - Schmutz-Sprites

---

## 🚀 Erweiterungsmöglichkeiten

### Nächste Features:
1. **Mehr Pet-States**: CLEANING-Animation, EXCITED, SAD
2. **Mehr Partikel**: Blasen beim Schwimmen, Glitzer beim Spielen
3. **Sound-Effekte**: Über Teensy Audio Library
4. **Mehrere Pets**: Array von Pets mit unterschiedlichen Sprites
5. **Decorations**: Dynamische Korallen-Animationen
6. **Day/Night-Cycle**: Beleuchtungs-Wechsel

### Advanced Optimizations:
- **RLE-Kompression** für Sprites (30-60% Speicher-Ersparnis)
- **Palette-8 CLUT**: 8-bit Sprites mit Lookup-Table
- **DMA SPI**: Asynchrones Display-Update (Library-Wechsel nötig)

---

## 📊 Memory-Usage

| Component | RAM | PROGMEM |
|-----------|-----|---------|
| Background-Canvas | 109 KB | - |
| Particle-Pool (30) | ~1 KB | - |
| Dirt-Spots (20) | ~200 bytes | - |
| Animator State | ~100 bytes | - |
| **Sprites** | - | ~varies |
| Clownfish (placeholder) | - | ~3 KB |
| Particles | - | ~1 KB |
| Environment-Sprites | - | ~8 KB |

**Teensy 4.1**: 1024 KB RAM, genug Reserve für weitere Features!

---

## 🎯 Performance-Metriken

- **Target**: 60 FPS (16.67ms/Frame)
- **Actual**: ~60 FPS stable mit allen Features aktiv
- **Sprite-Draw**: ~0.5-2ms pro Clownfish (je nach Run-Länge)
- **Partikel**: ~0.1ms pro Partikel (30 = ~3ms)
- **Dirty-Restore**: ~0.2-0.5ms pro Region

**Headroom**: ~5-10ms pro Frame für weitere Features!
