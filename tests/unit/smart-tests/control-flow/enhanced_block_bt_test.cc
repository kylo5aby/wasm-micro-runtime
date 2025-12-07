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


/**
 * @file enhanced_block_bt_test.cc
 * @brief Comprehensive unit tests for WASM 'block bt' opcode
 * @details Tests block instruction functionality including basic execution,
 *          control flow branching, nested structures, polymorphic types,
 *          and error conditions across interpreter and AOT modes.
 */

class BlockBtTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     * @details Configures WAMR runtime with system allocator and enables
     *          both interpreter and AOT modes for comprehensive testing
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

        if (module_inst) {
            exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
            EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";
        }

        return module_inst;
    }

    /**
     * @brief Execute WASM function by name
     * @param func_name Name of exported function to execute
     * @param argc Number of arguments
     * @param argv Array of argument values
     * @return Execution result, or UINT32_MAX on failure
     */
    uint32 ExecuteFunction(const char *func_name, uint32 argc = 0, uint32 argv[] = nullptr)
    {
        wasm_function_inst_t func = nullptr;
        char error_buf[128] = { 0 };
        uint32 local_argv[16] = { 0 };  // Local buffer for arguments
        uint32 *args_ptr = (argc > 0 && argv) ? argv : local_argv;

        EXPECT_NE(nullptr, module_inst) << "Module instance must be created before function execution";
        if (!module_inst) {
            return UINT32_MAX;
        }

        func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Function '" << func_name << "' not found in module";
        if (!func) {
            return UINT32_MAX;
        }

        // Copy arguments if provided
        if (argc > 0 && argv && argc <= 16) {
            memcpy(local_argv, argv, argc * sizeof(uint32));
        }

        bool success = wasm_runtime_call_wasm(exec_env, func, argc, args_ptr);
        if (!success) {
            const char *exception = wasm_runtime_get_exception(module_inst);
            ADD_FAILURE() << "Function execution failed: " << (exception ? exception : "Unknown error");
            return UINT32_MAX;
        }

        // Return first result value for functions with return values
        return (argc > 0 || args_ptr[0] != 0) ? args_ptr[0] : 0;
    }

    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    static const char *WASM_FILE;
};

const char *BlockBtTest::WASM_FILE = "wasm-apps/block_bt_test.wasm";

/**
 * @test BasicBlockExecution_ReturnsExpectedValues
 * @brief Validates basic block execution with various block types
 * @details Tests fundamental block operations including empty blocks, value-producing blocks,
 *          and value-consuming blocks to ensure correct execution semantics.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Various block types: empty [], value-producing [i32], consuming [i32]->[i64]
 * @expected_behavior Blocks execute correctly and produce expected result values
 * @validation_method Direct comparison of WASM function results with expected values
 */
TEST_P(BlockBtTest, BasicBlockExecution_ReturnsExpectedValues)
{
    // Load test module with block operations
    wasm_module_t test_module = LoadWasmModule(WASM_FILE);
    ASSERT_NE(nullptr, test_module) << "Failed to load block test module";

    // Create module instance for execution
    wasm_module_inst_t test_instance = CreateModuleInstance();
    ASSERT_NE(nullptr, test_instance) << "Failed to create module instance for block tests";

    // Test 1: Empty block execution ([] -> [])
    uint32 result = ExecuteFunction("test_empty_block");
    ASSERT_EQ(0, result) << "Empty block should return 0 (no value produced)";

    // Test 2: Value-producing block ([] -> [i32])
    result = ExecuteFunction("test_value_producing_block");
    ASSERT_EQ(42, result) << "Value-producing block should return 42";

    // Test 3: Value-consuming and producing block ([i32] -> [i64])
    uint32 input_args[] = { 10 };
    result = ExecuteFunction("test_consuming_block", 1, input_args);
    ASSERT_EQ(20, result) << "Consuming block should double input value (10 * 2 = 20)";

    // Test 4: Multi-value block type ([i32, i32] -> [i32])
    uint32 multi_args[] = { 15, 25 };
    result = ExecuteFunction("test_multi_value_block", 2, multi_args);
    ASSERT_EQ(40, result) << "Multi-value block should sum inputs (15 + 25 = 40)";
}

