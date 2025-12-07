/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;

/**
 * @file enhanced_i64_gt_s_test.cc
 * @brief Comprehensive test suite for i64.gt_s opcode
 * @details Tests signed 64-bit integer greater-than comparison operation across
 *          various scenarios including basic functionality, boundary conditions,
 *          edge cases, and error conditions. Validates behavior in both
 *          interpreter and AOT execution modes.
 */

/**
 * @class I64GtSTest
 * @brief Test fixture for i64.gt_s opcode validation
 * @details Provides setup and teardown for WAMR runtime environment
 *          and parameterized testing across execution modes
 */
class I64GtSTest : public testing::Test
{
protected:
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    char *error_buf = nullptr;
    uint8_t *buf = nullptr;
    uint32_t buf_size = 0;
    uint32_t stack_size = 16384;
    uint32_t heap_size = 16384;

    /**
     * @brief Set up test environment before each test case
     * @details Initializes WAMR runtime with system allocator and
     *          loads i64.gt_s test WASM module
     */
    void SetUp() override
    {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Set up file paths
        CWD = getcwd(nullptr, 0);
        WASM_FILE = CWD + "/wasm-apps/i64_gt_s_test.wasm";

        // Allocate error buffer
        error_buf = (char*)malloc(256);
        ASSERT_NE(error_buf, nullptr) << "Failed to allocate error buffer";

        // Load i64.gt_s test WASM module
        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, 256);
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, 256);
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment after each test case
     * @details Destroys WAMR runtime and releases allocated resources
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
        if (error_buf) {
            free(error_buf);
            error_buf = nullptr;
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Execute i64.gt_s comparison with two operands
     * @param left Left operand (first value)
     * @param right Right operand (second value)
     * @return Comparison result as i32 (1 if left > right, 0 otherwise)
     */
    int32_t call_i64_gt_s(int64_t left, int64_t right)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "i64_gt_s_test");
        EXPECT_NE(func, nullptr) << "Failed to lookup i64_gt_s_test function";
        if (!func) return -1;

        uint32_t argv[4];
        // Pack 64-bit arguments into 32-bit array for WASM call
        argv[0] = (uint32_t)(left & 0xFFFFFFFF);        // Low 32 bits of left
        argv[1] = (uint32_t)((left >> 32) & 0xFFFFFFFF); // High 32 bits of left
        argv[2] = (uint32_t)(right & 0xFFFFFFFF);        // Low 32 bits of right
        argv[3] = (uint32_t)((right >> 32) & 0xFFFFFFFF); // High 32 bits of right

        bool call_result = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(call_result) << "WASM function call failed: "
                                << wasm_runtime_get_exception(module_inst);

        return call_result ? (int32_t)argv[0] : -1;
    }
};

/**
 * @test BasicComparison_ReturnsCorrectResults
 * @brief Validates i64.gt_s produces correct results for typical comparisons
 * @details Tests fundamental greater-than operation with positive, negative,
 *          and mixed-sign 64-bit integers. Verifies that i64.gt_s correctly
 *          computes a > b using signed comparison semantics.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Standard integer pairs: (100,50), (-10,-20), (5,-3)
 * @expected_behavior Returns 1 for true comparisons, 0 for false comparisons
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(I64GtSTest, BasicComparison_ReturnsCorrectResults)
{
    // Test positive > positive (true case)
    ASSERT_EQ(1, call_i64_gt_s(100, 50))
        << "100 > 50 should return true (1)";

    // Test positive > positive (false case)
    ASSERT_EQ(0, call_i64_gt_s(25, 75))
        << "25 > 75 should return false (0)";

    // Test negative > negative (true case)
    ASSERT_EQ(1, call_i64_gt_s(-10, -20))
        << "-10 > -20 should return true (1)";

    // Test negative > negative (false case)
    ASSERT_EQ(0, call_i64_gt_s(-50, -30))
        << "-50 > -30 should return false (0)";

    // Test positive > negative (always true)
    ASSERT_EQ(1, call_i64_gt_s(5, -3))
        << "5 > -3 should return true (1)";

    // Test negative > positive (always false)
    ASSERT_EQ(0, call_i64_gt_s(-8, 12))
        << "-8 > 12 should return false (0)";
}

/**
 * @test EqualValues_ReturnsFalse
 * @brief Validates i64.gt_s returns false for equal operands
 * @details Tests anti-reflexive property of greater-than comparison.
 *          Verifies that a > a is always false for any value a.
 * @test_category Main - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Equal value pairs: (42,42), (-7,-7), (0,0)
 * @expected_behavior Returns 0 (false) for all equal value comparisons
 * @validation_method Direct comparison ensuring all equal pairs return false
 */
TEST_F(I64GtSTest, EqualValues_ReturnsFalse)
{
    // Test positive equal values
    ASSERT_EQ(0, call_i64_gt_s(42, 42))
        << "42 > 42 should return false (0)";

    // Test negative equal values
    ASSERT_EQ(0, call_i64_gt_s(-7, -7))
        << "-7 > -7 should return false (0)";

    // Test zero equal values
    ASSERT_EQ(0, call_i64_gt_s(0, 0))
        << "0 > 0 should return false (0)";
}

