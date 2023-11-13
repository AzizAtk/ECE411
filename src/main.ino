/**
 * @file    main.c
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
#include "Arduino.h"
#include "audio_reactive.h"

int fft_results[16];

void setup() {
  Serial.begin(115200);
  setup_audio();
}

void loop() {
  Serial.print("[ ");
  for (int i = 0; i < 16; i++) {
    Serial.print(fft_results[i]);

    if (i < 15) Serial.print(", ");
  }
  Serial.println(" ]");

  delay(500);
}