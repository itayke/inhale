#include "BreathData.h"
#include <Arduino.h>

void BreathData::init() {
  currentState = BREATH_IDLE;
  breathStartTime = 0;
  lastBreathTime = 0;
  breathCount = 0;
  averageBreathDuration = 0;
  sessionStartTime = millis();
  inhaleThreshold = DEFAULT_INHALE_THRESHOLD;
  exhaleThreshold = DEFAULT_EXHALE_THRESHOLD;
}

void BreathData::detect(float pressureDelta) {
  BreathState previousState = currentState;
  unsigned long now = millis();

  // Detect breath state based on pressure delta
  if (pressureDelta < inhaleThreshold) {
    currentState = BREATH_INHALE;
  } else if (pressureDelta > exhaleThreshold) {
    currentState = BREATH_EXHALE;
  } else {
    // Check for breath hold (stable pressure for >3 seconds)
    if (now - lastBreathTime > BREATH_HOLD_TIMEOUT_MS &&
        abs(pressureDelta) < BREATH_HOLD_STABILITY_PA) {
      currentState = BREATH_HOLD;
    } else {
      currentState = BREATH_IDLE;
    }
  }

  // Detect breath transitions for counting
  if (previousState != currentState) {
    breathStartTime = now;

    // Count a full breath cycle when transitioning from exhale to inhale
    if (previousState == BREATH_EXHALE && currentState == BREATH_INHALE) {
      breathCount++;

      // Update average breath duration
      unsigned long cycleDuration = now - lastBreathTime;
      if (breathCount == 1) {
        averageBreathDuration = cycleDuration;
      } else {
        averageBreathDuration =
          (averageBreathDuration * (breathCount - 1) + cycleDuration) / breathCount;
      }
    }

    lastBreathTime = now;
  }
}

void BreathData::resetSession() {
  breathCount = 0;
  averageBreathDuration = 0;
  sessionStartTime = millis();
}
