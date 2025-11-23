#ifndef GUIDED_MODE_H
#define GUIDED_MODE_H

#include "../detection/breath.h"

// Initialize guided mode (call when entering mode)
void guidedModeInit();

// Draw guided breathing visualization
void drawGuidedMode(BreathData& breathData);

#endif // GUIDED_MODE_H
