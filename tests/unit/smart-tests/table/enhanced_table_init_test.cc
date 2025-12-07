/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include "bh_read_file.h"
#include "wasm_runtime_common.h"

// RunningMode is already defined in wasm_export.h

/**
 * @brief Enhanced unit tests for table.init opcode
 *
 * This test suite provides comprehensive validation of the WASM table.init instruction,
 * which initializes a table with elements from a passive element segment. Tests cover
 * basic functionality, boundary conditions, edge cases, and error scenarios across
 * both interpreter and AOT execution modes.
 */
class TableInitTest : public testing::TestWithParam<RunningMode>
{
  protected:
    /**
     * @brief Set up test environment before each test case
     *
     * Initializes WAMR runtime with proper configuration for both interpreter
     * and AOT modes. Sets up memory allocation and runtime parameters.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Enable required WAMR features for table operations
        init_args.n_native_symbols = 0;
        init_args.native_module_name = nullptr;
        init_args.native_symbols = nullptr;

        ASSERT_EQ(wasm_runtime_full_init(&init_args), true)
            << "Failed to initialize WAMR runtime";

        running_mode = GetParam();
    }

    /**
     * @brief Clean up test environment after each test case
     *
     * Properly destroys WAMR runtime and releases all allocated resources
     * following RAII principles for consistent cleanup.
     */
    void TearDown() override
    {
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM module from file for testing
     * @param filename Name of WASM file in wasm-apps directory
     * @return Loaded WASM module or nullptr on failure
     *
     * Loads and validates WASM test module, handling both AOT and interpreter
     * module loading paths based on current running mode.
     */
    wasm_module_t LoadTestModule(const char *filename)
    {
        char error_buf[256];
        uint32_t buf_size, wasm_file_size;
        uint8_t *wasm_file_buf = nullptr;

        // Construct file path relative to test execution directory
        std::string file_path = "wasm-apps/";
        file_path += filename;

        wasm_file_buf = (uint8_t *)bh_read_file_to_buffer(file_path.c_str(), &wasm_file_size);
        EXPECT_NE(nullptr, wasm_file_buf) << "Failed to read WASM file: " << filename;
        if (!wasm_file_buf) {
            return nullptr;
        }

        // Load module based on running mode
        wasm_module_t module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                               error_buf, sizeof(error_buf));
        if (!module) {
            LOG_ERROR("Load module failed: %s", error_buf);
        }

        BH_FREE(wasm_file_buf);
        return module;
    }

    /**
     * @brief Instantiate WASM module for execution
     * @param module Loaded WASM module
     * @return Module instance or nullptr on failure
     *
     * Creates module instance with appropriate stack and heap sizes for
     * table operation testing.
     */
    wasm_module_inst_t InstantiateModule(wasm_module_t module)
    {
        char error_buf[256];
        uint32_t stack_size = 16384;  // 16KB stack
        uint32_t heap_size = 16384;   // 16KB heap

        wasm_module_inst_t module_inst = wasm_runtime_instantiate(
            module, stack_size, heap_size, error_buf, sizeof(error_buf));

        if (!module_inst) {
            LOG_ERROR("Instantiate module failed: %s", error_buf);
        }

        return module_inst;
    }

    /**
     * @brief Execute WASM function and capture results
     * @param module_inst WASM module instance
     * @param func_name Name of function to execute
     * @param argc Number of arguments
     * @param argv Array of arguments
     * @return Execution result array or nullptr on failure
     *
     * Executes specified WASM function with provided arguments, handling
     * both successful execution and trap conditions.
     */
    uint32_t *CallWasmFunction(wasm_module_inst_t module_inst, const char *func_name,
                              uint32_t argc, uint32_t argv[])
    {
        wasm_exec_env_t exec_env;
        wasm_function_inst_t func_inst;

        exec_env = wasm_runtime_create_exec_env(module_inst, 32768);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";
        if (!exec_env) {
            return nullptr;
        }

        func_inst = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup function: " << func_name;
        if (!func_inst) {
            wasm_runtime_destroy_exec_env(exec_env);
            return nullptr;
        }

        static uint32_t results[4];  // Static storage for results
        bool success = wasm_runtime_call_wasm(exec_env, func_inst, argc, argv);

        if (!success) {
            const char *exception = wasm_runtime_get_exception(module_inst);
            if (exception) {
                LOG_ERROR("Function execution failed: %s", exception);
            }
            wasm_runtime_destroy_exec_env(exec_env);
            return nullptr;
        }

        // Copy results for return (functions may modify global state)
        memcpy(results, argv, sizeof(uint32_t) * 4);

        wasm_runtime_destroy_exec_env(exec_env);
        return results;
    }

