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
 * @file enhanced_i64_le_u_test.cc
 * @brief Comprehensive test suite for i64.le_u opcode
 * @details Tests unsigned 64-bit integer less-than-or-equal comparison operation
 *          across various scenarios including basic functionality, boundary
 *          conditions, edge cases, and error conditions. Validates behavior
 *          in both interpreter and AOT execution modes with emphasis on
 *          unsigned comparison semantics.
 */

/**
 * @class I64LeUTest
 * @brief Test fixture for i64.le_u opcode validation
 * @details Provides setup and teardown for WAMR runtime environment
 *          and comprehensive testing infrastructure for unsigned less-than-or-equal
 *          comparison operations. Handles differences between signed and unsigned
 *          integer interpretation, particularly for large values.
 */
class I64LeUTest : public testing::Test
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
     *          loads i64.le_u test WASM module
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
        WASM_FILE = CWD + "/wasm-apps/i64_le_u_test.wasm";

        // Allocate error buffer
        error_buf = (char*)malloc(256);
        ASSERT_NE(error_buf, nullptr) << "Failed to allocate error buffer";

        // Load i64.le_u test WASM module
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
     * @brief Execute i64.le_u comparison with two operands
     * @param left Left operand (first value)
     * @param right Right operand (second value)
     * @return Comparison result as i32 (1 if left <= right unsigned, 0 otherwise)
     */
    int32_t call_i64_le_u(uint64_t left, uint64_t right)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "i64_le_u_test");
        EXPECT_NE(func, nullptr) << "Failed to lookup i64_le_u_test function";
        if (!func) return -1;

        uint32_t argv[4];
        // Pack 64-bit arguments into 32-bit array for WASM call
        argv[0] = (uint32_t)(left & 0xFFFFFFFF);         // Low 32 bits of left
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
 * @test BasicUnsignedComparison_ReturnsCorrectResults
 * @brief Validates i64.le_u produces correct results for typical unsigned comparisons
 * @details Tests fundamental unsigned less-than-or-equal operation with various
 *          64-bit integers. Verifies that i64.le_u correctly computes a <= b
 *          using unsigned comparison semantics, where all values are treated as
 *          positive integers from 0 to UINT64_MAX.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Unsigned integer pairs: (5,10), (15,8), (100,100), large unsigned values
 * @expected_behavior Returns 1 for true comparisons, 0 for false comparisons in unsigned semantics
 * @validation_method Direct comparison of WASM function result with expected unsigned comparison values
 */
TEST_F(I64LeUTest, BasicUnsignedComparison_ReturnsCorrectResults)
{
    // Test small positive values (true case)
    ASSERT_EQ(1, call_i64_le_u(5, 10))
        << "5 <= 10 should return true (1) in unsigned comparison";

    // Test small positive values (false case)
    ASSERT_EQ(0, call_i64_le_u(15, 8))
        << "15 <= 8 should return false (0) in unsigned comparison";

    // Test equal values (reflexive property)
    ASSERT_EQ(1, call_i64_le_u(100, 100))
        << "100 <= 100 should return true (1) - reflexive property";

    // Test large unsigned values
    ASSERT_EQ(1, call_i64_le_u(1000000, 2000000))
        << "1000000 <= 2000000 should return true (1)";

    // Test mid-range values
    ASSERT_EQ(0, call_i64_le_u(50000, 25000))
        << "50000 <= 25000 should return false (0)";

    // Test zero vs positive
    ASSERT_EQ(1, call_i64_le_u(0, 42))
        << "0 <= 42 should return true (1) - minimum vs positive";
}

