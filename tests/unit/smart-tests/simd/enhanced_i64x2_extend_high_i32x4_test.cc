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
 * @brief Test class for i64x2.extend_high_i32x4 SIMD opcode
 * @details Provides WAMR runtime environment with proper initialization and cleanup.
 *          Tests the SIMD instruction that extends the high 2 elements of an i32x4 vector
 *          to i64 values with sign extension. Takes lanes 2 and 3 from input i32x4 vector
 *          and produces i64x2 vector with sign-extended values.
 */
class I64x2ExtendHighI32x4TestSuite : public testing::TestWithParam<TestRunningMode>
{
protected:
    /**
     * @brief Set up test environment before each test case
     * @details Initializes WAMR runtime, loads test module, and prepares execution context
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i64x2.extend_high_i32x4 test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i64x2_extend_high_i32x4_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i64x2.extend_high_i32x4 tests";
    }

    /**
     * @brief Clean up test environment after each test case
     * @details Properly releases all allocated resources using RAII pattern
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM extend_high test function
     * @details Invokes the WASM test function that performs i64x2.extend_high_i32x4 operation
     * @param input_data Array of 4 i32 values representing i32x4 input vector
     * @param result_data Output array to receive 2 i64 values representing i64x2 result vector
     */
    bool call_extend_high_test(const int32_t input_data[4], int64_t result_data[2])
    {
        // Prepare arguments: 4 input i32 values as separate parameters
        uint32_t argv[4];

        // Set up input vector lanes
        for (int i = 0; i < 4; i++) {
            argv[i] = static_cast<uint32_t>(input_data[i]);
        }

        // Call WASM function with 4 arguments (one per i32 lane)
        bool call_success = dummy_env->execute("test_extend_high", 4, argv);

        if (call_success) {
            // Extract result values from the 4 return values (2 i64s as 4 i32s)
            // Each i64 is returned as two i32 values: low 32 bits, high 32 bits
            uint64_t result0 = static_cast<uint64_t>(argv[0]) |
                              (static_cast<uint64_t>(argv[1]) << 32);
            uint64_t result1 = static_cast<uint64_t>(argv[2]) |
                              (static_cast<uint64_t>(argv[3]) << 32);

            result_data[0] = static_cast<int64_t>(result0);
            result_data[1] = static_cast<int64_t>(result1);
        }

        return call_success;
    }

