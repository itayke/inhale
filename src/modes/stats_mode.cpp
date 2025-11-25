#include "stats_mode.h"
#include "../config.h"
#include "../hardware/display.h"
#include <Arduino.h>

void drawStatsMode(const BreathData& breathData, float pressureDelta) {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate < (1000 / STATS_UPDATE_HZ)) return;
  lastUpdate = now;

  GFXcanvas16& canvas = getCanvas();
  canvas.fillScreen(ST77XX_BLACK);

  // Title bar
  canvas.fillRect(0, 0, SCREEN_WIDTH, 18, rgb565(0, 80, 120));
  canvas.setCursor(20, 5);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.print("SESSION STATS");

  // Calculate statistics
  unsigned long sessionDuration = (now - breathData.sessionStartTime) / 1000;
  float breathsPerMinute = 0;
  if (sessionDuration > 0) {
    breathsPerMinute = (breathData.breathCount * 60.0) / sessionDuration;
  }

  int yPos = 25;

  // Breath count - large display
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_CYAN);
  canvas.setCursor(5, yPos);
  canvas.print("Total Breaths");

  canvas.setTextSize(3);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setCursor(40, yPos + 12);
  canvas.print(breathData.breathCount);

  yPos += 40;

  // Breathing rate
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_GREEN);
  canvas.setCursor(5, yPos);
  canvas.print("Breaths/Min");

  canvas.setTextSize(2);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setCursor(45, yPos + 12);
  if (breathData.breathCount > 0) {
    canvas.print(breathsPerMinute, 1);
  } else {
    canvas.print("--");
  }

  // Draw breathing rate bar
  int rateBarWidth = constrain(breathsPerMinute * 5, 0, SCREEN_WIDTH - 20);
  canvas.fillRect(10, yPos + 30, rateBarWidth, 4, ST77XX_GREEN);
  canvas.drawRect(10, yPos + 30, SCREEN_WIDTH - 20, 4, ST77XX_GRAY);

  yPos += 42;

  // Average breath duration
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_MAGENTA);
  canvas.setCursor(5, yPos);
  canvas.print("Avg Duration");

  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setCursor(80, yPos);
  if (breathData.breathCount > 0) {
    canvas.print(breathData.averageBreathDuration / 1000.0, 1);
    canvas.print("s");
  } else {
    canvas.print("--");
  }

  yPos += 15;

  // Session time
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_YELLOW);
  canvas.setCursor(5, yPos);
  canvas.print("Session Time");

  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setCursor(80, yPos);
  int minutes = sessionDuration / 60;
  int seconds = sessionDuration % 60;
  if (minutes < 10) canvas.print("0");
  canvas.print(minutes);
  canvas.print(":");
  if (seconds < 10) canvas.print("0");
  canvas.print(seconds);

  yPos += 15;

  // Current pressure (live indicator)
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_ORANGE);
  canvas.setCursor(5, yPos);
  canvas.print("Pressure");

  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setCursor(65, yPos);
  canvas.print(pressureDelta, 1);
  canvas.print("Pa");

  // Mini live breath indicator at bottom
  int miniBarY = SCREEN_HEIGHT - 12;
  canvas.setTextSize(1);
  canvas.setTextColor(ST77XX_GRAY);
  canvas.setCursor(5, miniBarY);

  switch (breathData.currentState) {
    case BREATH_INHALE:
      canvas.setTextColor(ST77XX_CYAN);
      canvas.print("INHALING");
      break;
    case BREATH_EXHALE:
      canvas.setTextColor(ST77XX_MAGENTA);
      canvas.print("EXHALING");
      break;
    case BREATH_HOLD:
      canvas.setTextColor(ST77XX_YELLOW);
      canvas.print("HOLDING");
      break;
    default:
      canvas.setTextColor(ST77XX_GRAY);
      canvas.print("IDLE");
      break;
  }

  // Mini pressure bar
  int miniBarWidth = map(constrain(pressureDelta, -25, 25), -25, 25, 0, 50);
  int miniBarX = SCREEN_WIDTH - 55;
  canvas.fillRect(miniBarX + 25, miniBarY + 2, 1, 6, ST77XX_WHITE);

  if (miniBarWidth > 25) {
    canvas.fillRect(miniBarX + 25, miniBarY + 2, miniBarWidth - 25, 6,
                 breathData.currentState == BREATH_EXHALE ? ST77XX_MAGENTA : ST77XX_GRAY);
  } else {
    canvas.fillRect(miniBarX + miniBarWidth, miniBarY + 2, 25 - miniBarWidth, 6,
                 breathData.currentState == BREATH_INHALE ? ST77XX_CYAN : ST77XX_GRAY);
  }

  // Blit canvas to display
  displayBlit();
}
