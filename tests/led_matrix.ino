/*
  ESP32 LED Matrix Test
  Test LED Matrix with ESP32 using LEDMatrix and FastLED libraries
*/

#include <FastLED.h>
#include <LEDMatrix.h>

#define LED_PIN 2
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

uint8_t hue;
int16_t counter;

void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(127);
  FastLED.clear(true);
  delay(500);
  FastLED.showColor(CRGB::Red);
  delay(1000);
  FastLED.showColor(CRGB::Lime);
  delay(1000);
  FastLED.showColor(CRGB::Blue);
  delay(1000);
  FastLED.showColor(CRGB::White);
  delay(1000);
  FastLED.clear(true);

  hue = 0;
  counter = 0;
}

void loop() {
  int16_t sx, sy, x, y;
  uint8_t h;

  FastLED.clear();

  // Light corner pixels
  leds(0, 0) = CRGB::Red;
  leds(31, 0) = CRGB::Blue;
  leds(0, 7) = CRGB::Green;
  leds(31, 7) = CRGB::White;

  FastLED.show();
}
