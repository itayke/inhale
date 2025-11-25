#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========================================
// Pin Definitions
// ========================================
// ST7735S Display (SPI)
#define TFT_CS    15
#define TFT_RST   4
#define TFT_DC    2
#define TFT_MOSI  23
#define TFT_SCLK  18

// BMP280 Sensor (I2C)
#define BMP_SDA   32
#define BMP_SCL   33

// ========================================
// Display Configuration
// ========================================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// Custom color definitions (not in all ST7735 library versions)
#define ST77XX_GRAY   0x8410  // RGB(128, 128, 128)
#define ST77XX_ORANGE 0xFD20  // RGB(255, 165, 0)

// ========================================
// Application Modes
// ========================================
enum AppMode {
  MODE_LIVE,
  MODE_GUIDED,
  MODE_CALIBRATION,
  MODE_STATS
};

// ========================================
// Breath Detection
// ========================================
enum BreathState {
  BREATH_IDLE,
  BREATH_INHALE,
  BREATH_EXHALE,
  BREATH_HOLD
};

// Default calibration thresholds (Pa)
#define DEFAULT_INHALE_THRESHOLD  -5.0f
#define DEFAULT_EXHALE_THRESHOLD   5.0f
#define BREATH_HOLD_TIMEOUT_MS     3000
#define BREATH_HOLD_STABILITY_PA   2.0f

// ========================================
// Guided Breathing
// ========================================
enum GuidedPhase {
  GUIDE_INHALE,
  GUIDE_HOLD,
  GUIDE_EXHALE
};

// Breathing pattern: 4-7-8 (inhale 4s, hold 7s, exhale 8s)
#define INHALE_DURATION   4000
#define HOLD_DURATION     7000
#define EXHALE_DURATION   8000

// ========================================
// Gesture Detection
// ========================================
#define GESTURE_LONG_BREATH_MS    1500
#define GESTURE_HOLD_MENU_MS      5000
#define GESTURE_DEBOUNCE_MS       1000
#define GESTURE_PUFF_WINDOW_MS    1000

// ========================================
// Update Rates
// ========================================
#define MAIN_LOOP_DELAY_MS        20    // ~50Hz
#define WAVE_UPDATE_FPS           30
#define GUIDED_UPDATE_FPS         30
#define CALIBRATION_UPDATE_HZ     10
#define STATS_UPDATE_HZ           1

#endif // CONFIG_H
