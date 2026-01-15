#include "actuators.h"
#include <Arduino.h>
#include "config.h"

void actuators_init() {
    // FAN PWM (LEDC)
    ledcSetup(LEDC_CHANNEL, LEDC_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(PIN_FAN, LEDC_CHANNEL);
    ledcWrite(LEDC_CHANNEL, 0);

    // PUMP RELAY
    pinMode(PIN_PUMP, OUTPUT);
    digitalWrite(PIN_PUMP, LOW);   // OFF by default

    Serial.println("[Actuators] Initialized");
}

void actuators_apply(const ActuatorOutput& output) {
    // Fan PWM
    ledcWrite(LEDC_CHANNEL, output.fanPwm);

    // Pump relay
    digitalWrite(PIN_PUMP, output.pumpActive ? HIGH : LOW);
}
