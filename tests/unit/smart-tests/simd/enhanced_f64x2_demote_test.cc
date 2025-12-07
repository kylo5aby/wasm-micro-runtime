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

static constexpr const char *MODULE_NAME = "f64x2_demote_test";
static constexpr const char *FUNC_NAME_DEMOTE = "test_f64x2_demote";
static constexpr const char *WASM_FILE = "wasm-apps/f64x2_demote_test.wasm";

/**
 * Enhanced test suite for WASM f64x2.demote opcode validation
 *
 * @brief Comprehensive testing of f32x4.demote_f64x2_zero SIMD instruction which converts
 *        two f64 values to two f32 values with IEEE 754 precision handling
 * @details Tests cover normal conversions, boundary conditions, special values,
 *          precision loss scenarios. The instruction zeros lanes 2 and 3 of the result.
 * @category SIMD Float - Vector floating-point type conversion operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c (f32x4.demote_f64x2_zero handler)
 */
class F64x2DemoteTestSuite : public testing::Test {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f32x4.demote_f64x2_zero test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.demote test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.demote tests";
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
     * Helper function to call WASM f64x2.demote function with two f64 values and extract f32x4 result
     *
     * @param f64_1 First f64 input value
     * @param f64_2 Second f64 input value
     * @param lane0 Output for first f32 lane
     * @param lane1 Output for second f32 lane
     * @return true if function execution succeeded, false otherwise
     */
    bool call_f64x2_demote_function(double f64_1, double f64_2, float *lane0, float *lane1)
    {
        uint32_t argv[4];  // 2 f64 inputs = 4 uint32_t words

        // Pack f64 values into argv buffer
        memcpy(&argv[0], &f64_1, sizeof(double));
        memcpy(&argv[2], &f64_2, sizeof(double));

        bool ret = dummy_env->execute(FUNC_NAME_DEMOTE, 4, argv);
        if (!ret) {
            return false;
        }

        // Extract f32x4 result - demote_f64x2_zero produces f32 values in lanes 0,1 and zeros in lanes 2,3
        *lane0 = *(float *)&argv[0];  // Lane 0 f32
        *lane1 = *(float *)&argv[1];  // Lane 1 f32

        return true;
    }