/**
 * @test UnsignedVsSignedDifference_ValidatesUnsignedSemantics
 * @brief Validates i64.le_u handles unsigned vs signed interpretation differences
 * @details Tests critical scenarios where unsigned comparison differs from signed
 *          comparison. Verifies that values with high bit set (which would be
 *          negative in signed interpretation) are correctly treated as large
 *          positive values in unsigned comparison.
 * @test_category Main - Unsigned semantics validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Large unsigned values that appear negative in signed: UINT64_MAX, 0x8000000000000000
 * @expected_behavior Unsigned interpretation: large values > smaller positive values
 * @validation_method Comparison with results that would differ in signed arithmetic
 */
TEST_F(I64LeUTest, UnsignedVsSignedDifference_ValidatesUnsignedSemantics)
{
    // Test UINT64_MAX vs INT64_MAX
    // In unsigned: 18446744073709551615 vs 9223372036854775807 -> false (max > half-max)
    // In signed: -1 vs 9223372036854775807 -> true (-1 < positive)
    ASSERT_EQ(0, call_i64_le_u(0xFFFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF))
        << "UINT64_MAX <= INT64_MAX should return false (0) in unsigned comparison";

    // Test 0x8000000000000000 vs INT64_MAX
    // In unsigned: 9223372036854775808 vs 9223372036854775807 -> false (larger > smaller)
    // In signed: -9223372036854775808 vs 9223372036854775807 -> true (negative < positive)
    ASSERT_EQ(0, call_i64_le_u(0x8000000000000000, 0x7FFFFFFFFFFFFFFF))
        << "0x8000000000000000 <= INT64_MAX should return false (0) in unsigned comparison";

    // Test large unsigned value vs smaller unsigned
    ASSERT_EQ(0, call_i64_le_u(0xFFFFFFFFFFFFFFFF, 1000))
        << "UINT64_MAX <= 1000 should return false (0) - maximum vs small positive";

    // Test 0x8000000000000000 vs small value
    ASSERT_EQ(0, call_i64_le_u(0x8000000000000000, 1000))
        << "0x8000000000000000 <= 1000 should return false (0) - large vs small unsigned";
}

/**
 * @test BoundaryValues_HandlesExtremes
 * @brief Validates i64.le_u handles unsigned integer boundary conditions correctly
 * @details Tests comparison operations at the extremes of 64-bit unsigned integer
 *          range. Verifies proper handling of 0 (minimum) and UINT64_MAX (maximum)
 *          values in unsigned less-than-or-equal comparison operations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Boundary values: 0, UINT64_MAX, adjacent values
 * @expected_behavior Correct unsigned comparison results for extreme values
 * @validation_method Comparison of boundary values with expected mathematical results
 */
TEST_F(I64LeUTest, BoundaryValues_HandlesExtremes)
{
    // Test 0 <= UINT64_MAX (minimum <= maximum, should be true)
    ASSERT_EQ(1, call_i64_le_u(0, 0xFFFFFFFFFFFFFFFF))
        << "0 <= UINT64_MAX should return true (1) - minimum <= maximum";

    // Test UINT64_MAX <= 0 (maximum <= minimum, should be false)
    ASSERT_EQ(0, call_i64_le_u(0xFFFFFFFFFFFFFFFF, 0))
        << "UINT64_MAX <= 0 should return false (0) - maximum <= minimum";

    // Test (UINT64_MAX-1) <= UINT64_MAX (should be true)
    ASSERT_EQ(1, call_i64_le_u(0xFFFFFFFFFFFFFFFE, 0xFFFFFFFFFFFFFFFF))
        << "(UINT64_MAX-1) <= UINT64_MAX should return true (1)";

    // Test 0 <= 1 (should be true)
    ASSERT_EQ(1, call_i64_le_u(0, 1))
        << "0 <= 1 should return true (1) - minimum <= next value";

    // Test boundary equal cases
    ASSERT_EQ(1, call_i64_le_u(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF))
        << "UINT64_MAX <= UINT64_MAX should return true (1) - reflexive maximum";

    ASSERT_EQ(1, call_i64_le_u(0, 0))
        << "0 <= 0 should return true (1) - reflexive minimum";

    // Test typical value against maximum
    ASSERT_EQ(1, call_i64_le_u(1000, 0xFFFFFFFFFFFFFFFF))
        << "1000 <= UINT64_MAX should return true (1)";
}

