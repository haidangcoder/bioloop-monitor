# Requirements Document

## Introduction

BioLoop Monitor MVP is an ESP32-based edge computing system for **scientific validation** of fuzzy logic control in banana waste composting. This MVP focuses on demonstrating correct fuzzy logic behavior for temperature-based fan control, with simple rule-based pump control for moisture management.

**MVP Scope:** Validate fuzzy logic correctness and modular AIoT architecture. Communication and UI features are intentionally minimal stubs.

**Non-Goals:** Production robustness, communication reliability, manual override modes, cloud AI integration.

## Glossary

- **BioLoop_System**: The complete ESP32-based composting control system (MVP version)
- **Sensor_Module**: Component responsible for reading and normalizing sensor data
- **Control_Module**: Component implementing fuzzy logic for fan control and rule-based pump control (hardware-agnostic, desktop-testable)
- **Actuator_Module**: Component managing PWM fan and relay/pump outputs
- **Comm_Module**: Minimal stub for architecture completeness (not a focus)
- **Orchestrator**: The main.cpp file coordinating all modules in auto mode only
- **DS18B20**: Digital temperature sensor for core compost temperature
- **Capacitive_Moisture_Sensor**: Analog sensor for moisture level detection
- **eFLL**: Embedded Fuzzy Logic Library for Arduino
- **LEDC**: ESP32 LED Control peripheral used for PWM generation
- **temp_error**: Difference between current temperature and setpoint
- **moisture_level**: Normalized moisture reading (0-100%)
- **fan_pwm**: PWM duty cycle output for ventilation fan (0-255)

## Requirements

### Requirement 1: Sensor Data Acquisition

**User Story:** As a researcher, I want the system to read temperature and moisture sensors, so that I have accurate environmental data for fuzzy control validation.

#### Acceptance Criteria

1. WHEN the Sensor_Module is initialized, THE Sensor_Module SHALL configure the DS18B20 on the OneWire bus and the capacitive moisture sensor on an analog pin
2. WHEN the Sensor_Module reads temperature, THE Sensor_Module SHALL return a validated temperature value in Celsius or an error indicator if the reading fails
3. WHEN the Sensor_Module reads moisture, THE Sensor_Module SHALL return a normalized value between 0 and 100 representing percentage moisture
4. IF a sensor reading is outside valid physical bounds (temperature: -10°C to 100°C, moisture raw: 0-4095), THEN THE Sensor_Module SHALL flag the reading as invalid
5. THE Sensor_Module SHALL NOT contain any control logic or actuator commands

### Requirement 2: Fuzzy Logic Fan Control

**User Story:** As a researcher, I want fuzzy logic to determine fan speed based on temperature error, so that I can validate smooth control behavior for composting regulation.

#### Acceptance Criteria

1. WHEN the Control_Module receives temp_error and moisture_level inputs, THE Control_Module SHALL compute fan_pwm output using eFLL fuzzy inference
2. THE Control_Module SHALL define fuzzy sets for temp_error: COLD (trapmf -30,-30,-10,-2), OPTIMAL (trimf -5,0,5), HOT (trapmf 2,10,30,30)
3. THE Control_Module SHALL define fuzzy sets for moisture_level: DRY (trapmf 0,0,20,35), OPTIMAL (trimf 30,50,70), WET (trapmf 65,80,100,100)
4. THE Control_Module SHALL define fuzzy sets for fan_pwm: OFF (trapmf 0,0,20,40), LOW (trimf 30,80,130), MEDIUM (trimf 100,150,200), HIGH (trapmf 180,220,255,255)
5. THE Control_Module SHALL implement exactly 7 fuzzy rules as specified in the design document
6. THE Control_Module SHALL NOT directly access any hardware pins or peripherals
7. THE Control_Module SHALL be fully testable on desktop without ESP32 hardware

### Requirement 3: Rule-Based Pump Control

**User Story:** As a researcher, I want simple rule-based pump control for moisture, so that the MVP remains focused on fuzzy logic validation for temperature.

#### Acceptance Criteria

1. WHEN moisture_level is in the DRY region (below 35%), THE Control_Module SHALL set pumpActive to true
2. WHEN moisture_level is NOT in the DRY region (35% or above), THE Control_Module SHALL set pumpActive to false
3. THE pump control logic SHALL NOT use fuzzy inference (intentionally simple for MVP)

### Requirement 4: Actuator Control

**User Story:** As a researcher, I want actuators to respond to control commands, so that the physical environment is regulated according to control decisions.

#### Acceptance Criteria

1. WHEN the Actuator_Module is initialized, THE Actuator_Module SHALL configure LEDC PWM channels for the fan and digital output for the pump relay
2. WHEN the Actuator_Module receives a fan_pwm value, THE Actuator_Module SHALL set the fan PWM duty cycle using ESP32 LEDC functions
3. WHEN the Actuator_Module receives a pump command, THE Actuator_Module SHALL activate or deactivate the pump relay accordingly
4. THE Actuator_Module SHALL NOT contain any control logic or decision-making
5. THE Actuator_Module SHALL clamp all PWM values to valid ranges (0-255)

### Requirement 5: Communication Stub

**User Story:** As a developer, I want a minimal communication stub, so that the architecture is complete but MVP focus remains on control logic.

#### Acceptance Criteria

1. THE Comm_Module SHALL provide hardcoded default setpoints (targetTemp=55°C, targetMoisture=60%)
2. THE Comm_Module SHALL expose comm_getSetpoints() returning the hardcoded defaults
3. THE Comm_Module SHALL NOT implement actual network communication for MVP
4. THE Comm_Module stub SHALL satisfy the modular architecture interface requirements

### Requirement 6: System Orchestration (Auto Mode Only)

**User Story:** As a researcher, I want a clean main loop that coordinates all modules in automatic fuzzy control mode, so that I can validate the control system behavior.

#### Acceptance Criteria

1. WHEN the system starts, THE Orchestrator SHALL initialize all modules in the correct order (sensors, actuators, control, comm)
2. THE Orchestrator SHALL execute a periodic control loop that: reads sensors, retrieves setpoints, calls control module, and commands actuators
3. THE Orchestrator SHALL NOT contain any sensor reading logic, control algorithms, or actuator driver code
4. THE Orchestrator SHALL pass data between modules using well-defined data structures
5. THE Orchestrator SHALL operate ONLY in automatic fuzzy control mode (no manual override for MVP)

### Requirement 7: Modular Architecture

**User Story:** As a developer, I want clearly separated modules with defined interfaces, so that I can develop and test each component independently.

#### Acceptance Criteria

1. THE BioLoop_System SHALL organize code into separate directories: sensors/, control/, actuators/, comm/, and main.cpp at root
2. EACH module SHALL expose a header file (.h) defining its public interface
3. EACH module SHALL hide implementation details in its source file (.cpp)
4. THE modules SHALL communicate only through function parameters and return values, not global variables
5. THE Control_Module SHALL be testable without hardware by accepting input values and returning output values
