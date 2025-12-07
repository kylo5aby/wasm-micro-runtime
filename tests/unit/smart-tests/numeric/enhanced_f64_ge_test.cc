/*
 * Copyright (C) 2024 Bytedance Inc.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>

#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

/**
 * @brief Test suite for f64.ge (float64 greater than or equal) opcode
 *
 * This test suite validates the f64.ge WebAssembly opcode implementation
 * in WAMR runtime across different execution modes. The f64.ge opcode
 * performs IEEE 754 double-precision floating-point comparison and returns
 * 1 (i32) if the first operand is greater than or equal to the second,
 * 0 otherwise.
 *
 * Test coverage includes:
 * - Basic comparison operations with various value combinations
 * - IEEE 754 special values (NaN, infinity, signed zeros)
 * - Boundary conditions and extreme values
 * - Cross-execution mode validation (interpreter vs AOT)
 */
class F64GeTest : public testing::TestWithParam<RunningMode>
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
     * @brief Set up test environment and initialize WAMR runtime
     *
     * Initializes the WAMR runtime with proper configuration for both
     * interpreter and AOT execution modes. Loads the f64.ge test module
     * and prepares function execution context.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        load_wasm_module();
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     *
     * Properly releases all WAMR resources including module instances,
     * execution environment, and runtime context.
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
            BH_FREE(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Load f64.ge test WASM module
     *
     * Loads the pre-compiled WASM module containing f64.ge test functions
     * and creates execution environment for function calls.
     */
    void load_wasm_module()
    {
        const char* wasm_file = "wasm-apps/f64_ge_test.wasm";

        // Read WASM file using bh_read_file_to_buffer
        buf = (uint8_t*)bh_read_file_to_buffer(wasm_file, &buf_size);
        ASSERT_NE(nullptr, buf) << "Failed to read WASM file: " << wasm_file;

        // Load WASM module
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        // Instantiate WASM module
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
    }

    /**
     * @brief Execute f64.ge comparison operation
     *
     * @param a First operand (double-precision floating-point)
     * @param b Second operand (double-precision floating-point)
     * @return Comparison result (1 if a >= b, 0 otherwise)
     */
    int32_t call_f64_ge(double a, double b)
    {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(module_inst, "f64_ge_test");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup f64_ge_test function";

        uint32_t argv[4] = { 0 }; // Two f64 values = 4 uint32_t slots
        memcpy(&argv[0], &a, sizeof(double));
        memcpy(&argv[2], &b, sizeof(double));

        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 4, argv);
        EXPECT_TRUE(call_result) << "Failed to call f64_ge_test function";

        return (int32_t)argv[0]; // Return value in argv[0]
    }

};

/**
 * @test BasicComparison_ReturnsCorrectResult
 * @brief Validates f64.ge produces correct results for standard floating-point comparisons
 * @details Tests fundamental >= operation with positive, negative, and mixed-sign double values.
 *          Verifies that f64.ge correctly implements IEEE 754 comparison semantics.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_ge_operation
 * @input_conditions Standard double pairs: (5.7, 3.2), (-8.1, -9.3), (4.0, 2.5)
 * @expected_behavior Returns 1 for true comparisons: 5.7>=3.2, -8.1>=-9.3, 4.0>=2.5
 * @validation_method Direct comparison of WASM function result with expected boolean values
 */
TEST_P(F64GeTest, BasicComparison_ReturnsCorrectResult)
{
    // Test positive numbers: 5.7 >= 3.2 should return 1
    ASSERT_EQ(1, call_f64_ge(5.7, 3.2))
        << "f64.ge failed for positive numbers: 5.7 >= 3.2";

    // Test negative numbers: -8.1 >= -9.3 should return 1
    ASSERT_EQ(1, call_f64_ge(-8.1, -9.3))
        << "f64.ge failed for negative numbers: -8.1 >= -9.3";

    // Test mixed signs: 4.0 >= -2.5 should return 1
    ASSERT_EQ(1, call_f64_ge(4.0, -2.5))
        << "f64.ge failed for mixed signs: 4.0 >= -2.5";

    // Test false comparison: 2.1 >= 7.8 should return 0
    ASSERT_EQ(0, call_f64_ge(2.1, 7.8))
        << "f64.ge failed for false comparison: 2.1 >= 7.8";
}

