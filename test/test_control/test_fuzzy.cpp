#include <unity.h>
#include "control.h"

void setUp(void) {
    control_init();
    
}

void tearDown(void) {
}

// Sanity test: fanPwm always in valid range [0, 255]
void test_fan_output_in_valid_range(void) {
    ControlInput input;
    
    // Test hot condition
    input.tempError = 20.0f;
    input.moistureLevel = 50.0f;
    ControlOutput output = control_compute(input);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(255, output.fanPwm);
    
    // Test cold condition
    input.tempError = -20.0f;
    input.moistureLevel = 50.0f;
    output = control_compute(input);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(255, output.fanPwm);
}

// Pump threshold test: ON below 35%, OFF at/above 35%
void test_pump_threshold(void) {
    ControlInput input;
    input.tempError = 0.0f;
    
    // Below threshold → pump ON
    input.moistureLevel = 30.0f;
    ControlOutput output1 = control_compute(input);
    TEST_ASSERT_TRUE(output1.pumpActive);
    
    // At threshold → pump OFF
    input.moistureLevel = 35.0f;
    ControlOutput output2 = control_compute(input);
    TEST_ASSERT_FALSE(output2.pumpActive);
    
    // Above threshold → pump OFF
    input.moistureLevel = 60.0f;
    ControlOutput output3 = control_compute(input);
    TEST_ASSERT_FALSE(output3.pumpActive);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_fan_output_in_valid_range);
    RUN_TEST(test_pump_threshold);
    return UNITY_END();
}
