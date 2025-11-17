#!/usr/bin/env python3
"""
PNG to RGB565 C Array Converter for Teensy Aquarium Project
Converts PNG sprites with transparency to RGB565 arrays
"""

from PIL import Image
import os
import glob

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565"""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

def png_to_rgb565_array(png_path, var_name, target_size=None):
    """Convert PNG to RGB565 C array"""
    img = Image.open(png_path).convert('RGBA')
    
    # Resize if target size specified
    if target_size:
        img = img.resize(target_size, Image.Resampling.NEAREST)
    
    width, height = img.size
    
    pixels = []
    for y in range(height):
        for x in range(width):
            r, g, b, a = img.getpixel((x, y))
            
            # Transparent pixels -> 0xF81F (pink marker)
            # Also treat pure white as transparent if alpha < 255
            if a < 128 or (r > 250 and g > 250 and b > 250 and a < 255):
                pixels.append(0xF81F)
            else:
                rgb565 = rgb888_to_rgb565(r, g, b)
                pixels.append(rgb565)
    
    # Generate C array
    output = f"// {os.path.basename(png_path)} - {width}x{height}\n"
    output += f"const uint16_t {var_name}[] PROGMEM = {{\n"
    
    for i in range(0, len(pixels), 16):
        line = pixels[i:i+16]
        output += "  " + ", ".join(f"0x{p:04X}" for p in line)
        if i + 16 < len(pixels):
            output += ","
        output += "\n"
    
    output += "};\n"
    
    return output, width, height

def main():
    png_dir = "src/sprites/pngs"
    output_dir = "src/sprites/generated"
    
    # Create output directory
    os.makedirs(output_dir, exist_ok=True)
    
    print("Converting PNG sprites to RGB565 arrays...\n")
    
    # Clownfish frames
    clownfish_files = {
        "Clownfish_idle_sprite_frame_1_f0a4e563.png": ("clownfish_idle_f0", "IDLE Frame 0"),
        "Clownfish_idle_sprite_frame_2_52dfe016.png": ("clownfish_idle_f1", "IDLE Frame 1"),
        "Clownfish_moving_sprite_frame_1_2ecba446.png": ("clownfish_moving_f0", "MOVING Frame 0"),
        "Clownfish_eating_sprite_frame_a18c18b7.png": ("clownfish_eating_f0", "EATING Frame 0"),
        "Clownfish_playing_sprite_frame_c97dedfd.png": ("clownfish_playing_f0", "PLAYING Frame 0"),
        "Clownfish_sleeping_sprite_frame_ae843813.png": ("clownfish_sleeping_f0", "SLEEPING Frame 0"),
    }
    
    clownfish_output = "#pragma once\n#include <Arduino.h>\n\n"
    clownfish_output += "// =============================================================================\n"
    clownfish_output += "// CLOWNFISH SPRITES - Generated from PNG\n"
    clownfish_output += "// =============================================================================\n\n"
    
    clownfish_width = 0
    clownfish_height = 0
    
    for filename, (var_name, desc) in clownfish_files.items():
        png_path = os.path.join(png_dir, filename)
        if os.path.exists(png_path):
            print(f"[+] Converting {filename} -> {var_name}")
            # Resize clownfish to 30x25
            array, w, h = png_to_rgb565_array(png_path, var_name, target_size=(30, 25))
            clownfish_output += f"// {desc}\n{array}\n"
            if clownfish_width == 0:
                clownfish_width = w
                clownfish_height = h
    
    # Write clownfish generated file
    with open(os.path.join(output_dir, "clownfish_frames.h"), "w") as f:
        f.write(clownfish_output)
    
    print(f"\n[OK] Clownfish sprites: {clownfish_width}x{clownfish_height}")
    print(f"   Output: {output_dir}/clownfish_frames.h")
    
    # Seahorse
    seahorse_file = "Seahorse_decoration_sprite_16x16_0d8b935d.png"
    seahorse_path = os.path.join(png_dir, seahorse_file)
    if os.path.exists(seahorse_path):
        print(f"\n[+] Converting {seahorse_file}")
        # Resize seahorse to 16x16 as specified in filename
        array, w, h = png_to_rgb565_array(seahorse_path, "seahorseBitmap", target_size=(16, 16))
        with open(os.path.join(output_dir, "seahorse.h"), "w") as f:
            f.write("#pragma once\n#include <Arduino.h>\n\n")
            f.write(array)
        print(f"[OK] Seahorse: {w}x{h}")
    
    print("\n[DONE] Conversion complete!")
    print("\nNext steps:")
    print("1. Review generated files in src/sprites/generated/")
    print("2. Integrate into clownfish.cpp and seahorse_sprite.cpp")

if __name__ == "__main__":
    main()
