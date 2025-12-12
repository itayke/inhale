#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>

class Display {
public:
  // Initialize ST7735 display
  void init();

  // Get reference to TFT for direct drawing
  Adafruit_ST7735& getTft();

  // Get reference to canvas for double-buffered drawing
  GFXcanvas16& getCanvas();

  // Blit canvas to display
  void blit();

  // Clear screen to black
  void clear();

  // Show a message centered on screen
  void showMessage(const char* message, uint16_t color);

  // Convert RGB to 565 format
  static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
};

// Global display instance (defined in main.cpp)
extern Display display;

#endif // DISPLAY_H
