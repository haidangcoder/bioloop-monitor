# Design Document: BioLoop Monitor MVP

## Overview

BioLoop Monitor MVP is a modular ESP32-based edge computing system for **scientific validation of fuzzy logic control** in banana waste composting. This MVP validates:

1. **Fuzzy logic correctness** for temperature-based fan control
2. **Clean modular AIoT architecture** with hardware-agnostic control module
3. **Property-based testing** for formal correctness verification

**MVP Scope Boundaries:**
- ✅ Fuzzy logic fan control (core value)
- ✅ Simple rule-based pump control (DRY → ON)
- ✅ Hardware-agnostic, desktop-testable control module
- ✅ Property-based tests for all correctness properties
- ❌ Communication robustness (stub only)
- ❌ Manual override mode
- ❌ Cloud AI integration
- ❌ UI polish

## Architecture

### Project Folder Structure

```
bioloop-monitor/
├── platformio.ini              # PlatformIO configuration
├── src/
│   ├── main.cpp                # Orchestrator - auto mode only
│   ├── sensors/
│   │   ├── sensors.h           # Public interface for sensor module
│   │   └── sensors.cpp         # DS18B20 + moisture sensor implementation
│   ├── control/
│   │   ├── control.h           # Public interface for fuzzy control
│   │   └── control.cpp         # eFLL fuzzy logic (hardware-agnostic)
│   ├── actuators/
│   │   ├── actuators.h         # Public interface for actuator control
│   │   └── actuators.cpp       # LEDC PWM fan + relay pump drivers
│   └── comm/
│       ├── comm.h              # Public interface (stub)
│       └── comm.cpp            # Minimal stub with hardcoded setpoints
├── include/
│   └── config.h                # Pin definitions, constants, setpoint defaults
├── lib/
│   └── README.md               # Library dependencies note
└── test/
    ├── test_control/
    │   └── test_fuzzy.cpp      # Property tests for control module
    ├── test_sensors/
    │   └── test_sensors.cpp    # Property tests for sensor module
    └── test_actuators/
        └── test_actuators.cpp  # Property tests for actuator module
```

### High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                    main.cpp (Orchestrator - Auto Mode Only)          │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐     │
│   │ sensors/ │───▶│ control/ │───▶│actuators/│    │  comm/   │     │
│   │  read()  │    │ compute()│    │  set()   │    │ (stub)   │     │
│   └──────────┘    └──────────┘    └──────────┘    └──────────┘     │
│        │                │               │               │           │
│        ▼                ▼               ▼               ▼           │
│   SensorData      ControlOutput    (hardware)     Setpoints        │
│                   (fuzzy fan +                    (hardcoded)      │
│                    rule pump)                                       │
└─────────────────────────────────────────────────────────────────────┘
         │                                               
         ▼                                               
   ┌──────────┐                                   
   │ DS18B20  │                                   
   │ Moisture │                                   
   └──────────┘                                   
```

## Components and Interfaces

### 1. Sensor Module (sensors/)

**Responsibility:** Read physical sensors, validate readings, normalize to standard ranges.

**Public Interface (sensors.h):**
```cpp
#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

struct SensorData {
    float temperature;      // Celsius, -10 to 100
    float moisture;         // Percentage, 0-100
    bool tempValid;         // True if reading is valid
    bool moistureValid;     // True if reading is valid
};

void sensors_init(uint8_t tempPin, uint8_t moisturePin);
SensorData sensors_read();

// Exposed for testing
float sensors_normalizeMoisture(uint16_t rawAdc);
bool sensors_validateTemperature(float temp);

#endif
```

**Internal Details:**
- Uses OneWire + DallasTemperature library for DS18B20
- Reads analog pin for capacitive moisture sensor
- Maps raw ADC (0-4095) to percentage (0-100), inverted for capacitive sensors
- Validates temperature bounds (-10°C to 100°C)
- Returns validity flags for each sensor

### 2. Control Module (control/) - CORE MVP VALUE

**Responsibility:** Implement fuzzy logic inference for fan control and rule-based pump control. **Pure computation, no hardware access, fully desktop-testable.**

**Public Interface (control.h):**
```cpp
#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

