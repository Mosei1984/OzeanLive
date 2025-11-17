#!/usr/bin/env python3
"""
Complete PNG to RGB565 Converter for all remaining sprites
"""

from PIL import Image
import os

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565"""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

def png_to_rgb565_array(png_path, var_name, target_size=None, crop_box=None):
    """Convert PNG to RGB565 C array"""
    img = Image.open(png_path).convert('RGBA')
    
    # Crop if specified (for sprite sheets)
    if crop_box:
        img = img.crop(crop_box)
    
    # Resize if target size specified
    if target_size:
        img = img.resize(target_size, Image.Resampling.NEAREST)
    
    width, height = img.size
    
    pixels = []
    for y in range(height):
        for x in range(width):
            r, g, b, a = img.getpixel((x, y))
            
            # Transparent pixels -> 0xF81F (pink marker)
            if a < 128 or (r > 250 and g > 250 and b > 250 and a < 255):
                pixels.append(0xF81F)
            else:
                rgb565 = rgb888_to_rgb565(r, g, b)
                pixels.append(rgb565)
    
    # Generate C array
    output = f"const uint16_t {var_name}[] PROGMEM = {{\n"
    
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
    
    os.makedirs(output_dir, exist_ok=True)
    
    print("Converting remaining sprites...\n")
    
    # =========================================================================
    # PARTICLES (8x8 each, crop from collection)
    # =========================================================================
    print("=== PARTICLES ===")
    particle_file = "Particle_effects_sprites_collection_846dea93.png"
    particle_path = os.path.join(png_dir, particle_file)
    
    if os.path.exists(particle_path):
        img = Image.open(particle_path)
        # Assuming grid layout: top-left crumb, top-right heart, middle ZZZ, bottom-left/right dirt
        
        particles_output = "#pragma once\n#include <Arduino.h>\n\n"
        particles_output += "// Particle sprites - 8x8\n\n"
        
        # Food Crumb (top-left quadrant)
        print("[+] Extracting Food Crumb")
        array, w, h = png_to_rgb565_array(particle_path, "particle_crumb", 
                                          target_size=(8, 8), crop_box=(0, 0, 512, 512))
        particles_output += "// Food Crumb\n" + array + "\n"
        
        # Heart (top-right quadrant)
        print("[+] Extracting Heart")
        array, w, h = png_to_rgb565_array(particle_path, "particle_heart",
                                          target_size=(8, 8), crop_box=(512, 0, 1024, 512))
        particles_output += "// Heart\n" + array + "\n"
        
        # ZZZ (middle)
        print("[+] Extracting ZZZ")
        array, w, h = png_to_rgb565_array(particle_path, "particle_zzz",
                                          target_size=(12, 12), crop_box=(256, 384, 768, 896))
        particles_output += "// ZZZ (12x12 for visibility)\n" + array + "\n"
        
        # Dirt/Algae (bottom-left)
        print("[+] Extracting Dirt/Algae")
        array, w, h = png_to_rgb565_array(particle_path, "particle_dirt",
                                          target_size=(8, 8), crop_box=(0, 512, 512, 1024))
        particles_output += "// Dirt/Algae\n" + array + "\n"
        
        with open(os.path.join(output_dir, "particles.h"), "w") as f:
            f.write(particles_output)
        
        print("[OK] Particles converted\n")
    
    # =========================================================================
    # KELP (16x32)
    # =========================================================================
    print("=== KELP ===")
    kelp_file = "Kelp_seaweed_sprite_16x32_8c59cf4b.png"
    kelp_path = os.path.join(png_dir, kelp_file)
    
    if os.path.exists(kelp_path):
        print("[+] Converting Kelp")
        array, w, h = png_to_rgb565_array(kelp_path, "kelpBitmap", target_size=(16, 32))
        
        with open(os.path.join(output_dir, "kelp.h"), "w") as f:
            f.write("#pragma once\n#include <Arduino.h>\n\n")
            f.write("// Kelp/Seaweed - 16x32\n")
            f.write(array)
        
        print(f"[OK] Kelp: {w}x{h}\n")
    
    # =========================================================================
    # CORALS (varying sizes, crop from collection)
    # =========================================================================
    print("=== CORALS ===")
    coral_file = "Coral_decoration_sprites_collection_23051b70.png"
    coral_path = os.path.join(png_dir, coral_file)
    
    if os.path.exists(coral_path):
        img = Image.open(coral_path)
        # 3x3 grid layout
        
        corals_output = "#pragma once\n#include <Arduino.h>\n\n"
        corals_output += "// Coral sprites from collection\n\n"
        
        # Brain Coral (top-left)
        print("[+] Extracting Brain Coral")
        array, w, h = png_to_rgb565_array(coral_path, "brain_coralBitmap",
                                          target_size=(24, 16), crop_box=(0, 0, 340, 340))
        corals_output += "// Brain Coral - 24x16\n" + array + "\n"
        
        # Fan Coral (top-middle)
        print("[+] Extracting Fan Coral")
        array, w, h = png_to_rgb565_array(coral_path, "fan_coralBitmap",
                                          target_size=(28, 24), crop_box=(340, 0, 680, 340))
        corals_output += "// Fan Coral - 28x24\n" + array + "\n"
        
        # Staghorn Coral (top-right)
        print("[+] Extracting Staghorn Coral")
        array, w, h = png_to_rgb565_array(coral_path, "staghorn_coralBitmap",
                                          target_size=(32, 28), crop_box=(680, 0, 1024, 340))
        corals_output += "// Staghorn Coral - 32x28\n" + array + "\n"
        
        # More corals from middle row
        print("[+] Extracting Additional Corals (middle row)")
        array, w, h = png_to_rgb565_array(coral_path, "coral_extra1",
                                          target_size=(24, 20), crop_box=(0, 340, 340, 680))
        corals_output += "// Extra Coral 1 - 24x20\n" + array + "\n"
        
        # Tube Coral (bottom-left)
        print("[+] Extracting Tube Coral")
        array, w, h = png_to_rgb565_array(coral_path, "tube_coralBitmap",
                                          target_size=(20, 24), crop_box=(680, 680, 1024, 1024))
        corals_output += "// Tube Coral - 20x24\n" + array + "\n"
        
        with open(os.path.join(output_dir, "corals.h"), "w") as f:
            f.write(corals_output)
        
        print("[OK] Corals converted\n")
    
    print("\n[DONE] All sprites converted!")
    print(f"\nGenerated files in {output_dir}/:")
    print("  - particles.h (Crumb, Heart, ZZZ, Dirt)")
    print("  - kelp.h")
    print("  - corals.h (Brain, Fan, Staghorn, Tube + extras)")

if __name__ == "__main__":
    main()
