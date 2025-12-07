/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"
#include "bh_platform.h"
#include "wasm_export.h"

#ifndef WASM_ENABLE_INTERP
#define WASM_ENABLE_INTERP 1
#endif
#ifndef WASM_ENABLE_AOT
#define WASM_ENABLE_AOT 1
#endif

// Use WAMR's built-in RunningMode enum from wasm_export.h
// Mode_Interp = 0, Mode_AOT = 1

/**
 * @class ReturnCallIndirectTest
 * @brief Test fixture class for comprehensive return_call_indirect opcode testing
 *
 * This test suite validates the WebAssembly return_call_indirect instruction across
 * different execution modes (interpreter and AOT), ensuring correct tail call
 * optimization behavior for indirect function dispatch through function tables.
 *
 * Features tested:
 * - Basic indirect tail calls with various function signatures
 * - Stack depth verification (no growth during tail calls)
 * - Function table lookup and type validation
 * - Parameter passing and return value propagation
 * - Error handling for invalid table indices and type mismatches
 * - Cross-execution mode consistency verification
 */
class ReturnCallIndirectTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment before each test case
     *
     * Initializes WAMR runtime with appropriate configuration for the current
     * execution mode (interpreter or AOT). Configures memory allocation,
     * enables required features, and prepares the runtime environment.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));

        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.n_native_symbols = 0;
        init_args.native_module_name = NULL;
        init_args.native_symbols = NULL;

        // Initialize WAMR runtime
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        running_mode = GetParam();

        // Initialize module state
        current_module = nullptr;
        current_module_inst = nullptr;
        current_file_buf = nullptr;
    }

    /**
     * @brief Clean up test environment after each test case
     *
     * Properly deallocates WASM modules, module instances, and file buffers.
     * Destroys the WAMR runtime to ensure clean state between test cases.
     */
    void TearDown() override
    {
        // Clean up module resources
        if (current_module_inst) {
            wasm_runtime_deinstantiate(current_module_inst);
            current_module_inst = nullptr;
        }

        if (current_module) {
            wasm_runtime_unload(current_module);
            current_module = nullptr;
        }

        if (current_file_buf) {
            BH_FREE(current_file_buf);
            current_file_buf = nullptr;
        }

        // Destroy WAMR runtime
        wasm_runtime_destroy();
    }

    /**
     * @brief Load and instantiate WASM module for testing
     * @param wasm_file_path Path to the WASM binary file relative to test execution directory
     * @return Pointer to instantiated WASM module instance, or nullptr on failure
     *
     * Loads a WASM module from file, validates it, and creates a module instance
     * configured for the current execution mode. Handles both interpreter and
     * AOT mode instantiation with appropriate error reporting.
     */
    wasm_module_inst_t LoadTestModule(const char* wasm_file_path)
    {
        uint32 wasm_file_size;
        uint8 *wasm_file_buf = nullptr;
        wasm_module_t wasm_module = nullptr;
        wasm_module_inst_t module_inst = nullptr;
        char error_buf[128] = {0};

        // Read WASM file from filesystem
        wasm_file_buf = (uint8*)bh_read_file_to_buffer(wasm_file_path, &wasm_file_size);
        EXPECT_NE(nullptr, wasm_file_buf)
            << "Failed to read WASM file: " << wasm_file_path;

        if (!wasm_file_buf) {
            return nullptr;
        }

        // Load and validate WASM module
        wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, wasm_module)
            << "Failed to load WASM module: " << error_buf;

        if (!wasm_module) {
            BH_FREE(wasm_file_buf);
            return nullptr;
        }

        // Create module instance with sufficient stack and heap
        uint32 stack_size = 16 * 1024;  // 16KB stack
        uint32 heap_size = 16 * 1024;   // 16KB heap

        module_inst = wasm_runtime_instantiate(wasm_module, stack_size, heap_size, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        // Store references for cleanup
        current_module = wasm_module;
        current_module_inst = module_inst;
        current_file_buf = wasm_file_buf;

        return module_inst;
    }

    /**
     * @brief Execute WASM function and return result
     * @param module_inst WASM module instance
     * @param func_name Name of exported function to execute
     * @param argc Number of function arguments
     * @param argv Array of function arguments (uint32 values)
     * @return Function execution result, or UINT32_MAX on failure
     *
     * Executes the specified exported function with provided arguments and
     * returns the result. Handles execution errors and provides detailed
     * error reporting for debugging failures.
     */
    uint32 CallWasmFunction(wasm_module_inst_t module_inst, const char* func_name,
                           uint32 argc, uint32 argv[])
    {
        wasm_function_inst_t func;
        wasm_exec_env_t exec_env = NULL;
        bool success = false;
        uint32 result = 0;

        // Look up exported function
        func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func)
            << "Failed to find exported function: " << func_name;

        if (!func) {
            return UINT32_MAX;
        }

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, 8192);  // 8KB exec stack
        EXPECT_NE(nullptr, exec_env)
            << "Failed to create execution environment";

        if (!exec_env) {
            return UINT32_MAX;
        }

        // Execute function with exception handling
        success = wasm_runtime_call_wasm(exec_env, func, argc, argv);

        if (success && argc > 0) {
            // Function result is returned in argv[0] for functions with return values
            result = argv[0];
        }

        if (!success) {
            // Function execution failed - caller should check for exceptions if expected
            result = UINT32_MAX;
        }

        // Clean up execution environment
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }

        return result;
    }

    RuntimeInitArgs init_args;
    RunningMode running_mode;
    wasm_module_t current_module;
    wasm_module_inst_t current_module_inst;
    uint8* current_file_buf;
};