    TestRunningMode running_mode;
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicExtension_TypicalValues_ReturnsCorrectResults
 * @brief Validates i64x2.extend_high_i32x4 correctly extends typical positive values
 * @details Tests fundamental sign extension operation with positive integers in high lanes.
 *          Verifies that i64x2.extend_high_i32x4 correctly sign-extends lanes 2 and 3
 *          from i32 to i64 while preserving the values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_extend_high_i32x4
 * @input_conditions Standard integer combinations: lanes 0,1 irrelevant, lanes 2,3 have test values
 * @expected_behavior Returns correct sign-extended i64 values: 100LL, 200LL
 * @validation_method Direct comparison of WASM function result with expected i64 values
 */
TEST_P(I64x2ExtendHighI32x4TestSuite, BasicExtension_TypicalValues_ReturnsCorrectResults)
{
    // Test input: only lanes 2,3 (high lanes) matter for this operation
    int32_t input[4] = {10, 20, 100, 200}; // lanes 0,1 ignored; lanes 2,3 extended
    int64_t result[2];
    int64_t expected[2] = {100LL, 200LL};

    ASSERT_TRUE(call_extend_high_test(input, result)) << "WASM function call failed";

    // Validate each lane of the result
    for (int i = 0; i < 2; i++) {
        ASSERT_EQ(result[i], expected[i])
            << "Lane " << i << " sign extension failed: expected " << expected[i]
            << ", got " << result[i];
    }
}

/**
 * @test SignExtension_NegativeValues_HandlesCorrectly
 * @brief Validates proper sign extension of negative i32 values to i64
 * @details Tests sign extension behavior with negative integers in high lanes.
 *          Verifies that negative i32 values are correctly sign-extended to i64
 *          with proper sign bit propagation to upper 32 bits.
 * @test_category Main - Sign extension validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_extend_high_i32x4
 * @input_conditions Negative i32 values: -1000, -2000 in high lanes
 * @expected_behavior Returns correct sign-extended i64 values: -1000LL, -2000LL
 * @validation_method Direct comparison verifying negative value sign extension
 */
TEST_P(I64x2ExtendHighI32x4TestSuite, SignExtension_NegativeValues_HandlesCorrectly)
{
    // Test input: negative values in high lanes (2,3)
    int32_t input[4] = {0, 0, -1000, -2000};
    int64_t result[2];
    int64_t expected[2] = {-1000LL, -2000LL};

    ASSERT_TRUE(call_extend_high_test(input, result)) << "WASM function call failed";

    // Validate each lane of the result
    for (int i = 0; i < 2; i++) {
        ASSERT_EQ(result[i], expected[i])
            << "Negative value lane " << i << " extension failed: expected " << expected[i]
            << ", got " << result[i];
    }
}

/**
 * @test BoundaryValues_MaxMinLimits_HandlesExtremesCorrectly
 * @brief Validates proper sign extension of i32 boundary values (INT32_MIN, INT32_MAX)
 * @details Tests sign extension behavior at the boundaries of signed 32-bit integers.
 *          Verifies that maximum positive (INT32_MAX) and minimum negative (INT32_MIN) values
 *          are correctly extended to their 64-bit representations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_extend_high_i32x4
 * @input_conditions i32 boundary values: INT32_MAX (2147483647), INT32_MIN (-2147483648)
 * @expected_behavior INT32_MAX → 2147483647LL, INT32_MIN → -2147483648LL with proper sign extension
 * @validation_method Direct comparison verifying boundary value sign extension
 */
TEST_P(I64x2ExtendHighI32x4TestSuite, BoundaryValues_MaxMinLimits_HandlesExtremesCorrectly)
{
    // Test input: boundary values at i32 limits in high lanes
    int32_t input[4] = {0, 0, INT32_MAX, INT32_MIN};
    int64_t result[2];
    int64_t expected[2] = {static_cast<int64_t>(INT32_MAX), static_cast<int64_t>(INT32_MIN)};

    ASSERT_TRUE(call_extend_high_test(input, result)) << "WASM function call failed";

    // Validate each lane focuses on boundary values
    for (int i = 0; i < 2; i++) {
        ASSERT_EQ(result[i], expected[i])
            << "Boundary value lane " << i << " extension failed: expected " << expected[i]
            << ", got " << result[i];
    }

    // Specific validation for critical boundary cases
    ASSERT_EQ(result[0], static_cast<int64_t>(INT32_MAX)) << "Maximum positive i32 extension failed";
    ASSERT_EQ(result[1], static_cast<int64_t>(INT32_MIN)) << "Minimum negative i32 extension failed";
}

/**
 * @test MixedSigns_PositiveNegative_ReturnsCorrectResults
 * @brief Validates mixed positive and negative values in high lanes
 * @details Tests sign extension with one positive and one negative value.
 *          Verifies that both positive and negative values are correctly handled
 *          in the same operation without interference.
 * @test_category Main - Mixed sign validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_extend_high_i32x4
 * @input_conditions Mixed signs: 42 (positive), -42 (negative) in high lanes
 * @expected_behavior Returns correct sign-extended i64 values: 42LL, -42LL
 * @validation_method Direct comparison of both positive and negative results
 */
TEST_P(I64x2ExtendHighI32x4TestSuite, MixedSigns_PositiveNegative_ReturnsCorrectResults)
{
    // Test input: mixed positive/negative in high lanes
    int32_t input[4] = {999, 888, 42, -42}; // lanes 0,1 ignored; lanes 2,3 are 42, -42
    int64_t result[2];
    int64_t expected[2] = {42LL, -42LL};

    ASSERT_TRUE(call_extend_high_test(input, result)) << "WASM function call failed";

    // Validate mixed sign results
    for (int i = 0; i < 2; i++) {
        ASSERT_EQ(result[i], expected[i])
            << "Mixed sign lane " << i << " extension failed: expected " << expected[i]
            << ", got " << result[i];
    }

    // Specific validation for mixed signs
    ASSERT_EQ(result[0], 42LL) << "Positive value extension failed";
    ASSERT_EQ(result[1], -42LL) << "Negative value extension failed";
}

/**
 * @test ZeroValues_AllZeros_PreservesZeros
 * @brief Validates zero values in high lanes produce zero results
 * @details Tests identity behavior where high lanes contain zero values.
 *          Verifies that zero values are properly extended to zero without corruption.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_extend_high_i32x4
 * @input_conditions Zero values: 0, 0 in high lanes
 * @expected_behavior Returns zero-extended i64 values: 0LL, 0LL
 * @validation_method Direct comparison verifying zero value extension
 */
TEST_P(I64x2ExtendHighI32x4TestSuite, ZeroValues_AllZeros_PreservesZeros)
{
    // Test input: zeros in high lanes
    int32_t input[4] = {111, 222, 0, 0}; // lanes 0,1 ignored; lanes 2,3 are zeros
    int64_t result[2];
    int64_t expected[2] = {0LL, 0LL};

    ASSERT_TRUE(call_extend_high_test(input, result)) << "WASM function call failed";

    // Validate zero extension
    for (int i = 0; i < 2; i++) {
        ASSERT_EQ(result[i], expected[i])
            << "Zero value lane " << i << " extension failed: expected " << expected[i]
            << ", got " << result[i];
    }

    // Specific validation for zero preservation
    ASSERT_EQ(result[0], 0LL) << "First zero value not preserved during extension";
    ASSERT_EQ(result[1], 0LL) << "Second zero value not preserved during extension";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I64x2ExtendHighI32x4TestSuite,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));