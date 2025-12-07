/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string WASM_FILE;
static const char *FUNC_NAME = "i64_rem_s_test";

class I64RemSTest : public testing::TestWithParam<RunningMode>
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

    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        WASM_FILE = "wasm-apps/i64_rem_s_test.wasm";

        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

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

    int64_t call_i64_rem_s(int64_t dividend, int64_t divisor)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, FUNC_NAME);
        EXPECT_NE(func, nullptr) << "Failed to lookup " << FUNC_NAME << " function";

        uint32_t argv[4];
        bool ret;

        // Pack i64 values into argv array (little endian)
        argv[0] = (uint32_t)(dividend & 0xFFFFFFFF);
        argv[1] = (uint32_t)(dividend >> 32);
        argv[2] = (uint32_t)(divisor & 0xFFFFFFFF);
        argv[3] = (uint32_t)(divisor >> 32);

        ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        // Extract i64 result from argv (little endian)
        int64_t result = ((int64_t)argv[1] << 32) | argv[0];
        return result;
    }

    bool call_i64_rem_s_expect_trap(int64_t dividend, int64_t divisor)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, FUNC_NAME);
        EXPECT_NE(func, nullptr) << "Failed to lookup " << FUNC_NAME << " function";

        uint32_t argv[4];

        // Pack i64 values into argv array (little endian)
        argv[0] = (uint32_t)(dividend & 0xFFFFFFFF);
        argv[1] = (uint32_t)(dividend >> 32);
        argv[2] = (uint32_t)(divisor & 0xFFFFFFFF);
        argv[3] = (uint32_t)(divisor >> 32);

        bool ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        return !ret; // Return true if trap occurred (call failed)
    }
};

INSTANTIATE_TEST_SUITE_P(RunningMode, I64RemSTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         testing::PrintToStringParamName());

/**
 * @test BasicRemainder_ReturnsCorrectResult
 * @brief Validates i64.rem_s produces correct signed remainder results for typical inputs
 * @details Tests fundamental signed remainder operation with various sign combinations.
 *          Verifies that i64.rem_s correctly computes a % b following C-style remainder semantics
 *          where the result sign matches the dividend sign.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_rem_s_operation
 * @input_conditions Standard i64 pairs with different sign combinations
 * @expected_behavior Returns mathematical remainder: 10%3=1, -10%3=-1, 10%-3=1, -10%-3=-1
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64RemSTest, BasicRemainder_ReturnsCorrectResult)
{
    // Positive dividend, positive divisor: 10 % 3 = 1
    ASSERT_EQ(call_i64_rem_s(10, 3), 1) << "Remainder of positive numbers failed";

    // Negative dividend, positive divisor: -10 % 3 = -1
    ASSERT_EQ(call_i64_rem_s(-10, 3), -1) << "Remainder of negative dividend failed";

    // Positive dividend, negative divisor: 10 % -3 = 1
    ASSERT_EQ(call_i64_rem_s(10, -3), 1) << "Remainder with negative divisor failed";

    // Negative dividend, negative divisor: -10 % -3 = -1
    ASSERT_EQ(call_i64_rem_s(-10, -3), -1) << "Remainder of both negative numbers failed";

    // Additional test cases
    ASSERT_EQ(call_i64_rem_s(15, 4), 3) << "Remainder 15 % 4 failed";
    ASSERT_EQ(call_i64_rem_s(-15, 4), -3) << "Remainder -15 % 4 failed";
    ASSERT_EQ(call_i64_rem_s(7, 5), 2) << "Remainder 7 % 5 failed";
}

/**
 * @test LargeNumbers_HandlesCorrectly
 * @brief Validates i64.rem_s handles large 64-bit values correctly
 * @details Tests remainder operations with large i64 values to ensure proper
 *          handling of full 64-bit range without overflow or precision issues.
 * @test_category Main - Large value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_rem_s_operation
 * @input_conditions Large i64 values near the middle range
 * @expected_behavior Correct remainder computation for large numbers
 * @validation_method Direct comparison with mathematically expected results
 */
TEST_P(I64RemSTest, LargeNumbers_HandlesCorrectly)
{
    // Large positive numbers
    ASSERT_EQ(call_i64_rem_s(1000000007LL, 1000000009LL), 1000000007LL)
        << "Large number remainder failed";

    // Large with smaller divisor
    ASSERT_EQ(call_i64_rem_s(1000000007LL, 1000LL), 7LL)
        << "Large number with small divisor failed";

    // Negative large numbers
    ASSERT_EQ(call_i64_rem_s(-1000000007LL, 1000LL), -7LL)
        << "Negative large number remainder failed";
}

