#include <math.h>
#include "pet.h"
#include "gfx.h"
#include "sprites/clownfish.h"
#include "animator.h"
#include "particles.h"
#include "dirt.h"
#include "eeprom_store.h"
#include "environment.h"

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

// Poop counter for feeding mechanic
static uint8_t feedCount = 0;
static uint8_t nextPoopAt = 5;

// Animation duration tracking
static float actionTimer = 0.0f;
static bool actionInProgress = false;

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

  nextPoopAt = random(5, 8);
  
  chooseNewTarget();
  
  initAnimator();
  gAnimator.currentClip = &CLIP_IDLE;
  
  petInitDone = true;
}

// Aktualisiert Pet-Werte in einfacher Simulationslogik
void updatePetStats() {
  // Handle time accumulation properly (even across millis wrap or long pauses)
  static uint32_t msAccum = 0;
  unsigned long now = millis();
  uint32_t dt = (uint32_t)(now - pet.lastUpdateMs);
  pet.lastUpdateMs = now;
  msAccum = min(msAccum + dt, 60000u); // cap at 60s
  uint32_t ticks = msAccum / 1000;
  msAccum -= ticks * 1000;
  if (ticks) {
    int h = pet.hunger + (int)ticks;
    int f = pet.fun    - (int)ticks;  // normal decay
    int e = pet.energy - (int)ticks;
    
    // Dirt accelerates fun decay (double speed when dirty)
    uint8_t dirt = getTotalDirtLevel();
    if (dirt > 0) {
      f -= (int)ticks;  // additional decay
    }
    
    pet.hunger = (h > 100) ? 100 : h;
    pet.fun    = (f <   0) ?   0 : f;
    pet.energy = (e <   0) ?   0 : e;
    saveStatsIfDue(pet.hunger, pet.fun, pet.energy, false);
  }
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

  // Block movement during EATING or POOPING animations
  if (gAnimator.currentState == ANIM_EATING || gAnimator.currentState == ANIM_POOPING) {
    fishVX = 0.0f;
    fishVY = 0.0f;
    return;
  }

  // Richtung zum Ziel
  float dx = targetX - fishX;
  float dy = targetY - fishY;
  float dist = sqrtf(dx*dx + dy*dy);

  // Don't choose new target while sleeping - stay at anemone
  bool sleeping = (gAnimator.currentState == ANIM_SLEEPING) || (pendingAction == ACTION_REST);
  
  if (dist < FISH_ARRIVE_RADIUS) {
    // Neuer Wegpunkt, wenn nahe genug (aber nicht beim Schlafen)
    if (!sleeping) {
      chooseNewTarget();
    } else {
      // Gently settle at anemone
      fishVX *= 0.85f;
      fishVY *= 0.85f;
    }
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

  // Handle action timers for EATING (10s) and POOPING (3s)
  if (actionInProgress) {
    actionTimer += dtSec;
    
    // Check if action duration completed
    bool actionComplete = false;
    if (gAnimator.currentState == ANIM_EATING && actionTimer >= 10.0f) {
      actionComplete = true;
      
      // Check if we should poop after eating
      if (feedCount >= nextPoopAt) {
        pendingAction = ACTION_POOP;
        actionInProgress = false;
        actionTimer = 0.0f;
      }
    } else if (gAnimator.currentState == ANIM_POOPING && actionTimer >= 3.0f) {
      actionComplete = true;
    }
    
    // Resume normal movement after action completes
    if (actionComplete) {
      actionInProgress = false;
      actionTimer = 0.0f;
      requestTransition(ANIM_IDLE, 0.25f);
    }
  }

  // Apply action effects immediately for instant response
  switch (pendingAction) {
    case ACTION_FEED:
      if (!actionInProgress) {
        requestTransition(ANIM_EATING, 0.25f);
        pet.hunger = max(0, pet.hunger - 20);
        pet.fun = min(100, pet.fun + 5);
        feedCount++;
        actionInProgress = true;
        actionTimer = 0.0f;
      }
      break;
    case ACTION_POOP:
      if (!actionInProgress) {
        requestTransition(ANIM_POOPING, 0.25f);
        spawnPoopSpot((int16_t)fishX);
        feedCount = 0;
        nextPoopAt = random(5, 8);
        actionInProgress = true;
        actionTimer = 0.0f;
      }
      break;
    case ACTION_PLAY:
      requestTransition(ANIM_PLAYING, 0.25f);
      // No particles needed - playing frame has hearts built-in
      pet.fun += 20;
      if (pet.fun > 100) pet.fun = 100;
      pet.energy -= 5;
      if (pet.energy < 0) pet.energy = 0;
      break;
    case ACTION_REST:
      requestTransition(ANIM_SLEEPING, 0.25f);
      // No particles needed - sleeping frame has ZZZ built-in
      {
        int16_t anemX, anemY;
        getAnemonePosition(anemX, anemY);
        targetX = anemX;
        targetY = anemY;
      }
      pet.energy = min(100, pet.energy + 20);
      pet.hunger = min(100, pet.hunger + 5);
      pet.fun = min(100, pet.fun + 5);  // slight fun boost
      break;
    case ACTION_CLEAN:
      {
        uint8_t dirtLevel = getTotalDirtLevel();
        if (dirtLevel > 0) {
          requestTransition(ANIM_MOVING, 0.25f);
          spawnDirtPuff(fishX, fishY, 12);
          cleanDirt();
          pet.fun = min(100, pet.fun + 15);  // instant fun boost
        }
      }
      break;
    case ACTION_NONE:
      {
        // Keep fish at anemone when sleeping
        if (gAnimator.currentState == ANIM_SLEEPING && gAnimator.transitionProgress >= 1.0f) {
          int16_t anemX, anemY;
          getAnemonePosition(anemX, anemY);
          targetX = anemX;
          targetY = anemY;
        }
        
        // Hysteresis to prevent animation thrashing at speed boundary
        float speed = sqrtf(fishVX * fishVX + fishVY * fishVY);
        if (gAnimator.currentState == ANIM_IDLE && speed > 12.0f) {
          requestTransition(ANIM_MOVING, 0.2f);
        } else if (gAnimator.currentState == ANIM_MOVING && speed < 8.0f) {
          requestTransition(ANIM_IDLE, 0.2f);
        }
      }
      break;
  }
  
  // Clear action after processing
  if (pendingAction != ACTION_NONE) {
    saveStatsIfDue(pet.hunger, pet.fun, pet.energy, true);
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

  drawSpriteOptimized(frame, CLOWNFISH_WIDTH, CLOWNFISH_HEIGHT, x, y, flip);

  prevFishDrawX = x;
  prevFishDrawY = y;
}

void restorePetRegion() {
  if (prevFishDrawX >= 0) {
    int16_t margin = 8;
    int16_t rx = max((int16_t)PLAY_AREA_X, (int16_t)(prevFishDrawX - margin));
    int16_t ry = max((int16_t)PLAY_AREA_Y, (int16_t)(prevFishDrawY - margin));
    int16_t rw = min((int16_t)(CLOWNFISH_WIDTH + margin * 2), (int16_t)(PLAY_AREA_X + PLAY_AREA_W - rx));
    int16_t rh = min((int16_t)(CLOWNFISH_HEIGHT + margin * 2), (int16_t)(PLAY_AREA_Y + PLAY_AREA_H - ry));
    if (rw > 0 && rh > 0) {
      restoreRegion(rx, ry, rw, rh);
    }
  }
}
