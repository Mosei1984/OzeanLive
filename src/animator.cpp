#include "animator.h"
#include "pet.h"

Animator gAnimator;

extern float fishVX;

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
}

void updateAnimator(float deltaTime) {
  gAnimator.timeInState += deltaTime;
  
  if (gAnimator.transitionProgress < 1.0f) {
    gAnimator.transitionProgress += deltaTime / gAnimator.transitionDuration;
    
    if (gAnimator.transitionProgress >= 1.0f) {
      gAnimator.transitionProgress = 1.0f;
      gAnimator.currentState = gAnimator.nextState;
      gAnimator.currentClip = gAnimator.nextClip;
      gAnimator.timeInState = 0.0f;
      gAnimator.currentFrame = 0;
      gAnimator.frameAccumulator = 0.0f;
    }
  }
  
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
  if (newState != gAnimator.currentState) {
    gAnimator.nextState = newState;
    gAnimator.transitionDuration = duration;
    gAnimator.transitionProgress = 0.0f;
  }
}

const uint16_t* getCurrentFrame() {
  if (gAnimator.currentClip == nullptr) {
    return nullptr;
  }
  
  if (gAnimator.currentFrame >= gAnimator.currentClip->frameCount) {
    return nullptr;
  }
  
  return gAnimator.currentClip->frames[gAnimator.currentFrame];
}

bool isFlipped() {
  return fishVX < 0.0f;
}
