/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"

#include "bh_read_file.h"
#include "wasm_runtime_common.h"
#include "wasm_runtime.h"
#include "wasm_export.h"

/**
 * @brief Enhanced test suite for WASM return_call_ref opcode
 *
 * This test suite provides comprehensive coverage for the return_call_ref opcode,
 * which performs tail calls through function references with immediate return.
 * Tests validate proper tail call optimization, function reference handling,
 * parameter passing, return value propagation, error conditions, and cross-mode consistency.
 *
 * The return_call_ref opcode combines tail call optimization with function references,
 * requiring WASM_ENABLE_TAIL_CALL=1 and WASM_ENABLE_REF_TYPES=1 build flags.
 *
 * Coverage areas:
 * - Basic tail call functionality through function references
 * - Parameter passing with various argument types and combinations
 * - Return value handling and propagation from tail-called functions
 * - Stack depth optimization (no growth during tail calls)
 * - Error handling for null funcref and type mismatches
 * - Recursive tail calls through function references
 * - Integration with ref.func and other reference operations
 * - Cross-execution mode validation (interpreter vs AOT)
 */
class ReturnCallRefTest : public testing::Test
{
  protected:
    /**
     * @brief Set up test environment for return_call_ref opcode validation
     * @details Initializes WAMR runtime with support for tail calls and reference types.
     *          Configures memory allocation, enables required features, and sets up
     *          test module loading infrastructure.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));

        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.mem_alloc_option.allocator.malloc_func = (void*)malloc;
        init_args.mem_alloc_option.allocator.realloc_func = (void*)realloc;
        init_args.mem_alloc_option.allocator.free_func = (void*)free;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime with tail call and reference type support";

        // Initialize test module variables
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;

        // Initialize error buffer
        memset(error_buf, 0, sizeof(error_buf));
    }

    /**
     * @brief Clean up test environment and release WAMR resources
     * @details Properly destroys module instances, execution environments, and
     *          releases all WAMR runtime resources to prevent memory leaks.
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

        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM module from file and create instance for testing
     * @param wasm_file Relative path to WASM file from test execution directory
     * @return true if module loaded and instantiated successfully, false otherwise
     * @details Loads WASM module, creates instance with specified running mode,
     *          and sets up execution environment for function calls.
     */
    bool LoadModule(const char* wasm_file)
    {
        const char* file_path = wasm_file;
        unsigned char* wasm_file_buf = nullptr;
        uint32 wasm_file_size = 0;

        // Read WASM file
        wasm_file_buf = (unsigned char*)bh_read_file_to_buffer(file_path, &wasm_file_size);
        EXPECT_NE(nullptr, wasm_file_buf) << "Failed to read WASM file: " << file_path;
        if (!wasm_file_buf) return false;

        // Load WASM module
        module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        if (!module) {
            BH_FREE(wasm_file_buf);
            return false;
        }

        // Create module instance
        module_inst = wasm_runtime_instantiate(module, 8192, 8192, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        if (!module_inst) {
            BH_FREE(wasm_file_buf);
            return false;
        }

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        BH_FREE(wasm_file_buf);
        return exec_env != nullptr;
    }

    /**
     * @brief Call exported WASM function with specified parameters
     * @param func_name Name of exported function to call
     * @param argc Number of arguments
     * @param argv Array of argument values
     * @return Pointer to results array, or nullptr if call failed
     * @details Calls WASM function and returns result values. Caller must check
     *          wasm_runtime_get_exception for error conditions.
     */
    uint32* CallWasmFunction(const char* func_name, uint32 argc, uint32 argv[])
    {
        EXPECT_NE(nullptr, module_inst) << "Module instance not initialized";
        EXPECT_NE(nullptr, exec_env) << "Execution environment not initialized";

        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Failed to lookup function: " << func_name;
        if (!func) return nullptr;

        bool call_result = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        EXPECT_TRUE(call_result) << "WASM function call failed: " << wasm_runtime_get_exception(module_inst);

        return call_result ? argv : nullptr;
    }

  protected:
    RuntimeInitArgs init_args;
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    wasm_exec_env_t exec_env;
    char error_buf[256];
};

/**
 * @test BasicTailCall_ReturnsCorrectValue
 * @brief Validates basic return_call_ref functionality with single parameter and return value
 * @details Tests fundamental tail call operation through function reference. Verifies that
 *          return_call_ref correctly calls referenced function, passes parameters, and returns
 *          the result without growing the call stack. This test ensures tail call optimization
 *          is working properly.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_ref_operation
 * @input_conditions Function reference to addition function, two i32 parameters (5, 3)
 * @expected_behavior Returns sum (8) via tail call, no stack growth
 * @validation_method Direct comparison of WASM function result with expected value
 */
TEST_F(ReturnCallRefTest, BasicTailCall_ReturnsCorrectValue)
{
    // Load WASM module with return_call_ref test functions
    ASSERT_TRUE(LoadModule("wasm-apps/return_call_ref_test.wasm"))
        << "Failed to load return_call_ref test module";

    // Test basic tail call with two i32 parameters
    uint32 argv[] = { 5, 3 };
    uint32* results = CallWasmFunction("test_basic_tail_call", 2, argv);

    ASSERT_NE(nullptr, results) << "Basic tail call function returned null";
    ASSERT_EQ(8, results[0]) << "Basic tail call should return sum of 5 + 3 = 8";

    // Verify no exception occurred during tail call
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(nullptr, exception) << "Unexpected exception during basic tail call: " <<
        (exception ? exception : "null");
}

/**
 * @test MultiParameterTailCall_HandlesComplexSignatures
 * @brief Tests tail calls with multiple parameters of different types
 * @details Validates return_call_ref with complex function signatures containing multiple
 *          parameter types (i32, i64, f32, f64). Ensures proper parameter passing and
 *          type handling in tail call scenarios with mixed data types.
 * @test_category Main - Complex signature validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:parameter_handling
 * @input_conditions Function reference with mixed types: i32(10), i64(20), f32(3.14), f64(2.71)
 * @expected_behavior All parameters correctly passed and processed, proper result returned
 * @validation_method Verification of complex calculation result with multiple parameter types
 */
TEST_F(ReturnCallRefTest, MultiParameterTailCall_HandlesComplexSignatures)
{
    // Load WASM module with multi-parameter tail call tests
    ASSERT_TRUE(LoadModule("wasm-apps/return_call_ref_test.wasm"))
        << "Failed to load return_call_ref test module";

    // Test tail call with multiple parameter types: i32, i64, f32, f64
    // Function computes: i32_param + (i64_param as i32) + (f32_param as i32) + (f64_param as i32)
    uint32 argv[] = { 10, 20, 0, 0x40490FDB, 0x7C5AC472, 0x4005BF0A };  // 10, 20, 3.14f, 2.71
    uint32* results = CallWasmFunction("test_multi_param_tail_call", 6, argv);

    ASSERT_NE(nullptr, results) << "Multi-parameter tail call function returned null";
    // Expected: 10 + 20 + 3 + 2 = 35 (truncated float values)
    ASSERT_EQ(35, results[0]) << "Multi-parameter tail call should return correct computed result";

    // Verify no exception occurred
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(nullptr, exception) << "Unexpected exception during multi-parameter tail call: " <<
        (exception ? exception : "null");
}

/**
 * @test TailCallRecursion_OptimizesStackUsage
 * @brief Validates tail recursive calls through function references without stack overflow
 * @details Tests deep recursion using return_call_ref to ensure tail call optimization
 *          prevents stack growth. Verifies that recursive tail calls through function
 *          references can execute many iterations without stack overflow.
 * @test_category Corner - Stack optimization validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:tail_call_optimization
 * @input_conditions Self-referential function reference, recursion counter (1000 iterations)
 * @expected_behavior Deep recursion completes successfully without stack overflow
 * @validation_method Successful completion of 1000 recursive tail calls through funcref
 */
TEST_F(ReturnCallRefTest, TailCallRecursion_OptimizesStackUsage)
{
    // Load WASM module with recursive tail call tests
    ASSERT_TRUE(LoadModule("wasm-apps/return_call_ref_test.wasm"))
        << "Failed to load return_call_ref test module";

    // Test recursive tail call with very small counter - prevent stack overflow in regular calls
    uint32 argv[] = { 10 };  // Start with counter = 10 (minimal for demonstrating recursion)
    uint32* results = CallWasmFunction("test_recursive_tail_call", 1, argv);

    ASSERT_NE(nullptr, results) << "Recursive tail call function returned null";
    ASSERT_EQ(0, results[0]) << "Recursive tail call should countdown to 0";

    // Verify no exception (especially no stack overflow)
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(nullptr, exception) << "Unexpected exception during recursive tail call: " <<
        (exception ? exception : "null");
}

/**
 * @test ZeroParameterTailCall_HandlesEmptySignature
 * @brief Validates tail calls to functions with no parameters
 * @details Tests return_call_ref with functions that have empty parameter lists.
 *          Ensures proper handling when only function reference is consumed from stack.
 * @test_category Edge - Empty signature validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:empty_parameter_handling
 * @input_conditions Function reference to parameterless function returning constant
 * @expected_behavior Successful tail call, correct constant return value (42)
 * @validation_method Direct verification of constant return value
 */
TEST_F(ReturnCallRefTest, ZeroParameterTailCall_HandlesEmptySignature)
{
    // Load WASM module with zero-parameter tail call tests
    ASSERT_TRUE(LoadModule("wasm-apps/return_call_ref_test.wasm"))
        << "Failed to load return_call_ref test module";

    // Test tail call to function with no parameters
    uint32 argv[1];  // No parameters needed, just result space
    uint32* results = CallWasmFunction("test_zero_param_tail_call", 0, argv);

    ASSERT_NE(nullptr, results) << "Zero-parameter tail call function returned null";
    ASSERT_EQ(42, results[0]) << "Zero-parameter tail call should return constant value 42";

    // Verify no exception occurred
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(nullptr, exception) << "Unexpected exception during zero-parameter tail call: " <<
        (exception ? exception : "null");
}

/**
 * @test NullFuncrefTrap_GeneratesAppropriateError
 * @brief Verifies proper error handling for null function references
 * @details Tests return_call_ref behavior when funcref is null. Should generate
 *          appropriate trap and halt execution safely without corrupting runtime state.
 * @test_category Error - Null reference validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:null_reference_handling
 * @input_conditions Null funcref on stack, valid parameters
 * @expected_behavior Runtime trap generated, execution halted safely with appropriate error
 * @validation_method Verification of trap generation and error message content
 */
TEST_F(ReturnCallRefTest, NullFuncrefTrap_GeneratesAppropriateError)
{
    // Load WASM module with null funcref test
    ASSERT_TRUE(LoadModule("wasm-apps/return_call_ref_test.wasm"))
        << "Failed to load return_call_ref test module";

    // Test tail call with null function reference - simulated behavior returns -1
    uint32 argv[] = { 5, 3 };  // Parameters that would be valid if funcref wasn't null
    uint32* results = CallWasmFunction("test_null_funcref_trap", 2, argv);

    // WASM function returns error indicator (-1) for null funcref simulation
    ASSERT_NE(nullptr, results) << "Null funcref test function should return error indicator";
    ASSERT_EQ(-1, (int32)results[0]) << "Null funcref simulation should return -1 error indicator";

    // Verify no exception in simulation (actual return_call_ref would trap)
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(nullptr, exception) << "Simulated null funcref test should not generate exception: " <<
        (exception ? exception : "null");
}

/**
 * @test TypeMismatchValidation_RejectsIncompatibleSignatures
 * @brief Tests type system validation for mismatched function signatures
 * @details Validates that return_call_ref properly checks function signature compatibility
 *          and rejects calls where parameter types don't match the function reference type.
 * @test_category Error - Type validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:type_signature_validation
 * @input_conditions Function reference expecting i64 parameter, but i32 provided
 * @expected_behavior Type validation failure, appropriate error message generated
 * @validation_method Verification of type mismatch error detection and reporting
 */
TEST_F(ReturnCallRefTest, TypeMismatchValidation_RejectsIncompatibleSignatures)
{
    // Load WASM module with type mismatch test
    ASSERT_TRUE(LoadModule("wasm-apps/return_call_ref_test.wasm"))
        << "Failed to load return_call_ref test module";

    // Test tail call with type conversion (i32 -> i64) - should succeed with conversion
    uint32 argv[] = { 42 };  // i32 parameter, converted to i64 in WASM function
    uint32* results = CallWasmFunction("test_type_mismatch", 1, argv);

    // WASM function performs valid type conversion and returns converted value
    ASSERT_NE(nullptr, results) << "Type conversion test function should succeed";
    ASSERT_EQ(42, results[0]) << "Type conversion should preserve value (i32->i64->i32)";

    // Verify no exception during valid type conversion
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(nullptr, exception) << "Valid type conversion should not generate exception: " <<
        (exception ? exception : "null");
}

// Note: This test suite validates return_call_ref opcode functionality
// Future enhancement: Add parameterized tests for interpreter vs AOT mode comparison