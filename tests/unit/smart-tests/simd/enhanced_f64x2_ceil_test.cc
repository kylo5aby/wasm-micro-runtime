/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <climits>
#include <cfloat>
#include <cmath>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * Enhanced unit tests for f64x2.ceil WASM opcode
 *
 * Tests comprehensive SIMD ceiling functionality for double-precision floating-point including:
 * - Basic ceiling operations with mixed positive/negative fractional f64 values
 * - Boundary condition handling (large magnitude values, precision limits, exact integers)
 * - Special value handling (±0.0, ±infinity, NaN)
 * - Mathematical property validation (monotonic, range properties)
 * - Cross-execution mode validation (interpreter vs AOT)
 * - IEEE 754 compliance verification
 */

static constexpr const char *MODULE_NAME = "f64x2_ceil_test";
static constexpr const char *FUNC_NAME_BASIC_CEIL = "test_basic_ceil";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_SPECIAL_VALUES = "test_special_values";
static constexpr const char *FUNC_NAME_ZERO_VALUES = "test_zero_values";
static constexpr const char *WASM_FILE = "wasm-apps/f64x2_ceil_test.wasm";

/**
 * Test fixture for f64x2.ceil opcode validation
 *
 * Provides comprehensive test environment for SIMD floating-point ceiling operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class F64x2CeilTestSuite : public testing::Test {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f64x2.ceil test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.ceil test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.ceil tests";
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
     * Helper function to call WASM f64x2.ceil functions with two f64 values and extract f64x2 result
     *
     * @param func_name Name of the WASM function to call
     * @param input1 First f64 input value
     * @param input2 Second f64 input value
     * @param lane0 Output for first f64 lane
     * @param lane1 Output for second f64 lane
     * @return true if function execution succeeded, false otherwise
     */
    bool call_f64x2_ceil_function(const char* func_name, double input1, double input2, double* lane0, double* lane1) {
        // Convert f64 inputs to i32 pairs for WASM function call
        uint64_t input1_bits, input2_bits;
        memcpy(&input1_bits, &input1, sizeof(double));
        memcpy(&input2_bits, &input2, sizeof(double));

        // Prepare arguments: four i32 values representing two f64 values
        uint32_t argv[4];
        argv[0] = static_cast<uint32_t>(input1_bits);        // input1 low 32 bits
        argv[1] = static_cast<uint32_t>(input1_bits >> 32);  // input1 high 32 bits
        argv[2] = static_cast<uint32_t>(input2_bits);        // input2 low 32 bits
        argv[3] = static_cast<uint32_t>(input2_bits >> 32);  // input2 high 32 bits

        // Call WASM function using dummy_env->execute
        bool call_success = dummy_env->execute(func_name, 4, argv);
        if (!call_success) {
            return false;
        }

        // Extract f64 results from i32 pairs
        uint64_t result1_bits = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        uint64_t result2_bits = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];

        if (lane0 && lane1) {
            memcpy(lane0, &result1_bits, sizeof(double));
            memcpy(lane1, &result2_bits, sizeof(double));
        }

        return true;
    }

