#include "calib_mode.h"
#include "../config.h"
#include "../hardware/display.h"
#include "../hardware/storage.h"
#include <Arduino.h>

static float minPressureSeen = 0;
static float maxPressureSeen = 0;
static bool calibrationActive = false;
static unsigned long lastGestureTime = 0;

bool drawCalibrationMode(BreathData& breathData, float pressureDelta) {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate < (1000 / CALIBRATION_UPDATE_HZ)) return false;
  lastUpdate = now;

  GFXcanvas16& canvas = getCanvas();

  // Track min/max pressure deltas during calibration
  if (calibrationActive) {
    if (pressureDelta < minPressureSeen) minPressureSeen = pressureDelta;
    if (pressureDelta > maxPressureSeen) maxPressureSeen = pressureDelta;
  } else {
    // Start calibration
    calibrationActive = true;
    minPressureSeen = pressureDelta;
    maxPressureSeen = pressureDelta;
  }

  // Auto-adjust thresholds to 70% of observed extremes
  float autoInhaleThresh = minPressureSeen * 0.7;
  float autoExhaleThresh = maxPressureSeen * 0.7;

  // Clear canvas
  canvas.fillScreen(ST77XX_BLACK);

  // Title
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_YELLOW);
  canvas.setCursor(25, 5);
  canvas.print("CALIBRATION");

  // Instructions
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setCursor(5, 20);
  canvas.print("Take a few deep");
  canvas.setCursor(5, 30);
  canvas.print("breaths...");

  // Draw real-time pressure bar
  int centerY = SCREEN_HEIGHT / 2;
  int barX = 10;
  int barWidth = 30;
  int maxBarHeight = 50;

  // Draw center line
  canvas.drawFastHLine(barX, centerY, SCREEN_WIDTH - barX - 10, ST77XX_GRAY);

  // Draw pressure bar
  int barHeight = constrain(pressureDelta * 2, -maxBarHeight, maxBarHeight);
  uint16_t barColor = (barHeight < 0) ? ST77XX_CYAN : ST77XX_MAGENTA;

  if (barHeight > 0) {
    canvas.fillRect(barX, centerY - barHeight, barWidth, barHeight, barColor);
  } else {
    canvas.fillRect(barX, centerY, barWidth, -barHeight, barColor);
  }

  // Draw min/max markers
  int minY = centerY - (int)(minPressureSeen * 2);
  int maxY = centerY - (int)(maxPressureSeen * 2);

  canvas.drawFastHLine(barX, minY, barWidth, ST77XX_CYAN);
  canvas.drawFastHLine(barX, maxY, barWidth, ST77XX_MAGENTA);

  // Draw threshold lines
  int inhaleThreshY = centerY - (int)(autoInhaleThresh * 2);
  int exhaleThreshY = centerY - (int)(autoExhaleThresh * 2);

  canvas.drawFastHLine(barX, inhaleThreshY, SCREEN_WIDTH - barX - 10, ST77XX_BLUE);
  canvas.drawFastHLine(barX, exhaleThreshY, SCREEN_WIDTH - barX - 10, ST77XX_RED);

  // Display current values
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_WHITE);

  canvas.setCursor(50, centerY - 60);
  canvas.print("Current: ");
  canvas.print(pressureDelta, 1);
  canvas.print("Pa");

  canvas.setCursor(50, centerY - 40);
  canvas.setTextColor(ST77XX_CYAN);
  canvas.print("Inhale min: ");
  canvas.print(minPressureSeen, 1);

  canvas.setCursor(50, centerY - 30);
  canvas.setTextColor(ST77XX_BLUE);
  canvas.print("Threshold: ");
  canvas.print(autoInhaleThresh, 1);

  canvas.setCursor(50, centerY + 25);
  canvas.setTextColor(ST77XX_MAGENTA);
  canvas.print("Exhale max: ");
  canvas.print(maxPressureSeen, 1);

  canvas.setCursor(50, centerY + 35);
  canvas.setTextColor(ST77XX_RED);
  canvas.print("Threshold: ");
  canvas.print(autoExhaleThresh, 1);

  // Instructions to save
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_GREEN);
  canvas.setCursor(5, SCREEN_HEIGHT - 25);
  canvas.print("Hold breath 5s");
  canvas.setCursor(5, SCREEN_HEIGHT - 15);
  canvas.print("to save & exit");

  // Blit canvas to display
  displayBlit();

  // Check if user wants to save (breath hold detected)
  if (breathData.currentState == BREATH_HOLD &&
      (now - breathData.breathStartTime) > 5000 &&
      (now - lastGestureTime) > 1000) {

    // Save the calibrated thresholds
    breathData.inhaleThreshold = autoInhaleThresh;
    breathData.exhaleThreshold = autoExhaleThresh;
    saveCalibration(breathData.inhaleThreshold, breathData.exhaleThreshold);

    lastGestureTime = now;
    calibrationActive = false;

    // Show confirmation on canvas
    canvas.fillScreen(ST77XX_BLACK);
    canvas.setCursor(20, 60);
    canvas.setTextColor(ST77XX_GREEN);
    canvas.setTextSize(1);
    canvas.println("SAVED!");
    canvas.setCursor(10, 75);
    canvas.print("Inhale: ");
    canvas.print(breathData.inhaleThreshold, 1);
    canvas.setCursor(10, 85);
    canvas.print("Exhale: ");
    canvas.print(breathData.exhaleThreshold, 1);
    displayBlit();
    delay(2000);

    // Signal to return to live mode
    return true;
  }

  return false;
}
