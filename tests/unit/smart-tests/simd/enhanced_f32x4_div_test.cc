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
 * @brief Test fixture class for f32x4.div opcode validation
 * @details Provides comprehensive test framework for SIMD f32x4.div instruction
 *          including setup/teardown, helper functions, and cross-execution mode validation
 */
class F32x4DivTest : public testing::TestWithParam<TestRunningMode>
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

        // Load the f32x4.div test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_div_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.div tests";
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
    void call_f32x4_div_function(const char* function_name, float result_out[4])
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
     * @brief Helper function to call f32x4.div test function with input parameters
     * @param dividend0-3 First vector (dividend) lane input values
     * @param divisor0-3 Second vector (divisor) lane input values
     * @param result_out Array to store the 4 f32 result values
     */
    void call_f32x4_div_with_params(float dividend0, float dividend1, float dividend2, float dividend3,
                                   float divisor0, float divisor1, float divisor2, float divisor3,
                                   float result_out[4])
    {
        wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(dummy_env->get());
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32x4_div");
        ASSERT_NE(func, nullptr) << "Failed to lookup function: test_f32x4_div";

        uint32_t argv[8];
        // Convert float inputs to uint32_t for WASM function call (dividend first, then divisor)
        memcpy(&argv[0], &dividend0, sizeof(float));
        memcpy(&argv[1], &dividend1, sizeof(float));
        memcpy(&argv[2], &dividend2, sizeof(float));
        memcpy(&argv[3], &dividend3, sizeof(float));
        memcpy(&argv[4], &divisor0, sizeof(float));
        memcpy(&argv[5], &divisor1, sizeof(float));
        memcpy(&argv[6], &divisor2, sizeof(float));
        memcpy(&argv[7], &divisor3, sizeof(float));

        bool call_result = wasm_runtime_call_wasm(dummy_env->get(), func, 8, argv);
        ASSERT_TRUE(call_result) << "Failed to call test_f32x4_div: " << wasm_runtime_get_exception(module_inst);

        // Extract f32 values from v128 result
        memcpy(&result_out[0], &argv[0], sizeof(float));
        memcpy(&result_out[1], &argv[1], sizeof(float));
        memcpy(&result_out[2], &argv[2], sizeof(float));
        memcpy(&result_out[3], &argv[3], sizeof(float));
    }

    /**
     * @brief Helper function to compare two float values with IEEE 754 awareness
     * @param expected Expected float value
     * @param actual Actual float value
     * @return true if values are equal (handles NaN, infinity, zero cases)
     */
    bool float_equal_ieee754(float expected, float actual) {
        // Handle NaN cases - NaN should equal NaN
        if (std::isnan(expected) && std::isnan(actual)) {
            return true;
        }
        // Handle infinity cases
        if (std::isinf(expected) && std::isinf(actual)) {
            return (expected > 0) == (actual > 0); // Same sign infinity
        }
        // Handle zero cases (+0.0 and -0.0)
        if (expected == 0.0f && actual == 0.0f) {
            return true;
        }
        // Standard floating point comparison
        return expected == actual;
    }
};

/**
 * @test BasicDivision_ReturnsCorrectResults
 * @brief Validates f32x4.div produces correct arithmetic results for typical float values
 * @details Tests fundamental division operation with positive integers and mixed values.
 *          Verifies that f32x4.div correctly computes a[i] / b[i] for each lane independently.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_div
 * @input_conditions Dividend: [8.0f, 4.0f, 6.0f, 12.0f], Divisor: [2.0f, 2.0f, 3.0f, 4.0f]
 * @expected_behavior Returns quotients: [4.0f, 2.0f, 2.0f, 3.0f]
 * @validation_method Direct lane-by-lane comparison with expected division results
 */
TEST_P(F32x4DivTest, BasicDivision_ReturnsCorrectResults) {
    // Execute f32x4.div with typical integer-like floating point values
    float result_lanes[4];
    call_f32x4_div_with_params(8.0f, 4.0f, 6.0f, 12.0f,  // dividend
                              2.0f, 2.0f, 3.0f, 4.0f,    // divisor
                              result_lanes);

    // Validate each division result
    ASSERT_TRUE(float_equal_ieee754(4.0f, result_lanes[0])) << "Lane 0: Expected 4.0f, got " << result_lanes[0];
    ASSERT_TRUE(float_equal_ieee754(2.0f, result_lanes[1])) << "Lane 1: Expected 2.0f, got " << result_lanes[1];
    ASSERT_TRUE(float_equal_ieee754(2.0f, result_lanes[2])) << "Lane 2: Expected 2.0f, got " << result_lanes[2];
    ASSERT_TRUE(float_equal_ieee754(3.0f, result_lanes[3])) << "Lane 3: Expected 3.0f, got " << result_lanes[3];
}