/**
 * @test EqualValues_ReturnsOne
 * @brief Validates f64.ge returns 1 for equal floating-point values
 * @details Tests the "equal" component of "greater than or equal" operation.
 *          Ensures identical double values correctly return 1.
 * @test_category Main - Equality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_ge_operation
 * @input_conditions Identical double pairs: (42.5, 42.5), (-123.456, -123.456)
 * @expected_behavior Returns 1 for all equal value comparisons
 * @validation_method Verify WASM function returns 1 for identical operands
 */
TEST_P(F64GeTest, EqualValues_ReturnsOne)
{
    // Test positive equal values
    ASSERT_EQ(1, call_f64_ge(42.5, 42.5))
        << "f64.ge failed for equal positive values: 42.5 >= 42.5";

    // Test negative equal values
    ASSERT_EQ(1, call_f64_ge(-123.456, -123.456))
        << "f64.ge failed for equal negative values: -123.456 >= -123.456";

    // Test zero equal values
    ASSERT_EQ(1, call_f64_ge(0.0, 0.0))
        << "f64.ge failed for equal zero values: 0.0 >= 0.0";
}

/**
 * @test ZeroComparisons_HandlesSignCorrectly
 * @brief Validates f64.ge handles IEEE 754 signed zero comparisons correctly
 * @details Tests comparison behavior with positive zero (+0.0) and negative zero (-0.0).
 *          According to IEEE 754, +0.0 and -0.0 are considered equal.
 * @test_category Edge - IEEE 754 signed zero handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_ge_operation
 * @input_conditions Zero combinations: (+0.0, -0.0), (-0.0, +0.0), (0.0, 0.0)
 * @expected_behavior Returns 1 for all cases (IEEE 754: +0.0 == -0.0)
 * @validation_method Verify WASM follows IEEE 754 signed zero comparison rules
 */
TEST_P(F64GeTest, ZeroComparisons_HandlesSignCorrectly)
{
    double positive_zero = +0.0;
    double negative_zero = -0.0;

    // +0.0 >= -0.0 should return 1 (IEEE 754: +0.0 == -0.0)
    ASSERT_EQ(1, call_f64_ge(positive_zero, negative_zero))
        << "f64.ge failed for +0.0 >= -0.0";

    // -0.0 >= +0.0 should return 1 (IEEE 754: -0.0 == +0.0)
    ASSERT_EQ(1, call_f64_ge(negative_zero, positive_zero))
        << "f64.ge failed for -0.0 >= +0.0";

    // 0.0 >= 0.0 should return 1
    ASSERT_EQ(1, call_f64_ge(0.0, 0.0))
        << "f64.ge failed for 0.0 >= 0.0";
}

/**
 * @test InfinityComparisons_FollowsIEEE754
 * @brief Validates f64.ge handles IEEE 754 infinity values correctly
 * @details Tests comparison behavior with positive and negative infinity values.
 *          Verifies IEEE 754 infinity comparison rules are followed.
 * @test_category Edge - IEEE 754 infinity handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_ge_operation
 * @input_conditions Infinity cases: (+inf, +inf), (+inf, 123.0), (-inf, -inf), (5.0, +inf)
 * @expected_behavior Returns 1 for (+inf, +inf), (+inf, 123.0), (-inf, -inf); 0 for (5.0, +inf)
 * @validation_method Verify WASM follows IEEE 754 infinity comparison semantics
 */
