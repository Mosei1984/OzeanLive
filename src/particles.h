#pragma once
#include <Arduino.h>

enum ParticleType {
  PARTICLE_NONE = 0,
  PARTICLE_DIRT
};

struct Particle {
  float x, y;
  float vx, vy;
  ParticleType type;
  uint8_t frame;
  float age;
  float lifetime;
  bool alive;
};

constexpr uint8_t MAX_PARTICLES = 30;

void initParticles();
void updateParticles(float deltaTime);
void restoreParticleRegions();
void drawParticles();

void spawnDirtPuff(float centerX, float centerY, uint8_t count);
