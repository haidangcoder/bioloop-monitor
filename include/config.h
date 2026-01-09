#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// BioLoop Monitor - InnoFarm MVP Configuration
// ============================================

// WiFi Credentials (UPDATE BEFORE DEMO)
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Server Configuration (UPDATE WITH YOUR SERVER IP)
#define SERVER_IP "192.168.1.100"  // Node.js server IP address
#define SERVER_PORT 3000

// Blynk Configuration (UPDATE WITH YOUR TOKEN)
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "BioLoop Monitor"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

// Pin Definitions
#define PIN_TEMP        4       // DS18B20 data pin
#define PIN_MOISTURE    34      // Capacitive moisture sensor (ADC)
#define PIN_FAN         25      // Fan PWM output
#define PIN_PUMP        26      // Pump relay output

// PWM Configuration
#define LEDC_CHANNEL    0
#define LEDC_FREQ       5000    // 5kHz PWM frequency
#define LEDC_RESOLUTION 8       // 8-bit resolution (0-255)

// Control Parameters
#define TARGET_TEMP     55.0f   // Target composting temperature (°C)
#define MOISTURE_THRESHOLD 35.0f // Pump ON if moisture below this (%)

// Timing
#define LOOP_PERIOD_MS  1000    // Main loop period (1 second)
#define BLYNK_UPDATE_MS 2000    // Blynk update interval (2 seconds)

// Sensor Calibration
#define MOISTURE_AIR    3500    // ADC value in air (dry)
#define MOISTURE_WATER  1500    // ADC value in water (wet)

// Blynk Virtual Pins
#define VPIN_TEMP       V0      // Temperature display
#define VPIN_MOISTURE   V1      // Moisture display
#define VPIN_FAN_PWM    V2      // Fan speed display
#define VPIN_PUMP       V3      // Pump status display
#define VPIN_TEMP_ERROR V4      // Temperature error display
#define VPIN_AUTO_MODE  V5      // Auto/Manual toggle
#define VPIN_MANUAL_FAN V6      // Manual fan control slider

#endif // CONFIG_H