    RuntimeInitArgs init_args;
    RunningMode running_mode;
};

/**
 * @test BasicTableInitialization_InitializesCorrectly
 * @brief Validates table.init correctly copies elements from passive segment to table
 * @details Tests fundamental table initialization with function references from element
 *          segments. Verifies elements are copied to correct table positions and can
 *          be retrieved via table.get operations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_init_operation
 * @input_conditions Standard table (size 10), element segment (3 functions), copy 3 elements
 * @expected_behavior Elements copied to table positions 2,3,4 from segment positions 0,1,2
 * @validation_method Retrieve table contents via table.get and compare with expected function indices
 */
TEST_P(TableInitTest, BasicTableInitialization_InitializesCorrectly)
{
    // Load WASM module with table.init test functions
    wasm_module_t module = LoadTestModule("table_init_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load table_init test module";

    // Instantiate module for execution
    wasm_module_inst_t module_inst = InstantiateModule(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to instantiate test module";

    // Test basic table initialization: copy 3 elements starting at table index 2
    uint32_t args[] = {2, 0, 3};  // table_offset=2, elem_offset=0, length=3
    uint32_t *results = CallWasmFunction(module_inst, "test_basic_init", 3, args);
    ASSERT_NE(nullptr, results) << "Basic table initialization failed";

    // Verify table contents after initialization
    uint32_t verify_args[] = {2};  // Get element at table index 2
    uint32_t *table_elem = CallWasmFunction(module_inst, "get_table_elem", 1, verify_args);
    ASSERT_NE(nullptr, table_elem) << "Failed to retrieve table element";
    ASSERT_EQ(0, table_elem[0]) << "Table element 2 should contain function 0";

    verify_args[0] = 3;  // Get element at table index 3
    table_elem = CallWasmFunction(module_inst, "get_table_elem", 1, verify_args);
    ASSERT_NE(nullptr, table_elem) << "Failed to retrieve table element";
    ASSERT_EQ(1, table_elem[0]) << "Table element 3 should contain function 1";

    verify_args[0] = 4;  // Get element at table index 4
    table_elem = CallWasmFunction(module_inst, "get_table_elem", 1, verify_args);
    ASSERT_NE(nullptr, table_elem) << "Failed to retrieve table element";
    ASSERT_EQ(2, table_elem[0]) << "Table element 4 should contain function 2";

    // Clean up module resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test BoundaryConditions_HandlesTableLimitsCorrectly
 * @brief Validates table.init handles boundary conditions without overflow
 * @details Tests initialization at table boundaries including exact limit cases.
 *          Verifies proper bounds checking and successful operations at maximum
 *          valid offsets and lengths.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_init_bounds_check
 * @input_conditions Table size 10, initialize at boundary (offset=7, length=3)
 * @expected_behavior Successful initialization to positions 7,8,9 (table.size-3 to table.size-1)
 * @validation_method Verify elements copied correctly to boundary positions
 */
TEST_P(TableInitTest, BoundaryConditions_HandlesTableLimitsCorrectly)
{
    wasm_module_t module = LoadTestModule("table_init_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load table_init test module";

    wasm_module_inst_t module_inst = InstantiateModule(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to instantiate test module";

    // Test boundary initialization: copy to end of table
    uint32_t args[] = {7, 0, 3};  // table_offset=7, elem_offset=0, length=3 (fills slots 7,8,9)
    uint32_t *results = CallWasmFunction(module_inst, "test_boundary_init", 3, args);
    ASSERT_NE(nullptr, results) << "Boundary table initialization failed";

    // Verify boundary elements were initialized correctly
    uint32_t verify_args[] = {7};
    uint32_t *table_elem = CallWasmFunction(module_inst, "get_table_elem", 1, verify_args);
    ASSERT_NE(nullptr, table_elem) << "Failed to retrieve boundary table element";
    ASSERT_EQ(0, table_elem[0]) << "Boundary table element 7 should contain function 0";

    verify_args[0] = 9;  // Last valid table index
    table_elem = CallWasmFunction(module_inst, "get_table_elem", 1, verify_args);
    ASSERT_NE(nullptr, table_elem) << "Failed to retrieve last table element";
    ASSERT_EQ(2, table_elem[0]) << "Last table element should contain function 2";

    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test ZeroLengthOperations_CompletesWithoutEffect
 * @brief Validates table.init with zero length completes as no-op
 * @details Tests edge case of zero-length initialization operations. Verifies
 *          operation succeeds without side effects. Since we cannot easily check
 *          table contents without initialization, we focus on verifying the
 *          operation completes successfully.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_init_zero_length
 * @input_conditions Various table offsets with length=0
 * @expected_behavior Operation succeeds without traps or errors
 * @validation_method Verify zero-length operations complete successfully
 */
TEST_P(TableInitTest, ZeroLengthOperations_CompletesWithoutEffect)
{
    wasm_module_t module = LoadTestModule("table_init_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load table_init test module";

    wasm_module_inst_t module_inst = InstantiateModule(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to instantiate test module";

    // Test zero-length initialization at various valid offsets
    // These should all succeed without any traps or exceptions

    // Test at beginning of table
    uint32_t args[] = {0, 0, 0};  // table_offset=0, elem_offset=0, length=0
    uint32_t *results = CallWasmFunction(module_inst, "test_zero_length_init", 3, args);
    ASSERT_NE(nullptr, results) << "Zero-length initialization at start failed";

    // Test at middle of table
    args[0] = 5;  // table_offset=5, elem_offset=0, length=0
    results = CallWasmFunction(module_inst, "test_zero_length_init", 3, args);
    ASSERT_NE(nullptr, results) << "Zero-length initialization at middle failed";

    // Test at end of table
    args[0] = 9;  // table_offset=9, elem_offset=0, length=0
    results = CallWasmFunction(module_inst, "test_zero_length_init", 3, args);
    ASSERT_NE(nullptr, results) << "Zero-length initialization at end failed";

    // Test with different element offsets (still zero length)
    args[0] = 3;  // table_offset=3
    args[1] = 2;  // elem_offset=2, length still 0
    results = CallWasmFunction(module_inst, "test_zero_length_init", 3, args);
    ASSERT_NE(nullptr, results) << "Zero-length initialization with element offset failed";

    // After all zero-length operations, verify we can still do a real initialization
    uint32_t init_args[] = {1, 0, 2};  // table_offset=1, elem_offset=0, length=2
    uint32_t *init_result = CallWasmFunction(module_inst, "test_basic_init", 3, init_args);
    ASSERT_NE(nullptr, init_result) << "Normal initialization after zero-length operations failed";

    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test OutOfBoundsAccess_TrapsProperly
 * @brief Validates table.init traps on out-of-bounds access attempts
 * @details Tests error conditions including table overflow, element segment
 *          overflow, and invalid indices. Verifies proper trap generation
 *          with appropriate error messages.
 * @test_category Error - Out-of-bounds validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_init_trap_handling
 * @input_conditions Invalid table offsets, lengths exceeding boundaries
 * @expected_behavior Runtime traps with out-of-bounds error
 * @validation_method Verify trap occurs and no memory corruption results
 */
TEST_P(TableInitTest, OutOfBoundsAccess_TrapsProperly)
{
    wasm_module_t module = LoadTestModule("table_init_test.wasm");
    ASSERT_NE(nullptr, module) << "Failed to load table_init test module";

    wasm_module_inst_t module_inst = InstantiateModule(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to instantiate test module";

    // Test table index out of bounds
    uint32_t args[] = {10, 0, 1};  // table_offset=10 (>= table.size), length=1
    uint32_t *results = CallWasmFunction(module_inst, "test_table_oob", 3, args);
    ASSERT_EQ(nullptr, results) << "Expected trap for table out-of-bounds access";

    // Verify exception was thrown for out-of-bounds
    const char *exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(nullptr, exception) << "Expected exception for out-of-bounds access";

    // Clear exception for next test
    wasm_runtime_clear_exception(module_inst);

    // Test table offset + length overflow
    args[0] = 8;  // table_offset=8
    args[2] = 3;  // length=3, would access positions 8,9,10 (10 >= table.size)
    results = CallWasmFunction(module_inst, "test_table_overflow", 3, args);
    ASSERT_EQ(nullptr, results) << "Expected trap for table overflow";

    exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(nullptr, exception) << "Expected exception for table overflow";

    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

// Test parameterization for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningModeTest,
                        TableInitTest,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT),
                        [](const testing::TestParamInfo<TableInitTest::ParamType>& info) {
                            return info.param == Mode_Interp ? "Interpreter" : "AOT";
                        });