/**
 * @test BoundaryValues_ProducesCorrectResults
 * @brief Validates i64.rem_s handles INT64_MIN, INT64_MAX boundary cases correctly
 * @details Tests critical boundary values including the special overflow case INT64_MIN % -1
 *          which should return 0 without causing a trap (unlike division).
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_rem_s_operation
 * @input_conditions INT64_MIN, INT64_MAX values with various divisors
 * @expected_behavior Correct remainder results, special case INT64_MIN % -1 = 0
 * @validation_method Boundary value testing with mathematically verified results
 */
TEST_P(I64RemSTest, BoundaryValues_ProducesCorrectResults)
{
    const int64_t INT64_MAX_VAL = 9223372036854775807LL;
    const int64_t INT64_MIN_VAL = (-9223372036854775807LL - 1LL);

    // INT64_MAX boundary tests
    ASSERT_EQ(call_i64_rem_s(INT64_MAX_VAL, 1), 0) << "INT64_MAX % 1 failed";
    ASSERT_EQ(call_i64_rem_s(INT64_MAX_VAL, 2), 1) << "INT64_MAX % 2 failed";
    ASSERT_EQ(call_i64_rem_s(INT64_MAX_VAL, -1), 0) << "INT64_MAX % -1 failed";

    // INT64_MIN boundary tests
    ASSERT_EQ(call_i64_rem_s(INT64_MIN_VAL, 1), 0) << "INT64_MIN % 1 failed";
    ASSERT_EQ(call_i64_rem_s(INT64_MIN_VAL, 2), 0) << "INT64_MIN % 2 failed";

    // Special overflow case: INT64_MIN % -1 = 0 (no trap)
    ASSERT_EQ(call_i64_rem_s(INT64_MIN_VAL, -1), 0) << "INT64_MIN % -1 special case failed";

    // Large divisor cases
    ASSERT_EQ(call_i64_rem_s(100, INT64_MAX_VAL), 100) << "Small number % INT64_MAX failed";
    ASSERT_EQ(call_i64_rem_s(-100, INT64_MAX_VAL), -100) << "Small negative % INT64_MAX failed";
}

/**
 * @test PowerOfTwoDivisors_OptimizedPaths
 * @brief Validates i64.rem_s works correctly with power-of-2 divisors
 * @details Tests remainder operations with powers of 2 which may have optimized
 *          implementation paths using bitwise operations.
 * @test_category Corner - Power of 2 optimization validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_rem_s_operation
 * @input_conditions Various numbers with power-of-2 divisors (2, 4, 8, 16, etc.)
 * @expected_behavior Correct remainder results matching bit pattern expectations
 * @validation_method Power of 2 divisor testing with bit-level validation
 */
TEST_P(I64RemSTest, PowerOfTwoDivisors_OptimizedPaths)
{
    // Powers of 2 divisors
    ASSERT_EQ(call_i64_rem_s(15, 8), 7) << "15 % 8 failed";
    ASSERT_EQ(call_i64_rem_s(-15, 8), -7) << "-15 % 8 failed";
    ASSERT_EQ(call_i64_rem_s(31, 16), 15) << "31 % 16 failed";
    ASSERT_EQ(call_i64_rem_s(-31, 16), -15) << "-31 % 16 failed";

    // Edge case with power of 2
    ASSERT_EQ(call_i64_rem_s(64, 64), 0) << "64 % 64 failed";
    ASSERT_EQ(call_i64_rem_s(63, 64), 63) << "63 % 64 failed";

    // Large power of 2
    ASSERT_EQ(call_i64_rem_s(1023, 1024), 1023) << "1023 % 1024 failed";
}

/**
 * @test ZeroDividend_ReturnsZero
 * @brief Validates that 0 % divisor always returns 0 for any non-zero divisor
 * @details Tests the mathematical property that zero divided by any non-zero
 *          number has zero remainder.
 * @test_category Edge - Zero dividend validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_rem_s_operation
 * @input_conditions Zero dividend with various non-zero divisors
 * @expected_behavior Always returns 0 regardless of divisor value or sign
 * @validation_method Zero dividend testing with multiple divisor values
 */
TEST_P(I64RemSTest, ZeroDividend_ReturnsZero)
{
    // 0 % positive divisor = 0
    ASSERT_EQ(call_i64_rem_s(0, 1), 0) << "0 % 1 failed";
    ASSERT_EQ(call_i64_rem_s(0, 5), 0) << "0 % 5 failed";
    ASSERT_EQ(call_i64_rem_s(0, 100), 0) << "0 % 100 failed";

    // 0 % negative divisor = 0
    ASSERT_EQ(call_i64_rem_s(0, -1), 0) << "0 % -1 failed";
    ASSERT_EQ(call_i64_rem_s(0, -5), 0) << "0 % -5 failed";
    ASSERT_EQ(call_i64_rem_s(0, -100), 0) << "0 % -100 failed";

    // 0 % large numbers = 0
    ASSERT_EQ(call_i64_rem_s(0, 9223372036854775807LL), 0) << "0 % INT64_MAX failed";
}

