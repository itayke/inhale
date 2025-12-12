#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

class PressureSensor {
public:
  // Initialize BMP280 sensor
  void init();

  // Calibrate baseline pressure (call once at startup)
  void calibrateBaseline();

  // Update current pressure reading (call every loop)
  void update();

  // Get normalized pressure: -1 (max inhale) to +1 (max exhale)
  float getNormalized() const;

  // Get raw pressure delta from baseline (in Pascals)
  float getDelta() const;

  // Get absolute pressure in Pascals
  float getAbsolutePressure() const;

  // Get current temperature in Celsius
  float getTemperature() const;

  // Calibration min/max (for diagnostics)
  float getMinDelta() const { return minPressureDelta; }
  float getMaxDelta() const { return maxPressureDelta; }

  // Reset min/max calibration
  void resetCalibration();

private:
  float baselinePressure = 0;
  float currentPressure = 0;
  float currentTemperature = 0;
  float pressureDelta = 0;
  float normalizedPressure = 0;

  // Calibration bounds for normalization
  float minPressureDelta = -10.0f;  // Initial estimate (inhale)
  float maxPressureDelta = 10.0f;   // Initial estimate (exhale)
};

// Global sensor instance (defined in main.cpp)
extern PressureSensor pressureSensor;

#endif // PRESSURE_SENSOR_H
