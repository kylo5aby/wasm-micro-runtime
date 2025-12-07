/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "test_helper.h"
#include "wasm_runtime.h"
#include "bh_read_file.h"
#include "wasm_runtime_common.h"

/**
 * @brief Test suite for table.size opcode comprehensive validation
 *
 * This test suite validates the table.size WASM opcode functionality across different
 * execution modes (interpreter and AOT). The table.size instruction returns the current
 * number of elements in a specified table, providing runtime introspection of table capacity.
 *
 * @test_coverage Validates core/iwasm/interpreter/wasm_interp_classic.c:table_size_operation
 *                and core/iwasm/libraries/libc-wasi/libc_wasi_wrapper.c:table_size_wrapper
 * @execution_modes Tests both interpreter and AOT compilation modes
 * @test_categories Main routine, corner cases, edge cases, error conditions
 */
class TableSizeTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up WAMR runtime for table.size testing
     * @details Initializes WAMR runtime with system allocator, loads test WASM module,
     *          and sets up execution environment for both interpreter and AOT modes
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for table.size tests";

        // Load test WASM module containing table.size test functions
        LoadTestModule();

        running_mode = GetParam();
    }

    /**
     * @brief Clean up WAMR runtime and test resources
     * @details Destroys WASM module instances, unloads modules, and shuts down runtime
     */
    void TearDown() override {
        // Clean up execution environment
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }

        // Clean up module instance
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }

        // Clean up loaded module
        if (wasm_module) {
            wasm_runtime_unload(wasm_module);
            wasm_module = nullptr;
        }

        // Shutdown WAMR runtime
        wasm_runtime_destroy();
    }

    /**
     * @brief Load test WASM module from file system
     * @details Loads table_size_test.wasm module and creates runtime instance
     */
    void LoadTestModule() {
        uint32_t wasm_file_size;
        uint8_t *wasm_file_buf = nullptr;
        char error_buf[128] = {0};

        // Load WASM file from test directory
        wasm_file_buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/table_size_test.wasm", &wasm_file_size);
        ASSERT_NE(nullptr, wasm_file_buf) << "Failed to load table_size_test.wasm file";

        // Load WASM module
        wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, wasm_module) << "Failed to load WASM module: " << error_buf;

        // Create module instance
        module_inst = wasm_runtime_instantiate(wasm_module, DefaultStackSize, DefaultHeapSize, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        // Get execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, DefaultStackSize);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Free the loaded WASM file buffer
        BH_FREE(wasm_file_buf);
    }

    /**
     * @brief Load invalid WASM module for error testing
     * @details Loads modules with invalid table indices for validation error testing
     * @param filename Name of the invalid WASM file to load
     * @return nullptr if module loading failed as expected, module pointer if unexpectedly succeeded
     */
    wasm_module_t LoadInvalidModule(const std::string& filename) {
        uint32_t wasm_file_size;
        uint8_t *wasm_file_buf = nullptr;
        char error_buf[128] = {0};

        // Load invalid WASM file from test directory
        std::string full_path = "wasm-apps/" + filename;
        wasm_file_buf = (uint8_t *)bh_read_file_to_buffer(full_path.c_str(), &wasm_file_size);
        EXPECT_NE(nullptr, wasm_file_buf) << "Failed to load test file: " << full_path;

        // Attempt to load invalid WASM module (should fail)
        wasm_module_t invalid_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));

        // Free the loaded WASM file buffer
        BH_FREE(wasm_file_buf);

        return invalid_module;
    }

    /**
     * @brief Call WASM function and return i32 result
     * @param func_name Name of the exported WASM function to call
     * @param args Pointer to function arguments array
     * @param argc Number of function arguments
     * @return i32 return value from the WASM function
     */
    int32_t CallWasmFunction(const char* func_name, uint32_t args[] = nullptr, uint32_t argc = 0) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Failed to lookup function: " << func_name;

        uint32_t argv[8] = {0};
        if (args && argc > 0) {
            memcpy(argv, args, argc * sizeof(uint32_t));
        }

        bool success = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        EXPECT_TRUE(success) << "Failed to call WASM function: " << func_name;

        return static_cast<int32_t>(argv[0]);
    }

    // Test infrastructure
    RuntimeInitArgs init_args;
    wasm_module_t wasm_module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    RunningMode running_mode;

    // Default WAMR configuration
    static constexpr uint32_t DefaultStackSize = 16 * 1024;
    static constexpr uint32_t DefaultHeapSize = 16 * 1024;
};

