#include "particles.h"

constexpr uint16_t T = TRANSPARENT_COLOR;
constexpr uint16_t BROWN = 0x6200;
constexpr uint16_t LIGHT_BROWN = 0x8A40;
constexpr uint16_t RED = 0xF800;
constexpr uint16_t PINK = 0xF81F;
constexpr uint16_t LIGHT_PINK = 0xFCDF;
constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t LIGHT_GRAY = 0xC618;
constexpr uint16_t GRAY = 0x8410;
constexpr uint16_t DARK_GRAY = 0x4208;
constexpr uint16_t DARK_GREEN = 0x0320;

const uint16_t PARTICLE_CRUMB[64] PROGMEM = {
  T, T, T, T, T, T, T, T,
  T, T, BROWN, BROWN, BROWN, T, T, T,
  T, BROWN, LIGHT_BROWN, LIGHT_BROWN, BROWN, BROWN, T, T,
  T, BROWN, LIGHT_BROWN, BROWN, BROWN, BROWN, T, T,
  T, BROWN, BROWN, BROWN, BROWN, T, T, T,
  T, T, BROWN, BROWN, T, T, T, T,
  T, T, T, T, T, T, T, T,
  T, T, T, T, T, T, T, T
};

const uint16_t PARTICLE_HEART[64] PROGMEM = {
  T, T, T, T, T, T, T, T,
  T, RED, PINK, T, RED, PINK, T, T,
  RED, LIGHT_PINK, LIGHT_PINK, PINK, LIGHT_PINK, LIGHT_PINK, RED, T,
  RED, LIGHT_PINK, LIGHT_PINK, LIGHT_PINK, LIGHT_PINK, LIGHT_PINK, RED, T,
  T, RED, LIGHT_PINK, LIGHT_PINK, LIGHT_PINK, RED, T, T,
  T, T, RED, LIGHT_PINK, RED, T, T, T,
  T, T, T, RED, T, T, T, T,
  T, T, T, T, T, T, T, T
};

const uint16_t PARTICLE_ZZZ[64] PROGMEM = {
  T, T, T, T, T, T, T, T,
  T, WHITE, WHITE, WHITE, WHITE, T, T, T,
  T, T, T, WHITE, T, T, T, T,
  T, T, WHITE, T, T, T, T, T,
  T, WHITE, WHITE, WHITE, WHITE, T, T, T,
  T, T, T, T, T, T, T, T,
  T, T, T, T, T, T, T, T,
  T, T, T, T, T, T, T, T
};

const uint16_t PARTICLE_DIRT[64] PROGMEM = {
  T, T, T, T, T, T, T, T,
  T, GRAY, LIGHT_GRAY, DARK_GRAY, T, T, T, T,
  T, DARK_GREEN, GRAY, GRAY, DARK_GRAY, T, T, T,
  T, T, DARK_GRAY, DARK_GREEN, GRAY, DARK_GRAY, T, T,
  T, T, T, DARK_GRAY, DARK_GREEN, GRAY, T, T,
  T, T, T, T, DARK_GRAY, T, T, T,
  T, T, T, T, T, T, T, T,
  T, T, T, T, T, T, T, T
};
