#include "sensor.h"
#include "display.h"
#include "../config.h"
#include <Wire.h>
#include <Adafruit_BMP280.h>

static Adafruit_BMP280 bmp;
static float baselinePressure = 0;
static float currentPressure = 0;
static float pressureDelta = 0;
static float currentTemperature = 0;

void sensorInit() {
  Serial.println("Initializing BMP280 sensor...");

  // Initialize I2C
  Wire.begin(BMP_SDA, BMP_SCL);

  // Initialize BMP280
  if (!bmp.begin(0x76)) { // Try address 0x76 first
    if (!bmp.begin(0x77)) { // Try alternate address
      Serial.println("ERROR: Could not find BMP280 sensor!");
      while (1) delay(100);
    }
  }

  // Configure BMP280 for high precision
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X16,  // Pressure oversampling
                  Adafruit_BMP280::SAMPLING_X2,   // Temperature oversampling
                  Adafruit_BMP280::FILTER_X16,    // Filtering
                  Adafruit_BMP280::STANDBY_MS_1); // Standby time

  Serial.println("BMP280 initialized");
}

void calibrateBaseline() {
  Serial.println("Calibrating baseline pressure...");

  displayShowMessage("Calibrating...\n  Breathe\n  normally", ST77XX_CYAN);

  // Take average of 50 readings
  float sum = 0;
  for (int i = 0; i < 50; i++) {
    sum += bmp.readPressure();
    delay(20);
  }

  baselinePressure = sum / 50.0;

  Serial.print("Baseline pressure: ");
  Serial.print(baselinePressure);
  Serial.println(" Pa");

  displayClear();
}

void updatePressure() {
  currentPressure = bmp.readPressure();
  pressureDelta = currentPressure - baselinePressure;
  currentTemperature = bmp.readTemperature();
}

float getPressureDelta() {
  return pressureDelta;
}

float getTemperature() {
  return currentTemperature;
}

float getAbsolutePressure() {
  return currentPressure;
}