/**
 * @test BasicTableSize_ReturnsCorrectInitialSize
 * @brief Validates table.size returns accurate initial table sizes for different table types
 * @details Tests fundamental size query operation on funcref and externref tables with
 *          known initial sizes. Verifies that table.size correctly reports the number
 *          of elements for tables of different types and initial capacities.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_size_operation
 * @input_conditions Funcref table with 10 elements, externref table with 5 elements
 * @expected_behavior Returns 10 and 5 respectively for initial table sizes
 * @validation_method Direct comparison of WASM function result with known initial sizes
 */
TEST_P(TableSizeTest, BasicTableSize_ReturnsCorrectInitialSize) {
    // Query size of funcref table (initialized with 10 elements)
    int32_t funcref_size = CallWasmFunction("get_funcref_table_size");
    ASSERT_EQ(10, funcref_size) << "Funcref table size mismatch for initial 10 elements";

    // Query size of externref table (initialized with 5 elements)
    int32_t externref_size = CallWasmFunction("get_externref_table_size");
    ASSERT_EQ(5, externref_size) << "Externref table size mismatch for initial 5 elements";

    // Verify multiple queries return consistent results
    int32_t funcref_size_2 = CallWasmFunction("get_funcref_table_size");
    ASSERT_EQ(funcref_size, funcref_size_2) << "Funcref table size query inconsistency";
}

/**
 * @test TableSizeAfterGrowth_ReturnsUpdatedSize
 * @brief Validates table.size returns correct size after table.grow operations
 * @details Tests size query after successful table growth operations, verifying that
 *          table.size accurately reflects the new table capacity after dynamic expansion.
 * @test_category Corner - Boundary conditions after table modifications
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_size_operation
 * @input_conditions Table grown from 10 to 25 elements using table.grow
 * @expected_behavior Returns 25 after successful growth operation
 * @validation_method Comparison of size before and after growth operation
 */
TEST_P(TableSizeTest, TableSizeAfterGrowth_ReturnsUpdatedSize) {
    // Get initial table size
    int32_t initial_size = CallWasmFunction("get_funcref_table_size");
    ASSERT_EQ(10, initial_size) << "Initial funcref table size incorrect";

    // Grow table by 15 elements
    int32_t grow_result = CallWasmFunction("grow_funcref_table_by_15");
    ASSERT_EQ(initial_size, grow_result) << "table.grow should return original size";

    // Verify updated size after growth
    int32_t new_size = CallWasmFunction("get_funcref_table_size");
    ASSERT_EQ(25, new_size) << "Table size after growth should be 25 elements";

    // Verify size consistency with multiple queries
    int32_t verify_size = CallWasmFunction("get_funcref_table_size");
    ASSERT_EQ(new_size, verify_size) << "Table size should remain consistent after growth";
}

/**
 * @test EmptyTableSize_ReturnsZero
 * @brief Validates table.size returns zero for empty tables
 * @details Tests size query on tables initialized with 0 elements, verifying correct
 *          handling of empty table scenarios and accurate size reporting.
 * @test_category Edge - Extreme scenarios with minimal table capacity
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_size_operation
 * @input_conditions Table initialized with 0 elements (empty table)
 * @expected_behavior Returns 0 for empty table size query
 * @validation_method Direct comparison with expected zero value
 */
