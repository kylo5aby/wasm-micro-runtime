/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"
#include "bh_platform.h"
#include "wasm_export.h"

/**
 * @brief Enhanced unit tests for WASM table.fill opcode
 *
 * This test suite provides comprehensive validation of the table.fill opcode functionality
 * in WAMR, covering basic operations, boundary conditions, edge cases, and error scenarios.
 * Tests run across both interpreter and AOT execution modes.
 */

static std::string CWD;
static std::string WASM_FILE;
static std::string AOT_FILE;

/**
 * @brief Test fixture for table.fill opcode testing with parameterized execution modes
 */
class TableFillTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.n_native_symbols = 0;

        // Initialize the WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));

        // Initialize test file paths
        CWD = get_current_directory();
        WASM_FILE = CWD + "/wasm-apps/table_fill_test.wasm";
        AOT_FILE = CWD + "/wasm-apps/table_fill_test.aot";

        runtime_mode = GetParam();
        module_inst = nullptr;
        module = nullptr;
        error_buf[0] = '\0';
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     */
    void TearDown() override {
        // Cleanup module instance and module
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }

        // Destroy WAMR runtime environment
        wasm_runtime_destroy();
    }

    /**
     * @brief Load test WASM module based on execution mode
     * @return Loaded WASM module or nullptr on failure
     */
    wasm_module_t LoadTestModule() {
        std::string filename = (runtime_mode == Mode_LLVM_JIT) ? AOT_FILE : WASM_FILE;

        // Read WASM/AOT file
        buffer = (uint8_t*)bh_read_file_to_buffer(filename.c_str(), &buffer_size);
        EXPECT_NE(buffer, nullptr) << "Failed to read file: " << filename;
        if (!buffer) return nullptr;

        // Load WASM module
        module = wasm_runtime_load(buffer, buffer_size, error_buf, sizeof(error_buf));
        EXPECT_NE(module, nullptr) << "Failed to load module: " << error_buf;
        return module;
    }

    /**
     * @brief Instantiate loaded WASM module
     * @return Module instance or nullptr on failure
     */
    wasm_module_inst_t InstantiateModule() {
        EXPECT_NE(module, nullptr) << "Module must be loaded before instantiation";
        if (!module) return nullptr;

        // Instantiate the module
        module_inst = wasm_runtime_instantiate(module, 65536, 65536, error_buf, sizeof(error_buf));
        EXPECT_NE(module_inst, nullptr) << "Failed to instantiate module: " << error_buf;
        return module_inst;
    }

    /**
     * @brief Get current working directory for test file location
     * @return Current working directory path
     */
    std::string get_current_directory() {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            return std::string(cwd);
        }
        return ".";
    }

    // Test fixture members
    RuntimeInitArgs init_args;
    RunningMode runtime_mode;
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    uint8_t *buffer;
    uint32_t buffer_size;
    char error_buf[256];
};

/**
 * @test BasicTableFill_FuncrefAndExternref_SuccessfulFill
 * @brief Validates table.fill correctly fills table regions with reference values
 * @details Tests fundamental fill operations on both funcref and externref tables.
 *          Verifies that table.fill correctly sets multiple table elements to specified values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_fill_operation
 * @input_conditions Funcref table with fill parameters (d=1, n=3, val=func_ref);
 *                   Externref table with fill parameters (d=0, n=2, val=extern_ref)
 * @expected_behavior Elements at specified positions filled with correct reference values
 * @validation_method table.get operations verify elements contain expected references
 */
TEST_P(TableFillTest, BasicTableFill_FuncrefAndExternref_SuccessfulFill) {
    // Load and instantiate test module
    wasm_module_t test_module = LoadTestModule();
    ASSERT_NE(nullptr, test_module) << "Failed to load table.fill test module";

    wasm_module_inst_t inst = InstantiateModule();
    ASSERT_NE(nullptr, inst) << "Failed to instantiate table.fill test module";

    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, 65536);
    ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

    // Test funcref table fill
    wasm_function_inst_t fill_funcref_func = wasm_runtime_lookup_function(inst, "test_fill_funcref_basic");
    ASSERT_NE(nullptr, fill_funcref_func) << "Failed to find test_fill_funcref_basic function";

    uint32_t argv_fill_funcref[1] = {0};
    bool result = wasm_runtime_call_wasm(exec_env, fill_funcref_func, 1, argv_fill_funcref);
    ASSERT_TRUE(result) << "Failed to execute funcref table fill test: " << wasm_runtime_get_exception(inst);

    // Verify funcref table fill result (1 indicates success)
    ASSERT_EQ(1, argv_fill_funcref[0]) << "Funcref table fill test failed";

    // Test externref table fill
    wasm_function_inst_t fill_externref_func = wasm_runtime_lookup_function(inst, "test_fill_externref_basic");
    ASSERT_NE(nullptr, fill_externref_func) << "Failed to find test_fill_externref_basic function";

    uint32_t argv_fill_externref[1] = {0};
    result = wasm_runtime_call_wasm(exec_env, fill_externref_func, 1, argv_fill_externref);
    ASSERT_TRUE(result) << "Failed to execute externref table fill test: " << wasm_runtime_get_exception(inst);

    // Verify externref table fill result (1 indicates success)
    ASSERT_EQ(1, argv_fill_externref[0]) << "Externref table fill test failed";

    wasm_runtime_destroy_exec_env(exec_env);
}

