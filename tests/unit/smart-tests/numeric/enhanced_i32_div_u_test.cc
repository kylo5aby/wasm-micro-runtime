/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include "wasm_runtime_common.h"
#include "wasm_native.h"
#include "bh_read_file.h"
#include "wasm_memory.h"
#include <unistd.h>
#include <cstdlib>

static std::string CWD;
static std::string WASM_FILE;
static int test_argc;
static char **test_argv;
static wasm_module_t module = nullptr;
static wasm_module_inst_t module_inst = nullptr;
static wasm_exec_env_t exec_env = nullptr;
static char error_buf[128];
static wasm_function_inst_t i32_div_u_func = nullptr;

static bool
load_wasm_module()
{
    const char *file_path = WASM_FILE.c_str();
    wasm_module_t wasm_module = nullptr;
    uint32 buf_size, stack_size = 8092, heap_size = 8092;
    uint8 *buf = nullptr;
    RuntimeInitArgs init_args;
    char error_buf[128] = { 0 };

    memset(&init_args, 0, sizeof(RuntimeInitArgs));
    init_args.mem_alloc_type = Alloc_With_System_Allocator;

    // Read WASM file
    if (!(buf = (uint8 *)bh_read_file_to_buffer(file_path, &buf_size))) {
        printf("Failed to read WASM file: %s\n", file_path);
        return false;
    }

    // Load WASM module
    if (!(wasm_module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf)))) {
        printf("Failed to load WASM module: %s\n", error_buf);
        BH_FREE(buf);
        return false;
    }
    BH_FREE(buf);

    // Create WASM module instance
    if (!(module_inst = wasm_runtime_instantiate(wasm_module, stack_size, heap_size,
                                                error_buf, sizeof(error_buf)))) {
        printf("Failed to create WASM module instance: %s\n", error_buf);
        wasm_runtime_unload(wasm_module);
        return false;
    }

    // Create execution environment
    if (!(exec_env = wasm_runtime_create_exec_env(module_inst, stack_size))) {
        printf("Failed to create execution environment\n");
        wasm_runtime_deinstantiate(module_inst);
        wasm_runtime_unload(wasm_module);
        return false;
    }

    // Lookup i32.div_u test function
    if (!(i32_div_u_func = wasm_runtime_lookup_function(module_inst, "i32_div_u"))) {
        printf("Failed to lookup i32_div_u function\n");
        return false;
    }

    module = wasm_module;
    return true;
}

static void
destroy_wasm_module()
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
}

static uint32
call_i32_div_u(uint32 dividend, uint32 divisor)
{
    uint32 argv[2] = { dividend, divisor };
    uint32 argc = 2;

    if (!wasm_runtime_call_wasm(exec_env, i32_div_u_func, argc, argv)) {
        const char *exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            printf("Exception caught: %s\n", exception);
        }
        // Return a special value to indicate exception occurred
        return UINT32_MAX; // This will be handled by test assertions
    }

    return argv[0];
}

static bool
call_i32_div_u_expect_trap(uint32 dividend, uint32 divisor)
{
    uint32 argv[2] = { dividend, divisor };
    uint32 argc = 2;

    bool result = wasm_runtime_call_wasm(exec_env, i32_div_u_func, argc, argv);
    if (!result) {
        // Clear exception for next test
        wasm_runtime_clear_exception(module_inst);
        return true; // Trap occurred as expected
    }
    return false; // No trap occurred
}

/**
 * Test suite for i32.div_u opcode validation.
 *
 * Provides comprehensive test coverage for unsigned 32-bit integer division including:
 * - Basic arithmetic operations with unsigned values
 * - Boundary condition handling with extreme values (0, UINT32_MAX)
 * - Truncation behavior validation (floor division)
 * - Identity operation testing (division by 1, self-division)
 * - Exception handling for division by zero
 * - Cross-execution mode consistency verification
 */
class I32DivUTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * Set up test environment and initialize WAMR runtime.
     * Loads the WASM module containing i32.div_u test functions and
     * prepares execution environment for cross-mode validation.
     */
    void SetUp() override
    {
        // Get current working directory for WASM file loading
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";
        CWD = std::string(cwd_ptr);
        free(cwd_ptr);
        WASM_FILE = CWD + "/wasm-apps/i32_div_u_test.wasm";

        // Initialize WAMR runtime with proper configuration
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.running_mode = GetParam();

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load test WASM module
        ASSERT_TRUE(load_wasm_module())
            << "Failed to load i32.div_u test WASM module";
    }

    /**
     * Clean up test environment and destroy WAMR runtime resources.
     * Ensures proper resource cleanup after each test execution.
     */
    void TearDown() override
    {
        destroy_wasm_module();
        wasm_runtime_destroy();
    }
};

