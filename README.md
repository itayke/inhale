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
- Asymmetric breath normalization (-1 to +1)
- Auto-expanding calibration bounds
- Session tracking (breath count, duration)

### Visualization Modes
- **Live Mode**: Real-time wave/water visualization responding to breath
- **Diagnostic Mode**: Raw sensor data, normalized values, calibration bounds

## Setup

### Hardware Assembly

See **[HARDWARE_SETUP.md](HARDWARE_SETUP.md)** for detailed wiring instructions with diagrams.

**Quick reference:**
- BMP280 → I2C (GPIO 21/22)
- ST7735S → SPI (GPIO 23, 18, 15, 2, 4)
- All components use 3.3V power

### Software Upload (ESP32)

1. Install [PlatformIO](https://platformio.org/install) or VS Code + PlatformIO extension
2. Clone this repository
3. Connect ESP32 via USB-C
4. Build and upload:
   ```bash
   pio run --target upload
   pio device monitor
   ```

### Desktop Simulator

Test and develop without hardware using the SDL2-based simulator.

**Prerequisites (macOS):**
```bash
brew install sdl2
```

**Build and run:**
```bash
pio run
./.pio/build/simulator/program
```

**Controls:**
- **Mouse Y position**: Simulates breath pressure (up = exhale, down = inhale)
- **Space**: Toggle between Live and Diagnostic modes
- **ESC / Q**: Quit

The simulator uses the real Adafruit GFX library for pixel-perfect rendering that matches the hardware display.

## Project Structure

The codebase uses a class-based architecture:

```
src/
├── main.cpp              # Shared entry point (ESP32 + Simulator)
├── config.h              # Configuration & constants
├── BreathData.cpp/h      # Breath detection, normalization, session tracking
├── Display.cpp/h         # Display interface (ESP32: ST7735S, Sim: SDL2)
├── Sensor.cpp/h          # Sensor interface (ESP32: BMP280, Sim: mouse Y)
├── Storage.cpp/h         # Storage interface (ESP32: NVS, Sim: in-memory)
└── modes/
    ├── live_mode.cpp/h       # Wave visualization (shared)
    └── diagnostic_mode.cpp/h # Sensor diagnostics (shared)

simulator/                # Platform shims for native build
├── Display.cpp           # SDL2 display using GFXcanvas16
├── Sensor.cpp            # Mouse Y-based breath simulation
├── Storage.cpp           # In-memory storage stub
├── Platform.h            # millis(), delay(), Serial shims
├── Arduino.h             # Arduino compatibility layer
├── Print.h               # Print class for Adafruit GFX
├── Wire.h                # I2C stub
└── SPI.h                 # SPI stub
```

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed module documentation.

## Development Status

- [x] Class-based architecture
- [x] Hardware initialization
- [x] Pressure reading & baseline calibration
- [x] Breath detection algorithm
- [x] Asymmetric breath normalization
- [x] Auto-expanding calibration bounds
- [x] Wave/water visualization
- [x] Diagnostic mode
- [x] NVS persistent storage
- [x] Desktop simulator (SDL2)
- [ ] Game mechanics (in progress)

## Data Persistence

Uses ESP32 NVS (Non-Volatile Storage) for:
- Calibration thresholds
- User preferences
- Session history (planned)

## Serial Monitor

Connect at 115200 baud to see debug output including:
- Pressure readings
- Breath state changes
- Calibration values
