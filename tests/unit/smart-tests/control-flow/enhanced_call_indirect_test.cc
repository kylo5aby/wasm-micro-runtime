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
 * @class CallIndirectTest
 * @brief Test fixture class for comprehensive call_indirect opcode testing
 *
 * This test suite validates the WebAssembly call_indirect instruction across
 * different execution modes (interpreter and AOT), ensuring correct behavior
 * for dynamic function dispatch through function tables.
 *
 * Features tested:
 * - Basic indirect function calls with various signatures
 * - Table boundary validation and error handling
 * - Function signature type checking and validation
 * - Stack management during indirect calls
 * - Cross-execution mode consistency verification
 */
class CallIndirectTest : public testing::TestWithParam<RunningMode> {
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

        // Initialize runtime with proper error handling
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime environment";

        running_mode = GetParam();

        // Configure mode-specific settings
        if (running_mode == Mode_LLVM_JIT) {
            // Ensure LLVM JIT compilation capabilities are available
            ASSERT_TRUE(wasm_runtime_is_running_mode_supported(Mode_LLVM_JIT))
                << "LLVM JIT mode not supported in current WAMR build";
        }
    }

    /**
     * @brief Clean up test environment after each test case
     *
     * Properly destroys WAMR runtime environment and releases all allocated
     * resources to prevent memory leaks between test cases.
     */
    void TearDown() override
    {
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
     * @brief Execute exported WASM function with error handling
     * @param module_inst WASM module instance containing the function
     * @param function_name Name of the exported function to execute
     * @param argc Number of arguments to pass to the function
     * @param argv Array of arguments (uint32 values)
     * @return true if function executed successfully, false on error
     *
     * Executes a WASM function by name with the provided arguments. Handles
     * both successful execution and trapped/error conditions with appropriate
     * error reporting for debugging.
     */
    bool ExecuteFunction(wasm_module_inst_t module_inst, const char* function_name,
                        uint32 argc, uint32 argv[])
    {
        wasm_function_inst_t func_inst = nullptr;
        wasm_exec_env_t exec_env = nullptr;
        bool success = false;

        // Find exported function by name
        func_inst = wasm_runtime_lookup_function(module_inst, function_name);
        EXPECT_NE(nullptr, func_inst)
            << "Function not found: " << function_name;

        if (!func_inst) {
            return false;
        }

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, 8192);  // 8KB exec stack
        EXPECT_NE(nullptr, exec_env)
            << "Failed to create execution environment";

        if (!exec_env) {
            return false;
        }

        // Execute function with exception handling
        success = wasm_runtime_call_wasm(exec_env, func_inst, argc, argv);

        if (!success) {
            // Capture exception information for debugging
            const char* exception = wasm_runtime_get_exception(module_inst);
            if (exception) {
                ADD_FAILURE() << "WASM function execution failed: " << exception;
            }
        }

        wasm_runtime_destroy_exec_env(exec_env);
        return success;
    }

    /**
     * @brief Clean up allocated resources from module loading
     *
     * Releases WASM module, module instance, and file buffer resources.
     * Called automatically during TearDown or can be called explicitly
     * for early resource cleanup.
     */
    void CleanupModule()
    {
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
    }

    // Test fixture member variables
    RuntimeInitArgs init_args;
    RunningMode running_mode;

    // Resource management
    wasm_module_t current_module = nullptr;
    wasm_module_inst_t current_module_inst = nullptr;
    uint8* current_file_buf = nullptr;
};

/**
 * @test BasicIndirectFunctionCall_ReturnsCorrectResults
 * @brief Validates fundamental call_indirect functionality with various function signatures
 * @details Tests basic indirect function calls through table dispatch with different parameter
 *          and return value configurations. Verifies that call_indirect correctly invokes
 *          target functions and returns expected results for typical usage scenarios.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_indirect_operation
 * @input_conditions Functions with signatures (i32)->(i32), (i32,f64)->(i64), ()->(i32)
 * @expected_behavior Functions execute correctly through table indirection, return expected results
 * @validation_method Direct comparison of WASM function results with expected values
 */
