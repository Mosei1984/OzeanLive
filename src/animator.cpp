#include "animator.h"
#include "pet.h"
#include "sprites/clownfish.h"

Animator gAnimator;

extern float fishVX;

// Helper: Map AnimState to AnimationClip
static const AnimationClip* clipForState(AnimState s) {
  switch (s) {
    case ANIM_IDLE:     return &CLIP_IDLE;
    case ANIM_MOVING:   return &CLIP_MOVING;
    case ANIM_EATING:   return &CLIP_EATING;
    case ANIM_PLAYING:  return &CLIP_PLAYING;
    case ANIM_SLEEPING: return &CLIP_SLEEPING;
    case ANIM_POOPING:  return &CLIP_POOPING;
    default:            return &CLIP_IDLE;
  }
}

void initAnimator() {
  gAnimator.currentState = ANIM_IDLE;
  gAnimator.nextState = ANIM_IDLE;
  gAnimator.timeInState = 0.0f;
  gAnimator.transitionProgress = 1.0f;
  gAnimator.transitionDuration = 0.0f;
  gAnimator.currentFrame = 0;
  gAnimator.frameAccumulator = 0.0f;
  gAnimator.currentClip = nullptr;
  gAnimator.nextClip = nullptr;
  gAnimator.currentClip = &CLIP_IDLE;
}

void updateAnimator(float deltaTime) {
  gAnimator.timeInState += deltaTime;
  
  if (gAnimator.transitionProgress < 1.0f) {
    // Guard against zero/negative duration
    if (gAnimator.transitionDuration <= 0.0f) {
      gAnimator.transitionProgress = 1.0f;
    } else {
      gAnimator.transitionProgress += deltaTime / gAnimator.transitionDuration;
    }
    
    if (gAnimator.transitionProgress >= 1.0f) {
      gAnimator.transitionProgress = 1.0f;
      gAnimator.currentState = gAnimator.nextState;
      gAnimator.currentClip = gAnimator.nextClip;
      gAnimator.timeInState = 0.0f;
      gAnimator.currentFrame = 0;
      gAnimator.frameAccumulator = 0.0f;
    }
  }
  
  if (!gAnimator.currentClip || gAnimator.currentClip->fps <= 0.0f) return;
  
  if (gAnimator.currentClip != nullptr && gAnimator.transitionProgress >= 1.0f) {
    float frameDuration = 1.0f / gAnimator.currentClip->fps;
    gAnimator.frameAccumulator += deltaTime;
    
    while (gAnimator.frameAccumulator >= frameDuration) {
      gAnimator.frameAccumulator -= frameDuration;
      gAnimator.currentFrame++;
      
      if (gAnimator.currentFrame >= gAnimator.currentClip->frameCount) {
        if (gAnimator.currentClip->loop) {
          gAnimator.currentFrame = 0;
        } else {
          gAnimator.currentFrame = gAnimator.currentClip->frameCount - 1;
        }
      }
    }
  }
}

void requestTransition(AnimState newState, float duration) {
  // Don't transition if already in that state
  if (newState == gAnimator.currentState) return;
  
  // Don't re-request if already transitioning to that state
  if (gAnimator.transitionProgress < 1.0f && gAnimator.nextState == newState) return;
  
  gAnimator.nextState = newState;
  gAnimator.nextClip = clipForState(newState);
  
  // Validate clip before transitioning
  if (!gAnimator.nextClip || gAnimator.nextClip->frameCount <= 0) {
#ifdef DEBUG_GAME_LOGIC
    Serial.println("[ANIMATOR] WARNING: Rejecting transition to invalid clip");
#endif
    return;
  }
  
  gAnimator.transitionDuration = max(0.001f, duration); // Ensure positive duration
  gAnimator.transitionProgress = 0.0f;
}

const uint16_t* getCurrentFrame() {
  const AnimationClip* clip = gAnimator.currentClip ? gAnimator.currentClip : &CLIP_IDLE;
  
  // Validate clip
  if (!clip || clip->frameCount <= 0) {
#ifdef DEBUG_GAME_LOGIC
    Serial.println("[ANIMATOR] WARNING: Invalid clip, falling back to IDLE");
#endif
    gAnimator.currentState = ANIM_IDLE;
    gAnimator.currentClip = &CLIP_IDLE;
    gAnimator.currentFrame = 0;
    return CLIP_IDLE.frames[0];
  }
  
  // Validate frame index
  if (gAnimator.currentFrame < 0 || gAnimator.currentFrame >= clip->frameCount) {
#ifdef DEBUG_GAME_LOGIC
    Serial.print("[ANIMATOR] WARNING: Frame index out of bounds: ");
    Serial.println(gAnimator.currentFrame);
#endif
    gAnimator.currentFrame = 0;
  }
  
  // Get frame pointer
  const uint16_t* frame = clip->frames[gAnimator.currentFrame];
  
  // Validate frame pointer
  if (!frame) {
#ifdef DEBUG_GAME_LOGIC
    Serial.print("[ANIMATOR] WARNING: Null frame at index ");
    Serial.println(gAnimator.currentFrame);
#endif
    // Try to find first valid frame in clip
    for (int i = 0; i < clip->frameCount; ++i) {
      if (clip->frames[i]) {
        gAnimator.currentFrame = i;
        return clip->frames[i];
      }
    }
    
    // Last resort: use idle frame 0
    Serial.println("[ANIMATOR] ERROR: No valid frames in clip, using IDLE");
    gAnimator.currentState = ANIM_IDLE;
    gAnimator.currentClip = &CLIP_IDLE;
    gAnimator.currentFrame = 0;
    return CLIP_IDLE.frames[0];
  }
  
  return frame;
}

bool isFlipped() {
  return fishVX < 0.0f;
}
