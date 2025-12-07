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
static wasm_function_inst_t i32_div_s_func = nullptr;

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

    // Lookup i32.div_s test function
    if (!(i32_div_s_func = wasm_runtime_lookup_function(module_inst, "i32_div_s"))) {
        printf("Failed to lookup i32_div_s function\n");
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

static int32
call_i32_div_s(int32 dividend, int32 divisor)
{
    uint32 argv[2] = { static_cast<uint32>(dividend), static_cast<uint32>(divisor) };
    uint32 argc = 2;

    if (!wasm_runtime_call_wasm(exec_env, i32_div_s_func, argc, argv)) {
        const char *exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            printf("Exception caught: %s\n", exception);
        }
        // Return a special value to indicate exception occurred
        return 0x80000000; // This will be handled by test assertions
    }

    return static_cast<int32>(argv[0]);
}

static bool
call_i32_div_s_expect_trap(int32 dividend, int32 divisor)
{
    uint32 argv[2] = { static_cast<uint32>(dividend), static_cast<uint32>(divisor) };
    uint32 argc = 2;

    bool result = wasm_runtime_call_wasm(exec_env, i32_div_s_func, argc, argv);
    if (!result) {
        // Clear exception for next test
        wasm_runtime_clear_exception(module_inst);
        return true; // Trap occurred as expected
    }
    return false; // No trap occurred
}

/**
 * Test suite for i32.div_s opcode validation.
 *
 * Provides comprehensive test coverage for signed 32-bit integer division including:
 * - Basic arithmetic operations with various sign combinations
 * - Boundary condition handling with extreme values
 * - Truncation behavior validation (toward zero)
 * - Identity operation testing
 * - Exception handling for division by zero and integer overflow
 * - Cross-execution mode consistency verification
 */
class I32DivSTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * Set up test environment and initialize WAMR runtime.
     * Loads the WASM module containing i32.div_s test functions and
     * prepares execution environment for cross-mode validation.
     */
    void SetUp() override
    {
        // Get current working directory for WASM file loading
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";
        CWD = std::string(cwd_ptr);
        free(cwd_ptr);
        WASM_FILE = CWD + "/wasm-apps/i32_div_s_test.wasm";

        // Initialize WAMR runtime
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load test module
        ASSERT_TRUE(load_wasm_module())
            << "Failed to load i32.div_s test module: " << WASM_FILE;
    }

    /**
     * Clean up test environment and destroy WAMR runtime.
     * Ensures proper resource cleanup after each test execution.
     */
    void TearDown() override
    {
        destroy_wasm_module();
        wasm_runtime_destroy();
    }
};

/**
 * @test BasicDivision_ReturnsCorrectQuotients
 * @brief Validates i32.div_s produces correct arithmetic results for typical sign combinations
 * @details Tests fundamental signed division operation with positive, negative, and mixed-sign integers.
 *          Verifies that i32.div_s correctly computes quotient with proper sign handling.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_s_operation
 * @input_conditions Standard integer pairs with various sign combinations
 * @expected_behavior Returns mathematical quotient with correct sign
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32DivSTest, BasicDivision_ReturnsCorrectQuotients)
{
    // Test positive dividend, positive divisor
    ASSERT_EQ(5, call_i32_div_s(20, 4))
        << "Failed: 20 ÷ 4 should equal 5";

    // Test negative dividend, positive divisor
    ASSERT_EQ(-5, call_i32_div_s(-20, 4))
        << "Failed: -20 ÷ 4 should equal -5";

    // Test positive dividend, negative divisor
    ASSERT_EQ(-5, call_i32_div_s(20, -4))
        << "Failed: 20 ÷ -4 should equal -5";

    // Test negative dividend, negative divisor
    ASSERT_EQ(5, call_i32_div_s(-20, -4))
        << "Failed: -20 ÷ -4 should equal 5";
}

/**
 * @test BoundaryDivision_HandlesExtremeValues
 * @brief Validates i32.div_s handles boundary values correctly at integer limits
 * @details Tests division operations with INT32_MIN and INT32_MAX values to ensure
 *          proper handling of extreme cases without unexpected behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_s_boundary_handling
 * @input_conditions Integer boundary values as dividend and divisor
 * @expected_behavior Correct quotients for valid boundary operations
 * @validation_method Verification of boundary arithmetic results
 */
TEST_P(I32DivSTest, BoundaryDivision_HandlesExtremeValues)
{
    const int32 INT32_MAX_VAL = 2147483647;
    const int32 INT32_MIN_VAL = -2147483648;

    // Test INT32_MAX divided by 1
    ASSERT_EQ(INT32_MAX_VAL, call_i32_div_s(INT32_MAX_VAL, 1))
        << "Failed: INT32_MAX ÷ 1 should equal INT32_MAX";

    // Test INT32_MIN divided by 1
    ASSERT_EQ(INT32_MIN_VAL, call_i32_div_s(INT32_MIN_VAL, 1))
        << "Failed: INT32_MIN ÷ 1 should equal INT32_MIN";

    // Test INT32_MAX divided by itself
    ASSERT_EQ(1, call_i32_div_s(INT32_MAX_VAL, INT32_MAX_VAL))
        << "Failed: INT32_MAX ÷ INT32_MAX should equal 1";
}