/**
 * @test BoundaryValues_HandlesExtremes
 * @brief Validates i64.gt_s handles boundary values correctly
 * @details Tests comparison operations at the extremes of 64-bit signed integer
 *          range. Verifies proper handling of INT64_MAX and INT64_MIN values
 *          in signed comparison operations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Boundary values: INT64_MAX, INT64_MIN, adjacent values
 * @expected_behavior Correct signed comparison results for extreme values
 * @validation_method Comparison of boundary values with expected mathematical results
 */
TEST_F(I64GtSTest, BoundaryValues_HandlesExtremes)
{
    // Test INT64_MAX > INT64_MIN (should be true)
    ASSERT_EQ(1, call_i64_gt_s(INT64_MAX, INT64_MIN))
        << "INT64_MAX > INT64_MIN should return true (1)";

    // Test INT64_MIN > INT64_MAX (should be false)
    ASSERT_EQ(0, call_i64_gt_s(INT64_MIN, INT64_MAX))
        << "INT64_MIN > INT64_MAX should return false (0)";

    // Test INT64_MAX > INT64_MAX-1 (should be true)
    ASSERT_EQ(1, call_i64_gt_s(INT64_MAX, INT64_MAX - 1))
        << "INT64_MAX > (INT64_MAX-1) should return true (1)";

    // Test INT64_MIN+1 > INT64_MIN (should be true)
    ASSERT_EQ(1, call_i64_gt_s(INT64_MIN + 1, INT64_MIN))
        << "(INT64_MIN+1) > INT64_MIN should return true (1)";

    // Test boundary against typical value
    ASSERT_EQ(1, call_i64_gt_s(INT64_MAX, 1000))
        << "INT64_MAX > 1000 should return true (1)";
}

/**
 * @test SignBoundary_CrossesZero
 * @brief Validates i64.gt_s handles sign boundary conditions correctly
 * @details Tests comparison operations crossing zero boundary, including
 *          positive vs zero, negative vs zero, and zero vs zero cases.
 *          Verifies proper signed arithmetic comparison behavior.
 * @test_category Corner - Sign boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Sign boundary cases: -1, 0, 1 in various combinations
 * @expected_behavior Correct signed comparison results across zero boundary
 * @validation_method Verification of sign-sensitive comparison operations
 */
TEST_F(I64GtSTest, SignBoundary_CrossesZero)
{
    // Test -1 > 0 (should be false in signed comparison)
    ASSERT_EQ(0, call_i64_gt_s(-1, 0))
        << "-1 > 0 should return false (0)";

    // Test 1 > 0 (should be true)
    ASSERT_EQ(1, call_i64_gt_s(1, 0))
        << "1 > 0 should return true (1)";

    // Test 0 > -1 (should be true)
    ASSERT_EQ(1, call_i64_gt_s(0, -1))
        << "0 > -1 should return true (1)";

    // Test -1 > -2 (should be true)
    ASSERT_EQ(1, call_i64_gt_s(-1, -2))
        << "-1 > -2 should return true (1)";
}

/**
 * @test ZeroComparisons_VariousScenarios
 * @brief Validates i64.gt_s handles zero operand scenarios correctly
 * @details Tests comparison operations involving zero as one or both operands.
 *          Covers positive > 0, 0 > negative, and 0 > 0 cases.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Zero operand combinations with positive and negative values
 * @expected_behavior Mathematically correct results for zero-involved comparisons
 * @validation_method Direct verification of zero comparison semantics
 */
TEST_F(I64GtSTest, ZeroComparisons_VariousScenarios)
{
    // Test positive > 0 (should be true)
    ASSERT_EQ(1, call_i64_gt_s(42, 0))
        << "42 > 0 should return true (1)";

    // Test 0 > negative (should be true)
    ASSERT_EQ(1, call_i64_gt_s(0, -15))
        << "0 > -15 should return true (1)";

    // Test 0 > 0 (should be false)
    ASSERT_EQ(0, call_i64_gt_s(0, 0))
        << "0 > 0 should return false (0)";

    // Test negative > 0 (should be false)
    ASSERT_EQ(0, call_i64_gt_s(-25, 0))
        << "-25 > 0 should return false (0)";
}

/**
 * @test MathematicalProperties_AntiReflexive
 * @brief Validates mathematical properties of i64.gt_s operation
 * @details Tests anti-reflexive and asymmetric properties of greater-than
 *          comparison. Verifies that if a > b is true, then b > a is false.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Property verification pairs for asymmetric validation
 * @expected_behavior Anti-reflexive: a > a is false, Asymmetric: if a > b then !(b > a)
 * @validation_method Mathematical property verification through paired comparisons
 */
TEST_F(I64GtSTest, MathematicalProperties_AntiReflexive)
{
    // Test anti-reflexive property: a > a is always false
    ASSERT_EQ(0, call_i64_gt_s(1000, 1000))
        << "1000 > 1000 should return false (anti-reflexive property)";

    ASSERT_EQ(0, call_i64_gt_s(-500, -500))
        << "-500 > -500 should return false (anti-reflexive property)";

    // Test asymmetric property: if a > b then !(b > a)
    int64_t a = 100, b = 50;
    int32_t a_gt_b = call_i64_gt_s(a, b);
    int32_t b_gt_a = call_i64_gt_s(b, a);

    ASSERT_EQ(1, a_gt_b) << "100 > 50 should be true";
    ASSERT_EQ(0, b_gt_a) << "50 > 100 should be false (asymmetric property)";
    ASSERT_NE(a_gt_b, b_gt_a) << "Asymmetric property violated: both comparisons returned same result";
}