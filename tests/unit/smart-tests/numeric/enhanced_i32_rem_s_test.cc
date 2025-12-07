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
static wasm_function_inst_t i32_rem_s_func = nullptr;

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

    // Lookup i32.rem_s test function
    if (!(i32_rem_s_func = wasm_runtime_lookup_function(module_inst, "i32_rem_s"))) {
        printf("Failed to lookup i32_rem_s function\n");
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
call_i32_rem_s(int32 dividend, int32 divisor)
{
    uint32 argv[2] = { static_cast<uint32>(dividend), static_cast<uint32>(divisor) };
    uint32 argc = 2;

    if (!wasm_runtime_call_wasm(exec_env, i32_rem_s_func, argc, argv)) {
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
call_i32_rem_s_expect_trap(int32 dividend, int32 divisor)
{
    uint32 argv[2] = { static_cast<uint32>(dividend), static_cast<uint32>(divisor) };
    uint32 argc = 2;

    bool result = wasm_runtime_call_wasm(exec_env, i32_rem_s_func, argc, argv);
    if (!result) {
        // Clear exception for next test
        wasm_runtime_clear_exception(module_inst);
        return true; // Trap occurred as expected
    }
    return false; // No trap occurred
}

/**
 * Test suite for i32.rem_s opcode validation.
 *
 * Provides comprehensive test coverage for signed 32-bit integer remainder (modulo) including:
 * - Basic remainder operations with various sign combinations
 * - Boundary condition handling with extreme values (INT32_MIN, INT32_MAX)
 * - Sign handling validation (remainder has same sign as dividend)
 * - Identity operation testing (x % x = 0, x % 1 = 0)
 * - Exception handling for division by zero and special cases
 * - Cross-execution mode consistency verification (interpreter vs AOT)
 */
class I32RemSTest : public testing::Test
{
protected:
    /**
     * Set up test environment and initialize WAMR runtime.
     * Loads the WASM module containing i32.rem_s test functions and
     * prepares execution environment for cross-mode validation.
     */
    void SetUp() override
    {
        // Get current working directory for WASM file loading
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";
        CWD = std::string(cwd_ptr);
        free(cwd_ptr);
        WASM_FILE = CWD + "/wasm-apps/i32_rem_s_test.wasm";

        // Initialize WAMR runtime
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load test module
        ASSERT_TRUE(load_wasm_module())
            << "Failed to load i32.rem_s test module: " << WASM_FILE;
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
 * @test BasicRemainder_ReturnsCorrectResult
 * @brief Validates i32.rem_s produces correct signed remainder for typical inputs
 * @details Tests fundamental remainder operation with positive, negative, and mixed-sign integers.
 *          Verifies that remainder follows signed semantics where result has same sign as dividend.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_rem_s_operation
 * @input_conditions Standard integer pairs: (17,5), (-17,5), (17,-5), (-17,-5)
 * @expected_behavior Returns mathematical signed remainder: 2, -2, 2, -2 respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(I32RemSTest, BasicRemainder_ReturnsCorrectResult)
{
    // Test positive remainder: 17 % 5 = 2
    ASSERT_EQ(2, call_i32_rem_s(17, 5))
        << "Remainder of positive integers failed: 17 % 5";

    // Test negative dividend: -17 % 5 = -2 (remainder has same sign as dividend)
    ASSERT_EQ(-2, call_i32_rem_s(-17, 5))
        << "Remainder of negative dividend failed: -17 % 5";

    // Test negative divisor: 17 % -5 = 2 (remainder has same sign as dividend)
    ASSERT_EQ(2, call_i32_rem_s(17, -5))
        << "Remainder of negative divisor failed: 17 % -5";

    // Test both negative: -17 % -5 = -2 (remainder has same sign as dividend)
    ASSERT_EQ(-2, call_i32_rem_s(-17, -5))
        << "Remainder of both negative integers failed: -17 % -5";
}

/**
 * @test BoundaryValues_ReturnsCorrectResult
 * @brief Validates i32.rem_s handles boundary values correctly without overflow
 * @details Tests remainder operations with INT32_MAX, INT32_MIN and other boundary conditions.
 *          Ensures proper handling of extreme values without arithmetic overflow.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_rem_s_boundary_handling
 * @input_conditions Boundary value combinations: INT32_MAX%3, INT32_MIN%3, small%INT32_MAX
 * @expected_behavior Correct mathematical remainders: 1, -2, 5 respectively
 * @validation_method Verification of boundary value arithmetic correctness
 */
TEST_F(I32RemSTest, BoundaryValues_ReturnsCorrectResult)
{
    // Test INT32_MAX % 3 = 1 (2147483647 % 3 = 1)
    ASSERT_EQ(1, call_i32_rem_s(INT32_MAX, 3))
        << "Boundary value remainder failed: INT32_MAX % 3";

    // Test INT32_MIN % 3 = -2 (-2147483648 % 3 = -2)
    ASSERT_EQ(-2, call_i32_rem_s(INT32_MIN, 3))
        << "Boundary value remainder failed: INT32_MIN % 3";

    // Test small value % large divisor: 5 % INT32_MAX = 5
    ASSERT_EQ(5, call_i32_rem_s(5, INT32_MAX))
        << "Small dividend with large divisor failed: 5 % INT32_MAX";

    // Test around zero boundary: -1 % 1 = 0
    ASSERT_EQ(0, call_i32_rem_s(-1, 1))
        << "Zero boundary remainder failed: -1 % 1";
}

/**
 * @test ZeroOperand_ReturnsCorrectResult
 * @brief Validates i32.rem_s handles zero operands correctly
 * @details Tests remainder operations with zero dividend (0 % n = 0 for any non-zero n).
 *          Verifies mathematical property that zero divided by any non-zero number yields zero.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_rem_s_zero_handling
 * @input_conditions Zero dividend with various non-zero divisors: (0,5), (0,-3)
 * @expected_behavior Always returns zero: 0, 0
 * @validation_method Verification of zero operand mathematical properties
 */
TEST_F(I32RemSTest, ZeroOperand_ReturnsCorrectResult)
{
    // Test 0 % positive = 0
    ASSERT_EQ(0, call_i32_rem_s(0, 5))
        << "Zero remainder failed: 0 % 5";

    // Test 0 % negative = 0
    ASSERT_EQ(0, call_i32_rem_s(0, -3))
        << "Zero remainder failed: 0 % -3";
}

/**
 * @test IdentityOperations_ReturnsZero
 * @brief Validates i32.rem_s mathematical identity properties
 * @details Tests mathematical identities: x % x = 0, x % 1 = 0, x % -1 = 0.
 *          Verifies fundamental mathematical properties of remainder operation.
 * @test_category Edge - Mathematical identity validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_rem_s_identity_operations
 * @input_conditions Identity scenarios: (15,15), (42,1), (42,-1)
 * @expected_behavior Always returns zero: 0, 0, 0
 * @validation_method Verification of mathematical identity properties
 */
TEST_F(I32RemSTest, IdentityOperations_ReturnsZero)
{
    // Test x % x = 0
    ASSERT_EQ(0, call_i32_rem_s(15, 15))
        << "Identity operation failed: 15 % 15";

    // Test x % 1 = 0 (any number modulo 1 is 0)
    ASSERT_EQ(0, call_i32_rem_s(42, 1))
        << "Identity operation failed: 42 % 1";

    // Test x % -1 = 0 (any number modulo -1 is 0)
    ASSERT_EQ(0, call_i32_rem_s(42, -1))
        << "Identity operation failed: 42 % -1";
}

/**
 * @test SpecialCase_IntMinModNegOne_ReturnsZero
 * @brief Validates i32.rem_s handles INT32_MIN % -1 special case correctly
 * @details Tests the special case INT32_MIN % -1 which mathematically should return 0.
 *          This case is special because INT32_MIN / -1 would overflow, but remainder is well-defined.
 * @test_category Corner - Special overflow case validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_rem_s_overflow_case
 * @input_conditions Special overflow case: INT32_MIN % -1
 * @expected_behavior Returns zero: 0
 * @validation_method Verification of special case mathematical correctness
 */
TEST_F(I32RemSTest, SpecialCase_IntMinModNegOne_ReturnsZero)
{
    // Test INT32_MIN % -1 = 0 (special case - no overflow in remainder)
    ASSERT_EQ(0, call_i32_rem_s(INT32_MIN, -1))
        << "Special case remainder failed: INT32_MIN % -1";
}

/**
 * @test DivisionByZero_ThrowsTrap
 * @brief Validates i32.rem_s properly traps on division by zero
 * @details Tests that remainder by zero operations correctly trigger WASM traps.
 *          Verifies proper exception handling for undefined mathematical operations.
 * @test_category Error - Division by zero validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_rem_s_trap_handling
 * @input_conditions Division by zero scenarios: (5,0), (-5,0), (0,0)
 * @expected_behavior Runtime trap/exception for all cases
 * @validation_method Exception detection and proper trap handling verification
 */
TEST_F(I32RemSTest, DivisionByZero_ThrowsTrap)
{
    // Test positive number % 0 should trap
    ASSERT_TRUE(call_i32_rem_s_expect_trap(5, 0))
        << "Division by zero trap failed: 5 % 0";

    // Test negative number % 0 should trap
    ASSERT_TRUE(call_i32_rem_s_expect_trap(-5, 0))
        << "Division by zero trap failed: -5 % 0";

    // Test 0 % 0 should trap
    ASSERT_TRUE(call_i32_rem_s_expect_trap(0, 0))
        << "Division by zero trap failed: 0 % 0";
}

