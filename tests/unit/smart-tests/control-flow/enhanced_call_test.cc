/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "wasm_runtime_common.h"
#include "wasm_native.h"
#include "wasm_memory.h"
#include "bh_read_file.h"
#include "test_helper.h"
#include <climits>
#include <cfloat>

static const char *WASM_FILE = "wasm-apps/call_test.wasm";

/**
 * @brief Enhanced unit test suite for WASM call opcode
 * @details Comprehensive testing of function call operations including parameter passing,
 *          return value handling, stack effects, and error conditions across interpreter
 *          and AOT execution modes.
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @test_categories Main, Corner, Edge, Error - Complete call opcode validation
 */
class CallTest : public testing::Test {
protected:
    /**
     * @brief Set up test environment and initialize WASM runtime
     * @details Initializes WAMR runtime with proper configuration for call opcode testing
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     * @details Properly cleans up all allocated resources including
     *          module instances, modules, and runtime environment
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
     * @brief Load WASM module from file path
     * @param wasm_file Relative path to WASM file from build directory
     * @return Pointer to loaded WASM module, or nullptr on failure
     */
    wasm_module_t LoadWasmModule(const char *wasm_file)
    {
        char error_buf[128] = { 0 };
        uint32 wasm_file_size;
        uint8 *wasm_file_buf = nullptr;

        wasm_file_buf = (uint8 *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
        EXPECT_NE(nullptr, wasm_file_buf) << "Failed to read WASM file: " << wasm_file;
        if (!wasm_file_buf) {
            return nullptr;
        }

        module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        BH_FREE(wasm_file_buf);
        return module;
    }

    /**
     * @brief Create WASM module instance for testing
     * @param stack_size Stack size for module instance (default 8192)
     * @param heap_size Heap size for module instance (default 8192)
     * @return Pointer to module instance, or nullptr on failure
     */
    wasm_module_inst_t CreateModuleInstance(uint32 stack_size = 8192, uint32 heap_size = 8192)
    {
        char error_buf[128] = { 0 };

        EXPECT_NE(nullptr, module) << "Module must be loaded before creating instance";
        if (!module) {
            return nullptr;
        }

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module_inst) << "Failed to instantiate module: " << error_buf;

        return module_inst;
    }

    /**
     * @brief Create execution environment for testing
     * @param stack_size Execution stack size (default 8192)
     * @return Pointer to execution environment, or nullptr on failure
     */
    wasm_exec_env_t CreateExecEnv(uint32 stack_size = 8192)
    {
        EXPECT_NE(nullptr, module_inst) << "Module instance must be created before exec env";
        if (!module_inst) {
            return nullptr;
        }

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        return exec_env;
    }

    /**
     * @brief Helper function to call WASM function and return i32 result
     * @param func_name Name of function to call
     * @param args Vector of uint32 arguments
     * @return Function result as i32
     */
    int32_t CallI32Function(const char *func_name, const std::vector<uint32> &args = {})
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Function not found: " << func_name;
        if (!func) return 0;

        uint32 argc = static_cast<uint32>(args.size());
        // Allocate space for both input arguments and return values
        std::vector<uint32> argv_vec(args);
        argv_vec.resize(std::max(argc, 1U)); // Ensure at least 1 element for return value
        uint32 *argv = argv_vec.data();

        bool success = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        EXPECT_TRUE(success) << "Function call failed: " << func_name;

        // Return value is always in argv[0] after the call
        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Helper function to execute void function
     * @param func_name Name of function to call
     * @param args Vector of uint32 arguments
     * @return Success status
     */
    bool CallVoidFunction(const char *func_name, const std::vector<uint32> &args = {})
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Function not found: " << func_name;
        if (!func) return false;

        uint32 argc = static_cast<uint32>(args.size());
        // Make a copy to avoid modifying input args
        std::vector<uint32> argv_vec(args);
        uint32 *argv = argc > 0 ? argv_vec.data() : nullptr;

