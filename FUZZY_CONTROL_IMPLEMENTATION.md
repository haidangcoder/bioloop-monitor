# Fuzzy Logic Control Implementation

## Overview

The BioLoop Monitor control module has been refactored to use **fuzzy logic** instead of hard thresholds. This provides smooth, gradual control transitions that are ideal for biological composting processes with high thermal inertia.

## Why Fuzzy Logic?

Traditional ON/OFF control causes:
- Abrupt switching that stresses actuators
- Oscillation around thresholds
- Poor response to gradual changes

Fuzzy logic provides:
- **Smooth transitions** between control states
- **Gradual response** to input changes
- **Reduced actuator wear** from fewer switches
- **Better modeling** of human expert reasoning

## Fuzzy System Design

### Inputs

#### 1. Temperature Error (°C)
Range: [-10, +10]

- **Cold**: [-10, -10, 0] - Temperature below target
- **Normal**: [-2, 0, 2] - Temperature near target (optimal)
- **Hot**: [0, 5, 10] - Temperature above target

#### 2. Moisture Level (%)
Range: [0, 100]

- **Dry**: [0, 0, 35] - Moisture below optimal
- **Normal**: [30, 50, 70] - Moisture in optimal range
- **Wet**: [60, 100, 100] - Moisture above optimal

### Outputs

#### 1. Fan PWM
Range: [0, 255]

- **Low**: [0, 0, 80] - Minimal fan speed
- **Medium**: [60, 130, 200] - Moderate fan speed
- **High**: [180, 255, 255] - Maximum fan speed

#### 2. Pump Level
Range: [0, 100] → Converted to boolean

- **Off**: [0, 0, 20] - No pumping
- **Low**: [10, 40, 70] - Minimal pumping
- **High**: [60, 100, 100] - Maximum pumping

**Conversion**: pumpLevel > 30% → pumpActive = true

### Fuzzy Rules

#### Fan Control (Temperature-based)
1. IF tempError IS Hot → fan IS High
2. IF tempError IS Normal → fan IS Medium
3. IF tempError IS Cold → fan IS Low

#### Pump Control (Moisture-based)
4. IF moisture IS Dry → pump IS High
5. IF moisture IS Normal → pump IS Low
6. IF moisture IS Wet → pump IS Off

## Implementation Details

### Defuzzification Method
**Centroid** - Provides balanced output across overlapping fuzzy sets

### Input Clamping
- Temperature error: [-10, +10]°C
- Moisture level: [0, 100]%

### Output Conversion
- Fan: Direct PWM [0-255]
- Pump: Boolean (pumpLevel > 30%)

## Testing Strategy

The unit tests validate:

1. **Output Range**: Fan PWM always in [0, 255]
2. **Monotonicity**: Fan increases smoothly with temperature error
3. **Smooth Transitions**: Pump activation transitions gradually
4. **Extreme Conditions**: Correct behavior at input boundaries
5. **No Abrupt Switching**: Small input changes → small output changes
6. **Input Safety**: Out-of-range inputs are safely clamped

## Benefits Over Hard Thresholds

| Aspect | Hard Threshold | Fuzzy Logic |
|--------|---------------|-------------|
| Transitions | Abrupt ON/OFF | Smooth gradual |
| Oscillation | High risk | Reduced |
| Actuator wear | Higher | Lower |
| Response | Binary | Proportional |
| Tuning | Difficult | Intuitive |

## Example Behavior

### Temperature Control
- At -5°C error: Fan ~20% (Low)
- At 0°C error: Fan ~50% (Medium)
- At +5°C error: Fan ~90% (High)

### Moisture Control
- At 20% moisture: Pump ON (High)
- At 35% moisture: Pump transition zone
- At 50% moisture: Pump OFF (Normal)
- At 70% moisture: Pump OFF (Wet)

## Memory Safety

- Static fuzzy system initialization
- No dynamic allocation during control loop
- Safe for embedded ESP32 environment
- Predictable execution time

## Future Enhancements

Possible improvements:
- Add temperature-moisture interaction rules
- Implement adaptive membership functions
- Add time-of-day considerations
- Tune membership functions based on field data
