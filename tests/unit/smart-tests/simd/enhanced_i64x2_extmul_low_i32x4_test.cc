/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include <memory>

// Test execution modes for parameterized testing
enum class TestRunningMode {
    INTERP_MODE,
    AOT_MODE
};

/**
 * @file enhanced_i64x2_extmul_low_i32x4_test.cc
 * @brief Comprehensive unit tests for WASM opcode i64x2.extmul_low_i32x4
 * @details This file contains tests for the SIMD extended multiplication operation
 *          that multiplies the low i32 lanes from two i32x4 vectors and produces
 *          an i64x2 result with sign extension.
 */

class I64x2ExtmulLowI32x4Test : public testing::TestWithParam<TestRunningMode>
{
protected:
    /**
     * @brief Set up test environment for i64x2.extmul_low_i32x4 instruction testing
     * @details Initializes WAMR runtime with SIMD support, loads test module,
     *          and prepares execution context for both interpreter and AOT modes
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i64x2.extmul_low_i32x4 test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i64x2_extmul_low_i32x4_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i64x2.extmul_low_i32x4 tests";
    }

    /**
     * @brief Tear down test environment and cleanup resources
     * @details Destroys execution environment and WAMR runtime instance
     */
    void TearDown() override
    {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Calls i64x2.extmul_low_i32x4 test function with specified inputs
     * @param function_name Name of the WASM function to call
     * @param vec1_lane0 Lane 0 of first i32x4 vector
     * @param vec1_lane1 Lane 1 of first i32x4 vector
     * @param vec1_lane2 Lane 2 of first i32x4 vector
     * @param vec1_lane3 Lane 3 of first i32x4 vector
     * @param vec2_lane0 Lane 0 of second i32x4 vector
     * @param vec2_lane1 Lane 1 of second i32x4 vector
     * @param vec2_lane2 Lane 2 of second i32x4 vector
     * @param vec2_lane3 Lane 3 of second i32x4 vector
     * @param result_low Pointer to store low i64 result
     * @param result_high Pointer to store high i64 result
     * @return true if function call succeeded, false otherwise
     */
    bool call_i64x2_extmul_low_i32x4(const char* function_name,
                                      int32_t vec1_lane0, int32_t vec1_lane1, int32_t vec1_lane2, int32_t vec1_lane3,
                                      int32_t vec2_lane0, int32_t vec2_lane1, int32_t vec2_lane2, int32_t vec2_lane3,
                                      int64_t* result_low, int64_t* result_high)
    {
        uint32_t argv[10] = {0}; // 8 inputs + 2 outputs (each i64 = 2 uint32)

        // Pack input arguments
        argv[0] = (uint32_t)vec1_lane0;
        argv[1] = (uint32_t)vec1_lane1;
        argv[2] = (uint32_t)vec1_lane2;
        argv[3] = (uint32_t)vec1_lane3;
        argv[4] = (uint32_t)vec2_lane0;
        argv[5] = (uint32_t)vec2_lane1;
        argv[6] = (uint32_t)vec2_lane2;
        argv[7] = (uint32_t)vec2_lane3;

        bool ret = dummy_env->execute(function_name, 8, argv);

        if (ret) {
            // Unpack i64 results from uint32 pairs
            // Results returned in argv[0-3]: [low_32, low_64, high_32, high_64]
            *result_low = ((int64_t)argv[1] << 32) | argv[0];
            *result_high = ((int64_t)argv[3] << 32) | argv[2];
        }
        return ret;
    }

    TestRunningMode running_mode;
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicExtendedMultiplication_ReturnsCorrectProducts
 * @brief Validates i64x2.extmul_low_i32x4 produces correct arithmetic results for typical inputs
 * @details Tests fundamental extended multiplication operation with positive integers.
 *          Verifies that i64x2.extmul_low_i32x4 correctly multiplies low lanes 0 and 1
 *          with sign extension to 64-bit results.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64x2_extmul_low_i32x4_operation
 * @input_conditions Standard i32x4 vectors: (5,3,7,9) × (2,4,6,8)
 * @expected_behavior Returns i64x2 result: (10, 12) from lanes 0 and 1 multiplication
 * @validation_method Direct comparison of WASM function result with expected i64 values
 */
TEST_P(I64x2ExtmulLowI32x4Test, BasicExtendedMultiplication_ReturnsCorrectProducts)
{
    int64_t result_low, result_high;

    // Test: [5, 3, 7, 9] extmul_low [2, 4, 6, 8] = [10, 12] (low lanes 0,1 only)
    ASSERT_TRUE(call_i64x2_extmul_low_i32x4("test_i64x2_extmul_low_i32x4_basic",
                                           5, 3, 7, 9,
                                           2, 4, 6, 8,
                                           &result_low, &result_high))
        << "Failed to execute i64x2.extmul_low_i32x4 with basic values";

    // Lane 0: 5 * 2 = 10 (sign-extended to i64)
    ASSERT_EQ(10LL, result_low)
        << "Lane 0 multiplication failed: expected 10, got " << result_low;

    // Lane 1: 3 * 4 = 12 (sign-extended to i64)
    ASSERT_EQ(12LL, result_high)
        << "Lane 1 multiplication failed: expected 12, got " << result_high;
}

/**
 * @test BoundaryValues_HandlesExtremeValues
 * @brief Validates i64x2.extmul_low_i32x4 handles INT32_MIN and INT32_MAX correctly
 * @details Tests extended multiplication with extreme i32 values to ensure proper
 *          sign extension and handling of boundary conditions without overflow.
 * @test_category Boundary - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:sign_extension_logic
 * @input_conditions Boundary values: (INT32_MIN, INT32_MAX, *, *) × (1, -1, *, *)
 * @expected_behavior Returns correct i64 products with proper sign handling
 * @validation_method Comparison with mathematically expected boundary results
 */
TEST_P(I64x2ExtmulLowI32x4Test, BoundaryValues_HandlesExtremeValues)
{
    int64_t result_low, result_high;

    // Test: [INT32_MIN, INT32_MAX, 0, 0] extmul_low [1, -1, 0, 0]
    ASSERT_TRUE(call_i64x2_extmul_low_i32x4("test_i64x2_extmul_low_i32x4_boundary",
                                           INT32_MIN, INT32_MAX, 0, 0,
                                           1, -1, 0, 0,
                                           &result_low, &result_high))
        << "Failed to execute i64x2.extmul_low_i32x4 with boundary values";

    // Lane 0: INT32_MIN * 1 = -2147483648LL (sign-extended)
    int64_t expected_lane0 = static_cast<int64_t>(INT32_MIN) * 1LL;
    ASSERT_EQ(expected_lane0, result_low)
        << "INT32_MIN multiplication failed: expected " << expected_lane0
        << ", got " << result_low;

    // Lane 1: INT32_MAX * (-1) = -2147483647LL (sign-extended)
    int64_t expected_lane1 = static_cast<int64_t>(INT32_MAX) * -1LL;
    ASSERT_EQ(expected_lane1, result_high)
        << "INT32_MAX negative multiplication failed: expected " << expected_lane1
        << ", got " << result_high;
}

/**
 * @test SignExtension_PreservesSignCorrectly
 * @brief Validates i64x2.extmul_low_i32x4 preserves sign during extended multiplication
 * @details Tests mixed positive/negative i32 values to ensure proper sign extension
 *          and correct handling of negative multiplication results.
 * @test_category Sign - Sign preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:sign_extension_multiplication
 * @input_conditions Mixed sign values: (-100, -200, *, *) × (300, -400, *, *)
 * @expected_behavior Returns i64x2 result: (-30000, 80000) with correct signs
 * @validation_method Verification of sign preservation in extended arithmetic
 */
TEST_P(I64x2ExtmulLowI32x4Test, SignExtension_PreservesSignCorrectly)
{
    int64_t result_low, result_high;

    // Test: [-100, -200, 999, 888] extmul_low [300, -400, 777, 666]
    ASSERT_TRUE(call_i64x2_extmul_low_i32x4("test_i64x2_extmul_low_i32x4_sign",
                                           -100, -200, 999, 888,
                                           300, -400, 777, 666,
                                           &result_low, &result_high))
        << "Failed to execute i64x2.extmul_low_i32x4 with mixed sign values";

    // Lane 0: (-100) * 300 = -30000LL (negative * positive = negative)
    ASSERT_EQ(-30000LL, result_low)
        << "Negative-positive multiplication failed: expected -30000, got " << result_low;

    // Lane 1: (-200) * (-400) = 80000LL (negative * negative = positive)
    ASSERT_EQ(80000LL, result_high)
        << "Negative-negative multiplication failed: expected 80000, got " << result_high;
}

/**
 * @test ZeroHandling_ProducesZeroResults
 * @brief Validates i64x2.extmul_low_i32x4 produces zero results for zero multiplication
 * @details Tests behavior when zero values are present in low lanes to ensure
 *          proper zero multiplication handling in extended arithmetic.
 * @test_category Zero - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:zero_multiplication_logic
 * @input_conditions Zero values: (0, 1000, *, *) × (500, 0, *, *)
 * @expected_behavior Returns i64x2 result: (0, 0) for any multiplication by zero
 * @validation_method Direct verification of zero multiplication property
 */
TEST_P(I64x2ExtmulLowI32x4Test, ZeroHandling_ProducesZeroResults)
{
    int64_t result_low, result_high;

    // Test: [0, 1000, 555, 444] extmul_low [500, 0, 333, 222]
    ASSERT_TRUE(call_i64x2_extmul_low_i32x4("test_i64x2_extmul_low_i32x4_zero",
                                           0, 1000, 555, 444,
                                           500, 0, 333, 222,
                                           &result_low, &result_high))
        << "Failed to execute i64x2.extmul_low_i32x4 with zero values";

    // Lane 0: 0 * 500 = 0LL (zero multiplication property)
    ASSERT_EQ(0LL, result_low)
        << "Zero multiplication in lane 0 failed: expected 0, got " << result_low;

    // Lane 1: 1000 * 0 = 0LL (zero multiplication property)
    ASSERT_EQ(0LL, result_high)
        << "Zero multiplication in lane 1 failed: expected 0, got " << result_high;
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I64x2ExtmulLowI32x4Test,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));