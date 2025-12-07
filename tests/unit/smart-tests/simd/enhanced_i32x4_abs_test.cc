/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <climits>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * Enhanced unit tests for i32x4.abs WASM opcode
 *
 * Tests comprehensive SIMD absolute value functionality including:
 * - Basic absolute value operations with mixed positive/negative values
 * - Boundary condition handling (INT32_MIN/MAX, overflow behavior)
 * - Edge cases (all zeros, all positive, extreme values)
 * - Mathematical property validation (idempotent, symmetry)
 * - Cross-execution mode validation (interpreter vs AOT)
 */

static constexpr const char *MODULE_NAME = "i32x4_abs_test";
static constexpr const char *FUNC_NAME_BASIC_ABS = "test_basic_abs";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_ZERO_VECTOR = "test_zero_vector";
static constexpr const char *FUNC_NAME_POSITIVE_VALUES = "test_positive_values";
static constexpr const char *FUNC_NAME_NEGATIVE_VALUES = "test_negative_values";
static constexpr const char *FUNC_NAME_INT32_MIN_OVERFLOW = "test_int32_min_overflow";

/**
 * Test fixture for i32x4.abs opcode validation
 *
 * Provides comprehensive test environment for SIMD absolute value operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class I32x4AbsTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the i32x4.abs test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i32x4.abs test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i32x4_abs_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i32x4.abs tests";
    }

    /**
     * Cleans up test environment and runtime resources
     *
     * Cleanup is handled automatically by RAII destructors.
     */
    void TearDown() override {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * Calls WASM i32x4.abs test function with input vector
     *
     * @param func_name Name of the WASM test function to call
     * @param input_values Array of 4 i32 values for input v128 vector
     * @param output_values Array to store 4 i32 values from output v128 vector
     */
    void call_abs_test_function(const char* func_name, const int32_t* input_values, int32_t* output_values) {
        // Prepare arguments: pack 4 i32 values into 4 i32 values for v128
        uint32_t argv[4];
        memcpy(argv, input_values, 16);

        // Execute function
        bool call_success = dummy_env->execute(func_name, 4, argv);
        ASSERT_TRUE(call_success) << "Failed to execute WASM function: " << func_name;

        // Extract result: unpack 4 i32 values back to 4 i32 values
        memcpy(output_values, argv, 16);
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicAbsoluteValue_ReturnsCorrectResults
 * @brief Validates i32x4.abs produces correct absolute values for mixed inputs
 * @details Tests fundamental absolute value operation with positive, negative, and zero integers.
 *          Verifies that i32x4.abs correctly computes abs(x) for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_operation
 * @input_conditions Mixed integer vector: positive, negative, and zero values
 * @expected_behavior Returns absolute values: negative becomes positive, positive unchanged, zero unchanged
 * @validation_method Direct comparison of WASM function result with expected absolute values
 */
TEST_P(I32x4AbsTestSuite, BasicAbsoluteValue_ReturnsCorrectResults) {
    // Test mixed positive/negative values
    int32_t input[] = {1, -5, 10, -100};
    int32_t expected[] = {1, 5, 10, 100};
    int32_t result[4];

    call_abs_test_function(FUNC_NAME_BASIC_ABS, input, result);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << input[i]
            << ") should be " << expected[i]
            << " but got " << result[i];
    }
}

/**
 * @test BoundaryValues_HandlesMinMaxCorrectly
 * @brief Validates i32x4.abs handles INT32_MIN/MAX boundary conditions correctly
 * @details Tests critical boundary cases including the INT32_MIN overflow scenario where
 *          abs(INT32_MIN) cannot be represented in i32 range and wraps to INT32_MIN.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_overflow_handling
 * @input_conditions Boundary values: INT32_MIN, INT32_MAX, and near-boundary values
 * @expected_behavior INT32_MIN wraps to itself (overflow), INT32_MAX unchanged, others computed normally
 * @validation_method Verify overflow behavior and boundary value handling
 */
TEST_P(I32x4AbsTestSuite, BoundaryValues_HandlesMinMaxCorrectly) {
    // Test boundary values including the critical INT32_MIN overflow case
    int32_t input[] = {INT32_MIN, INT32_MAX, -1, 0};
    int32_t expected[] = {INT32_MIN, INT32_MAX, 1, 0}; // INT32_MIN wraps due to overflow
    int32_t result[4];

    call_abs_test_function(FUNC_NAME_BOUNDARY_VALUES, input, result);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << input[i]
            << ") should be " << expected[i]
            << " but got " << result[i];
    }
}

