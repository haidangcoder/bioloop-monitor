# Implementation Plan: BioLoop Monitor MVP

## Overview

This plan implements the BioLoop Monitor MVP for **scientific validation of fuzzy logic control** in banana waste composting. The MVP focuses on:

1. Validating fuzzy logic correctness for temperature-based fan control
2. Demonstrating clean modular AIoT architecture
3. Property-based testing for formal correctness verification

**All tasks are MANDATORY.** Property tests have equal priority to implementation.

## Tasks

- [ ] 1. Set up project structure and configuration
  - Create PlatformIO project with directory structure (src/sensors/, src/control/, src/actuators/, src/comm/)
  - Create platformio.ini with ESP32 board configuration and library dependencies (OneWire, DallasTemperature, eFLL)
  - Create include/config.h with pin definitions, constants, and hardcoded setpoints
  - Create lib/README.md noting library dependencies
  - _Requirements: 7.1, 7.2, 7.3_

- [ ] 2. Implement Sensor Module
  - [ ] 2.1 Create sensors.h with SensorData struct and function declarations
    - Define SensorData struct with temperature, moisture, tempValid, moistureValid fields
    - Declare sensors_init(), sensors_read(), sensors_normalizeMoisture(), sensors_validateTemperature()
    - _Requirements: 1.1, 1.2, 1.3, 1.5_

  - [ ] 2.2 Implement sensors.cpp with DS18B20 and moisture sensor reading
    - Implement sensors_init() to configure OneWire bus and analog pin
    - Implement sensors_read() with temperature reading, moisture ADC reading, normalization, and validation
    - Implement sensors_normalizeMoisture() mapping ADC (0-4095) to percentage (0-100)
    - Implement sensors_validateTemperature() checking bounds (-10°C to 100°C)
    - _Requirements: 1.1, 1.2, 1.3, 1.4_

  - [ ] 2.3 Write property test for moisture normalization (Property 1)
    - **Property 1: Moisture Normalization Bounds**
    - Generate random ADC values [0, 65535], verify output clamped to [0, 100]
    - Minimum 100 iterations
    - **Validates: Requirements 1.3**

  - [ ] 2.4 Write property test for temperature validation (Property 2)
    - **Property 2: Temperature Validation Bounds**
    - Generate random temperatures [-50, 150], verify tempValid = false outside [-10, 100]
    - Minimum 100 iterations
    - **Validates: Requirements 1.2, 1.4**

- [ ] 3. Implement Control Module (Hardware-Agnostic, Desktop-Testable)
  - [ ] 3.1 Create control.h with ControlInput, ControlOutput structs and function declarations
    - Define ControlInput struct with tempError, moistureLevel fields
    - Define ControlOutput struct with fanPwm, pumpActive fields
    - Declare control_init() and control_compute() functions
    - _Requirements: 2.1, 2.6, 2.7_

  - [ ] 3.2 Implement control.cpp with eFLL fuzzy logic for fan control
    - Initialize Fuzzy object with input fuzzy sets (temp_error: COLD, OPTIMAL, HOT)
    - Initialize input fuzzy sets (moisture_level: DRY, OPTIMAL, WET)
    - Initialize output fuzzy sets (fan_pwm: OFF, LOW, MEDIUM, HIGH)
    - Implement exactly 7 fuzzy rules as specified in design
    - Ensure no hardware dependencies (pure computation)
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5_

  - [ ] 3.3 Implement rule-based pump control in control.cpp
    - Implement pump logic: moisture < 35% → ON, else OFF
    - This is intentionally NOT fuzzy (MVP scope constraint)
    - _Requirements: 3.1, 3.2, 3.3_

  - [ ] 3.4 Write property test for control output bounds (Property 3)
    - **Property 3: Control Output Bounds**
    - Generate random ControlInput (tempError [-30,30], moisture [0,100])
    - Verify fanPwm always in [0, 255]
    - Minimum 100 iterations
    - **Validates: Requirements 2.1, 2.4**

  - [ ] 3.5 Write property test for fuzzy control monotonicity (Property 4)
    - **Property 4: Fuzzy Control Monotonicity (Hot Region)**
    - Generate pairs with same moisture, tempError1 > tempError2 > 0
    - Verify fanPwm1 >= fanPwm2 (hotter = more fan)
    - Minimum 100 iterations
    - **Validates: Requirements 2.5**

  - [ ] 3.6 Write property test for pump rule correctness (Property 6)
    - **Property 6: Pump Rule Correctness**
    - Generate random moisture [0, 100]
    - Verify: moisture < 35 → pumpActive = true, else false
    - Minimum 100 iterations
    - **Validates: Requirements 3.1, 3.2**

  - [ ] 3.7 Write property test for cold region fan off (Property 7)
    - **Property 7: Cold Region Fan Off**
    - Generate inputs with tempError < -2 (cold region)
    - Verify fanPwm in OFF range [0, 40]
    - Minimum 100 iterations
    - **Validates: Requirements 2.5 (Rule 7)**

