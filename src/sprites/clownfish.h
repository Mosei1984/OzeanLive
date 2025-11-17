#pragma once
#include <Arduino.h>
#include "../animator.h"

// Clownfish - 30x25

const uint16_t CLOWNFISH_WIDTH  = 30;
const uint16_t CLOWNFISH_HEIGHT = 25;

// Base sprite (backward compatibility)
extern const uint16_t clownfishBitmap[] PROGMEM;

// Animation Clips für alle States
extern const AnimationClip CLIP_IDLE;
extern const AnimationClip CLIP_MOVING;
extern const AnimationClip CLIP_EATING;
extern const AnimationClip CLIP_PLAYING;
extern const AnimationClip CLIP_SLEEPING;
extern const AnimationClip CLIP_POOPING;
