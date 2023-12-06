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

#define BRIGHTNESS 32  // Set the brightness level (0-255)
#define LED_PIN 26
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define BANDS 16
#define BAR_WIDTH 2
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, VERTICAL_ZIGZAG_MATRIX> led_matrix;

typedef struct {
  int x;
  int y;
} Coordinate;

int fft_results[16];  // Results from FFT
uint8_t previous_fft_values[16] = {};
uint8_t bar_heights[16] = {};
uint8_t peaks[16] = {};
uint64_t color_shift = 0;

void setup() {
  Serial.begin(115200);

  setup_audio();
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(led_matrix[0], NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  draw_start_screen();
}

void loop() {
  fadeToBlackBy(led_matrix[0], NUM_LEDS, 32);

  // Process FFT results
  for (int i = 0; i < BANDS; i++) {
    uint8_t fft_value = fft_results[i];
    fft_value = ((previous_fft_values[i] * 3) + fft_value) / 4;  // Smooth
    bar_heights[i] = fft_value / (255 / MATRIX_HEIGHT);          // Scale bar height
    previous_fft_values[i] = fft_value;

    if (bar_heights[i] > peaks[i]) {
      peaks[i] = min((int)bar_heights[i], MATRIX_HEIGHT - 1);
    }
  }

  // Draw bars
  for (int band = 0; band < BANDS; band++) {
    draw_bar(band, bar_heights[band]);
  }

  // Decay peaks
  EVERY_N_MILLISECONDS(200) {
    for (int i = 0; i < BANDS; i++) {
      if (peaks[i] > 0) {
        peaks[i]--;
      }
    }
  }

  // Shift colors over time
  EVERY_N_MILLISECONDS(250) {
    color_shift++;
  }

  FastLED.show();
}

/**
 * @brief Draw a bar on the LED matrix.
 *
 * @param band    Band number.
 * @param height  Bar height.
 */
void draw_bar(int band, int height) {
  int x_start = BAR_WIDTH * band;
  int y_max = MATRIX_HEIGHT - 1;

  for (int x = x_start; x < x_start + BAR_WIDTH; x++) {
    CHSV color = CHSV(band * 16 + color_shift, 255, 255);

    for (int y = 0; y <= height; y++) {
      led_matrix(x, y_max - y) = color;
    }

    // Draw peak
    if (peaks[band] > 0) {
      color.s /= 2.5;  // Desaturate
      led_matrix(x, y_max - peaks[band]) = color;
    }
  }
}

/**
 * @brief Draw the start screen.
 */
void draw_start_screen() {
  Coordinate ece411[] = {{2, 1},  {3, 1},  {4, 1},  {5, 1},  {8, 1},  {9, 1},  {12, 1}, {13, 1}, {14, 1}, {15, 1}, {20, 1}, {24, 1}, {28, 1},
                         {2, 2},  {7, 2},  {10, 2}, {12, 2}, {19, 2}, {20, 2}, {23, 2}, {24, 2}, {27, 2}, {28, 2}, {2, 3},  {3, 3},  {4, 3},
                         {7, 3},  {12, 3}, {13, 3}, {14, 3}, {18, 3}, {20, 3}, {24, 3}, {28, 3}, {2, 4},  {7, 4},  {10, 4}, {12, 4}, {18, 4},
                         {19, 4}, {20, 4}, {21, 4}, {24, 4}, {28, 4}, {2, 5},  {3, 5},  {4, 5},  {5, 5},  {8, 5},  {9, 5},  {12, 5}, {13, 5},
                         {14, 5}, {15, 5}, {20, 5}, {23, 5}, {24, 5}, {25, 5}, {27, 5}, {28, 5}, {29, 5}};
  int ece411_size = sizeof(ece411) / sizeof(ece411[0]);

  Coordinate team5[] = {
      {2, 1},  {3, 1},  {4, 1},  {5, 1},  {6, 1},  {8, 1},  {9, 1},  {10, 1}, {11, 1}, {14, 1}, {15, 1}, {18, 1}, {22, 1}, {26, 1}, {27, 1},
      {28, 1}, {29, 1}, {4, 2},  {8, 2},  {13, 2}, {16, 2}, {18, 2}, {19, 2}, {21, 2}, {22, 2}, {26, 2}, {4, 3},  {8, 3},  {9, 3},  {10, 3},
      {13, 3}, {14, 3}, {15, 3}, {16, 3}, {18, 3}, {20, 3}, {22, 3}, {26, 3}, {27, 3}, {28, 3}, {4, 4},  {8, 4},  {13, 4}, {16, 4}, {18, 4},
      {22, 4}, {29, 4}, {4, 5},  {8, 5},  {9, 5},  {10, 5}, {11, 5}, {13, 5}, {16, 5}, {18, 5}, {22, 5}, {26, 5}, {27, 5}, {28, 5},
  };
  int team5_size = sizeof(team5) / sizeof(team5[0]);

  FastLED.clear(true);
  delay(500);
  FastLED.showColor(CRGB::Red);
  delay(500);
  FastLED.showColor(CRGB::Green);
  delay(500);
  FastLED.showColor(CRGB::Blue);
  delay(500);
  FastLED.showColor(CRGB::White);
  delay(500);

  // Draw 'ECE 411'
  fill_solid(led_matrix[0], NUM_LEDS, CRGB(32, 0, 0));
  for (int i = 0; i < ece411_size; i++) {
    led_matrix(ece411[i].x, ece411[i].y) = CRGB::White;
  }
  led_matrix.DrawLine(0, 7, 31, 7, CRGB::Black);
  FastLED.show();
  delay(2000);

  FastLED.clear(true);
  delay(250);

  // Draw 'Team 5'
  fill_solid(led_matrix[0], NUM_LEDS, CRGB(32, 0, 0));
  for (int i = 0; i < team5_size; i++) {
    led_matrix(team5[i].x, team5[i].y) = CRGB::White;
  }
  led_matrix.DrawLine(0, 7, 31, 7, CRGB::Black);
  FastLED.show();

  delay(2000);
  FastLED.clear(true);
  delay(500);
}
