#include "control.h"
#include <Fuzzy.h>

static Fuzzy* fuzzy = nullptr;

static float clampf(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

static uint8_t clampPwm(float value) {
    if (value < 0.0f) return 0;
    if (value > 255.0f) return 255;
    return static_cast<uint8_t>(value);
}

void control_init() {
    if (fuzzy != nullptr) {
        delete fuzzy;
    }
    fuzzy = new Fuzzy();

    // Input 1: tempError [-30, +30]
    FuzzyInput* tempError = new FuzzyInput(1);
    
    FuzzySet* tempCold = new FuzzySet(-30, -30, -10, -2);
    tempError->addFuzzySet(tempCold);
    
    FuzzySet* tempOptimal = new FuzzySet(-5, 0, 0, 5);
    tempError->addFuzzySet(tempOptimal);
    
    FuzzySet* tempHot = new FuzzySet(2, 10, 30, 30);
    tempError->addFuzzySet(tempHot);
    
    fuzzy->addFuzzyInput(tempError);

    // Input 2: moistureLevel [0, 100]
    FuzzyInput* moisture = new FuzzyInput(2);
    
    FuzzySet* moistDry = new FuzzySet(0, 0, 20, 35);
    moisture->addFuzzySet(moistDry);
    
    FuzzySet* moistOptimal = new FuzzySet(30, 50, 50, 70);
    moisture->addFuzzySet(moistOptimal);
    
    FuzzySet* moistWet = new FuzzySet(65, 80, 100, 100);
    moisture->addFuzzySet(moistWet);
    
    fuzzy->addFuzzyInput(moisture);

    // Output: fanPwm [0, 255]
    FuzzyOutput* fanOutput = new FuzzyOutput(1);
    
    FuzzySet* fanOff = new FuzzySet(0, 0, 20, 40);
    fanOutput->addFuzzySet(fanOff);
    
    FuzzySet* fanLow = new FuzzySet(30, 80, 80, 130);
    fanOutput->addFuzzySet(fanLow);
    
    FuzzySet* fanMedium = new FuzzySet(100, 150, 150, 200);
    fanOutput->addFuzzySet(fanMedium);
    
    FuzzySet* fanHigh = new FuzzySet(180, 220, 255, 255);
    fanOutput->addFuzzySet(fanHigh);
    
    fuzzy->addFuzzyOutput(fanOutput);

    // Rule 1: IF tempError IS HOT AND moisture IS DRY THEN fan HIGH
    FuzzyRuleAntecedent* rule1Ant = new FuzzyRuleAntecedent();
    rule1Ant->joinWithAND(tempHot, moistDry);
    FuzzyRuleConsequent* rule1Con = new FuzzyRuleConsequent();
    rule1Con->addOutput(fanHigh);
    FuzzyRule* rule1 = new FuzzyRule(1, rule1Ant, rule1Con);
    fuzzy->addFuzzyRule(rule1);

    // Rule 2: IF tempError IS HOT AND moisture IS OPTIMAL THEN fan MEDIUM
    FuzzyRuleAntecedent* rule2Ant = new FuzzyRuleAntecedent();
    rule2Ant->joinWithAND(tempHot, moistOptimal);
    FuzzyRuleConsequent* rule2Con = new FuzzyRuleConsequent();
    rule2Con->addOutput(fanMedium);
    FuzzyRule* rule2 = new FuzzyRule(2, rule2Ant, rule2Con);
    fuzzy->addFuzzyRule(rule2);

    // Rule 3: IF tempError IS HOT AND moisture IS WET THEN fan LOW
    FuzzyRuleAntecedent* rule3Ant = new FuzzyRuleAntecedent();
    rule3Ant->joinWithAND(tempHot, moistWet);
    FuzzyRuleConsequent* rule3Con = new FuzzyRuleConsequent();
    rule3Con->addOutput(fanLow);
    FuzzyRule* rule3 = new FuzzyRule(3, rule3Ant, rule3Con);
    fuzzy->addFuzzyRule(rule3);

    // Rule 4: IF tempError IS OPTIMAL AND moisture IS DRY THEN fan LOW
    FuzzyRuleAntecedent* rule4Ant = new FuzzyRuleAntecedent();
    rule4Ant->joinWithAND(tempOptimal, moistDry);
    FuzzyRuleConsequent* rule4Con = new FuzzyRuleConsequent();
    rule4Con->addOutput(fanLow);
    FuzzyRule* rule4 = new FuzzyRule(4, rule4Ant, rule4Con);
    fuzzy->addFuzzyRule(rule4);

    // Rule 5: IF tempError IS OPTIMAL AND moisture IS OPTIMAL THEN fan LOW
    FuzzyRuleAntecedent* rule5Ant = new FuzzyRuleAntecedent();
    rule5Ant->joinWithAND(tempOptimal, moistOptimal);
    FuzzyRuleConsequent* rule5Con = new FuzzyRuleConsequent();
    rule5Con->addOutput(fanLow);
    FuzzyRule* rule5 = new FuzzyRule(5, rule5Ant, rule5Con);
    fuzzy->addFuzzyRule(rule5);

    // Rule 6: IF tempError IS OPTIMAL AND moisture IS WET THEN fan MEDIUM
    FuzzyRuleAntecedent* rule6Ant = new FuzzyRuleAntecedent();
    rule6Ant->joinWithAND(tempOptimal, moistWet);
    FuzzyRuleConsequent* rule6Con = new FuzzyRuleConsequent();
    rule6Con->addOutput(fanMedium);
    FuzzyRule* rule6 = new FuzzyRule(6, rule6Ant, rule6Con);
    fuzzy->addFuzzyRule(rule6);

    // Rule 7: IF tempError IS COLD THEN fan OFF (any moisture)
    FuzzyRuleAntecedent* rule7Ant = new FuzzyRuleAntecedent();
    rule7Ant->joinSingle(tempCold);
    FuzzyRuleConsequent* rule7Con = new FuzzyRuleConsequent();
    rule7Con->addOutput(fanOff);
    FuzzyRule* rule7 = new FuzzyRule(7, rule7Ant, rule7Con);
    fuzzy->addFuzzyRule(rule7);
}

ControlOutput control_compute(const ControlInput& input) {
    ControlOutput output;
    
    // Clamp inputs to valid ranges
    float tempErr = clampf(input.tempError, -30.0f, 30.0f);
    float moisture = clampf(input.moistureLevel, 0.0f, 100.0f);
    
    // Set fuzzy inputs
    fuzzy->setInput(1, tempErr);
    fuzzy->setInput(2, moisture);
    
    // Run fuzzy inference
    fuzzy->fuzzify();
    
    // Get defuzzified output
    float fanValue = fuzzy->defuzzify(1);
    
    // Clamp and convert to uint8_t
    output.fanPwm = clampPwm(fanValue);
    
    // Pump rule: deterministic threshold (NOT fuzzy)
    output.pumpActive = (moisture < 35.0f);
    
    return output;
}
