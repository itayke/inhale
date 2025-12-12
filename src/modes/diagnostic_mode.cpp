#include "diagnostic_mode.h"
#include "../config.h"
#include "../hardware/display.h"
#include "../hardware/sensor.h"

void drawDiagnosticMode(float pressureDelta) {
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