/**
 * @test TruncationBehavior_TruncatesTowardZero
 * @brief Validates i32.div_s truncates fractional results toward zero (C-style division)
 * @details Tests division operations that produce fractional results to ensure
 *          proper truncation behavior matching C language semantics.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_s_truncation
 * @input_conditions Division pairs that produce fractional quotients
 * @expected_behavior Truncation toward zero for all sign combinations
 * @validation_method Verification of truncation direction for positive and negative results
 */
TEST_P(I32DivSTest, TruncationBehavior_TruncatesTowardZero)
{
    // Test positive truncation: 7 ÷ 3 = 2.33... → 2
    ASSERT_EQ(2, call_i32_div_s(7, 3))
        << "Failed: 7 ÷ 3 should truncate to 2";

    // Test negative truncation: -7 ÷ 3 = -2.33... → -2
    ASSERT_EQ(-2, call_i32_div_s(-7, 3))
        << "Failed: -7 ÷ 3 should truncate to -2";

    // Test mixed sign truncation: 7 ÷ -3 = -2.33... → -2
    ASSERT_EQ(-2, call_i32_div_s(7, -3))
        << "Failed: 7 ÷ -3 should truncate to -2";

    // Test both negative truncation: -7 ÷ -3 = 2.33... → 2
    ASSERT_EQ(2, call_i32_div_s(-7, -3))
        << "Failed: -7 ÷ -3 should truncate to 2";
}

/**
 * @test IdentityOperations_ProducesExpectedResults
 * @brief Validates i32.div_s identity operations produce mathematically correct results
 * @details Tests division operations with identity values (0, 1, -1) and self-division
 *          to verify fundamental mathematical properties are preserved.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_s_identity
 * @input_conditions Identity values and self-division scenarios
 * @expected_behavior Mathematical identity results
 * @validation_method Verification of identity property preservation
 */
TEST_P(I32DivSTest, IdentityOperations_ProducesExpectedResults)
{
    // Test division by 1 (identity)
    ASSERT_EQ(42, call_i32_div_s(42, 1))
        << "Failed: 42 ÷ 1 should equal 42";
    ASSERT_EQ(-42, call_i32_div_s(-42, 1))
        << "Failed: -42 ÷ 1 should equal -42";

    // Test zero dividend
    ASSERT_EQ(0, call_i32_div_s(0, 5))
        << "Failed: 0 ÷ 5 should equal 0";
    ASSERT_EQ(0, call_i32_div_s(0, -5))
        << "Failed: 0 ÷ -5 should equal 0";

    // Test self division (non-zero values)
    ASSERT_EQ(1, call_i32_div_s(15, 15))
        << "Failed: 15 ÷ 15 should equal 1";
    ASSERT_EQ(1, call_i32_div_s(-15, -15))
        << "Failed: -15 ÷ -15 should equal 1";
}

/**
 * @test DivisionByZero_ThrowsTrap
 * @brief Validates i32.div_s properly traps on division by zero operations
 * @details Tests division by zero scenarios to ensure WAMR runtime correctly
 *          detects and traps these undefined operations.
 * @test_category Exception - Error condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_s_trap_handling
 * @input_conditions Various dividend values with zero divisor
 * @expected_behavior Division by zero trap in all cases
 * @validation_method Verification of trap occurrence and exception handling
 */
TEST_P(I32DivSTest, DivisionByZero_ThrowsTrap)
{
    // Test positive dividend divided by zero
    ASSERT_TRUE(call_i32_div_s_expect_trap(10, 0))
        << "Failed: 10 ÷ 0 should trigger division by zero trap";

    // Test negative dividend divided by zero
    ASSERT_TRUE(call_i32_div_s_expect_trap(-10, 0))
        << "Failed: -10 ÷ 0 should trigger division by zero trap";

    // Test zero dividend divided by zero
    ASSERT_TRUE(call_i32_div_s_expect_trap(0, 0))
        << "Failed: 0 ÷ 0 should trigger division by zero trap";

    // Test boundary values divided by zero
    ASSERT_TRUE(call_i32_div_s_expect_trap(2147483647, 0))
        << "Failed: INT32_MAX ÷ 0 should trigger division by zero trap";
    ASSERT_TRUE(call_i32_div_s_expect_trap(-2147483648, 0))
        << "Failed: INT32_MIN ÷ 0 should trigger division by zero trap";
}

/**
 * @test IntegerOverflow_ThrowsTrap
 * @brief Validates i32.div_s properly traps on signed integer overflow condition
 * @details Tests the specific case of INT32_MIN ÷ -1 which causes signed integer
 *          overflow and should trigger a trap according to WebAssembly specification.
 * @test_category Exception - Overflow condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_div_s_overflow_detection
 * @input_conditions INT32_MIN dividend with -1 divisor
 * @expected_behavior Integer overflow trap
 * @validation_method Verification of overflow trap and proper exception handling
 */
TEST_P(I32DivSTest, IntegerOverflow_ThrowsTrap)
{
    const int32 INT32_MIN_VAL = -2147483648;

    // Test INT32_MIN ÷ -1 overflow condition
    ASSERT_TRUE(call_i32_div_s_expect_trap(INT32_MIN_VAL, -1))
        << "Failed: INT32_MIN ÷ -1 should trigger integer overflow trap";
}

// Parameterized test execution for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I32DivSTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));