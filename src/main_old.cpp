#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_BMP280.h>
#include <Preferences.h>

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
#define BMP_SDA   21
#define BMP_SCL   22

// ========================================
// Display Configuration
// ========================================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// Custom color definitions (not in all ST7735 library versions)
#define ST77XX_GRAY   0x8410  // RGB(128, 128, 128)
#define ST77XX_ORANGE 0xFD20  // RGB(255, 165, 0)

// ========================================
// Global Objects
// ========================================
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BMP280 bmp;
Preferences preferences;

// ========================================
// Application State
// ========================================
enum AppMode {
  MODE_LIVE,
  MODE_GUIDED,
  MODE_CALIBRATION,
  MODE_STATS
};

AppMode currentMode = MODE_LIVE;

// ========================================
// Breath Detection Variables
// ========================================
float baselinePressure = 0;
float currentPressure = 0;
float pressureDelta = 0;

// Calibration thresholds (will be loaded from NVS)
float inhaleThreshold = -5.0;   // Pa below baseline
float exhaleThreshold = 5.0;    // Pa above baseline

// Breath state tracking
enum BreathState {
  BREATH_IDLE,
  BREATH_INHALE,
  BREATH_EXHALE,
  BREATH_HOLD
};

BreathState currentBreathState = BREATH_IDLE;
unsigned long breathStartTime = 0;
unsigned long lastBreathTime = 0;

// Session statistics
int breathCount = 0;
unsigned long sessionStartTime = 0;
float averageBreathDuration = 0;

// ========================================
// Gesture Detection Variables
// ========================================
unsigned long lastPuffTime = 0;
int puffCount = 0;
bool gestureDetected = false;
unsigned long lastGestureTime = 0;
bool longBreathGestureTriggered = false;

// ========================================
// Guided Breathing Variables
// ========================================
enum GuidedPhase {
  GUIDE_INHALE,
  GUIDE_HOLD,
  GUIDE_EXHALE
};

GuidedPhase guidedPhase = GUIDE_INHALE;
unsigned long guidedPhaseStart = 0;

// Breathing pattern: 4-7-8 (inhale 4s, hold 7s, exhale 8s)
const int INHALE_DURATION = 4000;
const int HOLD_DURATION = 7000;
const int EXHALE_DURATION = 8000;

// ========================================
// Function Prototypes
// ========================================
void initHardware();
void loadCalibration();
void saveCalibration();
void calibrateBaseline();
void updatePressure();
void detectBreath();
void detectGestures();
void updateDisplay();
void drawWave();
void drawGuidedBreathing();
void drawCalibration();
void drawStats();

// ========================================
// Setup
// ========================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Inhale - Breath Visualization Device");
  Serial.println("====================================");

  initHardware();
  loadCalibration();
  calibrateBaseline();

  sessionStartTime = millis();

  Serial.println("System ready!");
}

// ========================================
// Main Loop
// ========================================
void loop() {
  updatePressure();
  detectBreath();
  detectGestures();
  updateDisplay();

  delay(20); // ~50Hz update rate
}

// ========================================
// Hardware Initialization
// ========================================
void initHardware() {
  Serial.println("Initializing hardware...");

  // Initialize I2C for BMP280
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

  // Initialize ST7735S display
  tft.initR(INITR_144GREENTAB); // Use GREENTAB for 128x128
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);

  Serial.println("ST7735S display initialized");

  // Initialize NVS
  preferences.begin("inhale", false);

  Serial.println("NVS storage initialized");
}

// ========================================
// Calibration Functions
// ========================================
void loadCalibration() {
  inhaleThreshold = preferences.getFloat("inhaleThresh", -5.0);
  exhaleThreshold = preferences.getFloat("exhaleThresh", 5.0);

  Serial.print("Loaded calibration - Inhale: ");
  Serial.print(inhaleThreshold);
  Serial.print(" Pa, Exhale: ");
  Serial.print(exhaleThreshold);
  Serial.println(" Pa");
}

