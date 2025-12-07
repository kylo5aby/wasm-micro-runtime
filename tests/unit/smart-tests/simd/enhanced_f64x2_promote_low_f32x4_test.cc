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

static constexpr const char *MODULE_NAME = "f64x2_promote_low_f32x4_test";
static constexpr const char *FUNC_NAME_PROMOTE_BASIC = "test_promote_basic";
static constexpr const char *FUNC_NAME_PROMOTE_BOUNDARY = "test_promote_boundary";
static constexpr const char *FUNC_NAME_PROMOTE_SPECIAL = "test_promote_special";
static constexpr const char *FUNC_NAME_PROMOTE_ZERO = "test_promote_zero";
static constexpr const char *WASM_FILE = "wasm-apps/f64x2_promote_low_f32x4_test.wasm";

/**
 * Enhanced test suite for WASM f64x2.promote_low_f32x4 opcode validation
 *
 * @brief Comprehensive testing of f64x2.promote_low_f32x4 SIMD instruction which converts
 *        the two lower f32 values in a v128 vector to f64 precision values
 * @details Tests cover normal conversions, boundary conditions, special values (NaN, infinity),
 *          signed zero handling, and precision preservation. The instruction extracts lanes 0,1
 *          from input f32x4 and promotes them to f64 in output f64x2.
 * @category SIMD Float - Vector floating-point type conversion operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c (f64x2.promote_low_f32x4 handler)
 */
class F64x2PromoteLowF32x4TestSuite : public testing::Test {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f64x2.promote_low_f32x4 test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.promote_low_f32x4 test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.promote_low_f32x4 tests";
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
     * Helper function to call WASM f64x2.promote_low_f32x4 function with f32x4 input and extract f64x2 result
     *
     * @param f32_0 First f32 input value (lane 0, will be promoted)
     * @param f32_1 Second f32 input value (lane 1, will be promoted)
     * @param f32_2 Third f32 input value (lane 2, ignored during promotion)
     * @param f32_3 Fourth f32 input value (lane 3, ignored during promotion)
     * @param func_name WASM function name to call
     * @param result_f64_0 Output for first promoted f64 value
     * @param result_f64_1 Output for second promoted f64 value
     * @return true if function execution succeeded, false otherwise
     */
    bool call_f64x2_promote_function(float f32_0, float f32_1, float f32_2, float f32_3,
                                   const char *func_name, double *result_f64_0, double *result_f64_1)
    {
        uint32_t argv[4];  // 4 f32 inputs = 4 uint32_t words

        // Pack f32x4 vector into argv buffer
        memcpy(&argv[0], &f32_0, sizeof(float));
        memcpy(&argv[1], &f32_1, sizeof(float));
        memcpy(&argv[2], &f32_2, sizeof(float));
        memcpy(&argv[3], &f32_3, sizeof(float));

        bool ret = dummy_env->execute(func_name, 4, argv);
        if (!ret) {
            return false;
        }

        // Extract f64x2 result - promote_low extracts lanes 0,1 from f32x4 and promotes to f64
        memcpy(result_f64_0, &argv[0], sizeof(double));  // First f64 from promoted f32 lane 0
        memcpy(result_f64_1, &argv[2], sizeof(double));  // Second f64 from promoted f32 lane 1

        return true;
    }

    /**
     * Helper to check if two double values are exactly equal or have same special value properties
     *
     * @param expected Expected double value
     * @param actual Actual double value
     * @return true if values are exactly equal or have same special properties
     */
    bool double_equals(double expected, double actual)
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
        if (expected == 0.0 && actual == 0.0) {
            return (std::signbit(expected) == std::signbit(actual));
        }