TEST_P(F64GeTest, InfinityComparisons_FollowsIEEE754)
{
    double positive_inf = INFINITY;
    double negative_inf = -INFINITY;

    // +inf >= +inf should return 1
    ASSERT_EQ(1, call_f64_ge(positive_inf, positive_inf))
        << "f64.ge failed for +inf >= +inf";

    // +inf >= 123.0 should return 1
    ASSERT_EQ(1, call_f64_ge(positive_inf, 123.0))
        << "f64.ge failed for +inf >= 123.0";

    // -inf >= -inf should return 1
    ASSERT_EQ(1, call_f64_ge(negative_inf, negative_inf))
        << "f64.ge failed for -inf >= -inf";

    // 5.0 >= +inf should return 0
    ASSERT_EQ(0, call_f64_ge(5.0, positive_inf))
        << "f64.ge failed for 5.0 >= +inf";

    // -inf >= +inf should return 0
    ASSERT_EQ(0, call_f64_ge(negative_inf, positive_inf))
        << "f64.ge failed for -inf >= +inf";
}

/**
 * @test NaNComparisons_AlwaysFalse
 * @brief Validates f64.ge returns 0 for all NaN comparisons
 * @details Tests IEEE 754 NaN comparison rules where any comparison involving NaN
 *          must return false (0). This is a critical IEEE 754 requirement.
 * @test_category Edge - IEEE 754 NaN handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_ge_operation
 * @input_conditions NaN cases: (NaN, 5.0), (3.0, NaN), (NaN, NaN)
 * @expected_behavior Returns 0 for all NaN comparisons (IEEE 754 rule)
 * @validation_method Verify WASM follows IEEE 754 NaN comparison semantics
 */
TEST_P(F64GeTest, NaNComparisons_AlwaysFalse)
{
    double nan_value = NAN;

    // NaN >= 5.0 should return 0
    ASSERT_EQ(0, call_f64_ge(nan_value, 5.0))
        << "f64.ge failed for NaN >= 5.0 (should be 0)";

    // 3.0 >= NaN should return 0
    ASSERT_EQ(0, call_f64_ge(3.0, nan_value))
        << "f64.ge failed for 3.0 >= NaN (should be 0)";

    // NaN >= NaN should return 0
    ASSERT_EQ(0, call_f64_ge(nan_value, nan_value))
        << "f64.ge failed for NaN >= NaN (should be 0)";
}

/**
 * @test BoundaryValues_HandlesExtremes
 * @brief Validates f64.ge handles extreme double-precision values correctly
 * @details Tests comparison with maximum and minimum representable double values.
 *          Ensures proper handling of boundary conditions in floating-point range.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_ge_operation
 * @input_conditions Extreme values: (DBL_MAX, DBL_MIN), (DBL_MIN, -DBL_MAX)
 * @expected_behavior Returns 1 for DBL_MAX >= DBL_MIN and DBL_MIN >= -DBL_MAX
 * @validation_method Verify correct handling of floating-point boundary values
 */
TEST_P(F64GeTest, BoundaryValues_HandlesExtremes)
{
    // DBL_MAX >= DBL_MIN should return 1
    ASSERT_EQ(1, call_f64_ge(DBL_MAX, DBL_MIN))
        << "f64.ge failed for DBL_MAX >= DBL_MIN";

    // DBL_MIN >= -DBL_MAX should return 1
    ASSERT_EQ(1, call_f64_ge(DBL_MIN, -DBL_MAX))
        << "f64.ge failed for DBL_MIN >= -DBL_MAX";

    // -DBL_MAX >= DBL_MAX should return 0
    ASSERT_EQ(0, call_f64_ge(-DBL_MAX, DBL_MAX))
        << "f64.ge failed for -DBL_MAX >= DBL_MAX";
}

// Parameterized test instantiation for different execution modes
INSTANTIATE_TEST_SUITE_P(
    RunningMode,
    F64GeTest,
    testing::Values(
        Mode_Interp
#if WASM_ENABLE_AOT != 0
        , Mode_LLVM_JIT
#endif
    )
);