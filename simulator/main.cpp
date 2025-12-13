// Simulator main - uses shared code from src/
#include "Platform.h"
#include "config.h"
#include "Display.h"
#include "Sensor.h"
#include "Storage.h"
#include "BreathData.h"
#include "modes/live_mode.h"
#include "modes/diagnostic_mode.h"

// Global instances
SerialMock Serial;
Display display;
Sensor pressureSensor;
Storage storage;
BreathData breathData;

// Application state
AppMode currentMode = MODE_LIVE;

void setup() {
  Serial.begin(115200);
  Serial.println("Inhale Simulator");
  Serial.println("================");
  Serial.println("Controls:");
  Serial.println("  Mouse Y: Breath pressure (up=inhale, down=exhale)");
  Serial.println("  Space: Toggle mode (Live/Diagnostic)");
  Serial.println("  ESC/Q: Quit");
  Serial.println("");

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    Serial.print("SDL_Init failed: ");
    Serial.println(SDL_GetError());
    return;
  }

  // Initialize components (sensor first to match ESP32 order)
  pressureSensor.init();
  display.init();
  storage.init();
  breathData.init();

  // Load calibration
  storage.loadCalibration(breathData.inhaleThreshold, breathData.exhaleThreshold);

  // Calibrate baseline
  pressureSensor.calibrateBaseline();

  Serial.println("Simulator ready!");
}

void loop() {
  // Update sensor with current mouse position
  pressureSensor.update();
  float pressureDelta = pressureSensor.getDelta();

  // Detect breath state
  breathData.detect(pressureDelta);

  // Render based on mode
  switch (currentMode) {
    case MODE_LIVE:
      drawLiveMode(pressureDelta);
      break;
    case MODE_DIAGNOSTIC:
      drawDiagnosticMode(pressureDelta);
      break;
  }
}

int main(int argc, char* argv[]) {
  setup();

  bool running = true;
  SDL_Event event;
  uint32_t lastLoopTime = 0;

  while (running) {
    // Handle events
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
              // Toggle mode
              currentMode = (currentMode == MODE_LIVE) ? MODE_DIAGNOSTIC : MODE_LIVE;
              Serial.print("Mode: ");
              Serial.println(currentMode == MODE_LIVE ? "LIVE" : "DIAGNOSTIC");
              break;
          }
          break;

        case SDL_MOUSEMOTION:
          // Update sensor with mouse Y position
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

    // Small delay to prevent CPU spinning
    SDL_Delay(1);
  }

  SDL_Quit();
  return 0;
}
