/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <cmath>
#include <cfloat>
#include <limits>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

// Test execution modes for parameterized testing
enum class TestRunningMode {
    INTERP_MODE,
    AOT_MODE
};

/**
 * @brief Test fixture class for f32x4.neg opcode validation
 * @details Provides comprehensive test framework for SIMD f32x4.neg instruction
 *          including setup/teardown, helper functions, and cross-execution mode validation
 */
class F32x4NegTest : public testing::TestWithParam<TestRunningMode>
{
protected:
    TestRunningMode running_mode;
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Sets up the test fixture with WAMR runtime initialization
     * @details Initializes WAMR runtime, loads test WASM module, and prepares execution context
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.neg test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_neg_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.neg tests";
    }

    /**
     * @brief Tears down the test fixture with proper cleanup
     * @details Destroys execution environment and WAMR runtime resources
     */
    void TearDown() override
    {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM function and extract v128 result as f32 array
     * @param function_name Name of the WASM function to call
     * @param result_out Array to store the 4 f32 result values
     */
    void call_f32x4_neg_function(const char* function_name, float result_out[4])
    {
        wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(dummy_env->get());
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, function_name);
        ASSERT_NE(func, nullptr) << "Failed to lookup function: " << function_name;

        uint32_t argv[4];  // v128 result as 4 x i32
        bool call_result = wasm_runtime_call_wasm(dummy_env->get(), func, 0, argv);
        ASSERT_TRUE(call_result) << "Failed to call " << function_name << ": " << wasm_runtime_get_exception(module_inst);

        // Extract f32 values from v128 result
        memcpy(&result_out[0], &argv[0], sizeof(float));
        memcpy(&result_out[1], &argv[1], sizeof(float));
        memcpy(&result_out[2], &argv[2], sizeof(float));
        memcpy(&result_out[3], &argv[3], sizeof(float));
    }

    /**
     * @brief Utility function to check if two floats have opposite signs
     * @param a First float value
     * @param b Second float value
     * @return true if signs are opposite, false otherwise
     */
    bool have_opposite_signs(float a, float b)
    {
        return std::signbit(a) != std::signbit(b);
    }

    /**
     * @brief Utility function to get the raw bit pattern of a float
     * @param f Float value
     * @return 32-bit integer with same bit pattern as the float
     */
    uint32_t float_to_bits(float f)
    {
        uint32_t bits;
        memcpy(&bits, &f, sizeof(float));
        return bits;
    }

    /**
     * @brief Utility function to create float from raw bit pattern
     * @param bits 32-bit integer bit pattern
     * @return Float with the specified bit pattern
     */
    float bits_to_float(uint32_t bits)
    {
        float f;
        memcpy(&f, &bits, sizeof(float));
        return f;
    }
};

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningMode, F32x4NegTest,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE),
                         [](const testing::TestParamInfo<TestRunningMode>& info) {
                             return info.param == TestRunningMode::INTERP_MODE ? "Interpreter" : "AOT";
                         });

/**
 * @test BasicNegation_ReturnsFlippedSigns
 * @brief Validates f32x4.neg produces correct sign flips for typical floating-point inputs
 * @details Tests fundamental negation operation with positive, negative, and mixed-sign values.
 *          Verifies that f32x4.neg correctly flips the sign bit of each lane independently.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/simd_f32x4_neg_operation
 * @input_conditions Standard f32 values: [1.0f, -2.5f, 3.14159f, -100.0f]
 * @expected_behavior Returns sign-flipped values: [-1.0f, 2.5f, -3.14159f, 100.0f]
 * @validation_method Direct comparison of negated results with expected sign-flipped values
 */
TEST_P(F32x4NegTest, BasicNegation_ReturnsFlippedSigns)
{
    float result[4];

    // Test basic negation with mixed positive/negative values
    // Expected: [1.0, -2.5, 3.14159, -100.0] -> [-1.0, 2.5, -3.14159, 100.0]
    call_f32x4_neg_function("test_f32x4_neg_basic", result);

    ASSERT_EQ(result[0], -1.0f) << "Failed to negate lane 0: 1.0f -> -1.0f";
    ASSERT_EQ(result[1], 2.5f) << "Failed to negate lane 1: -2.5f -> 2.5f";
    ASSERT_FLOAT_EQ(result[2], -3.14159f) << "Failed to negate lane 2: 3.14159f -> -3.14159f";
    ASSERT_EQ(result[3], 100.0f) << "Failed to negate lane 3: -100.0f -> 100.0f";

    // Verify sign bits are flipped correctly
    ASSERT_TRUE(have_opposite_signs(1.0f, result[0])) << "Lane 0 signs not opposite";
    ASSERT_TRUE(have_opposite_signs(-2.5f, result[1])) << "Lane 1 signs not opposite";
    ASSERT_TRUE(have_opposite_signs(3.14159f, result[2])) << "Lane 2 signs not opposite";
    ASSERT_TRUE(have_opposite_signs(-100.0f, result[3])) << "Lane 3 signs not opposite";
}

/**
 * @test BoundaryValues_PreserveMantissaExponent
 * @brief Validates f32x4.neg preserves mantissa and exponent for boundary floating-point values
 * @details Tests negation of maximum, minimum, and subnormal values to ensure only sign bit changes.
 *          Verifies precision is maintained for extreme values where bit-level accuracy matters.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/simd_f32x4_neg_boundary_handling
 * @input_conditions Boundary values: FLT_MAX, -FLT_MAX, FLT_MIN, subnormal
 * @expected_behavior Sign flipped while mantissa/exponent bits preserved exactly
 * @validation_method Bit-exact validation of floating-point representation
 */
TEST_P(F32x4NegTest, BoundaryValues_PreserveMantissaExponent)
{
    float result[4];

    // Test boundary values: [FLT_MAX, -FLT_MAX, FLT_MIN, subnormal]
    // Expected: [-FLT_MAX, FLT_MAX, -FLT_MIN, -subnormal]
    call_f32x4_neg_function("test_f32x4_neg_boundary", result);

    // Extract expected values (based on bit patterns in WAT file)
    const float flt_max = bits_to_float(0x7F7FFFFF);         // FLT_MAX
    const float neg_flt_max = bits_to_float(0xFF7FFFFF);     // -FLT_MAX
    const float flt_min = bits_to_float(0x00800000);         // FLT_MIN (smallest normalized)
    const float subnormal = bits_to_float(0x00000001);       // Smallest subnormal

    // Verify exact negation - sign bit flipped, rest preserved
    ASSERT_EQ(result[0], -flt_max) << "Failed to negate FLT_MAX correctly";
    ASSERT_EQ(result[1], flt_max) << "Failed to negate -FLT_MAX correctly";
    ASSERT_EQ(result[2], -flt_min) << "Failed to negate FLT_MIN correctly";
    ASSERT_EQ(result[3], -subnormal) << "Failed to negate subnormal correctly";

    // Test mantissa/exponent preservation by comparing bit patterns
    uint32_t original_flt_max = float_to_bits(flt_max);
    uint32_t negated_flt_max = float_to_bits(result[0]);
    uint32_t mantissa_exponent_mask = 0x7FFFFFFF;  // All bits except sign bit

    ASSERT_EQ(original_flt_max & mantissa_exponent_mask, negated_flt_max & mantissa_exponent_mask)
        << "Mantissa/exponent not preserved for FLT_MAX negation";

    // Verify exact bit patterns for sign flip
    ASSERT_EQ(original_flt_max ^ 0x80000000, negated_flt_max)
        << "Sign bit not flipped correctly for FLT_MAX";
}

/**
 * @test SpecialValues_HandleIEEE754Correctly
 * @brief Validates f32x4.neg handles IEEE 754 special values according to standard
 * @details Tests proper handling of positive/negative zero, infinity values ensuring
 *          IEEE 754 compliance for special floating-point representations.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/simd_f32x4_neg_ieee754_compliance
 * @input_conditions Special IEEE 754 values: +0.0f, -0.0f, +∞, -∞
 * @expected_behavior IEEE 754 compliant negation: -0.0f, +0.0f, -∞, +∞
 * @validation_method IEEE 754 signbit checks and infinity validation
 */
TEST_P(F32x4NegTest, SpecialValues_HandleIEEE754Correctly)
{
    float result[4];

    // Test special values: [+0.0, -0.0, +inf, -inf]
    // Expected: [-0.0, +0.0, -inf, +inf]
    call_f32x4_neg_function("test_f32x4_neg_special", result);

    // Extract expected values (based on bit patterns in WAT file)
    const float pos_zero = bits_to_float(0x00000000);  // +0.0
    const float neg_zero = bits_to_float(0x80000000);  // -0.0
    const float pos_inf = bits_to_float(0x7F800000);   // +inf
    const float neg_inf = bits_to_float(0xFF800000);   // -inf

    // Verify zero handling (+0.0 -> -0.0, -0.0 -> +0.0)
    ASSERT_EQ(result[0], 0.0f) << "Result should still be zero value";
    ASSERT_EQ(result[1], 0.0f) << "Result should still be zero value";
    ASSERT_TRUE(std::signbit(result[0])) << "Positive zero should become negative zero";
    ASSERT_FALSE(std::signbit(result[1])) << "Negative zero should become positive zero";

    // Verify infinity handling (+∞ -> -∞, -∞ -> +∞)
    ASSERT_TRUE(std::isinf(result[2])) << "Result should be infinity";
    ASSERT_TRUE(std::isinf(result[3])) << "Result should be infinity";
    ASSERT_TRUE(std::signbit(result[2])) << "Positive infinity should become negative infinity";
    ASSERT_FALSE(std::signbit(result[3])) << "Negative infinity should become positive infinity";

    // Verify exact bit patterns for signed zeros
    uint32_t result0_bits = float_to_bits(result[0]);
    uint32_t result1_bits = float_to_bits(result[1]);

    ASSERT_EQ(0x80000000, result0_bits) << "Expected negative zero bit pattern";
    ASSERT_EQ(0x00000000, result1_bits) << "Expected positive zero bit pattern";
}

/**
 * @test NaNValues_PreservePayload
 * @brief Validates f32x4.neg preserves NaN payloads while flipping sign bit
 * @details Tests that NaN values have their sign bit flipped but payload bits preserved,
 *          ensuring IEEE 754 compliant NaN handling for both quiet and signaling NaNs.
 * @test_category Edge - NaN handling validation
 * @coverage_target core/iwasm/interpreter/simd_f32x4_neg_nan_handling
 * @input_conditions Various NaN values: qNaN, sNaN, custom payload NaN, normal value
 * @expected_behavior Sign flipped, NaN payload preserved, non-NaN handled normally
 * @validation_method isnan() checks and bit-level payload preservation validation
 */
TEST_P(F32x4NegTest, NaNValues_PreservePayload)
{
    float result[4];

    // Test NaN values: [qNaN, sNaN, custom_payload_NaN, normal_value]
    // Expected: sign flipped, payload preserved for NaNs
    call_f32x4_neg_function("test_f32x4_neg_nan", result);

    // Extract expected values (based on bit patterns in WAT file)
    const float quiet_nan = bits_to_float(0x7FC00000);     // Quiet NaN
    const float signaling_nan = bits_to_float(0x7F800001); // Signaling NaN
    const float custom_nan = bits_to_float(0x7FC12345);    // Custom payload NaN
    const float normal_val = bits_to_float(0x42280000);    // 42.0f

    // Verify all NaN inputs remain NaN after negation
    ASSERT_TRUE(std::isnan(result[0])) << "Quiet NaN should remain NaN after negation";
    ASSERT_TRUE(std::isnan(result[1])) << "Signaling NaN should remain NaN after negation";
    ASSERT_TRUE(std::isnan(result[2])) << "Custom NaN should remain NaN after negation";
    ASSERT_FALSE(std::isnan(result[3])) << "Normal value should not become NaN";

    // Verify NaN payload preservation (all bits except sign bit)
    uint32_t quiet_nan_bits = float_to_bits(quiet_nan);
    uint32_t result0_bits = float_to_bits(result[0]);
    uint32_t payload_mask = 0x7FFFFFFF;  // All bits except sign bit

    ASSERT_EQ(quiet_nan_bits & payload_mask, result0_bits & payload_mask)
        << "Quiet NaN payload not preserved";

    uint32_t custom_nan_bits = float_to_bits(custom_nan);
    uint32_t result2_bits = float_to_bits(result[2]);

    ASSERT_EQ(custom_nan_bits & payload_mask, result2_bits & payload_mask)
        << "Custom NaN payload not preserved";

    // Verify sign bit is flipped for NaN values
    ASSERT_EQ((quiet_nan_bits ^ 0x80000000), result0_bits) << "Sign bit not flipped for quiet NaN";
    ASSERT_EQ((custom_nan_bits ^ 0x80000000), result2_bits) << "Sign bit not flipped for custom NaN";

    // Verify normal value is negated correctly (42.0f -> -42.0f)
    ASSERT_EQ(result[3], -42.0f) << "Normal value not negated correctly in mixed NaN test";
}

