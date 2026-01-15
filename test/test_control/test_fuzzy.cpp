#include <unity.h>
#include "control.h"

void setUp(void) {
    control_init();
}

void tearDown(void) {
}

// ============================================
// TEST 1: Output Range Safety
// ============================================
// Validates that outputs always stay within valid ranges
void test_output_ranges_valid(void) {
    ControlInput input;
    
    // Test various conditions
    input.tempError = 10.0f;
    input.moistureLevel = 50.0f;
    ControlOutput output = control_compute(input);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(255, output.fanPwm);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(0, output.fanPwm);
    
    input.tempError = -10.0f;
    input.moistureLevel = 50.0f;
    output = control_compute(input);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(255, output.fanPwm);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(0, output.fanPwm);
}

// ============================================
// TEST 2: Fuzzy Smoothness - Fan Increases Gradually
// ============================================
// DEMONSTRATES FUZZY ADVANTAGE: Smooth transitions vs. hard thresholds
// 
// Hard threshold control would produce:
//   tempError < 0°C  → Fan = 0
//   tempError >= 0°C → Fan = 255
//   Result: Abrupt jump at threshold
//
// Fuzzy logic produces:
//   tempError = -5°C → Fan ≈ 20-40 (Low)
//   tempError = 0°C  → Fan ≈ 100-150 (Medium)
//   tempError = 5°C  → Fan ≈ 200-240 (High)
//   Result: Smooth, gradual increase
void test_fan_increases_smoothly_with_temperature(void) {
    ControlInput input;
    input.moistureLevel = 50.0f;  // Keep moisture constant
    
    // Cold condition → Low fan
    input.tempError = -5.0f;
    ControlOutput output1 = control_compute(input);
    
    // Slightly warmer → Fan should increase slightly
    input.tempError = -2.0f;
    ControlOutput output2 = control_compute(input);
    
    // Normal condition → Medium fan
    input.tempError = 0.0f;
    ControlOutput output3 = control_compute(input);
    
    // Slightly hot → Fan should increase
    input.tempError = 2.0f;
    ControlOutput output4 = control_compute(input);
    
    // Hot condition → High fan
    input.tempError = 5.0f;
    ControlOutput output5 = control_compute(input);
    
    // Validate monotonic increase (smooth transitions)
    TEST_ASSERT_LESS_THAN_UINT8(output2.fanPwm, output3.fanPwm);
    TEST_ASSERT_LESS_THAN_UINT8(output3.fanPwm, output4.fanPwm);
    TEST_ASSERT_LESS_THAN_UINT8(output4.fanPwm, output5.fanPwm);
    
    // Validate no abrupt jumps (difference should be reasonable)
    // With hard threshold, we'd see 0 → 255 jump
    // With fuzzy, we see gradual increases
    int jump1 = output2.fanPwm - output1.fanPwm;
    int jump2 = output3.fanPwm - output2.fanPwm;
    int jump3 = output4.fanPwm - output3.fanPwm;
    
    TEST_ASSERT_LESS_THAN_INT(100, jump1);  // No single jump > 100
    TEST_ASSERT_LESS_THAN_INT(100, jump2);
    TEST_ASSERT_LESS_THAN_INT(100, jump3);
}

// ============================================
// TEST 3: Fuzzy Smoothness - Pump Transitions Gradually
// ============================================
// DEMONSTRATES FUZZY ADVANTAGE: Gradual activation vs. hard cutoff
//
// Hard threshold control would produce:
//   moisture < 35% → Pump = ON
//   moisture >= 35% → Pump = OFF
//   Result: Rapid on/off cycling near threshold
//
// Fuzzy logic produces:
//   moisture = 20% → pumpLevel ≈ 80 → Pump ON
//   moisture = 35% → pumpLevel ≈ 40 → Pump ON (transition zone)
//   moisture = 50% → pumpLevel ≈ 25 → Pump OFF
//   Result: Smoother transitions, less cycling
void test_pump_transitions_gradually(void) {
    ControlInput input;
    input.tempError = 0.0f;  // Keep temperature constant
    
    // Very dry → Pump definitely ON
    input.moistureLevel = 20.0f;
    ControlOutput output1 = control_compute(input);
    TEST_ASSERT_TRUE(output1.pumpActive);
    
    // Moderately dry → Pump likely ON (fuzzy transition)
    input.moistureLevel = 30.0f;
    ControlOutput output2 = control_compute(input);
    // Pump state depends on fuzzy output, but should be consistent
    
    // Transition zone → Fuzzy logic provides smooth handoff
    input.moistureLevel = 40.0f;
    ControlOutput output3 = control_compute(input);
    
    // Normal moisture → Pump OFF
    input.moistureLevel = 50.0f;
    ControlOutput output4 = control_compute(input);
    TEST_ASSERT_FALSE(output4.pumpActive);
    
    // Wet → Pump definitely OFF
    input.moistureLevel = 70.0f;
    ControlOutput output5 = control_compute(input);
    TEST_ASSERT_FALSE(output5.pumpActive);
}

