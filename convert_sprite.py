#!/usr/bin/env python3

from PIL import Image
import sys
import os

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565 format"""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def png_to_rgb565_array(png_path, width, height, var_name, output_file=None):
    """Convert PNG to RGB565 C array for Arduino/Teensy"""
    
    if not os.path.exists(png_path):
        print(f"Error: File '{png_path}' not found!")
        sys.exit(1)
    
    img = Image.open(png_path).convert('RGBA')
    img = img.resize((width, height), Image.NEAREST)
    
    output = []
    output.append(f"// Generated from: {os.path.basename(png_path)}")
    output.append(f"// Dimensions: {width}x{height} pixels")
    output.append(f"const uint16_t {var_name}[] PROGMEM = {{")
    
    for y in range(height):
        row = []
        for x in range(width):
            r, g, b, a = img.getpixel((x, y))
            
            if a < 128:
                row.append("0xF81F")
            else:
                rgb565 = rgb888_to_rgb565(r, g, b)
                row.append(f"0x{rgb565:04X}")
        
        line = "  " + ", ".join(row)
        if y < height - 1:
            line += ","
        output.append(line)
    
    output.append("};")
    
    result = "\n".join(output)
    
    if output_file:
        with open(output_file, 'w') as f:
            f.write(result)
        print(f"✓ Converted successfully to: {output_file}")
    else:
        print(result)
    
    return result

def main():
    if len(sys.argv) < 5:
        print("Usage: python convert_sprite.py <input.png> <width> <height> <var_name> [output.txt]")
        print("\nExample:")
        print("  python convert_sprite.py clownfish.png 30 25 clownfish_idle1")
        print("  python convert_sprite.py clownfish.png 30 25 clownfish_idle1 output.txt")
        print("\nSprite Dimensions:")
        print("  - Clownfish: 30x25")
        print("  - Corals: 16x16")
        print("  - Seahorse: 16x16")
        print("  - Kelp: 16x32")
        print("  - Particles: 8x8")
        sys.exit(1)
    
    png_path = sys.argv[1]
    width = int(sys.argv[2])
    height = int(sys.argv[3])
    var_name = sys.argv[4]
    output_file = sys.argv[5] if len(sys.argv) > 5 else None
    
    png_to_rgb565_array(png_path, width, height, var_name, output_file)

if __name__ == "__main__":
    main()
