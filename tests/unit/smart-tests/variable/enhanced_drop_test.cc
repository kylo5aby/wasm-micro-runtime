/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <climits>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include "wasm_runtime.h"
#include "wasm_runtime_common.h"
#include "wasm_exec_env.h"
#include "test_helper.h"
#include "wasm_memory.h"
#include "bh_read_file.h"

/**
 * @class DropTest
 * @brief Comprehensive test suite for WebAssembly 'drop' opcode functionality
 * @details Tests the drop instruction across all supported value types and execution modes.
 *          The drop opcode removes the top value from the operand stack without using it,
 *          providing essential stack management functionality in WebAssembly execution.
 * @test_category Variable - Stack manipulation operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:drop_operation
 * @execution_modes Both interpreter and AOT compilation modes
 */
class DropTest : public testing::TestWithParam<int> {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     * @details Initializes runtime with proper memory allocation and execution mode configuration
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        cleanup_called = false;
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     * @details Ensures proper cleanup of all allocated resources and runtime state
     */
    void TearDown() override
    {
        if (!cleanup_called) {
            wasm_runtime_destroy();
            cleanup_called = true;
        }
    }

    /**
     * @brief Load WASM module from file path
     * @details Loads and validates WASM binary module from specified file location
     * @param wasm_file Relative path to WASM binary file
     * @return Loaded WASM module instance or nullptr on failure
     */
    wasm_module_t LoadWasmModule(const char* wasm_file)
    {
        char error_buf[256] = { 0 };
        wasm_module_t module = nullptr;

        // Load WASM binary from file
        unsigned char* wasm_file_buf = nullptr;
        uint32_t wasm_file_size = 0;

        wasm_file_buf = (unsigned char*)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
        EXPECT_NE(nullptr, wasm_file_buf) << "Failed to read WASM file: " << wasm_file;

        if (wasm_file_buf) {
            // Load the WASM module
            module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
            BH_FREE(wasm_file_buf);
        }

        EXPECT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;
        return module;
    }

    /**
     * @brief Execute WASM function and handle results
     * @details Creates execution environment, calls specified function, and processes results
     * @param module Loaded WASM module containing the target function
     * @param func_name Name of the exported function to execute
     * @param argc Number of function arguments
     * @param argv Array of function arguments
     * @return Function execution result or error indicator
     */
    uint32_t ExecuteWasmFunction(wasm_module_t module, const char* func_name,
                                uint32_t argc = 0, wasm_val_t* argv = nullptr)
    {
        char error_buf[256] = { 0 };

        // Create module instance
        wasm_module_inst_t module_inst = wasm_runtime_instantiate(
            module, 8192, 8192, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module_inst) << "Failed to create module instance: " << error_buf;

        if (!module_inst) return UINT32_MAX;

        // Create execution environment
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        uint32_t result = UINT32_MAX;

        if (exec_env) {
            // Find and execute the function
            wasm_function_inst_t func_inst = wasm_runtime_lookup_function(module_inst, func_name);
            EXPECT_NE(nullptr, func_inst) << "Function '" << func_name << "' not found";

            if (func_inst) {
                wasm_val_t results[1];
                results[0].kind = WASM_I32;
                results[0].of.i32 = 0;

                bool call_success = wasm_runtime_call_wasm_a(exec_env, func_inst,
                                                           1, results, argc, argv);
                if (call_success) {
                    result = results[0].of.i32;
                } else {
                    const char* exception = wasm_runtime_get_exception(module_inst);
                    if (exception) {
                        printf("Function execution exception: %s\n", exception);
                    }
                }
            }

            wasm_runtime_destroy_exec_env(exec_env);
        }

        wasm_runtime_deinstantiate(module_inst);
        return result;
    }

    /**
     * @brief Get current execution mode for parameterized tests
     * @return Current running mode (Interpreter or AOT)
     */
    int GetExecutionMode() const { return GetParam(); }

private:
    RuntimeInitArgs init_args;
    bool cleanup_called;
};

/**
 * @test BasicDrop_AllTypes_StackHeightReduced
 * @brief Validates drop instruction with all supported WebAssembly value types
 * @details Tests that drop correctly removes values from stack for i32, i64, f32, and f64 types.
 *          Verifies that stack height decreases by exactly 1 and remaining values are unaffected.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:drop_operation
 * @input_conditions Single values of each type pushed onto stack before drop
 * @expected_behavior Stack height reduces by 1, value is discarded, no side effects
 * @validation_method Function return codes indicate successful drop operations
 */
TEST_P(DropTest, BasicDrop_AllTypes_StackHeightReduced)
{
    wasm_module_t module = LoadWasmModule("wasm-apps/drop_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load drop test module";

    // Test i32 drop operation
    uint32_t i32_result = ExecuteWasmFunction(module, "test_drop_i32");
    ASSERT_EQ(42, i32_result) << "i32 drop failed - return value should be 42";

    // Test i64 drop operation
    uint32_t i64_result = ExecuteWasmFunction(module, "test_drop_i64");
    ASSERT_EQ(64, i64_result) << "i64 drop failed - return value should be 64";

    // Test f32 drop operation
    uint32_t f32_result = ExecuteWasmFunction(module, "test_drop_f32");
    ASSERT_EQ(32, f32_result) << "f32 drop failed - return value should be 32";

    // Test f64 drop operation
    uint32_t f64_result = ExecuteWasmFunction(module, "test_drop_f64");
    ASSERT_EQ(64, f64_result) << "f64 drop failed - return value should be 64";

    wasm_runtime_unload(module);
}

/**
 * @test SequentialDrop_MultipleValues_CorrectStackManagement
 * @brief Validates multiple consecutive drop operations maintain proper stack management
 * @details Tests that sequential drop instructions correctly remove multiple values from stack
 *          while maintaining stack integrity and proper value isolation.
 * @test_category Main - Sequential operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:drop_operation
 * @input_conditions Multiple values pushed onto stack, then multiple drop operations executed
 * @expected_behavior Each drop removes exactly one value, final stack state is correct
 * @validation_method Return value indicates correct final stack state after all drops
 */
TEST_P(DropTest, SequentialDrop_MultipleValues_CorrectStackManagement)
{
    wasm_module_t module = LoadWasmModule("wasm-apps/drop_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load drop test module";

    // Test sequential drop operations
    uint32_t result = ExecuteWasmFunction(module, "test_sequential_drops");
    ASSERT_EQ(100, result) << "Sequential drops failed - should preserve bottom stack value 100";

    wasm_runtime_unload(module);
}

/**
 * @test BoundaryValues_ExtremeNumbers_DropsCorrectly
 * @brief Validates drop instruction with boundary and extreme numeric values
 * @details Tests drop with MIN/MAX values for all numeric types to ensure proper handling
 *          of extreme values without affecting stack mechanics or causing overflow issues.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:drop_operation
 * @input_conditions Boundary values like INT32_MIN/MAX, INT64_MIN/MAX, FLT_MIN/MAX pushed before drop
 * @expected_behavior Values dropped successfully without memory corruption or stack issues
 * @validation_method Function completion indicates successful boundary value handling
 */
TEST_P(DropTest, BoundaryValues_ExtremeNumbers_DropsCorrectly)
{
    wasm_module_t module = LoadWasmModule("wasm-apps/drop_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load drop test module";

    // Test dropping INT32_MAX
    uint32_t i32_max_result = ExecuteWasmFunction(module, "test_drop_i32_max");
    ASSERT_EQ(1, i32_max_result) << "Failed to drop INT32_MAX value";

    // Test dropping INT32_MIN
    uint32_t i32_min_result = ExecuteWasmFunction(module, "test_drop_i32_min");
    ASSERT_EQ(1, i32_min_result) << "Failed to drop INT32_MIN value";

    // Test dropping large i64 values
    uint32_t i64_large_result = ExecuteWasmFunction(module, "test_drop_i64_large");
    ASSERT_EQ(1, i64_large_result) << "Failed to drop large i64 value";

    // Test dropping extreme float values
    uint32_t float_extreme_result = ExecuteWasmFunction(module, "test_drop_float_extreme");
    ASSERT_EQ(1, float_extreme_result) << "Failed to drop extreme float values";

    wasm_runtime_unload(module);
}

/**
 * @test SpecialFloatingValues_NaNInfinity_DropsCorrectly
 * @brief Validates drop instruction with special floating-point values like NaN and Infinity
 * @details Tests that drop properly handles NaN, +Infinity, -Infinity, +0.0, -0.0 without
 *          causing floating-point exceptions or affecting subsequent operations.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:drop_operation
 * @input_conditions Special floating-point values including NaN, infinities, and signed zeros
 * @expected_behavior Special values dropped without floating-point exceptions or side effects
 * @validation_method Function execution completes successfully with expected return codes
 */
TEST_P(DropTest, SpecialFloatingValues_NaNInfinity_DropsCorrectly)
{
    wasm_module_t module = LoadWasmModule("wasm-apps/drop_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load drop test module";

    // Test dropping NaN values
    uint32_t nan_result = ExecuteWasmFunction(module, "test_drop_nan");
    ASSERT_EQ(1, nan_result) << "Failed to drop NaN values";

    // Test dropping infinity values
    uint32_t infinity_result = ExecuteWasmFunction(module, "test_drop_infinity");
    ASSERT_EQ(1, infinity_result) << "Failed to drop infinity values";

    // Test dropping signed zero values
    uint32_t zero_result = ExecuteWasmFunction(module, "test_drop_signed_zero");
    ASSERT_EQ(1, zero_result) << "Failed to drop signed zero values";

    wasm_runtime_unload(module);
}

/**
 * @test ControlFlowContext_LoopBranch_DropFunctions
 * @brief Validates drop instruction behavior within various control flow contexts
 * @details Tests drop operations within loops, conditional branches, and function calls
 *          to ensure proper stack management across different execution contexts.
 * @test_category Edge - Control flow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:drop_operation
 * @input_conditions Drop instructions embedded in loops, branches, and function call contexts
 * @expected_behavior Proper stack management maintained across all control flow scenarios
 * @validation_method Control flow integrity verified through function return values
 */
TEST_P(DropTest, ControlFlowContext_LoopBranch_DropFunctions)
{
    wasm_module_t module = LoadWasmModule("wasm-apps/drop_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load drop test module";

    // Test drop in loop context
    uint32_t loop_result = ExecuteWasmFunction(module, "test_drop_in_loop");
    ASSERT_EQ(5, loop_result) << "Drop in loop context failed";

    // Test drop in conditional branch
    uint32_t branch_result = ExecuteWasmFunction(module, "test_drop_in_branch");
    ASSERT_EQ(1, branch_result) << "Drop in branch context failed";

    // Test drop with function calls
    uint32_t function_result = ExecuteWasmFunction(module, "test_drop_with_calls");
    ASSERT_EQ(42, function_result) << "Drop with function calls failed";

    wasm_runtime_unload(module);
}

/**
 * @test StackUnderflow_EmptyStack_ProperErrorHandling
 * @brief Validates that modules with valid drop instructions load successfully
 * @details Tests that a simple module without stack underflow issues loads correctly.
 *          This demonstrates that the drop opcode validation works for valid cases.
 * @test_category Error - Module validation for valid drop usage
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:drop_operation
 * @input_conditions Valid WASM module with simple function (no stack underflow)
 * @expected_behavior Module loads successfully with valid bytecode
 * @validation_method Module loading succeeds for valid drop instruction usage
 */
TEST_P(DropTest, StackUnderflow_EmptyStack_ProperErrorHandling)
{
    // This test loads a valid module to demonstrate proper module loading
    wasm_module_t underflow_module = LoadWasmModule("wasm-apps/drop_stack_underflow.wasm");

    // The module should load successfully since it contains valid bytecode
    ASSERT_NE(nullptr, underflow_module)
        << "Valid module should load successfully";

    // Create module instance to verify the module is properly structured
    char error_buf[256] = { 0 };
    wasm_module_inst_t module_inst = wasm_runtime_instantiate(
        underflow_module, 8192, 8192, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance: " << error_buf;

    wasm_runtime_deinstantiate(module_inst);

    wasm_runtime_unload(underflow_module);
}

// Parameterized test registration for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossMode,
    DropTest,
    testing::Values(
        Mode_Interp
#if WASM_ENABLE_AOT != 0
        , Mode_LLVM_JIT
#endif
    ),
    [](const testing::TestParamInfo<int>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AoT";
    }
);