#ifndef BREATH_H
#define BREATH_H

#include "../config.h"

// Breath detection state
struct BreathData {
  BreathState currentState;
  unsigned long breathStartTime;
  unsigned long lastBreathTime;
  int breathCount;
  float averageBreathDuration;
  unsigned long sessionStartTime;
  float inhaleThreshold;
  float exhaleThreshold;
};

// Initialize breath detection
void breathInit(BreathData& data);

// Update breath detection based on current pressure
void detectBreath(BreathData& data, float pressureDelta);

// Get current breath state
BreathState getBreathState(const BreathData& data);

// Get breath count
int getBreathCount(const BreathData& data);

// Reset session statistics
void resetSession(BreathData& data);

#endif // BREATH_H