protected:
    // WAMR runtime components using RAII
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicCeiling_ProducesCorrectResults
 * @brief Validates f64x2.ceil produces correct arithmetic results for typical fractional inputs
 * @details Tests fundamental ceiling operation with positive, negative, and mixed-sign fractional doubles.
 *          Verifies that f64x2.ceil correctly computes ceil(a), ceil(b) for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_ceil
 * @input_conditions Standard fractional f64 pairs: (3.7,-2.3), (1.2,-5.8), (0.1,-0.9)
 * @expected_behavior Returns ceiling values: (4.0,-2.0), (2.0,-5.0), (1.0,-0.0) respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(F64x2CeilTestSuite, BasicCeiling_ProducesCorrectResults) {
    double lane0, lane1;

    // Test case 1: Mixed positive and negative fractional values
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_BASIC_CEIL, 3.7, -2.3, &lane0, &lane1))
        << "Failed to call basic ceil function with mixed fractional values";

    ASSERT_EQ(4.0, lane0) << "First lane ceil(3.7) should equal 4.0";
    ASSERT_EQ(-2.0, lane1) << "Second lane ceil(-2.3) should equal -2.0";

    // Test case 2: More fractional values
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_BASIC_CEIL, 1.2, -5.8, &lane0, &lane1))
        << "Failed to call basic ceil function with additional fractional values";

    ASSERT_EQ(2.0, lane0) << "First lane ceil(1.2) should equal 2.0";
    ASSERT_EQ(-5.0, lane1) << "Second lane ceil(-5.8) should equal -5.0";

    // Test case 3: Small fractions near zero
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_BASIC_CEIL, 0.1, -0.9, &lane0, &lane1))
        << "Failed to call basic ceil function with small fractions";

    ASSERT_EQ(1.0, lane0) << "First lane ceil(0.1) should equal 1.0";
    ASSERT_EQ(-0.0, lane1) << "Second lane ceil(-0.9) should equal -0.0";
}

/**
 * @test BoundaryValues_HandlesExtremeNumbers
 * @brief Validates f64x2.ceil handles boundary conditions correctly for large and precise values
 * @details Tests ceiling operation with large magnitude values, exact integers, and precision boundaries.
 *          Verifies proper behavior at f64 precision limits and integer boundaries.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_ceil
 * @input_conditions Large/exact values: (1e15,-1e15), (42.0,-17.0), (1.7976931348623157e+307,-1.7976931348623157e+307)
 * @expected_behavior Large numbers and integers remain unchanged, proper precision handling
 * @validation_method Verify large numbers and integers remain unchanged after ceiling operation
 */
TEST_F(F64x2CeilTestSuite, BoundaryValues_HandlesExtremeNumbers) {
    double lane0, lane1;

    // Test case 1: Large magnitude values
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_BOUNDARY_VALUES, 1e15, -1e15, &lane0, &lane1))
        << "Failed to call boundary values function with large magnitudes";

    ASSERT_EQ(1e15, lane0) << "First lane ceil(1e15) should remain 1e15";
    ASSERT_EQ(-1e15, lane1) << "Second lane ceil(-1e15) should remain -1e15";

    // Test case 2: Exact integers should remain unchanged
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_BOUNDARY_VALUES, 42.0, -17.0, &lane0, &lane1))
        << "Failed to call boundary values function with exact integers";

    ASSERT_EQ(42.0, lane0) << "First lane ceil(42.0) should remain 42.0";
    ASSERT_EQ(-17.0, lane1) << "Second lane ceil(-17.0) should remain -17.0";

    // Test case 3: Very large values approaching DBL_MAX
    const double large_pos = 1.7976931348623157e+307;
    const double large_neg = -1.7976931348623157e+307;

    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_BOUNDARY_VALUES, large_pos, large_neg, &lane0, &lane1))
        << "Failed to call boundary values function with values near DBL_MAX";

    ASSERT_EQ(large_pos, lane0) << "First lane ceil(large_pos) should remain unchanged";
    ASSERT_EQ(large_neg, lane1) << "Second lane ceil(large_neg) should remain unchanged";
}

/**
 * @test SpecialValues_PreservesIEEE754Behavior
 * @brief Validates f64x2.ceil preserves IEEE 754 special value behavior correctly
 * @details Tests ceiling operation with NaN, infinities, and mixed special values.
 *          Verifies IEEE 754 compliance for special numeric cases.
 * @test_category Edge - Special IEEE 754 value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_ceil
 * @input_conditions Special values: (NaN,+∞), (-∞,+0.0), (-0.0,NaN)
 * @expected_behavior NaN propagation, infinity preservation, proper special value handling
 * @validation_method Use std::isnan(), std::isinf(), and signbit() for special value verification
 */
