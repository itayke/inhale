# Hardware Setup Guide

## Parts Needed

- **ESP32** development board (Type-C USB)
- **BMP280** pressure/temperature sensor module
- **ST7735S** 128x128 TFT display (4-wire SPI, 3.3V)
- **Breadboard** (recommended for prototyping)
- **Jumper wires** (male-to-female or male-to-male depending on your modules)
- **Breathing chamber** (sealed container with tube attachment)
- **USB-C cable** (for power and programming)

## Wiring Connections

### Step 1: Connect BMP280 Sensor (I2C)

The BMP280 uses I2C communication (2 wires for data).

| BMP280 Pin | ESP32 Pin | Wire Color Suggestion |
|------------|-----------|----------------------|
| VCC        | 3.3V      | Red                  |
| GND        | GND       | Black                |
| SDA        | D21 (GPIO 21) | Blue             |
| SCL / SCK  | D22 (GPIO 22) | Yellow           |

**Notes:**
- Some BMP280 modules work with 5V, but use **3.3V** to be safe
- GPIO 21 and 22 are the default I2C pins on ESP32
- SDA = Data line, SCL = Clock line

### Step 2: Connect ST7735S Display (SPI)

The display uses SPI communication (faster, more wires).

| ST7735S Pin | ESP32 Pin | Wire Color Suggestion |
|-------------|-----------|----------------------|
| VCC         | 3.3V      | Red                  |
| GND         | GND       | Black                |
| SCL / SCLK / SCK | D18 (GPIO 18) | Yellow        |
| SDA / MOSI  | D23 (GPIO 23) | Blue              |
| CS          | D15 (GPIO 15) | Green             |
| DC / RS / A0 | D2 (GPIO 2)  | Orange            |
| RST / RESET | D4 (GPIO 4)   | White             |
| LED / BL    | 3.3V      | Red (optional)       |

**Notes:**
- Display **must** be 3.3V version (not 5V)
- MOSI = Master Out Slave In (data from ESP32 to display)
- CS = Chip Select
- DC = Data/Command selector
- RST = Reset pin
- If your display has a backlight (LED/BL) pin, connect to 3.3V for full brightness

### Step 3: Breathing Chamber Connection

1. **Seal the BMP280** inside an airtight container (plastic box, jar, etc.)
2. **Drill a hole** in the container lid for a tube (8-10mm diameter)
3. **Insert tube** (aquarium tubing, silicone tube, or drinking straw works)
4. **Seal around tube** with hot glue or silicone sealant
5. **Add mouthpiece** to the tube end (optional, for comfort)

**Important:** The BMP280 must be inside the sealed chamber to detect pressure changes when you breathe through the tube.

## Visual Wiring Diagram

```
                    ESP32 Development Board
                  ┌─────────────────────────┐
                  │                         │
       ┌──────────┤ 3.3V                    │
       │  ┌───────┤ GND                     │
       │  │       │                         │
       │  │  ┌────┤ GPIO 21 (I2C SDA)      │
       │  │  │┌───┤ GPIO 22 (I2C SCL)      │
       │  │  ││   │                         │
       │  │  ││┌──┤ GPIO 23 (SPI MOSI)     │
       │  │  │││┌─┤ GPIO 18 (SPI SCLK)     │
       │  │  ││││┌┤ GPIO 15 (SPI CS)       │
       │  │  │││││┤ GPIO 2  (DC)           │
       │  │  │││││┤ GPIO 4  (RST)          │
       │  │  │││││└────────────────────────┘
       │  │  ││││└─────────────┐
       │  │  │││└────────────┐ │
       │  │  ││└───────────┐ │ │
       │  │  │└──────────┐ │ │ │
       │  │  │           │ │ │ │
       │  │  │      ST7735S Display
       │  │  │     ┌──────────────┐
       │  └──┼─────┤ GND          │
       └─────┼─────┤ VCC (3.3V)   │
             │  ┌──┤ SCL          │
             │  │┌─┤ SDA (MOSI)   │
             │  ││┌┤ CS           │
             │  │││┤ DC           │
             │  │││┤ RST          │
             │  │││└──────────────┘
             │  ││└────┐
             │  │└───┐ │
             │  └──┐ │ │
             │     │ │ │
       BMP280 Sensor │ │
      ┌──────────────┤ │
      │ VCC (3.3V)   │ │
   ┌──┤ GND          │ │
   │  │ SDA ─────────┘ │
   │  │ SCL ───────────┘
   │  └──────────────┘
   │
   └─── Sealed Chamber
        with tube
```

