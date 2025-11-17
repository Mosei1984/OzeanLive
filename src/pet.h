#pragma once
#include "gfx.h"

// Struktur für einfache Tamagotchi-Statuswerte
struct PetStats {
  int hunger;    // 0 = satt, 100 = sehr hungrig
  int fun;       // 0 = gelangweilt, 100 = sehr happy
  int energy;    // 0 = müde, 100 = fit
  unsigned long lastUpdateMs;
};

extern PetStats pet;

// Initialisiert Pet-Werte
void initPet();

// Aktualisiert Pet-Werte abhängig von Zeit & Aktionen
void updatePetStats();

// Zeichnet Fisch mit Animation & Wegpunkt-Navigation
void drawPetAnimated(float dtSec);

// Aktionen, die über das Menü ausgelöst werden
enum PetAction {
  ACTION_NONE,
  ACTION_FEED,
  ACTION_PLAY,
  ACTION_REST,
  ACTION_CLEAN
};

extern PetAction pendingAction;

// globale Animationsphase (für Sinusbewegungen)
extern float animPhase;

// Velocity für Animator isFlipped()
extern float fishVX;
extern float fishVY;
