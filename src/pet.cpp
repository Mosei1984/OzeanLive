#include <math.h>
#include "pet.h"
#include "gfx.h"
#include "sprites/clownfish.h"
#include "animator.h"
#include "particles.h"
#include "dirt.h"
#include "eeprom_store.h"
#include "environment.h"

static constexpr uint32_t LIFE_YEAR_SEC = 24ul*60*60;
static constexpr uint32_t BAD_START_SEC = 3600;
static constexpr uint32_t HP_LOSS_INTERVAL_SEC = 3600;

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

// Idle pause system (fish rests randomly at waypoints)
static bool isIdlePausing = false;
static float idlePauseTimer = 0.0f;
static float idlePauseDuration = 0.0f;
static float timeSinceLastPause = 0.0f;

// Poop counter for feeding mechanic
static uint8_t feedCount = 0;
static uint8_t nextPoopAt = 5;

// Animation duration tracking
static float actionTimer = 0.0f;
static bool actionInProgress = false;

// Public function to check if action is in progress
bool isActionInProgress() {
  return actionInProgress;
}

// HP damage tracking
static uint32_t hungerBadSec = 0;
static uint32_t energyBadSec = 0;
static uint32_t funBadSec = 0;
static uint32_t hungerDmgAcc = 0;
static uint32_t energyDmgAcc = 0;
static uint32_t funDmgAcc = 0;

// Parameter für "Steering"
constexpr float FISH_MAX_SPEED = 20.0f;    // Pixel pro Sekunde (reduziert von 60)
constexpr float FISH_STEER_FORCE = 15.0f;  // wie schnell Richtung gewechselt wird (reduziert von 80)
constexpr float FISH_ARRIVE_RADIUS = 10.0f;

// Hilfsfunktion: neuen Wegpunkt wählen (zufällig im Aquarium)
static void chooseNewTarget() {
  targetX = PLAY_AREA_X + 60 + random(0, max(10, PLAY_AREA_W - 80));
  targetY = PLAY_AREA_Y + 40 + random(0, max(10, PLAY_AREA_H - 60));
}

int16_t getMaxHP() {
  uint32_t years = pet.ageSec / LIFE_YEAR_SEC;
  int16_t extra = (years / 5u) * 10;
  return 20 + extra;
}

// Initialisiert Pet-Werte
void initPet() {
#ifdef DEBUG_GAME_LOGIC
  Serial.println("[PET] Initializing pet...");
#endif
  
  pet.hunger = 30;
  pet.fun    = 70;
  pet.energy = 80;
  pet.lastUpdateMs = millis();
  pet.ageSec = 0;
  pet.hp = getMaxHP();
  pet.dead = false;

  // Startposition des Fisches in die Mitte legen
  fishX = PLAY_AREA_X + PLAY_AREA_W / 2.0f;
  fishY = PLAY_AREA_Y + PLAY_AREA_H / 2.0f;
  fishVX = 0.0f;
  fishVY = 0.0f;

  nextPoopAt = random(2, 10);
  
  chooseNewTarget();
  
  initAnimator();
  gAnimator.currentClip = &CLIP_IDLE;
  
  petInitDone = true;
  
#ifdef DEBUG_GAME_LOGIC
  Serial.print("[PET] Pet initialized - HP: ");
  Serial.print(pet.hp);
  Serial.print(", Position: (");
  Serial.print((int)fishX);
  Serial.print(", ");
  Serial.print((int)fishY);
  Serial.println(")");
#endif
}

