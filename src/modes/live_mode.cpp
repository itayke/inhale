#include "live_mode.h"
#include "../config.h"
#include "../BreathData.h"
#include "../Display.h"
#include <Arduino.h>

void drawLiveMode(float pressureDelta) {
  static unsigned long lastUpdate = 0;
  static float wavePhase = 0;
  static float targetWaveHeight = SCREEN_HEIGHT / 2;
  static float currentWaveHeight = SCREEN_HEIGHT / 2;

  unsigned long now = millis();
  if (now - lastUpdate < (1000 / WAVE_UPDATE_FPS)) return;
  float dt = (now - lastUpdate) / 1000.0;
  lastUpdate = now;

  Canvas& canvas = display.getCanvas();

  // Animate wave phase (horizontal scroll)
  wavePhase += 2.5 * dt;
  if (wavePhase > TWO_PI) wavePhase -= TWO_PI;

  // Calculate target wave height based on normalized breath (-1 to +1)
  // Inhale (negative) = wave drops, Exhale (positive) = wave rises
  float normalized = breathData.getNormalizedBreath();
  float maxDisplacement = 50.0f;  // Max pixels from center
  targetWaveHeight = (SCREEN_HEIGHT / 2) + (normalized * maxDisplacement);

  // Smooth interpolation for organic movement
  currentWaveHeight += (targetWaveHeight - currentWaveHeight) * 0.1;

  // Determine wave colors based on breath state
  uint16_t waterColor, foamColor;
  switch (breathData.getState()) {
    case BREATH_INHALE:
      waterColor = Display::rgb565(0, 100, 200);    // Deep blue
      foamColor = Display::rgb565(100, 150, 255);   // Light blue
      break;
    case BREATH_EXHALE:
      waterColor = Display::rgb565(0, 150, 200);    // Cyan
      foamColor = Display::rgb565(150, 255, 255);   // Bright cyan
      break;
    case BREATH_HOLD:
      waterColor = Display::rgb565(100, 0, 150);    // Purple
      foamColor = Display::rgb565(200, 100, 255);   // Light purple
      break;
    default:
      waterColor = Display::rgb565(0, 120, 180);    // Medium blue
      foamColor = Display::rgb565(120, 180, 255);   // Sky blue
      break;
  }

  // Draw sky gradient to canvas
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    uint8_t brightness = map(y, 0, SCREEN_HEIGHT, 60, 20);
    uint16_t skyColor = Display::rgb565(brightness, brightness, brightness + 30);
    canvas.drawFastHLine(0, y, SCREEN_WIDTH, skyColor);
  }

  // Draw multi-layer wave for depth to canvas
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    // Primary wave (main surface)
    float wave1 = sin(x * 0.15 + wavePhase) * 8;
    float wave2 = sin(x * 0.08 + wavePhase * 1.3) * 5;
    float wave3 = sin(x * 0.22 - wavePhase * 0.7) * 3;

    int waveY = (int)(currentWaveHeight + wave1 + wave2 + wave3);

    // Clamp wave height
    waveY = constrain(waveY, 10, SCREEN_HEIGHT - 10);

    // Draw foam/crest (lighter color at wave peak)
    int foamHeight = abs((int)wave1) / 2 + 2;
    canvas.drawFastVLine(x, waveY - foamHeight, foamHeight, foamColor);

    // Draw water body below wave
    canvas.drawFastVLine(x, waveY, SCREEN_HEIGHT - waveY, waterColor);
  }

  // Draw HUD overlay to canvas
  canvas.setCursor(4, 4);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);

  // Mode indicator
  canvas.print("LIVE");

  // Breath count
  canvas.setCursor(4, SCREEN_HEIGHT - 10);
  canvas.print("Breaths: ");
  canvas.print(breathData.getBreathCount());

  // Breath state indicator (top right)
  const char* stateText;
  switch (breathData.getState()) {
    case BREATH_INHALE: stateText = "IN "; break;
    case BREATH_EXHALE: stateText = "OUT"; break;
    case BREATH_HOLD:   stateText = "HLD"; break;
    default:            stateText = "..."; break;
  }
  canvas.setCursor(SCREEN_WIDTH - 22, 4);
  canvas.print(stateText);

  // Blit canvas to display
  display.blit();
}
