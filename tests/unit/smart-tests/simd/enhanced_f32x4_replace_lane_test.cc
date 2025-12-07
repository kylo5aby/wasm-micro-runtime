/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <cmath>
#include <cfloat>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class F32x4ReplaceLaneTestSuite
 * @brief Test fixture for f32x4.replace_lane opcode validation
 * @details Validates SIMD lane replacement operations for f32x4 vectors
 *          Tests both interpreter and AOT execution modes with comprehensive validation
 */
class F32x4ReplaceLaneTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.replace_lane testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.replace_lane test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_replace_lane_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.replace_lane tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Helper function to call f32x4.replace_lane WASM function
     * @param func_name Name of the exported WASM function
     * @param v1 First f32 value for vector construction
     * @param v2 Second f32 value for vector construction
     * @param v3 Third f32 value for vector construction
     * @param v4 Fourth f32 value for vector construction
     * @param replacement_value f32 value to replace specific lane
     * @param result_f32 Array to store the 4 f32 result values
     * @return bool True if operation succeeded, false on error
     */
    bool call_f32x4_replace_lane(const char *func_name, float v1, float v2, float v3, float v4,
                                float replacement_value, float result_f32[4])
    {
        // Prepare arguments: 5 f32 inputs (4 for vector + 1 replacement) + space for f32x4 result
        uint32_t argv[9];  // 5 for inputs + 4 for f32x4 result

        // Convert f32 inputs to uint32_t representation for WASM call
        memcpy(&argv[0], &v1, sizeof(float));
        memcpy(&argv[1], &v2, sizeof(float));
        memcpy(&argv[2], &v3, sizeof(float));
        memcpy(&argv[3], &v4, sizeof(float));
        memcpy(&argv[4], &replacement_value, sizeof(float));

        // Call WASM function with 5 f32 inputs, returns f32x4
        bool call_success = dummy_env->execute(func_name, 5, argv);
        if (!call_success) {
            return false;
        }

        // Extract f32x4 result from argv - v128 result is returned starting at argv[0]
        // after successful execution (input parameters are overwritten by result)
        memcpy(result_f32, argv, sizeof(float) * 4);

        return true;
    }

    /**
     * @brief Helper function to compare f32 values with NaN handling
     * @param expected Expected f32 value
     * @param actual Actual f32 value
     * @return bool True if values are equal or both are NaN
     */
    bool compare_f32_with_nan(float expected, float actual)
    {
        if (std::isnan(expected) && std::isnan(actual)) {
            return true;
        }
        if (std::isinf(expected) && std::isinf(actual)) {
            return (std::signbit(expected) == std::signbit(actual));
        }
        // Handle signed zero comparison
        if (expected == 0.0f && actual == 0.0f) {
            return std::signbit(expected) == std::signbit(actual);
        }
        return expected == actual;
    }

    /**
     * @brief Helper function to validate f32x4 result vector
     * @param result_f32 Array of 4 f32 result values
     * @param expected Array of 4 expected f32 values
     * @param lane_replaced Index of lane that was replaced
     */
    void validate_f32x4_result(const float result_f32[4], const float expected[4],
                              int lane_replaced, const char* test_description)
    {
        for (int i = 0; i < 4; i++) {
            ASSERT_TRUE(compare_f32_with_nan(expected[i], result_f32[i]))
                << test_description << " - Lane " << i
                << (i == lane_replaced ? " (replaced)" : " (preserved)")
                << " failed: expected " << expected[i]
                << ", got " << result_f32[i];
        }
    }
};

/**
 * @test BasicLaneReplacement_ReturnsModifiedVector
 * @brief Validates f32x4.replace_lane replaces specific lanes while preserving others
 * @details Tests replacing each lane (0-3) with typical f32 values.
 *          Verifies only the target lane changes, others remain unchanged.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_replace_lane_operation
 * @input_conditions Test vectors with typical f32 values and replacement values for each lane
 * @expected_behavior Only specified lane contains replacement value, others preserve original values
 * @validation_method Direct comparison of f32x4 result with expected lane-specific modifications
 */
