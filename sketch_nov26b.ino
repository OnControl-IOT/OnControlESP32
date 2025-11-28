#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "Max30102Reader.h"
#include "Mlx90614Reader.h"

// ============================================
// CONFIGURACION - MODIFICAR SEGUN TU RED
// ============================================
const char* WIFI_SSID     = "ROMI";
const char* WIFI_PASSWORD = "Surco2024";
const char* API_URL       = "http://pruebaiot.free.beeceptor.com/api/v1/data-records/";

// ============================================
// TIEMPOS DE LECTURA (en milisegundos)
// ============================================
const unsigned long TIEMPO_LECTURA_MAX = 40000;  // 40 segundos para BPM/SpO2
const unsigned long TIEMPO_LECTURA_MLX = 10000;  // 10 segundos para temperatura

// ============================================
// MAQUINA DE ESTADOS
// ============================================
enum AppState {
  STATE_INIT,
  STATE_WAIT_MAX_PROMPT,
  STATE_READING_MAX,
  STATE_WAIT_MLX_PROMPT,
  STATE_READING_MLX,
  STATE_SENDING_DATA,
  STATE_DONE
};

AppState appState = STATE_INIT;
unsigned long stateStartMs = 0;

// ============================================
// SENSORES
// ============================================
Max30102Reader maxReader;
Mlx90614Reader tempReader;

// ============================================
// ACUMULADORES PARA PROMEDIOS
// ============================================
struct VitalSigns {
  float bpmSum;
  int bpmCount;
  float spo2Sum;
  int spo2Count;
  double tempSum;
  int tempCount;

  void reset() {
    bpmSum = 0; bpmCount = 0;
    spo2Sum = 0; spo2Count = 0;
    tempSum = 0; tempCount = 0;
  }

  float getBpmAvg() { return (bpmCount > 0) ? bpmSum / bpmCount : 0; }
  float getSpo2Avg() { return (spo2Count > 0) ? spo2Sum / spo2Count : 0; }
  double getTempAvg() { return (tempCount > 0) ? tempSum / tempCount : 0; }
} vitals;

// ============================================
// FUNCIONES AUXILIARES
// ============================================

void printHeader(const char* title) {
  Serial.println(F(""));
  Serial.println(F("============================================"));
  Serial.print(F("  "));
  Serial.println(title);
  Serial.println(F("============================================"));
  Serial.println(F(""));
}

void printProgress(unsigned long elapsed, unsigned long total) {
  int percent = (elapsed * 100) / total;
  int seconds = (total - elapsed) / 1000;
  
  Serial.print(F("["));
  for (int i = 0; i < 20; i++) {
    if (i < percent / 5) Serial.print(F("#"));
    else Serial.print(F("-"));
  }
  Serial.print(F("] "));
  Serial.print(percent);
  Serial.print(F("% - "));
  Serial.print(seconds);
  Serial.println(F("s restantes"));
}

bool waitForUserConfirmation() {
  if (!Serial.available()) return false;

  String input = Serial.readStringUntil('\n');
  input.trim();
  input.toLowerCase();

  if (input == "si" || input == "s" || input == "yes" || input == "y") {
    return true;
  }

  Serial.println(F("Escribe 'si' o 's' para continuar."));
  return false;
}

void connectWiFi() {
  Serial.print(F("Conectando a WiFi"));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int retries = 30;
  while (WiFi.status() != WL_CONNECTED && retries-- > 0) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("Conectado! IP: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("No se pudo conectar a WiFi (continuando offline)"));
  }
}

void sendToAPI(float bpm, float spo2, double temp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi desconectado. No se pueden enviar datos."));
    return;
  }

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");

  // Construir JSON
  String json = "{";
  json += "\"bpm\":" + String(bpm, 1) + ",";
  json += "\"spo2\":" + String(spo2, 1) + ",";
  json += "\"temperatura\":" + String(temp, 2);
  json += "}";

  Serial.print(F("Enviando: "));
  Serial.println(json);

  int httpCode = http.POST(json);
  
  if (httpCode > 0) {
    Serial.print(F("Respuesta HTTP: "));
    Serial.println(httpCode);
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
      Serial.println(F("Datos enviados exitosamente!"));
    }
  } else {
    Serial.print(F("Error HTTP: "));
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  printHeader("ESP32 Monitor de Signos Vitales");
  Serial.println(F("MAX30102 (BPM, SpO2) + MLX90614 (Temperatura)"));
  Serial.println(F(""));

  // Conectar WiFi
  connectWiFi();
  Serial.println(F(""));

  // Inicializar sensores
  Serial.println(F("Inicializando sensores..."));
  maxReader.begin();
  tempReader.begin();
  Serial.println(F(""));

  appState = STATE_WAIT_MAX_PROMPT;
}

