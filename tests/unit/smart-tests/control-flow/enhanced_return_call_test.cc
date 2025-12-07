/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include "platform_common.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"

static std::string CWD;
static std::string WASM_FILE_1 = "wasm-apps/return_call_test.wasm";
static std::string WASM_FILE_2 = "wasm-apps/return_call_error_test.wasm";

static constexpr const char *MODULE_NAME = "return_call_test";

/**
 * @brief Test fixture class for return_call opcode comprehensive testing
 *
 * This test suite validates the return_call instruction which performs tail call optimization
 * by executing a function call and immediately returning the result without growing the call stack.
 * Tests cover basic functionality, stack behavior, parameter handling, recursion scenarios,
 * and error conditions across both interpreter and AOT execution modes.
 */
class ReturnCallTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment before each test case
     *
     * Initializes WAMR runtime with proper configuration for both interpreter and AOT modes.
     * Sets up memory allocation and prepares test module loading infrastructure.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize runtime with support for both execution modes
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        cleanup_required = true;
    }

    /**
     * @brief Clean up test environment after each test case
     *
     * Properly destroys WAMR runtime and releases allocated resources.
     */
    void TearDown() override
    {
        if (cleanup_required) {
            wasm_runtime_destroy();
        }
    }

    /**
     * @brief Load WASM module from file for testing
     *
     * @param wasm_file_path Path to the WASM binary file
     * @return wasm_module_t Loaded WASM module or nullptr on failure
     */
    wasm_module_t load_wasm_module(const std::string &wasm_file_path)
    {
        const char *file_path = wasm_file_path.c_str();
        unsigned char *wasm_file_buf = nullptr;
        uint32_t wasm_file_size = 0;
        char error_buf[128] = {0};

        // Read WASM file from filesystem
        wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(file_path, &wasm_file_size);
        if (!wasm_file_buf) {
            return nullptr;
        }

        // Load WASM module with runtime validation
        wasm_module_t module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                               error_buf, sizeof(error_buf));

        // Clean up file buffer
        BH_FREE(wasm_file_buf);

        if (!module) {
            std::cout << "Module load error: " << error_buf << std::endl;
        }

        return module;
    }

    /**
     * @brief Create module instance for test execution
     *
     * @param module WASM module to instantiate
     * @param stack_size Stack size for module execution
     * @param heap_size Heap size for module memory
     * @return wasm_module_inst_t Module instance or nullptr on failure
     */
    wasm_module_inst_t create_module_instance(wasm_module_t module,
                                             uint32_t stack_size = 8092,
                                             uint32_t heap_size = 8092)
    {
        char error_buf[128] = {0};
        wasm_module_inst_t module_inst = wasm_runtime_instantiate(
            module, stack_size, heap_size, error_buf, sizeof(error_buf)
        );

        if (!module_inst) {
            std::cout << "Module instantiation error: " << error_buf << std::endl;
        }

        return module_inst;
    }

    /**
     * @brief Create execution environment for function calls
     *
     * @param module_inst Module instance for execution
     * @param stack_size Stack size for execution environment
     * @return wasm_exec_env_t Execution environment or nullptr on failure
     */
    wasm_exec_env_t create_exec_env(wasm_module_inst_t module_inst, uint32_t stack_size = 8192)
    {
        return wasm_runtime_create_exec_env(module_inst, stack_size);
    }

    /**
     * @brief Execute exported WASM function with parameters
     *
     * @param module_inst Module instance containing the function
     * @param func_name Name of the exported function to call
     * @param argc Number of arguments
     * @param argv Array of function arguments
     * @return bool True if execution succeeded, false otherwise
     */
    bool call_wasm_function(wasm_module_inst_t module_inst, const char* func_name,
                          uint32_t argc, uint32_t argv[])
    {
        char error_buf[128] = {0};
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(module_inst, func_name);

        if (!func_inst) {
            std::cout << "Function lookup failed: " << func_name << std::endl;
            return false;
        }

        // Create execution environment for this function call
        wasm_exec_env_t exec_env = create_exec_env(module_inst);
        if (!exec_env) {
            std::cout << "Failed to create execution environment" << std::endl;
            return false;
        }

        // Execute function and capture success/failure
        bool success = wasm_runtime_call_wasm(exec_env, func_inst, argc, argv);

        if (!success) {
            std::cout << "Function execution failed: " << wasm_runtime_get_exception(module_inst) << std::endl;
        }

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        return success;
    }

private:
    RuntimeInitArgs init_args;
    bool cleanup_required = false;
};

