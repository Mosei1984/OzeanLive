#include "clownfish.h"
#include "../animator.h"
#include "clownfish_frames.h"

// =============================================================================
// IDLE Animation - 1 Frame (nur idle_f0)
// =============================================================================
const uint16_t* const clownfish_idle_frames[] PROGMEM = {
  clownfish_idle_f0
};

// =============================================================================
// MOVING Animation - 5 Frames (idle_f0 -> moving_f0 -> moving_f1 -> moving_f0 -> idle_f0)
// =============================================================================
const uint16_t* const clownfish_moving_frames[] PROGMEM = {
  clownfish_idle_f0,      // start idle
  clownfish_moving_f0,    // swim 1
  clownfish_moving_f1,    // swim 2
  clownfish_moving_f0,    // swim 1 again
  clownfish_idle_f0       // back to idle
};

// =============================================================================
// EATING Animation - 1 Frame (statischer Fisch mit offenem Mund)
// =============================================================================
const uint16_t* const clownfish_eating_frames[] PROGMEM = {
  clownfish_eating_f0
};

// =============================================================================
// PLAYING Animation - 3 Frames (Bälle 2s, dann Herzen 1s)
// =============================================================================
const uint16_t* const clownfish_playing_frames[] PROGMEM = {
  clownfish_playing_f1,   // fish with colorful balls (1s)
  clownfish_playing_f1,   // fish with colorful balls (2s total)
  clownfish_playing_f0    // fish with hearts (3s total)
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
  1,      // frameCount (only idle_f0)
  1.0f,   // fps (irrelevant for 1 frame)
  true    // loop
};

const AnimationClip CLIP_MOVING = {
  clownfish_moving_frames,
  5,      // frameCount (idle -> moving_f0 -> moving_f1 -> moving_f0 -> idle)
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
  3,      // frameCount (balls 2s, hearts 1s)
  1.0f,   // fps (1 FPS = 1 second per frame, total 3s)
  false   // no loop (play once: balls, balls, hearts)
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