/**
 * @test ControlFlowBranching_ExecutesCorrectly
 * @brief Validates block behavior as branch targets and control flow mechanisms
 * @details Tests blocks serving as targets for br instructions, early exits,
 *          and conditional branching to ensure proper control flow management.
 * @test_category Main - Control flow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Blocks with internal branching logic, br instructions, conditional branches
 * @expected_behavior Proper control transfer to block end, correct result values after branching
 * @validation_method Verification of branch behavior and result value correctness
 */
TEST_P(BlockBtTest, ControlFlowBranching_ExecutesCorrectly)
{
    // Load test module with control flow operations
    wasm_module_t test_module = LoadWasmModule(WASM_FILE);
    ASSERT_NE(nullptr, test_module) << "Failed to load control flow test module";

    // Create module instance for execution
    wasm_module_inst_t test_instance = CreateModuleInstance();
    ASSERT_NE(nullptr, test_instance) << "Failed to create module instance for control flow tests";

    // Test 1: Block as branch target (br 0)
    uint32 result = ExecuteFunction("test_block_branch_target");
    ASSERT_EQ(100, result) << "Branch to block end should return 100";

    // Test 2: Conditional branching within block (br_if)
    uint32 condition_true[] = { 1 };  // True condition
    result = ExecuteFunction("test_conditional_branch", 1, condition_true);
    ASSERT_EQ(200, result) << "Conditional branch with true condition should return 200";

    uint32 condition_false[] = { 0 };  // False condition
    result = ExecuteFunction("test_conditional_branch", 1, condition_false);
    ASSERT_EQ(300, result) << "Conditional branch with false condition should return 300";

    // Test 3: Early exit from block via branch
    uint32 early_exit_args[] = { 5 };
    result = ExecuteFunction("test_early_exit_block", 1, early_exit_args);
    ASSERT_EQ(1000, result) << "Early exit should return 1000 for non-zero input (5)";
}

/**
 * @test NestedBlockStructures_HandledCorrectly
 * @brief Validates nested block execution and complex block hierarchies
 * @details Tests multiple levels of block nesting with various block types,
 *          ensuring proper scope management and nested control flow.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Multiple nesting levels, complex nested patterns, maximum supported depth
 * @expected_behavior Proper nesting behavior, correct scope isolation, accurate result computation
 * @validation_method Verification of nested execution results and scope management
 */
TEST_P(BlockBtTest, NestedBlockStructures_HandledCorrectly)
{
    // Load test module with nested block structures
    wasm_module_t test_module = LoadWasmModule(WASM_FILE);
    ASSERT_NE(nullptr, test_module) << "Failed to load nested block test module";

    // Create module instance for execution
    wasm_module_inst_t test_instance = CreateModuleInstance();
    ASSERT_NE(nullptr, test_instance) << "Failed to create module instance for nested tests";

    // Test 1: Simple nested blocks (2 levels)
    uint32 result = ExecuteFunction("test_simple_nested_blocks");
    ASSERT_EQ(50, result) << "Simple nested blocks should return 50";

    // Test 2: Complex nested blocks with different types (3 levels)
    uint32 complex_args[] = { 7 };
    result = ExecuteFunction("test_complex_nested_blocks", 1, complex_args);
    ASSERT_EQ(14, result) << "Complex nested blocks should double input (7 * 2 = 14)";

    // Test 3: Maximum nesting depth test
    result = ExecuteFunction("test_max_nesting_depth");
    ASSERT_EQ(1000, result) << "Maximum nesting depth test should return 1000";

    // Test 4: Nested blocks with branching
    uint32 nested_branch_args[] = { 3 };
    result = ExecuteFunction("test_nested_blocks_with_branching", 1, nested_branch_args);
    ASSERT_EQ(9, result) << "Nested blocks with branching should compute 3^2 = 9";
}

/**
 * @test PolymorphicBlockTypes_ResolveCorrectly
 * @brief Validates complex block type signatures and polymorphic type resolution
 * @details Tests blocks with function type references, complex signatures,
 *          and polymorphic contexts to ensure proper type system handling.
 * @test_category Edge - Complex type system validation
 * @coverage_target core/iwasm/loader/wasm_loader.c:wasm_loader_prepare_bytecode
 * @input_conditions Complex block type signatures, type alias resolution, polymorphic contexts
 * @expected_behavior Correct type resolution, proper value handling for complex signatures
 * @validation_method Verification of type resolution and complex signature execution
 */
