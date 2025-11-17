#pragma once
#include <Arduino.h>

// Load pet stats from EEPROM (returns true if valid save found)
bool loadStats(int16_t& hunger, int16_t& fun, int16_t& energy);

// Save pet stats to EEPROM with rate limiting
// eventSave=true bypasses minimum interval for important events
void saveStatsIfDue(int16_t hunger, int16_t fun, int16_t energy, bool eventSave);
