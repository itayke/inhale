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
- **Long Exhale** (1.5+ sec): Next mode
- **Long Inhale** (1.5+ sec): Previous mode
- **Breath Hold** (5+ sec): Reset session

(Physical button support can be added later)

## Setup

### Hardware Assembly

See **[HARDWARE_SETUP.md](HARDWARE_SETUP.md)** for detailed wiring instructions with diagrams.

**Quick reference:**
- BMP280 → I2C (GPIO 21/22)
- ST7735S → SPI (GPIO 23, 18, 15, 2, 4)
- All components use 3.3V power

### Software Upload

1. Install [PlatformIO](https://platformio.org/install) or VS Code + PlatformIO extension
2. Clone this repository
3. Connect ESP32 via USB-C
4. Build and upload:
   ```bash
   pio run --target upload
   pio device monitor
   ```

## Project Structure

The codebase uses a modular architecture for maintainability:

```
src/
├── main.cpp              # Application orchestration
├── config.h              # Configuration & constants
├── hardware/             # Hardware abstraction (display, sensor, storage)
├── detection/            # Breath & gesture detection algorithms
└── modes/                # Visualization modes (live, guided, calib, stats)
```

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed module documentation.

## Development Status

- [x] Modular architecture
- [x] Hardware initialization
- [x] Pressure reading & calibration
- [x] Breath detection algorithm
- [x] Gesture recognition
- [x] Wave/water visualization
- [x] Guided breathing mode (4-7-8 pattern)
- [x] Calibration UI
- [x] Session statistics tracking
- [x] NVS persistent storage

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