void saveCalibration() {
  preferences.putFloat("inhaleThresh", inhaleThreshold);
  preferences.putFloat("exhaleThresh", exhaleThreshold);

  Serial.println("Calibration saved to NVS");
}

void calibrateBaseline() {
  Serial.println("Calibrating baseline pressure...");

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 60);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);
  tft.println("Calibrating...");
  tft.println("  Breathe");
  tft.println("  normally");

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

  tft.fillScreen(ST77XX_BLACK);
}

// ========================================
// Pressure & Breath Detection
// ========================================
void updatePressure() {
  currentPressure = bmp.readPressure();
  pressureDelta = currentPressure - baselinePressure;
}

void detectBreath() {
  BreathState previousState = currentBreathState;
  unsigned long now = millis();

  // Detect breath state based on pressure delta
  if (pressureDelta < inhaleThreshold) {
    currentBreathState = BREATH_INHALE;
  } else if (pressureDelta > exhaleThreshold) {
    currentBreathState = BREATH_EXHALE;
  } else {
    // Check for breath hold (stable pressure for >3 seconds)
    if (now - lastBreathTime > 3000 && abs(pressureDelta) < 2.0) {
      currentBreathState = BREATH_HOLD;
    } else {
      currentBreathState = BREATH_IDLE;
    }
  }

  // Detect breath transitions for counting
  if (previousState != currentBreathState) {
    breathStartTime = now;

    // Count a full breath cycle when transitioning from exhale to inhale
    if (previousState == BREATH_EXHALE && currentBreathState == BREATH_INHALE) {
      breathCount++;

      // Update average breath duration
      unsigned long cycleDuration = now - lastBreathTime;
      if (breathCount == 1) {
        averageBreathDuration = cycleDuration;
      } else {
        averageBreathDuration = (averageBreathDuration * (breathCount - 1) + cycleDuration) / breathCount;
      }
    }

    lastBreathTime = now;
  }
}

// ========================================
// Gesture Detection
// ========================================
void detectGestures() {
  unsigned long now = millis();
  unsigned long breathDuration = now - breathStartTime;

  // Prevent gesture spam - require 1 second between gestures
  if (now - lastGestureTime < 1000) {
    return;
  }

  // Reset puff count if too much time has passed
  if (now - lastPuffTime > 1000) {
    puffCount = 0;
  }

  // Reset long breath gesture flag when returning to idle
  if (currentBreathState == BREATH_IDLE) {
    longBreathGestureTriggered = false;
  }

  // ===== LONG EXHALE GESTURE: Next Mode =====
  if (currentBreathState == BREATH_EXHALE &&
      !longBreathGestureTriggered &&
      breathDuration > 1500) {

    longBreathGestureTriggered = true;
    lastGestureTime = now;

    // Cycle to next mode
    currentMode = (AppMode)((currentMode + 1) % 4);

    Serial.print("GESTURE: Long exhale -> Mode: ");
    switch (currentMode) {
      case MODE_LIVE:        Serial.println("LIVE"); break;
      case MODE_GUIDED:      Serial.println("GUIDED"); break;
      case MODE_CALIBRATION: Serial.println("CALIBRATION"); break;
      case MODE_STATS:       Serial.println("STATS"); break;
    }

    // Visual feedback
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(20, 60);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    switch (currentMode) {
      case MODE_LIVE:        tft.print("LIVE"); break;
      case MODE_GUIDED:      tft.print("GUIDED"); break;
      case MODE_CALIBRATION: tft.print("CALIB"); break;
      case MODE_STATS:       tft.print("STATS"); break;
    }
    delay(800); // Show mode name briefly
  }

  // ===== LONG INHALE GESTURE: Previous Mode =====
  if (currentBreathState == BREATH_INHALE &&
      !longBreathGestureTriggered &&
      breathDuration > 1500) {

    longBreathGestureTriggered = true;
    lastGestureTime = now;

    // Cycle to previous mode
    currentMode = (AppMode)((currentMode + 3) % 4); // +3 is same as -1 in mod 4

    Serial.print("GESTURE: Long inhale -> Mode: ");
    switch (currentMode) {
      case MODE_LIVE:        Serial.println("LIVE"); break;
      case MODE_GUIDED:      Serial.println("GUIDED"); break;
      case MODE_CALIBRATION: Serial.println("CALIBRATION"); break;
      case MODE_STATS:       Serial.println("STATS"); break;
    }

    // Visual feedback
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(20, 60);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    switch (currentMode) {
      case MODE_LIVE:        tft.print("LIVE"); break;
      case MODE_GUIDED:      tft.print("GUIDED"); break;
      case MODE_CALIBRATION: tft.print("CALIB"); break;
      case MODE_STATS:       tft.print("STATS"); break;
    }
    delay(800); // Show mode name briefly
  }

  // ===== BREATH HOLD GESTURE: Reset Session =====
  if (currentBreathState == BREATH_HOLD &&
      !longBreathGestureTriggered &&
      breathDuration > 5000) {

    longBreathGestureTriggered = true;
    lastGestureTime = now;

    Serial.println("GESTURE: Breath hold -> Reset session");

    // Reset session statistics
    breathCount = 0;
    averageBreathDuration = 0;
    sessionStartTime = millis();

    // Visual feedback
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 60);
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(1);
    tft.println("SESSION RESET");
    delay(1000);
  }

  // ===== DOUBLE PUFF GESTURE: Reserved for future use =====
  // (Could be used for menu selection in calibration mode)
}

