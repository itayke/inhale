#ifndef LIVE_MODE_H
#define LIVE_MODE_H

#include "../detection/breath.h"

// Draw live wave visualization
void drawLiveMode(const BreathData& breathData, float pressureDelta);

#endif // LIVE_MODE_H