    /**
     * Helper to check if two float values are approximately equal or have same special value properties
     *
     * @param expected Expected float value
     * @param actual Actual float value
     * @param tolerance Relative tolerance for comparison of finite values
     * @return true if values match within tolerance or have same special properties
     */
    bool float_equals(float expected, float actual, float tolerance = 1e-6f)
    {
        // Handle NaN cases
        if (std::isnan(expected) && std::isnan(actual)) return true;
        if (std::isnan(expected) || std::isnan(actual)) return false;

        // Handle infinity cases
        if (std::isinf(expected) && std::isinf(actual)) {
            return (std::signbit(expected) == std::signbit(actual));
        }
        if (std::isinf(expected) || std::isinf(actual)) return false;

        // Handle zero cases (preserve sign)
        if (expected == 0.0f && actual == 0.0f) {
            return (std::signbit(expected) == std::signbit(actual));
        }

        // Handle normal finite values
        if (expected == 0.0f) {
            return std::abs(actual) <= tolerance;
        }
        return std::abs(expected - actual) <= tolerance * std::abs(expected);
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicDemotion_ReturnsCorrectConversion
 * @brief Validates f64x2.demote produces correct f32 results for typical f64 inputs
 * @details Tests fundamental f64→f32 conversion with standard values that fit within f32 range.
 *          Verifies IEEE 754 conversion behavior with representable values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_demote_handler
 * @input_conditions Standard f64 pairs: (1.5, -2.25), (42.0, -1000.0), (0.125, 3.14159)
 * @expected_behavior Returns equivalent f32 values with preserved precision where possible
 * @validation_method Direct comparison of WASM function result with expected f32 values
 */
TEST_F(F64x2DemoteTestSuite, BasicDemotion_ReturnsCorrectConversion)
{
    float lane0, lane1;

    // Test standard f64 values that convert cleanly to f32
    ASSERT_TRUE(call_f64x2_demote_function(1.5, -2.25, &lane0, &lane1))
        << "Failed to call f64x2.demote function with standard values";
    ASSERT_TRUE(float_equals(1.5f, lane0))
        << "Lane 0: Expected 1.5f, got " << lane0;
    ASSERT_TRUE(float_equals(-2.25f, lane1))
        << "Lane 1: Expected -2.25f, got " << lane1;

    // Test larger values within f32 range
    ASSERT_TRUE(call_f64x2_demote_function(42.0, -1000.0, &lane0, &lane1))
        << "Failed to call f64x2.demote function with larger values";
    ASSERT_TRUE(float_equals(42.0f, lane0))
        << "Lane 0: Expected 42.0f, got " << lane0;
    ASSERT_TRUE(float_equals(-1000.0f, lane1))
        << "Lane 1: Expected -1000.0f, got " << lane1;

    // Test fractional values
    ASSERT_TRUE(call_f64x2_demote_function(0.125, 3.14159, &lane0, &lane1))
        << "Failed to call f64x2.demote function with fractional values";
    ASSERT_TRUE(float_equals(0.125f, lane0))
        << "Lane 0: Expected 0.125f, got " << lane0;
    ASSERT_TRUE(float_equals(3.14159f, lane1))
        << "Lane 1: Expected 3.14159f, got " << lane1;
}

/**
 * @test PrecisionHandling_TruncatesMantissaCorrectly
 * @brief Validates f64x2.demote handles precision loss during f64→f32 conversion
 * @details Tests high-precision f64 values that cannot be exactly represented in f32.
 *          Verifies proper IEEE 754 rounding behavior and mantissa truncation.
 * @test_category Main - Precision handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_demote_handler
 * @input_conditions High-precision f64 values requiring mantissa truncation
 * @expected_behavior Returns properly rounded f32 values per IEEE 754 standards
 * @validation_method Compare results with expected IEEE 754 f64→f32 conversion outcomes
 */
TEST_F(F64x2DemoteTestSuite, PrecisionHandling_TruncatesMantissaCorrectly)
{
    float lane0, lane1;

    // Test f64 value with high precision that gets truncated in f32
    double high_precision = 1.23456789012345;
    float expected_f32 = static_cast<float>(high_precision); // IEEE 754 conversion

    ASSERT_TRUE(call_f64x2_demote_function(high_precision, high_precision, &lane0, &lane1))
        << "Failed to call f64x2.demote function with high precision values";
    ASSERT_TRUE(float_equals(expected_f32, lane0))
        << "Lane 0: Expected " << expected_f32 << ", got " << lane0;
    ASSERT_TRUE(float_equals(expected_f32, lane1))
        << "Lane 1: Expected " << expected_f32 << ", got " << lane1;

    // Test another high-precision case
    double high_precision2 = -9.87654321098765e15;
    float expected_f32_2 = static_cast<float>(high_precision2);

    ASSERT_TRUE(call_f64x2_demote_function(high_precision2, 0.0, &lane0, &lane1))
        << "Failed to call f64x2.demote function with large high precision value";
    ASSERT_TRUE(float_equals(expected_f32_2, lane0))
        << "Lane 0: Expected " << expected_f32_2 << ", got " << lane0;
}

/**
 * @test BoundaryConversion_HandlesLimits
 * @brief Validates f64x2.demote correctly handles f64 values at f32 boundary limits
 * @details Tests f64 values at the edge of f32 representable range to verify
 *          proper boundary condition handling without overflow/underflow.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_demote_handler
 * @input_conditions f64 values at f32 limits: ±FLT_MAX, ±FLT_MIN representable values
 * @expected_behavior Returns f32 boundary values without overflow to infinity
 * @validation_method Compare with IEEE 754 expected conversion results for boundary values
 */
TEST_F(F64x2DemoteTestSuite, BoundaryConversion_HandlesLimits)
{
    // Test values near f32 maximum (but representable)
    double near_f32_max = static_cast<double>(FLT_MAX) * 0.999;
    float expected_max = static_cast<float>(near_f32_max);

    float lane0, lane1;
    ASSERT_TRUE(call_f64x2_demote_function(near_f32_max, -near_f32_max, &lane0, &lane1))
        << "Failed to call f64x2.demote function with boundary values";
    ASSERT_TRUE(float_equals(expected_max, lane0))
        << "Lane 0: Expected " << expected_max << ", got " << lane0;
    ASSERT_TRUE(float_equals(-expected_max, lane1))
        << "Lane 1: Expected " << -expected_max << ", got " << lane1;

    // Test values near f32 minimum normal (smallest positive normal f32)
    double near_f32_min = static_cast<double>(FLT_MIN) * 2.0;
    float expected_min = static_cast<float>(near_f32_min);

    ASSERT_TRUE(call_f64x2_demote_function(near_f32_min, -near_f32_min, &lane0, &lane1))
        << "Failed to call f64x2.demote function with minimum boundary values";
    ASSERT_TRUE(float_equals(expected_min, lane0))
        << "Lane 0: Expected " << expected_min << ", got " << lane0;
    ASSERT_TRUE(float_equals(-expected_min, lane1))
        << "Lane 1: Expected " << -expected_min << ", got " << lane1;
}

/**
 * @test OverflowConversion_BecomesInfinity
 * @brief Validates f64x2.demote converts f64 overflow values to f32 infinity
 * @details Tests f64 values exceeding f32 range to verify proper overflow handling
 *          per IEEE 754 standards (large values become ±infinity).
 * @test_category Corner - Overflow condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_demote_handler
 * @input_conditions f64 values exceeding f32 range (> FLT_MAX, < -FLT_MAX)
 * @expected_behavior Returns ±infinity for overflow conditions
 * @validation_method Verify results are infinity with correct sign
 */
TEST_F(F64x2DemoteTestSuite, OverflowConversion_BecomesInfinity)
{
    // Test f64 values that exceed f32 range (should become infinity)
    double overflow_positive = static_cast<double>(FLT_MAX) * 2.0;
    double overflow_negative = -overflow_positive;

    float lane0, lane1;
    ASSERT_TRUE(call_f64x2_demote_function(overflow_positive, overflow_negative, &lane0, &lane1))
        << "Failed to call f64x2.demote function with overflow values";

    ASSERT_TRUE(std::isinf(lane0))
        << "Lane 0: Expected +infinity for overflow, got " << lane0;
    ASSERT_FALSE(std::signbit(lane0))
        << "Lane 0: Expected positive infinity";

    ASSERT_TRUE(std::isinf(lane1))
        << "Lane 1: Expected -infinity for overflow, got " << lane1;
    ASSERT_TRUE(std::signbit(lane1))
        << "Lane 1: Expected negative infinity";

    // Test with extreme large values
    double extreme_large = 1e100;  // Much larger than f32 can represent
    ASSERT_TRUE(call_f64x2_demote_function(extreme_large, -extreme_large, &lane0, &lane1))
        << "Failed to call f64x2.demote function with extreme large values";

    ASSERT_TRUE(std::isinf(lane0) && !std::signbit(lane0))
        << "Lane 0: Expected +infinity for extreme value, got " << lane0;
    ASSERT_TRUE(std::isinf(lane1) && std::signbit(lane1))
        << "Lane 1: Expected -infinity for extreme value, got " << lane1;
}

/**
 * @test UnderflowConversion_BecomesSubnormalOrZero
 * @brief Validates f64x2.demote handles f64 underflow values correctly
 * @details Tests very small f64 values to verify proper underflow handling
 *          (values become subnormal f32 or flush to zero per IEEE 754).
 * @test_category Corner - Underflow condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_demote_handler
 * @input_conditions Very small f64 values (< FLT_MIN, approaching zero)
 * @expected_behavior Returns subnormal f32 values or zero for extreme underflow
 * @validation_method Verify results follow IEEE 754 underflow behavior
 */
TEST_F(F64x2DemoteTestSuite, UnderflowConversion_BecomesSubnormalOrZero)
{
    // Test very small f64 values (should become subnormal or zero in f32)
    double underflow_positive = static_cast<double>(FLT_MIN) * 0.5;
    double underflow_negative = -underflow_positive;

    float lane0, lane1;
    ASSERT_TRUE(call_f64x2_demote_function(underflow_positive, underflow_negative, &lane0, &lane1))
        << "Failed to call f64x2.demote function with underflow values";

    // Results should be finite (not NaN or infinity) and very small
    ASSERT_TRUE(std::isfinite(lane0))
        << "Lane 0: Expected finite result for underflow, got " << lane0;
    ASSERT_TRUE(std::isfinite(lane1))
        << "Lane 1: Expected finite result for underflow, got " << lane1;

    ASSERT_LE(std::abs(lane0), FLT_MIN)
        << "Lane 0: Expected very small positive result, got " << lane0;
    ASSERT_LE(std::abs(lane1), FLT_MIN)
        << "Lane 1: Expected very small negative result, got " << lane1;

    // Test extremely small values that should flush to zero
    double extremely_small = 1e-50;  // Much smaller than f32 can represent
    ASSERT_TRUE(call_f64x2_demote_function(extremely_small, -extremely_small, &lane0, &lane1))
        << "Failed to call f64x2.demote function with extremely small values";

    // May be zero or subnormal, but should maintain sign
    if (lane0 == 0.0f) {
        ASSERT_FALSE(std::signbit(lane0))
            << "Lane 0: Expected positive zero";
    }
    if (lane1 == 0.0f) {
        ASSERT_TRUE(std::signbit(lane1))
            << "Lane 1: Expected negative zero";
    }
}

/**
 * @test SpecialValues_PreservesNaNAndInfinity
 * @brief Validates f64x2.demote preserves special IEEE 754 values correctly
 * @details Tests NaN, infinity, and signed zero handling to verify proper
 *          special value conversion from f64 to f32 format.
 * @test_category Edge - Special value preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_demote_handler
 * @input_conditions Special f64 values: NaN, ±infinity, ±0.0
 * @expected_behavior Preserves special value semantics in f32 format
 * @validation_method Verify special value properties (isnan, isinf, signbit) are maintained
 */
TEST_F(F64x2DemoteTestSuite, SpecialValues_PreservesNaNAndInfinity)
{
    float lane0, lane1;

    // Test NaN preservation
    double nan_value = std::numeric_limits<double>::quiet_NaN();
    ASSERT_TRUE(call_f64x2_demote_function(nan_value, nan_value, &lane0, &lane1))
        << "Failed to call f64x2.demote function with NaN values";

    ASSERT_TRUE(std::isnan(lane0))
        << "Lane 0: Expected NaN preservation, got " << lane0;
    ASSERT_TRUE(std::isnan(lane1))
        << "Lane 1: Expected NaN preservation, got " << lane1;

    // Test infinity preservation
    double pos_inf = std::numeric_limits<double>::infinity();
    double neg_inf = -pos_inf;
    ASSERT_TRUE(call_f64x2_demote_function(pos_inf, neg_inf, &lane0, &lane1))
        << "Failed to call f64x2.demote function with infinity values";

    ASSERT_TRUE(std::isinf(lane0) && !std::signbit(lane0))
        << "Lane 0: Expected positive infinity, got " << lane0;
    ASSERT_TRUE(std::isinf(lane1) && std::signbit(lane1))
        << "Lane 1: Expected negative infinity, got " << lane1;

    // Test signed zero preservation
    double pos_zero = 0.0;
    double neg_zero = -0.0;
    ASSERT_TRUE(call_f64x2_demote_function(pos_zero, neg_zero, &lane0, &lane1))
        << "Failed to call f64x2.demote function with signed zero values";

    ASSERT_EQ(0.0f, lane0)
        << "Lane 0: Expected positive zero, got " << lane0;
    ASSERT_FALSE(std::signbit(lane0))
        << "Lane 0: Expected positive zero sign";

    ASSERT_EQ(0.0f, lane1)
        << "Lane 1: Expected negative zero, got " << lane1;
    ASSERT_TRUE(std::signbit(lane1))
        << "Lane 1: Expected negative zero sign";
}

/**
 * @test SubnormalInput_HandlesCorrectly
 * @brief Validates f64x2.demote correctly processes subnormal f64 inputs
 * @details Tests subnormal (denormalized) f64 values to verify proper handling
 *          of non-normalized input values in SIMD conversion context.
 * @test_category Edge - Subnormal input handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_demote_handler
 * @input_conditions Subnormal f64 values (very small denormalized numbers)
 * @expected_behavior Processes subnormal inputs and produces valid f32 outputs
 * @validation_method Verify subnormal f64 inputs convert to expected f32 results
 */
TEST_F(F64x2DemoteTestSuite, SubnormalInput_HandlesCorrectly)
{
    // Create subnormal f64 values (very small, denormalized)
    double subnormal_f64 = std::numeric_limits<double>::denorm_min();
    double subnormal_neg_f64 = -subnormal_f64;

    float lane0, lane1;
    ASSERT_TRUE(call_f64x2_demote_function(subnormal_f64, subnormal_neg_f64, &lane0, &lane1))
        << "Failed to call f64x2.demote function with subnormal values";

    // Subnormal f64 should convert to zero or subnormal f32
    ASSERT_TRUE(std::isfinite(lane0))
        << "Lane 0: Expected finite result for subnormal input, got " << lane0;
    ASSERT_TRUE(std::isfinite(lane1))
        << "Lane 1: Expected finite result for subnormal input, got " << lane1;

    // Results should be very small or zero
    ASSERT_LE(std::abs(lane0), FLT_MIN)
        << "Lane 0: Expected very small result from subnormal f64, got " << lane0;
    ASSERT_LE(std::abs(lane1), FLT_MIN)
        << "Lane 1: Expected very small result from subnormal f64, got " << lane1;

    // Test larger subnormal that might preserve some precision
    double larger_subnormal = subnormal_f64 * 1e10;
    ASSERT_TRUE(call_f64x2_demote_function(larger_subnormal, -larger_subnormal, &lane0, &lane1))
        << "Failed to call f64x2.demote function with larger subnormal values";

    ASSERT_TRUE(std::isfinite(lane0))
        << "Lane 0: Expected finite result, got " << lane0;
    ASSERT_TRUE(std::isfinite(lane1))
        << "Lane 1: Expected finite result, got " << lane1;
}

// Note: This test runs in interpreter mode by default with DummyExecEnv helper
// AOT testing would require separate test infrastructure for compiled modules