#include "particles.h"
#include "gfx.h"
#include "sprite_common.h"
#include "sprites/particles.h"
#include <math.h>

constexpr uint8_t PARTICLE_MAX_WIDTH = PARTICLE_DIRT_WIDTH;
constexpr uint8_t PARTICLE_MAX_HEIGHT = PARTICLE_DIRT_HEIGHT;

Particle gParticles[MAX_PARTICLES];

static struct {
  int16_t x, y;
} prevPositions[MAX_PARTICLES];

// Helper: Get particle sprite dimensions
static void getParticleWH(ParticleType type, uint8_t* w, uint8_t* h) {
  switch (type) {
    case PARTICLE_DIRT:
      *w = PARTICLE_DIRT_WIDTH;
      *h = PARTICLE_DIRT_HEIGHT;
      break;
    default:
      *w = 8;
      *h = 8;
      break;
  }
}

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

    switch (p.type) {
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

// Restore all particle regions (call BEFORE drawing other moving objects)
void restoreParticleRegions() {
  for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
    if (prevPositions[i].x >= 0) {
      if (prevPositions[i].x < PLAY_AREA_X || prevPositions[i].x >= PLAY_AREA_X + PLAY_AREA_W ||
          prevPositions[i].y < PLAY_AREA_Y || prevPositions[i].y >= PLAY_AREA_Y + PLAY_AREA_H) {
        prevPositions[i].x = -1;
        prevPositions[i].y = -1;
        continue;
      }
      uint8_t w, h;
      // Use last known type if particle still alive, otherwise default
      if (gParticles[i].alive) {
        getParticleWH(gParticles[i].type, &w, &h);
      } else {
        w = PARTICLE_MAX_WIDTH;
        h = PARTICLE_MAX_HEIGHT;
      }
      int16_t margin = 2;
      int16_t rx = max((int16_t)PLAY_AREA_X, (int16_t)(prevPositions[i].x - margin));
      int16_t ry = max((int16_t)PLAY_AREA_Y, (int16_t)(prevPositions[i].y - margin));
      int16_t rw = min((int16_t)(w + margin * 2), (int16_t)(PLAY_AREA_X + PLAY_AREA_W - rx));
      int16_t rh = min((int16_t)(h + margin * 2), (int16_t)(PLAY_AREA_Y + PLAY_AREA_H - ry));
      if (rw > 0 && rh > 0) {
        restoreRegion(rx, ry, rw, rh);
      }
      prevPositions[i].x = -1;
      prevPositions[i].y = -1;
    }
  }
}

void drawParticles() {
  for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
    if (!gParticles[i].alive) {
      continue;
    }

    Particle& p = gParticles[i];

    int16_t px = static_cast<int16_t>(p.x);
    int16_t py = static_cast<int16_t>(p.y);

    float alpha = 1.0f - (p.age / p.lifetime);
    alpha = constrain(alpha, 0.0f, 1.0f);

    const uint16_t* sprite = nullptr;
    uint8_t w = 8, h = 8;

    switch (p.type) {
      case PARTICLE_DIRT:
        sprite = particle_dirt;
        w = PARTICLE_DIRT_WIDTH;
        h = PARTICLE_DIRT_HEIGHT;
        break;
      default:
        break;
    }

    if (sprite) {
      for (uint8_t dy = 0; dy < h; dy++) {
        for (uint8_t dx = 0; dx < w; dx++) {
          int16_t sx = px + dx;
          int16_t sy = py + dy;
          if (sx < PLAY_AREA_X || sx >= PLAY_AREA_X + PLAY_AREA_W ||
              sy < PLAY_AREA_Y || sy >= PLAY_AREA_Y + PLAY_AREA_H) continue;
          uint16_t color = pgm_read_word(&sprite[dy * w + dx]);
          if (color != TRANSPARENT_COLOR && shouldDrawPixel(sx, sy, alpha)) {
            tft.drawPixel(sx, sy, color);
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