## Power Connections

**All 3.3V connections can share the same ESP32 3.3V pin:**
- Use a breadboard power rail, or
- Use a Y-splitter wire

**All GND connections must share common ground:**
- Connect all GND pins together

## Assembly Steps

### 1. Prepare Breadboard (Recommended)

```
1. Place ESP32 in center of breadboard
2. Connect power rails:
   - Red rail → ESP32 3.3V
   - Black rail → ESP32 GND
```

### 2. Connect BMP280

```
1. Place BMP280 on breadboard
2. Wire connections (see Step 1 table)
3. Double-check VCC is 3.3V (not 5V!)
```

### 3. Connect Display

```
1. Display might have pins pre-soldered or need header pins
2. Connect all 7-8 wires (see Step 2 table)
3. Test: Power ESP32, display backlight should turn on
```

### 4. Build Breathing Chamber

```
1. Put BMP280 inside sealed container
2. Route wires through small gap in lid (seal with putty)
3. Attach breathing tube to container
4. Test seal: blow into tube, container should pressurize
```

## Testing Connections

### Test 1: Power On

```bash
# Connect ESP32 via USB-C
# You should see:
✓ ESP32 LED turns on
✓ Display backlight illuminates
```

### Test 2: Upload Test Sketch

```bash
# In VS Code with PlatformIO:
pio run --target upload
pio device monitor
```

**Expected serial output:**
```
Inhale - Breath Visualization Device
====================================
Initializing BMP280 sensor...
BMP280 initialized
Initializing ST7735S display...
ST7735S display initialized
```

### Test 3: BMP280 Detection

**If you see:** `ERROR: Could not find BMP280 sensor!`

**Troubleshooting:**
1. Check I2C wiring (SDA/SCL not swapped?)
2. Verify 3.3V power to BMP280
3. Try alternate I2C address (code tries both 0x76 and 0x77)

### Test 4: Display Test

**If display is blank or garbled:**

1. Check all 7 SPI connections
2. Verify display is 3.3V model
3. Try different display initialization in `hardware/display.cpp`:
   ```cpp
   tft.initR(INITR_BLACKTAB);  // Instead of INITR_144GREENTAB
   ```

### Test 5: Pressure Reading

Blow into the tube while watching serial monitor:
```
Current pressure delta: +15.2 Pa  ← Exhale detected
Current pressure delta: -12.8 Pa  ← Inhale detected
```

## Common Issues

### Display shows wrong colors
- Wrong init flag - try different `INITR_*` options

### BMP280 not detected
- Wrong I2C address (0x76 vs 0x77)
- Loose SDA/SCL connections
- Power issue (needs 3.3V)

### Pressure doesn't change
- Chamber not sealed properly
- BMP280 not inside chamber
- Tube blocked or kinked

### ESP32 won't flash
- Hold BOOT button while uploading
- Try different USB cable (data, not charge-only)

## Final Setup

Once everything works:

1. **Secure connections** with hot glue or tape
2. **Mount display** where visible
3. **Position chamber** comfortably
4. **Route tube** to comfortable breathing position
5. **Power on** and run calibration mode!

## Next Steps

→ See [TESTING.md](TESTING.md) for software testing and calibration
