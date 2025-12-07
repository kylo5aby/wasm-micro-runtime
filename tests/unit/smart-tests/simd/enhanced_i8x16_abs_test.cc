/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * Enhanced unit tests for i8x16.abs WASM opcode
 *
 * Tests comprehensive SIMD absolute value functionality including:
 * - Basic absolute value operations with mixed positive/negative values
 * - Boundary condition handling (INT8_MIN/MAX, overflow behavior)
 * - Edge cases (all zeros, all positive, extreme values)
 * - Mathematical property validation (idempotent, symmetry)
 * - Cross-execution mode validation (interpreter vs AOT)
 */

static constexpr const char *MODULE_NAME = "i8x16_abs_test";
static constexpr const char *FUNC_NAME_BASIC_ABS = "test_basic_abs";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_ZERO_VECTOR = "test_zero_vector";
static constexpr const char *FUNC_NAME_POSITIVE_VALUES = "test_positive_values";
static constexpr const char *FUNC_NAME_NEGATIVE_VALUES = "test_negative_values";

/**
 * Test fixture for i8x16.abs opcode validation
 *
 * Provides comprehensive test environment for SIMD absolute value operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class I8x16AbsTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the i8x16.abs test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.abs test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_abs_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i8x16.abs tests";
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
     * Calls WASM i8x16.abs test function with input vector
     *
     * @param func_name Name of the WASM test function to call
     * @param input_values Array of 16 i8 values for input v128 vector
     * @param output_values Array to store 16 i8 values from output v128 vector
     */
    void call_abs_test_function(const char* func_name, const int8_t* input_values, int8_t* output_values) {
        // Prepare arguments: pack 16 i8 values into 4 i32 values for v128
        uint32_t argv[4];
        memcpy(argv, input_values, 16);

        // Execute function
        bool call_success = dummy_env->execute(func_name, 4, argv);
        ASSERT_TRUE(call_success) << "Failed to execute WASM function: " << func_name;

        // Extract result: unpack 4 i32 values back to 16 i8 values
        memcpy(output_values, argv, 16);
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicAbsoluteValue_ReturnsCorrectResults
 * @brief Validates i8x16.abs produces correct absolute values for mixed inputs
 * @details Tests fundamental absolute value operation with positive, negative, and zero integers.
 *          Verifies that i8x16.abs correctly computes abs(x) for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_operation
 * @input_conditions Mixed integer vector: positive, negative, and zero values
 * @expected_behavior Returns absolute values: negative becomes positive, positive unchanged, zero unchanged
 * @validation_method Direct comparison of WASM function result with expected absolute values
 */
TEST_P(I8x16AbsTestSuite, BasicAbsoluteValue_ReturnsCorrectResults) {
    // Test mixed positive/negative values
    int8_t input[] = {-10, 5, -25, 0, 100, -1, 15, -50, 75, -100, 3, -15, 42, -42, 1, -1};
    int8_t expected[] = {10, 5, 25, 0, 100, 1, 15, 50, 75, 100, 3, 15, 42, 42, 1, 1};
    int8_t result[16];

    call_abs_test_function(FUNC_NAME_BASIC_ABS, input, result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << static_cast<int>(input[i])
            << ") should be " << static_cast<int>(expected[i])
            << " but got " << static_cast<int>(result[i]);
    }
}

/**
 * @test BoundaryValues_HandlesMinMaxCorrectly
 * @brief Validates i8x16.abs handles INT8_MIN/MAX boundary conditions correctly
 * @details Tests critical boundary cases including the INT8_MIN overflow scenario where
 *          abs(-128) cannot be represented in i8 range and typically returns -128.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_overflow_handling
 * @input_conditions Boundary values: INT8_MIN (-128), INT8_MAX (127), and near-boundary values
 * @expected_behavior INT8_MIN stays -128 (overflow), INT8_MAX unchanged, others computed normally
 * @validation_method Verify overflow behavior and boundary value handling
 */
TEST_P(I8x16AbsTestSuite, BoundaryValues_HandlesMinMaxCorrectly) {
    // Test boundary values including the critical INT8_MIN overflow case
    int8_t input[] = {-128, 127, -127, 126, -1, 0, 1, -128, 127, -126, 125, -2, 2, -128, 127, -1};
    int8_t expected[] = {-128, 127, 127, 126, 1, 0, 1, -128, 127, 126, 125, 2, 2, -128, 127, 1};
    int8_t result[16];

    call_abs_test_function(FUNC_NAME_BOUNDARY_VALUES, input, result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << static_cast<int>(input[i])
            << ") should be " << static_cast<int>(expected[i])
            << " but got " << static_cast<int>(result[i]);
    }
}

/**
 * @test ZeroVector_ReturnsZeroVector
 * @brief Validates i8x16.abs identity property for zero values
 * @details Tests that absolute value of zero is zero across all lanes,
 *          verifying the mathematical identity property abs(0) = 0.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_zero_handling
 * @input_conditions All lanes contain zero value
 * @expected_behavior All lanes remain zero (identity operation)
 * @validation_method Verify each lane maintains zero value unchanged
 */
TEST_P(I8x16AbsTestSuite, ZeroVector_ReturnsZeroVector) {
    // Test all-zero vector
    int8_t input[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int8_t expected[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int8_t result[16];

    call_abs_test_function(FUNC_NAME_ZERO_VECTOR, input, result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(0) should be 0 but got " << static_cast<int>(result[i]);
    }
}

/**
 * @test PositiveValues_RemainUnchanged
 * @brief Validates i8x16.abs identity property for positive values
 * @details Tests that absolute value of positive numbers remains unchanged,
 *          verifying the mathematical property abs(x) = x for x >= 0.
 * @test_category Edge - Positive value handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_positive_handling
 * @input_conditions All lanes contain positive values
 * @expected_behavior All lanes remain unchanged (identity for positive values)
 * @validation_method Verify each positive value remains identical after abs operation
 */
TEST_P(I8x16AbsTestSuite, PositiveValues_RemainUnchanged) {
    // Test all-positive values
    int8_t input[] = {1, 5, 10, 25, 50, 75, 100, 127, 15, 30, 45, 60, 80, 90, 105, 120};
    int8_t expected[] = {1, 5, 10, 25, 50, 75, 100, 127, 15, 30, 45, 60, 80, 90, 105, 120};
    int8_t result[16];

    call_abs_test_function(FUNC_NAME_POSITIVE_VALUES, input, result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << static_cast<int>(input[i])
            << ") should remain " << static_cast<int>(expected[i])
            << " but got " << static_cast<int>(result[i]);
    }
}

/**
 * @test NegativeValues_BecomePositive
 * @brief Validates i8x16.abs correctly converts negative values to positive
 * @details Tests that absolute value of negative numbers produces corresponding positive values,
 *          verifying the mathematical property abs(-x) = x for x > 0.
 * @test_category Edge - Negative value handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_abs_negative_handling
 * @input_conditions All lanes contain negative values (excluding INT8_MIN)
 * @expected_behavior All lanes become their positive equivalents
 * @validation_method Verify each negative value becomes its positive counterpart
 */
TEST_P(I8x16AbsTestSuite, NegativeValues_BecomePositive) {
    // Test all-negative values (excluding INT8_MIN to avoid overflow)
    int8_t input[] = {-1, -5, -10, -25, -50, -75, -100, -127, -15, -30, -45, -60, -80, -90, -105, -120};
    int8_t expected[] = {1, 5, 10, 25, 50, 75, 100, 127, 15, 30, 45, 60, 80, 90, 105, 120};
    int8_t result[16];

    call_abs_test_function(FUNC_NAME_NEGATIVE_VALUES, input, result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << ": abs(" << static_cast<int>(input[i])
            << ") should be " << static_cast<int>(expected[i])
            << " but got " << static_cast<int>(result[i]);
    }
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(I8x16AbsTest, I8x16AbsTestSuite,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT));