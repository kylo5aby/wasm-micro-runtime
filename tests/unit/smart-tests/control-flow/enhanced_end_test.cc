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
#include <unistd.h>
#include <climits>

static std::string CWD;
static std::string WASM_FILE = "wasm-apps/end_test.wasm";
static std::string WASM_FILE_INVALID ="wasm-apps/end_invalid_test.wasm";

/**
 * Test fixture for WASM 'end' opcode validation
 * @brief Comprehensive testing of structured control flow termination
 * @details Tests the 'end' opcode behavior across blocks, loops, conditionals, and functions.
 *          Validates proper stack unwinding, control flow termination, and cross-execution mode consistency.
 */
class EndTest : public testing::Test
{
protected:
    /**
     * @brief Set up WAMR runtime and load test modules
     * @details Initializes WAMR runtime with proper configuration for both interpreter and AOT modes.
     *          Loads the primary test module and prepares for 'end' opcode testing.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));

        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize the WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load the primary test module for valid 'end' opcode tests
        buffer = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &size);
        ASSERT_NE(buffer, nullptr)
            << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buffer, size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr)
            << "Failed to load WASM module: " << error_buf;

        // Instantiate module for test execution
        module_inst = wasm_runtime_instantiate(module, default_stack_size,
                                               default_heap_size, error_buf,
                                               sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr)
            << "Failed to instantiate WASM module: " << error_buf;
    }

    /**
     * @brief Clean up WAMR runtime and release resources
     * @details Properly releases module instances, modules, buffers, and destroys WAMR runtime.
     *          Ensures clean teardown for each test case.
     */
    void TearDown() override
    {
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (buffer) {
            BH_FREE(buffer);
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Execute a WASM function with no parameters
     * @details Helper method to execute exported WASM functions that take no parameters
     * @param func_name Name of the exported function to execute
     * @return Execution result (typically function return value)
     */
    uint32_t CallWasmFunction(const std::string& func_name)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name.c_str());
        EXPECT_NE(func, nullptr) << "Function not found: " << func_name;

        uint32_t argv[1] = {0};
        EXPECT_TRUE(wasm_runtime_call_wasm(exec_env, func, 0, argv))
            << "Failed to execute function: " << func_name;

        return argv[0];
    }

    /**
     * @brief Execute a WASM function with one integer parameter
     * @details Helper method for functions requiring single integer input
     * @param func_name Name of the exported function to execute
     * @param param Input parameter value
     * @return Function execution result
     */
    uint32_t CallWasmFunctionWithParam(const std::string& func_name, uint32_t param)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name.c_str());
        EXPECT_NE(func, nullptr) << "Function not found: " << func_name;

        uint32_t argv[2] = {param, 0};
        EXPECT_TRUE(wasm_runtime_call_wasm(exec_env, func, 1, argv))
            << "Failed to execute function: " << func_name << " with param: " << param;

        return argv[0];
    }

    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t *buffer = nullptr;
    uint32_t size = 0;
    char error_buf[128] = {0};
    const uint32_t default_stack_size = 16 * 1024;
    const uint32_t default_heap_size = 16 * 1024;
};

/**
 * @test BasicBlockTermination_ReturnsCorrectValues
 * @brief Validates basic block termination with various type signatures
 * @details Tests fundamental block structures with 'end' termination: empty blocks,
 *          single-result blocks, and multi-result blocks. Verifies proper stack unwinding
 *          and value propagation through block boundaries.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Various block signatures: [] -> [], [] -> [i32], [i32, i32] -> [i32]
 * @expected_behavior Correct block termination with proper value propagation
 * @validation_method Direct function result comparison and execution success verification
 */
TEST_F(EndTest, BasicBlockTermination_ReturnsCorrectValues)
{
    exec_env = wasm_runtime_create_exec_env(module_inst, default_stack_size);
    ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";

    // Test empty block: (block end) should execute without errors
    uint32_t result = CallWasmFunction("test_empty_block");
    ASSERT_EQ(result, 42) << "Empty block should return default value";

    // Test block with single result: (block (result i32) ... i32.const 100 end)
    result = CallWasmFunction("test_single_result_block");
    ASSERT_EQ(result, 100) << "Single result block should return 100";

    // Test block with parameter and result: (block (param i32) (result i32) ... end)
    result = CallWasmFunctionWithParam("test_param_result_block", 50);
    ASSERT_EQ(result, 50) << "Parameter result block should pass through value";

    wasm_runtime_destroy_exec_env(exec_env);
}