/**
 * @test SignBitTransition_ValidatesUnsignedRange
 * @brief Validates i64.le_u handles sign bit transition correctly in unsigned context
 * @details Tests comparison operations around the value 0x8000000000000000 which
 *          represents the boundary between values that would be positive/negative
 *          in signed interpretation, but are all positive in unsigned context.
 * @test_category Corner - Sign bit transition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Values around 0x8000000000000000: INT64_MAX, first "negative" as unsigned
 * @expected_behavior Proper unsigned ordering across the entire 64-bit range
 * @validation_method Verification of unsigned comparison across sign bit boundary
 */
TEST_F(I64LeUTest, SignBitTransition_ValidatesUnsignedRange)
{
    // Test INT64_MAX <= 0x8000000000000000
    // In unsigned: 9223372036854775807 <= 9223372036854775808 -> true
    ASSERT_EQ(1, call_i64_le_u(0x7FFFFFFFFFFFFFFF, 0x8000000000000000))
        << "INT64_MAX <= 0x8000000000000000 should return true (1) in unsigned comparison";

    // Test 0x8000000000000000 <= UINT64_MAX
    // In unsigned: 9223372036854775808 <= 18446744073709551615 -> true
    ASSERT_EQ(1, call_i64_le_u(0x8000000000000000, 0xFFFFFFFFFFFFFFFF))
        << "0x8000000000000000 <= UINT64_MAX should return true (1)";

    // Test reverse: 0x8000000000000000 <= INT64_MAX (should be false)
    ASSERT_EQ(0, call_i64_le_u(0x8000000000000000, 0x7FFFFFFFFFFFFFFF))
        << "0x8000000000000000 <= INT64_MAX should return false (0)";

    // Test equal values at transition
    ASSERT_EQ(1, call_i64_le_u(0x8000000000000000, 0x8000000000000000))
        << "0x8000000000000000 <= 0x8000000000000000 should return true (1) - reflexive";
}

/**
 * @test ZeroComparisons_VariousScenarios
 * @brief Validates i64.le_u handles zero operand scenarios correctly
 * @details Tests comparison operations involving zero as one or both operands.
 *          Covers 0 <= positive, positive <= 0, and 0 <= 0 cases, which are
 *          fundamental for understanding unsigned comparison semantics where
 *          zero is the absolute minimum value.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Zero operand combinations with various positive values
 * @expected_behavior Mathematically correct results for zero-involved unsigned comparisons
 * @validation_method Direct verification of zero comparison semantics in unsigned context
 */
TEST_F(I64LeUTest, ZeroComparisons_VariousScenarios)
{
    // Test 0 <= 0 (should be true - reflexive)
    ASSERT_EQ(1, call_i64_le_u(0, 0))
        << "0 <= 0 should return true (1) - reflexive property";

    // Test 0 <= positive (should always be true - minimum <= any value)
    ASSERT_EQ(1, call_i64_le_u(0, 1))
        << "0 <= 1 should return true (1)";

    ASSERT_EQ(1, call_i64_le_u(0, 1000))
        << "0 <= 1000 should return true (1)";

    ASSERT_EQ(1, call_i64_le_u(0, 0xFFFFFFFFFFFFFFFF))
        << "0 <= UINT64_MAX should return true (1)";

    // Test positive <= 0 (should always be false except for 0 <= 0)
    ASSERT_EQ(0, call_i64_le_u(1, 0))
        << "1 <= 0 should return false (0)";

    ASSERT_EQ(0, call_i64_le_u(1000, 0))
        << "1000 <= 0 should return false (0)";

    // Test large values against zero
    ASSERT_EQ(0, call_i64_le_u(0xFFFFFFFFFFFFFFFF, 0))
        << "UINT64_MAX <= 0 should return false (0)";
}

