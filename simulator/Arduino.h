#ifndef ARDUINO_H_SHIM
#define ARDUINO_H_SHIM

// Arduino.h compatibility shim for simulator
// Provides types and macros needed by Adafruit GFX

#include "Platform.h"
#include "Print.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>

// PROGMEM support (AVR flash memory - just use regular memory on desktop)
#define PROGMEM
#define pgm_read_byte(addr)   (*(const uint8_t *)(addr))
#define pgm_read_word(addr)   (*(const uint16_t *)(addr))
#define pgm_read_dword(addr)  (*(const uint32_t *)(addr))
#define pgm_read_pointer(addr) (*(const void **)(addr))

// String helper
#define F(x) (x)

// __FlashStringHelper - used for F() macro strings on AVR
class __FlashStringHelper;

// Arduino String class (simplified)
class String : public std::string {
public:
  String() : std::string() {}
  String(const char* s) : std::string(s ? s : "") {}
  String(const std::string& s) : std::string(s) {}
  String(int n) : std::string(std::to_string(n)) {}
  String(unsigned int n) : std::string(std::to_string(n)) {}
  String(long n) : std::string(std::to_string(n)) {}
  String(unsigned long n) : std::string(std::to_string(n)) {}
  String(float n, int d = 2) { char buf[32]; snprintf(buf, sizeof(buf), "%.*f", d, n); assign(buf); }
  String(double n, int d = 2) { char buf[32]; snprintf(buf, sizeof(buf), "%.*f", d, n); assign(buf); }

  const char* c_str() const { return std::string::c_str(); }
  unsigned int length() const { return (unsigned int)std::string::length(); }
};

// Min/max (Adafruit GFX uses these)
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

#endif // ARDUINO_H_SHIM
