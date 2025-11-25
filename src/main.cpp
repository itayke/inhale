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
// Debug Modes (temporary)
// ========================================
#define DIAGNOSTIC_MODE false
#define AUTO_PROGRESS_MODE true
#define AUTO_PROGRESS_INTERVAL_MS 10000

// ========================================
// Global Application State
// ========================================
AppMode currentMode = MODE_LIVE;
BreathData breathData;
unsigned long lastModeChangeTime = 0;

// ========================================
// Diagnostic Display
// ========================================
void drawDiagnostic(float pressureDelta) {
  static unsigned long lastUpdate = 0;

  unsigned long now = millis();
  if (now - lastUpdate < 100) return;  // 10 FPS
  lastUpdate = now;

  GFXcanvas16& canvas = getCanvas();
  canvas.fillScreen(ST77XX_BLACK);

  // Title
  canvas.setCursor(10, 10);
  canvas.setTextColor(ST77XX_YELLOW);
  canvas.setTextSize(1);
  canvas.println("DIAGNOSTIC MODE");

  // Pressure label
  canvas.setCursor(10, 35);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.print("Pressure Delta:");

  // Draw pressure value
  canvas.setCursor(10, 50);
  canvas.setTextSize(2);
  if (pressureDelta >= 0) {
    canvas.setTextColor(ST77XX_CYAN);
    canvas.print("+");
  } else {
    canvas.setTextColor(ST77XX_MAGENTA);
  }
  canvas.print(pressureDelta, 1);
  canvas.setTextSize(1);
  canvas.print(" Pa");

  // Draw bar
  int barY = 80;
  int barCenter = SCREEN_WIDTH / 2;
  int barWidth = constrain(abs(pressureDelta) * 2, 0, 50);

  canvas.drawFastHLine(10, barY, SCREEN_WIDTH - 20, ST77XX_GRAY);
  canvas.drawFastVLine(barCenter, barY - 5, 10, ST77XX_WHITE);

  if (pressureDelta > 0) {
    canvas.fillRect(barCenter, barY - 3, barWidth, 6, ST77XX_CYAN);
  } else {
    canvas.fillRect(barCenter - barWidth, barY - 3, barWidth, 6, ST77XX_MAGENTA);
  }

  // Temperature label and value
  canvas.setCursor(10, 100);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.print("Temperature:");

  canvas.setCursor(10, 115);
  canvas.setTextSize(2);
  canvas.setTextColor(ST77XX_ORANGE);
  float temp = getTemperature();
  canvas.print(temp, 1);
  canvas.setTextSize(1);
  canvas.print(" C");

  // Blit canvas to display
  displayBlit();
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
      lastModeChangeTime = millis();
      Serial.print("Mode changed to: ");
      Serial.println(currentMode);
      break;

    case GESTURE_PREV_MODE:
      currentMode = (AppMode)((currentMode + 3) % 4); // +3 mod 4 = -1
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
      // No gesture detected
      break;
  }

#if AUTO_PROGRESS_MODE
  // Auto-progress through modes every 10 seconds
  unsigned long now = millis();
  if (now - lastModeChangeTime >= AUTO_PROGRESS_INTERVAL_MS) {
    currentMode = (AppMode)((currentMode + 1) % 4);
    showModeTransition(currentMode);
    lastModeChangeTime = now;
    Serial.print("Auto-progress to mode: ");
    Serial.println(currentMode);
  }
#endif

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
