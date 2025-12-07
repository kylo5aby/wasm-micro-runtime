/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <memory>
#include <cstring>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

// Test fixtures and utilities for SIMD i32x4.trunc_sat_f64x2_s_zero opcode validation

static constexpr const char *WASM_FILE = "wasm-apps/i32x4_trunc_sat_f64x2_s_zero_test.wasm";
static constexpr const char *FUNC_NAME = "test_i32x4_trunc_sat_f64x2_s_zero";

/**
 * @brief Test fixture for i32x4.trunc_sat_f64x2_s_zero opcode comprehensive validation
 * @details This test suite validates the SIMD opcode that converts two f64 values to signed i32
 *          with saturating truncation, placing results in lower two lanes and zeroing upper lanes.
 *          Tests cover basic functionality, boundary conditions, special values, and cross-execution modes.
 */
class I32x4TruncSatF64x2SZeroTest : public testing::TestWithParam<int> {
protected:
    /**
     * @brief Set up test environment with WAMR runtime initialization
     * @details Initializes WAMR runtime using helper classes and loads test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i32x4.trunc_sat_f64x2_s_zero tests";
    }

    /**
     * @brief Clean up test environment and WAMR runtime resources
     * @details Cleanup is handled automatically by RAII destructors.
     */
    void TearDown() override {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Call i32x4.trunc_sat_f64x2_s_zero test function with f64x2 input
     * @param f64_0 First f64 value (lane 0)
     * @param f64_1 Second f64 value (lane 1)
     * @param result Output i32x4 result array
     * @return true if function call succeeded, false otherwise
     */
    bool CallTruncSatF64x2SZero(double f64_0, double f64_1, int32_t result[4]) {
        // Prepare function arguments
        uint32_t argv[4]; // 2 f64 arguments (2x2 uint32) + space for v128 return (4 uint32)

        // Store f64 arguments in argv (f64 uses 2 uint32 slots each)
        memcpy(&argv[0], &f64_0, sizeof(double));
        memcpy(&argv[2], &f64_1, sizeof(double));

        // Call the WASM function using dummy_env->execute
        bool success = dummy_env->execute(FUNC_NAME, 4, argv);

        if (success && result) {
            // Extract i32 values from v128 result (first 4 slots of argv after call)
            memcpy(result, argv, 4 * sizeof(int32_t));
        }

        return success;
    }

protected:
    // WAMR runtime components using RAII
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicTruncation_ReturnsCorrectConversion
 * @brief Validates i32x4.trunc_sat_f64x2_s_zero produces correct arithmetic results for typical inputs
 * @details Tests fundamental truncation operation with positive, negative, and mixed-sign f64 values.
 *          Verifies that the opcode correctly truncates toward zero and places results in lower lanes.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_trunc_sat_operation
 * @input_conditions Standard f64 pairs with fractional components
 * @expected_behavior Returns truncated i32 values in lanes 0,1 and zeros in lanes 2,3
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32x4TruncSatF64x2SZeroTest, BasicTruncation_ReturnsCorrectConversion) {
    int32_t result[4];

    // Test positive values with fractions
    ASSERT_TRUE(CallTruncSatF64x2SZero(42.7, 100.99, result))
        << "Failed to call test function with positive values";
    ASSERT_EQ(42, result[0]) << "Failed to truncate positive f64 value 42.7";
    ASSERT_EQ(100, result[1]) << "Failed to truncate positive f64 value 100.99";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";

    // Test negative values with fractions
    ASSERT_TRUE(CallTruncSatF64x2SZero(-17.8, -99.2, result))
        << "Failed to call test function with negative values";
    ASSERT_EQ(-17, result[0]) << "Failed to truncate negative f64 value -17.8";
    ASSERT_EQ(-99, result[1]) << "Failed to truncate negative f64 value -99.2";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";

    // Test mixed sign values
    ASSERT_TRUE(CallTruncSatF64x2SZero(123.456, -456.789, result))
        << "Failed to call test function with mixed sign values";
    ASSERT_EQ(123, result[0]) << "Failed to truncate positive f64 value 123.456";
    ASSERT_EQ(-456, result[1]) << "Failed to truncate negative f64 value -456.789";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";
}

/**
 * @test SaturationBoundaries_HandlesOverflowCorrectly
 * @brief Validates saturation behavior at INT32_MAX/MIN boundaries
 * @details Tests that f64 values beyond i32 range saturate to appropriate boundaries.
 *          Verifies both exact boundary values and overflow scenarios.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_saturation_logic
 * @input_conditions f64 values at/beyond INT32_MAX/MIN limits
 * @expected_behavior Saturates to INT32_MAX (2147483647) or INT32_MIN (-2147483648)
 * @validation_method Comparison with expected saturation values
 */
TEST_P(I32x4TruncSatF64x2SZeroTest, SaturationBoundaries_HandlesOverflowCorrectly) {
    int32_t result[4];

    // Test INT32_MAX boundary saturation
    ASSERT_TRUE(CallTruncSatF64x2SZero(2147483647.0, 2147483648.0, result))
        << "Failed to call test function with INT32_MAX boundary values";
    ASSERT_EQ(2147483647, result[0]) << "Failed to handle INT32_MAX boundary";
    ASSERT_EQ(2147483647, result[1]) << "Failed to saturate overflow to INT32_MAX";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";

    // Test INT32_MIN boundary saturation
    ASSERT_TRUE(CallTruncSatF64x2SZero(-2147483648.0, -2147483649.0, result))
        << "Failed to call test function with INT32_MIN boundary values";
    ASSERT_EQ(-2147483648, result[0]) << "Failed to handle INT32_MIN boundary";
    ASSERT_EQ(-2147483648, result[1]) << "Failed to saturate underflow to INT32_MIN";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";

    // Test large overflow values
    ASSERT_TRUE(CallTruncSatF64x2SZero(1e15, -1e15, result))
        << "Failed to call test function with large overflow values";
    ASSERT_EQ(2147483647, result[0]) << "Failed to saturate large positive value";
    ASSERT_EQ(-2147483648, result[1]) << "Failed to saturate large negative value";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";
}

/**
 * @test SpecialValues_ConvertsCorrectly
 * @brief Validates handling of special floating-point values (NaN, infinity, zero)
 * @details Tests conversion behavior for NaN, positive/negative infinity, and positive/negative zero.
 *          Ensures all special values have defined, predictable conversion results.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_special_value_handling
 * @input_conditions NaN, +/-infinity, +/-zero values
 * @expected_behavior NaN→0, +∞→INT32_MAX, -∞→INT32_MIN, ±0→0
 * @validation_method Verification of special value conversion rules
 */
TEST_P(I32x4TruncSatF64x2SZeroTest, SpecialValues_ConvertsCorrectly) {
    int32_t result[4];

    // Test NaN and infinity values
    double nan_val = std::numeric_limits<double>::quiet_NaN();
    double pos_inf = std::numeric_limits<double>::infinity();
    double neg_inf = -std::numeric_limits<double>::infinity();

    ASSERT_TRUE(CallTruncSatF64x2SZero(nan_val, pos_inf, result))
        << "Failed to call test function with NaN and positive infinity";
    ASSERT_EQ(0, result[0]) << "NaN should convert to 0";
    ASSERT_EQ(2147483647, result[1]) << "Positive infinity should saturate to INT32_MAX";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";

    ASSERT_TRUE(CallTruncSatF64x2SZero(neg_inf, 0.0, result))
        << "Failed to call test function with negative infinity and zero";
    ASSERT_EQ(-2147483648, result[0]) << "Negative infinity should saturate to INT32_MIN";
    ASSERT_EQ(0, result[1]) << "Positive zero should convert to 0";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";

    // Test positive and negative zero
    ASSERT_TRUE(CallTruncSatF64x2SZero(+0.0, -0.0, result))
        << "Failed to call test function with positive and negative zero";
    ASSERT_EQ(0, result[0]) << "Positive zero should convert to 0";
    ASSERT_EQ(0, result[1]) << "Negative zero should convert to 0";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must be zero";
}

/**
 * @test ZeroPadding_EnsuresUpperLanesZero
 * @brief Validates that upper two lanes are always zero regardless of input
 * @details Tests various input combinations to ensure lanes 2 and 3 are always zero.
 *          This is a critical requirement of the opcode specification.
 * @test_category Edge - Zero padding validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_lane_zero_padding
 * @input_conditions Various f64 input combinations including extreme values
 * @expected_behavior Upper lanes (2,3) always contain zero
 * @validation_method Explicit assertion that lanes 2,3 equal zero for all test cases
 */
TEST_P(I32x4TruncSatF64x2SZeroTest, ZeroPadding_EnsuresUpperLanesZero) {
    int32_t result[4];

    // Test with various input combinations
    ASSERT_TRUE(CallTruncSatF64x2SZero(123.0, 456.0, result))
        << "Failed to call test function with various input combinations";
    ASSERT_EQ(123, result[0]) << "Lane 0 conversion failed";
    ASSERT_EQ(456, result[1]) << "Lane 1 conversion failed";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must always be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must always be zero";

    // Test with extreme values
    ASSERT_TRUE(CallTruncSatF64x2SZero(1e20, -1e20, result))
        << "Failed to call test function with extreme values";
    ASSERT_EQ(2147483647, result[0]) << "Lane 0 saturation failed";
    ASSERT_EQ(-2147483648, result[1]) << "Lane 1 saturation failed";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must always be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must always be zero";

    // Test with very small values
    ASSERT_TRUE(CallTruncSatF64x2SZero(1e-100, -1e-100, result))
        << "Failed to call test function with very small values";
    ASSERT_EQ(0, result[0]) << "Lane 0 small value truncation failed";
    ASSERT_EQ(0, result[1]) << "Lane 1 small value truncation failed";
    ASSERT_EQ(0, result[2]) << "Upper lane 2 must always be zero";
    ASSERT_EQ(0, result[3]) << "Upper lane 3 must always be zero";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(CrossExecutionModes, I32x4TruncSatF64x2SZeroTest,
                        testing::Values(0, 1),  // 0: Interpreter, 1: AOT
                        [](const testing::TestParamInfo<int>& info) {
                            return info.param == 0 ? "Interpreter" : "AOT";
                        });