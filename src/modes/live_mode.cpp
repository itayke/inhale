#include "live_mode.h"
#include "../config.h"
#include "../hardware/display.h"
#include <Arduino.h>

void drawLiveMode(const BreathData& breathData, float pressureDelta) {
  static unsigned long lastUpdate = 0;
  static float wavePhase = 0;
  static float targetWaveHeight = SCREEN_HEIGHT / 2;
  static float currentWaveHeight = SCREEN_HEIGHT / 2;

  unsigned long now = millis();
  if (now - lastUpdate < (1000 / WAVE_UPDATE_FPS)) return;
  float dt = (now - lastUpdate) / 1000.0;
  lastUpdate = now;

  Adafruit_ST7735& tft = getDisplay();

  // Animate wave phase (horizontal scroll)
  wavePhase += 2.5 * dt;
  if (wavePhase > TWO_PI) wavePhase -= TWO_PI;

  // Calculate target wave height based on pressure
  // Inhale (negative pressure) = wave drops
  // Exhale (positive pressure) = wave rises
  float pressureInfluence = constrain(pressureDelta * 1.5, -50, 50);
  targetWaveHeight = (SCREEN_HEIGHT / 2) + pressureInfluence;

  // Smooth interpolation for organic movement
  currentWaveHeight += (targetWaveHeight - currentWaveHeight) * 0.1;

  // Determine wave colors based on breath state
  uint16_t waterColor, foamColor;
  switch (breathData.currentState) {
    case BREATH_INHALE:
      waterColor = tft.color565(0, 100, 200);    // Deep blue
      foamColor = tft.color565(100, 150, 255);   // Light blue
      break;
    case BREATH_EXHALE:
      waterColor = tft.color565(0, 150, 200);    // Cyan
      foamColor = tft.color565(150, 255, 255);   // Bright cyan
      break;
    case BREATH_HOLD:
      waterColor = tft.color565(100, 0, 150);    // Purple
      foamColor = tft.color565(200, 100, 255);   // Light purple
      break;
    default:
      waterColor = tft.color565(0, 120, 180);    // Medium blue
      foamColor = tft.color565(120, 180, 255);   // Sky blue
      break;
  }

  // Clear screen - sky gradient
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    uint8_t brightness = map(y, 0, SCREEN_HEIGHT, 60, 20);
    uint16_t skyColor = tft.color565(brightness, brightness, brightness + 30);
    tft.drawFastHLine(0, y, SCREEN_WIDTH, skyColor);
  }

  // Draw multi-layer wave for depth
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
    tft.drawFastVLine(x, waveY - foamHeight, foamHeight, foamColor);

    // Draw water body below wave
    tft.drawFastVLine(x, waveY, SCREEN_HEIGHT - waveY, waterColor);
  }

  // Draw HUD overlay
  tft.setCursor(4, 4);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  // Mode indicator
  tft.print("LIVE");

  // Breath count
  tft.setCursor(4, SCREEN_HEIGHT - 10);
  tft.print("Breaths: ");
  tft.print(breathData.breathCount);

  // Breath state indicator (top right)
  const char* stateText;
  switch (breathData.currentState) {
    case BREATH_INHALE: stateText = "IN "; break;
    case BREATH_EXHALE: stateText = "OUT"; break;
    case BREATH_HOLD:   stateText = "HLD"; break;
    default:            stateText = "..."; break;
  }
  tft.setCursor(SCREEN_WIDTH - 22, 4);
  tft.print(stateText);
}
