// Simulator implementation of Sensor
#include "Sensor.h"
#include "Display.h"
#include "config.h"

extern SerialMock Serial;

void Sensor::init() {
  Serial.println("Initializing simulated sensor...");
  Serial.println("Use mouse Y position to simulate breath pressure");
  Serial.println("  - Move mouse UP = Inhale (negative pressure)");
  Serial.println("  - Move mouse DOWN = Exhale (positive pressure)");
  Serial.println("  - Center = Neutral");
  Serial.println("Simulated sensor initialized!");

  baselinePressure = 101325.0f;  // Standard atmospheric pressure (Pa)
  currentPressure = 101325.0f;
  currentTemperature = 22.0f;    // Room temperature (C)
}

void Sensor::calibrateBaseline() {
  Serial.println("Calibrating baseline (simulated)...");
  display.showMessage("Calibrating...\n  Move mouse\n  to center", ST77XX_CYAN);
  SDL_Delay(1000);
  display.clear();
  Serial.println("Baseline calibrated (simulated)");
}

void Sensor::setMouseY(int mouseY, int windowHeight) {
  _mouseY = mouseY;
  _windowHeight = windowHeight;
}

void Sensor::update() {
  // Map mouse Y to pressure delta
  // Center of window = 0 Pa
  // Top of window = -50 Pa (inhale)
  // Bottom of window = +50 Pa (exhale)
  int centerY = _windowHeight / 2;
  float normalizedY = (float)(_mouseY - centerY) / (float)centerY;  // -1 to +1

  // Scale to pressure range (±50 Pa is typical breath range)
  pressureDelta = normalizedY * 50.0f;

  // Update absolute pressure for display
  currentPressure = baselinePressure + pressureDelta;
}