// Aktualisiert Pet-Werte in einfacher Simulationslogik
void updatePetStats() {
  if (pet.dead) return;

  // Handle time accumulation properly (even across millis wrap or long pauses)
  static uint32_t ageAccum = 0;
  static uint32_t statAccum = 0; // Accumulator for hunger/fun/energy (2 minute intervals)
  static uint32_t sleepAccum = 0; // Accumulator for sleep energy (10 second intervals)
  unsigned long now = millis();
  uint32_t dt = (uint32_t)(now - pet.lastUpdateMs);
  pet.lastUpdateMs = now;
  
  ageAccum = min(ageAccum + dt, 60000u); // cap at 60s
  statAccum = min(statAccum + dt, 240000u); // cap at 4 minutes
  sleepAccum = min(sleepAccum + dt, 60000u); // cap at 60s
  
  // Age updates every second
  uint32_t ageTicks = ageAccum / 1000;
  ageAccum -= ageTicks * 1000;
  
  // Stats update every 2 minutes (120 seconds = 120000ms)
  uint32_t statTicks = statAccum / 120000;
  statAccum -= statTicks * 120000;
  
  // Sleep energy updates every 10 seconds (10000ms)
  uint32_t sleepTicks = sleepAccum / 10000;
  sleepAccum -= sleepTicks * 10000;
  
  if (ageTicks) {
    pet.ageSec += ageTicks;
    
    int16_t maxHP = getMaxHP();
    if (pet.hp > maxHP) pet.hp = maxHP;
  }
  
  if (statTicks) {
    int h = pet.hunger + (int)statTicks; // +1 every 2 minutes
    int f = pet.fun    - (int)statTicks; // -1 every 2 minutes
    int e = pet.energy - (int)statTicks; // -1 every 2 minutes
    
    // Dirt accelerates fun decay (double speed when dirty)
    uint8_t dirt = getTotalDirtLevel();
    if (dirt > 0) {
      f -= (int)statTicks;  // additional decay
    }
    
    pet.hunger = (h > 100) ? 100 : h;
    pet.fun    = (f <   0) ?   0 : f;
    pet.energy = (e <   0) ?   0 : e;
  }
  
  // Sleep energy regeneration (+1 every 10 seconds while sleeping)
  if (sleepTicks && gAnimator.currentState == ANIM_SLEEPING) {
    int e = pet.energy + (int)sleepTicks;
    pet.energy = (e > 100) ? 100 : e;
  }
  
  // HP damage logic (checked every second via ageTicks)
  if (ageTicks) {
    bool hungerBad = (pet.hunger >= 100);
    bool energyBad = (pet.energy == 0);
    bool funBad = (pet.fun == 0);
    
    if (hungerBad) {
      hungerBadSec += ageTicks;
      if (hungerBadSec > BAD_START_SEC) {
        hungerDmgAcc += ageTicks;
        if (hungerDmgAcc >= HP_LOSS_INTERVAL_SEC) {
          pet.hp--;
          hungerDmgAcc = 0;
        }
      }
    } else {
      hungerBadSec = 0;
      hungerDmgAcc = 0;
    }
    
    if (energyBad) {
      energyBadSec += ageTicks;
      if (energyBadSec > BAD_START_SEC) {
        energyDmgAcc += ageTicks;
        if (energyDmgAcc >= HP_LOSS_INTERVAL_SEC) {
          pet.hp--;
          energyDmgAcc = 0;
        }
      }
    } else {
      energyBadSec = 0;
      energyDmgAcc = 0;
    }
    
    if (funBad) {
      funBadSec += ageTicks;
      if (funBadSec > BAD_START_SEC) {
        funDmgAcc += ageTicks;
        if (funDmgAcc >= HP_LOSS_INTERVAL_SEC) {
          pet.hp--;
          funDmgAcc = 0;
        }
      }
    } else {
      funBadSec = 0;
      funDmgAcc = 0;
    }
    
    if (pet.hp <= 0) {
      pet.dead = true;
    }
    
    saveFullIfDue(pet.hunger, pet.fun, pet.energy, pet.hp, pet.ageSec, pet.dead, false);
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

  // Track time since last pause
  if (!isIdlePausing) {
    timeSinceLastPause += dtSec;
  }
  
  // Handle idle pausing at waypoints
  if (isIdlePausing) {
    idlePauseTimer += dtSec;
    fishVX *= 0.9f; // Gradually stop
    fishVY *= 0.9f;
    
    if (idlePauseTimer >= idlePauseDuration) {
      isIdlePausing = false;
      idlePauseTimer = 0.0f;
      timeSinceLastPause = 0.0f; // Reset timer after pause
    }
    return; // Stay idle during pause
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
      // Calculate minimum pause interval based on energy
      // Energy 100% -> 120s (2 minutes) between pauses
      // Energy 20%  -> 30s between pauses
      float energyNorm = pet.energy / 100.0f; // 0.0 to 1.0
      float minPauseInterval = 30.0f + (energyNorm * 90.0f); // 30s to 120s
      
      // Check if enough time has passed since last pause
      if (timeSinceLastPause >= minPauseInterval) {
        // Start idle pause
        isIdlePausing = true;
        idlePauseTimer = 0.0f;
        // Duration: 1-3 seconds, longer when tired
        idlePauseDuration = 3.0f + (random(0, 500) / 100.0f) * (1.0f - energyNorm);
#ifdef DEBUG_GAME_LOGIC
        Serial.print("[PET] Idle pause for ");
        Serial.print(idlePauseDuration);
        Serial.print("s (next pause in ");
        Serial.print(minPauseInterval);
        Serial.println("s)");
#endif
      } else {
        // Continue to next waypoint
        chooseNewTarget();
      }
    } else {
      // Gently settle at anemone
      fishVX *= 0.55f;
      fishVY *= 0.65f;
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
  FramePhase phase = getFramePhase();
  
  // COLLECT phase: Update physics and register dirty rects
  if (phase == PHASE_COLLECT) {
    updateFishMovement(dtSec);
    updateAnimator(dtSec);

    // Handle action timers for EATING (5s), PLAYING (3s) and POOPING (3s)
    if (dtSec > 0 && actionInProgress) {
      actionTimer += dtSec;
    
#ifdef DEBUG_GAME_LOGIC
    static float lastLogTimer = -1.0f;
    if ((int)actionTimer != (int)lastLogTimer) {
      Serial.print("[PET] Action timer: ");
      Serial.print(actionTimer);
      Serial.print("s (state: ");
      Serial.print(gAnimator.currentState);
      Serial.println(")");
      lastLogTimer = actionTimer;
    }
#endif
    
    // Check if action duration completed
    if (gAnimator.currentState == ANIM_EATING && actionTimer >= 5.0f) {
      // Check if we should poop after eating
      if (feedCount >= nextPoopAt) {
#ifdef DEBUG_GAME_LOGIC
        Serial.print("[PET] Feed count reached ");
        Serial.print(feedCount);
        Serial.print("/");
        Serial.print(nextPoopAt);
        Serial.println(" - Triggering poop!");
#endif
        pendingAction = ACTION_POOP;
        actionInProgress = false;
        actionTimer = 0.0f;
      } else {
#ifdef DEBUG_GAME_LOGIC
        Serial.println("[PET] Eating finished - Return to idle");
#endif
        // Just finish eating
        actionInProgress = false;
        actionTimer = 0.0f;
        requestTransition(ANIM_IDLE, 0.25f);
      }
    } else if (gAnimator.currentState == ANIM_POOPING && actionTimer >= 3.0f) {
#ifdef DEBUG_GAME_LOGIC
      Serial.println("[PET] Pooping finished - Return to idle");
#endif
      // Finish pooping
      actionInProgress = false;
      actionTimer = 0.0f;
      requestTransition(ANIM_IDLE, 0.25f);
    } else if (gAnimator.currentState == ANIM_PLAYING && actionTimer >= 3.0f) {
#ifdef DEBUG_GAME_LOGIC
      Serial.println("[PET] Playing finished - Return to idle");
#endif
      // Finish playing
      actionInProgress = false;
      actionTimer = 0.0f;
      requestTransition(ANIM_IDLE, 0.25f);
    } else if (gAnimator.currentState == ANIM_SLEEPING) {
      // Auto-wakeup when energy reaches 100%
      if (pet.energy >= 100) {
#ifdef DEBUG_GAME_LOGIC
        Serial.println("[PET] Energy 100% - Auto wakeup from sleep!");
#endif
        actionInProgress = false;
        actionTimer = 0.0f;
        requestTransition(ANIM_IDLE, 0.25f);
      }
    }
  }

  // Apply action effects immediately for instant response
  switch (pendingAction) {
    case ACTION_FEED:
      if (!actionInProgress) {
#ifdef DEBUG_GAME_LOGIC
        Serial.print("[PET] Action: FEEDING - Starting eating animation (count: ");
        Serial.print(feedCount + 1);
        Serial.print("/");
        Serial.print(nextPoopAt);
        Serial.println(")");
#endif
        requestTransition(ANIM_EATING, 0.25f);
        pet.hunger = max(0, pet.hunger - 5);
        pet.fun = min(100, pet.fun + 3);
        feedCount++;
        actionInProgress = true;
        actionTimer = 0.0f;
      }
      break;
    case ACTION_POOP:
      if (!actionInProgress) {
#ifdef DEBUG_GAME_LOGIC
        Serial.println("[PET] Action: POOPING - Spawning dirt spot");
#endif
        requestTransition(ANIM_POOPING, 0.25f);
        spawnPoopSpot((int16_t)fishX);
        feedCount = 0;
        nextPoopAt = random(3, 10);
#ifdef DEBUG_GAME_LOGIC
        Serial.print("[PET] Poop done - Reset counter, next poop at ");
        Serial.println(nextPoopAt);
#endif
        actionInProgress = true;
        actionTimer = 0.0f;
      }
      break;
    case ACTION_PLAY:
#ifdef DEBUG_GAME_LOGIC
      Serial.println("[PET] Action: PLAYING - Starting play animation");
#endif
      requestTransition(ANIM_PLAYING, 0.25f);
      // No particles needed - playing frame has hearts built-in
      pet.fun += 25;
      if (pet.fun > 100) pet.fun = 100;
      pet.energy -= 15;
      if (pet.energy < 0) pet.energy = 0;
      
      // Start play action (3 second animation)
      actionInProgress = true;
      actionTimer = 0.0f;
      break;
    case ACTION_REST:
#ifdef DEBUG_GAME_LOGIC
      Serial.println("[PET] Action: SLEEPING - Going to anemone");
#endif
      requestTransition(ANIM_SLEEPING, 0.25f);
      // No particles needed - sleeping frame has ZZZ built-in
      {
        int16_t anemX, anemY;
        getAnemonePosition(anemX, anemY);
        targetX = anemX;
        targetY = anemY;
      }
      pet.energy = min(100, pet.energy + 10);
      pet.hunger = min(100, pet.hunger + 2);
      pet.fun = min(100, pet.fun + 2);  // slight fun boost
      
      // Start sleep action (will be ended by auto-wakeup logic)
      actionInProgress = true;
      actionTimer = 0.0f;
      break;
    case ACTION_CLEAN:
      {
        uint8_t dirtLevel = getTotalDirtLevel();
        if (dirtLevel > 0) {
          requestTransition(ANIM_MOVING, 0.25f);
          spawnDirtPuff(fishX, fishY, 12);
          cleanDirt();
          pet.fun = min(100, pet.fun + 5);  // instant fun boost
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
        
        // Animation state machine based on speed and idle pause
        float speed = sqrtf(fishVX * fishVX + fishVY * fishVY);
        
        // If pausing at waypoint, transition to idle (but not during actions)
        if (isIdlePausing && !actionInProgress) {
          if (gAnimator.currentState != ANIM_IDLE) {
            requestTransition(ANIM_IDLE, 0.3f);
          }
        }
        // Otherwise use speed-based animation with hysteresis
        else if (!actionInProgress && gAnimator.currentState == ANIM_IDLE && speed > 12.0f) {
          requestTransition(ANIM_MOVING, 0.2f);
        } else if (!actionInProgress && gAnimator.currentState == ANIM_MOVING && speed < 8.0f) {
          requestTransition(ANIM_IDLE, 0.2f);
        }
      }
      break;
  }
  
  // Clear action after processing
  if (pendingAction != ACTION_NONE) {
    saveFullIfDue(pet.hunger, pet.fun, pet.energy, pet.hp, pet.ageSec, pet.dead, true);
    pendingAction = ACTION_NONE;
  }

    // Speed-dependent tail movement using swimPhase
    float speedNorm = sqrtf(fishVX*fishVX + fishVY*fishVY) / FISH_MAX_SPEED;
    float swingAmp = 3.0f + speedNorm * 3.0f;
    float bobAmp = 2.0f + speedNorm * 2.0f;
    
    // Scale animation amplitude during transitions, but keep motion smooth (0.4 -> 1.0)
    float ampScale = (gAnimator.transitionProgress < 1.0f) 
                     ? (0.3f + 0.7f * gAnimator.transitionProgress)
                     : 1.0f;
    float swing = sinf(swimPhase) * swingAmp * ampScale;
    float bob = sinf(swimPhase * 0.8f) * bobAmp * ampScale;
    
    // Add chewing wobble during EATING animation (4 Hz up-down motion)
    if (gAnimator.currentState == ANIM_EATING && actionInProgress) {
      float chewPhase = actionTimer * 4.0f * 2.0f * PI; // 4 Hz wobble
      bob += sinf(chewPhase) * 3.0f; // 3 pixel vertical wobble
    }

    int16_t x = static_cast<int16_t>(fishX + swing - CLOWNFISH_WIDTH / 2);
    int16_t y = static_cast<int16_t>(fishY + bob - CLOWNFISH_HEIGHT / 2);

    // Clamp to play area to prevent UI ghosting
    x = constrain(x, (int16_t)PLAY_AREA_X, (int16_t)(PLAY_AREA_X + PLAY_AREA_W - CLOWNFISH_WIDTH));
    y = constrain(y, (int16_t)PLAY_AREA_Y, (int16_t)(PLAY_AREA_Y + PLAY_AREA_H - CLOWNFISH_HEIGHT));

    // Register dirty rect for fish (current + previous position)
    addDirtyRectPair(x, y, CLOWNFISH_WIDTH, CLOWNFISH_HEIGHT, prevFishDrawX, prevFishDrawY);
  }
  // DRAW phase: Draw fish and commit position
  else if (phase == PHASE_DRAW) {
    // Speed-dependent tail movement using swimPhase
    float speedNorm = sqrtf(fishVX*fishVX + fishVY*fishVY) / FISH_MAX_SPEED;
    float swingAmp = 3.0f + speedNorm * 3.0f;
    float bobAmp = 2.0f + speedNorm * 2.0f;
    
    float ampScale = (gAnimator.transitionProgress < 1.0f) 
                     ? (0.4f + 0.6f * gAnimator.transitionProgress)
                     : 1.0f;
    float swing = sinf(swimPhase) * swingAmp * ampScale;
    float bob = sinf(swimPhase * 0.7f) * bobAmp * ampScale;
    
    if (gAnimator.currentState == ANIM_EATING && actionInProgress) {
      float chewPhase = actionTimer * 4.0f * 2.0f * PI;
      bob += sinf(chewPhase) * 3.0f;
    }

    const uint16_t* frame = getCurrentFrame();
    bool flip = isFlipped();

    int16_t x = static_cast<int16_t>(fishX + swing - CLOWNFISH_WIDTH / 2);
    int16_t y = static_cast<int16_t>(fishY + bob - CLOWNFISH_HEIGHT / 2);

    x = constrain(x, (int16_t)PLAY_AREA_X, (int16_t)(PLAY_AREA_X + PLAY_AREA_W - CLOWNFISH_WIDTH));
    y = constrain(y, (int16_t)PLAY_AREA_Y, (int16_t)(PLAY_AREA_Y + PLAY_AREA_H - CLOWNFISH_HEIGHT));

    drawSpriteOptimized(frame, CLOWNFISH_WIDTH, CLOWNFISH_HEIGHT, x, y, flip);

    // Commit current position for next frame
    prevFishDrawX = x;
    prevFishDrawY = y;
  }
}

void restorePetRegion() {
  if (prevFishDrawX >= 0) {
#ifdef DEBUG_GRAPHICS
    Serial.print("[FISH-RESTORE] ");
#endif
    int16_t margin = 1; // Reduced from 8 - only need small margin for swing/bob
    int16_t rx = max((int16_t)PLAY_AREA_X, (int16_t)(prevFishDrawX - margin));
    int16_t ry = max((int16_t)PLAY_AREA_Y, (int16_t)(prevFishDrawY - margin));
    int16_t rw = min((int16_t)(CLOWNFISH_WIDTH + margin * 2), (int16_t)(PLAY_AREA_X + PLAY_AREA_W - rx));
    int16_t rh = min((int16_t)(CLOWNFISH_HEIGHT + margin * 2), (int16_t)(PLAY_AREA_Y + PLAY_AREA_H - ry));
    if (rw > 0 && rh > 0) {
      restoreRegion(rx, ry, rw, rh);
    }
    
    // Invalidate after restore to prevent double-restore
    prevFishDrawX = -1;
    prevFishDrawY = -1;
  }
}

float getFishX() { return fishX; }
float getFishY() { return fishY; }