// ========================================
// Display Update
// ========================================
void updateDisplay() {
  switch (currentMode) {
    case MODE_LIVE:
      drawWave();
      break;
    case MODE_GUIDED:
      drawGuidedBreathing();
      break;
    case MODE_CALIBRATION:
      drawCalibration();
      break;
    case MODE_STATS:
      drawStats();
      break;
  }
}

// ========================================
// Wave Visualization
// ========================================
void drawWave() {
  static unsigned long lastUpdate = 0;
  static float wavePhase = 0;
  static float targetWaveHeight = SCREEN_HEIGHT / 2;
  static float currentWaveHeight = SCREEN_HEIGHT / 2;

  unsigned long now = millis();
  if (now - lastUpdate < 33) return; // ~30 FPS
  float dt = (now - lastUpdate) / 1000.0;
  lastUpdate = now;

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
  switch (currentBreathState) {
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
  tft.print(breathCount);

  // Breath state indicator (top right)
  const char* stateText;
  switch (currentBreathState) {
    case BREATH_INHALE: stateText = "IN "; break;
    case BREATH_EXHALE: stateText = "OUT"; break;
    case BREATH_HOLD:   stateText = "HLD"; break;
    default:            stateText = "..."; break;
  }
  tft.setCursor(SCREEN_WIDTH - 22, 4);
  tft.print(stateText);
}

// ========================================
// Guided Breathing Mode
// ========================================
void drawGuidedBreathing() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate < 33) return; // ~30 FPS
  lastUpdate = now;

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
        breathCount++; // Count completed breath cycle
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
  tft.print(breathCount);
}

