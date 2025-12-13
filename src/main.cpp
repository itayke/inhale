#ifdef SIMULATOR
  #include "Platform.h"
#else
  #include <Arduino.h>
#endif

#include "config.h"
#include "BreathData.h"
#include "Display.h"
#include "Sensor.h"
#include "Storage.h"
#include "modes/live_mode.h"
#include "modes/diagnostic_mode.h"

// ========================================
// Global Application State
// ========================================
#ifdef SIMULATOR
SerialMock Serial;
#endif

AppMode currentMode = MODE_LIVE;
BreathData breathData;
Display display;
Sensor pressureSensor;
Storage storage;

// ========================================
// Setup
// ========================================
void setup() {
  Serial.begin(115200);

#ifdef SIMULATOR
  Serial.println("Inhale Simulator");
  Serial.println("================");
  Serial.println("Controls:");
  Serial.println("  Mouse Y: Breath pressure (up=exhale, down=inhale)");
  Serial.println("  Space: Toggle mode (Live/Diagnostic)");
  Serial.println("  ESC/Q: Quit");
  Serial.println("");

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    Serial.print("SDL_Init failed: ");
    Serial.println(SDL_GetError());
    return;
  }
#else
  delay(1000);
  Serial.println("Inhale - Breath Visualization Device");
  Serial.println("====================================");
#endif

  // Initialize components (sensor first to avoid I2C conflicts)
  pressureSensor.init();
  display.init();
  storage.init();
  breathData.init();

  // Load calibration from storage
  storage.loadCalibration(breathData.inhaleThreshold, breathData.exhaleThreshold);

  // Calibrate baseline
  pressureSensor.calibrateBaseline();

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

#ifndef SIMULATOR
  delay(MAIN_LOOP_DELAY_MS);
#endif
}

// ========================================
// Simulator Entry Point
// ========================================
#ifdef SIMULATOR
int main(int argc, char* argv[]) {
  setup();

  bool running = true;
  SDL_Event event;
  uint32_t lastLoopTime = 0;

  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = false;
          break;

        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_q:
              running = false;
              break;
            case SDLK_SPACE:
              currentMode = (currentMode == MODE_LIVE) ? MODE_DIAGNOSTIC : MODE_LIVE;
              Serial.print("Mode: ");
              Serial.println(currentMode == MODE_LIVE ? "LIVE" : "DIAGNOSTIC");
              break;
          }
          break;

        case SDL_MOUSEMOTION:
          pressureSensor.setMouseY(event.motion.y, SCREEN_HEIGHT * 4);
          break;
      }
    }

    // Run main loop at ~50Hz
    uint32_t now = millis();
    if (now - lastLoopTime >= MAIN_LOOP_DELAY_MS) {
      lastLoopTime = now;
      loop();
    }

    SDL_Delay(1);
  }

  SDL_Quit();
  return 0;
}
#endif
