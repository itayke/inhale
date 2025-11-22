# Testing & Tuning Guide

## Initial Setup

### 1. Upload the Firmware

```bash
pio run --target upload
pio device monitor
```

### 2. Check Serial Monitor Output

At 115200 baud, you should see:
- "Inhale - Breath Visualization Device"
- Hardware initialization messages
- BMP280 sensor detected
- Display initialized
- Baseline calibration complete

### 3. Verify Hardware Connections

If you see errors:
- **BMP280 not found**: Check I2C wiring (SDA/SCL), try addresses 0x76 or 0x77
- **Display issues**: Verify SPI connections (MOSI, SCLK, CS, DC, RST)

## Testing Breath Detection

### Live Mode (Default)

The device starts in **LIVE** mode showing the animated wave.

**Test breathing:**
1. **Inhale** (suck through tube) - Wave should drop, turn deep blue
2. **Exhale** (blow into tube) - Wave should rise, turn cyan
3. **Hold breath** - Wave should stabilize

**Watch serial monitor** for breath state changes:
- "BREATH_INHALE"
- "BREATH_EXHALE"
- "BREATH_HOLD"
- Breath count increments

### Sensitivity Issues?

If breath detection is too sensitive or not sensitive enough:

**Option 1: Run Calibration Mode**
1. Long exhale (1.5+ seconds) → Navigate to CALIBRATION mode
2. Take 3-5 deep breaths through the tube
3. Watch the pressure bar and thresholds adjust
4. Hold breath for 5 seconds to save

**Option 2: Manually adjust in code** (src/main.cpp:54-55)
```cpp
float inhaleThreshold = -5.0;   // More negative = less sensitive
float exhaleThreshold = 5.0;    // More positive = less sensitive
```

## Testing Gesture Navigation

### Mode Switching

**Long Exhale (1.5s)** → Next mode
- LIVE → GUIDED → CALIBRATION → STATS → LIVE...

**Long Inhale (1.5s)** → Previous mode
- LIVE → STATS → CALIBRATION → GUIDED → LIVE...

**Watch for**:
- Mode name appears on screen briefly
- Serial output: "GESTURE: Long exhale -> Mode: GUIDED"

**Not triggering?**
- Gestures are debounced (1s cooldown)
- Must hold breath for full 1.5 seconds
- Try exhaling/inhaling more forcefully

### Session Reset

**Breath Hold (5s)** → Reset session statistics

**Expected:**
- "SESSION RESET" appears on screen
- Breath count returns to 0
- Serial: "GESTURE: Breath hold -> Reset session"

## Testing Each Mode

### LIVE Mode
- ✓ Wave animates smoothly
- ✓ Wave height responds to breathing
- ✓ Colors change with breath state
- ✓ Breath count increments
- ✓ State indicator (IN/OUT/HLD) updates

### GUIDED Mode
- ✓ Circle expands during INHALE phase (4s)
- ✓ Circle stays full during HOLD phase (7s)
- ✓ Circle contracts during EXHALE phase (8s)
- ✓ Progress bar shows phase progress
- ✓ Countdown timer shows seconds remaining
- ✓ Colors change per phase

### CALIBRATION Mode
- ✓ Real-time pressure bar moves with breathing
- ✓ Min/max markers track extremes
- ✓ Threshold lines auto-adjust (70% of extremes)
- ✓ Numeric values update
- ✓ 5s breath hold saves and returns to LIVE

### STATS Mode
- ✓ Total breaths displayed large
- ✓ Breaths/min calculated correctly
- ✓ Average breath duration shown
- ✓ Session time counts up
- ✓ Mini live indicator shows current breath state

## Common Issues & Solutions

### Display Issues

**Garbled/shifted display:**
- Try different `INITR_` flag in main.cpp:140:
  ```cpp
  tft.initR(INITR_144GREENTAB);  // or try:
  // tft.initR(INITR_BLACKTAB);
  // tft.initR(INITR_REDTAB);
  ```

**Display flickers:**
- Reduce update rate in loop delay (main.cpp:125)
- Check power supply - display needs stable 3.3V

### Pressure Sensor Issues

**Baseline keeps drifting:**
- Temperature changes affect readings
- Seal your breathing chamber better
- Re-calibrate baseline on startup

**No pressure changes detected:**
- Check tube connection to sensor
- Verify chamber is sealed
- Try blocking tube with finger - should show pressure change

**Readings noisy:**
- Increase filtering in main.cpp:151:
  ```cpp
  bmp.setSampling(...,
    Adafruit_BMP280::FILTER_X16  // Increase to FILTER_X16
  );
  ```

### Gesture Problems

**Gestures too easy to trigger accidentally:**
- Increase durations in main.cpp:290, 323, 356
  ```cpp
  if (breathDuration > 2000) {  // Change to 2500 or 3000
  ```

**Gestures don't trigger:**
- Decrease durations
- Check you're breathing forcefully enough to cross thresholds
- Monitor serial output to see if breath states are detected

## Optimization Tips

### Performance

**If display updates are slow:**
- Wave drawing is expensive - reduce complexity
- Lower FPS in drawWave() (currently 30 FPS)
- Use faster ESP32 variant or overclock

### Battery Life (future enhancement)

Ideas for low-power operation:
- Lower display brightness
- Reduce update rates
- Add sleep mode after inactivity
- Use ESP32 deep sleep with motion wake

### Advanced Tuning

**Breath detection algorithm** (main.cpp:234-262):
- `inhaleThreshold` / `exhaleThreshold`: Adjust sensitivity
- Breath cycle counting logic
- Hold detection timeout (currently 3s)

**Visual adjustments:**
- Wave amplitude multiplier (main.cpp:461)
- Color schemes per mode
- Animation speeds

## Next Steps

Once everything works:

1. **Add physical buttons** (optional):
   - Connect to GPIO pins
   - Add debouncing logic
   - Map to mode switching

2. **Enhance features**:
   - Save session history to NVS
   - Multiple breathing patterns in guided mode
   - Biofeedback games/challenges
   - Bluetooth data export

3. **Build an enclosure**:
   - 3D print or laser-cut case
   - Mount display on front
   - Integrate breathing chamber
   - Add button cutouts if using physical controls

## Debugging Checklist

- [ ] Serial monitor shows no errors on boot
- [ ] Display shows calibration screen, then LIVE mode
- [ ] Pressure readings change when breathing through tube
- [ ] Breath states (IN/OUT/HLD) detected correctly
- [ ] Gestures navigate between modes
- [ ] All 4 modes render correctly
- [ ] Calibration saves and persists across reboots
- [ ] Session stats accurate

**Still having issues?** Check:
- PlatformIO library versions
- ESP32 board variant compatibility
- Wiring with multimeter for continuity
- Power supply provides enough current
