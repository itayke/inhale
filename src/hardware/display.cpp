#include "display.h"
#include "../config.h"
#include <Adafruit_GFX.h>

static Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Shared canvas for double-buffering (32KB) - used by all modes
static GFXcanvas16 canvas(SCREEN_WIDTH, SCREEN_HEIGHT);

void displayInit() {
  Serial.println("Initializing ST7735S display...");

  tft.initR(INITR_144GREENTAB); // Use GREENTAB for 128x128
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);

  Serial.println("ST7735S display initialized");
}

Adafruit_ST7735& getDisplay() {
  return tft;
}

GFXcanvas16& getCanvas() {
  return canvas;
}

void displayBlit() {
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
}

void displayClear() {
  tft.fillScreen(ST77XX_BLACK);
}

void displayShowMessage(const char* message, uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 60);
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.println(message);
}

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