- [ ] 4. Checkpoint - Verify sensor and control modules
  - Ensure all property tests pass (Properties 1-4, 6-7)
  - Verify control module compiles and runs on desktop without ESP32 hardware
  - Ask the user if questions arise

- [ ] 5. Implement Actuator Module
  - [ ] 5.1 Create actuators.h with function declarations
    - Declare actuators_init(), actuators_setFan(), actuators_setPump(), actuators_stop()
    - Declare actuators_clampPwm() for testing
    - _Requirements: 4.1, 4.2, 4.3, 4.4_

  - [ ] 5.2 Implement actuators.cpp with ESP32 LEDC PWM
    - Implement actuators_init() to configure LEDC channel (5kHz, 8-bit) and pump relay pin
    - Implement actuators_setFan() with PWM clamping via actuators_clampPwm()
    - Implement actuators_setPump() for relay control
    - Implement actuators_stop() for emergency shutdown (all outputs off)
    - Implement actuators_clampPwm() clamping any int to [0, 255]
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_

  - [ ] 5.3 Write property test for PWM clamping (Property 5)
    - **Property 5: Actuator PWM Clamping**
    - Generate random integers [-1000, 1000]
    - Verify actuators_clampPwm() returns value in [0, 255]
    - Minimum 100 iterations
    - **Validates: Requirements 4.5**

- [ ] 6. Implement Communication Stub
  - [ ] 6.1 Create comm.h with Setpoints struct and function declarations
    - Define Setpoints struct with targetTemp, targetMoisture only
    - Declare comm_init() and comm_getSetpoints()
    - _Requirements: 5.1, 5.2, 5.4_

  - [ ] 6.2 Implement comm.cpp as minimal stub
    - Implement comm_init() as no-op
    - Implement comm_getSetpoints() returning hardcoded defaults (55°C, 60%)
    - No network code for MVP
    - _Requirements: 5.1, 5.2, 5.3, 5.4_

- [ ] 7. Checkpoint - Verify all modules independently
  - Ensure all property tests pass (Properties 1-7)
  - Verify each module compiles independently
  - Ask the user if questions arise

- [ ] 8. Implement Orchestrator (main.cpp) - Auto Mode Only
  - [ ] 8.1 Implement setup() function
    - Initialize Serial for debugging
    - Call module init functions in order: sensors, actuators, control, comm
    - Print startup message indicating MVP fuzzy control validation
    - _Requirements: 6.1, 6.3_

  - [ ] 8.2 Implement loop() function (auto mode only)
    - Read sensors with sensors_read()
    - Get setpoints with comm_getSetpoints()
    - Compute temp_error = temperature - targetTemp
    - Call control_compute() with ControlInput
    - Call actuators_setFan() and actuators_setPump()
    - Print debug output (temp, moisture, error, fan, pump)
    - Add loop delay (LOOP_PERIOD_MS from config)
    - NO manual mode logic
    - _Requirements: 6.2, 6.3, 6.4, 6.5_

- [ ] 9. Final checkpoint - Full system verification
  - Ensure all 7 property tests pass
  - Verify complete data flow: sensors → control → actuators
  - Verify fuzzy logic produces expected outputs for test inputs
  - Document any observations for scientific validation
  - Ask the user if questions arise

## Notes

- **ALL TASKS ARE MANDATORY** - No optional tasks for MVP
- Property-based tests have equal priority to implementation
- Control module MUST be hardware-agnostic and desktop-testable
- Pump control is intentionally rule-based, NOT fuzzy (MVP scope)
- Communication module is a stub only - no network code
- System operates in AUTO MODE ONLY - no manual override
- Minimum 100 iterations per property test for statistical confidence
- Uses PlatformIO native test framework for property-based testing