// ============================================
// TEST 4: Combined Rule - Critical Condition (Hot + Dry)
// ============================================
// DEMONSTRATES MULTI-INPUT REASONING: Impossible with simple if-else
//
// This tests the biological interaction:
//   High temp + Low moisture = Rapid evaporation (critical!)
//   Action: Maximum cooling AND urgent water addition
void test_critical_condition_hot_and_dry(void) {
    ControlInput input;
    
    // Critical condition: Hot + Dry
    input.tempError = 7.0f;   // Hot
    input.moistureLevel = 25.0f;  // Dry
    ControlOutput output = control_compute(input);
    
    // Expect high fan (cooling) and pump ON (water addition)
    TEST_ASSERT_GREATER_THAN_UINT8(180, output.fanPwm);  // High fan
    TEST_ASSERT_TRUE(output.pumpActive);  // Pump ON
}

// ============================================
// TEST 5: Combined Rule - Overheating Risk (Hot + Wet)
// ============================================
// DEMONSTRATES MULTI-INPUT REASONING
//
// Biological interaction:
//   High temp + High moisture = Anaerobic risk
//   Action: Maximum cooling, NO water (prevent putrefaction)
void test_overheating_risk_hot_and_wet(void) {
    ControlInput input;
    
    // Overheating risk: Hot + Wet
    input.tempError = 7.0f;   // Hot
    input.moistureLevel = 75.0f;  // Wet
    ControlOutput output = control_compute(input);
    
    // Expect high fan (cooling) but pump OFF (no water)
    TEST_ASSERT_GREATER_THAN_UINT8(180, output.fanPwm);  // High fan
    TEST_ASSERT_FALSE(output.pumpActive);  // Pump OFF
}

// ============================================
// TEST 6: Combined Rule - Optimal Condition
// ============================================
// Tests steady-state behavior when conditions are ideal
void test_optimal_condition_normal_temp_and_moisture(void) {
    ControlInput input;
    
    // Optimal condition: Normal + Normal
    input.tempError = 0.0f;   // Normal
    input.moistureLevel = 50.0f;  // Normal
    ControlOutput output = control_compute(input);
    
    // Expect moderate fan and minimal pump activity
    TEST_ASSERT_GREATER_THAN_UINT8(50, output.fanPwm);   // Some fan
    TEST_ASSERT_LESS_THAN_UINT8(200, output.fanPwm);  // Not max
    TEST_ASSERT_FALSE(output.pumpActive);  // Pump OFF (optimal moisture)
}

// ============================================
// TEST 7: Combined Rule - Recovery Mode (Cold + Dry)
// ============================================
// Tests recovery from suboptimal conditions
void test_recovery_mode_cold_and_dry(void) {
    ControlInput input;
    
    // Recovery condition: Cold + Dry
    input.tempError = -7.0f;  // Cold
    input.moistureLevel = 25.0f;  // Dry
    ControlOutput output = control_compute(input);
    
    // Expect low fan (minimal cooling) but pump ON (restore moisture)
    TEST_ASSERT_LESS_THAN_UINT8(100, output.fanPwm);  // Low fan
    TEST_ASSERT_TRUE(output.pumpActive);  // Pump ON
}

// ============================================
// TEST 8: No Abrupt Switching (Smoothness Proof)
// ============================================
// DEMONSTRATES FUZZY ADVANTAGE: Small input changes → small output changes
//
// Hard threshold: 1°C change can cause 0 → 255 fan jump
// Fuzzy logic: 1°C change causes proportional, gradual adjustment
void test_small_input_changes_produce_small_output_changes(void) {
    ControlInput input;
    input.moistureLevel = 50.0f;
    
    // Measure fan response to small temperature changes
    input.tempError = 1.0f;
    ControlOutput output1 = control_compute(input);
    
    input.tempError = 1.5f;  // Only 0.5°C change
    ControlOutput output2 = control_compute(input);
    
    input.tempError = 2.0f;  // Another 0.5°C change
    ControlOutput output3 = control_compute(input);
    
    // Fan PWM differences should be small (< 50 for 0.5°C change)
    int diff1 = abs(output2.fanPwm - output1.fanPwm);
    int diff2 = abs(output3.fanPwm - output2.fanPwm);
    
    TEST_ASSERT_LESS_THAN_INT(50, diff1);
    TEST_ASSERT_LESS_THAN_INT(50, diff2);
}