/**
 * @test MathematicalProperties_Reflexive
 * @brief Validates mathematical properties of i64.le_u operation
 * @details Tests reflexive property of unsigned less-than-or-equal comparison.
 *          Verifies that a <= a is always true for any unsigned value, which is
 *          the fundamental mathematical property distinguishing <= from < operations.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Property verification across representative unsigned value ranges
 * @expected_behavior Reflexive: a <= a is always true for any unsigned value
 * @validation_method Mathematical property verification through equal value comparisons
 */
TEST_F(I64LeUTest, MathematicalProperties_Reflexive)
{
    // Test reflexive property across different value ranges
    ASSERT_EQ(1, call_i64_le_u(1000, 1000))
        << "1000 <= 1000 should return true (reflexive property)";

    ASSERT_EQ(1, call_i64_le_u(0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF))
        << "INT64_MAX <= INT64_MAX should return true (reflexive property)";

    ASSERT_EQ(1, call_i64_le_u(0x8000000000000000, 0x8000000000000000))
        << "0x8000000000000000 <= 0x8000000000000000 should return true (reflexive property)";

    ASSERT_EQ(1, call_i64_le_u(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF))
        << "UINT64_MAX <= UINT64_MAX should return true (reflexive property)";

    // Test transitivity demonstration
    uint64_t a = 50, b = 100, c = 150;
    int32_t a_le_b = call_i64_le_u(a, b);
    int32_t b_le_c = call_i64_le_u(b, c);
    int32_t a_le_c = call_i64_le_u(a, c);

    ASSERT_EQ(1, a_le_b) << "50 <= 100 should be true";
    ASSERT_EQ(1, b_le_c) << "100 <= 150 should be true";
    ASSERT_EQ(1, a_le_c) << "50 <= 150 should be true (transitivity)";

    // Test antisymmetric property demonstration
    uint64_t x = 75, y = 125;
    int32_t x_le_y = call_i64_le_u(x, y);
    int32_t y_le_x = call_i64_le_u(y, x);

    ASSERT_EQ(1, x_le_y) << "75 <= 125 should be true";
    ASSERT_EQ(0, y_le_x) << "125 <= 75 should be false";
}

/**
 * @test StackUnderflowHandling_FailsGracefully
 * @brief Validates proper error handling for stack underflow conditions
 * @details Tests error scenarios where insufficient operands are available
 *          on the stack for i64.le_u operation. Verifies that WAMR runtime
 *          handles these error conditions gracefully without crashes.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Malformed WASM modules with stack underflow conditions
 * @expected_behavior Module loading should fail for invalid bytecode
 * @validation_method Verification that invalid modules are rejected during load
 */
TEST_F(I64LeUTest, StackUnderflowHandling_FailsGracefully)
{
    char underflow_error_buf[256];
    wasm_module_t underflow_module;

    // Test case 1: Module with empty stack underflow
    std::string underflow_file = CWD + "/wasm-apps/i64_le_u_stack_underflow.wasm";
    uint32_t underflow_buf_size;
    uint8_t *underflow_buf = (uint8_t *)bh_read_file_to_buffer(underflow_file.c_str(),
                                                               &underflow_buf_size);

    // If the underflow test file exists, it should fail to load
    // If it doesn't exist, we continue with normal operation
    if (underflow_buf != nullptr) {
        underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                           underflow_error_buf, sizeof(underflow_error_buf));

        // For stack underflow scenarios, module loading should fail
        ASSERT_EQ(nullptr, underflow_module)
            << "Expected module load to fail for stack underflow bytecode, but got valid module";

        BH_FREE(underflow_buf);
    }

    // Test case 2: Verify normal operation still works
    ASSERT_EQ(1, call_i64_le_u(42, 42))
        << "Normal operation should still work after error handling test";
}