struct ControlInput {
    float tempError;        // Current temp - setpoint (negative = too cold)
    float moistureLevel;    // 0-100 percentage
};

struct ControlOutput {
    uint8_t fanPwm;         // 0-255 PWM duty cycle
    bool pumpActive;        // True to activate pump
};

void control_init();
ControlOutput control_compute(ControlInput input);

#endif
```

**Fuzzy Logic Design (Fan Control):**

*Input Fuzzy Sets - temp_error (-30 to +30°C):*
- COLD: trapmf(-30, -30, -10, -2)
- OPTIMAL: trimf(-5, 0, 5)
- HOT: trapmf(2, 10, 30, 30)

*Input Fuzzy Sets - moisture_level (0-100%):*
- DRY: trapmf(0, 0, 20, 35)
- OPTIMAL: trimf(30, 50, 70)
- WET: trapmf(65, 80, 100, 100)

*Output Fuzzy Sets - fan_pwm (0-255):*
- OFF: trapmf(0, 0, 20, 40)
- LOW: trimf(30, 80, 130)
- MEDIUM: trimf(100, 150, 200)
- HIGH: trapmf(180, 220, 255, 255)

*Fuzzy Rules (exactly 7 rules):*
1. IF temp_error IS HOT AND moisture IS DRY THEN fan IS HIGH
2. IF temp_error IS HOT AND moisture IS OPTIMAL THEN fan IS MEDIUM
3. IF temp_error IS HOT AND moisture IS WET THEN fan IS LOW
4. IF temp_error IS OPTIMAL AND moisture IS DRY THEN fan IS LOW
5. IF temp_error IS OPTIMAL AND moisture IS OPTIMAL THEN fan IS LOW
6. IF temp_error IS OPTIMAL AND moisture IS WET THEN fan IS MEDIUM
7. IF temp_error IS COLD THEN fan IS OFF (any moisture)

**Pump Control (Simple Rule-Based, NOT Fuzzy):**
- IF moisture_level < 35% (DRY threshold) THEN pump IS ON
- ELSE pump IS OFF

*Rationale: Pump control is intentionally non-fuzzy to keep MVP focused on validating fuzzy logic for temperature control.*

### 3. Actuator Module (actuators/)

**Responsibility:** Drive physical outputs using ESP32 LEDC PWM. No decision logic.

**Public Interface (actuators.h):**
```cpp
#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <stdint.h>

void actuators_init(uint8_t fanPin, uint8_t pumpPin, uint8_t ledcChannel);
void actuators_setFan(uint8_t pwmValue);    // 0-255, clamped internally
void actuators_setPump(bool active);
void actuators_stop();                       // Emergency stop - all off

// Exposed for testing
uint8_t actuators_clampPwm(int value);

#endif
```

**Internal Details:**
- Uses ESP32 LEDC for PWM generation (5kHz, 8-bit resolution)
- Fan pin configured as LEDC output
- Pump pin configured as digital output for relay
- All values clamped to valid ranges before applying

### 4. Communication Module (comm/) - STUB ONLY

**Responsibility:** Provide hardcoded setpoints for architecture completeness. **Not a focus for MVP.**

**Public Interface (comm.h):**
```cpp
#ifndef COMM_H
#define COMM_H

struct Setpoints {
    float targetTemp;       // Hardcoded: 55.0°C
    float targetMoisture;   // Hardcoded: 60.0%
};

void comm_init();
Setpoints comm_getSetpoints();

#endif
```

**Implementation (Stub):**
```cpp
#include "comm.h"

static Setpoints defaultSetpoints = {55.0f, 60.0f};

void comm_init() {
    // No-op for MVP stub
}