// ========================================
// Calibration Mode
// ========================================
void drawCalibration() {
  static unsigned long lastUpdate = 0;
  static float minPressureSeen = 0;
  static float maxPressureSeen = 0;
  static bool calibrationActive = false;

  unsigned long now = millis();
  if (now - lastUpdate < 100) return; // 10 FPS for calibration
  lastUpdate = now;

  // Track min/max pressure deltas during calibration
  if (calibrationActive) {
    if (pressureDelta < minPressureSeen) minPressureSeen = pressureDelta;
    if (pressureDelta > maxPressureSeen) maxPressureSeen = pressureDelta;
  } else {
    // Start calibration
    calibrationActive = true;
    minPressureSeen = pressureDelta;
    maxPressureSeen = pressureDelta;
  }

  // Auto-adjust thresholds to 70% of observed extremes
  float autoInhaleThresh = minPressureSeen * 0.7;
  float autoExhaleThresh = maxPressureSeen * 0.7;

  // Clear screen
  tft.fillScreen(ST77XX_BLACK);

  // Title
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(25, 5);
  tft.print("CALIBRATION");

  // Instructions
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 20);
  tft.print("Take a few deep");
  tft.setCursor(5, 30);
  tft.print("breaths...");

  // Draw real-time pressure bar
  int centerY = SCREEN_HEIGHT / 2;
  int barX = 10;
  int barWidth = 30;
  int maxBarHeight = 50;

  // Draw center line
  tft.drawFastHLine(barX, centerY, SCREEN_WIDTH - barX - 10, ST77XX_GRAY);

  // Draw pressure bar
  int barHeight = constrain(pressureDelta * 2, -maxBarHeight, maxBarHeight);
  uint16_t barColor = (barHeight < 0) ? ST77XX_CYAN : ST77XX_MAGENTA;

  if (barHeight > 0) {
    tft.fillRect(barX, centerY - barHeight, barWidth, barHeight, barColor);
  } else {
    tft.fillRect(barX, centerY, barWidth, -barHeight, barColor);
  }

  // Draw min/max markers
  int minY = centerY - (int)(minPressureSeen * 2);
  int maxY = centerY - (int)(maxPressureSeen * 2);

  tft.drawFastHLine(barX, minY, barWidth, ST77XX_CYAN);
  tft.drawFastHLine(barX, maxY, barWidth, ST77XX_MAGENTA);

  // Draw threshold lines
  int inhaleThreshY = centerY - (int)(autoInhaleThresh * 2);
  int exhaleThreshY = centerY - (int)(autoExhaleThresh * 2);

  tft.drawFastHLine(barX, inhaleThreshY, SCREEN_WIDTH - barX - 10, ST77XX_BLUE);
  tft.drawFastHLine(barX, exhaleThreshY, SCREEN_WIDTH - barX - 10, ST77XX_RED);

  // Display current values
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(50, centerY - 60);
  tft.print("Current: ");
  tft.print(pressureDelta, 1);
  tft.print("Pa");

  tft.setCursor(50, centerY - 40);
  tft.setTextColor(ST77XX_CYAN);
  tft.print("Inhale min: ");
  tft.print(minPressureSeen, 1);

  tft.setCursor(50, centerY - 30);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("Threshold: ");
  tft.print(autoInhaleThresh, 1);

  tft.setCursor(50, centerY + 25);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print("Exhale max: ");
  tft.print(maxPressureSeen, 1);

  tft.setCursor(50, centerY + 35);
  tft.setTextColor(ST77XX_RED);
  tft.print("Threshold: ");
  tft.print(autoExhaleThresh, 1);

  // Instructions to save
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(5, SCREEN_HEIGHT - 25);
  tft.print("Hold breath 5s");
  tft.setCursor(5, SCREEN_HEIGHT - 15);
  tft.print("to save & exit");

  // Check if user wants to save (breath hold detected)
  if (currentBreathState == BREATH_HOLD &&
      (now - breathStartTime) > 5000 &&
      (now - lastGestureTime) > 1000) {

    // Save the calibrated thresholds
    inhaleThreshold = autoInhaleThresh;
    exhaleThreshold = autoExhaleThresh;
    saveCalibration();

    lastGestureTime = now;
    calibrationActive = false;

    // Show confirmation
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(20, 60);
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(1);
    tft.println("SAVED!");
    tft.setCursor(10, 75);
    tft.print("Inhale: ");
    tft.print(inhaleThreshold, 1);
    tft.setCursor(10, 85);
    tft.print("Exhale: ");
    tft.print(exhaleThreshold, 1);
    delay(2000);

    // Return to live mode
    currentMode = MODE_LIVE;
  }
}

