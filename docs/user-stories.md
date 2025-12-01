# User Stories
This document contains the user stories with acceptance criteria for the ESP32 Vital Signs Monitor, focusing on the functional requirements of measuring BPM, SpO₂, body temperature, sending data to a remote API, and activating an alarm through an LED indicator.  
The criteria use the Given–When–Then format in third person to specify observable behavior.

---

## US01: Start a Guided Measurement Cycle
As a user of the device,  
I want to begin the measurement process only after confirming it,  
so that I can control when the system starts collecting vital sign data.

### Acceptance Criteria
**Scenario 1: Measurement begins after console confirmation**  
- **Given** the ESP32 is powered on, sensors are initialized, and the system displays “Type 'si' to begin”,  
- **When** the user types “si”, “s”, “yes”, or “y” in the serial monitor,  
- **Then** the system clears previous measurement data, initializes the MAX30102 buffers, and begins the BPM and SpO₂ measurement phase.

---

## US02: Measure and Average BPM and SpO₂
As a user of the device,  
I want the system to measure BPM and SpO₂ for a defined time interval,  
so that I can obtain representative average values of my vital signs.

### Acceptance Criteria
**Scenario 1: Timed MAX30102 measurement**  
- **Given** the user has confirmed the start of the BPM and SpO₂ measurement,  
- **When** the system reads data from the MAX30102 sensor for 40 seconds, storing only valid readings,  
- **Then** the system periodically displays progress, accumulates valid samples, and when the time ends, prompts the user to continue with the temperature measurement.

---

## US03: Measure and Average Body Temperature
As a user of the device,  
I want the system to take temperature readings for a short interval,  
so that I can obtain a reliable averaged temperature value.

### Acceptance Criteria
**Scenario 1: Timed MLX90614 measurement**  
- **Given** the BPM and SpO₂ measurement has finished and the system asks the user to confirm temperature measurement,  
- **When** the user types “si” and the system reads temperature values for 10 seconds,  
- **Then** the system records only valid readings, displays them in real time, and computes their average when the time expires.

---

## US04: Display a Summary of Averaged Measurements
As a user of the device,  
I want to see a summary of the averaged vital signs,  
so that I can understand the collected results at a glance.

### Acceptance Criteria
**Scenario 1: Summary printed in serial monitor**  
- **Given** the system has completed both measurement phases,  
- **When** the system enters the summary state,  
- **Then** it displays the averaged BPM, SpO₂, and temperature values along with the number of valid samples used for each parameter, or shows “No valid data” when applicable.

---

## US05: Send Vital Signs to a Remote API in JSON Format
As a user of the device,  
I want the measured vital signs to be sent to a remote API,  
so that they can be stored or processed externally.

### Acceptance Criteria
**Scenario 1: Successful JSON transmission with device credentials**  
- **Given** the ESP32 is connected to WiFi and valid BPM and temperature averages exist,  
- **When** the system builds a JSON body containing `device_id`, `bpm`, `temp`, and `spo2`, and sends an HTTP POST request with proper headers,  
- **Then** the API receives the data, and the system displays the HTTP status and “Data sent successfully!” if the response code is 200 or 201.

---

## US06: Handle Missing WiFi During Data Transmission
As a user of the device,  
I want the system to detect when WiFi is unavailable,  
so that the program does not crash and I understand why data was not sent.

### Acceptance Criteria
**Scenario 1: Attempt to send while offline**  
- **Given** the ESP32 is not connected to WiFi,  
- **When** the system attempts to send data to the API,  
- **Then** the system prints “WiFi disconnected. Cannot send data.” and skips the HTTP request without halting the device.

---

## US07: Process API Response and Activate LED Alarm
As a user of the device,  
I want the system to react to the server’s alarm command,  
so that I receive a visual alert indicating a potential health risk.

### Acceptance Criteria
**Scenario 1: LED controlled by `actuator_command.alarm`**  
- **Given** the system has received a valid JSON response from the API,  
- **When** the system parses the field `actuator_command.alarm`,  
- **Then** the LED turns ON when the value is `true`, turns OFF when `false`, and the system prints the alarm status in the serial monitor.

**Scenario 2: Invalid JSON response**  
- **Given** the system receives an HTTP response with an invalid JSON body,  
- **When** the system attempts to parse it,  
- **Then** the system displays a parsing error message and leaves the LED in its current state without interrupting execution.

---

## US08: Allow Repeated Measurement Cycles
As a user of the device,  
I want to start a new measurement cycle after one has finished,  
so that I can perform multiple readings without reprogramming the ESP32.

### Acceptance Criteria
**Scenario 1: Restart process after completion**  
- **Given** the system displays “Measurement finished” after completing a cycle,  
- **When** the user types “si” again in the serial monitor,  
- **Then** the system resets the workflow and returns to the state where BPM and SpO₂ measurement can begin again.

---