        // For f32→f64 promotion, values should be exactly equal (no precision loss)
        return expected == actual;
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicPromotion_TypicalValues_ReturnsCorrectResults
 * @brief Validates f64x2.promote_low_f32x4 produces correct f64 results for typical f32 inputs
 * @details Tests fundamental f32→f64 promotion with standard values. Verifies that lower two
 *          lanes (0, 1) of f32x4 are correctly extracted and promoted to f64 precision while
 *          upper lanes (2, 3) are ignored during the conversion process.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_promote_low_f32x4_handler
 * @input_conditions Standard f32x4 vector: [1.5f, 2.25f, 999.0f, 888.0f] (only lanes 0,1 matter)
 * @expected_behavior Returns f64x2 vector: [1.5, 2.25] (exact promotion, upper lanes ignored)
 * @validation_method Direct comparison of WASM function result with expected f64 values
 */
TEST_F(F64x2PromoteLowF32x4TestSuite, BasicPromotion_TypicalValues_ReturnsCorrectResults)
{
    double result_f64_0, result_f64_1;

    // Test standard f32 values that promote exactly to f64 (lanes 2,3 are ignored)
    ASSERT_TRUE(call_f64x2_promote_function(1.5f, 2.25f, 999.0f, 888.0f,
                                          FUNC_NAME_PROMOTE_BASIC, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with standard values";
    ASSERT_TRUE(double_equals(1.5, result_f64_0))
        << "Lane 0: Expected 1.5, got " << result_f64_0;
    ASSERT_TRUE(double_equals(2.25, result_f64_1))
        << "Lane 1: Expected 2.25, got " << result_f64_1;

    // Test with negative values and small fractional parts
    ASSERT_TRUE(call_f64x2_promote_function(-3.75f, 0.125f, 1000.0f, -2000.0f,
                                          FUNC_NAME_PROMOTE_BASIC, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with negative/fractional values";
    ASSERT_TRUE(double_equals(-3.75, result_f64_0))
        << "Lane 0: Expected -3.75, got " << result_f64_0;
    ASSERT_TRUE(double_equals(0.125, result_f64_1))
        << "Lane 1: Expected 0.125, got " << result_f64_1;

    // Test with larger values within f32 range
    ASSERT_TRUE(call_f64x2_promote_function(42000.5f, -1500.25f, 0.0f, 0.0f,
                                          FUNC_NAME_PROMOTE_BASIC, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with larger values";
    ASSERT_TRUE(double_equals(42000.5, result_f64_0))
        << "Lane 0: Expected 42000.5, got " << result_f64_0;
    ASSERT_TRUE(double_equals(-1500.25, result_f64_1))
        << "Lane 1: Expected -1500.25, got " << result_f64_1;
}

/**
 * @test BoundaryValues_MinMaxLimits_PromotesCorrectly
 * @brief Validates f64x2.promote_low_f32x4 correctly handles f32 boundary values
 * @details Tests promotion of f32 boundary values (MIN, MAX, denormals) to verify
 *          proper boundary condition handling. All f32 values should promote exactly
 *          to f64 since f64 has greater precision and range than f32.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_promote_low_f32x4_handler
 * @input_conditions f32x4 with boundary values: [FLT_MIN, FLT_MAX, ignored, ignored]
 * @expected_behavior Returns f64x2 with exact representations of FLT_MIN and FLT_MAX
 * @validation_method Compare results with expected exact f32→f64 promotion values
 */
TEST_F(F64x2PromoteLowF32x4TestSuite, BoundaryValues_MinMaxLimits_PromotesCorrectly)
{
    double result_f64_0, result_f64_1;

    // Test f32 boundary values: MIN and MAX
    float f32_min = FLT_MIN;  // Smallest positive normalized f32
    float f32_max = FLT_MAX;  // Largest finite f32
    ASSERT_TRUE(call_f64x2_promote_function(f32_min, f32_max, 0.0f, 0.0f,
                                          FUNC_NAME_PROMOTE_BOUNDARY, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with boundary values";

    // Promotion should be exact (no precision loss from f32 to f64)
    ASSERT_TRUE(double_equals(static_cast<double>(f32_min), result_f64_0))
        << "Lane 0: Expected " << static_cast<double>(f32_min) << ", got " << result_f64_0;
    ASSERT_TRUE(double_equals(static_cast<double>(f32_max), result_f64_1))
        << "Lane 1: Expected " << static_cast<double>(f32_max) << ", got " << result_f64_1;

    // Test negative boundary values
    ASSERT_TRUE(call_f64x2_promote_function(-f32_min, -f32_max, 1.0f, 1.0f,
                                          FUNC_NAME_PROMOTE_BOUNDARY, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with negative boundary values";

    ASSERT_TRUE(double_equals(static_cast<double>(-f32_min), result_f64_0))
        << "Lane 0: Expected " << static_cast<double>(-f32_min) << ", got " << result_f64_0;
    ASSERT_TRUE(double_equals(static_cast<double>(-f32_max), result_f64_1))
        << "Lane 1: Expected " << static_cast<double>(-f32_max) << ", got " << result_f64_1;

    // Test denormal (subnormal) f32 values
    float denormal_min = std::numeric_limits<float>::denorm_min();
    ASSERT_TRUE(call_f64x2_promote_function(denormal_min, -denormal_min, 0.0f, 0.0f,
                                          FUNC_NAME_PROMOTE_BOUNDARY, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with denormal values";

    ASSERT_TRUE(double_equals(static_cast<double>(denormal_min), result_f64_0))
        << "Lane 0: Expected " << static_cast<double>(denormal_min) << ", got " << result_f64_0;
    ASSERT_TRUE(double_equals(static_cast<double>(-denormal_min), result_f64_1))
        << "Lane 1: Expected " << static_cast<double>(-denormal_min) << ", got " << result_f64_1;
}

/**
 * @test SpecialValues_NanInfinity_PreservesIeeeSemantics
 * @brief Validates f64x2.promote_low_f32x4 preserves IEEE 754 special values correctly
 * @details Tests NaN, infinity, and signed zero handling to verify proper special value
 *          conversion from f32 to f64 format. All IEEE 754 special values should be
 *          preserved during promotion with proper sign and payload handling.
 * @test_category Edge - Special value preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_promote_low_f32x4_handler
 * @input_conditions f32x4 with special values: [NaN, +∞, -∞, -0.0f] combinations
 * @expected_behavior Preserves special value semantics in f64 format with proper signs
 * @validation_method Verify special value properties (isnan, isinf, signbit) are maintained
 */
TEST_F(F64x2PromoteLowF32x4TestSuite, SpecialValues_NanInfinity_PreservesIeeeSemantics)
{
    double result_f64_0, result_f64_1;

    // Test NaN and infinity preservation
    float nan_f32 = std::numeric_limits<float>::quiet_NaN();
    float pos_inf_f32 = std::numeric_limits<float>::infinity();
    ASSERT_TRUE(call_f64x2_promote_function(nan_f32, pos_inf_f32, 0.0f, 0.0f,
                                          FUNC_NAME_PROMOTE_SPECIAL, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with NaN and +∞";

    ASSERT_TRUE(std::isnan(result_f64_0))
        << "Lane 0: Expected NaN preservation, got " << result_f64_0;
    ASSERT_TRUE(std::isinf(result_f64_1) && !std::signbit(result_f64_1))
        << "Lane 1: Expected positive infinity, got " << result_f64_1;

    // Test negative infinity and signaling NaN
    float neg_inf_f32 = -std::numeric_limits<float>::infinity();
    float snan_f32 = std::numeric_limits<float>::signaling_NaN();
    ASSERT_TRUE(call_f64x2_promote_function(neg_inf_f32, snan_f32, 1.0f, 1.0f,
                                          FUNC_NAME_PROMOTE_SPECIAL, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with -∞ and sNaN";

    ASSERT_TRUE(std::isinf(result_f64_0) && std::signbit(result_f64_0))
        << "Lane 0: Expected negative infinity, got " << result_f64_0;
    ASSERT_TRUE(std::isnan(result_f64_1))
        << "Lane 1: Expected NaN (from sNaN), got " << result_f64_1;

    // Test mixed special values
    ASSERT_TRUE(call_f64x2_promote_function(pos_inf_f32, neg_inf_f32, 0.0f, 0.0f,
                                          FUNC_NAME_PROMOTE_SPECIAL, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with mixed infinities";

    ASSERT_TRUE(std::isinf(result_f64_0) && !std::signbit(result_f64_0))
        << "Lane 0: Expected positive infinity, got " << result_f64_0;
    ASSERT_TRUE(std::isinf(result_f64_1) && std::signbit(result_f64_1))
        << "Lane 1: Expected negative infinity, got " << result_f64_1;
}

/**
 * @test ZeroValues_PositiveNegative_MaintainsSignBit
 * @brief Validates f64x2.promote_low_f32x4 maintains signed zero distinction
 * @details Tests signed zero promotion (+0.0f vs -0.0f) to verify proper sign bit
 *          preservation during f32→f64 conversion. IEEE 754 requires distinction
 *          between positive and negative zero to be maintained.
 * @test_category Edge - Signed zero handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f64x2_promote_low_f32x4_handler
 * @input_conditions f32x4 with signed zeros: [+0.0f, -0.0f, ignored, ignored]
 * @expected_behavior Returns f64x2 with [+0.0, -0.0] maintaining sign bits correctly
 * @validation_method Use signbit() to verify sign preservation for zero values
 */
TEST_F(F64x2PromoteLowF32x4TestSuite, ZeroValues_PositiveNegative_MaintainsSignBit)
{
    double result_f64_0, result_f64_1;

    // Test positive and negative zero preservation
    float pos_zero = 0.0f;
    float neg_zero = -0.0f;
    ASSERT_TRUE(call_f64x2_promote_function(pos_zero, neg_zero, 1.0f, 1.0f,
                                          FUNC_NAME_PROMOTE_ZERO, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with signed zeros";

    // Verify both results are zero with correct signs
    ASSERT_EQ(0.0, result_f64_0)
        << "Lane 0: Expected positive zero value, got " << result_f64_0;
    ASSERT_FALSE(std::signbit(result_f64_0))
        << "Lane 0: Expected positive zero sign, got negative sign";

    ASSERT_EQ(0.0, result_f64_1)
        << "Lane 1: Expected negative zero value, got " << result_f64_1;
    ASSERT_TRUE(std::signbit(result_f64_1))
        << "Lane 1: Expected negative zero sign, got positive sign";

    // Test reversed order
    ASSERT_TRUE(call_f64x2_promote_function(neg_zero, pos_zero, -1.0f, -1.0f,
                                          FUNC_NAME_PROMOTE_ZERO, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with reversed signed zeros";

    ASSERT_EQ(0.0, result_f64_0)
        << "Lane 0: Expected negative zero value, got " << result_f64_0;
    ASSERT_TRUE(std::signbit(result_f64_0))
        << "Lane 0: Expected negative zero sign, got positive sign";

    ASSERT_EQ(0.0, result_f64_1)
        << "Lane 1: Expected positive zero value, got " << result_f64_1;
    ASSERT_FALSE(std::signbit(result_f64_1))
        << "Lane 1: Expected positive zero sign, got negative sign";

    // Test mixed zero and non-zero values
    ASSERT_TRUE(call_f64x2_promote_function(pos_zero, 1.0f, 0.0f, 0.0f,
                                          FUNC_NAME_PROMOTE_ZERO, &result_f64_0, &result_f64_1))
        << "Failed to call f64x2.promote_low_f32x4 function with mixed zero/non-zero";

    ASSERT_TRUE(double_equals(0.0, result_f64_0) && !std::signbit(result_f64_0))
        << "Lane 0: Expected positive zero, got " << result_f64_0;
    ASSERT_TRUE(double_equals(1.0, result_f64_1))
        << "Lane 1: Expected 1.0, got " << result_f64_1;
}

// Note: This test runs in interpreter mode by default with DummyExecEnv helper
// AOT testing would require separate test infrastructure for compiled modules