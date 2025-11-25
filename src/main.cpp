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
// Diagnostic Mode (temporary)
// ========================================
#define DIAGNOSTIC_MODE false

// ========================================
// Global Application State
// ========================================
AppMode currentMode = MODE_LIVE;
BreathData breathData;

// ========================================
// Diagnostic Display
// ========================================
void drawDiagnosticStatic() {
  // Draw static elements once
  Adafruit_ST7735& tft = getDisplay();
  tft.fillScreen(ST77XX_BLACK);

  // Title
  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.println("DIAGNOSTIC MODE");

  // Pressure label
  tft.setCursor(10, 35);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("Pressure Delta:");

  // Bar baseline
  int barY = 80;
  tft.drawFastHLine(10, barY, SCREEN_WIDTH - 20, ST77XX_GRAY);
  tft.drawFastVLine(SCREEN_WIDTH / 2, barY - 5, 10, ST77XX_WHITE);

  // Temperature label
  tft.setCursor(10, 100);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("Temperature:");
}

void drawDiagnostic(float pressureDelta) {
  static unsigned long lastUpdate = 0;
  static bool firstRun = true;

  unsigned long now = millis();
  if (now - lastUpdate < 100) return;  // 10 FPS
  lastUpdate = now;

  Adafruit_ST7735& tft = getDisplay();

  // Draw static elements on first run
  if (firstRun) {
    drawDiagnosticStatic();
    firstRun = false;
  }

  // Clear only the dynamic pressure value area
  tft.fillRect(10, 50, 110, 20, ST77XX_BLACK);

  // Draw pressure value
  tft.setCursor(10, 50);
  tft.setTextSize(2);
  if (pressureDelta >= 0) {
    tft.setTextColor(ST77XX_CYAN);
    tft.print("+");
  } else {
    tft.setTextColor(ST77XX_MAGENTA);
  }
  tft.print(pressureDelta, 1);
  tft.setTextSize(1);
  tft.print(" Pa");

  // Clear and redraw bar
  int barY = 80;
  int barCenter = SCREEN_WIDTH / 2;
  int barWidth = constrain(abs(pressureDelta) * 2, 0, 50);

  // Clear bar area (but preserve center line)
  tft.fillRect(10, barY - 3, SCREEN_WIDTH - 20, 6, ST77XX_BLACK);
  tft.drawFastHLine(10, barY, SCREEN_WIDTH - 20, ST77XX_GRAY);
  tft.drawFastVLine(barCenter, barY - 5, 10, ST77XX_WHITE);

  // Draw new bar
  if (pressureDelta > 0) {
    tft.fillRect(barCenter, barY - 3, barWidth, 6, ST77XX_CYAN);
  } else {
    tft.fillRect(barCenter - barWidth, barY - 3, barWidth, 6, ST77XX_MAGENTA);
  }

  // Clear and draw temperature
  tft.fillRect(10, 115, 80, 16, ST77XX_BLACK);
  tft.setCursor(10, 115);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_ORANGE);
  float temp = getTemperature();
  tft.print(temp, 1);
  tft.setTextSize(1);
  tft.print(" C");
}

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

  Serial.println("System ready!");
}

// ========================================
// Main Loop
// ========================================
void loop() {
  // Update sensor readings
  updatePressure();
  float pressureDelta = getPressureDelta();

#if DIAGNOSTIC_MODE
  // Simple diagnostic display
  drawDiagnostic(pressureDelta);
  delay(MAIN_LOOP_DELAY_MS);
  return;
#endif

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
