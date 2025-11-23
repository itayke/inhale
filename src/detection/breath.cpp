#include "breath.h"
#include <Arduino.h>

void breathInit(BreathData& data) {
  data.currentState = BREATH_IDLE;
  data.breathStartTime = 0;
  data.lastBreathTime = 0;
  data.breathCount = 0;
  data.averageBreathDuration = 0;
  data.sessionStartTime = millis();
  data.inhaleThreshold = DEFAULT_INHALE_THRESHOLD;
  data.exhaleThreshold = DEFAULT_EXHALE_THRESHOLD;
}

void detectBreath(BreathData& data, float pressureDelta) {
  BreathState previousState = data.currentState;
  unsigned long now = millis();

  // Detect breath state based on pressure delta
  if (pressureDelta < data.inhaleThreshold) {
    data.currentState = BREATH_INHALE;
  } else if (pressureDelta > data.exhaleThreshold) {
    data.currentState = BREATH_EXHALE;
  } else {
    // Check for breath hold (stable pressure for >3 seconds)
    if (now - data.lastBreathTime > BREATH_HOLD_TIMEOUT_MS &&
        abs(pressureDelta) < BREATH_HOLD_STABILITY_PA) {
      data.currentState = BREATH_HOLD;
    } else {
      data.currentState = BREATH_IDLE;
    }
  }

  // Detect breath transitions for counting
  if (previousState != data.currentState) {
    data.breathStartTime = now;

    // Count a full breath cycle when transitioning from exhale to inhale
    if (previousState == BREATH_EXHALE && data.currentState == BREATH_INHALE) {
      data.breathCount++;

      // Update average breath duration
      unsigned long cycleDuration = now - data.lastBreathTime;
      if (data.breathCount == 1) {
        data.averageBreathDuration = cycleDuration;
      } else {
        data.averageBreathDuration =
          (data.averageBreathDuration * (data.breathCount - 1) + cycleDuration) / data.breathCount;
      }
    }

    data.lastBreathTime = now;
  }
}

BreathState getBreathState(const BreathData& data) {
  return data.currentState;
}

int getBreathCount(const BreathData& data) {
  return data.breathCount;
}

void resetSession(BreathData& data) {
  data.breathCount = 0;
  data.averageBreathDuration = 0;
  data.sessionStartTime = millis();
}