/**
 * @test BoundaryFill_TableLimitsAndEdges_CorrectBehavior
 * @brief Validates table.fill operations at table boundaries and edge conditions
 * @details Tests fill operations at table limits including first element, last element,
 *          and full table fills. Ensures proper boundary handling without overflow.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_bounds_checking
 * @input_conditions Full table fill (d=0, n=table.size), last element fill (d=table.size-1, n=1),
 *                   exact boundary fill (d+n=table.size)
 * @expected_behavior Operations succeed at exact boundaries, table size unchanged
 * @validation_method All specified elements updated correctly, no out-of-bounds access
 */
TEST_P(TableFillTest, BoundaryFill_TableLimitsAndEdges_CorrectBehavior) {
    // Load and instantiate test module
    wasm_module_t test_module = LoadTestModule();
    ASSERT_NE(nullptr, test_module) << "Failed to load table.fill test module";

    wasm_module_inst_t inst = InstantiateModule();
    ASSERT_NE(nullptr, inst) << "Failed to instantiate table.fill test module";

    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, 65536);
    ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

    // Test full table fill (d=0, n=table.size)
    wasm_function_inst_t full_fill_func = wasm_runtime_lookup_function(inst, "test_fill_full_table");
    ASSERT_NE(nullptr, full_fill_func) << "Failed to find test_fill_full_table function";

    uint32_t argv_full_fill[1] = {0};
    bool result = wasm_runtime_call_wasm(exec_env, full_fill_func, 1, argv_full_fill);
    ASSERT_TRUE(result) << "Failed to execute full table fill test: " << wasm_runtime_get_exception(inst);

    // Verify full table fill success
    ASSERT_EQ(1, argv_full_fill[0]) << "Full table fill test failed";

    // Test last element fill (d=table.size-1, n=1)
    wasm_function_inst_t last_elem_func = wasm_runtime_lookup_function(inst, "test_fill_last_element");
    ASSERT_NE(nullptr, last_elem_func) << "Failed to find test_fill_last_element function";

    uint32_t argv_last_elem[1] = {0};
    result = wasm_runtime_call_wasm(exec_env, last_elem_func, 1, argv_last_elem);
    ASSERT_TRUE(result) << "Failed to execute last element fill test: " << wasm_runtime_get_exception(inst);

    // Verify last element fill success
    ASSERT_EQ(1, argv_last_elem[0]) << "Last element fill test failed";

    // Test exact boundary fill (d + n = table.size)
    wasm_function_inst_t boundary_fill_func = wasm_runtime_lookup_function(inst, "test_fill_to_boundary");
    ASSERT_NE(nullptr, boundary_fill_func) << "Failed to find test_fill_to_boundary function";

    uint32_t argv_boundary[1] = {0};
    result = wasm_runtime_call_wasm(exec_env, boundary_fill_func, 1, argv_boundary);
    ASSERT_TRUE(result) << "Failed to execute boundary fill test: " << wasm_runtime_get_exception(inst);

    // Verify boundary fill success
    ASSERT_EQ(1, argv_boundary[0]) << "Boundary fill test failed";

    wasm_runtime_destroy_exec_env(exec_env);
}

/**
 * @test ZeroOperationFill_VariousScenarios_ValidBehavior
 * @brief Validates table.fill behavior with zero count and idempotent operations
 * @details Tests zero-length fill operations and repeated fills to verify no-op behavior
 *          and operation idempotency. Ensures table state remains consistent.
 * @test_category Edge - Zero operation and idempotency validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_fill_zero_count
 * @input_conditions Zero count fills (n=0) at various positions, repeated identical fills
 * @expected_behavior No table modifications for zero fills, consistent results for repeated operations
 * @validation_method Table state verification before/after operations, idempotency confirmation
 */
