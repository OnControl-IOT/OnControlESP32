#include <Arduino.h>
#include "Max30102Reader.h"
#include "Mlx90614Reader.h"

enum AppState {
  WAIT_MAX_PROMPT,
  READING_MAX,
  WAIT_MLX_PROMPT,
  READING_MLX,
  DONE
};

AppState appState = WAIT_MAX_PROMPT;
unsigned long stateStartMs = 0;

Max30102Reader maxReader;
Mlx90614Reader tempReader;

// Verificar el si
bool userSaidYes() {
  if (!Serial.available()) return false;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "si" || cmd == "s") {
    return true;
  }

  Serial.println(F("Entrada no reconocida. Escribe 'si' o 's' y presiona ENTER."));
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; 
  }

  Serial.println(F("\n ESP32 + MAX30102 + MLX90614 "));
  Serial.println();

  // Se inicia los sensores
  maxReader.begin();
  tempReader.begin();  

  Serial.println(F("\nSensores inicializados correctamente.\n"));
  Serial.println(F("¿Deseas comenzar la lectura de BPM y SpO2?"));
  Serial.println(F("Escribe 'si' o 's' y presiona ENTER para iniciar."));

  appState = WAIT_MAX_PROMPT;
}

void loop() {
  switch (appState) {

    case WAIT_MAX_PROMPT:
      if (userSaidYes()) {
        Serial.println(F("\nIniciando lectura del MAX30102 por 20 segundos "));
        maxReader.fillInitialBuffers();   
        stateStartMs = millis();
        appState = READING_MAX;
      }
      break;

    case READING_MAX: {
      unsigned long elapsed = millis() - stateStartMs;
      if (elapsed >= 20000UL) {  // 20 segundos
        Serial.println(F("\nLectura del MAX30102 finalizada."));
        Serial.println(F("--------------------------------------------\n"));
        Serial.println(F("¿Deseas comenzar la lectura de la temperatura?"));
        Serial.println(F("Escribe 'si' o 's' y presiona ENTER para iniciar."));
        appState = WAIT_MLX_PROMPT;
        break;
      }

      // Un paso de actualización + impresión
      maxReader.stepAndComputeAndPrint();

      delay(10);
    }
    break;

    case WAIT_MLX_PROMPT:
      if (userSaidYes()) {
        Serial.println(F("\nIniciando lectura del MLX90614 por 10 segundos "));
        stateStartMs = millis();
        appState = READING_MLX;
      }
      break;

    case READING_MLX: {
      unsigned long elapsed = millis() - stateStartMs;
      if (elapsed >= 10000UL) {  // 10 segundos
        Serial.println(F("\nLectura del MLX90614 finalizada."));
        Serial.println(F("--------------------------------------------"));
        Serial.println(F("Lecturas finalizadas"));
        appState = DONE;
        break;
      }

      tempReader.printOnce();   // imprime temperaturas
      delay(500);               
    }
    break;

    case DONE:
      // Serial.println(F("\n¿Reiniciar flujo desde BPM y SpO2? Escribe 'si' o 's'."));
      // appState = WAIT_MAX_PROMPT;
      break;
  }
}
