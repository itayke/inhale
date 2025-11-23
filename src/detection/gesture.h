#ifndef GESTURE_H
#define GESTURE_H

#include "../config.h"
#include "breath.h"

enum GestureType {
  GESTURE_NONE,
  GESTURE_NEXT_MODE,
  GESTURE_PREV_MODE,
  GESTURE_RESET_SESSION
};

// Initialize gesture detection
void gestureInit();

// Detect gestures based on breath state
GestureType detectGestures(const BreathData& breathData);

#endif // GESTURE_H
