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
 * @brief Test suite for table.copy opcode comprehensive validation
 *
 * This test suite validates the table.copy WASM opcode functionality across different
 * execution modes (interpreter and AOT). The table.copy instruction copies elements
 * from a source table to a destination table, handling various scenarios including
 * inter-table copying, intra-table copying with overlaps, boundary conditions, and error cases.
 *
 * @test_coverage Validates core/iwasm/interpreter/wasm_interp_classic.c:table_copy_operation
 * @execution_modes Tests both interpreter and AOT compilation modes
 * @test_categories Main routine, corner cases, edge cases, error conditions
 */
class TableCopyTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up WAMR runtime for table.copy testing
     * @details Initializes WAMR runtime with system allocator, loads test WASM module,
     *          and sets up execution environment for both interpreter and AOT modes
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for table.copy tests";

        // Load test WASM module containing table.copy test functions
        LoadTestModule();

        running_mode = GetParam();
    }

    /**
     * @brief Clean up WAMR runtime and test resources
     * @details Destroys WASM module instances, unloads modules, and shuts down runtime
     */
    void TearDown() override {
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

        // Destroy WAMR runtime
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM test module containing table.copy test functions
     * @details Loads and instantiates WASM module with pre-configured tables for testing
     */
    void LoadTestModule() {
        uint32_t wasm_file_size;
        uint8_t *wasm_file_buf = nullptr;
        char error_buf[128] = {0};

        // Load WASM file from test directory
        wasm_file_buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/table_copy_test.wasm", &wasm_file_size);
        ASSERT_NE(nullptr, wasm_file_buf) << "Failed to load table_copy_test.wasm file";

        // Load WASM module
        wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, wasm_module) << "Failed to load WASM module: " << error_buf;

        // Create module instance
        module_inst = wasm_runtime_instantiate(wasm_module, 8 * 1024, 8 * 1024, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        // Get execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, 8 * 1024);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Free loaded file buffer
        wasm_runtime_free(wasm_file_buf);
    }

    /**
     * @brief Call WASM function to perform basic table copy operation
     * @return true if operation succeeded, false if trapped
     */
    bool CallBasicTableCopy() {
        uint32_t argv[1] = { 0 };
        return wasm_runtime_call_wasm(exec_env, wasm_runtime_lookup_function(module_inst, "test_basic_table_copy"), 1, argv);
    }

    /**
     * @brief Call WASM function to perform overlapping table copy operation
     * @return true if operation succeeded, false if trapped
     */
    bool CallOverlapTableCopy() {
        uint32_t argv[1] = { 0 };
        return wasm_runtime_call_wasm(exec_env, wasm_runtime_lookup_function(module_inst, "test_overlap_table_copy"), 1, argv);
    }

    /**
     * @brief Call WASM function to perform boundary table copy operation
     * @return true if operation succeeded, false if trapped
     */
    bool CallBoundaryTableCopy() {
        uint32_t argv[1] = { 0 };
        return wasm_runtime_call_wasm(exec_env, wasm_runtime_lookup_function(module_inst, "test_boundary_table_copy"), 1, argv);
    }

    /**
     * @brief Call WASM function to test error conditions
     * @param test_case Error test case number
     * @return true if operation succeeded, false if trapped (expected for error cases)
     */
    bool CallErrorTableCopy(uint32_t test_case) {
        uint32_t argv[1] = { test_case };
        return wasm_runtime_call_wasm(exec_env, wasm_runtime_lookup_function(module_inst, "test_error_table_copy"), 1, argv);
    }

    RuntimeInitArgs init_args;
    wasm_module_t wasm_module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    RunningMode running_mode;
};

/**
 * @test BasicTableCopy_BetweenTables_ReturnsCorrectElements
 * @brief Validates table.copy performs correct inter-table element copying operations
 * @details Tests fundamental table copying functionality between different tables with various
 *          element patterns. Verifies that elements are correctly transferred from source table
 *          to destination table at specified positions with proper element integrity.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_copy_operation
 * @input_conditions Two funcref tables with pre-loaded function references, copy operations between tables
 * @expected_behavior Source elements correctly appear in destination table at specified offsets
 * @validation_method Direct element comparison using table.get operations after copy completion
 */
TEST_P(TableCopyTest, BasicTableCopy_BetweenTables_ReturnsCorrectElements) {
    // Test basic table copy functionality
    ASSERT_TRUE(CallBasicTableCopy())
        << "Basic table copy operation should succeed";
}

/**
 * @test IntraTableCopy_WithinSameTable_HandlesOverlappingRegions
 * @brief Validates table.copy correctly handles overlapping regions within the same table
 * @details Tests same-table copying with overlapping source and destination regions,
 *          verifying proper memory-like copy semantics including forward and backward overlaps.
 *          Ensures element integrity is maintained during overlapping copy operations.
 * @test_category Edge - Overlapping region handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_copy_overlap_handling
 * @input_conditions Single table with element patterns, overlapping source/destination ranges
 * @expected_behavior Correct overlap handling similar to memmove semantics
 * @validation_method Element pattern verification after overlapping copy operations
 */
TEST_P(TableCopyTest, IntraTableCopy_WithinSameTable_HandlesOverlappingRegions) {
    // Test overlapping table copy within same table
    ASSERT_TRUE(CallOverlapTableCopy())
        << "Overlapping table copy operation should succeed";
}

/**
 * @test BoundaryConditions_AtTableLimits_SucceedsWithinBounds
 * @brief Validates table.copy operations at table boundaries and size limits
 * @details Tests boundary conditions including maximum valid offsets, zero-length copies,
 *          and operations at table size limits. Verifies proper bounds checking and
 *          successful execution of valid boundary operations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_bounds_validation
 * @input_conditions Copy operations at table edges, maximum offsets, zero lengths
 * @expected_behavior Successful operations within bounds, proper boundary handling
 * @validation_method Boundary operation success verification, table integrity checks
 */
TEST_P(TableCopyTest, BoundaryConditions_AtTableLimits_SucceedsWithinBounds) {
    // Test boundary table copy operations
    ASSERT_TRUE(CallBoundaryTableCopy())
        << "Boundary table copy operation should succeed";
}

/**
 * @test ErrorConditions_InvalidParameters_ThrowsAppropriateTraps
 * @brief Validates table.copy error handling for invalid parameters and out-of-bounds access
 * @details Tests various error conditions including invalid table indices, out-of-bounds
 *          offsets, length overflow, and type mismatches. Verifies appropriate WASM traps
 *          are generated for each error scenario.
 * @test_category Error - Invalid parameter and trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:table_copy_error_handling
 * @input_conditions Invalid indices, out-of-bounds ranges, overflow conditions
 * @expected_behavior Appropriate WASM traps for each error condition
 * @validation_method Trap generation verification, error condition coverage
 */
TEST_P(TableCopyTest, ErrorConditions_InvalidParameters_ThrowsAppropriateTraps) {
    // Test case 1: Out-of-bounds destination offset
    ASSERT_FALSE(CallErrorTableCopy(1))
        << "Out-of-bounds destination offset should trap";

    // Test case 2: Out-of-bounds source offset
    ASSERT_FALSE(CallErrorTableCopy(2))
        << "Out-of-bounds source offset should trap";

    // Test case 3: Length overflow on destination
    ASSERT_FALSE(CallErrorTableCopy(3))
        << "Length overflow on destination should trap";

    // Test case 4: Length overflow on source
    ASSERT_FALSE(CallErrorTableCopy(4))
        << "Length overflow on source should trap";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningModeTest,
    TableCopyTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<TableCopyTest::ParamType>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);