/**
 * @test UnitDivisors_ReturnsZero
 * @brief Validates that any_number % ±1 always returns 0
 * @details Tests the mathematical property that any integer divided by ±1
 *          has zero remainder since division is exact.
 * @test_category Edge - Unit divisor validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_rem_s_operation
 * @input_conditions Various numbers with divisors of ±1
 * @expected_behavior Always returns 0 for any dividend when divisor is ±1
 * @validation_method Unit divisor testing with multiple dividend values
 */
TEST_P(I64RemSTest, UnitDivisors_ReturnsZero)
{
    // x % 1 = 0 for any x
    ASSERT_EQ(call_i64_rem_s(42, 1), 0) << "42 % 1 failed";
    ASSERT_EQ(call_i64_rem_s(-42, 1), 0) << "-42 % 1 failed";
    ASSERT_EQ(call_i64_rem_s(9223372036854775807LL, 1), 0) << "INT64_MAX % 1 failed";

    // x % -1 = 0 for any x
    ASSERT_EQ(call_i64_rem_s(42, -1), 0) << "42 % -1 failed";
    ASSERT_EQ(call_i64_rem_s(-42, -1), 0) << "-42 % -1 failed";
    ASSERT_EQ(call_i64_rem_s((-9223372036854775807LL - 1LL), -1), 0) << "INT64_MIN % -1 failed";
}

/**
 * @test IdentityOperations_ReturnsZero
 * @brief Validates that x % x always returns 0 for any non-zero x
 * @details Tests the mathematical property that any non-zero number divided by
 *          itself has zero remainder (exact division).
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_rem_s_operation
 * @input_conditions Various non-zero numbers with themselves as divisor
 * @expected_behavior Always returns 0 when dividend equals divisor
 * @validation_method Identity operation testing with multiple values
 */
TEST_P(I64RemSTest, IdentityOperations_ReturnsZero)
{
    // x % x = 0 for any non-zero x
    ASSERT_EQ(call_i64_rem_s(5, 5), 0) << "5 % 5 failed";
    ASSERT_EQ(call_i64_rem_s(-5, -5), 0) << "-5 % -5 failed";
    ASSERT_EQ(call_i64_rem_s(100, 100), 0) << "100 % 100 failed";
    ASSERT_EQ(call_i64_rem_s(-100, -100), 0) << "-100 % -100 failed";

    // Large number identity
    ASSERT_EQ(call_i64_rem_s(1000000007LL, 1000000007LL), 0) << "Large number identity failed";

    // Boundary values identity
    ASSERT_EQ(call_i64_rem_s(9223372036854775807LL, 9223372036854775807LL), 0)
        << "INT64_MAX identity failed";
}

/**
 * @test DivisionByZero_TriggersCorrectTrap
 * @brief Validates that division by zero triggers proper WASM trap behavior
 * @details Tests that any_number % 0 causes WASM_EXCEPTION_INTEGER_DIVIDE_BY_ZERO
 *          and proper trap handling in the WASM runtime.
 * @test_category Error - Division by zero trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_rem_s_operation
 * @input_conditions Various dividends with zero divisor
 * @expected_behavior Runtime trap with integer division by zero exception
 * @validation_method Exception handling testing with trap detection
 */
TEST_P(I64RemSTest, DivisionByZero_TriggersCorrectTrap)
{
    // Test various dividends with zero divisor should all trap
    ASSERT_TRUE(call_i64_rem_s_expect_trap(1, 0)) << "1 % 0 should trap";
    ASSERT_TRUE(call_i64_rem_s_expect_trap(-1, 0)) << "-1 % 0 should trap";
    ASSERT_TRUE(call_i64_rem_s_expect_trap(100, 0)) << "100 % 0 should trap";
    ASSERT_TRUE(call_i64_rem_s_expect_trap(-100, 0)) << "-100 % 0 should trap";

    // Boundary values with zero divisor
    ASSERT_TRUE(call_i64_rem_s_expect_trap(9223372036854775807LL, 0)) << "INT64_MAX % 0 should trap";
    ASSERT_TRUE(call_i64_rem_s_expect_trap((-9223372036854775807LL - 1LL), 0)) << "INT64_MIN % 0 should trap";

    // Zero dividend with zero divisor should also trap
    ASSERT_TRUE(call_i64_rem_s_expect_trap(0, 0)) << "0 % 0 should trap";
}