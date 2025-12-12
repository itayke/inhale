#ifndef BREATH_DATA_H
#define BREATH_DATA_H

#include "config.h"

class BreathData {
public:
  // Initialize breath detection
  void init();

  // Update breath detection based on current pressure
  void detect(float pressureDelta);

  // Reset session statistics
  void resetSession();

  // Getters
  BreathState getState() const { return currentState; }
  int getBreathCount() const { return breathCount; }
  float getAverageBreathDuration() const { return averageBreathDuration; }
  unsigned long getSessionStartTime() const { return sessionStartTime; }
  unsigned long getBreathStartTime() const { return breathStartTime; }

  // Calibration thresholds
  float inhaleThreshold;
  float exhaleThreshold;

private:
  BreathState currentState = BREATH_IDLE;
  unsigned long breathStartTime = 0;
  unsigned long lastBreathTime = 0;
  int breathCount = 0;
  float averageBreathDuration = 0;
  unsigned long sessionStartTime = 0;
};

// Global breath data instance (defined in main.cpp)
extern BreathData breathData;

#endif // BREATH_DATA_H
