# User Stories – Monitor de Signos Vitales ESP32

Este documento contiene las user stories con criterios de aceptación para el proyecto **ESP32 Monitor de Signos Vitales**, enfocado en los requisitos funcionales de medición de BPM, SpO₂ y temperatura corporal.  
Los criterios siguen el formato **Given–When–Then**, descritos en tercera persona para especificar comportamientos observables, siguiendo el estilo del documento de referencia del curso.

---

## US01: Start measuring BPM and SpO₂
As a user of the device,  
I want to start the measurement of heart rate (BPM) and blood oxygen level (SpO₂) through console confirmation,  
so that the system begins recording valid biometric samples on demand.

### Acceptance Criteria
**Scenario 1: Start MAX30102 measurement upon confirmation**  
- **Given** the device has initialized its sensors and displays “Listo para medir BPM y SpO₂” through Serial,  
- **When** the user types “si” in the Serial console,  
- **Then** the system begins the MAX30102 measurement phase and starts filling the buffer with biometric samples.

---

## US02: Measure and process heart rate (BPM) samples
As a user of the device,  
I want the system to detect valid BPM values during the measurement window,  
so that the average BPM can be computed reliably.

### Acceptance Criteria
**Scenario 1: Detection and accumulation of BPM**  
- **Given** the MAX30102 sensor is actively collecting IR and Red samples,  
- **When** valid heartbeats are detected within the configured time window,  
- **Then** the system accumulates BPM values into an internal counter and updates the progress periodically.

---

## US03: Measure and process SpO₂ samples
As a user of the device,  
I want the system to calculate SpO₂ values using the MAX30102 signal buffers,  
so that the average oxygen saturation can be computed after measurement.

### Acceptance Criteria
**Scenario 1: Detection and accumulation of SpO₂**  
- **Given** the MAX30102 sensor maintains IR and Red buffers,  
- **When** the system computes a valid SpO₂ ratio R and the value falls within physiological ranges,  
- **Then** the system accumulates the SpO₂ value and marks it as valid for averaging.

---

## US04: Start measuring body temperature
As a user of the device,  
I want to begin temperature measurement only after confirming through Serial,  
so that temperature readings occur only when the sensor is properly positioned.

### Acceptance Criteria
**Scenario 1: Start MLX90614 measurement upon confirmation**  
- **Given** the system has finished the BPM/SpO₂ phase and displays “Listo para medir temperatura”,  
- **When** the user enters “si” via Serial,  
- **Then** the system starts the MLX90614 temperature reading phase for the configured duration.

---

## US05: Record valid temperature samples
As a user of the device,  
I want the system to validate each temperature reading,  
so that only realistic human body temperature values are used in the final average.

### Acceptance Criteria
**Scenario 1: Accept only valid human temperature range**  
- **Given** the MLX90614 sensor returns object temperature values,  
- **When** a reading is between 20°C and 45°C,  
- **Then** the sample is added to the temperature accumulator; otherwise, the system requests the user to adjust the sensor.

---

## US06: Compute averages for all biometric measurements
As a user of the device,  
I want the device to calculate average BPM, SpO₂ and temperature values,  
so that the final output represents a stable and reliable measurement.

### Acceptance Criteria
**Scenario 1: Average valid samples**  
- **Given** the system has gathered valid BPM, SpO₂ and temperature samples,  
- **When** the measurement phases conclude,  
- **Then** the system computes the average for each parameter and displays a summary with sample count and average values.

---

## US07: Send biometric data to an external API
As a developer or system integrator,  
I want the device to send the averaged BPM, SpO₂ and temperature in a JSON object via HTTP POST,  
so that the data can be stored or analyzed externally.

### Acceptance Criteria
**Scenario 1: Successful POST request with JSON payload**  
- **Given** WiFi is connected and valid averages exist for BPM, SpO₂ and temperature,  
- **When** the system sends the POST request to the configured API URL,  
- **Then** the system transmits a JSON payload containing the averaged values and displays the HTTP response status.

---

## US08: Handle WiFi connection for API communication
As a user or integrator,  
I want the device to attempt WiFi connection at startup,  
so that network-dependent features become available without manual configuration.

### Acceptance Criteria
**Scenario 1: WiFi connection attempt and notification**  
- **Given** valid SSID and password parameters,  
- **When** the system boots up,  
- **Then** it attempts to connect to WiFi, displays connection progress, and prints the IP address if successful.

---

## US09: Manage interaction workflow through Serial console
As a user without a graphical interface,  
I want to control the whole process through Serial-based prompts,  
so that the device can operate in environments with minimal UI.

### Acceptance Criteria
**Scenario 1: Step-by-step interactive flow**  
- **Given** the system is in a waiting state for the next measurement phase,  
- **When** the user inputs a confirmation command ("si"),  
- **Then** the system transitions to the next measurement state and continues the workflow.

---

## US10: Provide progress feedback during measurement
As a user of the measurement system,  
I want the device to show progress while collecting samples,  
so that I can know how much time remains for each measurement phase.

### Acceptance Criteria
**Scenario 1: Progress bar during BPM/SpO₂ phase**  
- **Given** the MAX30102 measurement window is active,  
- **When** at least 5 seconds have elapsed since the last update,  
- **Then** the system displays a progress bar with percentage and remaining time on Serial.

---

