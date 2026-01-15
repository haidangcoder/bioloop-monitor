// ESP32 HTTP Client - POST sensor data to Node.js server
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include "config.h"

// Build server URL from config
String getServerURL() {
    return "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/api/data";
}

// Send sensor data to server via HTTP POST
void sendDataToServer(float temperature, float moisture, float temp_error, int fan_pwm, bool pump) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] WiFi not connected");
        return;
    }
    
    HTTPClient http;
    
    // Begin HTTP connection
    String serverURL = getServerURL();
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");
    
    // Build JSON payload
    String jsonPayload = "{";
    jsonPayload += "\"temperature\":" + String(temperature, 1) + ",";
    jsonPayload += "\"moisture\":" + String(moisture, 1) + ",";
    jsonPayload += "\"temp_error\":" + String(temp_error, 2) + ",";
    jsonPayload += "\"fan_pwm\":" + String(fan_pwm) + ",";
    jsonPayload += "\"pump\":" + String(pump ? "true" : "false");
    jsonPayload += "}";
    
    Serial.println("[HTTP] Sending: " + jsonPayload);
    
    // Send POST request
    int httpResponseCode = http.POST(jsonPayload);
    
    // Handle response
    if (httpResponseCode > 0) {
        Serial.print("[HTTP] Response code: ");
        Serial.println(httpResponseCode);
        
        String response = http.getString();
        Serial.println("[HTTP] Response: " + response);
    } else {
        Serial.print("[HTTP] Error: ");
        Serial.println(http.errorToString(httpResponseCode));
    }
    
    // Clean up
    http.end();
}

// Example usage in main loop:
/*
void loop() {
    // Read sensors
    SensorData data = sensors_read();
    
    // Compute control
    ControlInput input = {data.temperature - 55.0f, data.moisture};
    ControlOutput output = control_compute(input);
    
    // Send to server every 5 seconds
    static unsigned long lastSend = 0;
    if (millis() - lastSend >= 5000) {
        sendDataToServer(
            data.temperature,
            data.moisture,
            input.tempError,
            output.fanPwm,
            output.pumpActive
        );
        lastSend = millis();
    }
    
    delay(1000);
}
*/
