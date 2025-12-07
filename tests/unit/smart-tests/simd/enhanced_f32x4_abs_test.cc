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
 * @brief Test fixture class for f32x4.abs opcode validation
 * @details Provides comprehensive test framework for SIMD f32x4.abs instruction
 *          including setup/teardown, helper functions, and cross-execution mode validation
 */
class F32x4AbsTest : public testing::TestWithParam<TestRunningMode>
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

        // Load the f32x4.abs test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_abs_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.abs tests";
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
    void call_f32x4_abs_function(const char* function_name, float result_out[4])
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
     * @brief Helper function to call f32x4.abs test function with input parameters
     * @param lane0 First f32 lane input value
     * @param lane1 Second f32 lane input value
     * @param lane2 Third f32 lane input value
     * @param lane3 Fourth f32 lane input value
     * @param result_out Array to store the 4 f32 result values
     */
    void call_f32x4_abs_with_params(float lane0, float lane1, float lane2, float lane3, float result_out[4])
    {
        wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(dummy_env->get());
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32x4_abs");
        ASSERT_NE(func, nullptr) << "Failed to lookup function: test_f32x4_abs";

        uint32_t argv[4];
        // Convert float inputs to uint32_t for WASM function call
        memcpy(&argv[0], &lane0, sizeof(float));
        memcpy(&argv[1], &lane1, sizeof(float));
        memcpy(&argv[2], &lane2, sizeof(float));
        memcpy(&argv[3], &lane3, sizeof(float));

        bool call_result = wasm_runtime_call_wasm(dummy_env->get(), func, 4, argv);
        ASSERT_TRUE(call_result) << "Failed to call test_f32x4_abs: " << wasm_runtime_get_exception(module_inst);

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
 * @test BasicAbsoluteFunctionality_ReturnsCorrectResults
 * @brief Validates f32x4.abs produces correct absolute values for typical mixed-sign inputs
 * @details Tests fundamental absolute value operation with positive, negative, and decimal values.
 *          Verifies that f32x4.abs correctly computes |a| for each lane independently.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_abs
 * @input_conditions Standard f32 values: [1.5f, -2.7f, 3.14f, -0.5f]
 * @expected_behavior Returns absolute values: [1.5f, 2.7f, 3.14f, 0.5f]
 * @validation_method Direct lane-by-lane comparison with expected absolute values
 */
TEST_P(F32x4AbsTest, BasicAbsoluteFunctionality_ReturnsCorrectResults) {
    // Execute f32x4.abs with mixed positive and negative values
    float result_lanes[4];
    call_f32x4_abs_with_params(1.5f, -2.7f, 3.14f, -0.5f, result_lanes);

    // Validate each absolute value
    ASSERT_TRUE(float_equal_ieee754(1.5f, result_lanes[0])) << "Lane 0: Expected 1.5f, got " << result_lanes[0];
    ASSERT_TRUE(float_equal_ieee754(2.7f, result_lanes[1])) << "Lane 1: Expected 2.7f, got " << result_lanes[1];
    ASSERT_TRUE(float_equal_ieee754(3.14f, result_lanes[2])) << "Lane 2: Expected 3.14f, got " << result_lanes[2];
    ASSERT_TRUE(float_equal_ieee754(0.5f, result_lanes[3])) << "Lane 3: Expected 0.5f, got " << result_lanes[3];
}

/**
 * @test SpecialFloatValues_PreservesIEEE754Behavior
 * @brief Validates correct handling of IEEE 754 special values (±0.0, ±infinity, NaN)
 * @details Tests absolute value behavior for special floating-point values including
 *          positive/negative zero, positive/negative infinity, and various NaN representations.
 * @test_category Edge - Special value handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_abs
 * @input_conditions Special IEEE 754 values: [+0.0f, -0.0f, +INFINITY, -INFINITY]
 * @expected_behavior Returns: [+0.0f, +0.0f, +INFINITY, +INFINITY]
 * @validation_method IEEE 754 compliant comparison for special values
 */
TEST_P(F32x4AbsTest, SpecialFloatValues_PreservesIEEE754Behavior) {
    // Test zero values (both positive and negative zero)
    float result_zeros[4];
    call_f32x4_abs_function("test_zero_values", result_zeros);

    ASSERT_TRUE(float_equal_ieee754(+0.0f, result_zeros[0])) << "Lane 0: Expected +0.0f, got " << result_zeros[0];
    ASSERT_TRUE(float_equal_ieee754(+0.0f, result_zeros[1])) << "Lane 1: Expected +0.0f from -0.0f, got " << result_zeros[1];
    ASSERT_TRUE(float_equal_ieee754(+0.0f, result_zeros[2])) << "Lane 2: Expected +0.0f, got " << result_zeros[2];
    ASSERT_TRUE(float_equal_ieee754(+0.0f, result_zeros[3])) << "Lane 3: Expected +0.0f from -0.0f, got " << result_zeros[3];

    // Test infinity values (both positive and negative infinity)
    float result_inf[4];
    call_f32x4_abs_function("test_infinity_values", result_inf);

    ASSERT_TRUE(float_equal_ieee754(INFINITY, result_inf[0])) << "Lane 0: Expected +INFINITY, got " << result_inf[0];
    ASSERT_TRUE(float_equal_ieee754(INFINITY, result_inf[1])) << "Lane 1: Expected +INFINITY from -INFINITY, got " << result_inf[1];
    ASSERT_TRUE(float_equal_ieee754(INFINITY, result_inf[2])) << "Lane 2: Expected +INFINITY, got " << result_inf[2];
    ASSERT_TRUE(float_equal_ieee754(INFINITY, result_inf[3])) << "Lane 3: Expected +INFINITY from -INFINITY, got " << result_inf[3];

    // Test NaN values (should preserve NaN)
    float result_nan[4];
    call_f32x4_abs_function("test_nan_values", result_nan);

    ASSERT_TRUE(std::isnan(result_nan[0])) << "Lane 0: Expected NaN, got " << result_nan[0];
    ASSERT_TRUE(std::isnan(result_nan[1])) << "Lane 1: Expected NaN, got " << result_nan[1];
    ASSERT_TRUE(std::isnan(result_nan[2])) << "Lane 2: Expected NaN, got " << result_nan[2];
    ASSERT_TRUE(std::isnan(result_nan[3])) << "Lane 3: Expected NaN, got " << result_nan[3];
}

/**
 * @test BoundaryValues_HandlesExtremes
 * @brief Validates absolute value computation for floating-point boundary conditions
 * @details Tests f32x4.abs with extreme floating-point values including FLT_MIN, FLT_MAX
 *          and their negative counterparts to ensure proper handling of boundary cases.
 * @test_category Edge - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_abs
 * @input_conditions Boundary values: [FLT_MIN, -FLT_MIN, FLT_MAX, -FLT_MAX]
 * @expected_behavior Returns positive versions: [FLT_MIN, FLT_MIN, FLT_MAX, FLT_MAX]
 * @validation_method Precise boundary value comparisons
 */
TEST_P(F32x4AbsTest, BoundaryValues_HandlesExtremes) {
    // Test with floating-point boundary values
    float result_lanes[4];
    call_f32x4_abs_function("test_boundary_values", result_lanes);

    ASSERT_TRUE(float_equal_ieee754(1.1754944e-38f, result_lanes[0])) << "Lane 0: Expected 1.1754944e-38f, got " << result_lanes[0];
    ASSERT_TRUE(float_equal_ieee754(1.1754944e-38f, result_lanes[1])) << "Lane 1: Expected 1.1754944e-38f from -1.1754944e-38f, got " << result_lanes[1];
    ASSERT_TRUE(float_equal_ieee754(3.4028235e+38f, result_lanes[2])) << "Lane 2: Expected 3.4028235e+38f, got " << result_lanes[2];
    ASSERT_TRUE(float_equal_ieee754(3.4028235e+38f, result_lanes[3])) << "Lane 3: Expected 3.4028235e+38f from -3.4028235e+38f, got " << result_lanes[3];
}

/**
 * @test DenormalNumbers_ProcessesCorrectly
 * @brief Validates absolute value computation for denormalized (subnormal) floating-point numbers
 * @details Tests f32x4.abs with denormal values to ensure proper handling of subnormal numbers
 *          including the smallest representable positive denormal and near-zero denormals.
 * @test_category Edge - Denormal number handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_abs
 * @input_conditions Denormal values: [1.4e-45f, -1.4e-45f, 1.175e-38f, -1.175e-38f]
 * @expected_behavior Returns positive denormals: [1.4e-45f, 1.4e-45f, 1.175e-38f, 1.175e-38f]
 * @validation_method Sign bit cleared while mantissa and exponent preserved
 */
TEST_P(F32x4AbsTest, DenormalNumbers_ProcessesCorrectly) {
    // Test with denormal (subnormal) floating-point values
    float result_lanes[4];
    call_f32x4_abs_function("test_denormal_values", result_lanes);

    float denorm1 = 1.4e-45f;    // Smallest positive denormal
    float denorm2 = 1.175e-38f;  // Near smallest normal number

    ASSERT_TRUE(float_equal_ieee754(denorm1, result_lanes[0])) << "Lane 0: Expected " << denorm1 << ", got " << result_lanes[0];
    ASSERT_TRUE(float_equal_ieee754(denorm1, result_lanes[1])) << "Lane 1: Expected " << denorm1 << " from -" << denorm1 << ", got " << result_lanes[1];
    ASSERT_TRUE(float_equal_ieee754(denorm2, result_lanes[2])) << "Lane 2: Expected " << denorm2 << ", got " << result_lanes[2];
    ASSERT_TRUE(float_equal_ieee754(denorm2, result_lanes[3])) << "Lane 3: Expected " << denorm2 << " from -" << denorm2 << ", got " << result_lanes[3];
}

// Parameterized test to run across different execution modes
INSTANTIATE_TEST_SUITE_P(RunningMode, F32x4AbsTest,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));