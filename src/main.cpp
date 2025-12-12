#include <Arduino.h>
#include "config.h"
#include "hardware/display.h"
#include "hardware/sensor.h"
#include "hardware/storage.h"
#include "detection/breath.h"
#include "detection/gesture.h"
#include "modes/live_mode.h"

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
  canvas.setCursor(10, 5);
  canvas.setTextColor(ST77XX_YELLOW);
  canvas.setTextSize(1);
  canvas.println("DIAGNOSTIC MODE");

  // Pressure delta
  canvas.setCursor(10, 22);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.print("Delta: ");
  if (pressureDelta >= 0) {
    canvas.setTextColor(ST77XX_CYAN);
    canvas.print("+");
  } else {
    canvas.setTextColor(ST77XX_MAGENTA);
  }
  canvas.print(pressureDelta, 1);
  canvas.print(" Pa");

  // Draw bar
  int barY = 40;
  int barCenter = SCREEN_WIDTH / 2;
  int barWidth = constrain(abs(pressureDelta) * 2, 0, 50);

  canvas.drawFastHLine(10, barY, SCREEN_WIDTH - 20, ST77XX_GRAY);
  canvas.drawFastVLine(barCenter, barY - 5, 10, ST77XX_WHITE);

  if (pressureDelta > 0) {
    canvas.fillRect(barCenter, barY - 3, barWidth, 6, ST77XX_CYAN);
  } else {
    canvas.fillRect(barCenter - barWidth, barY - 3, barWidth, 6, ST77XX_MAGENTA);
  }

  // Absolute pressure in inHg
  canvas.setCursor(10, 58);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.print("Pressure:");

  canvas.setCursor(10, 72);
  canvas.setTextSize(2);
  canvas.setTextColor(ST77XX_GREEN);
  float pressureInHg = getAbsolutePressure() / 3386.39;  // Pa to inHg
  canvas.print(pressureInHg, 2);
  canvas.setTextSize(1);
  canvas.print(" inHg");

  // Temperature
  canvas.setCursor(10, 100);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.print("Temp:");

  canvas.setCursor(10, 114);
  canvas.setTextSize(2);
  canvas.setTextColor(ST77XX_ORANGE);
  float temp = getTemperature();
  canvas.print(temp, 1);
  canvas.setTextSize(1);
  canvas.print("C ");
  canvas.setTextColor(ST77XX_YELLOW);
  float tempF = temp * 9.0 / 5.0 + 32.0;
  canvas.print(tempF, 1);
  canvas.print("F");

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
      drawDiagnostic(pressureDelta);
      break;
  }

  delay(MAIN_LOOP_DELAY_MS);
}