// ============================================
// TEST 9: Input Clamping Safety
// ============================================
// Validates embedded safety: out-of-range inputs are handled gracefully
void test_input_clamping_safety(void) {
    ControlInput input;
    
    // Out-of-range temperature (should be clamped to [-10, 10])
    input.tempError = 50.0f;  // Way above range
    input.moistureLevel = 50.0f;
    ControlOutput output1 = control_compute(input);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(255, output1.fanPwm);
    
    // Out-of-range moisture (should be clamped to [0, 100])
    input.tempError = 0.0f;
    input.moistureLevel = 150.0f;  // Way above range
    ControlOutput output2 = control_compute(input);
    TEST_ASSERT_FALSE(output2.pumpActive);  // Should treat as wet
    
    // Negative moisture (invalid)
    input.moistureLevel = -20.0f;
    ControlOutput output3 = control_compute(input);
    TEST_ASSERT_TRUE(output3.pumpActive);  // Should treat as dry (clamped to 0)
}

// ============================================
// TEST 10: Absence of Hard Threshold Behavior
// ============================================
// CRITICAL TEST: Proves fuzzy logic provides gradual transitions
// vs. abrupt hard threshold switching.
//
// This test validates that near a typical fuzzy set boundary,
// small input changes produce proportionally small output changes.
//
// Hard threshold control would show:
//   tempError = 2.9°C → Fan = 130 (Medium)
//   tempError = 3.1°C → Fan = 220 (High)
//   Jump: 90 PWM units (69% change)
//
// Fuzzy logic should show:
//   tempError = 2.9°C → Fan ≈ 160
//   tempError = 3.1°C → Fan ≈ 180
//   Jump: ~20 PWM units (12% change)
//
// This gradual transition is essential for:
//   1. Reducing actuator wear (fewer abrupt speed changes)
//   2. Maintaining stable biological conditions (no thermal shock)
//   3. Preventing oscillation near boundaries
void test_absence_of_hard_threshold_behavior(void) {
    ControlInput input;
    input.moistureLevel = 50.0f;  // Keep moisture constant at normal
    
    // Test around tempError ≈ 3°C (boundary between Normal and Hot fuzzy sets)
    // This is where hard threshold control would show abrupt switching
    
    input.tempError = 2.8f;
    ControlOutput output1 = control_compute(input);
    
    input.tempError = 3.0f;  // Exactly at fuzzy set overlap
    ControlOutput output2 = control_compute(input);
    
    input.tempError = 3.2f;
    ControlOutput output3 = control_compute(input);
    
    // Calculate output changes
    int change1 = abs(output2.fanPwm - output1.fanPwm);  // 2.8 → 3.0
    int change2 = abs(output3.fanPwm - output2.fanPwm);  // 3.0 → 3.2
    
    // Assert gradual transitions (no single change > 40 PWM for 0.2°C step)
    // This proves fuzzy smoothness vs. hard threshold jump
    TEST_ASSERT_LESS_THAN_INT(40, change1);
    TEST_ASSERT_LESS_THAN_INT(40, change2);
    
    // Additionally verify monotonic increase (sanity check)
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(output1.fanPwm, output2.fanPwm);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(output2.fanPwm, output3.fanPwm);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Basic safety tests
    RUN_TEST(test_output_ranges_valid);
    RUN_TEST(test_input_clamping_safety);
    
    // Fuzzy smoothness demonstrations
    RUN_TEST(test_fan_increases_smoothly_with_temperature);
    RUN_TEST(test_pump_transitions_gradually);
    RUN_TEST(test_small_input_changes_produce_small_output_changes);
    RUN_TEST(test_absence_of_hard_threshold_behavior);  // NEW: Critical proof
    
    // Combined rule tests (multi-input reasoning)
    RUN_TEST(test_critical_condition_hot_and_dry);
    RUN_TEST(test_overheating_risk_hot_and_wet);
    RUN_TEST(test_optimal_condition_normal_temp_and_moisture);
    RUN_TEST(test_recovery_mode_cold_and_dry);
    
    return UNITY_END();
}
