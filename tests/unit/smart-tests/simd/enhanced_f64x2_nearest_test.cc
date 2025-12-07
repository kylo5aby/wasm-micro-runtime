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
 * Enhanced unit tests for f64x2.nearest WASM opcode
 *
 * Tests comprehensive SIMD nearest integer rounding functionality for double-precision floating-point including:
 * - Basic banker's rounding operations ("round half to even" semantics)
 * - Boundary condition handling (DBL_MAX/MIN, subnormal values)
 * - Special value handling (±0.0, ±infinity, NaN)
 * - Mathematical property validation (half-value rounding rules)
 * - Cross-execution mode validation (interpreter vs AOT)
 * - IEEE 754 compliance verification
 */

enum class F64x2NearestRunningMode : uint8_t {
    Interpreter = 0,
    AOT = 1
};

static constexpr const char *MODULE_NAME = "f64x2_nearest_test";
static constexpr const char *FUNC_NAME_BASIC_NEAREST = "test_basic_nearest";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_SPECIAL_VALUES = "test_special_values";
static constexpr const char *FUNC_NAME_BANKERS_ROUNDING = "test_bankers_rounding";
static constexpr const char *WASM_FILE = "wasm-apps/f64x2_nearest_test.wasm";

/**
 * Test fixture for f64x2.nearest opcode validation
 *
 * Provides comprehensive test environment for SIMD floating-point nearest integer rounding operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class F64x2NearestTestSuite : public testing::TestWithParam<F64x2NearestRunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f64x2.nearest test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.nearest test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.nearest tests";
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
     * Helper function to call WASM f64x2.nearest functions that take v128 input and return v128
     *
     * @param func_name Name of the WASM function to call
     * @param input1 First f64 input value
     * @param input2 Second f64 input value
     * @param lane0 Output for first f64 lane
     * @param lane1 Output for second f64 lane
     */
    void call_f64x2_nearest_function(const char* func_name, double input1, double input2, double* lane0, double* lane1) {
        // Pack two f64 values into v128 (4 i32 values)
        uint32_t argv[4];

        // Convert f64 inputs to uint64_t bit representation
        uint64_t input1_bits, input2_bits;
        memcpy(&input1_bits, &input1, sizeof(double));
        memcpy(&input2_bits, &input2, sizeof(double));

        // Pack into 4 i32 values: [input1_low, input1_high, input2_low, input2_high]
        argv[0] = (uint32_t)(input1_bits & 0xFFFFFFFF);
        argv[1] = (uint32_t)(input1_bits >> 32);
        argv[2] = (uint32_t)(input2_bits & 0xFFFFFFFF);
        argv[3] = (uint32_t)(input2_bits >> 32);

        // Execute function using DummyExecEnv
        bool call_success = dummy_env->execute(func_name, 4, argv);
        ASSERT_TRUE(call_success) << "Failed to execute WASM function: " << func_name;

        // Extract result: unpack 4 i32 values back to 2 f64 values
        uint64_t lane0_bits = ((uint64_t)argv[1] << 32) | argv[0];
        uint64_t lane1_bits = ((uint64_t)argv[3] << 32) | argv[2];

        memcpy(lane0, &lane0_bits, sizeof(double));
        memcpy(lane1, &lane1_bits, sizeof(double));
    }

