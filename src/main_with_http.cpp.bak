// BioLoop Monitor - Complete ESP32 Firmware with HTTP Logging
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors/sensors.h"
#include "control/control.h"
#include "actuators/actuators.h"
#include "http_client.h"

// Timing variables
unsigned long lastControlLoop = 0;
unsigned long lastHttpPost = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== BioLoop Monitor - InnoFarm MVP ===");
    
    // Initialize WiFi
    Serial.print("[WiFi] Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
    
    // Initialize modules
    sensors_init();
    actuators_init(PIN_FAN, PIN_PUMP, LEDC_CHANNEL);
    control_init();
    
    Serial.println("[System] All modules initialized");
    Serial.println("[System] Starting control loop...\n");
}

void loop() {
    unsigned long now = millis();
    
    // Main control loop - every 1 second
    if (now - lastControlLoop >= LOOP_PERIOD_MS) {
        lastControlLoop = now;
        
        // 1. Read sensors
        SensorData sensorData = sensors_read();
        
        // 2. Compute control output
        ControlInput input;
        input.tempError = sensorData.temperature - TARGET_TEMP;
        input.moistureLevel = sensorData.moisture;
        
        ControlOutput output = control_compute(input);
        
        // 3. Apply actuator commands
        actuators_setFan(output.fanPwm);
        actuators_setPump(output.pumpActive);
        
        // 4. Print to Serial
        Serial.printf("[Data] T:%.1f M:%.1f Err:%.2f Fan:%d Pump:%s\n",
            sensorData.temperature,
            sensorData.moisture,
            input.tempError,
            output.fanPwm,
            output.pumpActive ? "ON" : "OFF"
        );
        
        // 5. Send to server every 5 seconds
        if (now - lastHttpPost >= 5000) {
            lastHttpPost = now;
            
            sendDataToServer(
                sensorData.temperature,
                sensorData.moisture,
                input.tempError,
                output.fanPwm,
                output.pumpActive
            );
        }
    }
}
