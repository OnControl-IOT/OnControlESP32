@startuml
title ESP32 Vital Signs Monitor - Class Diagram

skinparam classAttributeIconSize 0
skinparam shadowing false

'============================
' Enumerations
'============================
enum AppState {
  STATE_INIT
  STATE_WAIT_MAX_PROMPT
  STATE_READING_MAX
  STATE_WAIT_MLX_PROMPT
  STATE_READING_MLX
  STATE_SENDING_DATA
  STATE_DONE
}

'============================
' Core Data Class
'============================
class VitalSigns {
  - bpmSum    : float
  - bpmCount  : int
  - spo2Sum   : float
  - spo2Count : int
  - tempSum   : double
  - tempCount : int
  + reset()          : void
  + getBpmAvg()      : float
  + getSpo2Avg()     : float
  + getTempAvg()     : double
}

'============================
' Sensor Classes
'============================
class Max30102Reader {
  + Max30102Reader()
  + begin()                       : void
  + fillInitialBuffers()          : void
  + stepAndComputeAndPrint()      : void
  + isHeartRateValid()            : bool
  + getHeartRate()                : float
  + isSpo2Valid()                 : bool
  + getSpo2()                     : float
}

class Mlx90614Reader {
  + Mlx90614Reader()
  + begin()                       : void
  + readAmbientTemp()             : double
  + readObjectTemp()              : double
  + printOnce()                   : void
}

'============================
' Main Controller (sketch .ino)
'============================
class VitalSignsMonitor {
  - appState      : AppState
  - stateStartMs  : unsigned long
  - vitals        : VitalSigns
  - maxReader     : Max30102Reader
  - tempReader    : Mlx90614Reader
  - LED_PIN       : int
  - WIFI_SSID     : const char*
  - WIFI_PASSWORD : const char*
  - API_URL       : const char*
  - DEVICE_ID     : const char*
  - API_KEY       : const char*
  - TIEMPO_LECTURA_MAX : unsigned long
  - TIEMPO_LECTURA_MLX : unsigned long

  + setup()                         : void
  + loop()                          : void
  - printHeader(title : const char*): void
  - printProgress(elapsed : unsigned long, total : unsigned long) : void
  - waitForUserConfirmation()       : bool
  - connectWiFi()                   : void
  - sendToAPI(bpm : float, spo2 : float, temp : double) : void
}

'============================
' External Infrastructure
'============================
class WiFi <<external>>
class HTTPClient <<external>>

'============================
' Relationships
'============================
VitalSignsMonitor "1" *-- "1" VitalSigns
VitalSignsMonitor "1" *-- "1" Max30102Reader
VitalSignsMonitor "1" *-- "1" Mlx90614Reader

VitalSignsMonitor ..> WiFi : uses
VitalSignsMonitor ..> HTTPClient : uses


@enduml