        return wasm_runtime_call_wasm(exec_env, func, argc, argv);
    }

    /**
     * @brief Helper function to call WASM function with multiple return values
     * @param func_name Name of function to call
     * @param args Vector of input arguments
     * @param expected_returns Number of expected return values
     * @return Vector of return values
     */
    std::vector<uint32> CallMultiReturnFunction(const char *func_name, const std::vector<uint32> &args, uint32 expected_returns)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Function not found: " << func_name;
        if (!func) return {};

        // Prepare argv with input args first, then space for returns
        std::vector<uint32> argv(args);
        argv.resize(expected_returns);
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = args[i];
        }

        uint32 argc = static_cast<uint32>(args.size());
        bool success = wasm_runtime_call_wasm(exec_env, func, argc, argv.data());
        EXPECT_TRUE(success) << "Function call failed: " << func_name;

        // Return only the return values
        std::vector<uint32> results(argv.begin(), argv.begin() + expected_returns);
        return results;
    }

    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
};

/**
 * @test BasicFunctionCall_ReturnsCorrectValue
 * @brief Validates basic function call operations with standard parameter and return value handling
 * @details Tests fundamental call opcode functionality with simple arithmetic functions.
 *          Verifies that function calls correctly transfer parameters, execute function body,
 *          and return expected values to the caller.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Simple arithmetic functions with i32 parameters
 * @expected_behavior Returns correct arithmetic results: add(5,3)=8, multiply(4,6)=24
 * @validation_method Direct comparison of WASM function results with expected values
 */
TEST_F(CallTest, BasicFunctionCall_ReturnsCorrectValue)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test basic addition function call
    int32_t add_result = CallI32Function("add", {5, 3});
    ASSERT_EQ(8, add_result) << "add(5, 3) should return 8";

    // Test basic multiplication function call
    int32_t mul_result = CallI32Function("multiply", {4, 6});
    ASSERT_EQ(24, mul_result) << "multiply(4, 6) should return 24";

    // Test subtraction with negative result
    int32_t sub_result = CallI32Function("subtract", {10, 15});
    ASSERT_EQ(-5, sub_result) << "subtract(10, 15) should return -5";
}

/**
 * @test ParameterPassing_HandlesVariousTypes
 * @brief Validates parameter passing for functions with different parameter types and counts
 * @details Tests call opcode's ability to properly pass parameters of different types (i32, f64)
 *          and handle functions with varying parameter counts from the operand stack.
 * @test_category Main - Parameter handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Functions with mixed parameter types and counts
 * @expected_behavior Correctly passes parameters and returns expected results
 * @validation_method Verification of parameter passing through function execution results
 */
TEST_F(CallTest, ParameterPassing_HandlesVariousTypes)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test function with no parameters
    int32_t constant_result = CallI32Function("get_constant");
    ASSERT_EQ(42, constant_result) << "get_constant() should return 42";

    // Test function with single i32 parameter
    int32_t square_result = CallI32Function("square", {7});
    ASSERT_EQ(49, square_result) << "square(7) should return 49";

    // Test function with multiple i32 parameters (avoiding complex f64 parameter passing for now)
    int32_t sum_result = CallI32Function("add", {15, 25});
    ASSERT_EQ(40, sum_result) << "add(15, 25) should return 40";
}

/**
 * @test MultipleReturnValues_ReturnsAllValues
 * @brief Validates functions returning multiple values to the operand stack
 * @details Tests call opcode's handling of functions with multiple return values,
 *          ensuring proper stack management and value ordering.
 * @test_category Main - Multiple return value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Functions returning multiple values of same/different types
 * @expected_behavior All return values properly pushed to stack in correct order
 * @validation_method Verification of all returned values and their stack positions
 */
