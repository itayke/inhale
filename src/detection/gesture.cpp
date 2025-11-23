#include "gesture.h"
#include "hardware/display.h"
#include <Arduino.h>

static unsigned long lastPuffTime = 0;
static int puffCount = 0;
static unsigned long lastGestureTime = 0;
static bool longBreathGestureTriggered = false;

void gestureInit() {
  lastPuffTime = 0;
  puffCount = 0;
  lastGestureTime = 0;
  longBreathGestureTriggered = false;
}

GestureType detectGestures(const BreathData& breathData) {
  unsigned long now = millis();
  unsigned long breathDuration = now - breathData.breathStartTime;

  // Prevent gesture spam - require debounce time between gestures
  if (now - lastGestureTime < GESTURE_DEBOUNCE_MS) {
    return GESTURE_NONE;
  }

  // Reset puff count if too much time has passed
  if (now - lastPuffTime > GESTURE_PUFF_WINDOW_MS) {
    puffCount = 0;
  }

  // Reset long breath gesture flag when returning to idle
  if (breathData.currentState == BREATH_IDLE) {
    longBreathGestureTriggered = false;
  }

  // ===== LONG EXHALE GESTURE: Next Mode =====
  if (breathData.currentState == BREATH_EXHALE &&
      !longBreathGestureTriggered &&
      breathDuration > GESTURE_LONG_BREATH_MS) {

    longBreathGestureTriggered = true;
    lastGestureTime = now;

    Serial.println("GESTURE: Long exhale (NEXT)");
    return GESTURE_NEXT_MODE;
  }

  // ===== LONG INHALE GESTURE: Previous Mode =====
  if (breathData.currentState == BREATH_INHALE &&
      !longBreathGestureTriggered &&
      breathDuration > GESTURE_LONG_BREATH_MS) {

    longBreathGestureTriggered = true;
    lastGestureTime = now;

    Serial.println("GESTURE: Long inhale (PREVIOUS)");
    return GESTURE_PREV_MODE;
  }

  // ===== BREATH HOLD GESTURE: Reset Session =====
  if (breathData.currentState == BREATH_HOLD &&
      !longBreathGestureTriggered &&
      breathDuration > GESTURE_HOLD_MENU_MS) {

    longBreathGestureTriggered = true;
    lastGestureTime = now;

    Serial.println("GESTURE: Breath hold (RESET SESSION)");
    return GESTURE_RESET_SESSION;
  }

  return GESTURE_NONE;
}
