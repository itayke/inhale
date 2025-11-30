#ifndef SENSOR_H
#define SENSOR_H

// Initialize BMP280 sensor
void sensorInit();

// Calibrate baseline pressure (call once at startup)
void calibrateBaseline();

// Update current pressure reading
void updatePressure();

// Get current pressure delta from baseline
float getPressureDelta();

// Get current temperature in Celsius
float getTemperature();

// Get absolute pressure in Pascals
float getAbsolutePressure();

#endif // SENSOR_H
