# Sprite Organization - Final Structure

## ✅ Organized Sprite Files

### Individual Creatures
- `anemone_green.h` - 50x50 green anemone sprite (inline bitmap)
- `bee_shrimp.h` - 14x6 bee shrimp sprite (inline bitmap)
- `clownfish.h` + `clownfish.cpp` - Clownfish animation definitions
- `clownfish_frames.h` - All clownfish animation frames

### Collections
- **`corals.h`** - ALL coral sprites in one file:
  - Brain Coral (24x16)
  - Fan Coral (28x24)
  - Staghorn Coral (32x28)
  - Tube Coral (20x24)
  - Extra Coral 1 (24x20)

### Environment
- `kelp.h` - 16x32 kelp sprite (inline bitmap)
- `stone.h` + `stone.cpp` - Stone decoration

### Effects
- `particles.h` - All particle sprites (crumb, heart, zzz, dirt) (inline bitmaps)
- `dirt_spots.h` + `dirt_spots.cpp` - Dirt spot variations
- `small_bubble.h` + `small_bubble.cpp` - Small bubbles
- `medium_bubble.h` + `medium_bubble.cpp` - Medium bubbles

### Other Creatures
- `seahorse_sprite.h` + `seahorse_sprite.cpp` - Seahorse sprite
- `seahorse.h` - Seahorse metadata

## File Pattern

All sprite header files follow this pattern:

```cpp
#pragma once
#include <Arduino.h>

// [Sprite Name] - [width]x[height]
const uint16_t [NAME]_WIDTH = [width];
const uint16_t [NAME]_HEIGHT = [height];

// Option 1: Inline bitmap
const uint16_t [name]Bitmap[] PROGMEM = {
  // ... data ...
};

// Option 2: External bitmap (defined in .cpp)
extern const uint16_t [name]Bitmap[] PROGMEM;
```

## Collection Pattern (corals.h)

Multiple sprites in one file:

```cpp
#pragma once
#include <Arduino.h>

// Sprite 1 - WxH
const uint16_t SPRITE1_WIDTH = W;
const uint16_t SPRITE1_HEIGHT = H;
const uint16_t sprite1Bitmap[] PROGMEM = { ... };

// Sprite 2 - WxH
const uint16_t SPRITE2_WIDTH = W;
const uint16_t SPRITE2_HEIGHT = H;
const uint16_t sprite2Bitmap[] PROGMEM = { ... };
```

## Why This Organization?

1. **Corals grouped together** - They're decorative elements, often used together in environment.cpp
2. **Individual files for creatures** - Each creature has its own behavior/animation
3. **Effects grouped** - Particles are small and used together
4. **Inline bitmaps where appropriate** - Smaller sprites keep data in header
5. **Separate .cpp for larger sprites** - Reduces header file size

## Usage Example

```cpp
// In environment.cpp
#include "sprites/corals.h"  // Gets ALL corals

// Draw brain coral
drawSprite(brain_coralBitmap, BRAIN_CORAL_WIDTH, BRAIN_CORAL_HEIGHT, x, y);

// Draw fan coral
drawSprite(fan_coralBitmap, FAN_CORAL_WIDTH, FAN_CORAL_HEIGHT, x, y);
```

## Transparency

All sprites use `0xF81F` (magenta/pink) as transparency marker, defined in `sprite_common.h` as `TRANSPARENT_COLOR`.
