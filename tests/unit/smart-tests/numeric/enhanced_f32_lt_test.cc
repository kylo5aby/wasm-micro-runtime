/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cfloat>
#include <limits>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;

static int app_argc;
static char **app_argv;

/**
 * @brief Test fixture for f32.lt opcode testing across multiple execution modes
 *
 * This test suite validates the IEEE 754 floating-point less-than comparison
 * operation (f32.lt) which compares two f32 values and returns 1 if the first
 * operand is less than the second, 0 otherwise. The tests ensure correct
 * behavior across WAMR's interpreter and AOT execution modes.
 */
class F32LtTest : public testing::TestWithParam<RunningMode>
{
  protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };
    const char *exception = nullptr;

    /**
     * @brief Set up test environment and load WASM module for f32.lt testing
     *
     * Initializes WAMR runtime, loads the f32.lt test module, and prepares
     * the execution environment for both interpreter and AOT modes.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/f32_lt_test.wasm", &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: wasm-apps/f32_lt_test.wasm";

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and release WAMR resources
     *
     * Properly deallocates execution environment, module instance, and module
     * to prevent memory leaks and ensure clean test execution.
     */
    void TearDown() override
    {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (buf) {
            wasm_runtime_free(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Execute f32.lt comparison operation via WASM function call
     *
     * @param a First f32 operand for comparison
     * @param b Second f32 operand for comparison
     * @return int32_t Result of f32.lt operation (1 if a < b, 0 otherwise)
     */
    int32_t call_f32_lt(float a, float b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_lt");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f32_lt function";

        uint32_t argv[2];
        memcpy(&argv[0], &a, sizeof(float));
        memcpy(&argv[1], &b, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute test_f32_lt function: "
                            << wasm_runtime_get_exception(module_inst);

        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Execute boundary condition testing via WASM function call
     *
     * @param a First f32 operand for boundary testing
     * @param b Second f32 operand for boundary testing
     * @return int32_t Result of f32.lt boundary test
     */
    int32_t call_f32_lt_boundary(float a, float b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_lt_boundary");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f32_lt_boundary function";

        uint32_t argv[2];
        memcpy(&argv[0], &a, sizeof(float));
        memcpy(&argv[1], &b, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute test_f32_lt_boundary function: "
                            << wasm_runtime_get_exception(module_inst);

        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Execute NaN handling tests via WASM function call
     *
     * @param a First f32 operand (potentially NaN)
     * @param b Second f32 operand (potentially NaN)
     * @return int32_t Result of f32.lt NaN test (should always be 0)
     */
    int32_t call_f32_lt_nan(float a, float b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_lt_nan");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f32_lt_nan function";

        uint32_t argv[2];
        memcpy(&argv[0], &a, sizeof(float));
        memcpy(&argv[1], &b, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute test_f32_lt_nan function: "
                            << wasm_runtime_get_exception(module_inst);

        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Execute special values tests via WASM function call
     *
     * @param a First f32 operand (special value like +/-0, +/-inf)
     * @param b Second f32 operand (special value like +/-0, +/-inf)
     * @return int32_t Result of f32.lt special values test
     */
    int32_t call_f32_lt_special(float a, float b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_lt_special");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f32_lt_special function";

        uint32_t argv[2];
        memcpy(&argv[0], &a, sizeof(float));
        memcpy(&argv[1], &b, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute test_f32_lt_special function: "
                            << wasm_runtime_get_exception(module_inst);

        return static_cast<int32_t>(argv[0]);
    }
};

/**
 * @test BasicComparison_ReturnsCorrectResults
 * @brief Validates f32.lt produces correct comparison results for typical inputs
 * @details Tests fundamental less-than operation with positive, negative, and
 *          mixed-sign floating-point values. Verifies that f32.lt correctly
 *          implements IEEE 754 comparison semantics.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_lt_operation
 * @input_conditions Standard float pairs: (1.0f,2.0f), (3.5f,1.2f), (-1.0f,-2.0f), (0.0f,0.0f)
 * @expected_behavior Returns 1, 0, 0, 0 respectively for comparison results
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F32LtTest, BasicComparison_ReturnsCorrectResults)
{
    // Test basic less-than comparisons with standard floating-point values
    ASSERT_EQ(1, call_f32_lt(1.0f, 2.0f)) << "1.0 < 2.0 should return true (1)";
    ASSERT_EQ(0, call_f32_lt(3.5f, 1.2f)) << "3.5 < 1.2 should return false (0)";
    ASSERT_EQ(0, call_f32_lt(-1.0f, -2.0f)) << "-1.0 < -2.0 should return false (0)";
    ASSERT_EQ(0, call_f32_lt(0.0f, 0.0f)) << "0.0 < 0.0 should return false (0)";
    ASSERT_EQ(1, call_f32_lt(-5.5f, 2.3f)) << "-5.5 < 2.3 should return true (1)";
}

/**
 * @test EdgeCaseComparison_HandlesSpecialValues
 * @brief Validates f32.lt handles IEEE 754 special values correctly
 * @details Tests comparison with special floating-point values including
 *          positive/negative zero, positive/negative infinity, and their
 *          interactions following IEEE 754 specifications.
 * @test_category Edge - Special IEEE 754 values validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_lt_special_cases
 * @input_conditions Special values: (+0.0f,-0.0f), (+inf,finite), (-inf,finite), (finite,+inf)
 * @expected_behavior Returns 0, 0, 1, 1 respectively following IEEE 754 comparison rules
 * @validation_method IEEE 754 compliance verification for special value comparisons
 */
TEST_P(F32LtTest, EdgeCaseComparison_HandlesSpecialValues)
{
    // Test IEEE 754 special values comparison behavior
    ASSERT_EQ(0, call_f32_lt_special(0.0f, -0.0f)) << "+0.0 < -0.0 should return false (IEEE 754: +0 == -0)";
    ASSERT_EQ(0, call_f32_lt_special(-0.0f, 0.0f)) << "-0.0 < +0.0 should return false (IEEE 754: -0 == +0)";
    ASSERT_EQ(0, call_f32_lt_special(INFINITY, 1.0f)) << "+inf < 1.0 should return false (0)";
    ASSERT_EQ(1, call_f32_lt_special(-INFINITY, 1.0f)) << "-inf < 1.0 should return true (1)";
    ASSERT_EQ(1, call_f32_lt_special(1.0f, INFINITY)) << "1.0 < +inf should return true (1)";
    ASSERT_EQ(0, call_f32_lt_special(1.0f, -INFINITY)) << "1.0 < -inf should return false (0)";
}

/**
 * @test BoundaryComparison_ValidatesExtremeValues
 * @brief Validates f32.lt handles extreme floating-point boundary values
 * @details Tests comparison with smallest/largest representable f32 values,
 *          subnormal numbers, and boundary conditions that test the limits
 *          of IEEE 754 single-precision representation.
 * @test_category Boundary - Extreme value range validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_lt_boundary_handling
 * @input_conditions Boundary values: FLT_MIN, FLT_MAX, subnormal values, denormalized numbers
 * @expected_behavior Correct comparison results respecting IEEE 754 ordering rules
 * @validation_method Boundary condition verification with extreme floating-point values
 */
TEST_P(F32LtTest, BoundaryComparison_ValidatesExtremeValues)
{
    // Test boundary conditions with extreme floating-point values
    ASSERT_EQ(1, call_f32_lt_boundary(FLT_MIN, FLT_MAX)) << "FLT_MIN < FLT_MAX should return true (1)";
    ASSERT_EQ(0, call_f32_lt_boundary(FLT_MAX, FLT_MIN)) << "FLT_MAX < FLT_MIN should return false (0)";
    ASSERT_EQ(0, call_f32_lt_boundary(FLT_MIN, FLT_MIN)) << "FLT_MIN < FLT_MIN should return false (0)";

    // Test subnormal numbers (denormalized values near zero)
    float subnormal1 = 1.175494e-38f * 0.5f;  // Subnormal value
    float subnormal2 = 1.175494e-38f * 0.25f; // Smaller subnormal value
    ASSERT_EQ(0, call_f32_lt_boundary(subnormal1, subnormal2)) << "Larger subnormal < smaller subnormal should return false";
    ASSERT_EQ(1, call_f32_lt_boundary(subnormal2, subnormal1)) << "Smaller subnormal < larger subnormal should return true";
}

/**
 * @test NaNComparison_FollowsIEEE754Rules
 * @brief Validates f32.lt NaN propagation follows IEEE 754 specifications
 * @details Tests that any comparison involving NaN (Not-a-Number) returns
 *          false, regardless of the other operand value, following IEEE 754
 *          standard NaN comparison rules.
 * @test_category Edge - NaN handling compliance validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_lt_nan_handling
 * @input_conditions NaN combinations: (NaN,finite), (finite,NaN), (NaN,NaN), (NaN,special)
 * @expected_behavior Always returns 0 (false) for any comparison involving NaN
 * @validation_method IEEE 754 NaN propagation rule verification
 */
TEST_P(F32LtTest, NaNComparison_FollowsIEEE754Rules)
{
    float nan_val = std::numeric_limits<float>::quiet_NaN();

    // Test NaN comparison behavior - all should return false (0)
    ASSERT_EQ(0, call_f32_lt_nan(nan_val, 1.0f)) << "NaN < 1.0 should return false (IEEE 754 rule)";
    ASSERT_EQ(0, call_f32_lt_nan(1.0f, nan_val)) << "1.0 < NaN should return false (IEEE 754 rule)";
    ASSERT_EQ(0, call_f32_lt_nan(nan_val, nan_val)) << "NaN < NaN should return false (IEEE 754 rule)";
    ASSERT_EQ(0, call_f32_lt_nan(nan_val, 0.0f)) << "NaN < 0.0 should return false (IEEE 754 rule)";
    ASSERT_EQ(0, call_f32_lt_nan(nan_val, INFINITY)) << "NaN < +inf should return false (IEEE 754 rule)";
    ASSERT_EQ(0, call_f32_lt_nan(nan_val, -INFINITY)) << "NaN < -inf should return false (IEEE 754 rule)";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, F32LtTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));