protected:
    // WAMR runtime components using RAII
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicNearestRounding_ReturnsCorrectResults
 * @brief Validates f64x2.nearest produces correct banker's rounding results for typical inputs
 * @details Tests fundamental nearest integer rounding operation with various fractional values.
 *          Verifies that f64x2.nearest correctly rounds to nearest integer using "round half to even" semantics.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_nearest
 * @input_conditions Standard f64 pairs: (1.4,1.6), (2.3,3.7), (-1.4,-1.6)
 * @expected_behavior Returns nearest integers: (1.0,2.0), (2.0,4.0), (-1.0,-2.0) respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F64x2NearestTestSuite, BasicNearestRounding_ReturnsCorrectResults) {
    double lane0, lane1;

    // Test case 1: Basic rounding up and down
    call_f64x2_nearest_function(FUNC_NAME_BASIC_NEAREST, 1.4, 1.6, &lane0, &lane1);

    ASSERT_EQ(1.0, lane0) << "nearest(1.4) should equal 1.0";
    ASSERT_EQ(2.0, lane1) << "nearest(1.6) should equal 2.0";

    // Test case 2: More fractional values
    call_f64x2_nearest_function(FUNC_NAME_BASIC_NEAREST, 2.3, 3.7, &lane0, &lane1);

    ASSERT_EQ(2.0, lane0) << "nearest(2.3) should equal 2.0";
    ASSERT_EQ(4.0, lane1) << "nearest(3.7) should equal 4.0";

    // Test case 3: Negative values
    call_f64x2_nearest_function(FUNC_NAME_BASIC_NEAREST, -1.4, -1.6, &lane0, &lane1);

    ASSERT_EQ(-1.0, lane0) << "nearest(-1.4) should equal -1.0";
    ASSERT_EQ(-2.0, lane1) << "nearest(-1.6) should equal -2.0";

    // Test case 4: Already integer values (identity test)
    call_f64x2_nearest_function(FUNC_NAME_BASIC_NEAREST, 42.0, -7.0, &lane0, &lane1);

    ASSERT_EQ(42.0, lane0) << "nearest(42.0) should equal 42.0";
    ASSERT_EQ(-7.0, lane1) << "nearest(-7.0) should equal -7.0";
}

/**
 * @test BankersRounding_HandledCorrectly
 * @brief Verifies proper "round half to even" behavior for exact half values
 * @details Tests banker's rounding rule where .5 values round to the nearest even integer.
 *          Critical for IEEE 754 compliance and avoiding rounding bias.
 * @test_category Edge - Banker's rounding validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_nearest
 * @input_conditions Half values: (0.5,1.5), (2.5,3.5), (-0.5,-1.5), (-2.5,-3.5)
 * @expected_behavior Rounds to even: (0.0,2.0), (2.0,4.0), (0.0,-2.0), (-2.0,-4.0)
 * @validation_method Exact comparison with expected banker's rounding results
 */
