#include "sensors.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

// DS18B20 setup
static OneWire oneWire(PIN_TEMP);
static DallasTemperature tempSensor(&oneWire);

void sensors_init() {
    // Initialize DS18B20
    tempSensor.begin();
    tempSensor.setResolution(12);  // 12-bit resolution
    
    // Initialize moisture ADC pin
    pinMode(PIN_MOISTURE, INPUT);
    
    Serial.println("[Sensors] Initialized");
}

SensorData sensors_read() {
    SensorData data;
    
    // Read temperature from DS18B20
    tempSensor.requestTemperatures();
    float temp = tempSensor.getTempCByIndex(0);
    
    // Validate temperature (-127 means disconnected, 85 means reset)
    if (temp == -127.0f || temp == 85.0f) {
        data.temperature = 0.0f;
        data.tempValid = false;
    } else if (temp < -10.0f || temp > 100.0f) {
        data.temperature = temp;
        data.tempValid = false;
    } else {
        data.temperature = temp;
        data.tempValid = true;
    }
    
    // Read moisture from capacitive sensor
    int rawAdc = analogRead(PIN_MOISTURE);
    
    // Map ADC to percentage (inverted: lower ADC = wetter)
    // Calibration: AIR=3500 (0%), WATER=1500 (100%)
    float moisture = 100.0f - ((float)(rawAdc - MOISTURE_WATER) / (MOISTURE_AIR - MOISTURE_WATER) * 100.0f);
    
    // Clamp to valid range
    if (moisture < 0.0f) moisture = 0.0f;
    if (moisture > 100.0f) moisture = 100.0f;
    
    data.moisture = moisture;
    data.moistureValid = (rawAdc > 100 && rawAdc < 4000);  // Basic sanity check
    
    return data;
}