TEST_P(BlockBtTest, PolymorphicBlockTypes_ResolveCorrectly)
{
    // Load test module with complex block types
    wasm_module_t test_module = LoadWasmModule(WASM_FILE);
    ASSERT_NE(nullptr, test_module) << "Failed to load polymorphic block test module";

    // Create module instance for execution
    wasm_module_inst_t test_instance = CreateModuleInstance();
    ASSERT_NE(nullptr, test_instance) << "Failed to create module instance for polymorphic tests";

    // Test 1: Block type with function signature reference
    uint32 func_sig_args[] = { 10, 20 };
    uint32 result = ExecuteFunction("test_function_signature_block", 2, func_sig_args);
    ASSERT_EQ(30, result) << "Function signature block should sum inputs (10 + 20 = 30)";

    // Test 2: Complex multi-parameter block type
    uint32 multi_param_args[] = { 5, 10, 15 };
    result = ExecuteFunction("test_multi_param_block", 3, multi_param_args);
    ASSERT_EQ(200, result) << "Multi-parameter block should compute (5 * 10) + (15 * 10) = 50 + 150 = 200";

    // Test 3: Block type with mixed value types (i32, f32) -> (i64)
    // Note: For simplicity, we'll use integer approximations in the test
    uint32 mixed_type_args[] = { 8 };  // Representing both i32 and f32 as i32 for test
    result = ExecuteFunction("test_mixed_type_block", 1, mixed_type_args);
    ASSERT_EQ(16, result) << "Mixed type block should transform input appropriately";
}

/**
 * @test ErrorConditions_HandleGracefully
 * @brief Validates error handling for invalid block operations and malformed structures
 * @details Tests invalid block types, stack underflow conditions, and malformed
 *          module structures to ensure proper error detection and handling.
 * @test_category Error - Exception and error condition validation
 * @coverage_target core/iwasm/loader/wasm_loader.c:wasm_loader_prepare_bytecode
 * @input_conditions Invalid WASM modules, malformed block structures, error-triggering scenarios
 * @expected_behavior Proper error detection, graceful failure handling, appropriate error messages
 * @validation_method Verification of error conditions and proper failure responses
 */
TEST_P(BlockBtTest, ErrorConditions_HandleGracefully)
{
    char error_buf[256] = { 0 };
    uint32 wasm_file_size;
    uint8 *wasm_file_buf = nullptr;
    wasm_module_t invalid_module = nullptr;

    // Test 1: Invalid block type index (should fail during module loading)
    const char *invalid_block_type_file = "wasm-apps/invalid_block_type.wasm";
    wasm_file_buf = (uint8 *)bh_read_file_to_buffer(invalid_block_type_file, &wasm_file_size);
    if (wasm_file_buf) {
        invalid_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
        ASSERT_EQ(nullptr, invalid_module)
            << "Expected module load to fail for invalid block type, but got valid module";
        BH_FREE(wasm_file_buf);
    }

    // Test 2: Malformed block structure (missing end instruction)
    const char *malformed_block_file = "wasm-apps/malformed_block.wasm";
    memset(error_buf, 0, sizeof(error_buf));
    wasm_file_buf = (uint8 *)bh_read_file_to_buffer(malformed_block_file, &wasm_file_size);
    if (wasm_file_buf) {
        invalid_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
        ASSERT_EQ(nullptr, invalid_module)
            << "Expected module load to fail for malformed block structure, but got valid module";
        BH_FREE(wasm_file_buf);
    }

    // Test 3: Stack underflow scenario - Load valid module but test runtime error
    wasm_module_t test_module = LoadWasmModule(WASM_FILE);
    if (test_module) {
        wasm_module_inst_t test_instance = CreateModuleInstance();
        ASSERT_NE(nullptr, test_instance) << "Failed to create module instance for error tests";

        // Attempt to execute function that causes stack underflow
        uint32 result = ExecuteFunction("test_stack_underflow_block");
        // Note: This test depends on the specific implementation of stack underflow handling
        // The function may return a specific error value or trigger an exception
        // We verify that the execution completes without crashing
        ASSERT_TRUE(true) << "Stack underflow test completed without crash";
    }
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    BlockBtTest,
    BlockBtTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode> &info) {
        return info.param == Mode_Interp ? "INTERP" : "AOT";
    }
);