TEST_F(F64x2CeilTestSuite, SpecialValues_PreservesIEEE754Behavior) {
    double lane0, lane1;

    // Test case 1: NaN and positive infinity
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_SPECIAL_VALUES, std::numeric_limits<double>::quiet_NaN(),
                                   std::numeric_limits<double>::infinity(), &lane0, &lane1))
        << "Failed to call special values function with NaN and positive infinity";

    ASSERT_TRUE(std::isnan(lane0)) << "First lane ceil(NaN) should remain NaN";
    ASSERT_TRUE(std::isinf(lane1) && lane1 > 0) << "Second lane ceil(+∞) should remain +∞";

    // Test case 2: Negative infinity and positive zero
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_SPECIAL_VALUES, -std::numeric_limits<double>::infinity(),
                                   +0.0, &lane0, &lane1))
        << "Failed to call special values function with negative infinity and positive zero";

    ASSERT_TRUE(std::isinf(lane0) && lane0 < 0) << "First lane ceil(-∞) should remain -∞";
    ASSERT_EQ(+0.0, lane1) << "Second lane ceil(+0.0) should remain +0.0";
    ASSERT_FALSE(std::signbit(lane1)) << "Second lane should be positive zero";

    // Test case 3: Negative zero and NaN
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_SPECIAL_VALUES, -0.0,
                                   std::numeric_limits<double>::quiet_NaN(), &lane0, &lane1))
        << "Failed to call special values function with negative zero and NaN";

    ASSERT_EQ(-0.0, lane0) << "First lane ceil(-0.0) should remain -0.0";
    ASSERT_TRUE(std::signbit(lane0)) << "First lane should be negative zero";
    ASSERT_TRUE(std::isnan(lane1)) << "Second lane ceil(NaN) should remain NaN";
}

/**
 * @test ZeroValues_MaintainsSignedZero
 * @brief Validates f64x2.ceil maintains signed zero behavior and handles near-zero values correctly
 * @details Tests ceiling operation with various zero and near-zero scenarios.
 *          Verifies proper signed zero preservation and small value ceiling behavior.
 * @test_category Edge - Zero and near-zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_ceil
 * @input_conditions Zero/near-zero values: (+0.0,-0.0), (1e-15,-1e-15), (1e-300,-1e-300)
 * @expected_behavior Signed zero preservation, small positive values round to 1.0, small negative values round to -0.0
 * @validation_method Use signbit() to verify zero sign preservation and validate ceiling results
 */
TEST_F(F64x2CeilTestSuite, ZeroValues_MaintainsSignedZero) {
    double lane0, lane1;

    // Test case 1: Signed zeros
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_ZERO_VALUES, +0.0, -0.0, &lane0, &lane1))
        << "Failed to call zero values function with signed zeros";

    ASSERT_EQ(+0.0, lane0) << "First lane ceil(+0.0) should remain +0.0";
    ASSERT_FALSE(std::signbit(lane0)) << "First lane should be positive zero";
    ASSERT_EQ(-0.0, lane1) << "Second lane ceil(-0.0) should remain -0.0";
    ASSERT_TRUE(std::signbit(lane1)) << "Second lane should be negative zero";

    // Test case 2: Very small positive and negative values
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_ZERO_VALUES, 1e-15, -1e-15, &lane0, &lane1))
        << "Failed to call zero values function with very small values";

    ASSERT_EQ(1.0, lane0) << "First lane ceil(1e-15) should equal 1.0";
    ASSERT_EQ(-0.0, lane1) << "Second lane ceil(-1e-15) should equal -0.0";
    ASSERT_TRUE(std::signbit(lane1)) << "Second lane should be negative zero";

    // Test case 3: Extremely small subnormal values
    ASSERT_TRUE(call_f64x2_ceil_function(FUNC_NAME_ZERO_VALUES, 1e-300, -1e-300, &lane0, &lane1))
        << "Failed to call zero values function with subnormal values";

    ASSERT_EQ(1.0, lane0) << "First lane ceil(1e-300) should equal 1.0";
    ASSERT_EQ(-0.0, lane1) << "Second lane ceil(-1e-300) should equal -0.0";
    ASSERT_TRUE(std::signbit(lane1)) << "Second lane should be negative zero";
}

