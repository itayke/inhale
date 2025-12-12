#include "Sensor.h"
#include "Display.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_BMP280.h>

// Internal BMP280 handle
static Adafruit_BMP280 bmp;

void Sensor::init() {
  Serial.println("Initializing BMP280 sensor...");

  delay(100);

  // Initialize BMP280 - specify chip ID explicitly for GY-BMP280 clones
  unsigned status = bmp.begin(0x76, 0x58);
  if (!status) {
    Serial.println("ERROR: Could not find BMP280 sensor at 0x76!");
    Serial.print("SensorID was: 0x");
    Serial.println(bmp.sensorID(), HEX);

    Serial.println("Trying alternate address 0x77 with chip ID 0x58...");
    status = bmp.begin(0x77, 0x58);
    if (!status) {
      Serial.println("Failed at 0x77 too!");
      Serial.println("ID of 0xFF = bad address or BMP180/BMP085");
      Serial.println("ID of 0x56-0x58 = BMP280");
      Serial.println("ID of 0x60 = BME280");
      while (1) delay(100);
    }
  }

  Serial.println("BMP280 initialized successfully!");

  // Configure BMP280 for high precision
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X16,  // Pressure oversampling
                  Adafruit_BMP280::SAMPLING_X2,   // Temperature oversampling
                  Adafruit_BMP280::FILTER_X16,    // Filtering
                  Adafruit_BMP280::STANDBY_MS_1); // Standby time
}

void Sensor::calibrateBaseline() {
  Serial.println("Calibrating baseline pressure...");

  display.showMessage("Calibrating...\n  Breathe\n  normally", ST77XX_CYAN);

  // Take average of 50 readings
  float sum = 0;
  for (int i = 0; i < 50; i++) {
    sum += bmp.readPressure();
    delay(20);
  }

  baselinePressure = sum / 50.0f;

  Serial.print("Baseline pressure: ");
  Serial.print(baselinePressure);
  Serial.println(" Pa");

  display.clear();
}

void Sensor::update() {
  currentPressure = bmp.readPressure();
  currentTemperature = bmp.readTemperature();
  pressureDelta = currentPressure - baselinePressure;
}