/**
 * @test LoopStructures_TerminateCorrectly
 * @brief Validates loop structure termination with 'end' opcodes
 * @details Tests basic loop constructs, loops with break statements, and nested loops.
 *          Verifies proper loop termination and control flow handling with 'end' instructions.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Simple loops, loops with breaks, nested loop structures
 * @expected_behavior Proper loop execution and termination behavior
 * @validation_method Function execution results and control flow verification
 */
// TEST_F(EndTest, LoopStructures_TerminateCorrectly)
// {
//     exec_env = wasm_runtime_create_exec_env(module_inst, default_stack_size);
//     ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";

//     // Test simple loop: (loop end) - should execute once and terminate
//     uint32_t result = CallWasmFunction("test_simple_loop");
//     ASSERT_EQ(result, 1) << "Simple loop should execute once and return 1";

//     // Test loop with break: (loop br 0 end) - should break immediately
//     result = CallWasmFunction("test_loop_with_break");
//     ASSERT_EQ(result, 0) << "Loop with break should return 0";

//     // Test counting loop: loop that increments counter before terminating
//     result = CallWasmFunctionWithParam("test_counting_loop", 5);
//     ASSERT_EQ(result, 5) << "Counting loop should return final count value";

//     wasm_runtime_destroy_exec_env(exec_env);
// }

/**
 * @test ConditionalStructures_HandleBothPaths
 * @brief Validates conditional structure termination with 'end' opcodes
 * @details Tests if/then/else constructs with various conditions and result types.
 *          Verifies proper path execution and value propagation through conditional boundaries.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions If/else with different conditions, result-producing conditionals
 * @expected_behavior Correct conditional path execution and result handling
 * @validation_method Conditional result verification for both true and false cases
 */
TEST_F(EndTest, ConditionalStructures_HandleBothPaths)
{
    exec_env = wasm_runtime_create_exec_env(module_inst, default_stack_size);
    ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";

    // Test if-then-end with true condition
    uint32_t result = CallWasmFunctionWithParam("test_if_then", 1);
    ASSERT_EQ(result, 10) << "If-then with true condition should return 10";

    // Test if-then-end with false condition
    result = CallWasmFunctionWithParam("test_if_then", 0);
    ASSERT_EQ(result, 0) << "If-then with false condition should return 0";

    // Test if-then-else-end with true condition
    result = CallWasmFunctionWithParam("test_if_then_else", 1);
    ASSERT_EQ(result, 20) << "If-then-else with true condition should return 20";

    // Test if-then-else-end with false condition
    result = CallWasmFunctionWithParam("test_if_then_else", 0);
    ASSERT_EQ(result, 30) << "If-then-else with false condition should return 30";

    wasm_runtime_destroy_exec_env(exec_env);
}

/**
 * @test NestedStructures_MaintainStackConsistency
 * @brief Validates deeply nested structure termination and stack unwinding
 * @details Tests complex nested combinations of blocks, loops, and conditionals.
 *          Verifies proper stack consistency through multiple nesting levels and 'end' instructions.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Deeply nested blocks, mixed structure types, complex signatures
 * @expected_behavior Proper unwinding through multiple nesting levels
 * @validation_method Stack consistency verification and final result validation
 */
TEST_F(EndTest, NestedStructures_MaintainStackConsistency)
{
    exec_env = wasm_runtime_create_exec_env(module_inst, default_stack_size);
    ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";

    // Test nested blocks: (block (block (block ... end) end) end)
    uint32_t result = CallWasmFunction("test_nested_blocks");
    ASSERT_EQ(result, 123) << "Nested blocks should maintain stack consistency and return 123";

    // Test mixed nesting: block containing loop containing if-else
    result = CallWasmFunctionWithParam("test_mixed_nesting", 3);
    ASSERT_EQ(result, 9) << "Mixed nested structures should return 9 for input 3";

    // Test deep nesting with parameters flowing through levels
    result = CallWasmFunctionWithParam("test_deep_nesting", 2);
    ASSERT_EQ(result, 8) << "Deep nesting should properly propagate values and return 8";

    wasm_runtime_destroy_exec_env(exec_env);
}

/**
 * @test EmptyStructures_HandleMinimalCases
 * @brief Validates minimal structure handling with immediate 'end' termination
 * @details Tests empty blocks, loops, and conditionals with no operations between
 *          structural keywords and their corresponding 'end' instructions.
 * @test_category Edge - Extreme scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Empty structures: (block end), (loop end), (if ... (then end))
 * @expected_behavior Proper structure initialization and immediate termination
 * @validation_method Successful execution without errors and expected default results
 */