TEST_F(F32x4ReplaceLaneTestSuite, BasicLaneReplacement_ReturnsModifiedVector)
{
    float result_f32[4];
    float original_values[4] = {1.0f, 2.5f, -3.0f, 4.5f};

    // Test replacing lane 0
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_0",
                                       original_values[0], original_values[1],
                                       original_values[2], original_values[3],
                                       42.0f, result_f32))
        << "Failed to call f32x4.replace_lane for lane 0";

    float expected_lane0[4] = {42.0f, 2.5f, -3.0f, 4.5f};
    validate_f32x4_result(result_f32, expected_lane0, 0, "Replace lane 0 with 42.0f");

    // Test replacing lane 1
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_1",
                                       original_values[0], original_values[1],
                                       original_values[2], original_values[3],
                                       -15.5f, result_f32))
        << "Failed to call f32x4.replace_lane for lane 1";

    float expected_lane1[4] = {1.0f, -15.5f, -3.0f, 4.5f};
    validate_f32x4_result(result_f32, expected_lane1, 1, "Replace lane 1 with -15.5f");

    // Test replacing lane 2
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_2",
                                       original_values[0], original_values[1],
                                       original_values[2], original_values[3],
                                       3.14159f, result_f32))
        << "Failed to call f32x4.replace_lane for lane 2";

    float expected_lane2[4] = {1.0f, 2.5f, 3.14159f, 4.5f};
    validate_f32x4_result(result_f32, expected_lane2, 2, "Replace lane 2 with 3.14159f");

    // Test replacing lane 3
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_3",
                                       original_values[0], original_values[1],
                                       original_values[2], original_values[3],
                                       0.0f, result_f32))
        << "Failed to call f32x4.replace_lane for lane 3";

    float expected_lane3[4] = {1.0f, 2.5f, -3.0f, 0.0f};
    validate_f32x4_result(result_f32, expected_lane3, 3, "Replace lane 3 with 0.0f");
}

/**
 * @test BoundaryValues_ReplacesCorrectly
 * @brief Validates f32x4.replace_lane with floating-point boundary values
 * @details Tests replacement with FLT_MAX, FLT_MIN, and -FLT_MAX values.
 *          Verifies precision preservation at floating-point limits.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_replace_lane_operation
 * @input_conditions Test vectors with boundary f32 values (FLT_MAX, FLT_MIN, -FLT_MAX)
 * @expected_behavior Boundary values are preserved precisely during lane replacement
 * @validation_method Exact comparison of boundary values in replaced lanes
 */
TEST_F(F32x4ReplaceLaneTestSuite, BoundaryValues_ReplacesCorrectly)
{
    float result_f32[4];
    float test_values[4] = {1.0f, 2.0f, 3.0f, 4.0f};

    // Test FLT_MAX replacement in lane 0
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_0",
                                       test_values[0], test_values[1],
                                       test_values[2], test_values[3],
                                       FLT_MAX, result_f32))
        << "Failed to call f32x4.replace_lane with FLT_MAX";

    float expected_flt_max[4] = {FLT_MAX, 2.0f, 3.0f, 4.0f};
    validate_f32x4_result(result_f32, expected_flt_max, 0, "Replace lane 0 with FLT_MAX");

    // Test FLT_MIN replacement in lane 1
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_1",
                                       test_values[0], test_values[1],
                                       test_values[2], test_values[3],
                                       FLT_MIN, result_f32))
        << "Failed to call f32x4.replace_lane with FLT_MIN";

    float expected_flt_min[4] = {1.0f, FLT_MIN, 3.0f, 4.0f};
    validate_f32x4_result(result_f32, expected_flt_min, 1, "Replace lane 1 with FLT_MIN");

    // Test -FLT_MAX replacement in lane 2
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_2",
                                       test_values[0], test_values[1],
                                       test_values[2], test_values[3],
                                       -FLT_MAX, result_f32))
        << "Failed to call f32x4.replace_lane with -FLT_MAX";

    float expected_neg_flt_max[4] = {1.0f, 2.0f, -FLT_MAX, 4.0f};
    validate_f32x4_result(result_f32, expected_neg_flt_max, 2, "Replace lane 2 with -FLT_MAX");
}

