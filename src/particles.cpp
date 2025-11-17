#include "particles.h"
#include "gfx.h"
#include "sprites/particles.h"
#include <math.h>

Particle gParticles[MAX_PARTICLES];

static struct {
  int16_t x, y;
} prevPositions[MAX_PARTICLES];

void initParticles() {
  for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
    gParticles[i].alive = false;
    prevPositions[i].x = -1;
    prevPositions[i].y = -1;
  }
}

static bool shouldDrawPixel(int16_t x, int16_t y, float alpha) {
  uint8_t pat = (x ^ y) & 3;
  if (alpha > 0.75f) return true;
  if (alpha > 0.50f) return pat < 3;
  if (alpha > 0.25f) return pat < 2;
  return pat == 0;
}

void updateParticles(float deltaTime) {
  if (deltaTime <= 0.0f || deltaTime > 0.5f) {
    deltaTime = 0.0167f;
  }

  for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
    if (!gParticles[i].alive) continue;

    Particle& p = gParticles[i];

    constexpr float GRAVITY = 20.0f;
    constexpr float BUOYANCY = -15.0f;

    switch (p.type) {
      case PARTICLE_FOOD_CRUMB:
        p.vy += GRAVITY * deltaTime;
        break;
      case PARTICLE_HEART:
      case PARTICLE_ZZZ:
        p.vy += BUOYANCY * deltaTime;
        break;
      case PARTICLE_DIRT:
        p.vy += GRAVITY * 0.5f * deltaTime;
        break;
      default:
        break;
    }

    p.vx *= 0.98f;
    p.vy *= 0.98f;

    p.x += p.vx * deltaTime;
    p.y += p.vy * deltaTime;

    p.age += deltaTime;
    if (p.age >= p.lifetime) {
      p.alive = false;
    }
  }
}

void drawParticles() {
  for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
    if (!gParticles[i].alive) {
      if (prevPositions[i].x >= 0) {
        restoreRegion(prevPositions[i].x - 2, prevPositions[i].y - 2, 16, 16);
        prevPositions[i].x = -1;
        prevPositions[i].y = -1;
      }
      continue;
    }

    Particle& p = gParticles[i];

    if (prevPositions[i].x >= 0) {
      restoreRegion(prevPositions[i].x - 2, prevPositions[i].y - 2, 16, 16);
    }

    int16_t px = static_cast<int16_t>(p.x);
    int16_t py = static_cast<int16_t>(p.y);

    float alpha = 1.0f - (p.age / p.lifetime);
    alpha = constrain(alpha, 0.0f, 1.0f);

    const uint16_t* sprite = nullptr;
    uint8_t w = 8, h = 8;

    switch (p.type) {
      case PARTICLE_FOOD_CRUMB:
        sprite = PARTICLE_CRUMB;
        w = PARTICLE_CRUMB_WIDTH;
        h = PARTICLE_CRUMB_HEIGHT;
        break;
      case PARTICLE_HEART:
        sprite = PARTICLE_HEART;
        w = PARTICLE_HEART_WIDTH;
        h = PARTICLE_HEART_HEIGHT;
        break;
      case PARTICLE_ZZZ:
        sprite = PARTICLE_ZZZ;
        w = PARTICLE_ZZZ_WIDTH;
        h = PARTICLE_ZZZ_HEIGHT;
        break;
      case PARTICLE_DIRT:
        sprite = PARTICLE_DIRT;
        w = PARTICLE_DIRT_WIDTH;
        h = PARTICLE_DIRT_HEIGHT;
        break;
      default:
        break;
    }

    if (sprite) {
      for (uint8_t dy = 0; dy < h; dy++) {
        for (uint8_t dx = 0; dx < w; dx++) {
          uint16_t color = sprite[dy * w + dx];
          if (color != TRANSPARENT_COLOR && shouldDrawPixel(px + dx, py + dy, alpha)) {
            tft.drawPixel(px + dx, py + dy, color);
          }
        }
      }
    }

    prevPositions[i].x = px;
    prevPositions[i].y = py;
  }
}

static void spawnParticle(float x, float y, float vx, float vy, ParticleType type, float lifetime) {
  for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
    if (!gParticles[i].alive) {
      gParticles[i].x = x;
      gParticles[i].y = y;
      gParticles[i].vx = vx;
      gParticles[i].vy = vy;
      gParticles[i].type = type;
      gParticles[i].frame = 0;
      gParticles[i].age = 0.0f;
      gParticles[i].lifetime = lifetime;
      gParticles[i].alive = true;
      return;
    }
  }
}

void spawnFoodCrumbs(float centerX, float centerY, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    float angle = random(0, 360) * DEG_TO_RAD;
    float speed = random(10, 40);
    float vx = cos(angle) * speed;
    float vy = sin(angle) * speed - 20.0f;
    float lifetime = random(10, 20) / 10.0f;
    spawnParticle(centerX, centerY, vx, vy, PARTICLE_FOOD_CRUMB, lifetime);
  }
}

void spawnHearts(float centerX, float centerY, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    float angle = random(-30, 30) * DEG_TO_RAD;
    float speed = random(15, 30);
    float vx = sin(angle) * speed;
    float vy = -abs(cos(angle) * speed);
    float lifetime = random(15, 25) / 10.0f;
    spawnParticle(centerX, centerY, vx, vy, PARTICLE_HEART, lifetime);
  }
}

void spawnZZZ(float centerX, float centerY, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    float vx = random(-10, 10);
    float vy = random(-30, -15);
    float lifetime = random(15, 30) / 10.0f;
    spawnParticle(centerX, centerY, vx, vy, PARTICLE_ZZZ, lifetime);
  }
}

void spawnDirtPuff(float centerX, float centerY, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    float angle = random(0, 360) * DEG_TO_RAD;
    float speed = random(20, 50);
    float vx = cos(angle) * speed;
    float vy = sin(angle) * speed;
    float lifetime = random(5, 10) / 10.0f;
    spawnParticle(centerX, centerY, vx, vy, PARTICLE_DIRT, lifetime);
  }
}
