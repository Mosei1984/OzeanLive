#pragma once
#include <Arduino.h>

constexpr int16_t DIRT_SPOT_SIZE = 12;

// Returns sprite data for dirt kind (0-3)
const uint16_t* getDirtSprite(uint8_t kind);