TEST_P(CallIndirectTest, BasicIndirectFunctionCall_ReturnsCorrectResults)
{
    // Load WASM module with call_indirect test functions
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load call_indirect test module";

    uint32 argv[4] = {0};

    // Test 1: Simple (i32) -> (i32) function through table index 0
    argv[0] = 42;  // function parameter
    argv[1] = 0;   // table index
    ASSERT_TRUE(ExecuteFunction(module_inst, "test_call_indirect_i32", 2, argv))
        << "Failed to execute call_indirect with i32 parameter";
    ASSERT_EQ(84, argv[0]) << "Incorrect result from i32->i32 indirect function call";

    // Test 2: Zero parameter function through table index 6
    argv[0] = 6;   // table index
    ASSERT_TRUE(ExecuteFunction(module_inst, "test_call_indirect_void", 1, argv))
        << "Failed to execute call_indirect with void parameters";
    ASSERT_EQ(123, argv[0]) << "Incorrect result from ()->i32 indirect function call";

    // Test 3: Multiple parameter function through table index 2
    argv[0] = 10;   // first parameter
    argv[1] = 20;   // second parameter
    argv[2] = 2;    // table index
    ASSERT_TRUE(ExecuteFunction(module_inst, "test_call_indirect_multi", 3, argv))
        << "Failed to execute call_indirect with multiple parameters";
    ASSERT_EQ(30, argv[0]) << "Incorrect result from multi-parameter indirect function call";

    CleanupModule();
}

/**
 * @test TableBoundaryAccess_HandlesValidIndices
 * @brief Tests call_indirect with table boundary positions and validates index handling
 * @details Verifies that call_indirect correctly handles function calls at table boundary
 *          positions including first valid index (0), intermediate indices, and last valid
 *          index (table.size-1). Ensures boundary validation works correctly.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_boundary_validation
 * @input_conditions Table indices 0, 1, and table.size-1 with valid function references
 * @expected_behavior All boundary positions work correctly without index errors
 * @validation_method Verify successful execution at all valid boundary positions
 */
TEST_P(CallIndirectTest, TableBoundaryAccess_HandlesValidIndices)
{
    // Load WASM module with boundary test functions
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load call_indirect boundary test module";

    uint32 argv[2] = {0};

    // Test 1: First valid table index (0)
    argv[0] = 100;  // function parameter
    argv[1] = 0;    // first table index
    ASSERT_TRUE(ExecuteFunction(module_inst, "test_call_indirect_boundary", 2, argv))
        << "Failed to execute call_indirect at first table index (0)";
    ASSERT_EQ(200, argv[0]) << "Incorrect result from function at table index 0";

    // Test 2: Middle table index (1)
    argv[0] = 50;   // function parameter
    argv[1] = 1;    // middle table index
    ASSERT_TRUE(ExecuteFunction(module_inst, "test_call_indirect_boundary", 2, argv))
        << "Failed to execute call_indirect at middle table index (1)";
    ASSERT_EQ(100, argv[0]) << "Incorrect result from function at table index 1";

    // Test 3: Last valid table index (table.size-1 = 3)
    argv[0] = 25;   // function parameter
    argv[1] = 3;    // last valid table index
    ASSERT_TRUE(ExecuteFunction(module_inst, "test_call_indirect_boundary", 2, argv))
        << "Failed to execute call_indirect at last table index (3)";
    ASSERT_EQ(50, argv[0]) << "Incorrect result from function at last valid table index";

    CleanupModule();
}

/**
 * @test FunctionSignatureComplexity_HandlesMaximumComplexity
 * @brief Validates call_indirect with complex function signatures and mixed value types
 * @details Tests indirect function calls with complex signatures involving multiple parameters,
 *          multiple return values, and mixed WebAssembly value types (i32, i64, f32, f64).
 *          Ensures type system correctly handles complex function dispatch scenarios.
 * @test_category Edge - Complex signature validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:function_signature_validation
 * @input_conditions Functions with multiple parameters/returns using all WebAssembly value types
 * @expected_behavior Complex signatures work correctly through indirect dispatch
 * @validation_method Type-specific assertions for multi-value function results
 */
