# Sprite Header File Standards

All sprite header files in the `src/sprites/` folder follow these conventions:

## File Structure

```cpp
#pragma once
#include <Arduino.h>

// [Sprite Name] - [width]x[height]
const uint16_t [SPRITE_NAME]_WIDTH  = [width];
const uint16_t [SPRITE_NAME]_HEIGHT = [height];

// If bitmap is defined in .cpp file:
extern const uint16_t [sprite_name]Bitmap[] PROGMEM;

// If bitmap is defined inline:
const uint16_t [sprite_name]Bitmap[] PROGMEM = {
  // ... data ...
};
```

## Naming Conventions

1. **Constants**: Use `UPPERCASE_WITH_UNDERSCORES` format
   - Width: `[SPRITE_NAME]_WIDTH`
   - Height: `[SPRITE_NAME]_HEIGHT`
   - Example: `BEE_SHRIMP_WIDTH`, `ANEMONE_GREEN_HEIGHT`

2. **Bitmap Arrays**: Use `camelCase` with `Bitmap` suffix
   - Format: `[sprite_name]Bitmap`
   - Example: `bee_shrimpBitmap`, `anemone_greenBitmap`, `clownfishBitmap`

3. **File Names**: Use `snake_case` matching the sprite name
   - Example: `bee_shrimp.h`, `anemone_green.h`, `seahorse_sprite.h`

## Type Standards

- All dimension constants use `const uint16_t` (not `constexpr`, not `int16_t`)
- All bitmap arrays are marked `PROGMEM` and use `const uint16_t`
- Transparency color is defined in `src/sprite_common.h` as `TRANSPARENT_COLOR = 0xF81F`

## Header Comments

Each sprite file includes a simple comment indicating the sprite name and dimensions:

```cpp
// [Sprite Name] - [width]x[height]
```

Example: `// Bee Shrimp - 14x6`

## Include Policy

- Always include `<Arduino.h>` first
- Do NOT include `"../sprite_common.h"` unless actually needed
- Only include additional headers if they provide required functionality
  - Example: `clownfish.h` includes `"../animator.h"` for animation support
  - Example: `particles.h` includes `"generated/particles.h"` for generated data

## Currently Standardized Sprites

### Creatures
- `bee_shrimp.h` - 14x6 - Red/white banded shrimp
- `clownfish.h` - 30x25 - Animated fish with multiple clips
- `seahorse_sprite.h` - 16x16 - Seahorse creature

### Environment
- `anemone_green.h` - 50x50 - Green anemone with flowing tentacles
- `kelp.h` - 16x32 - Kelp plant
- `brain_coral.h` - 24x16 - Brain coral decoration
- `fan_coral.h` - 28x24 - Fan coral decoration
- `staghorn_coral.h` - 32x28 - Staghorn coral
- `tube_coral.h` - 20x24 - Tube coral
- `stone.h` - 8x8 - Small stone decoration

### Effects
- `particles.h` - Various particle effects (crumb, heart, zzz, dirt)
- `small_bubble.h` - 6x6 - Small bubble
- `medium_bubble.h` - 16x16 - Medium bubble
- `dirt_spots.h` - 8x8 - Dirt spots with multiple variants

## Migration Notes

### Changed Constants
- `SHRIMP_WIDTH/HEIGHT` → `BEE_SHRIMP_WIDTH/HEIGHT`
- `shrimpBitmap` → `bee_shrimpBitmap`
- `ANEMONE_WIDTH/HEIGHT` → `ANEMONE_GREEN_WIDTH/HEIGHT`
- `anemoneBitmap` → `anemone_greenBitmap`

### Updated Files
All references in the following files have been updated:
- `src/shrimp.cpp` - Uses new `BEE_SHRIMP_*` constants
- `src/environment.cpp` - Uses new `ANEMONE_GREEN_*` constants
