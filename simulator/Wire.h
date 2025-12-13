#ifndef WIRE_H_SHIM
#define WIRE_H_SHIM

// Wire.h stub for simulator
// GFXcanvas16 doesn't use I2C, but Adafruit_GFX.h includes Adafruit_BusIO

#include <cstdint>

class TwoWire {
public:
  void begin() {}
  void beginTransmission(uint8_t) {}
  uint8_t endTransmission(bool = true) { return 0; }
  size_t write(uint8_t) { return 1; }
  size_t write(const uint8_t*, size_t len) { return len; }
  uint8_t requestFrom(uint8_t, uint8_t) { return 0; }
  int available() { return 0; }
  int read() { return -1; }
  void setClock(uint32_t) {}
};

extern TwoWire Wire;

#endif // WIRE_H_SHIM
