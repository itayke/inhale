#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>

// Initialize ST7735 display
void displayInit();

// Get reference to display object for drawing
Adafruit_ST7735& getDisplay();

// Get reference to shared canvas (for double-buffering)
GFXcanvas16& getCanvas();

// Blit shared canvas to display
void displayBlit();

// Utility functions
void displayClear();
void displayShowMessage(const char* message, uint16_t color);

// Helper to convert RGB to 565 format
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);

#endif // DISPLAY_H
