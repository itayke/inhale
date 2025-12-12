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
3. **Hold breath** - Wave should stabilize, turn purple after 3s

**Watch serial monitor** for breath state changes and calibration values.

### Normalization

The system uses asymmetric normalization:
- Inhale maps to -1 (at max observed inhale strength)
- Exhale maps to +1 (at max observed exhale strength)
- Calibration bounds auto-expand as you breathe

**Test normalization:**
1. Start breathing normally
2. Watch min/max bounds in diagnostic mode
3. Breathe more forcefully - bounds should expand
4. Normalized value should stay within -1 to +1

### Sensitivity Issues?

If breath detection is too sensitive or not sensitive enough:

**Adjust thresholds in code** (`config.h`):
```cpp
#define DEFAULT_INHALE_THRESHOLD  -5.0f   // More negative = less sensitive
#define DEFAULT_EXHALE_THRESHOLD   5.0f   // More positive = less sensitive
```

## Testing Each Mode

### LIVE Mode
- Wave animates smoothly
- Wave height responds to breathing (normalized)
- Colors change with breath state:
  - Inhale: Deep blue
  - Exhale: Cyan
  - Hold: Purple
  - Idle: Medium blue
- Breath count increments on exhale-to-inhale transition
- State indicator (IN/OUT/HLD/...) updates

### DIAGNOSTIC Mode
- Pressure delta updates in real-time
- Normalized value bar moves with breathing
- Absolute pressure displayed in inHg
- Temperature shown in both C and F
- Min/Max calibration bounds shown at bottom

## Common Issues & Solutions

### Display Issues

**Garbled/shifted display:**
- Try different `INITR_` flag in `Display.cpp`:
  ```cpp
  tft.initR(INITR_144GREENTAB);  // or try:
  // tft.initR(INITR_BLACKTAB);
  // tft.initR(INITR_REDTAB);
  ```

**Display flickers:**
- Display uses double-buffering, so flickering is unlikely
- Check power supply - display needs stable 3.3V

### Pressure Sensor Issues

**Baseline keeps drifting:**
- Temperature changes affect readings
- Seal your breathing chamber better
- BMP280 has hardware filtering (FILTER_X16) to reduce noise

**No pressure changes detected:**
- Check tube connection to sensor chamber
- Verify chamber is sealed
- Try blocking tube with finger - should show pressure change

**Readings noisy:**
- BMP280 is configured with maximum filtering
- Check chamber seal
- Ensure stable power supply

### Normalization Issues

**Values stuck at 0:**
- Min/max bounds need to exceed 0.1 Pa threshold
- Breathe more forcefully to establish bounds

**Values always at extremes (-1 or +1):**
- Initial bounds are -10/+10 Pa
- As you breathe, bounds expand to match your range
- This is expected behavior - normalization adapts to you

## Optimization Tips

### Performance

**If display updates are slow:**
- Wave drawing uses double-buffering at 30 FPS
- Diagnostic mode runs at 10 FPS
- Both are optimized for smooth operation

### Advanced Tuning

**Breath detection** (`config.h`):
- `DEFAULT_INHALE_THRESHOLD` / `DEFAULT_EXHALE_THRESHOLD`: Detection sensitivity
- `BREATH_HOLD_TIMEOUT_MS`: Time before hold state triggers (default 3s)
- `BREATH_HOLD_STABILITY_PA`: Pressure stability threshold for hold detection

**Update rates** (`config.h`):
- `MAIN_LOOP_DELAY_MS`: Main loop timing (default 20ms = ~50Hz)
- `WAVE_UPDATE_FPS`: Live mode refresh rate (default 30)
- `DIAGNOSTIC_UPDATE_FPS`: Diagnostic mode refresh rate (default 10)

**Visual adjustments** (`live_mode.cpp`):
- `maxDisplacement`: Wave height range (default 50 pixels)
- Wave colors per breath state

## Debugging Checklist

- [ ] Serial monitor shows no errors on boot
- [ ] Display shows calibration message, then LIVE mode
- [ ] Pressure readings change when breathing through tube
- [ ] Breath states (IN/OUT/HLD) detected correctly
- [ ] Wave responds to breathing in LIVE mode
- [ ] Normalized values shown correctly in DIAGNOSTIC mode
- [ ] Min/max bounds expand with breathing
- [ ] Calibration thresholds persist across reboots

**Still having issues?** Check:
- PlatformIO library versions
- ESP32 board variant compatibility
- Wiring with multimeter for continuity
- Power supply provides enough current
