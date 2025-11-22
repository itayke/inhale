# Inhale - Breath Visualization Device

ESP32-based interactive breathing visualization system using pressure sensing and visual feedback.

## Hardware

- **ESP32** (Type-C variant)
- **BMP280** - Pressure/Temperature sensor
- **ST7735S** - 128x128 4-Wire SPI TFT Display (3.3V)
- Closed chamber with breathing tube

## Pin Configuration

### BMP280 Sensor (I2C)
- SDA: GPIO 21
- SCL: GPIO 22
- VCC: 3.3V
- GND: GND

### ST7735S Display (SPI)
- MOSI: GPIO 23
- SCLK: GPIO 18
- CS: GPIO 15
- DC: GPIO 2
- RST: GPIO 4
- VCC: 3.3V
- GND: GND

## Features

### Breath Detection
- Real-time pressure monitoring
- Automatic baseline calibration
- Inhale/Exhale/Hold detection
- Session tracking (breath count, duration, patterns)

### Visualization Modes
- **Live Mode**: Real-time wave/water visualization
- **Guided Mode**: Paced breathing exercises
- **Calibration Mode**: Set personal pressure thresholds
- **Stats Mode**: View session metrics

### Breath Gesture Navigation
- **Double Puff** (2 quick exhales): Select/Enter
- **Long Exhale** (2+ sec): Next option/mode
- **Long Inhale** (2+ sec): Previous option/mode
- **Breath Hold** (5+ sec): Menu toggle

(Physical button support planned for future)

## Setup

1. Install PlatformIO
2. Clone this repository
3. Connect hardware according to pin configuration
4. Build and upload:
   ```bash
   pio run --target upload
   ```

## Development Status

- [x] Project structure
- [x] Hardware initialization
- [x] Basic pressure reading
- [x] Breath detection algorithm
- [x] Gesture detection framework
- [ ] Wave/water visualization
- [ ] Guided breathing mode
- [ ] Calibration UI
- [ ] Complete session tracking

## Data Persistence

Uses ESP32 NVS (Non-Volatile Storage) for:
- Calibration thresholds
- User preferences
- Session history (planned)

## Serial Monitor

Connect at 115200 baud to see debug output including:
- Pressure readings
- Breath state changes
- Gesture detection
- Calibration values
