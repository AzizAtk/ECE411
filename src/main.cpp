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

void setup() { pinMode(LED_BUILTIN, OUTPUT); }

void loop() {
  // Test board
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);

  delay(1000);
}