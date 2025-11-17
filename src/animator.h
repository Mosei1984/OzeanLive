#pragma once
#include <Arduino.h>

enum AnimState {
  ANIM_IDLE = 0,
  ANIM_MOVING,
  ANIM_EATING,
  ANIM_PLAYING,
  ANIM_SLEEPING
};

struct AnimationClip {
  const uint16_t* const* frames;
  uint8_t frameCount;
  float fps;
  bool loop;
};

struct Animator {
  AnimState currentState;
  AnimState nextState;
  
  float timeInState;
  float transitionProgress;
  float transitionDuration;
  
  uint8_t currentFrame;
  float frameAccumulator;
  
  const AnimationClip* currentClip;
  const AnimationClip* nextClip;
};

extern Animator gAnimator;

void initAnimator();
void updateAnimator(float deltaTime);
void requestTransition(AnimState newState, float duration);
const uint16_t* getCurrentFrame();
bool isFlipped();
