#include "Max30102Reader.h"

Max30102Reader::Max30102Reader()
  : spo2(0),
    spo2Valid(0),
    heartRate(0),
    heartRateValid(0),
    lastSampleMillis(0),
    samplePeriodMs(1000UL / SAMPLE_RATE) {}

void Max30102Reader::begin() {
  // I2C MAX30102 - SDA=21, SCL=22
  Wire.begin(21, 22);
  Wire.setClock(100000);  // 100 kHz

  byte ledBrightness = 0x1F;
  byte sampleAverage = 8;
  byte ledMode       = 2;     
  int  sampleRate    = 400;   // Hz interno
  int  pulseWidth    = 411;
  int  adcRange      = 16384;

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("ERROR: MAX3010x no encontrado. Revisa cableado."));
    while (true) {
      delay(1000);
    }
  }

  sensor.setup(ledBrightness, sampleAverage, ledMode,
               sampleRate, pulseWidth, adcRange);
  sensor.enableFIFORollover();
  sensor.clearFIFO();
}

bool Max30102Reader::collectSample(uint32_t &ir, uint32_t &red) {
  if (millis() - lastSampleMillis < samplePeriodMs) return false;
  lastSampleMillis = millis();

  if (!sensor.available()) {
    sensor.check(); // 
  }
  if (!sensor.available()) return false;

  red = sensor.getRed();
  ir  = sensor.getIR();
  sensor.nextSample();
  return true;
}

void Max30102Reader::fillInitialBuffers() {
  Serial.println(F("Cargando buffer inicial del MAX30102 "));
  Serial.println(F("Mantén el dedo en el sensor, sin moverlo."));

  int filled = 0;
  while (filled < BUFFER_LENGTH) {
    uint32_t ir, red;
    if (collectSample(ir, red)) {
      irBuffer[filled]  = ir;
      redBuffer[filled] = red;
      filled++;
    }
    delay(1);
  }

  Serial.println(F("Buffer inicial correcto. Iniciando lecturas..."));

  // cálculo inicial
  maxim_heart_rate_and_oxygen_saturation(
      irBuffer, BUFFER_LENGTH,
      redBuffer,
      &spo2, &spo2Valid, &heartRate, &heartRateValid);
}

void Max30102Reader::shiftAndAppendSamples(int step) {
  // desplazar a la izquierda y agregar 'step' nuevas muestras
  memmove(irBuffer,  irBuffer  + step, (BUFFER_LENGTH - step) * sizeof(uint32_t));
  memmove(redBuffer, redBuffer + step, (BUFFER_LENGTH - step) * sizeof(uint32_t));

  int idx = BUFFER_LENGTH - step;
  while (idx < BUFFER_LENGTH) {
    uint32_t ir, red;
    if (collectSample(ir, red)) {
      irBuffer[idx]  = ir;
      redBuffer[idx] = red;
      idx++;
    } else {
      delay(1);
    }
  }
}

void Max30102Reader::stepAndComputeAndPrint() {
  // avanzar ~1 s de datos nuevos
  shiftAndAppendSamples(STEP_SAMPLES);

  // recalcular BPM / SpO2
  maxim_heart_rate_and_oxygen_saturation(
      irBuffer, BUFFER_LENGTH,
      redBuffer,
      &spo2, &spo2Valid, &heartRate, &heartRateValid);

  // salida por Serial
  Serial.print(F("IR="));   Serial.print(irBuffer[BUFFER_LENGTH - 1]);
  Serial.print(F("  RED=")); Serial.print(redBuffer[BUFFER_LENGTH - 1]);
  Serial.print(F("  BPM="));
  if (heartRateValid) Serial.print(heartRate); else Serial.print(F("--"));
  Serial.print(F("  SpO2="));
  if (spo2Valid) Serial.print(spo2); else Serial.print(F("--"));
  Serial.println(F("  (mantén el dedo firme)"));
}
