# Architecture Overview

## Project Structure

```
inhale/
├── src/
│   ├── main.cpp                    # Application entry point & orchestration
│   ├── config.h                    # Global configuration & constants
│   │
│   ├── hardware/                   # Hardware abstraction layer
│   │   ├── display.cpp/h           # ST7735 display initialization & utilities
│   │   ├── sensor.cpp/h            # BMP280 pressure sensor operations
│   │   └── storage.cpp/h           # NVS persistent storage
│   │
│   ├── detection/                  # Detection algorithms
│   │   ├── breath.cpp/h            # Breath state detection & tracking
│   │   └── gesture.cpp/h           # Breath gesture recognition
│   │
│   └── modes/                      # Display modes
│       ├── live_mode.cpp/h         # Real-time wave visualization
│       ├── guided_mode.cpp/h       # Guided breathing exercise
│       ├── calib_mode.cpp/h        # Calibration interface
│       └── stats_mode.cpp/h        # Session statistics
│
├── platformio.ini                  # Build configuration
├── README.md                       # Project overview
├── TESTING.md                      # Testing & tuning guide
└── ARCHITECTURE.md                 # This file
```

## Module Responsibilities

### Core (`main.cpp`)

- Application setup and initialization
- Main event loop
- Mode switching orchestration
- Gesture handling and state transitions

**Dependencies:** All modules

### Configuration (`config.h`)

- Pin assignments (I2C, SPI)
- Screen dimensions and colors
- Enumerations (AppMode, BreathState, GuidedPhase, GestureType)
- Timing constants and thresholds
- No dependencies (included by all modules)

### Hardware Layer

#### `hardware/display`
- ST7735 display initialization
- Provides display object reference
- Utility functions (clear, show message)

**Dependencies:** config.h, Adafruit libraries

#### `hardware/sensor`
- BMP280 sensor initialization and configuration
- Baseline pressure calibration
- Continuous pressure reading
- Pressure delta calculation

**Dependencies:** config.h, hardware/display, Adafruit BMP280

#### `hardware/storage`
- NVS (Non-Volatile Storage) initialization
- Load/save calibration thresholds
- Persistent preferences management

**Dependencies:** config.h, ESP32 Preferences

### Detection Layer

#### `detection/breath`
- Breath state machine (idle/inhale/exhale/hold)
- Breath cycle counting
- Average duration calculation
- Session statistics tracking
- Manages `BreathData` structure

**Dependencies:** config.h

#### `detection/gesture`
- Detects breath-based gestures:
  - Long exhale (1.5s) → next mode
  - Long inhale (1.5s) → previous mode
  - Breath hold (5s) → reset session
- Debouncing and anti-spam logic

**Dependencies:** config.h, detection/breath, hardware/display

### Modes Layer

Each mode is a self-contained visualization module with its own rendering logic.

#### `modes/live_mode`
- Animated multi-layer wave visualization
- Responsive to breath pressure
- Color changes based on breath state
- HUD overlay with stats

**Dependencies:** config.h, detection/breath, hardware/display

#### `modes/guided_mode`
- 4-7-8 breathing pattern (4s in, 7s hold, 8s out)
- Animated expanding/contracting circle
- Phase indicators and countdown
- Progress bar visualization

**Dependencies:** config.h, detection/breath, hardware/display

#### `modes/calib_mode`
- Real-time pressure bar visualization
- Auto-calibration (70% of observed extremes)
- Interactive threshold adjustment
- Save confirmation on breath hold
- Returns boolean to signal completion

**Dependencies:** config.h, detection/breath, hardware/display, hardware/storage

#### `modes/stats_mode`
- Session statistics dashboard
- Breaths per minute calculation
- Average duration display
- Session elapsed time
- Live mini breath indicator

**Dependencies:** config.h, detection/breath, hardware/display

## Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                         main.cpp                            │
│                    (Orchestration Layer)                    │
└──────────────┬──────────────────────────────┬───────────────┘
               │                              │
     ┌─────────▼─────────┐          ┌────────▼────────┐
     │  Sensor Update    │          │  Mode Rendering │
     └─────────┬─────────┘          └────────┬────────┘
               │                              │
     ┌─────────▼─────────┐          ┌────────▼────────────────┐
     │ hardware/sensor   │          │    modes/*_mode.cpp     │
     │  - Read pressure  │          │  - Draw visualizations  │
     └─────────┬─────────┘          └─────────────────────────┘
               │
     ┌─────────▼─────────┐
     │ detection/breath  │
     │  - Detect state   │
     │  - Count breaths  │
     └─────────┬─────────┘
               │
     ┌─────────▼─────────┐
     │ detection/gesture │
     │  - Recognize      │
     │  - Return action  │
     └───────────────────┘
```

## Key Design Patterns

### Separation of Concerns
Each module has a single, well-defined responsibility. Hardware interfacing is isolated from business logic, which is separate from presentation.

### Data Encapsulation
`BreathData` struct centralizes all breath detection state, passed by reference to modules that need it.

### Stateless Rendering
Mode drawing functions are called every frame and handle their own internal state via static variables when needed.

### Dependency Injection
Modules don't create global objects; they receive references or use accessor functions (`getDisplay()`).

## Adding New Features

### New Visualization Mode

1. Create `src/modes/new_mode.cpp` and `.h`
2. Add mode to `AppMode` enum in `config.h`
3. Include header in `main.cpp`
4. Add case to mode switch in `loop()`

### New Sensor

1. Create `src/hardware/new_sensor.cpp` and `.h`
2. Initialize in `main.cpp` setup()
3. Update as needed in loop()

### New Gesture

1. Add `GESTURE_NEW_ACTION` to `GestureType` enum in `detection/gesture.h`
2. Implement detection logic in `detectGestures()`
3. Handle in `main.cpp` gesture switch

## Build System

PlatformIO automatically discovers and compiles all `.cpp` files in `src/` and subdirectories. No manual build configuration needed.

## Testing Strategy

- **Unit testing**: Each module can be tested independently
- **Hardware-in-loop**: Use serial monitor for breath detection debugging
- **Visual testing**: Each mode can be entered and tested separately
- **Integration**: Main loop coordinates all components

See `TESTING.md` for detailed testing procedures.
