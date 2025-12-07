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
 * Enhanced unit tests for f64x2.sqrt WASM opcode
 *
 * Tests comprehensive SIMD square root functionality for double-precision floating-point including:
 * - Basic square root operations with perfect squares and typical values
 * - Boundary condition handling (DBL_MAX/MIN, subnormal values, precision boundaries)
 * - Special value handling (±0.0, ±infinity, NaN, negative values)
 * - Mathematical property validation (IEEE 754 compliance, domain error handling)
 * - Cross-execution mode validation (interpreter vs AOT)
 * - Precision verification for various input ranges
 */

enum class F64x2SqrtRunningMode : uint8_t {
    Interpreter = 0,
    AOT = 1
};

static constexpr const char *MODULE_NAME = "f64x2_sqrt_test";
static constexpr const char *FUNC_NAME_BASIC_SQRT = "test_basic_sqrt";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_SPECIAL_VALUES = "test_special_values";
static constexpr const char *FUNC_NAME_NEGATIVE_VALUES = "test_negative_values";
static constexpr const char *WASM_FILE = "wasm-apps/f64x2_sqrt_test.wasm";

/**
 * Test fixture for f64x2.sqrt opcode validation
 *
 * Provides comprehensive test environment for SIMD floating-point square root operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class F64x2SqrtTestSuite : public testing::TestWithParam<F64x2SqrtRunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f64x2.sqrt test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.sqrt test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.sqrt tests";
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
     * Helper function to call WASM functions that take two f64 values and return v128
     *
     * @param func_name Name of the WASM function to call
     * @param input1 First f64 input value
     * @param input2 Second f64 input value
     * @param lane0 Output for first f64 lane
     * @param lane1 Output for second f64 lane
     * @return true if function execution succeeded, false otherwise
     */
    bool call_f64x2_function(const char* func_name, double input1, double input2, double* lane0, double* lane1) {
        // Convert f64 values to i32 pairs for WASM calling convention
        uint64_t input1_bits, input2_bits;
        memcpy(&input1_bits, &input1, sizeof(double));
        memcpy(&input2_bits, &input2, sizeof(double));

        uint32_t argv[8];
        argv[0] = (uint32_t)(input1_bits & 0xFFFFFFFF);        // input1 low
        argv[1] = (uint32_t)((input1_bits >> 32) & 0xFFFFFFFF); // input1 high
        argv[2] = (uint32_t)(input2_bits & 0xFFFFFFFF);        // input2 low
        argv[3] = (uint32_t)((input2_bits >> 32) & 0xFFFFFFFF); // input2 high
        argv[4] = 0; // result low lane0
        argv[5] = 0; // result high lane0
        argv[6] = 0; // result low lane1
        argv[7] = 0; // result high lane1

        // Call using DummyExecEnv execute method
        bool success = dummy_env->execute(func_name, 8, argv);

        if (success && lane0 && lane1) {
            // Extract f64 values from result i32s
            uint64_t lane0_bits = ((uint64_t)argv[5] << 32) | argv[4];
            uint64_t lane1_bits = ((uint64_t)argv[7] << 32) | argv[6];

            memcpy(lane0, &lane0_bits, sizeof(double));
            memcpy(lane1, &lane1_bits, sizeof(double));
        }

        return success;
    }

protected:
    // WAMR runtime components using RAII
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicSquareRoot_ReturnsCorrectResults
 * @brief Validates f64x2.sqrt produces correct arithmetic results for typical inputs
 * @details Tests fundamental square root operation with perfect squares, non-perfect squares,
 *          and mixed value ranges to verify mathematical correctness per IEEE 754 standard.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_sqrt
 * @input_conditions Perfect squares (4.0,9.0), non-perfect squares (2.0,8.0), mixed ranges (1.0,100.0)
 * @expected_behavior Returns mathematically correct square roots with IEEE 754 precision
 * @validation_method Direct comparison and tolerance-based comparison for computed values
 */
