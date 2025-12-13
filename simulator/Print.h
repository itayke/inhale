#ifndef PRINT_H_SHIM
#define PRINT_H_SHIM

// Print.h compatibility shim for simulator
// Provides Print class needed by Adafruit GFX

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdint>

// Base number formats
#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

// Print class - base class for text output (Adafruit GFX inherits from this)
class Print {
public:
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t *buffer, size_t size) {
    size_t n = 0;
    while (size--) {
      if (write(*buffer++)) n++;
      else break;
    }
    return n;
  }
  size_t write(const char *str) {
    if (str == nullptr) return 0;
    return write((const uint8_t *)str, strlen(str));
  }
  size_t print(const char s[]) { return write(s); }
  size_t print(char c) { return write(c); }
  size_t print(int n, int base = 10) {
    char buf[16];
    snprintf(buf, sizeof(buf), base == 16 ? "%x" : "%d", n);
    return write(buf);
  }
  size_t print(unsigned int n, int base = 10) {
    char buf[16];
    snprintf(buf, sizeof(buf), base == 16 ? "%x" : "%u", n);
    return write(buf);
  }
  size_t print(long n, int base = 10) {
    char buf[24];
    snprintf(buf, sizeof(buf), base == 16 ? "%lx" : "%ld", n);
    return write(buf);
  }
  size_t print(unsigned long n, int base = 10) {
    char buf[24];
    snprintf(buf, sizeof(buf), base == 16 ? "%lx" : "%lu", n);
    return write(buf);
  }
  size_t print(double n, int digits = 2) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", digits, n);
    return write(buf);
  }
  size_t println(const char s[] = "") { size_t n = print(s); n += write('\n'); return n; }
  size_t println(char c) { size_t n = print(c); n += write('\n'); return n; }
  size_t println(int n, int base = 10) { size_t r = print(n, base); r += write('\n'); return r; }
  size_t println(unsigned int n, int base = 10) { size_t r = print(n, base); r += write('\n'); return r; }
  size_t println(long n, int base = 10) { size_t r = print(n, base); r += write('\n'); return r; }
  size_t println(unsigned long n, int base = 10) { size_t r = print(n, base); r += write('\n'); return r; }
  size_t println(double n, int digits = 2) { size_t r = print(n, digits); r += write('\n'); return r; }
};

#endif // PRINT_H_SHIM
