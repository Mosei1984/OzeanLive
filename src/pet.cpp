#include <math.h>
#include "pet.h"
#include "gfx.h"
#include "sprites/clownfish.h"
#include "animator.h"
#include "particles.h"
#include "dirt.h"

// Globale Pet-Instanz
PetStats pet;
PetAction pendingAction = ACTION_NONE;
float animPhase = 0.0f;

// Interne Variablen für Fischbewegung
static float fishX = 0.0f;
static float fishY = 0.0f;
float fishVX = 0.0f;
float fishVY = 0.0f;

static float prevFishDrawX = -1.0f;
static float prevFishDrawY = -1.0f;

// aktueller Ziel-Wegpunkt
static float targetX = 0.0f;
static float targetY = 0.0f;
static bool petInitDone = false;

// Swim phase for organic movement
static float swimPhase = 0.0f;

// Movement noise for natural behavior
static float noiseAccum = 0.0f;
static float noiseValue = 0.0f;

// Parameter für "Steering"
constexpr float FISH_MAX_SPEED = 60.0f;    // Pixel pro Sekunde
constexpr float FISH_STEER_FORCE = 80.0f;  // wie schnell Richtung gewechselt wird
constexpr float FISH_ARRIVE_RADIUS = 10.0f;

// Hilfsfunktion: neuen Wegpunkt wählen (zufällig im Aquarium)
static void chooseNewTarget() {
  targetX = PLAY_AREA_X + 40 + random(0, max(10, PLAY_AREA_W - 80));
  targetY = PLAY_AREA_Y + 20 + random(0, max(10, PLAY_AREA_H - 60));
}

// Initialisiert Pet-Werte
void initPet() {
  pet.hunger = 30;
  pet.fun    = 70;
  pet.energy = 80;
  pet.lastUpdateMs = millis();

  // Startposition des Fisches in die Mitte legen
  fishX = PLAY_AREA_X + PLAY_AREA_W / 2.0f;
  fishY = PLAY_AREA_Y + PLAY_AREA_H / 2.0f;
  fishVX = 0.0f;
  fishVY = 0.0f;

  chooseNewTarget();
  
  initAnimator();
  gAnimator.currentClip = &CLIP_IDLE;
  
  petInitDone = true;
}

// Aktualisiert Pet-Werte in einfacher Simulationslogik
void updatePetStats() {
  unsigned long now = millis();
  unsigned long dt = now - pet.lastUpdateMs;
  if (dt < 1000) return; // nur etwa einmal pro Sekunde

  pet.lastUpdateMs = now;

  // Passive Veränderung über Zeit
  if (pet.hunger < 100) pet.hunger += 1;
  if (pet.fun    >   0) pet.fun    -= 1;
  if (pet.energy >   0) pet.energy -= 1;
}

// Easing functions for smooth movement
static float easeOutCubic(float t) {
  float f = t - 1.0f;
  return f * f * f + 1.0f;
}

// Unused for now, kept for future use
// static float easeInOutQuad(float t) {
//   return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
// }

// Interne Funktion: Fischposition entlang Wegpunkten aktualisieren
static void updateFishMovement(float dtSec) {
  if (!petInitDone) {
    initPet();
    return;
  }

  if (dtSec <= 0.0f || dtSec > 0.5f) {
    dtSec = 0.02f;
  }

  // Richtung zum Ziel
  float dx = targetX - fishX;
  float dy = targetY - fishY;
  float dist = sqrtf(dx*dx + dy*dy);

  if (dist < FISH_ARRIVE_RADIUS) {
    // Neuer Wegpunkt, wenn nahe genug
    chooseNewTarget();
  } else if (dist > 0.1f) {
    // Normalisiere Richtung
    float dirX = dx / dist;
    float dirY = dy / dist;

    // gewünschte Geschwindigkeit (sanft abbremsen beim Ziel mit Easing)
    float desiredSpeed = FISH_MAX_SPEED;
    if (dist < 50.0f) {
      float normalizedDist = dist / 50.0f;
      desiredSpeed = FISH_MAX_SPEED * easeOutCubic(normalizedDist);
    }
    float desiredVX = dirX * desiredSpeed;
    float desiredVY = dirY * desiredSpeed;
    
    // Add lateral noise for natural behavior
    float currentSpeed = sqrtf(fishVX*fishVX + fishVY*fishVY);
    if (dist > FISH_ARRIVE_RADIUS && currentSpeed > 5.0f) {
      noiseAccum += dtSec * 2.0f;
      if (noiseAccum > 1.0f) {
        noiseValue = (random(-100, 101) / 100.0f) * 0.15f;
        noiseAccum = 0.0f;
      }
      float perpX = -dirY;
      float perpY = dirX;
      desiredVX += perpX * noiseValue * FISH_MAX_SPEED * 0.2f;
      desiredVY += perpY * noiseValue * FISH_MAX_SPEED * 0.2f;
    }

    // Steering = gewünschte Geschwindigkeit - aktuelle Geschwindigkeit
    float steerX = desiredVX - fishVX;
    float steerY = desiredVY - fishVY;

    // Limit Steering-Kraft
    float steerMag = sqrtf(steerX*steerX + steerY*steerY);
    if (steerMag > FISH_STEER_FORCE) {
      steerX = steerX / steerMag * FISH_STEER_FORCE;
      steerY = steerY / steerMag * FISH_STEER_FORCE;
    }

    // Geschwindigkeit integrieren
    fishVX += steerX * dtSec;
    fishVY += steerY * dtSec;

    // Geschwindigkeitsbegrenzung
    float vMag = sqrtf(fishVX*fishVX + fishVY*fishVY);
    if (vMag > FISH_MAX_SPEED && vMag > 0.0f) {
      fishVX = fishVX / vMag * FISH_MAX_SPEED;
      fishVY = fishVY / vMag * FISH_MAX_SPEED;
    }

    // Position integrieren
    fishX += fishVX * dtSec;
    fishY += fishVY * dtSec;
  }
  
  // Update swim phase based on current speed
  float currentSpeed = sqrtf(fishVX*fishVX + fishVY*fishVY);
  float speedNorm = currentSpeed / FISH_MAX_SPEED;
  const float TAIL_FREQ_HZ = 1.6f;
  swimPhase += speedNorm * (2.0f * PI * TAIL_FREQ_HZ) * dtSec;
  if (swimPhase > 2.0f * PI) swimPhase -= 2.0f * PI;

  // Begrenzung auf Play-Area
  if (fishX < PLAY_AREA_X + CLOWNFISH_WIDTH / 2) {
    fishX = PLAY_AREA_X + CLOWNFISH_WIDTH / 2;
    fishVX = fabsf(fishVX);
  }
  if (fishX > PLAY_AREA_X + PLAY_AREA_W - CLOWNFISH_WIDTH / 2) {
    fishX = PLAY_AREA_X + PLAY_AREA_W - CLOWNFISH_WIDTH / 2;
    fishVX = -fabsf(fishVX);
  }
  if (fishY < PLAY_AREA_Y + CLOWNFISH_HEIGHT / 2) {
    fishY = PLAY_AREA_Y + CLOWNFISH_HEIGHT / 2;
    fishVY = fabsf(fishVY);
  }
  if (fishY > PLAY_AREA_Y + PLAY_AREA_H - CLOWNFISH_HEIGHT / 2) {
    fishY = PLAY_AREA_Y + PLAY_AREA_H - CLOWNFISH_HEIGHT / 2;
    fishVY = -fabsf(fishVY);
  }
}