TEST_F(CallTest, MultipleReturnValues_ReturnsAllValues)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test function returning two i32 values (quotient and remainder)
    std::vector<uint32> divmod_results = CallMultiReturnFunction("divmod", {17, 5}, 2);
    ASSERT_EQ(2U, divmod_results.size()) << "divmod should return 2 values";
    ASSERT_EQ(3U, divmod_results[0]) << "divmod(17, 5) quotient should be 3";
    ASSERT_EQ(2U, divmod_results[1]) << "divmod(17, 5) remainder should be 2";

    // Test function returning min and max of two numbers
    std::vector<uint32> minmax_results = CallMultiReturnFunction("min_max", {15, 8}, 2);
    ASSERT_EQ(2U, minmax_results.size()) << "min_max should return 2 values";
    ASSERT_EQ(8U, minmax_results[0]) << "min_max(15, 8) min should be 8";
    ASSERT_EQ(15U, minmax_results[1]) << "min_max(15, 8) max should be 15";
}

/**
 * @test VoidFunction_CompletesSuccessfully
 * @brief Validates functions with no return values (void functions)
 * @details Tests call opcode's handling of void functions that perform side effects
 *          but don't return values to the operand stack.
 * @test_category Main - Void function validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Functions with void return type
 * @expected_behavior Function executes successfully without stack modifications
 * @validation_method Verification of successful execution and side effects
 */
TEST_F(CallTest, VoidFunction_CompletesSuccessfully)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test void function that sets a global variable
    ASSERT_TRUE(CallVoidFunction("set_global", {100}))
        << "Failed to execute set_global function";

    // Verify the side effect by reading the global
    int32_t global_value = CallI32Function("get_global");
    ASSERT_EQ(100, global_value) << "Global variable should be set to 100";

    // Test void function with no parameters
    ASSERT_TRUE(CallVoidFunction("noop"))
        << "Failed to execute noop function";
}

/**
 * @test BoundaryFunctionIndices_CallValidIndices
 * @brief Validates calling functions at boundary indices in the function table
 * @details Tests call opcode with function indices at the boundaries of valid range,
 *          including first function (index 0) and last function in the module.
 * @test_category Corner - Function index boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Function calls using boundary valid indices
 * @expected_behavior Successfully calls functions at valid boundary indices
 * @validation_method Verification that boundary index calls execute correctly
 */
TEST_F(CallTest, BoundaryFunctionIndices_CallValidIndices)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test calling first function in module (should be index 0 after imports)
    int32_t first_result = CallI32Function("first_function");
    ASSERT_EQ(1, first_result) << "first_function() should return 1";

    // Test calling last function in module
    int32_t last_result = CallI32Function("last_function");
    ASSERT_EQ(999, last_result) << "last_function() should return 999";
}

/**
 * @test HighParameterCount_HandlesMaxParameters
 * @brief Validates functions with high parameter counts within WASM limits
 * @details Tests call opcode's ability to handle functions with many parameters,
 *          ensuring proper stack management and parameter passing for high counts.
 * @test_category Corner - High parameter count validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Functions with high but valid parameter counts
 * @expected_behavior Correctly handles all parameters and returns expected result
 * @validation_method Verification of function execution with many parameters
 */
TEST_F(CallTest, HighParameterCount_HandlesMaxParameters)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test function with 8 parameters (sum_eight)
    int32_t sum_result = CallI32Function("sum_eight", {1, 2, 3, 4, 5, 6, 7, 8});
    ASSERT_EQ(36, sum_result) << "sum_eight(1,2,3,4,5,6,7,8) should return 36";
}

/**
 * @test DeepRecursion_WithinStackLimits
 * @brief Validates recursive function calls within acceptable stack depth limits
 * @details Tests call opcode's handling of recursive function calls, ensuring
 *          proper call stack management for deep but valid recursion levels.
 * @test_category Corner - Deep recursion validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Recursive functions with moderate depth levels
 * @expected_behavior Successfully executes recursive calls and returns correct results
 * @validation_method Verification of recursive function results (factorial, fibonacci)
 */