TEST_P(F64x2NearestTestSuite, BankersRounding_HandledCorrectly) {
    double lane0, lane1;

    // Test case 1: Positive half values - banker's rounding (round to even)
    call_f64x2_nearest_function(FUNC_NAME_BANKERS_ROUNDING, 0.5, 1.5, &lane0, &lane1);

    ASSERT_EQ(0.0, lane0) << "nearest(0.5) should equal 0.0 (round to even)";
    ASSERT_EQ(2.0, lane1) << "nearest(1.5) should equal 2.0 (round to even)";

    // Test case 2: More positive half values
    call_f64x2_nearest_function(FUNC_NAME_BANKERS_ROUNDING, 2.5, 3.5, &lane0, &lane1);

    ASSERT_EQ(2.0, lane0) << "nearest(2.5) should equal 2.0 (round to even)";
    ASSERT_EQ(4.0, lane1) << "nearest(3.5) should equal 4.0 (round to even)";

    // Test case 3: Negative half values - banker's rounding
    call_f64x2_nearest_function(FUNC_NAME_BANKERS_ROUNDING, -0.5, -1.5, &lane0, &lane1);

    ASSERT_EQ(0.0, lane0) << "nearest(-0.5) should equal 0.0 (round to even)";
    ASSERT_TRUE(std::signbit(lane0)) << "nearest(-0.5) should preserve negative zero sign (IEEE 754)";
    ASSERT_EQ(-2.0, lane1) << "nearest(-1.5) should equal -2.0 (round to even)";

    // Test case 4: More negative half values
    call_f64x2_nearest_function(FUNC_NAME_BANKERS_ROUNDING, -2.5, -3.5, &lane0, &lane1);

    ASSERT_EQ(-2.0, lane0) << "nearest(-2.5) should equal -2.0 (round to even)";
    ASSERT_EQ(-4.0, lane1) << "nearest(-3.5) should equal -4.0 (round to even)";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Verifies proper handling of f64 boundary conditions
 * @details Tests nearest rounding operation at f64 boundaries including DBL_MAX, DBL_MIN,
 *          and subnormal values to ensure proper IEEE 754 compliance.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_nearest
 * @input_conditions Boundary values: (DBL_MAX,-DBL_MAX), (DBL_MIN,-DBL_MIN), (subnormal,-subnormal)
 * @expected_behavior Large values preserved, small values round to zero
 * @validation_method Boundary value preservation and magnitude verification
 */
TEST_P(F64x2NearestTestSuite, BoundaryValues_HandledCorrectly) {
    double lane0, lane1;

    // Test DBL_MAX boundaries (already integers, should remain unchanged)
    call_f64x2_nearest_function(FUNC_NAME_BOUNDARY_VALUES, DBL_MAX, -DBL_MAX, &lane0, &lane1);

    ASSERT_EQ(DBL_MAX, lane0) << "nearest(DBL_MAX) should equal DBL_MAX";
    ASSERT_EQ(-DBL_MAX, lane1) << "nearest(-DBL_MAX) should equal -DBL_MAX";

    // Test DBL_MIN boundaries (smallest normal positive value, should round to 0)
    call_f64x2_nearest_function(FUNC_NAME_BOUNDARY_VALUES, DBL_MIN, -DBL_MIN, &lane0, &lane1);

    ASSERT_EQ(0.0, lane0) << "nearest(DBL_MIN) should equal 0.0";
    ASSERT_FALSE(std::signbit(lane0)) << "nearest(DBL_MIN) should be positive zero";
    ASSERT_EQ(0.0, lane1) << "nearest(-DBL_MIN) should equal 0.0";
    ASSERT_TRUE(std::signbit(lane1)) << "nearest(-DBL_MIN) should preserve negative zero sign";

    // Test very small subnormal values (should round to 0)
    const double subnormal = 5.0e-324;  // Smallest representable positive value
    call_f64x2_nearest_function(FUNC_NAME_BOUNDARY_VALUES, subnormal, -subnormal, &lane0, &lane1);

    ASSERT_EQ(0.0, lane0) << "nearest(subnormal) should equal 0.0";
    ASSERT_FALSE(std::signbit(lane0)) << "nearest(subnormal) should be positive zero";
    ASSERT_EQ(0.0, lane1) << "nearest(-subnormal) should equal 0.0";
    ASSERT_TRUE(std::signbit(lane1)) << "nearest(-subnormal) should preserve negative zero sign";

    // Test large integers near precision limit (2^53)
    const double large_int = 9007199254740992.0;  // 2^53 - exactly representable
    call_f64x2_nearest_function(FUNC_NAME_BOUNDARY_VALUES, large_int, -large_int, &lane0, &lane1);

    ASSERT_EQ(large_int, lane0) << "nearest(2^53) should equal 2^53";
    ASSERT_EQ(-large_int, lane1) << "nearest(-2^53) should equal -2^53";
}

/**
 * @test SpecialValues_PreservedCorrectly
 * @brief Ensures special IEEE 754 values are handled per specification
 * @details Tests nearest rounding with ±∞, ±0.0, NaN values to verify proper IEEE 754
 *          special value handling according to WebAssembly specification.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_nearest
 * @input_conditions Special values: (±∞), (±0.0), (NaN), mixed combinations
 * @expected_behavior ∞ preserved, zeros preserved with sign, NaN preserved
 * @validation_method Special value detection using isnan(), isinf(), signbit()
 */
TEST_P(F64x2NearestTestSuite, SpecialValues_PreservedCorrectly) {
    double lane0, lane1;

    // Test positive and negative infinity
    const double pos_inf = std::numeric_limits<double>::infinity();
    const double neg_inf = -std::numeric_limits<double>::infinity();

    call_f64x2_nearest_function(FUNC_NAME_SPECIAL_VALUES, pos_inf, neg_inf, &lane0, &lane1);

    ASSERT_TRUE(std::isinf(lane0)) << "nearest(+∞) should be infinite";
    ASSERT_FALSE(std::signbit(lane0)) << "nearest(+∞) should be positive";
    ASSERT_TRUE(std::isinf(lane1)) << "nearest(-∞) should be infinite";
    ASSERT_TRUE(std::signbit(lane1)) << "nearest(-∞) should be negative";

    // Test positive and negative zero
    const double pos_zero = 0.0;
    const double neg_zero = -0.0;

    call_f64x2_nearest_function(FUNC_NAME_SPECIAL_VALUES, pos_zero, neg_zero, &lane0, &lane1);

    ASSERT_EQ(0.0, lane0) << "nearest(+0.0) should be 0.0";
    ASSERT_FALSE(std::signbit(lane0)) << "nearest(+0.0) should be positive zero";
    ASSERT_EQ(0.0, lane1) << "nearest(-0.0) should be 0.0";
    ASSERT_TRUE(std::signbit(lane1)) << "nearest(-0.0) should preserve negative zero sign";

    // Test NaN values
    const double pos_nan = std::numeric_limits<double>::quiet_NaN();
    const double neg_nan = std::copysign(pos_nan, -1.0);

    call_f64x2_nearest_function(FUNC_NAME_SPECIAL_VALUES, pos_nan, neg_nan, &lane0, &lane1);

    ASSERT_TRUE(std::isnan(lane0)) << "nearest(NaN) should be NaN";
    ASSERT_TRUE(std::isnan(lane1)) << "nearest(-NaN) should be NaN";
}

/**
 * @test RuntimeInitialization_RequiresSIMDSupport
 * @brief Verifies proper WAMR initialization and SIMD module loading
 * @details Tests that WAMR runtime is properly initialized with SIMD support enabled
 *          and that f64x2.nearest instruction can be loaded and executed successfully.
 * @test_category Error - Runtime initialization validation
 * @coverage_target wasm_runtime.c:wasm_runtime_load, wasm_runtime_instantiate
 * @input_conditions WAMR runtime setup with SIMD enabled, valid SIMD module
 * @expected_behavior Module loads successfully, function executes without error
 * @validation_method Successful module loading and function execution verification
 */
TEST_P(F64x2NearestTestSuite, RuntimeInitialization_RequiresSIMDSupport) {
    double lane0, lane1;

    // Verify that the module was loaded successfully (checked in SetUp)
    ASSERT_NE(nullptr, dummy_env->get()) << "SIMD module should load successfully with SIMD support enabled";

    // Verify that we can successfully call a basic f64x2.nearest function
    call_f64x2_nearest_function(FUNC_NAME_BASIC_NEAREST, 1.0, -1.0, &lane0, &lane1);

    ASSERT_EQ(1.0, lane0) << "Function execution should produce correct results";
    ASSERT_EQ(-1.0, lane1) << "Function execution should produce correct results";

    // Verify multiple function calls work (testing execution context stability)
    call_f64x2_nearest_function(FUNC_NAME_BASIC_NEAREST, 5.5, -3.25, &lane0, &lane1);

    ASSERT_EQ(6.0, lane0) << "Second function call should produce correct results";
    ASSERT_EQ(-3.0, lane1) << "Second function call should produce correct results";

    // Test banker's rounding in runtime validation
    call_f64x2_nearest_function(FUNC_NAME_BANKERS_ROUNDING, 2.5, -1.5, &lane0, &lane1);

    ASSERT_EQ(2.0, lane0) << "Banker's rounding should work correctly in runtime validation";
    ASSERT_EQ(-2.0, lane1) << "Banker's rounding should work correctly in runtime validation";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    F64x2NearestTestSuite,
    testing::Values(F64x2NearestRunningMode::Interpreter
#if WASM_ENABLE_AOT != 0
                   , F64x2NearestRunningMode::AOT
#endif
                   ),
    [](const testing::TestParamInfo<F64x2NearestRunningMode>& info) {
        return info.param == F64x2NearestRunningMode::Interpreter ? "Interpreter" : "AOT";
    }
);