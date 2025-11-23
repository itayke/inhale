#ifndef CALIB_MODE_H
#define CALIB_MODE_H

#include "../detection/breath.h"

// Draw calibration mode UI
// Returns true if calibration is complete and should return to live mode
bool drawCalibrationMode(BreathData& breathData, float pressureDelta);

#endif // CALIB_MODE_H