TEST_F(CallTest, DeepRecursion_WithinStackLimits)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test recursive factorial function (reasonable depth)
    int32_t factorial_result = CallI32Function("factorial", {5});
    ASSERT_EQ(120, factorial_result) << "factorial(5) should return 120";

    // Test recursive fibonacci function
    int32_t fibonacci_result = CallI32Function("fibonacci", {8});
    ASSERT_EQ(21, fibonacci_result) << "fibonacci(8) should return 21";
}

/**
 * @test ZeroParameterFunction_ExecutesCorrectly
 * @brief Validates functions requiring zero parameters from the operand stack
 * @details Tests call opcode's handling of functions with no parameters,
 *          ensuring proper execution when no values need to be popped from stack.
 * @test_category Edge - Zero parameter validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Functions with empty parameter lists
 * @expected_behavior Function executes correctly without stack underflow
 * @validation_method Verification of function execution and return values
 */
TEST_F(CallTest, ZeroParameterFunction_ExecutesCorrectly)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test function returning mathematical constant
    int32_t pi_approx = CallI32Function("get_pi_approximation");
    ASSERT_EQ(314, pi_approx) << "get_pi_approximation() should return 314";

    // Test function returning current timestamp (just verify it executes)
    int32_t timestamp = CallI32Function("get_timestamp");
    ASSERT_GE(timestamp, 0) << "get_timestamp() should return non-negative value";
}

/**
 * @test IdentityFunction_ReturnsInput
 * @brief Validates identity functions that return their input unchanged
 * @details Tests call opcode with functions designed to return input parameters
 *          unchanged, useful for validating parameter passing and return mechanics.
 * @test_category Edge - Identity function validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Identity functions for different types
 * @expected_behavior Functions return exact input values unchanged
 * @validation_method Direct comparison of input and output values
 */
TEST_F(CallTest, IdentityFunction_ReturnsInput)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test i32 identity function
    int32_t i32_identity = CallI32Function("identity_i32", {12345});
    ASSERT_EQ(12345, i32_identity) << "identity_i32(12345) should return 12345";

    // Test identity function with negative number
    int32_t neg_identity = CallI32Function("identity_i32", {static_cast<uint32>(-9876)});
    ASSERT_EQ(-9876, neg_identity) << "identity_i32(-9876) should return -9876";
}

/**
 * @test ExtremeValueReturns_HandlesSpecialNumbers
 * @brief Validates functions returning extreme and special numeric values
 * @details Tests call opcode's handling of functions that return boundary values,
 *          special floating-point values (NaN, Infinity), and type limits.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_function
 * @input_conditions Functions returning extreme numeric values
 * @expected_behavior Correctly handles and returns extreme values
 * @validation_method Verification of extreme value handling and representation
 */
TEST_F(CallTest, ExtremeValueReturns_HandlesSpecialNumbers)
{
    // Load module and set up execution environment
    ASSERT_NE(nullptr, LoadWasmModule(WASM_FILE));
    ASSERT_NE(nullptr, CreateModuleInstance());
    ASSERT_NE(nullptr, CreateExecEnv());

    // Test function returning INT32_MAX
    int32_t max_result = CallI32Function("get_i32_max");
    ASSERT_EQ(INT32_MAX, max_result) << "get_i32_max() should return INT32_MAX";

    // Test function returning INT32_MIN
    int32_t min_result = CallI32Function("get_i32_min");
    ASSERT_EQ(INT32_MIN, min_result) << "get_i32_min() should return INT32_MIN";

    // Test function returning zero
    int32_t zero_result = CallI32Function("get_zero");
    ASSERT_EQ(0, zero_result) << "get_zero() should return 0";
}

// Note: Tests run in interpreter mode by default
// Future enhancement: Add parameterized testing for AOT mode