#pragma once
#include <Arduino.h>

// Globale Transparenzfarbe (Pink aus deinem Sprite-Generator)
constexpr uint16_t TRANSPARENT_COLOR = 0xF81F; // magenta
constexpr uint16_t TRANSPARENT_COLOR_SWAP = 0x1FF8; // byte-swapped magenta

// Robust transparency check (handles both byte orders)
inline bool isTransparent16(uint16_t c) { 
  return c == TRANSPARENT_COLOR || c == TRANSPARENT_COLOR_SWAP; 
}
