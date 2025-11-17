#pragma once
#include <Arduino.h>

struct DirtSpot {
  int16_t x, y;
  uint8_t kind;      // 0-3 for visual variety
  uint8_t strength;  // 0-100 (opacity/darkness)
  float timeAlive;
  bool active;
};

constexpr uint8_t MAX_DIRT_SPOTS = 20;

void initDirt();
void updateDirt(float deltaTime);
void drawDirt();
void cleanDirt();  // Reduce all spots when cleaning action triggered
uint8_t getTotalDirtLevel();  // Sum of all strengths (for status)