// ========================================
// Stats Display
// ========================================
void drawStats() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate < 1000) return; // Update once per second
  lastUpdate = now;

  tft.fillScreen(ST77XX_BLACK);

  // Title bar
  tft.fillRect(0, 0, SCREEN_WIDTH, 18, tft.color565(0, 80, 120));
  tft.setCursor(20, 5);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("SESSION STATS");

  // Calculate statistics
  unsigned long sessionDuration = (now - sessionStartTime) / 1000;
  float breathsPerMinute = 0;
  if (sessionDuration > 0) {
    breathsPerMinute = (breathCount * 60.0) / sessionDuration;
  }

  int yPos = 25;

  // Breath count - large display
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, yPos);
  tft.print("Total Breaths");

  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(40, yPos + 12);
  tft.print(breathCount);

  yPos += 40;

  // Breathing rate
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(5, yPos);
  tft.print("Breaths/Min");

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(45, yPos + 12);
  if (breathCount > 0) {
    tft.print(breathsPerMinute, 1);
  } else {
    tft.print("--");
  }

  // Draw breathing rate bar
  int rateBarWidth = constrain(breathsPerMinute * 5, 0, SCREEN_WIDTH - 20);
  tft.fillRect(10, yPos + 30, rateBarWidth, 4, ST77XX_GREEN);
  tft.drawRect(10, yPos + 30, SCREEN_WIDTH - 20, 4, ST77XX_GRAY);

  yPos += 42;

  // Average breath duration
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setCursor(5, yPos);
  tft.print("Avg Duration");

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(80, yPos);
  if (breathCount > 0) {
    tft.print(averageBreathDuration / 1000.0, 1);
    tft.print("s");
  } else {
    tft.print("--");
  }

  yPos += 15;

  // Session time
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(5, yPos);
  tft.print("Session Time");

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(80, yPos);
  int minutes = sessionDuration / 60;
  int seconds = sessionDuration % 60;
  if (minutes < 10) tft.print("0");
  tft.print(minutes);
  tft.print(":");
  if (seconds < 10) tft.print("0");
  tft.print(seconds);

  yPos += 15;

  // Current pressure (live indicator)
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_ORANGE);
  tft.setCursor(5, yPos);
  tft.print("Pressure");

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(65, yPos);
  tft.print(pressureDelta, 1);
  tft.print("Pa");

  // Mini live breath indicator at bottom
  int miniBarY = SCREEN_HEIGHT - 12;
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GRAY);
  tft.setCursor(5, miniBarY);

  switch (currentBreathState) {
    case BREATH_INHALE:
      tft.setTextColor(ST77XX_CYAN);
      tft.print("INHALING");
      break;
    case BREATH_EXHALE:
      tft.setTextColor(ST77XX_MAGENTA);
      tft.print("EXHALING");
      break;
    case BREATH_HOLD:
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("HOLDING");
      break;
    default:
      tft.setTextColor(ST77XX_GRAY);
      tft.print("IDLE");
      break;
  }

  // Mini pressure bar
  int miniBarWidth = map(constrain(pressureDelta, -25, 25), -25, 25, 0, 50);
  int miniBarX = SCREEN_WIDTH - 55;
  tft.fillRect(miniBarX + 25, miniBarY + 2, 1, 6, ST77XX_WHITE);

  if (miniBarWidth > 25) {
    tft.fillRect(miniBarX + 25, miniBarY + 2, miniBarWidth - 25, 6,
                 currentBreathState == BREATH_EXHALE ? ST77XX_MAGENTA : ST77XX_GRAY);
  } else {
    tft.fillRect(miniBarX + miniBarWidth, miniBarY + 2, 25 - miniBarWidth, 6,
                 currentBreathState == BREATH_INHALE ? ST77XX_CYAN : ST77XX_GRAY);
  }
}
