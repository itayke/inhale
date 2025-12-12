#include "Storage.h"
#include "config.h"
#include <Preferences.h>
#include <Arduino.h>

static Preferences preferences;

void Storage::init() {
  preferences.begin("inhale", false);
  Serial.println("NVS storage initialized");
}

void Storage::loadCalibration(float& inhaleThreshold, float& exhaleThreshold) {
  inhaleThreshold = preferences.getFloat("inhaleThresh", DEFAULT_INHALE_THRESHOLD);
  exhaleThreshold = preferences.getFloat("exhaleThresh", DEFAULT_EXHALE_THRESHOLD);

  Serial.print("Loaded calibration - Inhale: ");
  Serial.print(inhaleThreshold);
  Serial.print(" Pa, Exhale: ");
  Serial.print(exhaleThreshold);
  Serial.println(" Pa");
}

void Storage::saveCalibration(float inhaleThreshold, float exhaleThreshold) {
  preferences.putFloat("inhaleThresh", inhaleThreshold);
  preferences.putFloat("exhaleThresh", exhaleThreshold);

  Serial.println("Calibration saved to NVS");
}