// Zeichnet Fisch + leichte Sinus-Animation
void drawPetAnimated(float dtSec) {
  updateFishMovement(dtSec);
  updateAnimator(dtSec);

  // Apply action effects immediately for instant response
  switch (pendingAction) {
    case ACTION_FEED:
      requestTransition(ANIM_EATING, 0.25f);
      spawnFoodCrumbs(fishX, fishY + 10, 8);
      pet.hunger -= 20;
      if (pet.hunger < 0) pet.hunger = 0;
      pet.energy += 5;
      if (pet.energy > 100) pet.energy = 100;
      break;
    case ACTION_PLAY:
      requestTransition(ANIM_PLAYING, 0.25f);
      spawnHearts(fishX, fishY - 10, 6);
      pet.fun += 20;
      if (pet.fun > 100) pet.fun = 100;
      pet.energy -= 5;
      if (pet.energy < 0) pet.energy = 0;
      break;
    case ACTION_REST:
      requestTransition(ANIM_SLEEPING, 0.25f);
      spawnZZZ(fishX - 15, fishY - 20, 3);
      pet.energy += 20;
      if (pet.energy > 100) pet.energy = 100;
      pet.hunger += 5;
      if (pet.hunger > 100) pet.hunger = 100;
      break;
    case ACTION_CLEAN:
      {
        uint8_t dirtLevel = getTotalDirtLevel();
        if (dirtLevel > 0) {
          requestTransition(ANIM_MOVING, 0.25f);
          spawnDirtPuff(fishX, fishY, 12);
          cleanDirt();
          pet.energy -= 10;
          if (pet.energy < 0) pet.energy = 0;
        }
      }
      break;
    case ACTION_NONE:
      {
        float speed = sqrtf(fishVX * fishVX + fishVY * fishVY);
        if (speed > 10.0f && gAnimator.currentState != ANIM_MOVING) {
          requestTransition(ANIM_MOVING, 0.2f);
        } else if (speed <= 10.0f && gAnimator.currentState == ANIM_MOVING) {
          requestTransition(ANIM_IDLE, 0.2f);
        }
      }
      break;
  }
  
  // Clear action after processing
  if (pendingAction != ACTION_NONE) {
    pendingAction = ACTION_NONE;
  }

  // Speed-dependent tail movement using swimPhase
  float speedNorm = sqrtf(fishVX*fishVX + fishVY*fishVY) / FISH_MAX_SPEED;
  float swingAmp = 3.0f + speedNorm * 3.0f;
  float bobAmp = 2.0f + speedNorm * 2.0f;
  
  // Scale animation amplitude during transitions, but keep motion smooth (0.4 -> 1.0)
  float ampScale = (gAnimator.transitionProgress < 1.0f) 
                   ? (0.4f + 0.6f * gAnimator.transitionProgress)
                   : 1.0f;
  float swing = sinf(swimPhase) * swingAmp * ampScale;
  float bob = sinf(swimPhase * 0.7f) * bobAmp * ampScale;

  const uint16_t* frame = getCurrentFrame();
  
  // Guard against null frame (shouldn't happen, but prevents crash)
  if (!frame) return;
  
  bool flip = isFlipped();

  int16_t x = static_cast<int16_t>(fishX + swing - CLOWNFISH_WIDTH / 2);
  int16_t y = static_cast<int16_t>(fishY + bob - CLOWNFISH_HEIGHT / 2);

  if (prevFishDrawX >= 0) {
    int16_t margin = 8;
    restoreRegion(prevFishDrawX - margin, prevFishDrawY - margin,
                  CLOWNFISH_WIDTH + margin * 2, CLOWNFISH_HEIGHT + margin * 2);
  }

  drawSpriteOptimized(frame, CLOWNFISH_WIDTH, CLOWNFISH_HEIGHT, x, y, flip);

  prevFishDrawX = x;
  prevFishDrawY = y;
}
