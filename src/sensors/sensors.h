#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

// Sensor readings structure
struct SensorData {
    float temperature;      // Celsius
    float moisture;         // Percentage (0-100)
    bool tempValid;         // True if reading OK
    bool moistureValid;     // True if reading OK
};

// Initialize sensors (call once in setup)
void sensors_init();

// Read all sensors (call each loop)
SensorData sensors_read();

#endif
