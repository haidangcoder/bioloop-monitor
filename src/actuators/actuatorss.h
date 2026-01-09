#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <stdint.h>
#include <stdbool.h>

struct ActuatorOutput {
    uint8_t fanPwm;     // 0–255
    bool pumpActive;    // true = ON
};

// Init hardware (call in setup)
void actuators_init();

// Apply control output (call in loop)
void actuators_apply(const ActuatorOutput& output);

#endif
