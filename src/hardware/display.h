#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_ST7735.h>

// Initialize ST7735 display
void displayInit();

// Get reference to display object for drawing
Adafruit_ST7735& getDisplay();

// Utility functions
void displayClear();
void displayShowMessage(const char* message, uint16_t color);

#endif // DISPLAY_H
