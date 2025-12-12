#include "Display.h"
#include "config.h"

static Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
static GFXcanvas16 canvas(SCREEN_WIDTH, SCREEN_HEIGHT);

void Display::init() {
  Serial.println("Initializing ST7735S display...");

  tft.initR(INITR_144GREENTAB);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);

  Serial.println("ST7735S display initialized");
}

Adafruit_ST7735& Display::getTft() {
  return tft;
}

GFXcanvas16& Display::getCanvas() {
  return canvas;
}

void Display::blit() {
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
}

void Display::clear() {
  tft.fillScreen(ST77XX_BLACK);
}

void Display::showMessage(const char* message, uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 60);
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.println(message);
}

uint16_t Display::rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