// ============================================
// LOOP - MAQUINA DE ESTADOS
// ============================================
void loop() {
  switch (appState) {

    // ----------------------------------------
    case STATE_WAIT_MAX_PROMPT:
      Serial.println(F("Listo para medir BPM y SpO2."));
      Serial.println(F("Escribe 'si' para comenzar:"));
      appState = STATE_INIT;  // Esperar input
      break;

    // ----------------------------------------
    case STATE_INIT:
      if (waitForUserConfirmation()) {
        vitals.reset();
        maxReader.fillInitialBuffers();
        stateStartMs = millis();
        appState = STATE_READING_MAX;
      }
      break;

    // ----------------------------------------
    case STATE_READING_MAX: {
      unsigned long elapsed = millis() - stateStartMs;
      
      if (elapsed >= TIEMPO_LECTURA_MAX) {
        printHeader("Lectura MAX30102 Completada");
        Serial.println(F("Listo para medir temperatura."));
        Serial.println(F("Escribe 'si' para continuar:"));
        appState = STATE_WAIT_MLX_PROMPT;
        break;
      }

      // Leer y procesar datos
      maxReader.stepAndComputeAndPrint();

      // Acumular valores válidos
      if (maxReader.isHeartRateValid()) {
        vitals.bpmSum += maxReader.getHeartRate();
        vitals.bpmCount++;
      }
      if (maxReader.isSpo2Valid()) {
        vitals.spo2Sum += maxReader.getSpo2();
        vitals.spo2Count++;
      }

      // Mostrar progreso cada 5 segundos
      static unsigned long lastProgress = 0;
      if (millis() - lastProgress >= 5000) {
        lastProgress = millis();
        printProgress(elapsed, TIEMPO_LECTURA_MAX);
      }

      delay(10);
    }
    break;

    // ----------------------------------------
    case STATE_WAIT_MLX_PROMPT:
      if (waitForUserConfirmation()) {
        printHeader("Midiendo Temperatura");
        Serial.println(F("Acerca el sensor a tu frente o muneca."));
        Serial.println(F(""));
        stateStartMs = millis();
        appState = STATE_READING_MLX;
      }
      break;

    // ----------------------------------------
    case STATE_READING_MLX: {
      unsigned long elapsed = millis() - stateStartMs;
      
      if (elapsed >= TIEMPO_LECTURA_MLX) {
        appState = STATE_SENDING_DATA;
        break;
      }

      double temp = tempReader.readObjectTemp();
      
      if (temp > 0) {  // Lectura válida
        vitals.tempSum += temp;
        vitals.tempCount++;
        
        Serial.print(F("Temperatura: "));
        Serial.print(temp, 1);
        Serial.println(F(" C"));
      } else {
        Serial.println(F("Acerca mas el sensor..."));
      }

      delay(500);
    }
    break;

    // ----------------------------------------
    case STATE_SENDING_DATA: {
      printHeader("Resumen de Mediciones");
      
      float bpmAvg = vitals.getBpmAvg();
      float spo2Avg = vitals.getSpo2Avg();
      double tempAvg = vitals.getTempAvg();

      Serial.print(F("BPM Promedio:    "));
      if (vitals.bpmCount > 0) {
        Serial.print(bpmAvg, 0);
        Serial.print(F(" ("));
        Serial.print(vitals.bpmCount);
        Serial.println(F(" muestras)"));
      } else {
        Serial.println(F("Sin datos validos"));
      }

      Serial.print(F("SpO2 Promedio:   "));
      if (vitals.spo2Count > 0) {
        Serial.print(spo2Avg, 0);
        Serial.print(F("% ("));
        Serial.print(vitals.spo2Count);
        Serial.println(F(" muestras)"));
      } else {
        Serial.println(F("Sin datos validos"));
      }

      Serial.print(F("Temp Promedio:   "));
      if (vitals.tempCount > 0) {
        Serial.print(tempAvg, 1);
        Serial.print(F(" C ("));
        Serial.print(vitals.tempCount);
        Serial.println(F(" muestras)"));
      } else {
        Serial.println(F("Sin datos validos"));
      }

      Serial.println(F(""));

      // Enviar a API
      if (vitals.bpmCount > 0 && vitals.tempCount > 0) {
        sendToAPI(bpmAvg, spo2Avg, tempAvg);
      } else {
        Serial.println(F("Datos insuficientes para enviar a la API."));
      }

      appState = STATE_DONE;
    }
    break;

    // ----------------------------------------
    case STATE_DONE:
      printHeader("Medicion Finalizada");
      Serial.println(F("Reinicia el ESP32 para una nueva medicion."));
      Serial.println(F("O escribe 'si' para medir de nuevo."));
      
      // Permitir reinicio
      if (waitForUserConfirmation()) {
        appState = STATE_WAIT_MAX_PROMPT;
      }
      delay(10000);
      break;
  }
}
