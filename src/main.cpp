/*
 * BioLoop Monitor - ESP32 Composting Control System
 * 
 * ============================================
 * ARCHITECTURE: Separation of Concerns
 * ============================================
 * 
 * This sketch handles ONLY:
 *   1. Sensor reading (temperature, moisture)
 *   2. Calling the fuzzy control module
 *   3. Driving actuators (fan relay, pump relay)
 * 
 * Decision logic is delegated to the FUZZY CONTROL MODULE:
 *   - control/control.h  (interface)
 *   - control/control.cpp (fuzzy inference engine)
 * 
 * Why Fuzzy Logic (vs. Hard Thresholds):
 *   - Smooth, gradual transitions (no abrupt ON/OFF switching)
 *   - Multi-input reasoning (temperature-moisture interactions)
 *   - Reduced actuator wear (fewer switching cycles)
 *   - Better modeling of biological processes with thermal inertia
 * 
 * ============================================
 * Hardware Configuration:
 * - ESP32 DevKit
 * - DS18B20 Temperature Sensor (GPIO 21, OneWire)
 * - Capacitive Soil Moisture Sensor (GPIO 34, Analog)
 * - Water Pump Relay (GPIO 26, ACTIVE LOW)
 * - Ventilation Fan Relay (GPIO 27, ACTIVE LOW)
 *
 * RELAY POLARITY NOTE:
 *   These relay modules are ACTIVE LOW (verified by hardware testing):
 *   - GPIO LOW  → Relay ON  (coil energized, contacts closed)
 *   - GPIO HIGH → Relay OFF (coil de-energized, contacts open)
 * 
 * Author: BioLoop Team
 * Date: 2025
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "control/control.h"  // Fuzzy logic control module

// ============================================
// WIFI CONFIGURATION - CẬP NHẬT THÔNG TIN WIFI CỦA BẠN
// ============================================
const char* WIFI_SSID = "Quoc Viet";      // Thay bằng tên WiFi của bạn
const char* WIFI_PASSWORD = "Vy@020514";  // Thay bằng mật khẩu WiFi
const char* SERVER_URL = "http://192.168.1.248:3000/api/data";  // IP máy tính của bạn

// ============================================
// CONTROL MODE SELECTION
// ============================================
// Set to true for FUZZY CONTROL (default, recommended)
// Set to false for HARD THRESHOLD CONTROL (baseline comparison only)
#define USE_FUZZY_CONTROL true

// ============================================
// PIN DEFINITIONS
// ============================================
#define PIN_TEMP_SENSOR   21    // DS18B20 OneWire data pin
#define PIN_MOISTURE      34    // Capacitive moisture sensor (ADC)
#define PIN_PUMP_RELAY    26    // Water pump relay control
#define PIN_FAN_RELAY     27    // Ventilation fan relay control

// ============================================
// FUZZY CONTROL PARAMETERS
// ============================================
// Target temperature midpoint for error calculation
#define TARGET_TEMP_MIDPOINT 52.5f  // °C (center of 50-55°C range)

// Fan relay activation threshold (fuzzy PWM → binary relay)
// Fan ON if fanPwm > this threshold
#define FAN_PWM_THRESHOLD 120

// ============================================
// HARD THRESHOLD PARAMETERS (BASELINE - for comparison only)
// ============================================
#if !USE_FUZZY_CONTROL
#define TARGET_TEMP_HIGH  55.0f   // Fan ON threshold (°C)
#define TARGET_TEMP_LOW   50.0f   // Fan OFF threshold (°C)
#define MOISTURE_PUMP_THRESHOLD 35.0f  // Pump ON threshold (%)
#endif

// ============================================
// SENSOR CALIBRATION
// ============================================
#define MOISTURE_AIR_VALUE    3500    // Sensor in air (dry)
#define MOISTURE_WATER_VALUE  1500    // Sensor in water (wet)

// ============================================
// TIMING INTERVALS
// ============================================
#define SENSOR_READ_INTERVAL  2000    // Read sensors every 2 seconds
#define LOG_INTERVAL          5000    // Log to Serial every 5 seconds
#define HTTP_SEND_INTERVAL    5000    // Send to server every 5 seconds

// ============================================
// GLOBAL VARIABLES
// ============================================

// OneWire and temperature sensor
OneWire oneWire(PIN_TEMP_SENSOR);
DallasTemperature tempSensor(&oneWire);

// Sensor readings
float currentTemperature = 0.0f;
float currentMoisture = 0.0f;
bool tempValid = false;
bool moistureValid = false;

// Actuator states
bool pumpActive = false;
bool fanActive = false;

// Fuzzy control output (for logging)
uint8_t lastFanPwm = 0;

// Timing variables (non-blocking)
unsigned long lastSensorRead = 0;
unsigned long lastLogTime = 0;
unsigned long lastHttpSend = 0;

// WiFi status
bool wifiConnected = false;

// ============================================
// FUNCTION DECLARATIONS
// ============================================
void initSensors();
void initActuators();
void readSensors();
float readTemperature();
float readMoisture();
void runFuzzyControl();
void setPump(bool state);
void setFan(bool state);
void logStatus();
String buildJsonPayload();
void initWiFi();
void sendDataToServer();

// Baseline functions (disabled when USE_FUZZY_CONTROL is true)
#if !USE_FUZZY_CONTROL
void runHardThresholdControl();
#endif

// ============================================
// WIFI INITIALIZATION
// ============================================
void initWiFi() {
  Serial.println("[WIFI] Connecting to WiFi...");
  Serial.printf("[WIFI] SSID: %s\n", WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.println("[WIFI] Connected!");
    Serial.printf("[WIFI] IP Address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("[WIFI] Connection failed! Running in offline mode.");
  }
}

// ============================================
// SEND DATA TO SERVER
// ============================================
void sendDataToServer() {
  if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] WiFi not connected, skipping...");
    return;
  }
  
  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  
  String payload = buildJsonPayload();
  Serial.printf("[HTTP] Sending: %s\n", payload.c_str());
  
  int httpCode = http.POST(payload);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("[HTTP] Response (%d): %s\n", httpCode, response.c_str());
  } else {
    Serial.printf("[HTTP] Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("  BioLoop Monitor - Composting System");
  Serial.println("========================================");
  
#if USE_FUZZY_CONTROL
  Serial.println("  Control Mode: FUZZY LOGIC (Active)");
#else
  Serial.println("  Control Mode: HARD THRESHOLD (Baseline)");
#endif
  
  Serial.println("Initializing...\n");
  
  initWiFi();
  initSensors();
  initActuators();
  
#if USE_FUZZY_CONTROL
  // Initialize fuzzy control system (called exactly once)
  Serial.println("[FUZZY] Initializing fuzzy inference engine...");
  control_init();
  Serial.println("[FUZZY] Fuzzy control ready");
  Serial.printf("[FUZZY] Fan threshold: PWM > %d → ON\n", FAN_PWM_THRESHOLD);
#endif
  
  Serial.println("\n[SYSTEM] Initialization complete!");
  Serial.println("[SYSTEM] Starting control loop...\n");
}

// ============================================
// MAIN LOOP (Non-blocking)
// ============================================
void loop() {
  unsigned long currentMillis = millis();
  
  // Read sensors at regular intervals
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;
    readSensors();
    
    // Execute control logic after sensor reading
#if USE_FUZZY_CONTROL
    runFuzzyControl();
#else
    runHardThresholdControl();
#endif
  }
  
  // Log status at regular intervals
  if (currentMillis - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = currentMillis;
    logStatus();
  }
  
  // Send data to server at regular intervals
  if (currentMillis - lastHttpSend >= HTTP_SEND_INTERVAL) {
    lastHttpSend = currentMillis;
    sendDataToServer();
  }
}

// ============================================
// SENSOR INITIALIZATION
// ============================================
void initSensors() {
  Serial.println("[SENSORS] Initializing DS18B20 temperature sensor...");
  tempSensor.begin();
  tempSensor.setResolution(12);  // 12-bit resolution (0.0625°C)
  
  int deviceCount = tempSensor.getDeviceCount();
  if (deviceCount > 0) {
    Serial.printf("[SENSORS] Found %d DS18B20 sensor(s)\n", deviceCount);
  } else {
    Serial.println("[SENSORS] WARNING: No DS18B20 sensor found!");
  }
  
  Serial.println("[SENSORS] Initializing moisture sensor on GPIO 34...");
  pinMode(PIN_MOISTURE, INPUT);
  Serial.println("[SENSORS] Moisture sensor ready");
}

// ============================================
// ACTUATOR INITIALIZATION
// ============================================
void initActuators() {
  Serial.println("[ACTUATORS] Initializing relay outputs...");
  
  pinMode(PIN_PUMP_RELAY, OUTPUT);
  pinMode(PIN_FAN_RELAY, OUTPUT);
  
  // SAFETY: Force both relays OFF at startup
  // This prevents unintended actuator activation during ESP32 boot/reset.
  // For ACTIVE HIGH relays: GPIO LOW = relay OFF
  digitalWrite(PIN_PUMP_RELAY, LOW);
  digitalWrite(PIN_FAN_RELAY, LOW);

  delay(500);
  pumpActive = false;
  fanActive = false;
  
  Serial.println("[ACTUATORS] Pump relay (GPIO 26) - OFF (ACTIVE HIGH)");
  Serial.println("[ACTUATORS] Fan relay (GPIO 27) - OFF (ACTIVE HIGH)");
}

// ============================================
// SENSOR READING
// ============================================
void readSensors() {
  currentTemperature = readTemperature();
  currentMoisture = readMoisture();
}

float readTemperature() {
  tempSensor.requestTemperatures();
  float temp = tempSensor.getTempCByIndex(0);
  
  // Validate reading
  // -127 = disconnected, 85 = power-on reset value
  if (temp == -127.0f || temp == 85.0f) {
    tempValid = false;
    return currentTemperature;  // Return last valid reading
  }
  
  // Check physical bounds
  if (temp < -10.0f || temp > 100.0f) {
    tempValid = false;
    return currentTemperature;
  }
  
  tempValid = true;
  return temp;
}

float readMoisture() {
  // Read ADC value (12-bit: 0-4095)
  int rawAdc = analogRead(PIN_MOISTURE);
  
  // Debug: Print raw ADC value
  Serial.printf("[DEBUG] Moisture ADC raw: %d\n", rawAdc);
  
  // Validate ADC reading
  if (rawAdc < 100 || rawAdc > 4000) {
    moistureValid = false;
    Serial.printf("[DEBUG] Moisture INVALID: ADC out of range\n");
    return currentMoisture;  // Return last valid reading
  }
  
  // Convert to percentage (CORRECTED FORMULA)
  // Capacitive sensor: HIGH ADC = DRY, LOW ADC = WET
  // moisture% = (AIR_VALUE - rawAdc) / (AIR_VALUE - WATER_VALUE) * 100
  float moisture = ((float)(MOISTURE_AIR_VALUE - rawAdc) / 
                    (float)(MOISTURE_AIR_VALUE - MOISTURE_WATER_VALUE)) * 100.0f;
  
  // Debug: Print calculated moisture
  Serial.printf("[DEBUG] Moisture calculated: %.1f%% (before clamping)\n", moisture);
  
  // Clamp to valid range
  if (moisture < 0.0f) moisture = 0.0f;
  if (moisture > 100.0f) moisture = 100.0f;
  
  moistureValid = true;
  Serial.printf("[DEBUG] Moisture final: %.1f%%\n", moisture);
  return moisture;
}

// ============================================
// FUZZY CONTROL (Active Mode)
// ============================================
// This function:
//   1. Builds ControlInput from sensor readings
//   2. Calls fuzzy inference engine
//   3. Maps fuzzy output to actuators
//
// No decision logic here - all intelligence is in control.cpp
// ============================================
void runFuzzyControl() {
  // Skip if sensors are invalid
  if (!tempValid || !moistureValid) {
    Serial.println("[DEBUG] Sensors INVALID - turning OFF all actuators");
    Serial.printf("[DEBUG] tempValid=%d, moistureValid=%d\n", tempValid, moistureValid);
    setFan(false);
    setPump(false);
    return;
  }
  
  // Build fuzzy input
  ControlInput input;
  input.tempError = currentTemperature - TARGET_TEMP_MIDPOINT;
  input.moistureLevel = currentMoisture;
  
  // Debug: Print fuzzy inputs
  Serial.printf("[DEBUG] Fuzzy Input: tempError=%.2f, moisture=%.1f%%\n", 
                input.tempError, input.moistureLevel);
  
  // Call fuzzy inference (no dynamic allocation)
  ControlOutput output = control_compute(input);
  
  // Store for logging
  lastFanPwm = output.fanPwm;
  
  // Debug: Print fuzzy outputs
  Serial.printf("[DEBUG] Fuzzy Output: fanPwm=%d, pumpActive=%d\n", 
                output.fanPwm, output.pumpActive);
  
  // Map fuzzy output to actuators
  // Fan: ON if PWM exceeds threshold (relay cannot do PWM)
  bool newFanState = (output.fanPwm > FAN_PWM_THRESHOLD);
  Serial.printf("[DEBUG] Fan decision: PWM=%d > threshold=%d ? %s\n", 
                output.fanPwm, FAN_PWM_THRESHOLD, newFanState ? "YES (ON)" : "NO (OFF)");
  
  if (newFanState != fanActive) {
    setFan(newFanState);
    Serial.printf("[FUZZY] Fan %s (PWM=%d, threshold=%d)\n", 
                  newFanState ? "ON" : "OFF", output.fanPwm, FAN_PWM_THRESHOLD);
  }
  
  // Pump: Use fuzzy boolean directly
  if (output.pumpActive != pumpActive) {
    setPump(output.pumpActive);
    Serial.printf("[FUZZY] Pump %s (moisture=%.1f%%)\n", 
                  output.pumpActive ? "ON" : "OFF", currentMoisture);
  }
}

// ============================================
// ACTUATOR CONTROL FUNCTIONS
// ============================================
// ACTIVE HIGH relay logic (CORRECTED):
//   state == true  → GPIO HIGH (relay ON)
//   state == false → GPIO LOW  (relay OFF)
// ============================================
void setPump(bool state) {
  pumpActive = state;
  digitalWrite(PIN_PUMP_RELAY, state ? HIGH : LOW);
  
  // Debug: Verify GPIO state
  int gpioState = digitalRead(PIN_PUMP_RELAY);
  Serial.printf("[DEBUG] Pump: state=%s → GPIO=%s (expected: %s)\n", 
                state ? "ON" : "OFF",
                gpioState == LOW ? "LOW" : "HIGH",
                state ? "HIGH" : "LOW");
}

void setFan(bool state) {
  fanActive = state;
  digitalWrite(PIN_FAN_RELAY, state ? HIGH : LOW);
  
  // Debug: Verify GPIO state
  int gpioState = digitalRead(PIN_FAN_RELAY);
  Serial.printf("[DEBUG] Fan: state=%s → GPIO=%s (expected: %s)\n", 
                state ? "ON" : "OFF",
                gpioState == LOW ? "LOW" : "HIGH",
                state ? "HIGH" : "LOW");
}

// ============================================
// STATUS LOGGING
// ============================================
void logStatus() {
  Serial.println("----------------------------------------");
  Serial.printf("[STATUS] Temperature: %.1f°C %s\n", 
                currentTemperature, tempValid ? "" : "(INVALID)");
  Serial.printf("[STATUS] Moisture: %.1f%% %s\n", 
                currentMoisture, moistureValid ? "" : "(INVALID)");
  
#if USE_FUZZY_CONTROL
  Serial.printf("[STATUS] Fan PWM: %d → %s\n", lastFanPwm, fanActive ? "ON" : "OFF");
#endif
  
  Serial.printf("[STATUS] Pump: %s\n", pumpActive ? "ON" : "OFF");
  Serial.printf("[STATUS] Fan: %s\n", fanActive ? "ON" : "OFF");
  
  Serial.println("[JSON] " + buildJsonPayload());
  Serial.println("----------------------------------------\n");
}

// ============================================
// JSON PAYLOAD BUILDER
// ============================================
String buildJsonPayload() {
  float tempError = currentTemperature - TARGET_TEMP_MIDPOINT;
  
  String json = "{";
  json += "\"temperature\":" + String(currentTemperature, 1) + ",";
  json += "\"moisture\":" + String(currentMoisture, 1) + ",";
  json += "\"temp_error\":" + String(tempError, 2) + ",";
#if USE_FUZZY_CONTROL
  json += "\"fan_pwm\":" + String(lastFanPwm) + ",";
#endif
  json += "\"fan\":" + String(fanActive ? "true" : "false") + ",";
  json += "\"pump\":" + String(pumpActive ? "true" : "false");
  json += "}";
  
  return json;
}


// ============================================
// ============================================
// BASELINE: HARD THRESHOLD CONTROL
// ============================================
// ============================================
// 
// The following code is the ORIGINAL hard-threshold control implementation.
// It is DISABLED by default (USE_FUZZY_CONTROL = true).
// 
// Preserved for:
//   1. Academic comparison (fuzzy vs. hard threshold)
//   2. Fallback if fuzzy module has issues
//   3. Reference implementation
//
// To enable: Set USE_FUZZY_CONTROL to false at top of file
//
// Limitations of hard threshold control:
//   - Abrupt ON/OFF switching at boundaries
//   - No multi-input reasoning (temp and moisture independent)
//   - Increased actuator wear from frequent switching
//   - Oscillation risk near thresholds
// ============================================

#if !USE_FUZZY_CONTROL

// Additional timing variables for baseline pump control
static unsigned long pumpStartTime_baseline = 0;
static unsigned long pumpDuration_baseline = 0;
static unsigned long lastPumpCycle_baseline = 0;

#define PUMP_DURATION_MILD     5000   // 50-55% moisture → 5 sec
#define PUMP_DURATION_MODERATE 10000  // 45-50% moisture → 10 sec
#define PUMP_DURATION_SEVERE   15000  // <45% moisture → 15 sec
#define PUMP_COOLDOWN          60000  // Minimum time between pump cycles

void runHardThresholdControl() {
  // ----------------------------------------
  // BASELINE PUMP CONTROL (Time-based)
  // ----------------------------------------
  unsigned long currentMillis = millis();
  
  // If pump is currently running, check if duration has elapsed
  if (pumpActive) {
    if (currentMillis - pumpStartTime_baseline >= pumpDuration_baseline) {
      setPump(false);
      lastPumpCycle_baseline = currentMillis;
      Serial.printf("[BASELINE] Pump cycle complete (%lu ms)\n", pumpDuration_baseline);
    }
  } else {
    // Check cooldown period
    if (currentMillis - lastPumpCycle_baseline >= PUMP_COOLDOWN) {
      // Check if pump activation is needed
      if (moistureValid && currentMoisture < MOISTURE_PUMP_THRESHOLD) {
        // Determine pump duration based on moisture deficit
        if (currentMoisture >= 30.0f) {
          pumpDuration_baseline = PUMP_DURATION_MILD;
        } else if (currentMoisture >= 25.0f) {
          pumpDuration_baseline = PUMP_DURATION_MODERATE;
        } else {
          pumpDuration_baseline = PUMP_DURATION_SEVERE;
        }
        
        pumpStartTime_baseline = currentMillis;
        setPump(true);
        Serial.printf("[BASELINE] Pump ON: moisture=%.1f%%, duration=%lu ms\n", 
                      currentMoisture, pumpDuration_baseline);
      }
    }
  }
  
  // ----------------------------------------
  // BASELINE FAN CONTROL (Hysteresis)
  // ----------------------------------------
  if (tempValid) {
    if (currentTemperature > TARGET_TEMP_HIGH && !fanActive) {
      setFan(true);
      Serial.printf("[BASELINE] Fan ON: temp=%.1f°C > %.1f°C\n", 
                    currentTemperature, TARGET_TEMP_HIGH);
    } 
    else if (currentTemperature < TARGET_TEMP_LOW && fanActive) {
      setFan(false);
      Serial.printf("[BASELINE] Fan OFF: temp=%.1f°C < %.1f°C\n", 
                    currentTemperature, TARGET_TEMP_LOW);
    }
  }
}

#endif // !USE_FUZZY_CONTROL