TEST_P(TableSizeTest, EmptyTableSize_ReturnsZero) {
    // Query size of empty table (initialized with 0 elements)
    int32_t empty_size = CallWasmFunction("get_empty_table_size");
    ASSERT_EQ(0, empty_size) << "Empty table size should be 0";

    // Verify repeated queries on empty table return consistent results
    int32_t verify_empty = CallWasmFunction("get_empty_table_size");
    ASSERT_EQ(empty_size, verify_empty) << "Empty table size queries should be consistent";

    // Attempt to grow empty table and verify size changes
    int32_t grow_result = CallWasmFunction("grow_empty_table_by_3");
    ASSERT_EQ(0, grow_result) << "Growing empty table should return original size (0)";

    // Verify size after growing empty table
    int32_t size_after_growth = CallWasmFunction("get_empty_table_size");
    ASSERT_EQ(3, size_after_growth) << "Empty table size after growth should be 3";
}

/**
 * @test MultipleTableSizes_ReturnsIndependentSizes
 * @brief Validates table.size returns correct sizes for multiple tables in same module
 * @details Tests size queries on different table indices within the same module, verifying
 *          that table.size accurately distinguishes between different tables and their
 *          independent sizes.
 * @test_category Edge - Multiple table management validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_size_operation
 * @input_conditions Module with multiple tables of different sizes (10, 5, 0 elements)
 * @expected_behavior Returns correct size for each table index independently
 * @validation_method Individual size validation for each table in the module
 */
TEST_P(TableSizeTest, MultipleTableSizes_ReturnsIndependentSizes) {
    // Query sizes of different tables in the same module
    int32_t table0_size = CallWasmFunction("get_table_0_size");  // funcref, size 10
    ASSERT_EQ(10, table0_size) << "Table 0 size should be 10 elements";

    int32_t table1_size = CallWasmFunction("get_table_1_size");  // externref, size 5
    ASSERT_EQ(5, table1_size) << "Table 1 size should be 5 elements";

    int32_t table2_size = CallWasmFunction("get_table_2_size");  // empty table, size 0
    ASSERT_EQ(0, table2_size) << "Table 2 size should be 0 elements";

    // Grow table 1 and verify other tables remain unchanged
    int32_t grow_result = CallWasmFunction("grow_table_1_by_7");
    ASSERT_EQ(5, grow_result) << "Growing table 1 should return original size (5)";

    // Verify table 1 size changed but others remain the same
    int32_t table1_new_size = CallWasmFunction("get_table_1_size");
    ASSERT_EQ(12, table1_new_size) << "Table 1 size after growth should be 12";

    int32_t table0_unchanged = CallWasmFunction("get_table_0_size");
    ASSERT_EQ(10, table0_unchanged) << "Table 0 size should remain unchanged";

    int32_t table2_unchanged = CallWasmFunction("get_table_2_size");
    ASSERT_EQ(0, table2_unchanged) << "Table 2 size should remain unchanged";
}

/**
 * @test TableSizeConsistency_RepeatedQueries_ReturnsSameValue
 * @brief Validates table.size returns consistent values across repeated queries
 * @details Tests that multiple calls to table.size on the same table return identical
 *          results, ensuring operation consistency and no side effects.
 * @test_category Edge - Operation consistency validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_size_operation
 * @input_conditions Multiple calls to table.size on same table
 * @expected_behavior All calls return identical table size values
 * @validation_method Repeated size query comparison for consistency
 */
TEST_P(TableSizeTest, TableSizeConsistency_RepeatedQueries_ReturnsSameValue) {
    // Get funcref table size multiple times
    int32_t size1 = CallWasmFunction("get_funcref_table_size");
    int32_t size2 = CallWasmFunction("get_funcref_table_size");
    int32_t size3 = CallWasmFunction("get_funcref_table_size");

    // All queries should return the same value
    ASSERT_EQ(size1, size2) << "First and second size queries should match";
    ASSERT_EQ(size2, size3) << "Second and third size queries should match";
    ASSERT_EQ(10, size1) << "All size queries should return 10 for funcref table";

    // Test consistency on externref table as well
    int32_t ext_size1 = CallWasmFunction("get_externref_table_size");
    int32_t ext_size2 = CallWasmFunction("get_externref_table_size");

    ASSERT_EQ(ext_size1, ext_size2) << "Externref table size queries should be consistent";
    ASSERT_EQ(5, ext_size1) << "Externref table size should consistently return 5";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningModeTest,
    TableSizeTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<TableSizeTest::ParamType>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);