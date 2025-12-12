#include <Arduino.h>
#include "config.h"
#include "hardware/display.h"
#include "hardware/sensor.h"
#include "hardware/storage.h"
#include "detection/breath.h"
#include "detection/gesture.h"
#include "modes/live_mode.h"
#include "modes/diagnostic_mode.h"

// ========================================
// Global Application State
// ========================================
AppMode currentMode = MODE_LIVE;
BreathData breathData;
unsigned long lastModeChangeTime = 0;

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
    case MODE_DIAGNOSTIC:
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("DIAG");
      break;
  }

  delay(500);
}

// ========================================
// Setup
// ========================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Inhale - Breath Visualization Device");
  Serial.println("====================================");

  // Initialize hardware (sensor first to avoid I2C conflicts)
  sensorInit();
  displayInit();
  storageInit();

  // Initialize detection systems
  breathInit(breathData);
  gestureInit();

  // Load calibration from storage
  loadCalibration(breathData.inhaleThreshold, breathData.exhaleThreshold);

  // Calibrate baseline
  calibrateBaseline();

  // Initialize mode change timer
  lastModeChangeTime = millis();

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
    case GESTURE_PREV_MODE:
      // Toggle between 2 modes
      currentMode = (AppMode) ((currentMode == MODE_LIVE) ? MODE_DIAGNOSTIC : MODE_LIVE);
      showModeTransition(currentMode);
      lastModeChangeTime = millis();
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
      break;
  }

  // Update display based on current mode
  switch (currentMode) {
    case MODE_LIVE:
      drawLiveMode(breathData, pressureDelta);
      break;

    case MODE_DIAGNOSTIC:
      drawDiagnosticMode(pressureDelta);
      break;
  }

  delay(MAIN_LOOP_DELAY_MS);
}
