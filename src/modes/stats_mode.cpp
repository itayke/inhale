#include "stats_mode.h"
#include "../config.h"
#include "../hardware/display.h"
#include <Arduino.h>

void drawStatsMode(const BreathData& breathData, float pressureDelta) {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate < (1000 / STATS_UPDATE_HZ)) return;
  lastUpdate = now;

  Adafruit_ST7735& tft = getDisplay();
  tft.fillScreen(ST77XX_BLACK);

  // Title bar
  tft.fillRect(0, 0, SCREEN_WIDTH, 18, tft.color565(0, 80, 120));
  tft.setCursor(20, 5);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("SESSION STATS");

  // Calculate statistics
  unsigned long sessionDuration = (now - breathData.sessionStartTime) / 1000;
  float breathsPerMinute = 0;
  if (sessionDuration > 0) {
    breathsPerMinute = (breathData.breathCount * 60.0) / sessionDuration;
  }

  int yPos = 25;

  // Breath count - large display
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, yPos);
  tft.print("Total Breaths");

  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(40, yPos + 12);
  tft.print(breathData.breathCount);

  yPos += 40;

  // Breathing rate
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(5, yPos);
  tft.print("Breaths/Min");

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(45, yPos + 12);
  if (breathData.breathCount > 0) {
    tft.print(breathsPerMinute, 1);
  } else {
    tft.print("--");
  }

  // Draw breathing rate bar
  int rateBarWidth = constrain(breathsPerMinute * 5, 0, SCREEN_WIDTH - 20);
  tft.fillRect(10, yPos + 30, rateBarWidth, 4, ST77XX_GREEN);
  tft.drawRect(10, yPos + 30, SCREEN_WIDTH - 20, 4, ST77XX_GRAY);

  yPos += 42;

  // Average breath duration
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setCursor(5, yPos);
  tft.print("Avg Duration");

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(80, yPos);
  if (breathData.breathCount > 0) {
    tft.print(breathData.averageBreathDuration / 1000.0, 1);
    tft.print("s");
  } else {
    tft.print("--");
  }

  yPos += 15;

  // Session time
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(5, yPos);
  tft.print("Session Time");

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(80, yPos);
  int minutes = sessionDuration / 60;
  int seconds = sessionDuration % 60;
  if (minutes < 10) tft.print("0");
  tft.print(minutes);
  tft.print(":");
  if (seconds < 10) tft.print("0");
  tft.print(seconds);

  yPos += 15;

  // Current pressure (live indicator)
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_ORANGE);
  tft.setCursor(5, yPos);
  tft.print("Pressure");

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(65, yPos);
  tft.print(pressureDelta, 1);
  tft.print("Pa");

  // Mini live breath indicator at bottom
  int miniBarY = SCREEN_HEIGHT - 12;
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GRAY);
  tft.setCursor(5, miniBarY);

  switch (breathData.currentState) {
    case BREATH_INHALE:
      tft.setTextColor(ST77XX_CYAN);
      tft.print("INHALING");
      break;
    case BREATH_EXHALE:
      tft.setTextColor(ST77XX_MAGENTA);
      tft.print("EXHALING");
      break;
    case BREATH_HOLD:
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("HOLDING");
      break;
    default:
      tft.setTextColor(ST77XX_GRAY);
      tft.print("IDLE");
      break;
  }

  // Mini pressure bar
  int miniBarWidth = map(constrain(pressureDelta, -25, 25), -25, 25, 0, 50);
  int miniBarX = SCREEN_WIDTH - 55;
  tft.fillRect(miniBarX + 25, miniBarY + 2, 1, 6, ST77XX_WHITE);

  if (miniBarWidth > 25) {
    tft.fillRect(miniBarX + 25, miniBarY + 2, miniBarWidth - 25, 6,
                 breathData.currentState == BREATH_EXHALE ? ST77XX_MAGENTA : ST77XX_GRAY);
  } else {
    tft.fillRect(miniBarX + miniBarWidth, miniBarY + 2, 25 - miniBarWidth, 6,
                 breathData.currentState == BREATH_INHALE ? ST77XX_CYAN : ST77XX_GRAY);
  }
}