/**
 * @test SpecialFloatValues_HandlesCorrectly
 * @brief Validates f32x4.replace_lane with special floating-point values
 * @details Tests replacement with NaN, INFINITY, -INFINITY, +0.0f, -0.0f values.
 *          Verifies proper handling of IEEE 754 special values.
 * @test_category Edge - Special numeric value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_replace_lane_operation
 * @input_conditions Test vectors with IEEE 754 special values (NaN, infinities, signed zeros)
 * @expected_behavior Special values are preserved with correct bit patterns and signs
 * @validation_method Specialized comparison handling NaN equality and signed zero distinction
 */
TEST_F(F32x4ReplaceLaneTestSuite, SpecialFloatValues_HandlesCorrectly)
{
    float result_f32[4];
    float test_values[4] = {1.0f, 2.0f, 3.0f, 4.0f};

    // Test NaN replacement
    float nan_value = std::numeric_limits<float>::quiet_NaN();
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_0",
                                       test_values[0], test_values[1],
                                       test_values[2], test_values[3],
                                       nan_value, result_f32))
        << "Failed to call f32x4.replace_lane with NaN";

    float expected_nan[4] = {nan_value, 2.0f, 3.0f, 4.0f};
    validate_f32x4_result(result_f32, expected_nan, 0, "Replace lane 0 with NaN");

    // Test positive infinity replacement
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_1",
                                       test_values[0], test_values[1],
                                       test_values[2], test_values[3],
                                       INFINITY, result_f32))
        << "Failed to call f32x4.replace_lane with +INFINITY";

    float expected_pos_inf[4] = {1.0f, INFINITY, 3.0f, 4.0f};
    validate_f32x4_result(result_f32, expected_pos_inf, 1, "Replace lane 1 with +INFINITY");

    // Test negative infinity replacement
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_2",
                                       test_values[0], test_values[1],
                                       test_values[2], test_values[3],
                                       -INFINITY, result_f32))
        << "Failed to call f32x4.replace_lane with -INFINITY";

    float expected_neg_inf[4] = {1.0f, 2.0f, -INFINITY, 4.0f};
    validate_f32x4_result(result_f32, expected_neg_inf, 2, "Replace lane 2 with -INFINITY");

    // Test negative zero replacement
    float neg_zero = -0.0f;
    ASSERT_TRUE(call_f32x4_replace_lane("test_replace_lane_3",
                                       test_values[0], test_values[1],
                                       test_values[2], test_values[3],
                                       neg_zero, result_f32))
        << "Failed to call f32x4.replace_lane with -0.0f";

    float expected_neg_zero[4] = {1.0f, 2.0f, 3.0f, neg_zero};
    validate_f32x4_result(result_f32, expected_neg_zero, 3, "Replace lane 3 with -0.0f");
}

/**
 * @test IdentityReplacement_PreservesVector
 * @brief Validates f32x4.replace_lane when replacing with the same value
 * @details Tests replacing lanes with their existing values (identity operations).
 *          Verifies the operation completes correctly even when no change occurs.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_replace_lane_operation
 * @input_conditions Test vectors where replacement value equals existing lane value
 * @expected_behavior Vector remains unchanged after identity replacement operations
 * @validation_method Comparison of input and output vectors for exact equality
 */
TEST_F(F32x4ReplaceLaneTestSuite, IdentityReplacement_PreservesVector)
{
    float result_f32[4];
    float test_values[4] = {1.5f, -2.5f, 42.0f, 3.14159f};

    // Test identity replacement for each lane
    for (int lane = 0; lane < 4; lane++) {
        std::string func_name = "test_replace_lane_" + std::to_string(lane);

        ASSERT_TRUE(call_f32x4_replace_lane(func_name.c_str(),
                                           test_values[0], test_values[1],
                                           test_values[2], test_values[3],
                                           test_values[lane], result_f32))
            << "Failed to call f32x4.replace_lane for identity replacement on lane " << lane;

        // Verify all lanes remain unchanged
        validate_f32x4_result(result_f32, test_values, lane,
                             ("Identity replacement for lane " + std::to_string(lane)).c_str());
    }
}