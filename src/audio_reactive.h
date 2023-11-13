/**
 * @file  audio_reactive.h
 * @brief Setup I2S and FFT to get audio data from the digital microphone and
 *       process it.
 *
 * @note  Based on WLED by atuline: https://github.com/atuline/WLED.
 *        FFT task is pinned to core 0.
 *
 */

#ifndef AUDIO_REACTIVE_H
#define AUDIO_REACTIVE_H

#include <arduinoFFT.h>
#include <driver/i2s.h>

// Audio configuration
#define GAIN 20    // Gain, boosts input level
#define SQUELCH 5  // Squelch, cuts out low level sounds

// I2S configuration
#define I2S_WS_PIN 15
#define I2S_SD_PIN 32
#define I2S_SCK_PIN 14
#define I2S_PORT I2S_NUM_0
#define MIN_SHOW_DELAY 15

// FFT configuration
const int block_size = 64;
const int sample_rate = 10240;
const uint16_t num_samples = 512;  // Must be a power of 2

TaskHandle_t fft_task_handle;

// FFT related variables
double major_peak = 0;
double magnitude = 0;
uint16_t raw_mic_data;  // Raw mic data

unsigned int sampling_period_us;
unsigned long microseconds;

// FFT arrays
extern int fft_results[16];  // Stores the final result
double real_values[num_samples];
double imag_values[num_samples];
double fft_bins[num_samples];
double fft_calculations[16];
double fft_result_max[16];

// Noise adjustment arrays
int linear_noise[16] = {34, 28, 26, 25, 20, 12, 9, 6, 4, 4, 3, 2, 2, 2, 2, 2};
double pink_noise_adjust[16] = {1.70, 1.71, 1.73, 1.78, 1.68, 1.56, 1.55, 1.63,
                                1.79, 1.62, 1.80, 2.06, 2.47, 3.35, 6.83, 9.55};

arduinoFFT FFT = arduinoFFT(real_values, imag_values, num_samples, sample_rate);

// Function prototypes
void setup_audio();
void i2s_install();
void i2s_setpin();
void test_microphone();
void read_microphone_data();
void execute_fft();
void post_process_fft();
double sum_fft_bins(int start, int end);
void fft_task(void* parameter);

void setup_audio() {
  i2s_install();
  i2s_setpin();
  delay(100);
  test_microphone();

  // Set sampling frequency
  sampling_period_us = round(1000000 * (1.0 / sample_rate));

  // Define the FFT Task and pin it to core 0
  xTaskCreatePinnedToCore(fft_task,          // Task function
                          "FFT Task",        // Task name
                          10000,             // Stack size in words
                          NULL,              // Task input parameter
                          1,                 // Task priority
                          &fft_task_handle,  // Task handle
                          0                  // Task core
  );
}

void i2s_install() {
  esp_err_t err;

  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),  // Receive mode
      .sample_rate = sample_rate * 2,  // Double the sample rate
      .bits_per_sample = i2s_bits_per_sample_t(16),
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,  // Interrupt level 1
      .dma_buf_count = 8,
      .dma_buf_len = block_size,
      .use_apll = false};

  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    exit(1);
  }

  Serial.println("I2S driver installed.");
}

void i2s_setpin() {
  esp_err_t err;

  const i2s_pin_config_t pin_config = {.bck_io_num = I2S_SCK_PIN,
                                       .ws_io_num = I2S_WS_PIN,
                                       .data_out_num = -1,  // Not used
                                       .data_in_num = I2S_SD_PIN};

  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    exit(1);
  }

  Serial.println("I2S pins configured.");
}

void test_microphone() {
  float mean = 0.0;
  int32_t sample_buffer[block_size];
  size_t bytes_read = 0;

  esp_err_t result = i2s_read(I2S_PORT, &sample_buffer, block_size, &bytes_read,
                              portMAX_DELAY);

  int samples_read = bytes_read / sizeof(int32_t);
  if (samples_read > 0) {
    for (int i = 0; i < samples_read; ++i) {
      mean += sample_buffer[i];
    }
    mean = mean / samples_read / 16384;
    Serial.println(mean != 0.0 ? "Digital microphone is working"
                               : "Digital microphone is not working");
  }
}

void read_microphone_data() {
  delay(1);
  microseconds = micros();

  for (int i = 0; i < num_samples; i++) {
    int32_t digital_sample = 0;
    size_t bytes_read = 0;
    esp_err_t result = i2s_read(I2S_PORT, &digital_sample,
                                sizeof(digital_sample), &bytes_read, 10);
    if (bytes_read > 0) {
      raw_mic_data = abs(digital_sample >> 16);
    }

    real_values[i] = raw_mic_data;
    imag_values[i] = 0;
    microseconds += sampling_period_us;
  }
}

void execute_fft() {
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // Weigh data
  FFT.Compute(FFT_FORWARD);                         // Compute FFT
  FFT.ComplexToMagnitude();                         // Compute magnitudes

  FFT.MajorPeak(&major_peak, &magnitude);  // Find most dominant frequency

  for (int i = 0; i < num_samples; i++) {
    double temp = abs(real_values[i]);
    temp /= 16.0;  // Reduce magnitude for linear scaling
    fft_bins[i] = temp;
  }
}

void post_process_fft() {
  // Define the bin ranges for each segment
  const int bin_ranges[16][2] = {
      {3, 4},      // 60 - 100 Hz
      {4, 5},      // 80 - 120 Hz
      {5, 7},      // 100 - 160 Hz
      {7, 9},      // 140 - 200 Hz
      {9, 12},     // 180 - 260 Hz
      {12, 16},    // 240 - 340 Hz
      {16, 21},    // 320 - 440 Hz
      {21, 28},    // 420 - 600 Hz
      {29, 37},    // 580 - 760 Hz
      {37, 48},    // 740 - 980 Hz
      {48, 64},    // 960 - 1300 Hz
      {64, 84},    // 1280 - 1700 Hz
      {84, 111},   // 1680 - 2240 Hz
      {111, 147},  // 2220 - 2960 Hz
      {147, 194},  // 2940 - 3900 Hz
      {194, 255}   // 3880 - 5120 Hz
  };
  const int divisors[16] = {2,  2,  3,  3,  4,  5,  6,  8,
                            10, 12, 17, 21, 28, 37, 48, 62};

  // Process each frequency range
  for (int i = 0; i < 16; i++) {
    fft_calculations[i] =
        sum_fft_bins(bin_ranges[i][0], bin_ranges[i][1]) / divisors[i];
    fft_calculations[i] =
        max(fft_calculations[i] - (float)SQUELCH * (float)linear_noise[i] / 4.0,
            0.0);
    fft_calculations[i] *= pink_noise_adjust[i];  // Adjust for pink noise
    fft_calculations[i] =
        fft_calculations[i] * GAIN / 40 + fft_calculations[i] / 16.0;
    fft_results[i] = constrain((int)fft_calculations[i], 0, 254);
  }
}

double sum_fft_bins(int start, int end) {
  double sum = 0.0;
  for (int i = start; i <= end; i++) {
    sum += fft_bins[i];
  }
  return sum;
}

void fft_task(void* parameter) {
  for (;;) {
    read_microphone_data();
    execute_fft();
    post_process_fft();
  }
}

#endif  // AUDIO_REACTIVE_H