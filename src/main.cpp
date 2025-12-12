#include <Arduino.h>
#include "config.h"
#include "BreathData.h"
#include "Display.h"
#include "PressureSensor.h"
#include "Storage.h"
#include "modes/live_mode.h"
#include "modes/diagnostic_mode.h"

// ========================================
// Global Application State
// ========================================
AppMode currentMode = MODE_LIVE;
BreathData breathData;
Display display;
PressureSensor pressureSensor;
Storage storage;
unsigned long lastModeChangeTime = 0;

// ========================================
// Helper Functions
// ========================================
void showModeTransition(AppMode mode) {
  Adafruit_ST7735& tft = display.getTft();
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
  pressureSensor.init();
  display.init();
  storage.init();

  // Initialize breath detection
  breathData.init();

  // Load calibration from storage
  storage.loadCalibration(breathData.inhaleThreshold, breathData.exhaleThreshold);

  // Calibrate baseline
  pressureSensor.calibrateBaseline();

  // Initialize mode change timer
  lastModeChangeTime = millis();

  Serial.println("System ready!");
}

// ========================================
// Main Loop
// ========================================
void loop() {
  // Update sensor readings
  pressureSensor.update();
  float pressureDelta = pressureSensor.getDelta();

  // Detect breath state
  breathData.detect(pressureDelta);

  // Update display based on current mode
  switch (currentMode) {
    case MODE_LIVE:
      drawLiveMode(pressureDelta);
      break;

    case MODE_DIAGNOSTIC:
      drawDiagnosticMode(pressureDelta);
      break;
  }

  delay(MAIN_LOOP_DELAY_MS);
}
