#pragma once
#include "gfx.h"

// Zeichnet Wasser, Sandboden und Korallen/Steine aufs TFT
void drawEnvironment();

// Zeichnet Wasser, Sandboden und Korallen/Steine ins Canvas
void drawEnvironmentToCanvas(GFXcanvas16* canvas);

// Get anemone position for sleeping fish
void getAnemonePosition(int16_t& x, int16_t& y);
