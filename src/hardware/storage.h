#ifndef STORAGE_H
#define STORAGE_H

// Initialize NVS storage
void storageInit();

// Load calibration values from NVS
void loadCalibration(float& inhaleThreshold, float& exhaleThreshold);

// Save calibration values to NVS
void saveCalibration(float inhaleThreshold, float exhaleThreshold);

#endif // STORAGE_H
