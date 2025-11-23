#include "guided_mode.h"
#include "../config.h"
#include "../hardware/display.h"
#include <Arduino.h>

static GuidedPhase guidedPhase = GUIDE_INHALE;
static unsigned long guidedPhaseStart = 0;

void guidedModeInit() {
  guidedPhase = GUIDE_INHALE;
  guidedPhaseStart = 0;
}

void drawGuidedMode(BreathData& breathData) {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate < (1000 / GUIDED_UPDATE_FPS)) return;
  lastUpdate = now;

  Adafruit_ST7735& tft = getDisplay();

  // Initialize guided phase start time if needed
  if (guidedPhaseStart == 0) {
    guidedPhaseStart = now;
  }

  unsigned long phaseElapsed = now - guidedPhaseStart;
  unsigned long phaseDuration;
  const char* phaseText;
  uint16_t phaseColor;

  // Determine current phase and its properties
  switch (guidedPhase) {
    case GUIDE_INHALE:
      phaseDuration = INHALE_DURATION;
      phaseText = "BREATHE IN";
      phaseColor = tft.color565(100, 200, 255); // Light blue
      break;
    case GUIDE_HOLD:
      phaseDuration = HOLD_DURATION;
      phaseText = "HOLD";
      phaseColor = tft.color565(200, 100, 255); // Purple
      break;
    case GUIDE_EXHALE:
      phaseDuration = EXHALE_DURATION;
      phaseText = "BREATHE OUT";
      phaseColor = tft.color565(100, 255, 200); // Cyan-green
      break;
  }

  // Check if phase is complete and transition
  if (phaseElapsed >= phaseDuration) {
    guidedPhaseStart = now;
    phaseElapsed = 0;

    switch (guidedPhase) {
      case GUIDE_INHALE:
        guidedPhase = GUIDE_HOLD;
        break;
      case GUIDE_HOLD:
        guidedPhase = GUIDE_EXHALE;
        break;
      case GUIDE_EXHALE:
        guidedPhase = GUIDE_INHALE;
        breathData.breathCount++; // Count completed breath cycle
        break;
    }
  }

  // Calculate progress through current phase (0.0 to 1.0)
  float progress = (float)phaseElapsed / phaseDuration;

  // Calculate circle radius based on phase and progress
  int minRadius = 15;
  int maxRadius = 55;
  int radius;

  switch (guidedPhase) {
    case GUIDE_INHALE:
      // Expand during inhale
      radius = minRadius + (maxRadius - minRadius) * progress;
      break;
    case GUIDE_HOLD:
      // Stay at max during hold
      radius = maxRadius;
      break;
    case GUIDE_EXHALE:
      // Contract during exhale
      radius = maxRadius - (maxRadius - minRadius) * progress;
      break;
  }

  // Clear screen
  tft.fillScreen(ST77XX_BLACK);

  // Draw pulsing guide circle (multiple rings for depth)
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  // Outer glow
  tft.drawCircle(centerX, centerY, radius + 3, tft.color565(50, 50, 100));
  tft.drawCircle(centerX, centerY, radius + 2, tft.color565(80, 80, 120));

  // Main circle
  tft.fillCircle(centerX, centerY, radius, phaseColor);

  // Inner highlight
  int highlightRadius = radius / 2;
  uint16_t highlightColor = tft.color565(255, 255, 255);
  tft.fillCircle(centerX - radius/4, centerY - radius/4, highlightRadius, highlightColor);

  // Draw phase instruction text
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  // Center the text
  int textWidth = strlen(phaseText) * 6; // Approximate char width
  tft.setCursor((SCREEN_WIDTH - textWidth) / 2, 15);
  tft.print(phaseText);

  // Draw progress bar at bottom
  int barWidth = (SCREEN_WIDTH - 20) * progress;
  tft.fillRect(10, SCREEN_HEIGHT - 15, barWidth, 5, phaseColor);
  tft.drawRect(10, SCREEN_HEIGHT - 15, SCREEN_WIDTH - 20, 5, ST77XX_WHITE);

  // Draw time remaining
  int secondsRemaining = (phaseDuration - phaseElapsed) / 1000 + 1;
  tft.setCursor(SCREEN_WIDTH / 2 - 6, SCREEN_HEIGHT - 25);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(secondsRemaining);

  // Draw mode indicator and breath count
  tft.setTextSize(1);
  tft.setCursor(4, 4);
  tft.print("GUIDED");

  tft.setCursor(SCREEN_WIDTH - 25, 4);
  tft.print(breathData.breathCount);
}
