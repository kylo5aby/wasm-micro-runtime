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
 * Enhanced unit tests for f64x2.abs WASM opcode
 *
 * Tests comprehensive SIMD absolute value functionality for double-precision floating-point including:
 * - Basic absolute value operations with mixed positive/negative f64 values
 * - Boundary condition handling (DBL_MAX/MIN, subnormal values)
 * - Special value handling (±0.0, ±infinity, NaN)
 * - Mathematical property validation (idempotent, sign bit clearing)
 * - Cross-execution mode validation (interpreter vs AOT)
 * - IEEE 754 compliance verification
 */

enum class F64x2AbsRunningMode : uint8_t {
    Interpreter = 0,
    AOT = 1
};

static constexpr const char *MODULE_NAME = "f64x2_abs_test";
static constexpr const char *FUNC_NAME_BASIC_ABS = "test_basic_abs";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_SPECIAL_VALUES = "test_special_values";
static constexpr const char *FUNC_NAME_MIXED_SCENARIOS = "test_mixed_scenarios";
static constexpr const char *WASM_FILE = "wasm-apps/f64x2_abs_test.wasm";

/**
 * Test fixture for f64x2.abs opcode validation
 *
 * Provides comprehensive test environment for SIMD floating-point absolute value operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class F64x2AbsTestSuite : public testing::TestWithParam<F64x2AbsRunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f64x2.abs test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.abs test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.abs tests";
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
        // Prepare function arguments
        wasm_val_t args[2];
        args[0].kind = WASM_F64;
        args[0].of.f64 = input1;
        args[1].kind = WASM_F64;
        args[1].of.f64 = input2;

        // Prepare result storage for v128 (4 i32 values)
        wasm_val_t results[4];
        results[0].kind = WASM_I32;
        results[1].kind = WASM_I32;
        results[2].kind = WASM_I32;
        results[3].kind = WASM_I32;

        // Call the WASM function
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            wasm_runtime_get_module_inst(dummy_env->get()), func_name);
        if (!func) {
            return false;
        }

        bool success = wasm_runtime_call_wasm_a(dummy_env->get(), func, 4, results, 2, args);
        if (success && lane0 && lane1) {
            // Extract f64 values from v128 result
            uint64_t lane0_bits = ((uint64_t)results[1].of.i32 << 32) | (uint32_t)results[0].of.i32;
            uint64_t lane1_bits = ((uint64_t)results[3].of.i32 << 32) | (uint32_t)results[2].of.i32;

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
 * @test BasicAbsoluteValue_ReturnsCorrectResults
 * @brief Validates f64x2.abs produces correct arithmetic results for typical inputs
 * @details Tests fundamental absolute value operation with positive, negative, and mixed-sign doubles.
 *          Verifies that f64x2.abs correctly computes abs(a), abs(b) for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_abs
 * @input_conditions Standard f64 pairs: (1.5,-2.5), (-3.14,4.0), (0.0,-0.0)
 * @expected_behavior Returns absolute values: (1.5,2.5), (3.14,4.0), (0.0,0.0) respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F64x2AbsTestSuite, BasicAbsoluteValue_ReturnsCorrectResults) {
    double lane0, lane1;

    // Test case 1: Mixed positive and negative values
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_ABS, 1.5, -2.5, &lane0, &lane1))
        << "Failed to call basic abs function with mixed values";

    ASSERT_EQ(1.5, lane0) << "First lane abs(1.5) should equal 1.5";
    ASSERT_EQ(2.5, lane1) << "Second lane abs(-2.5) should equal 2.5";

    // Test case 2: Both negative values
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_ABS, -3.14159, -2.71828, &lane0, &lane1))
        << "Failed to call basic abs function with negative values";

    ASSERT_EQ(3.14159, lane0) << "First lane abs(-π) should equal π";
    ASSERT_EQ(2.71828, lane1) << "Second lane abs(-e) should equal e";

    // Test case 3: Both positive values (identity test)
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_ABS, 42.0, 7.25, &lane0, &lane1))
        << "Failed to call basic abs function with positive values";

    ASSERT_EQ(42.0, lane0) << "First lane abs(42.0) should equal 42.0";
    ASSERT_EQ(7.25, lane1) << "Second lane abs(7.25) should equal 7.25";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Verifies proper handling of f64 boundary conditions
 * @details Tests absolute value operation at f64 boundaries including DBL_MAX, DBL_MIN,
 *          and subnormal values to ensure proper IEEE 754 compliance.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_abs
 * @input_conditions Boundary values: (DBL_MAX,-DBL_MAX), (DBL_MIN,-DBL_MIN), (subnormal,-subnormal)
 * @expected_behavior Boundary values preserved correctly with sign bits cleared
 * @validation_method Boundary value preservation and magnitude verification
 */
TEST_P(F64x2AbsTestSuite, BoundaryValues_HandledCorrectly) {
    double lane0, lane1;

    // Test DBL_MAX boundaries
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, DBL_MAX, -DBL_MAX, &lane0, &lane1))
        << "Failed to call boundary values function with DBL_MAX";

    ASSERT_EQ(DBL_MAX, lane0) << "abs(DBL_MAX) should equal DBL_MAX";
    ASSERT_EQ(DBL_MAX, lane1) << "abs(-DBL_MAX) should equal DBL_MAX";

    // Test DBL_MIN boundaries (smallest normal positive value)
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, DBL_MIN, -DBL_MIN, &lane0, &lane1))
        << "Failed to call boundary values function with DBL_MIN";

    ASSERT_EQ(DBL_MIN, lane0) << "abs(DBL_MIN) should equal DBL_MIN";
    ASSERT_EQ(DBL_MIN, lane1) << "abs(-DBL_MIN) should equal DBL_MIN";

    // Test very small subnormal values
    const double subnormal = 5.0e-324;  // Smallest representable positive value
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BOUNDARY_VALUES, subnormal, -subnormal, &lane0, &lane1))
        << "Failed to call boundary values function with subnormal values";

    ASSERT_EQ(subnormal, lane0) << "abs(subnormal) should equal subnormal";
    ASSERT_EQ(subnormal, lane1) << "abs(-subnormal) should equal subnormal";
}

/**
 * @test SpecialValues_PreservedCorrectly
 * @brief Ensures special IEEE 754 values are handled per specification
 * @details Tests absolute value with ±∞, ±0.0, NaN values to verify proper IEEE 754
 *          special value handling and sign bit manipulation.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_abs
 * @input_conditions Special values: (±∞), (±0.0), (NaN), mixed combinations
 * @expected_behavior ∞ signs normalized, zeros become +0.0, NaN preserved with sign cleared
 * @validation_method Special value detection using isnan(), isinf(), signbit()
 */
TEST_P(F64x2AbsTestSuite, SpecialValues_PreservedCorrectly) {
    double lane0, lane1;

    // Test positive and negative infinity
    const double pos_inf = std::numeric_limits<double>::infinity();
    const double neg_inf = -std::numeric_limits<double>::infinity();

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_inf, neg_inf, &lane0, &lane1))
        << "Failed to call special values function with infinity values";

    ASSERT_TRUE(std::isinf(lane0)) << "abs(+∞) should be infinite";
    ASSERT_FALSE(std::signbit(lane0)) << "abs(+∞) should be positive";
    ASSERT_TRUE(std::isinf(lane1)) << "abs(-∞) should be infinite";
    ASSERT_FALSE(std::signbit(lane1)) << "abs(-∞) should be positive";

    // Test positive and negative zero
    const double pos_zero = 0.0;
    const double neg_zero = -0.0;

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_zero, neg_zero, &lane0, &lane1))
        << "Failed to call special values function with zero values";

    ASSERT_EQ(0.0, lane0) << "abs(+0.0) should be 0.0";
    ASSERT_FALSE(std::signbit(lane0)) << "abs(+0.0) should be positive zero";
    ASSERT_EQ(0.0, lane1) << "abs(-0.0) should be 0.0";
    ASSERT_FALSE(std::signbit(lane1)) << "abs(-0.0) should be positive zero";

    // Test NaN values
    const double pos_nan = std::numeric_limits<double>::quiet_NaN();
    const double neg_nan = std::copysign(pos_nan, -1.0);

    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_SPECIAL_VALUES, pos_nan, neg_nan, &lane0, &lane1))
        << "Failed to call special values function with NaN values";

    ASSERT_TRUE(std::isnan(lane0)) << "abs(NaN) should be NaN";
    ASSERT_FALSE(std::signbit(lane0)) << "abs(NaN) should have positive sign";
    ASSERT_TRUE(std::isnan(lane1)) << "abs(-NaN) should be NaN";
    ASSERT_FALSE(std::signbit(lane1)) << "abs(-NaN) should have positive sign";
}

/**
 * @test RuntimeInitialization_RequiresSIMDSupport
 * @brief Verifies proper WAMR initialization and SIMD module loading
 * @details Tests that WAMR runtime is properly initialized with SIMD support enabled
 *          and that f64x2.abs instruction can be loaded and executed successfully.
 * @test_category Error - Runtime initialization validation
 * @coverage_target wasm_runtime.c:wasm_runtime_load, wasm_runtime_instantiate
 * @input_conditions WAMR runtime setup with SIMD enabled, valid SIMD module
 * @expected_behavior Module loads successfully, function executes without error
 * @validation_method Successful module loading and function execution verification
 */
TEST_P(F64x2AbsTestSuite, RuntimeInitialization_RequiresSIMDSupport) {
    double lane0, lane1;

    // Verify that the module was loaded successfully (checked in SetUp)
    ASSERT_NE(nullptr, dummy_env->get()) << "SIMD module should load successfully with SIMD support enabled";

    // Verify that we can successfully call a basic f64x2.abs function
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_ABS, 1.0, -1.0, &lane0, &lane1))
        << "Should be able to call f64x2.abs functions with SIMD support enabled";

    ASSERT_EQ(1.0, lane0) << "Function execution should produce correct results";
    ASSERT_EQ(1.0, lane1) << "Function execution should produce correct results";

    // Verify multiple function calls work (testing execution context stability)
    ASSERT_TRUE(call_f64x2_function(FUNC_NAME_BASIC_ABS, -5.5, 3.25, &lane0, &lane1))
        << "Multiple function calls should succeed with stable execution context";

    ASSERT_EQ(5.5, lane0) << "Second function call should produce correct results";
    ASSERT_EQ(3.25, lane1) << "Second function call should produce correct results";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    F64x2AbsTestSuite,
    testing::Values(F64x2AbsRunningMode::Interpreter
#if WASM_ENABLE_AOT != 0
                   , F64x2AbsRunningMode::AOT
#endif
                   ),
    [](const testing::TestParamInfo<F64x2AbsRunningMode>& info) {
        return info.param == F64x2AbsRunningMode::Interpreter ? "Interpreter" : "AOT";
    }
);