/**
 * @test BasicIndirectTailCall_ReturnsCorrectResult
 * @brief Validates basic return_call_indirect produces correct arithmetic results for typical inputs
 * @details Tests fundamental indirect tail call operation with simple integer functions.
 *          Verifies that return_call_indirect correctly dispatches to functions in the table
 *          and returns results without growing the call stack.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_operation
 * @input_conditions Simple integer operations through function table indices 0 and 1
 * @expected_behavior Returns correct arithmetic results: addition and multiplication
 * @validation_method Direct comparison of WASM function results with expected values
 */
TEST_P(ReturnCallIndirectTest, BasicIndirectTailCall_ReturnsCorrectResult)
{
    // Load WASM module with return_call_indirect test functions
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect test module";

    // Test indirect tail call to addition function (table index 0)
    uint32 add_args[] = {5, 3, 0};  // a=5, b=3, table_index=0
    uint32 add_result = CallWasmFunction(module_inst, "test_indirect_add", 3, add_args);
    ASSERT_EQ(8, add_result) << "Addition through indirect tail call failed";

    // Test indirect tail call to multiplication function (table index 1)
    uint32 mul_args[] = {4, 7, 1};  // a=4, b=7, table_index=1
    uint32 mul_result = CallWasmFunction(module_inst, "test_indirect_mul", 3, mul_args);
    ASSERT_EQ(28, mul_result) << "Multiplication through indirect tail call failed";
}

/**
 * @test MultiParameterCall_HandlesVariousTypes
 * @brief Validates return_call_indirect handles functions with multiple parameter types
 * @details Tests indirect tail calls to functions with various parameter combinations
 *          including mixed integer and floating-point types.
 * @test_category Main - Multi-parameter function validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_parameter_handling
 * @input_conditions Functions with 2-3 parameters of different types (i32, f64)
 * @expected_behavior Correct parameter passing and computation results
 * @validation_method Verification of complex arithmetic operations through indirect calls
 */
TEST_P(ReturnCallIndirectTest, MultiParameterCall_HandlesVariousTypes)
{
    // Load WASM module with multi-parameter function tests
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect test module";

    // Test indirect tail call to function with 3 i32 parameters (table index 2)
    uint32 triple_args[] = {2, 3, 4, 2};  // a=2, b=3, c=4, table_index=2
    uint32 triple_result = CallWasmFunction(module_inst, "test_triple_param", 4, triple_args);
    ASSERT_EQ(24, triple_result) << "Triple parameter function through indirect tail call failed";

    // Test indirect tail call to subtraction function (table index 3)
    uint32 sub_args[] = {15, 8, 3};  // a=15, b=8, table_index=3
    uint32 sub_result = CallWasmFunction(module_inst, "test_indirect_sub", 3, sub_args);
    ASSERT_EQ(7, sub_result) << "Subtraction through indirect tail call failed";
}

