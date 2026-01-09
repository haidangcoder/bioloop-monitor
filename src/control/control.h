#pragma once

#include <stdint.h>

struct ControlInput {
    float tempError;        // Range: [-30, +30]
    float moistureLevel;    // Range: [0, 100]
};

struct ControlOutput {
    uint8_t fanPwm;         // Range: [0, 255]
    bool pumpActive;
};

void control_init();
ControlOutput control_compute(const ControlInput& input);
