// Simulator implementation of Display
// Uses Adafruit GFXcanvas16 for rendering, SDL2 for display
#include "Display.h"
#include "config.h"
#include "Platform.h"

static GFXcanvas16* canvas = nullptr;
static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* texture = nullptr;
static const int SCALE = 4;

void Display::init() {
  Serial.println("Initializing SDL2 display...");

  window = SDL_CreateWindow(
    "Inhale Simulator",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE,
    SDL_WINDOW_SHOWN
  );

  if (!window) {
    Serial.print("SDL_CreateWindow failed: ");
    Serial.println(SDL_GetError());
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    Serial.print("SDL_CreateRenderer failed: ");
    Serial.println(SDL_GetError());
    return;
  }

  texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGB565,
    SDL_TEXTUREACCESS_STREAMING,
    SCREEN_WIDTH, SCREEN_HEIGHT
  );

  if (!texture) {
    Serial.print("SDL_CreateTexture failed: ");
    Serial.println(SDL_GetError());
    return;
  }

  // Use the real Adafruit GFXcanvas16!
  canvas = new GFXcanvas16(SCREEN_WIDTH, SCREEN_HEIGHT);

  Serial.println("SDL2 display initialized successfully!");
}

Canvas& Display::getCanvas() {
  return *canvas;
}

void Display::blit() {
  // Copy GFXcanvas16 buffer directly to SDL texture
  SDL_UpdateTexture(texture, nullptr, canvas->getBuffer(), SCREEN_WIDTH * sizeof(uint16_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}

void Display::clear() {
  canvas->fillScreen(ST77XX_BLACK);
  blit();
}

void Display::showMessage(const char* message, uint16_t color) {
  canvas->fillScreen(ST77XX_BLACK);
  canvas->setCursor(10, SCREEN_HEIGHT / 2 - 10);
  canvas->setTextColor(color);
  canvas->setTextSize(1);
  canvas->print(message);
  blit();
}

uint16_t Display::rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
