# OzeanLive - Teensy 4.1 Aquarium Tamagotchi

## Project Overview

This is an **embedded systems project** written in C++ for the Teensy 4.1 microcontroller. It's a digital aquarium/Tamagotchi game that runs on physical hardware with a display and buttons.

**Important**: This project **cannot run in the Replit cloud environment** because it requires specific hardware:
- Teensy 4.1 microcontroller (600 MHz ARM Cortex-M7)
- ST7789 display (1.9" 320x170px)
- 3 physical buttons

## What This Project Does

A highly optimized aquarium Tamagotchi game featuring:
- 60 FPS smooth animations
- Pet animations (Idle, Moving, Eating, Playing, Sleeping)
- Particle effects (food crumbs, hearts, ZZZ, dirt)
- Dirt/cleaning system
- Natural movement with steering behaviors
- Performance-optimized sprite rendering

## Project Structure

```
src/
├── main.cpp           - Main program loop
├── animator.cpp/h     - Animation state machine
├── pet.cpp/h          - Pet logic and movement
├── particles.cpp/h    - Particle system
├── dirt.cpp/h         - Dirt/cleaning system
├── gfx.cpp/h          - Display and sprite drawing
├── environment.cpp/h  - Background/decorations
├── bubbles.cpp/h      - Bubble animations
├── menu.cpp/h         - Button input and menu
└── sprites/           - Sprite assets
```

## Technology Stack

- **Language**: C++ (Arduino Framework)
- **Build System**: PlatformIO
- **Target Hardware**: Teensy 4.1
- **Libraries**:
  - Adafruit GFX Library @ ^1.11.9
  - Adafruit ST7735 and ST7789 Library @ ^1.10.3

## Development in Replit

While you cannot run this project in Replit (it needs physical hardware), you can:
1. Edit the code
2. Review the architecture
3. Understand the game logic
4. Potentially compile with PlatformIO (if PlatformIO CLI is available)

## Hardware Setup

### Pin Configuration (Teensy 4.1)
- Display CS: Pin 10
- Display DC: Pin 9
- Display RST: Pin 8
- Button Left: Pin 2
- Button OK: Pin 3
- Button Right: Pin 4

## To Build and Upload (on local machine)

```bash
pio run --target upload
```

## Generated Sprites

Beautiful pixel art sprites have been created for the game:

### Location
All sprites are in: `attached_assets/generated_images/`

### What's Available
- **Clownfish Animations**: 6 sprite frames for different states (idle, moving, eating, playing, sleeping)
- **Environment**: Seahorse, corals, kelp decorations
- **Particles**: Food crumbs, hearts, ZZZ, dirt effects

### Conversion Tool
Use `convert_sprite.py` to convert PNG sprites to RGB565 format:

```bash
python convert_sprite.py input.png 30 25 variable_name output.txt
```

See `SPRITE_CONVERSION_GUIDE.md` for detailed instructions!

## Recent Changes

- [2025-11-17] Generated sprite artwork for all game elements
- [2025-11-17] Created sprite conversion tools and documentation
- [2025-11-17] Imported from GitHub into Replit
- Project is ready for code editing but cannot execute without hardware

## Notes

This is a hardware project, not a web application. It's designed to be compiled and uploaded to a Teensy 4.1 microcontroller, not run in a browser or cloud environment.