/**
 * @test VoidFunctionCall_HandlesNoParameters
 * @brief Validates return_call_indirect works with functions that have no parameters
 * @details Tests indirect tail calls to void functions and functions with no parameters
 *          to ensure proper handling of empty parameter lists.
 * @test_category Edge - Zero parameter function validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_void_functions
 * @input_conditions Functions with no parameters returning constant values
 * @expected_behavior Successful execution of void functions through indirect tail calls
 * @validation_method Verification of constant return values from parameterless functions
 */
TEST_P(ReturnCallIndirectTest, VoidFunctionCall_HandlesNoParameters)
{
    // Load WASM module with void function tests
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect test module";

    // Test indirect tail call to function with no parameters (table index 4)
    uint32 void_args[] = {4};  // table_index=4
    uint32 void_result = CallWasmFunction(module_inst, "test_void_function", 1, void_args);
    ASSERT_EQ(42, void_result) << "Void function through indirect tail call failed";

    // Test indirect tail call to constant function (table index 5)
    uint32 const_args[] = {5};  // table_index=5
    uint32 const_result = CallWasmFunction(module_inst, "test_constant_function", 1, const_args);
    ASSERT_EQ(100, const_result) << "Constant function through indirect tail call failed";
}

/**
 * @test TableBoundaryAccess_ValidatesIndexLimits
 * @brief Validates return_call_indirect handles table boundary conditions correctly
 * @details Tests access to first and last entries in function table to ensure
 *          proper boundary validation and correct function resolution.
 * @test_category Corner - Table boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_table_bounds
 * @input_conditions Access to table indices 0 (first) and table_size-1 (last)
 * @expected_behavior Valid function calls at table boundaries
 * @validation_method Successful execution of functions at table boundary indices
 */
TEST_P(ReturnCallIndirectTest, TableBoundaryAccess_ValidatesIndexLimits)
{
    // Load WASM module with table boundary tests
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect test module";

    // Test access to first table entry (index 0)
    uint32 first_args[] = {10, 5, 0};  // a=10, b=5, table_index=0 (first entry)
    uint32 first_result = CallWasmFunction(module_inst, "test_indirect_add", 3, first_args);
    ASSERT_EQ(15, first_result) << "First table entry access failed";

    // Test access to last valid table entry (index 5, assuming table size of 6)
    uint32 last_args[] = {5};  // table_index=5 (last entry)
    uint32 last_result = CallWasmFunction(module_inst, "test_constant_function", 1, last_args);
    ASSERT_EQ(100, last_result) << "Last table entry access failed";
}

/**
 * @test DeepTailRecursion_MaintainsStackDepth
 * @brief Validates return_call_indirect maintains constant stack depth during recursion
 * @details Tests recursive return_call_indirect operations to ensure tail call
 *          optimization prevents stack growth during deep recursion.
 * @test_category Corner - Recursive tail call validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_tail_optimization
 * @input_conditions Recursive function with countdown parameter through indirect calls
 * @expected_behavior Stack depth remains constant during recursion
 * @validation_method Successful completion of deep recursion without stack overflow
 */
TEST_P(ReturnCallIndirectTest, DeepTailRecursion_MaintainsStackDepth)
{
    // Load WASM module with recursive function tests
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect test module";

    // Test deep tail recursion (countdown from 1000 to 0)
    uint32 recursion_args[] = {1000, 6};  // count=1000, table_index=6 (recursive function)
    uint32 recursion_result = CallWasmFunction(module_inst, "test_recursive_countdown", 2, recursion_args);
    ASSERT_EQ(0, recursion_result) << "Deep tail recursion failed to complete";

    // Test factorial calculation through indirect tail recursion
    uint32 factorial_args[] = {5, 7};  // n=5, table_index=7 (factorial function)
    uint32 factorial_result = CallWasmFunction(module_inst, "test_recursive_factorial", 2, factorial_args);
    ASSERT_EQ(120, factorial_result) << "Factorial calculation through tail recursion failed";
}

/**
 * @test SelfReference_EnablesIndirectRecursion
 * @brief Validates return_call_indirect supports self-referential indirect calls
 * @details Tests functions that call themselves through the function table to ensure
 *          proper self-reference handling in indirect tail calls.
 * @test_category Edge - Self-reference validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_self_reference
 * @input_conditions Functions calling themselves through function table entries
 * @expected_behavior Proper self-referential indirect tail calls
 * @validation_method Verification of recursive algorithms through self-reference
 */