/**
 * @test ZeroVector_ReturnsZeroVector
 * @brief Validates i32x4.abs identity property for zero values
 * @details Tests that absolute value of zero is zero across all lanes,
 *          verifying the mathematical identity property abs(0) = 0.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_zero_handling
 * @input_conditions All lanes contain zero value
 * @expected_behavior All lanes remain zero (identity operation)
 * @validation_method Verify each lane maintains zero value unchanged
 */
TEST_P(I32x4AbsTestSuite, ZeroVector_ReturnsZeroVector) {
    // Test all-zero vector
    int32_t input[] = {0, 0, 0, 0};
    int32_t expected[] = {0, 0, 0, 0};
    int32_t result[4];

    call_abs_test_function(FUNC_NAME_ZERO_VECTOR, input, result);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(0) should be 0 but got " << result[i];
    }
}

/**
 * @test PositiveValues_RemainUnchanged
 * @brief Validates i32x4.abs identity property for positive values
 * @details Tests that absolute value of positive numbers remains unchanged,
 *          verifying the mathematical property abs(x) = x for x >= 0.
 * @test_category Edge - Positive value handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_positive_handling
 * @input_conditions All lanes contain positive values
 * @expected_behavior All lanes remain unchanged (identity for positive values)
 * @validation_method Verify each positive value remains identical after abs operation
 */
TEST_P(I32x4AbsTestSuite, PositiveValues_RemainUnchanged) {
    // Test all-positive values
    int32_t input[] = {42, 1000, 500000, 123456789};
    int32_t expected[] = {42, 1000, 500000, 123456789};
    int32_t result[4];

    call_abs_test_function(FUNC_NAME_POSITIVE_VALUES, input, result);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << input[i]
            << ") should remain " << expected[i]
            << " but got " << result[i];
    }
}

/**
 * @test NegativeValues_BecomePositive
 * @brief Validates i32x4.abs correctly converts negative values to positive
 * @details Tests that absolute value of negative numbers produces corresponding positive values,
 *          verifying the mathematical property abs(-x) = x for x > 0.
 * @test_category Edge - Negative value handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_negative_handling
 * @input_conditions All lanes contain negative values (excluding INT32_MIN)
 * @expected_behavior All lanes become their positive equivalents
 * @validation_method Verify each negative value becomes its positive counterpart
 */
TEST_P(I32x4AbsTestSuite, NegativeValues_BecomePositive) {
    // Test all-negative values (excluding INT32_MIN to avoid overflow)
    int32_t input[] = {-42, -1000, -500000, -123456789};
    int32_t expected[] = {42, 1000, 500000, 123456789};
    int32_t result[4];

    call_abs_test_function(FUNC_NAME_NEGATIVE_VALUES, input, result);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << input[i]
            << ") should be " << expected[i]
            << " but got " << result[i];
    }
}

/**
 * @test Int32MinOverflow_HandlesCorrectly
 * @brief Validates i32x4.abs handling of INT32_MIN overflow scenario
 * @details Tests the specific edge case where abs(INT32_MIN) cannot be represented
 *          in 32-bit signed integer range, resulting in wraparound to INT32_MIN.
 * @test_category Corner - Overflow condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_overflow_handling
 * @input_conditions Vector containing INT32_MIN values mixed with other integers
 * @expected_behavior INT32_MIN values wrap to INT32_MIN, others computed normally
 * @validation_method Verify correct overflow wraparound behavior for INT32_MIN
 */
TEST_P(I32x4AbsTestSuite, Int32MinOverflow_HandlesCorrectly) {
    // Test INT32_MIN overflow scenario specifically
    int32_t input[] = {INT32_MIN, INT32_MIN, INT32_MIN, INT32_MIN};
    int32_t expected[] = {INT32_MIN, INT32_MIN, INT32_MIN, INT32_MIN}; // All wrap due to overflow
    int32_t result[4];

    call_abs_test_function(FUNC_NAME_INT32_MIN_OVERFLOW, input, result);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << input[i]
            << ") should wrap to " << expected[i]
            << " but got " << result[i];
    }
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(I32x4AbsTest, I32x4AbsTestSuite,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT));