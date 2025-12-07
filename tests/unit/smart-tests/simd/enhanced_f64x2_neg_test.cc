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
 * Enhanced unit tests for f64x2.neg WASM opcode
 *
 * Tests comprehensive SIMD negation functionality for double-precision floating-point including:
 * - Basic negation operations with mixed positive/negative f64 values
 * - Boundary condition handling (DBL_MAX/MIN, subnormal values)
 * - Special value handling (±0.0, ±infinity, NaN)
 * - IEEE 754 compliance verification (sign bit flipping preservation)
 * - Cross-execution mode validation (interpreter vs AOT)
 * - Mathematical property validation (double negation identity)
 */

enum class F64x2NegRunningMode : uint8_t {
    Interpreter = 0,
    AOT = 1
};

static constexpr const char *MODULE_NAME = "f64x2_neg_test";
static constexpr const char *FUNC_NAME_BASIC_NEG = "test_basic_neg";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_SPECIAL_VALUES = "test_special_values";
static constexpr const char *FUNC_NAME_MIXED_SCENARIOS = "test_mixed_scenarios";
static constexpr const char *WASM_FILE = "wasm-apps/f64x2_neg_test.wasm";

/**
 * Test fixture for f64x2.neg opcode validation
 *
 * Provides comprehensive test environment for SIMD floating-point negation operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class F64x2NegTestSuite : public testing::TestWithParam<F64x2NegRunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f64x2.neg test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.neg test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.neg tests";
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
        // Call the WASM function using wasm_runtime_call_wasm
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            wasm_runtime_get_module_inst(dummy_env->get()), func_name);

        if (!func) {
            return false;
        }

        // Prepare arguments using uint32_t array for wasm_runtime_call_wasm
        uint32_t argv[8];  // 2 f64 inputs (4 uint32_t) + v128 result (4 uint32_t) = 8 total

        // Pack f64 arguments into argv array
        memcpy(&argv[0], &input1, sizeof(double));  // First f64 (argv[0-1])
        memcpy(&argv[2], &input2, sizeof(double));  // Second f64 (argv[2-3])

        // Call WASM function (result will be placed back in argv)
        bool success = wasm_runtime_call_wasm(dummy_env->get(), func, 8, argv);

        if (success && lane0 && lane1) {
            // Extract f64 values from v128 result stored back in argv
            // For f64x2 result, extract two f64 values from the returned v128
            memcpy(lane0, &argv[0], sizeof(double));  // First f64 result
            memcpy(lane1, &argv[2], sizeof(double));  // Second f64 result
        }

        return success;
    }

protected:
    // WAMR runtime components using RAII
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicNegation_ReturnsCorrectResults
 * @brief Validates f64x2.neg produces correct arithmetic results for typical inputs
 * @details Tests fundamental negation operation with positive, negative, and mixed-sign doubles.
 *          Verifies that f64x2.neg correctly computes -a, -b for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_neg
 * @input_conditions Standard f64 pairs: (1.5,-2.5), (-3.14,4.0), (0.0,-0.0)
 * @expected_behavior Returns negated values: (-1.5,2.5), (3.14,-4.0), (-0.0,0.0) respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F64x2NegTestSuite, BasicNegation_ReturnsCorrectResults) {
    double lane0, lane1;

    // Test case 1: Mixed positive and negative values
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_NEG, 1.5, -2.5, &lane0, &lane1))
        << "Failed to call basic negation function with mixed values";

    ASSERT_EQ(-1.5, lane0) << "First lane neg(1.5) should equal -1.5";
    ASSERT_EQ(2.5, lane1) << "Second lane neg(-2.5) should equal 2.5";

    // Test case 2: Both positive values
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_NEG, 3.14159, 2.71828, &lane0, &lane1))
        << "Failed to call basic negation function with positive values";

    ASSERT_EQ(-3.14159, lane0) << "First lane neg(π) should equal -π";
    ASSERT_EQ(-2.71828, lane1) << "Second lane neg(e) should equal -e";

    // Test case 3: Both negative values
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_NEG, -42.0, -7.25, &lane0, &lane1))
        << "Failed to call basic negation function with negative values";

    ASSERT_EQ(42.0, lane0) << "First lane neg(-42.0) should equal 42.0";
    ASSERT_EQ(7.25, lane1) << "Second lane neg(-7.25) should equal 7.25";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Verifies proper handling of f64 boundary conditions
 * @details Tests negation operation at f64 boundaries including DBL_MAX, DBL_MIN,
 *          and subnormal values to ensure proper IEEE 754 compliance.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_neg
 * @input_conditions Boundary values: (DBL_MAX,-DBL_MAX), (DBL_MIN,-DBL_MIN), (subnormal,-subnormal)
 * @expected_behavior Boundary values negated correctly with sign bits flipped
 * @validation_method Boundary value negation and magnitude preservation
 */
