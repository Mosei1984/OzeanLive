# 🎨 Sprite Conversion Guide for OzeanLive

## Generated Sprites Location
All sprites are in: `attached_assets/generated_images/`

## Required Format
- **Color Format**: RGB565 (16-bit)
- **Transparency Color**: `0xF81F` (Magenta/Pink)
- **Output**: C array in PROGMEM for Arduino/Teensy

## Sprite Dimensions

### Clownfish Animations (30x25 pixels)
- `Clownfish_idle_sprite_frame_1_f0a4e563.png`
- `Clownfish_idle_sprite_frame_2_52dfe016.png`
- `Clownfish_moving_sprite_frame_1_2ecba446.png`
- `Clownfish_eating_sprite_frame_a18c18b7.png`
- `Clownfish_playing_sprite_frame_c97dedfd.png`
- `Clownfish_sleeping_sprite_frame_ae843813.png`

### Environment Sprites
- **Seahorse**: 16x16 pixels - `Seahorse_decoration_sprite_16x16_0d8b935d.png`
- **Corals**: 16x16 pixels each - `Coral_decoration_sprites_collection_23051b70.png`
- **Kelp**: 16x32 pixels - `Kelp_seaweed_sprite_16x32_8c59cf4b.png`

### Particle Sprites (8x8 pixels)
- `Particle_effects_sprites_collection_846dea93.png`

## Conversion Methods

### Method 1: Using Python Script (Recommended)

Create a Python script to convert PNG to RGB565 C arrays:

```python
from PIL import Image
import sys

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565"""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def png_to_rgb565_array(png_path, width, height, var_name):
    """Convert PNG to RGB565 C array"""
    img = Image.open(png_path).convert('RGBA')
    img = img.resize((width, height), Image.NEAREST)  # Pixel-perfect scaling
    
    print(f"const uint16_t {var_name}[] PROGMEM = {{")
    
    for y in range(height):
        row = []
        for x in range(width):
            r, g, b, a = img.getpixel((x, y))
            
            # If transparent or mostly transparent, use transparency color
            if a < 128:
                row.append("0xF81F")
            else:
                rgb565 = rgb888_to_rgb565(r, g, b)
                row.append(f"0x{rgb565:04X}")
        
        print("  " + ", ".join(row) + ("," if y < height - 1 else ""))
    
    print("};")

# Usage example:
# png_to_rgb565_array("clownfish_idle1.png", 30, 25, "clownfish_idle_frame1")
```

### Method 2: Using Online Tools

1. **Image Resize**:
   - Use https://www.birme.net/ or GIMP
   - Set to "Nearest Neighbor" for pixel-perfect scaling
   - Resize to exact dimensions

2. **RGB565 Conversion**:
   - Use http://www.rinkydinkelectronics.com/t_imageconverter565.php
   - Or https://github.com/watterott/Arduino-Libs/tree/master/BMP2ARRAY

3. **Manual Editing**:
   - Replace transparent areas with `0xF81F`
   - Format as C array

### Method 3: Using Aseprite (Best for Pixel Art)

1. Open sprite in Aseprite
2. Scale to target dimensions (Sprite → Sprite Size → Algorithm: Nearest Neighbor)
3. Export as PNG
4. Use Python script or online converter

## Integration into Code

### Example: Clownfish Idle Animation

Replace in `src/sprites/clownfish.cpp`:

```cpp
// Frame 1 - IDLE
const uint16_t clownfish_idle_frame1[] PROGMEM = {
  // ... your converted RGB565 data here
};

// Frame 2 - IDLE  
const uint16_t clownfish_idle_frame2[] PROGMEM = {
  // ... your converted RGB565 data here
};

// Update AnimationClip
const uint16_t* IDLE_FRAMES[] = {
  clownfish_idle_frame1,
  clownfish_idle_frame2,
  clownfish_idle_frame1,
  clownfish_idle_frame2
};

const AnimationClip CLIP_IDLE = {
  IDLE_FRAMES,
  4,      // frameCount
  7.0f,   // fps
  true    // loop
};
```

### Example: Particles

Replace in `src/sprites/particles.cpp`:

```cpp
const uint16_t PARTICLE_HEART[64] PROGMEM = {
  // ... 8x8 = 64 pixels in RGB565 format
};
```

## Testing Tips

1. **Start Small**: Test one sprite first (e.g., idle frame 1)
2. **Check Transparency**: Ensure `0xF81F` shows as transparent
3. **Verify Dimensions**: Count array elements (width × height)
4. **Visual Test**: Upload to Teensy and see it on display
5. **Iterate**: Adjust colors or details based on how it looks

## Color Calibration

ST7789 displays may show colors differently than your screen:
- Test on actual hardware
- Adjust RGB values if needed
- Consider display brightness settings

## File Structure

After conversion, your sprite files should look like:

```
src/sprites/
├── clownfish.cpp       # Multiple frame arrays + AnimationClips
├── clownfish.h         # Frame declarations
├── particles.cpp       # 4 particle types (8x8 each)
├── particles.h         # Particle declarations
├── seahorse_sprite.cpp # Updated seahorse
├── [coral_name].cpp    # Individual coral sprites
└── kelp.cpp           # Vertical kelp sprite
```

## Quick Python Converter

Save this as `convert_sprite.py`:

```python
#!/usr/bin/env python3
from PIL import Image
import sys

if len(sys.argv) != 5:
    print("Usage: python convert_sprite.py <input.png> <width> <height> <var_name>")
    sys.exit(1)

png_path = sys.argv[1]
width = int(sys.argv[2])
height = int(sys.argv[3])
var_name = sys.argv[4]

img = Image.open(png_path).convert('RGBA')
img = img.resize((width, height), Image.NEAREST)

print(f"const uint16_t {var_name}[] PROGMEM = {{")

for y in range(height):
    row = []
    for x in range(width):
        r, g, b, a = img.getpixel((x, y))
        if a < 128:
            row.append("0xF81F")
        else:
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            row.append(f"0x{rgb565:04X}")
    print("  " + ", ".join(row) + ("," if y < height - 1 else ""))

print("};")
```

Run with:
```bash
python convert_sprite.py clownfish_idle1.png 30 25 clownfish_idle1 > output.txt
```

## Need More Frames?

The current sprites are base frames. For full animation:
- **IDLE**: 4 frames (gentle floating)
- **MOVING**: 6 frames (active swimming)
- **EATING**: 5 frames (bite animation)
- **PLAYING**: 6 frames (excited movement)
- **SLEEPING**: 3 frames (breathing slowly)

You can duplicate/modify the generated sprites or request more variations!

---

Happy sprite converting! 🐠✨
