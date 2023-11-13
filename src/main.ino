/**
 * @file    main.ino
 * @author  ECE411 Team 5:
 *            Abdulaziz Alateeqi,
 *            Meshal Almutairi,
 *            Flynn Flynn,
 *            Gene Hu.
 * @brief   Audio Visualizer source code.
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <Arduino.h>
#include <FastLED.h>
#include <LEDMatrix.h>

#include "audio_reactive.h"

#define BRIGHTNESS 127  // Set the brightness level (0-255)
#define LED_PIN 2
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define BANDS 16
#define BAR_WIDTH 2
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define MATRIX_SCALE (255 / MATRIX_HEIGHT)

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, VERTICAL_ZIGZAG_MATRIX> led_matrix;

int fft_results[16];  // Results from FFT
uint8_t previous_fft_values[16] = {};
uint8_t bar_heights[16] = {};

void setup() {
  Serial.begin(115200);

  setup_audio();
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(led_matrix[0], NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void draw_band(int band, int y_max) {
  int x_start = BAR_WIDTH * band;

  for (int x = x_start; x < x_start + BAR_WIDTH; x++) {
    for (int y = 0; y <= y_max; y++) {
      led_matrix(x, y) = CHSV(y * MATRIX_SCALE, 255, 255);
    }
  }
}

void loop() {
  FastLED.clear();

  // Process FFT results
  for (int i = 0; i < BANDS; i++) {
    uint8_t fft_value = fft_results[i];
    fft_value = ((previous_fft_values[i] * 3) + fft_value) / 4;  // Smooth
    bar_heights[i] = fft_value / MATRIX_SCALE;  // Scale bar height
    previous_fft_values[i] = fft_value;
  }

  // Draw bars
  for (int band = 0; band < BANDS; band++) {
    draw_band(band, bar_heights[band]);
  }

  FastLED.show();
}