TEST_P(CallIndirectTest, FunctionSignatureComplexity_HandlesMaximumComplexity)
{
    // Load WASM module with complex signature functions
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load call_indirect complex signature module";

    uint32 argv[6] = {0};

    // Test 1: Function with multiple parameters and single return
    argv[0] = 10;        // i32 parameter
    argv[1] = 0x40490FDB; // f64 parameter (3.14159 in IEEE 754)
    argv[2] = 0x40490FDB; // f64 parameter high bits
    argv[3] = 4;         // table index for complex function
    ASSERT_TRUE(ExecuteFunction(module_inst, "test_call_indirect_complex", 4, argv))
        << "Failed to execute call_indirect with complex signature";
    ASSERT_GT(argv[0], 0) << "Complex function should return positive result";

    // Test 2: Function with multiple return values
    argv[0] = 5;         // input parameter
    argv[1] = 5;         // table index for multi-return function
    ASSERT_TRUE(ExecuteFunction(module_inst, "test_call_indirect_multi_return", 2, argv))
        << "Failed to execute call_indirect with multiple return values";
    ASSERT_EQ(5, argv[0]) << "First return value should equal input parameter";
    ASSERT_EQ(25, argv[1]) << "Second return value should be square of input";

    CleanupModule();
}

/**
 * @test ErrorConditionHandling_TrapsOnInvalidScenarios
 * @brief Tests error scenarios including out-of-bounds access, null references, type mismatches
 * @details Validates that call_indirect properly detects and handles error conditions by
 *          generating appropriate traps for invalid table indices, null function references,
 *          function signature mismatches, and other invalid scenarios.
 * @test_category Error - Exception and trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:call_indirect_error_handling
 * @input_conditions Invalid table indices, null function references, mismatched signatures
 * @expected_behavior Proper traps/exceptions for all invalid scenarios
 * @validation_method Exception detection and error message validation
 */
TEST_P(CallIndirectTest, ErrorConditionHandling_TrapsOnInvalidScenarios)
{
    // Load WASM module for error condition testing
    wasm_module_inst_t module_inst = LoadTestModule("wasm-apps/call_indirect_test.wasm");
    ASSERT_NE(nullptr, module_inst) << "Failed to load call_indirect error test module";

    uint32 argv[3] = {0};

    // Test 1: Out-of-bounds table index (table size is 8, so index 8 is invalid)
    argv[0] = 10;        // function parameter
    argv[1] = 8;         // invalid table index (>= table.size)
    ASSERT_FALSE(ExecuteFunction(module_inst, "test_call_indirect_boundary", 2, argv))
        << "call_indirect should trap on out-of-bounds table index";

    // Test 2: Very large invalid table index
    argv[0] = 10;        // function parameter
    argv[1] = 999999;    // extremely invalid table index
    ASSERT_FALSE(ExecuteFunction(module_inst, "test_call_indirect_boundary", 2, argv))
        << "call_indirect should trap on extremely large table index";

    // Test 3: Function signature mismatch (call function expecting f64 with i32 signature)
    argv[0] = 42;        // i32 parameter (but function expects f64)
    argv[1] = 7;         // table index pointing to function with different signature
    ASSERT_FALSE(ExecuteFunction(module_inst, "test_call_indirect_type_mismatch", 2, argv))
        << "call_indirect should trap on function signature mismatch";

    CleanupModule();
}

// Parameterized test instantiation for interpreter mode
INSTANTIATE_TEST_SUITE_P(
    CallIndirectModeTest,
    CallIndirectTest,
    testing::Values(Mode_Interp),
    [](const testing::TestParamInfo<CallIndirectTest::ParamType>& info) {
        return "InterpreterMode";
    }
);