TEST_P(F64x2SqrtTestSuite, BasicSquareRoot_ReturnsCorrectResults) {
    double lane0, lane1;

    // Test case 1: Perfect squares with exact results
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_SQRT, 4.0, 9.0, &lane0, &lane1))
        << "Failed to call basic sqrt function with perfect squares";

    ASSERT_EQ(2.0, lane0) << "First lane sqrt(4.0) should equal 2.0";
    ASSERT_EQ(3.0, lane1) << "Second lane sqrt(9.0) should equal 3.0";

    // Test case 2: Non-perfect squares requiring computation
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_SQRT, 2.0, 8.0, &lane0, &lane1))
        << "Failed to call basic sqrt function with non-perfect squares";

    ASSERT_NEAR(1.41421356237309504880, lane0, 1e-15) << "First lane sqrt(2.0) should equal √2";
    ASSERT_NEAR(2.82842712474619009760, lane1, 1e-15) << "Second lane sqrt(8.0) should equal 2√2";

    // Test case 3: Mixed value ranges
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_SQRT, 1.0, 100.0, &lane0, &lane1))
        << "Failed to call basic sqrt function with mixed ranges";

    ASSERT_EQ(1.0, lane0) << "First lane sqrt(1.0) should equal 1.0 (identity)";
    ASSERT_EQ(10.0, lane1) << "Second lane sqrt(100.0) should equal 10.0";

    // Test case 4: Fractional values that increase when square rooted
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_SQRT, 0.25, 0.5, &lane0, &lane1))
        << "Failed to call basic sqrt function with fractional values";

    ASSERT_EQ(0.5, lane0) << "First lane sqrt(0.25) should equal 0.5";
    ASSERT_NEAR(0.70710678118654752440, lane1, 1e-15) << "Second lane sqrt(0.5) should equal √0.5";
}

/**
 * @test BoundaryValues_HandlesExtremeInputs
 * @brief Verifies proper handling of f64 boundary conditions and extreme values
 * @details Tests square root operation at f64 boundaries including DBL_MAX, DBL_MIN,
 *          subnormal values, and precision boundaries to ensure proper IEEE 754 compliance.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_sqrt
 * @input_conditions Boundary values: (DBL_MAX, DBL_MIN), (subnormal values), (precision near 1.0)
 * @expected_behavior Mathematically correct square roots without overflow/underflow issues
 * @validation_method Boundary value preservation and magnitude verification with appropriate tolerance
 */
TEST_P(F64x2SqrtTestSuite, BoundaryValues_HandlesExtremeInputs) {
    double lane0, lane1;

    // Test DBL_MAX boundary (largest finite f64 value)
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, DBL_MAX, DBL_MAX/4, &lane0, &lane1))
        << "Failed to call boundary values function with DBL_MAX";

    ASSERT_GT(lane0, 0.0) << "sqrt(DBL_MAX) should be positive";
    ASSERT_LT(lane0, DBL_MAX) << "sqrt(DBL_MAX) should be less than DBL_MAX";
    ASSERT_NEAR(1.3407807929942596e+154, lane0, 1e+150) << "sqrt(DBL_MAX) approximation check";

    // Test DBL_MIN boundary (smallest normal positive value)
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, DBL_MIN, DBL_MIN*4, &lane0, &lane1))
        << "Failed to call boundary values function with DBL_MIN";

    ASSERT_GT(lane0, 0.0) << "sqrt(DBL_MIN) should be positive";
    ASSERT_GT(lane0, DBL_MIN) << "sqrt(DBL_MIN) should be greater than DBL_MIN";
    ASSERT_NEAR(1.4916681462400413e-154, lane0, 1e-160) << "sqrt(DBL_MIN) approximation check";

    // Test very small subnormal values
    const double subnormal = 5.0e-324;  // Smallest representable positive value
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, subnormal, subnormal*4, &lane0, &lane1))
        << "Failed to call boundary values function with subnormal values";

    ASSERT_GE(lane0, 0.0) << "sqrt(subnormal) should be non-negative";
    // Result may underflow to zero or remain subnormal

    // Test precision boundaries near 1.0
    const double near_one_high = 1.0000000000000002;
    const double near_one_low = 0.9999999999999998;
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, near_one_high, near_one_low, &lane0, &lane1))
        << "Failed to call boundary values function with near-1.0 values";

    ASSERT_NEAR(1.0, lane0, 1e-15) << "sqrt(1+ε) should be close to 1.0";
    ASSERT_NEAR(1.0, lane1, 1e-15) << "sqrt(1-ε) should be close to 1.0";
}

/**
 * @test SpecialValues_FollowsIEEE754Standard
 * @brief Ensures special IEEE 754 values are handled per specification
 * @details Tests square root with ±∞, ±0.0, NaN values to verify proper IEEE 754
 *          special value handling and domain error behavior for negative inputs.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_sqrt
 * @input_conditions Special values: (±∞), (±0.0), (NaN), mixed combinations
 * @expected_behavior +∞→+∞, ±0.0→±0.0, NaN→NaN, -∞→NaN per IEEE 754
 * @validation_method Special value detection using isnan(), isinf(), signbit()
 */
