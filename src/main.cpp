#include <Arduino.h>
#include "config.h"
#include "hardware/display.h"
#include "hardware/sensor.h"
#include "hardware/storage.h"
#include "detection/breath.h"
#include "detection/gesture.h"
#include "modes/live_mode.h"
#include "modes/guided_mode.h"
#include "modes/calib_mode.h"
#include "modes/stats_mode.h"

// ========================================
// Global Application State
// ========================================
AppMode currentMode = MODE_LIVE;
BreathData breathData;

// ========================================
// Helper Functions
// ========================================
void showModeTransition(AppMode mode) {
  Adafruit_ST7735& tft = getDisplay();
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(20, 60);
  tft.setTextSize(2);

  switch (mode) {
    case MODE_LIVE:
      tft.setTextColor(ST77XX_CYAN);
      tft.print("LIVE");
      break;
    case MODE_GUIDED:
      tft.setTextColor(ST77XX_GREEN);
      tft.print("GUIDED");
      guidedModeInit();
      break;
    case MODE_CALIBRATION:
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("CALIB");
      break;
    case MODE_STATS:
      tft.setTextColor(ST77XX_MAGENTA);
      tft.print("STATS");
      break;
  }

  delay(800);
}

// ========================================
// Setup
// ========================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Inhale - Breath Visualization Device");
  Serial.println("====================================");

  // Initialize hardware
  displayInit();
  sensorInit();
  storageInit();

  // Initialize detection systems
  breathInit(breathData);
  gestureInit();

  // Load calibration from storage
  loadCalibration(breathData.inhaleThreshold, breathData.exhaleThreshold);

  // Calibrate baseline
  calibrateBaseline();

  Serial.println("System ready!");
}

// ========================================
// Main Loop
// ========================================
void loop() {
  // Update sensor readings
  updatePressure();
  float pressureDelta = getPressureDelta();

  // Detect breath state
  detectBreath(breathData, pressureDelta);

  // Detect gestures and handle mode changes
  GestureType gesture = detectGestures(breathData);

  switch (gesture) {
    case GESTURE_NEXT_MODE:
      currentMode = (AppMode)((currentMode + 1) % 4);
      showModeTransition(currentMode);
      Serial.print("Mode changed to: ");
      Serial.println(currentMode);
      break;

    case GESTURE_PREV_MODE:
      currentMode = (AppMode)((currentMode + 3) % 4); // +3 mod 4 = -1
      showModeTransition(currentMode);
      Serial.print("Mode changed to: ");
      Serial.println(currentMode);
      break;

    case GESTURE_RESET_SESSION:
      resetSession(breathData);
      displayShowMessage("SESSION\n RESET", ST77XX_GREEN);
      delay(1000);
      break;

    case GESTURE_NONE:
    default:
      // No gesture detected
      break;
  }

  // Update display based on current mode
  switch (currentMode) {
    case MODE_LIVE:
      drawLiveMode(breathData, pressureDelta);
      break;

    case MODE_GUIDED:
      drawGuidedMode(breathData);
      break;

    case MODE_CALIBRATION:
      if (drawCalibrationMode(breathData, pressureDelta)) {
        // Calibration complete, return to live mode
        currentMode = MODE_LIVE;
        showModeTransition(currentMode);
      }
      break;

    case MODE_STATS:
      drawStatsMode(breathData, pressureDelta);
      break;
  }

  delay(MAIN_LOOP_DELAY_MS);
}
