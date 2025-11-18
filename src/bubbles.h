#pragma once
#include "gfx.h"

// Initialize bubbles
void initBubbles();

// Restore bubble regions (call during RESTORE phase)
void restoreBubblesRegion();

// Update and draw all bubbles (call during DRAW phase)
void updateAndDrawBubbles(float dtSec);

// Update fish origin used for fish bubble spawns (-1 disables fish spawns)
void bubblesSetFishOrigin(int16_t x, int16_t y);
