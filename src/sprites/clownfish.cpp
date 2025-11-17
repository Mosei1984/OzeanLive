#include "clownfish.h"
#include "../animator.h"
#include "generated/clownfish_frames.h"

// =============================================================================
// IDLE Animation - 4 Frames (leichtes Atmen, 7 FPS)
// =============================================================================
const uint16_t* const clownfish_idle_frames[] PROGMEM = {
  clownfish_idle_f0,
  clownfish_idle_f1,
  clownfish_idle_f0,
  clownfish_idle_f1
};

// =============================================================================
// MOVING Animation - 6 Frames (Schwimm-Zyklus, 10 FPS)
// =============================================================================
const uint16_t* const clownfish_moving_frames[] PROGMEM = {
  clownfish_moving_f0,
  clownfish_moving_f0,
  clownfish_moving_f0,
  clownfish_moving_f0,
  clownfish_moving_f0,
  clownfish_moving_f0
};

// =============================================================================
// EATING Animation - 5 Frames (Mund auf/zu, 10 FPS, non-loop)
// =============================================================================
const uint16_t* const clownfish_eating_frames[] PROGMEM = {
  clownfish_eating_f0,
  clownfish_eating_f0,
  clownfish_eating_f0,
  clownfish_eating_f0,
  clownfish_eating_f0
};

// =============================================================================
// PLAYING Animation - 6 Frames (fröhliche Bewegung, 12 FPS)
// =============================================================================
const uint16_t* const clownfish_playing_frames[] PROGMEM = {
  clownfish_playing_f0,
  clownfish_playing_f0,
  clownfish_playing_f0,
  clownfish_playing_f0,
  clownfish_playing_f0,
  clownfish_playing_f0
};

// =============================================================================
// SLEEPING Animation - 3 Frames (langsames Atmen, 5 FPS)
// =============================================================================
const uint16_t* const clownfish_sleeping_frames[] PROGMEM = {
  clownfish_sleeping_f0,
  clownfish_sleeping_f0,
  clownfish_sleeping_f0
};

// =============================================================================
// Animation Clips
// =============================================================================
const AnimationClip CLIP_IDLE = {
  clownfish_idle_frames,
  4,      // frameCount
  7.0f,   // fps
  true    // loop
};

const AnimationClip CLIP_MOVING = {
  clownfish_moving_frames,
  6,      // frameCount
  10.0f,  // fps
  true    // loop
};

const AnimationClip CLIP_EATING = {
  clownfish_eating_frames,
  5,      // frameCount
  10.0f,  // fps
  false   // non-loop (one-shot)
};

const AnimationClip CLIP_PLAYING = {
  clownfish_playing_frames,
  6,      // frameCount
  12.0f,  // fps
  true    // loop
};

const AnimationClip CLIP_SLEEPING = {
  clownfish_sleeping_frames,
  3,      // frameCount
  5.0f,   // fps
  true    // loop
};
