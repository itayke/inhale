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
  normalizedBreath = 0;
  minPressureDelta = -10.0f;
  maxPressureDelta = 10.0f;
}

void BreathData::detect(float pressureDelta) {
  BreathState previousState = currentState;
  unsigned long now = millis();

  // Auto-expand calibration bounds
  if (pressureDelta < minPressureDelta) {
    minPressureDelta = pressureDelta;
  }
  if (pressureDelta > maxPressureDelta) {
    maxPressureDelta = pressureDelta;
  }

  // Calculate normalized breath (-1 to +1)
  if (pressureDelta < 0 && minPressureDelta < -0.1f) {
    // Inhale: map [minPressureDelta..0] → [-1..0]
    normalizedBreath = pressureDelta / (-minPressureDelta);
  } else if (pressureDelta > 0 && maxPressureDelta > 0.1f) {
    // Exhale: map [0..maxPressureDelta] → [0..1]
    normalizedBreath = pressureDelta / maxPressureDelta;
  } else {
    normalizedBreath = 0;
  }

  // Clamp to valid range
  if (normalizedBreath < -1.0f) normalizedBreath = -1.0f;
  if (normalizedBreath > 1.0f) normalizedBreath = 1.0f;

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

void BreathData::resetCalibration() {
  minPressureDelta = -10.0f;
  maxPressureDelta = 10.0f;
}
