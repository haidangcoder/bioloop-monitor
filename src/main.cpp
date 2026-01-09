#include <Arduino.h>
#include "actuators.h"

void setup() {
    Serial.begin(115200);
    actuators_init();

    Serial.println("=== Actuator Test ===");

    ActuatorOutput test;

    test.fanPwm = 200;
    test.pumpActive = true;
    actuators_apply(test);
    Serial.println("Fan HIGH, Pump ON");
    delay(3000);

    test.fanPwm = 50;
    test.pumpActive = false;
    actuators_apply(test);
    Serial.println("Fan LOW, Pump OFF");
    delay(3000);
}

void loop() {
    // empty
}