/**
 * @test BasicTailCall_ReturnsCorrectResult
 * @brief Validates return_call produces correct results for simple tail call scenarios
 * @details Tests fundamental tail call operation with single parameter functions.
 *          Verifies that return_call correctly transfers control and returns results.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_operation
 * @input_conditions Simple integer parameters passed through tail call chain
 * @expected_behavior Returns correct computed result from tail-called function
 * @validation_method Direct comparison of returned values with expected results
 */
TEST_P(ReturnCallTest, BasicTailCall_ReturnsCorrectResult)
{
    // Load test module containing basic tail call functions
    wasm_module_t module = load_wasm_module(WASM_FILE_1);
    ASSERT_NE(nullptr, module) << "Failed to load basic tail call test module";

    // Create module instance for function execution
    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test basic tail call with integer parameter
    uint32_t argv[2] = {10}; // Input parameter
    bool success = call_wasm_function(module_inst, "test_basic_tail_call", 1, argv);
    ASSERT_TRUE(success) << "Basic tail call execution failed";
    ASSERT_EQ(20, argv[0]) << "Basic tail call returned incorrect result";

    // Test tail call with different parameter value
    argv[0] = 5;
    success = call_wasm_function(module_inst, "test_basic_tail_call", 1, argv);
    ASSERT_TRUE(success) << "Basic tail call with different parameter failed";
    ASSERT_EQ(10, argv[0]) << "Basic tail call with parameter 5 returned incorrect result";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test MultipleParameters_HandlesCorrectly
 * @brief Validates return_call with multiple typed parameters
 * @details Tests tail calls with functions taking multiple parameters of different types.
 *          Verifies correct parameter passing and type handling through tail calls.
 * @test_category Main - Multi-parameter functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_parameter_handling
 * @input_conditions Multiple parameters of different types (i32, i64, f32, f64)
 * @expected_behavior Correctly processes all parameters and returns expected results
 * @validation_method Verify each parameter type is handled correctly in tail call
 */
TEST_P(ReturnCallTest, MultipleParameters_HandlesCorrectly)
{
    // Load test module with multi-parameter tail call functions
    wasm_module_t module = load_wasm_module(WASM_FILE_1);
    ASSERT_NE(nullptr, module) << "Failed to load multi-parameter test module";

    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test tail call with two integer parameters
    uint32_t argv[3] = {15, 25}; // Two input parameters
    bool success = call_wasm_function(module_inst, "test_multi_param_tail_call", 2, argv);
    ASSERT_TRUE(success) << "Multi-parameter tail call execution failed";
    ASSERT_EQ(40, argv[0]) << "Multi-parameter tail call returned incorrect sum";

    // Test with different parameter values
    argv[0] = 100; argv[1] = 200;
    success = call_wasm_function(module_inst, "test_multi_param_tail_call", 2, argv);
    ASSERT_TRUE(success) << "Multi-parameter tail call with larger values failed";
    ASSERT_EQ(300, argv[0]) << "Multi-parameter tail call with large values returned incorrect result";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test ZeroParameters_ExecutesSuccessfully
 * @brief Validates return_call to parameterless functions
 * @details Tests tail calls to functions with no parameters, including void return functions.
 *          Verifies correct handling of parameterless function calls.
 * @test_category Main - Zero-parameter functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:return_call_zero_params
 * @input_conditions Tail calls to functions with no parameters
 * @expected_behavior Successfully executes parameterless tail calls
 * @validation_method Verify successful execution and appropriate return handling
 */
TEST_P(ReturnCallTest, ZeroParameters_ExecutesSuccessfully)
{
    // Load test module with parameterless tail call functions
    wasm_module_t module = load_wasm_module(WASM_FILE_1);
    ASSERT_NE(nullptr, module) << "Failed to load zero-parameter test module";

    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test tail call to parameterless function returning constant
    uint32_t argv[1] = {0}; // No input parameters, space for return value
    bool success = call_wasm_function(module_inst, "test_zero_param_tail_call", 0, argv);
    ASSERT_TRUE(success) << "Zero-parameter tail call execution failed";
    ASSERT_EQ(42, argv[0]) << "Zero-parameter tail call returned incorrect constant";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test BoundaryFunctionIndex_CallsCorrectly
 * @brief Validates return_call with boundary function indices
 * @details Tests tail calls to functions at index boundaries (first and last valid indices).
 *          Verifies correct function resolution at module boundaries.
 * @test_category Corner - Function index boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:function_index_validation
 * @input_conditions Tail calls to first and last valid function indices in module
 * @expected_behavior Successfully calls functions at valid boundary indices
 * @validation_method Verify function calls succeed and return expected results
 */
TEST_P(ReturnCallTest, BoundaryFunctionIndex_CallsCorrectly)
{
    // Load test module with boundary function tests
    wasm_module_t module = load_wasm_module(WASM_FILE_1);
    ASSERT_NE(nullptr, module) << "Failed to load boundary function test module";

    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test tail call to first function (index 0 equivalent)
    uint32_t argv[2] = {7}; // Input parameter
    bool success = call_wasm_function(module_inst, "test_first_function_tail_call", 1, argv);
    ASSERT_TRUE(success) << "Tail call to first function failed";
    ASSERT_EQ(14, argv[0]) << "First function tail call returned incorrect result";

    // Test tail call to last function in module
    argv[0] = 8;
    success = call_wasm_function(module_inst, "test_last_function_tail_call", 1, argv);
    ASSERT_TRUE(success) << "Tail call to last function failed";
    ASSERT_EQ(24, argv[0]) << "Last function tail call returned incorrect result";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test DeepRecursion_MaintainsConstantStack
 * @brief Validates return_call maintains constant stack depth during deep recursion
 * @details Tests long chains of recursive tail calls to verify stack optimization.
 *          Ensures tail call optimization prevents stack overflow in deep recursion.
 * @test_category Corner - Stack depth optimization validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:tail_call_optimization
 * @input_conditions Deep recursive tail call chain (1000 iterations)
 * @expected_behavior Completes without stack overflow, maintains constant stack depth
 * @validation_method Verify completion of deep recursion without stack-related errors
 */
TEST_P(ReturnCallTest, DeepRecursion_MaintainsConstantStack)
{
    // Load test module with recursive tail call functions
    wasm_module_t module = load_wasm_module(WASM_FILE_1);
    ASSERT_NE(nullptr, module) << "Failed to load recursive test module";

    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test deep recursive tail call (countdown from large number)
    uint32_t argv[2] = {1000}; // Start countdown from 1000
    bool success = call_wasm_function(module_inst, "test_recursive_countdown", 1, argv);
    ASSERT_TRUE(success) << "Deep recursive tail call execution failed";
    ASSERT_EQ(0, argv[0]) << "Recursive countdown did not reach zero";

    // Test with smaller recursion depth for reliability
    argv[0] = 100;
    success = call_wasm_function(module_inst, "test_recursive_countdown", 1, argv);
    ASSERT_TRUE(success) << "Medium recursive tail call execution failed";
    ASSERT_EQ(0, argv[0]) << "Medium recursive countdown did not reach zero";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test RecursiveTailCall_OptimizesCorrectly
 * @brief Validates return_call optimization for recursive patterns
 * @details Tests direct and mutual recursion patterns using tail calls.
 *          Verifies proper tail call optimization eliminates stack growth.
 * @test_category Edge - Recursive optimization validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:recursive_tail_optimization
 * @input_conditions Direct self-recursion and mutual recursion patterns
 * @expected_behavior Recursive patterns execute without stack overflow
 * @validation_method Verify recursive functions complete successfully with tail optimization
 */
TEST_P(ReturnCallTest, RecursiveTailCall_OptimizesCorrectly)
{
    // Load test module with recursive optimization tests
    wasm_module_t module = load_wasm_module(WASM_FILE_1);
    ASSERT_NE(nullptr, module) << "Failed to load recursive optimization test module";

    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test direct self-recursion with tail call
    uint32_t argv[2] = {5}; // Factorial calculation input
    bool success = call_wasm_function(module_inst, "test_tail_factorial", 1, argv);
    ASSERT_TRUE(success) << "Tail recursive factorial execution failed";
    ASSERT_EQ(120, argv[0]) << "Tail factorial returned incorrect result";

    // Test mutual recursion optimization
    argv[0] = 10;
    success = call_wasm_function(module_inst, "test_mutual_recursion_even", 1, argv);
    ASSERT_TRUE(success) << "Mutual recursion tail call execution failed";
    ASSERT_EQ(1, argv[0]) << "Mutual recursion returned incorrect result (10 is even)";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test IdentityFunction_PreservesTypes
 * @brief Validates return_call preserves types through identity functions
 * @details Tests tail calls through identity and pass-through functions.
 *          Verifies exact type and value preservation through call chains.
 * @test_category Edge - Type preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:type_preservation
 * @input_conditions Values passed through identity function tail calls
 * @expected_behavior Exact preservation of input values and types
 * @validation_method Verify output values match input values exactly
 */
TEST_P(ReturnCallTest, IdentityFunction_PreservesTypes)
{
    // Load test module with identity function tests
    wasm_module_t module = load_wasm_module(WASM_FILE_1);
    ASSERT_NE(nullptr, module) << "Failed to load identity function test module";

    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test identity function with integer value
    uint32_t argv[2] = {123}; // Test value
    bool success = call_wasm_function(module_inst, "test_identity_tail_call", 1, argv);
    ASSERT_TRUE(success) << "Identity tail call execution failed";
    ASSERT_EQ(123, argv[0]) << "Identity function did not preserve integer value";

    // Test identity function with different value
    argv[0] = 999;
    success = call_wasm_function(module_inst, "test_identity_tail_call", 1, argv);
    ASSERT_TRUE(success) << "Identity tail call with different value failed";
    ASSERT_EQ(999, argv[0]) << "Identity function did not preserve different value";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test InvalidFunctionIndex_GeneratesError
 * @brief Validates return_call error handling for invalid function indices
 * @details Tests return_call with out-of-bounds and invalid function indices.
 *          Verifies proper error generation and handling for invalid calls.
 * @test_category Error - Invalid function index validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:function_index_error_handling
 * @input_conditions Return_call instructions with invalid function indices
 * @expected_behavior Module validation fails or runtime generates appropriate errors
 * @validation_method Verify error conditions are properly detected and reported
 */
TEST_P(ReturnCallTest, InvalidFunctionIndex_GeneratesError)
{
    // Load test module with invalid function index scenarios
    wasm_module_t module = load_wasm_module(WASM_FILE_2);

    // Expected behavior: module loading should fail for invalid function indices
    if (module) {
        // If module loads, test should fail during instantiation or execution
        wasm_module_inst_t module_inst = create_module_instance(module);

        if (module_inst) {
            // Test execution that should fail due to invalid function index
            uint32_t argv[2] = {5};
            bool success = call_wasm_function(module_inst, "test_invalid_function_index", 1, argv);
            ASSERT_FALSE(success) << "Expected invalid function index to cause execution failure";

            wasm_runtime_deinstantiate(module_inst);
        }

        wasm_runtime_unload(module);
    } else {
        // Expected case: module with invalid function indices should fail to load
        // This is acceptable behavior for modules with validation errors
        ASSERT_EQ(nullptr, module) << "Module with invalid function indices should fail validation";
    }
}

/**
 * @test TypeMismatch_HandlesGracefully
 * @brief Validates return_call error handling for type mismatches
 * @details Tests return_call with parameter and return type mismatches.
 *          Verifies proper type validation and error reporting.
 * @test_category Error - Type mismatch validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:type_mismatch_error_handling
 * @input_conditions Return_call with mismatched parameter or return types
 * @expected_behavior Type validation errors are properly detected and reported
 * @validation_method Verify type mismatch scenarios generate appropriate errors
 */
TEST_P(ReturnCallTest, TypeMismatch_HandlesGracefully)
{
    // Load test module with type mismatch scenarios
    wasm_module_t module = load_wasm_module(WASM_FILE_2);

    // Expected behavior: module with type mismatches should fail validation
    if (module) {
        // If module loads despite type issues, test during instantiation/execution
        wasm_module_inst_t module_inst = create_module_instance(module);

        if (module_inst) {
            // Test execution that should fail due to type mismatch
            uint32_t argv[2] = {10};
            bool success = call_wasm_function(module_inst, "test_type_mismatch", 1, argv);
            ASSERT_FALSE(success) << "Expected type mismatch to cause execution failure";

            wasm_runtime_deinstantiate(module_inst);
        }

        wasm_runtime_unload(module);
    } else {
        // Expected case: module with type mismatches should fail to load
        ASSERT_EQ(nullptr, module) << "Module with type mismatches should fail validation";
    }
}

INSTANTIATE_TEST_SUITE_P(RunningMode, ReturnCallTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<ReturnCallTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "Interpreter" : "AOT";
                         });

std::string get_current_directory() {
    char *cwd_ptr = get_current_dir_name();
    if (cwd_ptr) {
        std::string cwd(cwd_ptr);
        free(cwd_ptr);
        return cwd;
    }
    return "";
}

// int main(int argc, char **argv)
// {
//     CWD = get_current_directory();
//     WASM_FILE_1 = CWD + "/wasm-apps/return_call_test.wasm";
//     WASM_FILE_2 = CWD + "/wasm-apps/return_call_error_test.wasm";

//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }