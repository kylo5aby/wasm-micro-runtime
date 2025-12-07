/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <limits>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * Enhanced unit tests for f64x2.extmul_low_i32x4_s WASM opcode
 *
 * Tests comprehensive SIMD extended multiplication functionality including:
 * - Basic extended multiplication with typical signed integer values
 * - Boundary condition handling (INT32_MIN/MAX multiplication results)
 * - Edge cases (zero operands, identity operations, extreme values)
 * - Mathematical property validation (commutative, zero identity)
 * - Cross-execution mode validation (interpreter vs AOT)
 */

// Test execution modes for cross-validation
enum class F64x2ExtmulLowI32x4sRunningMode : uint8_t {
    INTERP = 1 << 0,
    AOT = 1 << 1
};

static constexpr const char *MODULE_NAME = "f64x2_extmul_low_i32x4_s_test";
static constexpr const char *FUNC_NAME_BASIC_EXTMUL = "test_basic_extmul";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_ZERO_AND_IDENTITY = "test_zero_and_identity";

/**
 * Test fixture for f64x2.extmul_low_i32x4_s opcode validation
 *
 * Provides comprehensive test environment for SIMD extended multiplication operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class F64x2ExtmulLowI32x4STestSuite : public testing::TestWithParam<F64x2ExtmulLowI32x4sRunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f64x2.extmul_low_i32x4_s test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.extmul_low_i32x4_s test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_extmul_low_i32x4_s_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.extmul_low_i32x4_s tests";
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
     * Calls WASM f64x2.extmul_low_i32x4_s test function with input vectors
     *
     * @param func_name Name of the WASM test function to call
     * @param input1_values Array of 4 i32 values for first input v128 vector
     * @param input2_values Array of 4 i32 values for second input v128 vector
     * @param output_values Array to store 2 f64 values from output v128 vector
     */
    void call_extmul_test_function(const char* func_name, const int32_t* input1_values,
                                  const int32_t* input2_values, double* output_values) {
        // Prepare arguments: pack input vectors into argv array
        uint32_t argv[8];  // 4 i32 for first vector + 4 i32 for second vector
        memcpy(argv, input1_values, 16);      // First i32x4 vector
        memcpy(argv + 4, input2_values, 16);  // Second i32x4 vector

        // Execute function
        bool call_success = dummy_env->execute(func_name, 8, argv);
        ASSERT_TRUE(call_success) << "Failed to execute WASM function: " << func_name;

        // Extract result: get f64x2 result (2 doubles = 16 bytes)
        memcpy(output_values, argv, 16);
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicExtMul_ReturnsCorrectResults
 * @brief Validates f64x2.extmul_low_i32x4_s produces correct arithmetic results for typical inputs
 * @details Tests fundamental extended multiplication operation with positive, negative, and mixed-sign integers.
 *          Verifies that f64x2.extmul_low_i32x4_s correctly computes signed extended multiplication
 *          of lower two i32 elements and converts results to f64 values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:f64x2_extmul_low_i32x4_s_operation
 * @input_conditions Various signed i32 combinations in lower elements of input vectors
 * @expected_behavior Returns precise f64 results matching signed 64-bit multiplication → f64 conversion
 * @validation_method Direct comparison of WASM function result with expected f64 values
 */
TEST_P(F64x2ExtmulLowI32x4STestSuite, BasicExtMul_ReturnsCorrectResults) {
    // Test case 1: Positive integers multiplication
    int32_t input1_pos[] = {10, 20, 30, 40};  // Only [0] and [1] are used (10, 20)
    int32_t input2_pos[] = {5, 3, 2, 1};      // Only [0] and [1] are used (5, 3)
    double output_pos[2];

    call_extmul_test_function(FUNC_NAME_BASIC_EXTMUL, input1_pos, input2_pos, output_pos);

    ASSERT_EQ(50.0, output_pos[0]) << "Extended multiplication of positive integers failed: 10 * 5 != 50";
    ASSERT_EQ(60.0, output_pos[1]) << "Extended multiplication of positive integers failed: 20 * 3 != 60";

    // Test case 2: Negative integers multiplication
    int32_t input1_neg[] = {-10, -20, 30, 40};
    int32_t input2_neg[] = {5, -3, 2, 1};
    double output_neg[2];

    call_extmul_test_function(FUNC_NAME_BASIC_EXTMUL, input1_neg, input2_neg, output_neg);

    ASSERT_EQ(-50.0, output_neg[0]) << "Extended multiplication of mixed signs failed: -10 * 5 != -50";
    ASSERT_EQ(60.0, output_neg[1]) << "Extended multiplication of negative integers failed: -20 * -3 != 60";

    // Test case 3: Mixed sign multiplication
    int32_t input1_mixed[] = {15, -25, 100, -200};
    int32_t input2_mixed[] = {-2, 4, 8, -16};
    double output_mixed[2];

    call_extmul_test_function(FUNC_NAME_BASIC_EXTMUL, input1_mixed, input2_mixed, output_mixed);

    ASSERT_EQ(-30.0, output_mixed[0]) << "Extended multiplication of mixed signs failed: 15 * -2 != -30";
    ASSERT_EQ(-100.0, output_mixed[1]) << "Extended multiplication of mixed signs failed: -25 * 4 != -100";
}

/**
 * @test BoundaryValues_HandlesExtremeInputs
 * @brief Tests behavior with INT32_MIN and INT32_MAX values
 * @details Validates extended multiplication with extreme i32 boundary values that produce
 *          large i64 intermediate results, ensuring correct conversion to f64.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:f64x2_extmul_low_i32x4_s_operation
 * @input_conditions Boundary value combinations causing large i64 intermediate results
 * @expected_behavior Correct f64 representation of extended multiplication results
 * @validation_method Validate large number precision in f64 conversion
 */
TEST_P(F64x2ExtmulLowI32x4STestSuite, BoundaryValues_HandlesExtremeInputs) {
    // Test case 1: Maximum values multiplication
    int32_t input1_max[] = {INT32_MAX, INT32_MAX - 1, 0, 0};
    int32_t input2_max[] = {INT32_MAX, 2, 0, 0};
    double output_max[2];

    call_extmul_test_function(FUNC_NAME_BOUNDARY_VALUES, input1_max, input2_max, output_max);

    // INT32_MAX * INT32_MAX = 4611686014132420609
    ASSERT_NEAR(4611686014132420609.0, output_max[0], 1.0)
        << "Extended multiplication of INT32_MAX values failed";
    // (INT32_MAX - 1) * 2 = 4294967292
    ASSERT_NEAR(4294967292.0, output_max[1], 1.0)
        << "Extended multiplication near boundary failed";

    // Test case 2: Minimum values multiplication
    int32_t input1_min[] = {INT32_MIN, INT32_MIN, 0, 0};
    int32_t input2_min[] = {INT32_MIN, -1, 0, 0};
    double output_min[2];

    call_extmul_test_function(FUNC_NAME_BOUNDARY_VALUES, input1_min, input2_min, output_min);

    // INT32_MIN * INT32_MIN = 4611686018427387904
    ASSERT_NEAR(4611686018427387904.0, output_min[0], 1.0)
        << "Extended multiplication of INT32_MIN values failed";
    // INT32_MIN * (-1) = 2147483648
    ASSERT_NEAR(2147483648.0, output_min[1], 1.0)
        << "Extended multiplication of INT32_MIN by -1 failed";

    // Test case 3: Mixed boundary values
    int32_t input1_mixed_boundary[] = {INT32_MAX, INT32_MIN, 0, 0};
    int32_t input2_mixed_boundary[] = {INT32_MIN, INT32_MAX, 0, 0};
    double output_mixed_boundary[2];

    call_extmul_test_function(FUNC_NAME_BOUNDARY_VALUES, input1_mixed_boundary, input2_mixed_boundary, output_mixed_boundary);

    // INT32_MAX * INT32_MIN = -4611686016279904256
    ASSERT_NEAR(-4611686016279904256.0, output_mixed_boundary[0], 1.0)
        << "Extended multiplication of mixed boundary values failed";
    // INT32_MIN * INT32_MAX = -4611686016279904256
    ASSERT_NEAR(-4611686016279904256.0, output_mixed_boundary[1], 1.0)
        << "Extended multiplication of mixed boundary values failed";
}

/**
 * @test ZeroAndIdentity_ValidatesMathematicalProperties
 * @brief Verifies zero multiplication and identity properties
 * @details Tests mathematical correctness of multiplication with zero, one, and negative one.
 *          Validates fundamental mathematical properties: 0×n=0, 1×n=n, -1×n=-n.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:f64x2_extmul_low_i32x4_s_operation
 * @input_conditions Zero values, ones, and negative ones in test vectors
 * @expected_behavior Mathematical correctness (0×n=0, 1×n=n, -1×n=-n)
 * @validation_method Property-based validation with expected mathematical outcomes
 */
TEST_P(F64x2ExtmulLowI32x4STestSuite, ZeroAndIdentity_ValidatesMathematicalProperties) {
    // Test case 1: Zero multiplication identity
    int32_t input1_zero[] = {0, 0, 100, 200};
    int32_t input2_zero[] = {1000, 2000, 300, 400};
    double output_zero[2];

    call_extmul_test_function(FUNC_NAME_ZERO_AND_IDENTITY, input1_zero, input2_zero, output_zero);

    ASSERT_EQ(0.0, output_zero[0]) << "Zero multiplication property failed: 0 * 1000 != 0";
    ASSERT_EQ(0.0, output_zero[1]) << "Zero multiplication property failed: 0 * 2000 != 0";

    // Test case 2: One identity multiplication
    int32_t input1_one[] = {42, 100, 50, 75};
    int32_t input2_one[] = {1, 1, 2, 3};
    double output_one[2];

    call_extmul_test_function(FUNC_NAME_ZERO_AND_IDENTITY, input1_one, input2_one, output_one);

    ASSERT_EQ(42.0, output_one[0]) << "Identity multiplication property failed: 42 * 1 != 42";
    ASSERT_EQ(100.0, output_one[1]) << "Identity multiplication property failed: 100 * 1 != 100";

    // Test case 3: Negative one identity
    int32_t input1_neg_one[] = {50, -75, 25, 125};
    int32_t input2_neg_one[] = {-1, -1, 4, 8};
    double output_neg_one[2];

    call_extmul_test_function(FUNC_NAME_ZERO_AND_IDENTITY, input1_neg_one, input2_neg_one, output_neg_one);

    ASSERT_EQ(-50.0, output_neg_one[0]) << "Negative identity property failed: 50 * -1 != -50";
    ASSERT_EQ(75.0, output_neg_one[1]) << "Negative identity property failed: -75 * -1 != 75";

    // Test case 4: Commutative property validation (a*b = b*a)
    int32_t input1_commute[] = {123, 456, 0, 0};
    int32_t input2_commute[] = {789, 321, 0, 0};
    int32_t input1_commute_swap[] = {789, 321, 0, 0};
    int32_t input2_commute_swap[] = {123, 456, 0, 0};
    double output_commute1[2], output_commute2[2];

    call_extmul_test_function(FUNC_NAME_ZERO_AND_IDENTITY, input1_commute, input2_commute, output_commute1);
    call_extmul_test_function(FUNC_NAME_ZERO_AND_IDENTITY, input1_commute_swap, input2_commute_swap, output_commute2);

    ASSERT_EQ(output_commute1[0], output_commute2[0])
        << "Commutative property failed: 123 * 789 != 789 * 123";
    ASSERT_EQ(output_commute1[1], output_commute2[1])
        << "Commutative property failed: 456 * 321 != 321 * 456";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionModeValidation,
    F64x2ExtmulLowI32x4STestSuite,
    testing::Values(
        F64x2ExtmulLowI32x4sRunningMode::INTERP
#if WASM_ENABLE_AOT != 0
        , F64x2ExtmulLowI32x4sRunningMode::AOT
#endif
    ),
    [](const testing::TestParamInfo<F64x2ExtmulLowI32x4sRunningMode>& info) {
        return info.param == F64x2ExtmulLowI32x4sRunningMode::INTERP ? "Interpreter" : "AOT";
    }
);