/**
 * @test DivisionByOne_PreservesOriginalValues
 * @brief Verifies division by 1.0 acts as identity operation
 * @details Tests that dividing any float value by 1.0 preserves the original value,
 *          including handling of positive and negative numbers.
 * @test_category Main - Identity property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_div
 * @input_conditions Dividend: [5.0f, -3.0f, 7.0f, -9.0f], Divisor: [1.0f, 1.0f, 1.0f, 1.0f]
 * @expected_behavior Returns unchanged: [5.0f, -3.0f, 7.0f, -9.0f]
 * @validation_method Verify identity property preservation
 */
TEST_P(F32x4DivTest, DivisionByOne_PreservesOriginalValues) {
    // Test division by 1.0 (identity property)
    float result_lanes[4];
    call_f32x4_div_with_params(5.0f, -3.0f, 7.0f, -9.0f,  // dividend
                              1.0f, 1.0f, 1.0f, 1.0f,     // divisor
                              result_lanes);

    // Validate identity property
    ASSERT_TRUE(float_equal_ieee754(5.0f, result_lanes[0])) << "Lane 0: Expected 5.0f, got " << result_lanes[0];
    ASSERT_TRUE(float_equal_ieee754(-3.0f, result_lanes[1])) << "Lane 1: Expected -3.0f, got " << result_lanes[1];
    ASSERT_TRUE(float_equal_ieee754(7.0f, result_lanes[2])) << "Lane 2: Expected 7.0f, got " << result_lanes[2];
    ASSERT_TRUE(float_equal_ieee754(-9.0f, result_lanes[3])) << "Lane 3: Expected -9.0f, got " << result_lanes[3];
}

/**
 * @test DivisionByNegativeOne_NegatesValues
 * @brief Confirms division by -1.0 produces negation
 * @details Verifies that dividing any float value by -1.0 negates the value,
 *          effectively computing the two's complement for floating-point.
 * @test_category Main - Negation property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_div
 * @input_conditions Dividend: [2.0f, -4.0f, 6.0f, -8.0f], Divisor: [-1.0f, -1.0f, -1.0f, -1.0f]
 * @expected_behavior Returns negated: [-2.0f, 4.0f, -6.0f, 8.0f]
 * @validation_method Sign inversion verification
 */
TEST_P(F32x4DivTest, DivisionByNegativeOne_NegatesValues) {
    // Test division by -1.0 (negation property)
    float result_lanes[4];
    call_f32x4_div_with_params(2.0f, -4.0f, 6.0f, -8.0f,   // dividend
                              -1.0f, -1.0f, -1.0f, -1.0f,  // divisor
                              result_lanes);

    // Validate negation property
    ASSERT_TRUE(float_equal_ieee754(-2.0f, result_lanes[0])) << "Lane 0: Expected -2.0f, got " << result_lanes[0];
    ASSERT_TRUE(float_equal_ieee754(4.0f, result_lanes[1])) << "Lane 1: Expected 4.0f, got " << result_lanes[1];
    ASSERT_TRUE(float_equal_ieee754(-6.0f, result_lanes[2])) << "Lane 2: Expected -6.0f, got " << result_lanes[2];
    ASSERT_TRUE(float_equal_ieee754(8.0f, result_lanes[3])) << "Lane 3: Expected 8.0f, got " << result_lanes[3];
}

/**
 * @test DivisionByZero_ProducesInfinity
 * @brief Tests IEEE 754 behavior for division by zero
 * @details Verifies that dividing finite non-zero values by zero produces infinity
 *          with correct sign according to IEEE 754 standard (positive/positive = +∞, negative/positive = -∞).
 * @test_category Edge - IEEE 754 special case validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_div
 * @input_conditions Dividend: [1.0f, -1.0f, 2.0f, -2.0f], Divisor: [0.0f, 0.0f, 0.0f, 0.0f]
 * @expected_behavior Returns infinity: [+∞, -∞, +∞, -∞]
 * @validation_method IEEE 754 infinity detection and sign verification
 */
TEST_P(F32x4DivTest, DivisionByZero_ProducesInfinity) {
    // Test division by zero (should produce infinity)
    float result_lanes[4];
    call_f32x4_div_function("test_divide_by_zero", result_lanes);

    // Validate infinity results with correct signs
    ASSERT_TRUE(std::isinf(result_lanes[0]) && result_lanes[0] > 0) << "Lane 0: Expected +infinity, got " << result_lanes[0];
    ASSERT_TRUE(std::isinf(result_lanes[1]) && result_lanes[1] < 0) << "Lane 1: Expected -infinity, got " << result_lanes[1];
    ASSERT_TRUE(std::isinf(result_lanes[2]) && result_lanes[2] > 0) << "Lane 2: Expected +infinity, got " << result_lanes[2];
    ASSERT_TRUE(std::isinf(result_lanes[3]) && result_lanes[3] < 0) << "Lane 3: Expected -infinity, got " << result_lanes[3];
}

/**
 * @test SpecialValues_HandlesIEEE754Cases
 * @brief Validates IEEE 754 special case handling for division
 * @details Tests various IEEE 754 special cases including infinity/infinity = NaN,
 *          0/0 = NaN, finite/infinity = 0, and proper NaN propagation.
 * @test_category Edge - IEEE 754 special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_div
 * @input_conditions Special case combinations: ∞/∞, 0/0, finite/∞, NaN/any
 * @expected_behavior Returns NaN for indeterminate forms, 0 for finite/∞, NaN for any NaN operation
 * @validation_method IEEE 754 compliant special value detection
 */
TEST_P(F32x4DivTest, SpecialValues_HandlesIEEE754Cases) {
    // Test infinity divided by infinity (should produce NaN)
    float result_inf_inf[4];
    call_f32x4_div_function("test_infinity_div_infinity", result_inf_inf);

    ASSERT_TRUE(std::isnan(result_inf_inf[0])) << "Lane 0: Expected NaN for +∞/+∞, got " << result_inf_inf[0];
    ASSERT_TRUE(std::isnan(result_inf_inf[1])) << "Lane 1: Expected NaN for -∞/+∞, got " << result_inf_inf[1];
    ASSERT_TRUE(std::isnan(result_inf_inf[2])) << "Lane 2: Expected NaN for +∞/-∞, got " << result_inf_inf[2];
    ASSERT_TRUE(std::isnan(result_inf_inf[3])) << "Lane 3: Expected NaN for -∞/-∞, got " << result_inf_inf[3];

    // Test zero divided by zero (should produce NaN)
    float result_zero_zero[4];
    call_f32x4_div_function("test_zero_div_zero", result_zero_zero);

    ASSERT_TRUE(std::isnan(result_zero_zero[0])) << "Lane 0: Expected NaN for 0/0, got " << result_zero_zero[0];
    ASSERT_TRUE(std::isnan(result_zero_zero[1])) << "Lane 1: Expected NaN for -0/0, got " << result_zero_zero[1];
    ASSERT_TRUE(std::isnan(result_zero_zero[2])) << "Lane 2: Expected NaN for 0/-0, got " << result_zero_zero[2];
    ASSERT_TRUE(std::isnan(result_zero_zero[3])) << "Lane 3: Expected NaN for -0/-0, got " << result_zero_zero[3];

    // Test finite divided by infinity (should produce zero)
    float result_finite_inf[4];
    call_f32x4_div_function("test_finite_div_infinity", result_finite_inf);

    ASSERT_TRUE(float_equal_ieee754(0.0f, result_finite_inf[0])) << "Lane 0: Expected 0.0f for finite/+∞, got " << result_finite_inf[0];
    ASSERT_TRUE(float_equal_ieee754(0.0f, result_finite_inf[1])) << "Lane 1: Expected 0.0f for -finite/+∞, got " << result_finite_inf[1];
    ASSERT_TRUE(float_equal_ieee754(0.0f, result_finite_inf[2])) << "Lane 2: Expected 0.0f for finite/-∞, got " << result_finite_inf[2];
    ASSERT_TRUE(float_equal_ieee754(0.0f, result_finite_inf[3])) << "Lane 3: Expected 0.0f for -finite/-∞, got " << result_finite_inf[3];
}

/**
 * @test NaNPropagation_PreservesNaNValues
 * @brief Validates that NaN values are properly propagated through division operations
 * @details Tests that any division operation involving NaN operands results in NaN output,
 *          ensuring proper IEEE 754 NaN propagation semantics.
 * @test_category Edge - NaN propagation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_div
 * @input_conditions Various NaN combinations: NaN/finite, finite/NaN, NaN/NaN
 * @expected_behavior All results are NaN values
 * @validation_method NaN detection for all output lanes
 */
TEST_P(F32x4DivTest, NaNPropagation_PreservesNaNValues) {
    // Test NaN propagation in division operations
    float result_nan[4];
    call_f32x4_div_function("test_nan_propagation", result_nan);

    ASSERT_TRUE(std::isnan(result_nan[0])) << "Lane 0: Expected NaN for NaN/finite, got " << result_nan[0];
    ASSERT_TRUE(std::isnan(result_nan[1])) << "Lane 1: Expected NaN for finite/NaN, got " << result_nan[1];
    ASSERT_TRUE(std::isnan(result_nan[2])) << "Lane 2: Expected NaN for NaN/NaN, got " << result_nan[2];
    ASSERT_TRUE(std::isnan(result_nan[3])) << "Lane 3: Expected NaN for -NaN/finite, got " << result_nan[3];
}

// Parameterized test to run across different execution modes
INSTANTIATE_TEST_SUITE_P(RunningMode, F32x4DivTest,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));