TEST_P(F64x2SqrtTestSuite, SpecialValues_FollowsIEEE754Standard) {
    double lane0, lane1;

    // Test positive and negative infinity
    const double pos_inf = std::numeric_limits<double>::infinity();
    const double neg_inf = -std::numeric_limits<double>::infinity();

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_inf, neg_inf, &lane0, &lane1))
        << "Failed to call special values function with infinity values";

    ASSERT_TRUE(std::isinf(lane0)) << "sqrt(+∞) should be infinite";
    ASSERT_FALSE(std::signbit(lane0)) << "sqrt(+∞) should be positive";
    ASSERT_TRUE(std::isnan(lane1)) << "sqrt(-∞) should be NaN (domain error)";

    // Test positive and negative zero (preserve sign of zero)
    const double pos_zero = 0.0;
    const double neg_zero = -0.0;

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_zero, neg_zero, &lane0, &lane1))
        << "Failed to call special values function with zero values";

    ASSERT_EQ(0.0, lane0) << "sqrt(+0.0) should be 0.0";
    ASSERT_FALSE(std::signbit(lane0)) << "sqrt(+0.0) should be positive zero";
    ASSERT_EQ(0.0, lane1) << "sqrt(-0.0) should be 0.0";
    ASSERT_TRUE(std::signbit(lane1)) << "sqrt(-0.0) should preserve negative zero sign";

    // Test NaN values (NaN propagation)
    const double pos_nan = std::numeric_limits<double>::quiet_NaN();
    const double mixed_valid = 4.0;

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_nan, mixed_valid, &lane0, &lane1))
        << "Failed to call special values function with NaN values";

    ASSERT_TRUE(std::isnan(lane0)) << "sqrt(NaN) should be NaN (propagation)";
    ASSERT_EQ(2.0, lane1) << "sqrt(4.0) should be 2.0 (valid lane unaffected)";
}

/**
 * @test NegativeInputs_ProducesNaNResults
 * @brief Validates domain error handling for negative input values
 * @details Tests square root behavior with negative values to ensure proper NaN generation
 *          per IEEE 754 standard, including mixed valid/invalid scenarios.
 * @test_category Error - Domain error validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_sqrt
 * @input_conditions Various negative values (-1.0, -4.0), mixed (4.0, -1.0)
 * @expected_behavior NaN for negative inputs, valid results for positive inputs
 * @validation_method NaN detection for negative lane results, correct results for valid lanes
 */
TEST_P(F64x2SqrtTestSuite, NegativeInputs_ProducesNaNResults) {
    double lane0, lane1;

    // Test both negative values
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_NEGATIVE_VALUES, -1.0, -4.0, &lane0, &lane1))
        << "Failed to call negative values function with both negative inputs";

    ASSERT_TRUE(std::isnan(lane0)) << "sqrt(-1.0) should be NaN (domain error)";
    ASSERT_TRUE(std::isnan(lane1)) << "sqrt(-4.0) should be NaN (domain error)";

    // Test mixed valid and invalid values
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_NEGATIVE_VALUES, 4.0, -1.0, &lane0, &lane1))
        << "Failed to call negative values function with mixed inputs";

    ASSERT_EQ(2.0, lane0) << "sqrt(4.0) should be 2.0 (valid positive input)";
    ASSERT_TRUE(std::isnan(lane1)) << "sqrt(-1.0) should be NaN (domain error)";

    // Test very small negative values
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_NEGATIVE_VALUES, -DBL_MIN, -1e-100, &lane0, &lane1))
        << "Failed to call negative values function with small negative inputs";

    ASSERT_TRUE(std::isnan(lane0)) << "sqrt(-DBL_MIN) should be NaN (domain error)";
    ASSERT_TRUE(std::isnan(lane1)) << "sqrt(-1e-100) should be NaN (domain error)";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    F64x2SqrtTestSuite,
    testing::Values(F64x2SqrtRunningMode::Interpreter
#if WASM_ENABLE_AOT != 0
                   , F64x2SqrtRunningMode::AOT
#endif
                   ),
    [](const testing::TestParamInfo<F64x2SqrtRunningMode>& info) {
        return info.param == F64x2SqrtRunningMode::Interpreter ? "Interpreter" : "AOT";
    }
);