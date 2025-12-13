#ifndef SPI_H_SHIM
#define SPI_H_SHIM

// SPI.h stub for simulator
// GFXcanvas16 doesn't use SPI, but Adafruit_GFX.h includes Adafruit_BusIO

#include <cstdint>

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

// BitOrder enum - must be defined before MSBFIRST/LSBFIRST
typedef enum {
  LSBFIRST = 0,
  MSBFIRST = 1
} BitOrder;

class SPISettings {
public:
  SPISettings() {}
  SPISettings(uint32_t, uint8_t, uint8_t) {}
};

class SPIClass {
public:
  void begin() {}
  void end() {}
  void beginTransaction(SPISettings) {}
  void endTransaction() {}
  uint8_t transfer(uint8_t data) { return data; }
  void transfer(void*, size_t) {}
};

extern SPIClass SPI;

#endif // SPI_H_SHIM
