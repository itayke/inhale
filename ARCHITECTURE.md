# Architecture Overview

## Project Structure

```
inhale/
├── src/
│   ├── main.cpp                    # Application entry point & orchestration
│   ├── config.h                    # Global configuration & constants
│   │
│   ├── BreathData.cpp/h            # Breath detection & normalization
│   ├── Display.cpp/h               # ST7735S display wrapper
│   ├── PressureSensor.cpp/h        # BMP280 sensor interface
│   ├── Storage.cpp/h               # NVS persistent storage
│   │
│   └── modes/                      # Display modes
│       ├── live_mode.cpp/h         # Real-time wave visualization
│       └── diagnostic_mode.cpp/h   # Sensor diagnostics display
│
├── platformio.ini                  # Build configuration
├── README.md                       # Project overview
├── TESTING.md                      # Testing & tuning guide
├── HARDWARE_SETUP.md               # Wiring instructions
└── ARCHITECTURE.md                 # This file
```

## Module Responsibilities

### Core (`main.cpp`)

- Application setup and initialization
- Main event loop
- Mode switching orchestration
- Global instance definitions

**Dependencies:** All modules

### Configuration (`config.h`)

- Pin assignments (I2C, SPI)
- Screen dimensions and colors
- Enumerations (AppMode, BreathState)
- Timing constants and thresholds
- No dependencies (included by all modules)

### Classes

#### `BreathData`

Handles breath detection, normalization, and session tracking.

**Responsibilities:**
- Breath state machine (idle/inhale/exhale/hold)
- Asymmetric breath normalization (-1 to +1)
- Auto-expanding min/max calibration bounds
- Breath cycle counting
- Session statistics

**Key Methods:**
- `init()` - Initialize breath detection
- `detect(pressureDelta)` - Update state based on pressure
- `getNormalizedBreath()` - Get normalized value (-1 to +1)
- `getMinDelta() / getMaxDelta()` - Calibration bounds
- `resetCalibration()` - Reset min/max bounds
- `resetSession()` - Reset session statistics

**Dependencies:** config.h

#### `PressureSensor`

Low-level BMP280 sensor interface for raw pressure data.

**Responsibilities:**
- BMP280 initialization and configuration
- Baseline pressure calibration
- Continuous pressure reading
- Temperature reading

**Key Methods:**
- `init()` - Initialize BMP280 sensor
- `calibrateBaseline()` - Calibrate baseline pressure
- `update()` - Read current pressure
- `getDelta()` - Get pressure delta from baseline (Pa)
- `getAbsolutePressure()` - Get absolute pressure (Pa)
- `getTemperature()` - Get temperature (Celsius)

**Dependencies:** config.h, Display, Adafruit BMP280

#### `Display`

ST7735S display wrapper with double-buffered rendering.

**Responsibilities:**
- Display initialization
- Double-buffered canvas for flicker-free rendering
- Utility functions (clear, show message, rgb565 color conversion)

**Key Methods:**
- `init()` - Initialize ST7735S display
- `getTft()` - Get raw Adafruit_ST7735 reference
- `getCanvas()` - Get GFXcanvas16 for drawing
- `blit()` - Transfer canvas to display
- `clear()` - Clear display
- `showMessage()` - Display centered message
- `rgb565(r, g, b)` - Convert RGB to 565 format (static)

**Dependencies:** config.h, Adafruit ST7735

#### `Storage`

NVS (Non-Volatile Storage) wrapper for persistent data.

**Responsibilities:**
- Initialize NVS namespace
- Load/save calibration thresholds

**Key Methods:**
- `init()` - Initialize NVS
- `loadCalibration()` - Load saved thresholds
- `saveCalibration()` - Save thresholds to NVS

**Dependencies:** config.h, ESP32 Preferences

### Modes Layer

Each mode is a self-contained visualization with its own rendering logic.

#### `modes/live_mode`

- Animated multi-layer wave visualization
- Wave height responds to normalized breath
- Color changes based on breath state
- HUD overlay with breath count and state

**Dependencies:** config.h, BreathData, Display

#### `modes/diagnostic_mode`

- Real-time pressure delta display
- Normalized breath value with bar visualization
- Absolute pressure in inHg
- Temperature in Celsius and Fahrenheit
- Calibration bounds (min/max)

**Dependencies:** config.h, BreathData, Display, PressureSensor

## Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                         main.cpp                            │
│                    (Orchestration Layer)                    │
└──────────────┬──────────────────────────────┬───────────────┘
               │                              │
     ┌─────────▼─────────┐          ┌────────▼────────┐
     │  PressureSensor   │          │  Mode Rendering │
     │    .update()      │          └────────┬────────┘
     └─────────┬─────────┘                   │
               │                    ┌────────▼────────────────┐
               │ pressureDelta      │    modes/*_mode.cpp     │
               │                    │  - Draw visualizations  │
     ┌─────────▼─────────┐          │  - Use breathData for   │
     │    BreathData     │          │    normalized values    │
     │    .detect()      │──────────┤  - Use display for      │
     │  - Normalize      │          │    rendering            │
     │  - Detect state   │          └─────────────────────────┘
     │  - Count breaths  │
     └───────────────────┘
```

## Key Design Patterns

### Class-Based Architecture

Each hardware component and logical unit is encapsulated in a class with clear responsibilities. Global instances are defined in `main.cpp` and declared `extern` in headers.

### Separation of Concerns

- `PressureSensor` handles raw sensor data only
- `BreathData` handles breath-related processing (normalization, detection)
- `Display` handles all rendering
- `Storage` handles persistence

### Double-Buffered Rendering

Display uses a `GFXcanvas16` off-screen buffer. All drawing happens to the canvas, then `blit()` transfers to the display for flicker-free updates.

### Auto-Expanding Calibration

`BreathData` automatically tracks min/max pressure deltas seen, allowing normalization to adapt to the user's breathing range without manual calibration.

## Adding New Features

### New Visualization Mode

1. Create `src/modes/new_mode.cpp` and `.h`
2. Add mode to `AppMode` enum in `config.h`
3. Include header in `main.cpp`
4. Add case to mode switch in `loop()`

### New Sensor Data

1. Add getter method to `PressureSensor` class
2. Update `PressureSensor::update()` to read new data
3. Use in modes as needed

## Build System

PlatformIO automatically discovers and compiles all `.cpp` files in `src/` and subdirectories. No manual build configuration needed.

## Testing Strategy

- **Hardware-in-loop**: Use serial monitor for debugging
- **Visual testing**: Each mode can be tested separately
- **Diagnostic mode**: Real-time sensor data verification

See `TESTING.md` for detailed testing procedures.