/**
 * @test LaneIndependence_ProcessIndependently
 * @brief Validates each f32x4 lane is processed independently during negation
 * @details Verifies that negation operates on each of the four lanes independently,
 *          with no cross-lane interference or dependencies affecting the results.
 * @test_category Edge - Lane independence validation
 * @coverage_target core/iwasm/interpreter/simd_f32x4_neg_lane_independence
 * @input_conditions Mixed special values across lanes: normal, infinity, NaN, zero
 * @expected_behavior Each lane negated independently without cross-lane effects
 * @validation_method Per-lane validation of independent negation behavior
 */
TEST_P(F32x4NegTest, LaneIndependence_ProcessIndependently)
{
    float result[4];

    // Test lane independence: [normal, infinity, NaN, zero] - different types per lane
    // Expected: each lane negated independently
    call_f32x4_neg_function("test_f32x4_neg_independence", result);

    // Extract expected values (based on bit patterns in WAT file)
    const float normal_val = bits_to_float(0x42F6E666);   // 123.45f
    const float pos_inf = bits_to_float(0x7F800000);      // Positive infinity
    const float quiet_nan = bits_to_float(0x7FC00000);    // Quiet NaN
    const float pos_zero = bits_to_float(0x00000000);     // Positive zero

    // Verify each lane is processed independently
    ASSERT_FLOAT_EQ(result[0], -123.45f) << "Lane 0: Normal value not negated correctly";

    ASSERT_TRUE(std::isinf(result[1])) << "Lane 1: Should remain infinity";
    ASSERT_TRUE(std::signbit(result[1])) << "Lane 1: Infinity sign not flipped";

    ASSERT_TRUE(std::isnan(result[2])) << "Lane 2: Should remain NaN";

    ASSERT_EQ(result[3], 0.0f) << "Lane 3: Zero magnitude should be preserved";
    ASSERT_TRUE(std::signbit(result[3])) << "Lane 3: Zero sign should be flipped";

    // Verify no cross-lane contamination by checking exact bit patterns
    uint32_t result0_bits = float_to_bits(result[0]);
    uint32_t result2_bits = float_to_bits(result[2]);
    uint32_t result3_bits = float_to_bits(result[3]);

    // Lane 0: Normal value negated (sign bit flipped)
    ASSERT_EQ(float_to_bits(normal_val) ^ 0x80000000, result0_bits) << "Lane 0 sign bit not flipped correctly";

    // Lane 2: NaN with sign bit flipped, payload preserved
    uint32_t nan_payload_mask = 0x7FFFFFFF;
    ASSERT_EQ(float_to_bits(quiet_nan) & nan_payload_mask, result2_bits & nan_payload_mask)
        << "Lane 2 NaN payload not preserved";

    // Lane 3: Zero with sign bit flipped
    ASSERT_EQ(0x80000000, result3_bits) << "Lane 3 should be negative zero";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}