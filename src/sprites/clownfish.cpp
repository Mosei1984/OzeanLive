#include "clownfish.h"
#include "../animator.h"
#include "clownfish_frames.h"

// =============================================================================
// IDLE Animation - 2 Frames (leichtes Atmen, 4 FPS)
// =============================================================================
const uint16_t* const clownfish_idle_frames[] PROGMEM = {
  clownfish_idle_f0,
  clownfish_idle_f1
};

// =============================================================================
// MOVING Animation - 6 Frames (idle1 -> swim -> idle2 -> swim -> idle1 -> swim, 10 FPS)
// =============================================================================
const uint16_t* const clownfish_moving_frames[] PROGMEM = {
  clownfish_idle_f0,      // idle 1
  clownfish_moving_f0,    // swim
  clownfish_idle_f1,      // idle 2
  clownfish_moving_f0,    // swim
  clownfish_idle_f0,      // idle 1
  clownfish_moving_f0     // swim
};

// =============================================================================
// EATING Animation - 1 Frame (statischer Fisch mit offenem Mund)
// =============================================================================
const uint16_t* const clownfish_eating_frames[] PROGMEM = {
  clownfish_eating_f0
};

// =============================================================================
// PLAYING Animation - 1 Frame (statischer Fisch mit Herzen)
// =============================================================================
const uint16_t* const clownfish_playing_frames[] PROGMEM = {
  clownfish_playing_f0
};

// =============================================================================
// SLEEPING Animation - 1 Frame (statischer Fisch mit ZZZ)
// =============================================================================
const uint16_t* const clownfish_sleeping_frames[] PROGMEM = {
  clownfish_sleeping_f0
};

// =============================================================================
// POOPING Animation - 1 Frame (statischer Fisch beim Kaka machen)
// =============================================================================
const uint16_t* const clownfish_pooping_frames[] PROGMEM = {
  clownfish_poopBitmap
};

// =============================================================================
// Animation Clips
// =============================================================================
const AnimationClip CLIP_IDLE = {
  clownfish_idle_frames,
  2,      // frameCount
  4.0f,   // fps (langsames Atmen)
  true    // loop
};

const AnimationClip CLIP_MOVING = {
  clownfish_moving_frames,
  6,      // frameCount
  10.0f,  // fps (smooth swimming cycle)
  true    // loop
};

const AnimationClip CLIP_EATING = {
  clownfish_eating_frames,
  1,      // frameCount (static frame mit offenem Mund)
  1.0f,   // fps (irrelevant bei 1 Frame)
  true    // loop (bleibt stehen während ACTION_FEED aktiv ist)
};

const AnimationClip CLIP_PLAYING = {
  clownfish_playing_frames,
  1,      // frameCount (static frame mit Herzen)
  1.0f,   // fps (irrelevant bei 1 Frame)
  true    // loop (bleibt stehen während ACTION_PLAY aktiv ist)
};

const AnimationClip CLIP_SLEEPING = {
  clownfish_sleeping_frames,
  1,      // frameCount (static frame mit ZZZ)
  1.0f,   // fps (irrelevant bei 1 Frame)
  true    // loop (bleibt stehen während ACTION_REST aktiv ist)
};

const AnimationClip CLIP_POOPING = {
  clownfish_pooping_frames,
  1,      // frameCount (static frame beim Kaka machen)
  0.333f, // fps (0.333 FPS = 3 Sekunden pro Frame)
  false   // no loop (einmalige Aktion)
};