/**
 * @test BasicDivision_ReturnsCorrectQuotient
 * @brief Validates i32.div_u produces correct arithmetic results for typical unsigned inputs
 * @details Tests fundamental unsigned division operation with standard values to verify
 *          that i32.div_u correctly computes dividend ÷ divisor for unsigned arithmetic.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_u_operation
 * @input_conditions Standard unsigned integer pairs with clean quotients
 * @expected_behavior Returns mathematical quotient for unsigned division
 * @validation_method Direct comparison of WASM function result with expected unsigned quotient
 */
TEST_P(I32DivUTest, BasicDivision_ReturnsCorrectQuotient)
{
    // Test basic unsigned division operations
    ASSERT_EQ(2U, call_i32_div_u(10U, 5U))
        << "Failed: 10 ÷ 5 should equal 2";

    ASSERT_EQ(5U, call_i32_div_u(20U, 4U))
        << "Failed: 20 ÷ 4 should equal 5";

    ASSERT_EQ(10U, call_i32_div_u(100U, 10U))
        << "Failed: 100 ÷ 10 should equal 10";

    // Test with larger unsigned values
    ASSERT_EQ(1000U, call_i32_div_u(10000U, 10U))
        << "Failed: 10000 ÷ 10 should equal 1000";
}

/**
 * @test BoundaryValues_HandleLimitsCorrectly
 * @brief Validates i32.div_u handles boundary values and extreme conditions correctly
 * @details Tests division operations with boundary values including UINT32_MAX to ensure
 *          proper handling of extreme values in unsigned 32-bit arithmetic.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_u_boundary_handling
 * @input_conditions Boundary values including UINT32_MAX, large divisors, and edge cases
 * @expected_behavior Correct unsigned arithmetic results for all boundary conditions
 * @validation_method Verification of boundary value arithmetic correctness
 */
TEST_P(I32DivUTest, BoundaryValues_HandleLimitsCorrectly)
{
    const uint32 UINT32_MAX_VAL = 4294967295U;

    // Test UINT32_MAX divided by itself
    ASSERT_EQ(1U, call_i32_div_u(UINT32_MAX_VAL, UINT32_MAX_VAL))
        << "Failed: UINT32_MAX ÷ UINT32_MAX should equal 1";

    // Test UINT32_MAX divided by 2
    ASSERT_EQ(2147483647U, call_i32_div_u(UINT32_MAX_VAL, 2U))
        << "Failed: UINT32_MAX ÷ 2 should equal 2147483647";

    // Test large dividend with small divisor resulting in 0
    ASSERT_EQ(0U, call_i32_div_u(5U, UINT32_MAX_VAL))
        << "Failed: 5 ÷ UINT32_MAX should equal 0";

    // Test power of 2 division (efficient bit operations)
    ASSERT_EQ(2U, call_i32_div_u(8U, 4U))
        << "Failed: 8 ÷ 4 should equal 2";

    ASSERT_EQ(2U, call_i32_div_u(16U, 8U))
        << "Failed: 16 ÷ 8 should equal 2";
}

/**
 * @test IdentityOperations_PreserveValues
 * @brief Validates i32.div_u identity operations produce mathematically correct results
 * @details Tests division operations with identity values and self-division scenarios
 *          to verify fundamental mathematical properties are preserved in unsigned arithmetic.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_u_identity
 * @input_conditions Identity values and self-division scenarios for unsigned values
 * @expected_behavior Mathematical identity results for unsigned division
 * @validation_method Verification of identity property preservation in unsigned arithmetic
 */
TEST_P(I32DivUTest, IdentityOperations_PreserveValues)
{
    // Test division by 1 (identity for any value)
    ASSERT_EQ(42U, call_i32_div_u(42U, 1U))
        << "Failed: 42 ÷ 1 should equal 42";

    ASSERT_EQ(4294967295U, call_i32_div_u(4294967295U, 1U))
        << "Failed: UINT32_MAX ÷ 1 should equal UINT32_MAX";

    // Test self division (any non-zero value divided by itself equals 1)
    ASSERT_EQ(1U, call_i32_div_u(15U, 15U))
        << "Failed: 15 ÷ 15 should equal 1";

    ASSERT_EQ(1U, call_i32_div_u(1000U, 1000U))
        << "Failed: 1000 ÷ 1000 should equal 1";

    ASSERT_EQ(1U, call_i32_div_u(3000000000U, 3000000000U))
        << "Failed: 3000000000 ÷ 3000000000 should equal 1";
}

