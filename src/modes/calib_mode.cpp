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

  Adafruit_ST7735& tft = getDisplay();

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

  // Clear screen
  tft.fillScreen(ST77XX_BLACK);

  // Title
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(25, 5);
  tft.print("CALIBRATION");

  // Instructions
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 20);
  tft.print("Take a few deep");
  tft.setCursor(5, 30);
  tft.print("breaths...");

  // Draw real-time pressure bar
  int centerY = SCREEN_HEIGHT / 2;
  int barX = 10;
  int barWidth = 30;
  int maxBarHeight = 50;

  // Draw center line
  tft.drawFastHLine(barX, centerY, SCREEN_WIDTH - barX - 10, ST77XX_GRAY);

  // Draw pressure bar
  int barHeight = constrain(pressureDelta * 2, -maxBarHeight, maxBarHeight);
  uint16_t barColor = (barHeight < 0) ? ST77XX_CYAN : ST77XX_MAGENTA;

  if (barHeight > 0) {
    tft.fillRect(barX, centerY - barHeight, barWidth, barHeight, barColor);
  } else {
    tft.fillRect(barX, centerY, barWidth, -barHeight, barColor);
  }

  // Draw min/max markers
  int minY = centerY - (int)(minPressureSeen * 2);
  int maxY = centerY - (int)(maxPressureSeen * 2);

  tft.drawFastHLine(barX, minY, barWidth, ST77XX_CYAN);
  tft.drawFastHLine(barX, maxY, barWidth, ST77XX_MAGENTA);

  // Draw threshold lines
  int inhaleThreshY = centerY - (int)(autoInhaleThresh * 2);
  int exhaleThreshY = centerY - (int)(autoExhaleThresh * 2);

  tft.drawFastHLine(barX, inhaleThreshY, SCREEN_WIDTH - barX - 10, ST77XX_BLUE);
  tft.drawFastHLine(barX, exhaleThreshY, SCREEN_WIDTH - barX - 10, ST77XX_RED);

  // Display current values
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(50, centerY - 60);
  tft.print("Current: ");
  tft.print(pressureDelta, 1);
  tft.print("Pa");

  tft.setCursor(50, centerY - 40);
  tft.setTextColor(ST77XX_CYAN);
  tft.print("Inhale min: ");
  tft.print(minPressureSeen, 1);

  tft.setCursor(50, centerY - 30);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("Threshold: ");
  tft.print(autoInhaleThresh, 1);

  tft.setCursor(50, centerY + 25);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print("Exhale max: ");
  tft.print(maxPressureSeen, 1);

  tft.setCursor(50, centerY + 35);
  tft.setTextColor(ST77XX_RED);
  tft.print("Threshold: ");
  tft.print(autoExhaleThresh, 1);

  // Instructions to save
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(5, SCREEN_HEIGHT - 25);
  tft.print("Hold breath 5s");
  tft.setCursor(5, SCREEN_HEIGHT - 15);
  tft.print("to save & exit");

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

    // Show confirmation
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(20, 60);
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(1);
    tft.println("SAVED!");
    tft.setCursor(10, 75);
    tft.print("Inhale: ");
    tft.print(breathData.inhaleThreshold, 1);
    tft.setCursor(10, 85);
    tft.print("Exhale: ");
    tft.print(breathData.exhaleThreshold, 1);
    delay(2000);

    // Signal to return to live mode
    return true;
  }

  return false;
}