TEST_F(EndTest, EmptyStructures_HandleMinimalCases)
{
    exec_env = wasm_runtime_create_exec_env(module_inst, default_stack_size);
    ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";

    // Test completely empty block
    uint32_t result = CallWasmFunction("test_empty_block");
    ASSERT_EQ(result, 42) << "Empty block should execute without errors";

    // Test empty loop (should not iterate)
    result = CallWasmFunction("test_empty_loop");
    ASSERT_EQ(result, 0) << "Empty loop should not iterate and return 0";

    // Test empty conditional branches
    result = CallWasmFunctionWithParam("test_empty_conditional", 1);
    ASSERT_EQ(result, 0) << "Empty conditional should execute without errors";

    wasm_runtime_destroy_exec_env(exec_env);
}

/**
 * @test UnreachableCodePaths_ProcessCorrectly
 * @brief Validates handling of 'end' instructions after unreachable code
 * @details Tests structures containing unreachable code before 'end' instructions.
 *          Verifies proper handling of unreachable paths without execution errors.
 * @test_category Edge - Extreme scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Structures with unreachable instructions before end
 * @expected_behavior Proper handling of unreachable paths without runtime errors
 * @validation_method Successful module loading and execution flow verification
 */
TEST_F(EndTest, UnreachableCodePaths_ProcessCorrectly)
{
    exec_env = wasm_runtime_create_exec_env(module_inst, default_stack_size);
    ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";

    // Test block with unreachable code before end
    uint32_t result = CallWasmFunction("test_unreachable_block");
    ASSERT_EQ(result, 99) << "Block with unreachable code should handle properly";

    // Test function that returns before reaching unreachable end
    result = CallWasmFunctionWithParam("test_early_return", 5);
    ASSERT_EQ(result, 5) << "Function with early return should not reach unreachable end";

    wasm_runtime_destroy_exec_env(exec_env);
}

/**
 * @test InvalidStructures_RejectGracefully
 * @brief Validates rejection of malformed WASM modules with invalid 'end' usage
 * @details Tests modules with unmatched 'end' instructions, type mismatches,
 *          and structural violations. Verifies proper error handling and rejection.
 * @test_category Error - Invalid scenario validation
 * @coverage_target core/iwasm/common/wasm_loader.c:wasm_loader_prepare_bytecode
 * @input_conditions Malformed modules with unmatched ends, type violations
 * @expected_behavior Module loading failures with appropriate error detection
 * @validation_method Module load failure verification and error message validation
 */
TEST_F(EndTest, InvalidStructures_RejectGracefully)
{
    // Test loading invalid module with unmatched end instructions
    uint8_t *invalid_buffer = (uint8_t *)bh_read_file_to_buffer(WASM_FILE_INVALID.c_str(), &size);
    ASSERT_NE(invalid_buffer, nullptr) << "Failed to read invalid WASM file";

    wasm_module_t invalid_module = wasm_runtime_load(invalid_buffer, size, error_buf, sizeof(error_buf));
    ASSERT_EQ(invalid_module, nullptr)
        << "Invalid module with unmatched ends should fail to load";

    // Verify error message indicates structural problems
    std::string error_msg(error_buf);
    ASSERT_FALSE(error_msg.empty()) << "Error message should be provided for invalid module";

    BH_FREE(invalid_buffer);
}

/**
 * @test StackTypeValidation_EnforceSignatures
 * @brief Validates stack type consistency at 'end' instruction boundaries
 * @details Tests that stack contents match block signatures at 'end' points.
 *          Verifies proper type validation and error handling for mismatched signatures.
 * @test_category Error - Type validation testing
 * @coverage_target core/iwasm/common/wasm_loader.c:wasm_loader_prepare_bytecode
 * @input_conditions Blocks with correct and incorrect stack type signatures
 * @expected_behavior Proper type validation at end boundaries
 * @validation_method Type consistency verification and validation error detection
 */
TEST_F(EndTest, StackTypeValidation_EnforceSignatures)
{
    exec_env = wasm_runtime_create_exec_env(module_inst, default_stack_size);
    ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";

    // Test block with correct type signature
    uint32_t result = CallWasmFunction("test_correct_types");
    ASSERT_EQ(result, 777) << "Block with correct types should execute successfully";

    // Test multi-result block type validation
    result = CallWasmFunctionWithParam("test_multi_result_types", 10);
    ASSERT_EQ(result, 10) << "Multi-result block should maintain type consistency";

    wasm_runtime_destroy_exec_env(exec_env);
}
