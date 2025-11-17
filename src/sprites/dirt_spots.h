#pragma once
#include <Arduino.h>

const uint16_t DIRT_SPOT_WIDTH = 8;
const uint16_t DIRT_SPOT_HEIGHT = 8;
const uint16_t DIRT_SPOT_SIZE = 8;  // For compatibility

// Returns sprite data for dirt kind (0-3)
const uint16_t* getDirtSprite(uint8_t kind);
