#pragma once
#include <Arduino.h>

enum ParticleType {
  PARTICLE_NONE = 0,
  PARTICLE_FOOD_CRUMB,
  PARTICLE_HEART,
  PARTICLE_ZZZ,
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
void drawParticles();

void spawnFoodCrumbs(float centerX, float centerY, uint8_t count);
void spawnHearts(float centerX, float centerY, uint8_t count);
void spawnZZZ(float centerX, float centerY, uint8_t count);
void spawnDirtPuff(float centerX, float centerY, uint8_t count);