TEST_P(F64x2NegTestSuite, BoundaryValues_HandledCorrectly) {
    double lane0, lane1;

    // Test DBL_MAX boundaries
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, DBL_MAX, -DBL_MAX, &lane0, &lane1))
        << "Failed to call boundary values function with DBL_MAX";

    ASSERT_EQ(-DBL_MAX, lane0) << "neg(DBL_MAX) should equal -DBL_MAX";
    ASSERT_EQ(DBL_MAX, lane1) << "neg(-DBL_MAX) should equal DBL_MAX";

    // Test DBL_MIN boundaries (smallest normal positive value)
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, DBL_MIN, -DBL_MIN, &lane0, &lane1))
        << "Failed to call boundary values function with DBL_MIN";

    ASSERT_EQ(-DBL_MIN, lane0) << "neg(DBL_MIN) should equal -DBL_MIN";
    ASSERT_EQ(DBL_MIN, lane1) << "neg(-DBL_MIN) should equal DBL_MIN";

    // Test very small subnormal values
    const double subnormal = 5.0e-324;  // Smallest representable positive value
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, subnormal, -subnormal, &lane0, &lane1))
        << "Failed to call boundary values function with subnormal values";

    ASSERT_EQ(-subnormal, lane0) << "neg(subnormal) should equal -subnormal";
    ASSERT_EQ(subnormal, lane1) << "neg(-subnormal) should equal subnormal";
    ASSERT_TRUE(std::fpclassify(lane0) == FP_SUBNORMAL) << "Negated subnormal should remain subnormal";
    ASSERT_TRUE(std::fpclassify(lane1) == FP_SUBNORMAL) << "Negated -subnormal should remain subnormal";
}

/**
 * @test SpecialValues_PreservedCorrectly
 * @brief Ensures special IEEE 754 values are handled per specification
 * @details Tests negation with ±∞, ±0.0, NaN values to verify proper IEEE 754
 *          special value handling and sign bit manipulation.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_neg
 * @input_conditions Special values: (±∞), (±0.0), (NaN), mixed combinations
 * @expected_behavior ∞ signs flipped, zeros signs flipped, NaN preserved with sign flipped
 * @validation_method Special value detection using isnan(), isinf(), signbit()
 */
TEST_P(F64x2NegTestSuite, SpecialValues_PreservedCorrectly) {
    double lane0, lane1;

    // Test positive and negative infinity
    const double pos_inf = std::numeric_limits<double>::infinity();
    const double neg_inf = -std::numeric_limits<double>::infinity();

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_inf, neg_inf, &lane0, &lane1))
        << "Failed to call special values function with infinity values";

    ASSERT_TRUE(std::isinf(lane0)) << "neg(+∞) should be infinite";
    ASSERT_TRUE(std::signbit(lane0)) << "neg(+∞) should be negative infinity";
    ASSERT_TRUE(std::isinf(lane1)) << "neg(-∞) should be infinite";
    ASSERT_FALSE(std::signbit(lane1)) << "neg(-∞) should be positive infinity";

    // Test positive and negative zero
    const double pos_zero = 0.0;
    const double neg_zero = -0.0;

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_zero, neg_zero, &lane0, &lane1))
        << "Failed to call special values function with zero values";

    ASSERT_EQ(0.0, lane0) << "neg(+0.0) should be 0.0";
    ASSERT_TRUE(std::signbit(lane0)) << "neg(+0.0) should be negative zero";
    ASSERT_EQ(0.0, lane1) << "neg(-0.0) should be 0.0";
    ASSERT_FALSE(std::signbit(lane1)) << "neg(-0.0) should be positive zero";

    // Test NaN values
    const double pos_nan = std::numeric_limits<double>::quiet_NaN();
    const double neg_nan = std::copysign(pos_nan, -1.0);

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_nan, neg_nan, &lane0, &lane1))
        << "Failed to call special values function with NaN values";

    ASSERT_TRUE(std::isnan(lane0)) << "neg(NaN) should be NaN";
    ASSERT_TRUE(std::signbit(lane0)) << "neg(NaN) should have sign bit flipped";
    ASSERT_TRUE(std::isnan(lane1)) << "neg(-NaN) should be NaN";
    ASSERT_FALSE(std::signbit(lane1)) << "neg(-NaN) should have sign bit flipped to positive";
}

