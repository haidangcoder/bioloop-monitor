#include "control.h"
#include <Fuzzy.h>

// ============================================
// FUZZY LOGIC CONTROL MODULE - BioLoop Monitor
// ============================================
// 
// This module implements fuzzy logic control for thermophilic composting.
// Unlike hard threshold (if-else) control, fuzzy logic provides:
//   1. Smooth, gradual transitions (no abrupt switching)
//   2. Multi-input reasoning (temperature-moisture interactions)
//   3. Reduced actuator wear (fewer on/off cycles)
//   4. Better modeling of biological processes with thermal inertia
//
// Biological Context:
//   - Target: 50-55°C (thermophilic bacteria optimal range)
//   - Moisture: 50-60% (microbial activity requires water)
//   - High temp + low moisture = rapid evaporation (critical condition)
//   - High temp + high moisture = anaerobic risk (needs ventilation)
//
// ============================================

static Fuzzy* fuzzy = nullptr;

// Pump state memory for hysteresis (prevent oscillation)
static bool lastPumpState = false;
static const float PUMP_ON_THRESHOLD = 35.0f;   // Turn ON when pumpLevel > 35%
static const float PUMP_OFF_THRESHOLD = 25.0f;  // Turn OFF when pumpLevel < 25%

// Utility: Clamp float to valid range
static float clampf(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

// Utility: Clamp and convert to PWM [0, 255]
static uint8_t clampPwm(float value) {
    if (value < 0.0f) return 0;
    if (value > 255.0f) return 255;
    return static_cast<uint8_t>(value);
}

// ============================================
// FUZZY SYSTEM INITIALIZATION
// ============================================
// Called once during control_init() to set up fuzzy variables and rules.
// This function encapsulates all fuzzy logic configuration for clarity.
// ============================================
static void initFuzzySystem() {
    fuzzy = new Fuzzy();

    // ----------------------------------------
    // INPUT 1: Temperature Error (°C)
    // ----------------------------------------
    // Represents deviation from target (52.5°C midpoint)
    // Negative = too cold, Positive = too hot
    
    FuzzyInput* tempError = new FuzzyInput(1);
    
    FuzzySet* tempCold = new FuzzySet(-10, -10, 0, 0);
    tempError->addFuzzySet(tempCold);
    
    FuzzySet* tempNormal = new FuzzySet(-2, 0, 0, 2);
    tempError->addFuzzySet(tempNormal);
    
    FuzzySet* tempHot = new FuzzySet(0, 5, 10, 10);
    tempError->addFuzzySet(tempHot);
    
    fuzzy->addFuzzyInput(tempError);

    // ----------------------------------------
    // INPUT 2: Moisture Level (%)
    // ----------------------------------------
    // Critical for microbial activity and evaporative cooling
    
    FuzzyInput* moisture = new FuzzyInput(2);
    
    FuzzySet* moistDry = new FuzzySet(0, 0, 35, 35);
    moisture->addFuzzySet(moistDry);
    
    FuzzySet* moistNormal = new FuzzySet(30, 50, 50, 70);
    moisture->addFuzzySet(moistNormal);
    
    FuzzySet* moistWet = new FuzzySet(60, 100, 100, 100);
    moisture->addFuzzySet(moistWet);
    
    fuzzy->addFuzzyInput(moisture);

    // ----------------------------------------
    // OUTPUT 1: Fan PWM [0, 255]
    // ----------------------------------------
    // Controls ventilation for temperature regulation and oxygen supply
    
    FuzzyOutput* fanOutput = new FuzzyOutput(1);
    
    FuzzySet* fanLow = new FuzzySet(0, 0, 80, 80);
    fanOutput->addFuzzySet(fanLow);
    
    FuzzySet* fanMedium = new FuzzySet(60, 130, 130, 200);
    fanOutput->addFuzzySet(fanMedium);
    
    // Fan "High" membership sharpened (190→200) to strengthen response
    // in critical biological conditions (Hot + Dry overheating risk)
    FuzzySet* fanHigh = new FuzzySet(200, 240, 255, 255);
    fanOutput->addFuzzySet(fanHigh);
    
    fuzzy->addFuzzyOutput(fanOutput);

    // ----------------------------------------
    // OUTPUT 2: Pump Level [0, 100]
    // ----------------------------------------
    // Controls water addition (converted to boolean via threshold)
    
    FuzzyOutput* pumpOutput = new FuzzyOutput(2);
    
    FuzzySet* pumpOff = new FuzzySet(0, 0, 20, 20);
    pumpOutput->addFuzzySet(pumpOff);
    
    FuzzySet* pumpLow = new FuzzySet(10, 40, 40, 70);
    pumpOutput->addFuzzySet(pumpLow);
    
    // Pump "High" membership sharpened (60→70) to strengthen response
    // in critical biological conditions (rapid evaporation risk)
    FuzzySet* pumpHigh = new FuzzySet(70, 100, 100, 100);
    pumpOutput->addFuzzySet(pumpHigh);
    
    fuzzy->addFuzzyOutput(pumpOutput);

    // ============================================
    // FUZZY RULES - COMBINED MULTI-INPUT LOGIC
    // ============================================
    // These rules capture temperature-moisture interactions that are
    // critical in biological composting but impossible to model cleanly
    // with simple if-else thresholds.
    //
    // Note: Rule count intentionally limited to 10 (4 combined + 6 fallback)
    // to maintain embedded system constraints (memory, execution time).
    // Full combinatorial expansion (3×3 = 9 combined rules) would provide
    // marginal benefit at cost of increased complexity and maintenance burden.
    // ============================================

    // ----------------------------------------
    // CRITICAL CONDITION: Hot + Dry
    // ----------------------------------------
    // Biological reasoning:
    //   - High temperature accelerates evaporation
    //   - Dry conditions stress thermophilic bacteria
    //   - Risk of process stalling or overheating
    // Action: Maximum cooling + urgent water addition
    
    FuzzyRuleAntecedent* criticalAnt = new FuzzyRuleAntecedent();
    criticalAnt->joinWithAND(tempHot, moistDry);
    FuzzyRuleConsequent* criticalCon = new FuzzyRuleConsequent();
    criticalCon->addOutput(fanHigh);
    criticalCon->addOutput(pumpHigh);
    fuzzy->addFuzzyRule(new FuzzyRule(1, criticalAnt, criticalCon));

    // ----------------------------------------
    // OVERHEATING RISK: Hot + Wet
    // ----------------------------------------
    // Biological reasoning:
    //   - High temperature + high moisture = anaerobic risk
    //   - Excess moisture blocks oxygen diffusion
    //   - Can lead to putrefaction instead of composting
    // Action: Maximum cooling, NO water addition
    
    FuzzyRuleAntecedent* overheatingAnt = new FuzzyRuleAntecedent();
    overheatingAnt->joinWithAND(tempHot, moistWet);
    FuzzyRuleConsequent* overheatingCon = new FuzzyRuleConsequent();
    overheatingCon->addOutput(fanHigh);
    overheatingCon->addOutput(pumpOff);
    fuzzy->addFuzzyRule(new FuzzyRule(2, overheatingAnt, overheatingCon));

    // ----------------------------------------
    // OPTIMAL CONDITION: Normal + Normal
    // ----------------------------------------
    // Biological reasoning:
    //   - Temperature in thermophilic range (50-55°C)
    //   - Moisture supports microbial activity
    //   - Maintain steady state with minimal intervention
    // Action: Moderate cooling, minimal water adjustment
    
    FuzzyRuleAntecedent* optimalAnt = new FuzzyRuleAntecedent();
    optimalAnt->joinWithAND(tempNormal, moistNormal);
    FuzzyRuleConsequent* optimalCon = new FuzzyRuleConsequent();
    optimalCon->addOutput(fanMedium);
    optimalCon->addOutput(pumpLow);
    fuzzy->addFuzzyRule(new FuzzyRule(3, optimalAnt, optimalCon));

    // ----------------------------------------
    // RECOVERY MODE: Cold + Dry
    // ----------------------------------------
    // Biological reasoning:
    //   - Low temperature indicates reduced microbial activity
    //   - Dry conditions further inhibit biological processes
    //   - Need to restore moisture without excessive cooling
    // Action: Minimal cooling, moderate water addition
    
    FuzzyRuleAntecedent* recoveryAnt = new FuzzyRuleAntecedent();
    recoveryAnt->joinWithAND(tempCold, moistDry);
    FuzzyRuleConsequent* recoveryCon = new FuzzyRuleConsequent();
    recoveryCon->addOutput(fanLow);
    recoveryCon->addOutput(pumpHigh);
    fuzzy->addFuzzyRule(new FuzzyRule(4, recoveryAnt, recoveryCon));

    // ----------------------------------------
    // FALLBACK RULES: Single-input conditions
    // ----------------------------------------
    // These provide baseline control when conditions don't match
    // the combined rules above. Lower priority due to rule ordering.
    
    // Temperature-only fallbacks
    FuzzyRuleAntecedent* hotFallbackAnt = new FuzzyRuleAntecedent();
    hotFallbackAnt->joinSingle(tempHot);
    FuzzyRuleConsequent* hotFallbackCon = new FuzzyRuleConsequent();
    hotFallbackCon->addOutput(fanHigh);
    fuzzy->addFuzzyRule(new FuzzyRule(5, hotFallbackAnt, hotFallbackCon));

    FuzzyRuleAntecedent* normalFallbackAnt = new FuzzyRuleAntecedent();
    normalFallbackAnt->joinSingle(tempNormal);
    FuzzyRuleConsequent* normalFallbackCon = new FuzzyRuleConsequent();
    normalFallbackCon->addOutput(fanMedium);
    fuzzy->addFuzzyRule(new FuzzyRule(6, normalFallbackAnt, normalFallbackCon));

    FuzzyRuleAntecedent* coldFallbackAnt = new FuzzyRuleAntecedent();
    coldFallbackAnt->joinSingle(tempCold);
    FuzzyRuleConsequent* coldFallbackCon = new FuzzyRuleConsequent();
    coldFallbackCon->addOutput(fanLow);
    fuzzy->addFuzzyRule(new FuzzyRule(7, coldFallbackAnt, coldFallbackCon));

    // Moisture-only fallbacks
    FuzzyRuleAntecedent* dryFallbackAnt = new FuzzyRuleAntecedent();
    dryFallbackAnt->joinSingle(moistDry);
    FuzzyRuleConsequent* dryFallbackCon = new FuzzyRuleConsequent();
    dryFallbackCon->addOutput(pumpHigh);
    fuzzy->addFuzzyRule(new FuzzyRule(8, dryFallbackAnt, dryFallbackCon));

    FuzzyRuleAntecedent* normalMoistFallbackAnt = new FuzzyRuleAntecedent();
    normalMoistFallbackAnt->joinSingle(moistNormal);
    FuzzyRuleConsequent* normalMoistFallbackCon = new FuzzyRuleConsequent();
    normalMoistFallbackCon->addOutput(pumpLow);
    fuzzy->addFuzzyRule(new FuzzyRule(9, normalMoistFallbackAnt, normalMoistFallbackCon));

    FuzzyRuleAntecedent* wetFallbackAnt = new FuzzyRuleAntecedent();
    wetFallbackAnt->joinSingle(moistWet);
    FuzzyRuleConsequent* wetFallbackCon = new FuzzyRuleConsequent();
    wetFallbackCon->addOutput(pumpOff);
    fuzzy->addFuzzyRule(new FuzzyRule(10, wetFallbackAnt, wetFallbackCon));
}


// ============================================
// PUBLIC API: Initialize Control System
// ============================================
// Must be called once before control_compute().
// Initializes the fuzzy inference system with all variables and rules.
// Safe to call multiple times (cleans up previous instance).
// ============================================
void control_init() {
    // Clean up existing fuzzy system if re-initializing
    if (fuzzy != nullptr) {
        delete fuzzy;
        fuzzy = nullptr;
    }
    
    // Initialize fuzzy system with all rules and variables
    initFuzzySystem();
}

// ============================================
// PUBLIC API: Compute Control Outputs
// ============================================
// Performs fuzzy inference to compute fan PWM and pump activation.
// 
// Advantages over hard threshold (if-else) control:
//   1. Smooth transitions: No abrupt jumps in fan speed or pump state
//   2. Multi-input reasoning: Captures temp-moisture interactions
//   3. Partial activation: Fuzzy sets allow gradual membership
//   4. Reduced switching: Fewer actuator cycles = longer lifespan
//
// Example: At 52°C with 45% moisture:
//   - Hard threshold: Fan ON (100%) or OFF (0%)
//   - Fuzzy logic: Fan at 65% (smooth proportional response)
// ============================================
ControlOutput control_compute(const ControlInput& input) {
    ControlOutput output;
    
    // Clamp inputs to valid ranges (embedded safety)
    float tempErr = clampf(input.tempError, -10.0f, 10.0f);
    float moisture = clampf(input.moistureLevel, 0.0f, 100.0f);
    
    // Set fuzzy inputs
    fuzzy->setInput(1, tempErr);
    fuzzy->setInput(2, moisture);
    
    // Run fuzzy inference: fuzzification → rule evaluation → defuzzification
    // Centroid method provides smooth, balanced output across overlapping rules
    fuzzy->fuzzify();
    
    // Get defuzzified fan output
    // Result is a weighted average of all active rules, not a binary decision
    float fanValue = fuzzy->defuzzify(1);
    output.fanPwm = clampPwm(fanValue);
    
    // Get defuzzified pump output
    // Convert continuous pump level [0-100] to boolean activation
    // Use hysteresis to prevent rapid switching (oscillation)
    float pumpLevel = fuzzy->defuzzify(2);
    
    // Hysteresis logic: Different thresholds for ON and OFF
    if (lastPumpState) {
        // Pump is currently ON → Turn OFF only if pumpLevel drops below 25%
        output.pumpActive = (pumpLevel > PUMP_OFF_THRESHOLD);
    } else {
        // Pump is currently OFF → Turn ON only if pumpLevel exceeds 35%
        output.pumpActive = (pumpLevel > PUMP_ON_THRESHOLD);
    }
    
    // Update state memory
    lastPumpState = output.pumpActive;
    
    return output;
}
