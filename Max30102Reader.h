#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

class Max30102Reader {
public:
  Max30102Reader();

  void begin();                 // inicializa I2C y el sensor
  void fillInitialBuffers();    // llena el buffer inicial ~4 s
  void stepAndComputeAndPrint(); // avanza la consola, calcula y muestra BPM/SpO2

private:
  MAX30105 sensor;

  static const byte SAMPLE_RATE   = 25;   // Hz
  static const int  BUFFER_LENGTH = 100;  // ~4 s de datos
  static const int  STEP_SAMPLES  = 25;   // actualiza cada ~1 s

  uint32_t irBuffer[BUFFER_LENGTH];
  uint32_t redBuffer[BUFFER_LENGTH];

  int32_t spo2;
  int8_t  spo2Valid;
  int32_t heartRate;
  int8_t  heartRateValid;

  uint32_t lastSampleMillis;
  uint32_t samplePeriodMs;

  bool collectSample(uint32_t &ir, uint32_t &red);
  void shiftAndAppendSamples(int step);
};