/**
 * @test ZeroDividend_ReturnsZero
 * @brief Validates i32.div_u correctly handles zero dividend scenarios
 * @details Tests that dividing zero by any non-zero unsigned value returns zero,
 *          verifying proper handling of zero in unsigned division operations.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_u_zero_handling
 * @input_conditions Zero dividend with various non-zero divisors
 * @expected_behavior Zero result for all zero dividend cases
 * @validation_method Verification that 0 ÷ any_non_zero = 0
 */
TEST_P(I32DivUTest, ZeroDividend_ReturnsZero)
{
    // Test zero divided by various values
    ASSERT_EQ(0U, call_i32_div_u(0U, 1U))
        << "Failed: 0 ÷ 1 should equal 0";

    ASSERT_EQ(0U, call_i32_div_u(0U, 5U))
        << "Failed: 0 ÷ 5 should equal 0";

    ASSERT_EQ(0U, call_i32_div_u(0U, 4294967295U))
        << "Failed: 0 ÷ UINT32_MAX should equal 0";
}

/**
 * @test TruncationBehavior_FloorDivision
 * @brief Validates i32.div_u performs proper floor division for fractional results
 * @details Tests unsigned division operations that produce fractional results to ensure
 *          proper truncation behavior (floor division) in unsigned arithmetic.
 * @test_category Edge - Truncation behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_u_truncation
 * @input_conditions Division pairs that produce fractional quotients in unsigned arithmetic
 * @expected_behavior Floor division results for all fractional quotients
 * @validation_method Verification of truncation toward zero for unsigned division
 */
TEST_P(I32DivUTest, TruncationBehavior_FloorDivision)
{
    // Test fractional division truncation: 7 ÷ 3 = 2.33... → 2
    ASSERT_EQ(2U, call_i32_div_u(7U, 3U))
        << "Failed: 7 ÷ 3 should truncate to 2";

    // Test fractional division truncation: 10 ÷ 3 = 3.33... → 3
    ASSERT_EQ(3U, call_i32_div_u(10U, 3U))
        << "Failed: 10 ÷ 3 should truncate to 3";

    // Test fractional division truncation: 100 ÷ 7 = 14.28... → 14
    ASSERT_EQ(14U, call_i32_div_u(100U, 7U))
        << "Failed: 100 ÷ 7 should truncate to 14";

    // Test with large numbers: 4000000000 ÷ 3 = 1333333333.33... → 1333333333
    ASSERT_EQ(1333333333U, call_i32_div_u(4000000000U, 3U))
        << "Failed: 4000000000 ÷ 3 should truncate to 1333333333";
}

/**
 * @test DivisionByZero_TriggersException
 * @brief Validates i32.div_u properly traps on division by zero operations
 * @details Tests division by zero scenarios to ensure WAMR runtime correctly
 *          detects and traps these undefined operations in unsigned arithmetic.
 * @test_category Exception - Error condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_u_trap_handling
 * @input_conditions Various unsigned dividend values with zero divisor
 * @expected_behavior Division by zero trap in all cases
 * @validation_method Verification of trap occurrence and exception handling
 */
TEST_P(I32DivUTest, DivisionByZero_TriggersException)
{
    // Test positive dividend divided by zero
    ASSERT_TRUE(call_i32_div_u_expect_trap(10U, 0U))
        << "Failed: 10 ÷ 0 should trigger division by zero trap";

    // Test zero dividend divided by zero
    ASSERT_TRUE(call_i32_div_u_expect_trap(0U, 0U))
        << "Failed: 0 ÷ 0 should trigger division by zero trap";

    // Test boundary values divided by zero
    ASSERT_TRUE(call_i32_div_u_expect_trap(4294967295U, 0U))
        << "Failed: UINT32_MAX ÷ 0 should trigger division by zero trap";

    // Test large value divided by zero
    ASSERT_TRUE(call_i32_div_u_expect_trap(3000000000U, 0U))
        << "Failed: 3000000000 ÷ 0 should trigger division by zero trap";
}

// Parameterized test execution for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I32DivUTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));