Setpoints comm_getSetpoints() {
    return defaultSetpoints;
}
```

### 5. Orchestrator (main.cpp) - Auto Mode Only

**Responsibility:** Coordinate module calls in correct sequence. Contains NO logic. **Auto mode only for MVP.**

**Control Loop Sequence:**
1. `SensorData data = sensors_read()` - Get current readings
2. `Setpoints sp = comm_getSetpoints()` - Get hardcoded setpoints
3. Compute temp_error = data.temperature - sp.targetTemp
4. `ControlOutput out = control_compute({temp_error, data.moisture})` - Fuzzy control
5. `actuators_setFan(out.fanPwm)` - Apply fan output
6. `actuators_setPump(out.pumpActive)` - Apply pump output
7. Delay for loop period (1000ms)

## Data Models

### SensorData
| Field | Type | Range | Description |
|-------|------|-------|-------------|
| temperature | float | -10 to 100 | Core compost temperature in Celsius |
| moisture | float | 0 to 100 | Moisture level as percentage |
| tempValid | bool | - | True if temperature reading is valid |
| moistureValid | bool | - | True if moisture reading is valid |

### ControlInput
| Field | Type | Range | Description |
|-------|------|-------|-------------|
| tempError | float | -30 to +30 | Difference from setpoint (positive = too hot) |
| moistureLevel | float | 0 to 100 | Current moisture percentage |

### ControlOutput
| Field | Type | Range | Description |
|-------|------|-------|-------------|
| fanPwm | uint8_t | 0 to 255 | PWM duty cycle for fan (fuzzy output) |
| pumpActive | bool | - | Whether pump should be running (rule-based) |

### Setpoints
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| targetTemp | float | 55.0 | Target composting temperature (hardcoded) |
| targetMoisture | float | 60.0 | Target moisture percentage (hardcoded) |

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

**All properties are MANDATORY for MVP validation.**

### Property 1: Moisture Normalization Bounds

*For any* raw ADC moisture reading (0-4095), the normalized moisture output SHALL always be in the range [0, 100].

**Validates: Requirements 1.3**

### Property 2: Temperature Validation Bounds

*For any* temperature reading outside the valid physical bounds (-10°C to 100°C), the Sensor_Module SHALL set tempValid to false.

**Validates: Requirements 1.2, 1.4**

### Property 3: Control Output Bounds

*For any* valid ControlInput (tempError in [-30, 30], moistureLevel in [0, 100]), the Control_Module SHALL return fanPwm in the range [0, 255].

**Validates: Requirements 2.1, 2.4**

### Property 4: Fuzzy Control Monotonicity (Hot Region)

*For any* two ControlInputs where both have moistureLevel equal and tempError1 > tempError2 > 0 (both in hot region), the resulting fanPwm1 SHALL be greater than or equal to fanPwm2 (hotter = more fan).

**Validates: Requirements 2.5**

### Property 5: Actuator PWM Clamping

*For any* integer value passed to actuators_clampPwm(), the result SHALL be in [0, 255]. Values < 0 become 0, values > 255 become 255.

**Validates: Requirements 4.5**

### Property 6: Pump Rule Correctness

*For any* ControlInput with moistureLevel < 35, the Control_Module SHALL return pumpActive = true. *For any* ControlInput with moistureLevel >= 35, the Control_Module SHALL return pumpActive = false.

**Validates: Requirements 3.1, 3.2**

### Property 7: Cold Region Fan Off

*For any* ControlInput with tempError < -2 (cold region), the Control_Module SHALL return fanPwm in the OFF range (0-40).

**Validates: Requirements 2.5 (Rule 7)**

## Error Handling

### Sensor Errors (MVP Scope)

| Error Condition | Detection | Response |
|-----------------|-----------|----------|
| DS18B20 disconnected | Read returns -127°C | Set tempValid = false |
| Temperature out of bounds | Value outside [-10, 100] | Set tempValid = false |
| Moisture ADC out of bounds | Value outside [0, 4095] | Set moistureValid = false |

### Actuator Errors (MVP Scope)

| Error Condition | Detection | Response |
|-----------------|-----------|----------|
| PWM value out of range | Input < 0 or > 255 | Clamp to valid range |

## Testing Strategy

### Property-Based Testing (MANDATORY)

Property-based tests are **required** for MVP validation. Use PlatformIO native test framework with custom random generators.

**Configuration:**
- Minimum 100 iterations per property test
- Each test tagged with: **Feature: bioloop-monitor, Property N: [property text]**

**Property Tests to Implement:**

1. **Property 1 Test:** Generate random ADC values [0, 65535], verify normalized output clamped to [0, 100]
2. **Property 2 Test:** Generate random temperatures in [-50, 150], verify tempValid = false for values outside [-10, 100]
3. **Property 3 Test:** Generate random ControlInput pairs (tempError [-30,30], moisture [0,100]), verify fanPwm in [0, 255]
4. **Property 4 Test:** Generate pairs of hot-region inputs with same moisture, verify fanPwm monotonicity
5. **Property 5 Test:** Generate random integers [-1000, 1000], verify clamping to [0, 255]
6. **Property 6 Test:** Generate random moisture values [0, 100], verify pump rule (< 35 → ON, >= 35 → OFF)
7. **Property 7 Test:** Generate cold-region inputs (tempError < -2), verify fanPwm in OFF range [0, 40]

### Unit Testing

Unit tests verify specific examples and edge cases:

**Control Module Tests:**
- Test fuzzy output for known input combinations (boundary values)
- Test each fuzzy rule fires correctly
- Test pump threshold at exactly 35%

**Sensor Module Tests:**
- Test normalization at ADC boundaries (0, 4095)
- Test validation at temperature boundaries (-10, 100)

## Key Design Decisions

### MVP-Specific Decisions

1. **Fuzzy Logic for Fan Only:** Pump uses simple threshold rule to keep MVP focused on validating fuzzy control correctness.

2. **No Manual Mode:** System operates only in automatic fuzzy control mode. Manual override adds complexity without validating fuzzy logic.

3. **Communication Stub:** Hardcoded setpoints satisfy architecture requirements without network complexity.

4. **Hardware-Agnostic Control:** Control module has zero ESP32 dependencies, enabling desktop property-based testing.

5. **All Tests Mandatory:** Property-based tests are required, not optional. Scientific validation requires formal correctness evidence.

### Architecture Decisions

1. **Strict Module Separation:** Each module has single responsibility, communicates only through defined interfaces.

2. **Orchestrator Contains No Logic:** main.cpp only calls module functions in sequence.

3. **eFLL Library:** Using established fuzzy logic library rather than custom implementation.

### Assumptions

1. **Single DS18B20 Sensor:** One temperature sensor for MVP.
2. **Capacitive Moisture Sensor:** Output decreases with moisture (inverted mapping).
3. **5kHz PWM Frequency:** Suitable for most DC fans.
4. **1-Second Loop Period:** Sufficient for composting control dynamics.
5. **Thermophilic Target:** 55°C target based on banana waste composting research.

## main.cpp Implementation (Auto Mode Only)

```cpp
#include <Arduino.h>
#include "config.h"
#include "sensors/sensors.h"
#include "control/control.h"
#include "actuators/actuators.h"
#include "comm/comm.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize modules in order
    sensors_init(PIN_TEMP, PIN_MOISTURE);
    actuators_init(PIN_FAN, PIN_PUMP, LEDC_CHANNEL);
    control_init();
    comm_init();
    
    Serial.println("BioLoop Monitor MVP - Fuzzy Control Validation");
}

void loop() {
    // 1. Read sensors
    SensorData sensorData = sensors_read();
    
    // 2. Get setpoints (hardcoded for MVP)
    Setpoints setpoints = comm_getSetpoints();
    
    // 3. Compute fuzzy control output (auto mode only)
    ControlInput input;
    input.tempError = sensorData.temperature - setpoints.targetTemp;
    input.moistureLevel = sensorData.moisture;
    ControlOutput output = control_compute(input);
    
    // 4. Apply actuator commands
    actuators_setFan(output.fanPwm);
    actuators_setPump(output.pumpActive);
    
    // 5. Debug output
    Serial.printf("T:%.1f M:%.1f Err:%.1f Fan:%d Pump:%d\n",
        sensorData.temperature, sensorData.moisture,
        input.tempError, output.fanPwm, output.pumpActive);
    
    // 6. Loop delay
    delay(LOOP_PERIOD_MS);
}
```