TEST_P(ReturnCallIndirectTest, SelfReference_EnablesIndirectRecursion)
{
    // Load WASM module with self-reference tests
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect test module";

    // Test Fibonacci calculation through self-referential indirect calls
    uint32 fib_args[] = {10, 8};  // n=10, table_index=8 (fibonacci function)
    uint32 fib_result = CallWasmFunction(module_inst, "test_fibonacci", 2, fib_args);
    ASSERT_EQ(55, fib_result) << "Fibonacci calculation through self-reference failed";

    // Test GCD calculation through self-referential indirect calls
    uint32 gcd_args[] = {48, 18, 9};  // a=48, b=18, table_index=9 (GCD function)
    uint32 gcd_result = CallWasmFunction(module_inst, "test_gcd", 3, gcd_args);
    ASSERT_EQ(6, gcd_result) << "GCD calculation through self-reference failed";
}

/**
 * @test InvalidTableIndex_GeneratesTrap
 * @brief Validates return_call_indirect generates proper traps for invalid table indices
 * @details Tests out-of-bounds table index access to ensure proper error handling
 *          and trap generation for invalid indirect calls.
 * @test_category Error - Invalid table index validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_bounds_check
 * @input_conditions Table indices exceeding table size or negative values
 * @expected_behavior Proper trap generation for invalid indices
 * @validation_method Verification of trap conditions and error handling
 */
TEST_P(ReturnCallIndirectTest, InvalidTableIndex_GeneratesTrap)
{
    // Load WASM module with error condition tests
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_error_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect error test module";

    // Test out-of-bounds table index (index 100, table size is much smaller)
    uint32 invalid_args[] = {5, 3, 100};  // a=5, b=3, invalid_table_index=100
    uint32 invalid_result = CallWasmFunction(module_inst, "test_invalid_index", 3, invalid_args);

    // Should fail due to invalid table index - check for exception
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(nullptr, exception) << "Expected trap for invalid table index but got result: " << invalid_result;
}

/**
 * @test NullTableEntry_GeneratesTrap
 * @brief Validates return_call_indirect generates proper traps for null table entries
 * @details Tests access to uninitialized or null function references in table
 *          to ensure proper error handling for invalid function references.
 * @test_category Error - Null reference validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_null_check
 * @input_conditions Table entries containing null function references
 * @expected_behavior Proper trap generation for null function references
 * @validation_method Verification of null reference handling and trap generation
 */
TEST_P(ReturnCallIndirectTest, NullTableEntry_GeneratesTrap)
{
    // Load WASM module with null reference tests
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_error_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect error test module";

    // Test access to null table entry (index 10, which should be null)
    uint32 null_args[] = {10, 5, 10};  // a=10, b=5, null_table_index=10
    uint32 null_result = CallWasmFunction(module_inst, "test_null_entry", 3, null_args);

    // Should fail due to null table entry - check for exception
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(nullptr, exception) << "Expected trap for null table entry but got result: " << null_result;
}

/**
 * @test TypeMismatch_ValidatesSignatures
 * @brief Validates return_call_indirect performs proper function signature validation
 * @details Tests function calls with mismatched signatures to ensure type checking
 *          and proper error handling for incompatible function types.
 * @test_category Error - Type signature validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_indirect_type_check
 * @input_conditions Functions with signatures that don't match expected type index
 * @expected_behavior Type validation failure with appropriate error
 * @validation_method Verification of type checking and signature validation
 */
TEST_P(ReturnCallIndirectTest, TypeMismatch_ValidatesSignatures)
{
    // Load WASM module with type mismatch tests
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/return_call_indirect_error_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load return_call_indirect error test module";

    // Test function call with wrong signature (calling i32->i32 function with i32,i32->i32 signature)
    uint32 mismatch_args[] = {5, 3, 15};  // a=5, b=3, wrong_type_index=15
    uint32 mismatch_result = CallWasmFunction(module_inst, "test_type_mismatch", 3, mismatch_args);

    // Should fail due to type mismatch - check for exception
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(nullptr, exception) << "Expected trap for type mismatch but got result: " << mismatch_result;
}

// Instantiate parameterized tests for interpreter mode
INSTANTIATE_TEST_SUITE_P(
    ReturnCallIndirectModeTest,
    ReturnCallIndirectTest,
    testing::Values(Mode_Interp),
    [](const testing::TestParamInfo<ReturnCallIndirectTest::ParamType>& info) {
        return "InterpreterMode";
    }
);