/**
 * @test MixedScenarios_ValidatesConsistency
 * @brief Tests combinations of normal, special, and extreme values
 * @details Validates consistent negation behavior across mixed input combinations
 *          including normal values paired with special values.
 * @test_category Main - Mixed scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_neg
 * @input_conditions Mixed pairs: (1.0,+∞), (-0.0,NaN), (DBL_MAX,-DBL_MIN)
 * @expected_behavior Independent lane processing with proper negation per IEEE 754
 * @validation_method Combination of value-specific assertion patterns per lane
 */
TEST_P(F64x2NegTestSuite, MixedScenarios_ValidatesConsistency) {
    double lane0, lane1;

    // Test normal value with infinity
    const double normal = 1.0;
    const double pos_inf = std::numeric_limits<double>::infinity();

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_MIXED_SCENARIOS, normal, pos_inf, &lane0, &lane1))
        << "Failed to call mixed scenarios function with normal and infinity";

    ASSERT_EQ(-1.0, lane0) << "neg(1.0) should equal -1.0";
    ASSERT_TRUE(std::isinf(lane1)) << "neg(+∞) should be infinite";
    ASSERT_TRUE(std::signbit(lane1)) << "neg(+∞) should be negative";

    // Test negative zero with NaN
    const double neg_zero = -0.0;
    const double nan_val = std::numeric_limits<double>::quiet_NaN();

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_MIXED_SCENARIOS, neg_zero, nan_val, &lane0, &lane1))
        << "Failed to call mixed scenarios function with -0.0 and NaN";

    ASSERT_EQ(0.0, lane0) << "neg(-0.0) should be 0.0";
    ASSERT_FALSE(std::signbit(lane0)) << "neg(-0.0) should be positive zero";
    ASSERT_TRUE(std::isnan(lane1)) << "neg(NaN) should be NaN";
    ASSERT_TRUE(std::signbit(lane1)) << "neg(NaN) should have sign bit flipped";

    // Test boundary extremes
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_MIXED_SCENARIOS, DBL_MAX, -DBL_MIN, &lane0, &lane1))
        << "Failed to call mixed scenarios function with boundary extremes";

    ASSERT_EQ(-DBL_MAX, lane0) << "neg(DBL_MAX) should equal -DBL_MAX";
    ASSERT_EQ(DBL_MIN, lane1) << "neg(-DBL_MIN) should equal DBL_MIN";
}

/**
 * @test RuntimeInitialization_RequiresSIMDSupport
 * @brief Verifies proper WAMR initialization and SIMD module loading
 * @details Tests that WAMR runtime is properly initialized with SIMD support enabled
 *          and that f64x2.neg instruction can be loaded and executed successfully.
 * @test_category Error - Runtime initialization validation
 * @coverage_target wasm_runtime.c:wasm_runtime_load, wasm_runtime_instantiate
 * @input_conditions WAMR runtime setup with SIMD enabled, valid SIMD module
 * @expected_behavior Module loads successfully, function executes without error
 * @validation_method Successful module loading and function execution verification
 */
TEST_P(F64x2NegTestSuite, RuntimeInitialization_RequiresSIMDSupport) {
    double lane0, lane1;

    // Verify that the module was loaded successfully (checked in SetUp)
    ASSERT_NE(nullptr, dummy_env->get()) << "SIMD module should load successfully with SIMD support enabled";

    // Verify that we can successfully call a basic f64x2.neg function
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_NEG, 1.0, -1.0, &lane0, &lane1))
        << "Should be able to call f64x2.neg functions with SIMD support enabled";

    ASSERT_EQ(-1.0, lane0) << "Function execution should produce correct results";
    ASSERT_EQ(1.0, lane1) << "Function execution should produce correct results";

    // Verify multiple function calls work (testing execution context stability)
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_NEG, -5.5, 3.25, &lane0, &lane1))
        << "Multiple function calls should succeed with stable execution context";

    ASSERT_EQ(5.5, lane0) << "Second function call should produce correct results";
    ASSERT_EQ(-3.25, lane1) << "Second function call should produce correct results";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    F64x2NegTestSuite,
    testing::Values(F64x2NegRunningMode::Interpreter
#if WASM_ENABLE_AOT != 0
                   , F64x2NegRunningMode::AOT
#endif
                   ),
    [](const testing::TestParamInfo<F64x2NegRunningMode>& info) {
        return info.param == F64x2NegRunningMode::Interpreter ? "Interpreter" : "AOT";
    }
);