#pragma once
#include <Arduino.h>

// Legacy API (version 1 format)
bool loadStats(int16_t& hunger, int16_t& fun, int16_t& energy);
void saveStatsIfDue(int16_t hunger, int16_t fun, int16_t energy, bool eventSave);

// Extended API (version 2 format)
bool hasSave();
bool loadFull(int16_t& hunger, int16_t& fun, int16_t& energy, int16_t& hp, uint32_t& ageSec, bool& dead);
void saveFullIfDue(int16_t hunger, int16_t fun, int16_t energy, int16_t hp, uint32_t ageSec, bool dead, bool eventSave);
void clearSave();