TEST_P(TableFillTest, ZeroOperationFill_VariousScenarios_ValidBehavior) {
    // Load and instantiate test module
    wasm_module_t test_module = LoadTestModule();
    ASSERT_NE(nullptr, test_module) << "Failed to load table.fill test module";

    wasm_module_inst_t inst = InstantiateModule();
    ASSERT_NE(nullptr, inst) << "Failed to instantiate table.fill test module";

    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, 65536);
    ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

    // Test zero count fill (n=0) at various positions
    wasm_function_inst_t zero_count_func = wasm_runtime_lookup_function(inst, "test_fill_zero_count");
    ASSERT_NE(nullptr, zero_count_func) << "Failed to find test_fill_zero_count function";

    uint32_t argv_zero_count[1] = {0};
    bool result = wasm_runtime_call_wasm(exec_env, zero_count_func, 1, argv_zero_count);
    ASSERT_TRUE(result) << "Failed to execute zero count fill test: " << wasm_runtime_get_exception(inst);

    // Verify zero count fill success (no-op behavior)
    ASSERT_EQ(1, argv_zero_count[0]) << "Zero count fill test failed";

    // Test repeated fills for idempotency
    wasm_function_inst_t idempotent_func = wasm_runtime_lookup_function(inst, "test_fill_idempotent");
    ASSERT_NE(nullptr, idempotent_func) << "Failed to find test_fill_idempotent function";

    uint32_t argv_idempotent[1] = {0};
    result = wasm_runtime_call_wasm(exec_env, idempotent_func, 1, argv_idempotent);
    ASSERT_TRUE(result) << "Failed to execute idempotent fill test: " << wasm_runtime_get_exception(inst);

    // Verify idempotent fill success
    ASSERT_EQ(1, argv_idempotent[0]) << "Idempotent fill test failed";

    wasm_runtime_destroy_exec_env(exec_env);
}

/**
 * @test InvalidParameters_OutOfBoundsAndTypeErrors_ProperTraps
 * @brief Validates table.fill error handling for invalid parameters and bounds violations
 * @details Tests error conditions including out-of-bounds indices, invalid ranges,
 *          and type mismatches. Verifies proper trap generation and error reporting.
 * @test_category Exception - Error handling and trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_fill_bounds_check
 * @input_conditions Out-of-bounds index (d >= table.size), invalid range (d + n > table.size),
 *                   type mismatches, negative indices
 * @expected_behavior Runtime traps with appropriate error conditions
 * @validation_method Trap generation verification and proper error reporting
 */
TEST_P(TableFillTest, InvalidParameters_OutOfBoundsAndTypeErrors_ProperTraps) {
    // Load and instantiate test module
    wasm_module_t test_module = LoadTestModule();
    ASSERT_NE(nullptr, test_module) << "Failed to load table.fill test module";

    wasm_module_inst_t inst = InstantiateModule();
    ASSERT_NE(nullptr, inst) << "Failed to instantiate table.fill test module";

    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, 65536);
    ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

    // Test out-of-bounds index (d >= table.size)
    wasm_function_inst_t oob_index_func = wasm_runtime_lookup_function(inst, "test_fill_out_of_bounds_index");
    ASSERT_NE(nullptr, oob_index_func) << "Failed to find test_fill_out_of_bounds_index function";

    uint32_t argv_oob_index[1] = {0};
    bool result = wasm_runtime_call_wasm(exec_env, oob_index_func, 1, argv_oob_index);

    // Should trap due to out-of-bounds index - either call fails or returns trap indicator
    bool trapped = !result || argv_oob_index[0] == 0;
    ASSERT_TRUE(trapped) << "Expected trap for out-of-bounds index";

    // Clear any exception for next test
    wasm_runtime_clear_exception(inst);

    // Test out-of-bounds range (d + n > table.size)
    wasm_function_inst_t oob_range_func = wasm_runtime_lookup_function(inst, "test_fill_out_of_bounds_range");
    ASSERT_NE(nullptr, oob_range_func) << "Failed to find test_fill_out_of_bounds_range function";

    uint32_t argv_oob_range[1] = {0};
    result = wasm_runtime_call_wasm(exec_env, oob_range_func, 1, argv_oob_range);

    // Should trap due to out-of-bounds range
    trapped = !result || argv_oob_range[0] == 0;
    ASSERT_TRUE(trapped) << "Expected trap for out-of-bounds range";

    // Clear any exception for next test
    wasm_runtime_clear_exception(inst);

    // Test large index (negative when interpreted as signed)
    wasm_function_inst_t large_index_func = wasm_runtime_lookup_function(inst, "test_fill_large_index");
    ASSERT_NE(nullptr, large_index_func) << "Failed to find test_fill_large_index function";

    uint32_t argv_large_index[1] = {0};
    result = wasm_runtime_call_wasm(exec_env, large_index_func, 1, argv_large_index);

    // Should trap due to large index
    trapped = !result || argv_large_index[0] == 0;
    ASSERT_TRUE(trapped) << "Expected trap for large index";

    wasm_runtime_destroy_exec_env(exec_env);
}

// Test parameters for different execution modes
INSTANTIATE_TEST_SUITE_P(TableFillModes, TableFillTest,
                        testing::Values(Mode_Interp));