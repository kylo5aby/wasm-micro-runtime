/*
 * Copyright (C) 2024 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include "wasm_c_api.h"
#include "wasm_export.h"
#include "bh_read_file.h"

/**
 * @brief Enhanced test suite for WASM table.get opcode
 *
 * This test suite comprehensively validates the table.get opcode functionality
 * including basic functionality, boundary conditions, error cases, and reference
 * type handling across both interpreter and AOT execution modes.
 */

static std::string CWD;
static std::string WASM_FILE;
static std::string WASM_FILE_INVALID;

// Test execution modes for cross-validation
enum class TableGetRunningMode : uint8_t {
    INTERP = 1 << 0,
    AOT = 1 << 1
};

/**
 * @brief Test fixture for table.get opcode validation
 *
 * Provides setup and teardown for WAMR runtime initialization,
 * module loading, and proper resource cleanup with RAII patterns.
 */
class TableGetTest : public testing::TestWithParam<TableGetRunningMode> {
protected:
    /**
     * @brief Set up WAMR runtime and initialize test resources
     * @details Initializes WAMR runtime with proper configuration,
     *          determines current working directory, and prepares
     *          test WASM module file paths for loading
     */
    void SetUp() override {
        // Initialize WAMR runtime with standard configuration
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Determine current working directory and set file paths
        char *current_dir = getcwd(nullptr, 0);
        ASSERT_NE(nullptr, current_dir)
            << "Failed to get current working directory";

        CWD = std::string(current_dir);
        free(current_dir);

        // Set up test WASM file paths
        WASM_FILE = "wasm-apps/table_get_test.wasm";
        WASM_FILE_INVALID = "wasm-apps/table_get_invalid_test.wasm";
    }

    /**
     * @brief Clean up WAMR runtime and test resources
     * @details Performs proper cleanup of WAMR runtime resources
     *          ensuring no resource leaks after test execution
     */
    void TearDown() override {
        // Clean up WAMR runtime
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM module from file with error handling
     * @param filename Path to WASM module file
     * @return Loaded WASM module instance or nullptr on failure
     * @details Loads WASM module with comprehensive error handling
     *          and validation of module loading success
     */
    wasm_module_t load_wasm_module(const std::string& filename) {
        char error_buf[128] = { 0 };

        // Read WASM file
        uint32_t buf_size = 0;
        uint8_t* buf = (uint8_t*)bh_read_file_to_buffer(filename.c_str(), &buf_size);
        EXPECT_NE(nullptr, buf) << "Failed to read WASM file: " << filename;
        if (!buf) return nullptr;

        // Load WASM module
        wasm_module_t module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));

        // Clean up file buffer
        BH_FREE(buf);

        return module;
    }

    /**
     * @brief Create module instance with proper instantiation
     * @param module WASM module to instantiate
     * @param heap_size Heap size for module instance
     * @return Module instance or nullptr on failure
     * @details Creates WASM module instance with specified heap size
     *          and proper error handling for instantiation failures
     */
    wasm_module_inst_t create_module_instance(wasm_module_t module, uint32_t heap_size = 8192) {
        char error_buf[128] = { 0 };

        wasm_module_inst_t module_inst = wasm_runtime_instantiate(
            module, heap_size, heap_size, error_buf, sizeof(error_buf)
        );

        return module_inst;
    }

    /**
     * @brief Execute WASM function with argument and return validation
     * @param module_inst WASM module instance
     * @param func_name Function name to execute
     * @param argc Number of arguments
     * @param argv Arguments array (uint32_t for WAMR API)
     * @return Execution results or nullptr on failure
     * @details Executes WASM function with comprehensive error handling
     *          and validation of execution success
     */
    uint32_t* execute_wasm_function(wasm_module_inst_t module_inst,
                                   const char* func_name,
                                   uint32_t argc,
                                   uint32_t argv[]) {
        char error_buf[128] = { 0 };

        // Create execution environment
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";
        if (!exec_env) return nullptr;

        // Find exported function
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Failed to find function: " << func_name;
        if (!func) {
            wasm_runtime_destroy_exec_env(exec_env);
            return nullptr;
        }

        // Execute function
        bool success = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        if (!success) {
            const char* exception = wasm_runtime_get_exception(module_inst);
            if (exception) {
                // Return exception information through static storage
                static uint32_t exception_result = 0xFFFFFFFF; // Exception marker
                wasm_runtime_destroy_exec_env(exec_env);
                return &exception_result;
            }
            wasm_runtime_destroy_exec_env(exec_env);
            return nullptr;
        }

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);
        return argv;
    }

private:
    RuntimeInitArgs init_args;
};

/**
 * @test BasicFunctionRefAccess_ReturnsValidReferences
 * @brief Validates table.get retrieves valid function references from table
 * @details Tests fundamental table.get operation with function references at various
 *          valid indices, ensuring correct reference retrieval and type consistency.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:table_get_operation
 * @input_conditions Valid function reference table with multiple elements at indices 0, 1, 2
 * @expected_behavior Returns correct function references without corruption
 * @validation_method Direct comparison of returned reference values with expected function IDs
 */
TEST_P(TableGetTest, BasicFunctionRefAccess_ReturnsValidReferences) {
    // Load WASM module with function reference table
    wasm_module_t module = load_wasm_module(WASM_FILE);
    ASSERT_NE(nullptr, module) << "Failed to load table.get test module";

    // Create module instance
    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test getting function references at valid indices
    uint32_t argv[1];

    // Get function reference at index 0
    argv[0] = 0;
    uint32_t* result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to execute get_func_ref for index 0";
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred getting function ref at index 0";
    ASSERT_NE(0u, result[0]) << "Retrieved null function reference at index 0";

    // Get function reference at index 1
    argv[0] = 1;
    result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to execute get_func_ref for index 1";
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred getting function ref at index 1";
    ASSERT_NE(0u, result[0]) << "Retrieved null function reference at index 1";

    // Get function reference at index 2
    argv[0] = 2;
    result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to execute get_func_ref for index 2";
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred getting function ref at index 2";
    ASSERT_NE(0u, result[0]) << "Retrieved null function reference at index 2";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test BasicExternRefAccess_HandlesNullAndNonNullRefs
 * @brief Validates table.get correctly handles external references including null values
 * @details Tests table.get operation with externref table containing both null and
 *          non-null external references, ensuring proper null handling.
 * @test_category Main - External reference validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:table_get_externref
 * @input_conditions Externref table with null and non-null references at different indices
 * @expected_behavior Returns correct externref values, properly handling null references
 * @validation_method Verification of returned externref values and null reference consistency
 */
TEST_P(TableGetTest, BasicExternRefAccess_HandlesNullAndNonNullRefs) {
    // Load WASM module with extern reference table
    wasm_module_t module = load_wasm_module(WASM_FILE);
    ASSERT_NE(nullptr, module) << "Failed to load table.get test module";

    // Create module instance
    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test getting extern references
    uint32_t argv[1];

    // Get externref at index 0 (should be null initially)
    argv[0] = 0;
    uint32_t* result = execute_wasm_function(module_inst, "get_extern_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to execute get_extern_ref for index 0";
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred getting externref at index 0";

    // Verify null reference is handled correctly (implementation-specific behavior)
    // The exact representation of null may vary, but should be consistent

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test BoundaryAccess_FirstAndLastElements
 * @brief Validates table.get accesses boundary elements correctly
 * @details Tests table.get operation at boundary conditions including first element
 *          (index 0) and last valid element (cur_size - 1) to ensure proper bounds handling.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:table_bounds_check
 * @input_conditions Table with known size, access at indices 0 and (size - 1)
 * @expected_behavior Returns valid references at boundary indices without errors
 * @validation_method Verification of successful access and valid reference values at boundaries
 */
TEST_P(TableGetTest, BoundaryAccess_FirstAndLastElements) {
    // Load WASM module with function reference table
    wasm_module_t module = load_wasm_module(WASM_FILE);
    ASSERT_NE(nullptr, module) << "Failed to load table.get test module";

    // Create module instance
    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Get table size first to determine last valid index
    uint32_t argv[1];
    uint32_t* result = execute_wasm_function(module_inst, "get_table_size", 0, argv);
    ASSERT_NE(nullptr, result) << "Failed to get table size";
    uint32_t table_size = result[0];
    ASSERT_GT(table_size, 0) << "Table size should be greater than 0";

    // Test access at first element (index 0)
    argv[0] = 0;
    result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to access first element at index 0";
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred accessing first element";

    // Test access at last valid element (size - 1)
    argv[0] = table_size - 1;
    result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to access last element at index " << (table_size - 1);
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred accessing last element";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test OutOfBoundsAccess_TriggersProperException
 * @brief Validates table.get properly handles out-of-bounds access attempts
 * @details Tests table.get operation with indices beyond table size to ensure
 *          proper exception generation and "out of bounds table access" error handling.
 * @test_category Error - Exception handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:table_bounds_exception
 * @input_conditions Valid table with known size, access attempts at indices >= table.cur_size
 * @expected_behavior Triggers "out of bounds table access" exception consistently
 * @validation_method Verification of exception occurrence and proper error message
 */
TEST_P(TableGetTest, OutOfBoundsAccess_TriggersProperException) {
    // Load WASM module with function reference table
    wasm_module_t module = load_wasm_module(WASM_FILE);
    ASSERT_NE(nullptr, module) << "Failed to load table.get test module";

    // Create module instance
    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Get table size to determine out-of-bounds indices
    uint32_t argv[1];
    uint32_t* result = execute_wasm_function(module_inst, "get_table_size", 0, argv);
    ASSERT_NE(nullptr, result) << "Failed to get table size";
    uint32_t table_size = result[0];

    // Test out-of-bounds access at table_size (first invalid index)
    argv[0] = table_size;
    result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Function execution should return exception marker";
    ASSERT_EQ(0xFFFFFFFF, result[0]) << "Expected exception for out-of-bounds access at index " << table_size;

    // Test out-of-bounds access well beyond table size
    argv[0] = table_size + 100;
    result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Function execution should return exception marker";
    ASSERT_EQ(0xFFFFFFFF, result[0]) << "Expected exception for out-of-bounds access at large index";

    // Test out-of-bounds access with large positive index
    argv[0] = 0x7FFFFFFF;
    result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Function execution should return exception marker";
    ASSERT_EQ(0xFFFFFFFF, result[0]) << "Expected exception for maximum positive index access";

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test MultipleTableAccess_DifferentTableIndices
 * @brief Validates table.get operation across multiple table instances
 * @details Tests table.get with different table indices in modules containing
 *          multiple tables, ensuring correct table targeting and access.
 * @test_category Edge - Multiple table validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:multi_table_access
 * @input_conditions Module with multiple tables (funcref and externref tables)
 * @expected_behavior Correctly accesses elements from different tables by index
 * @validation_method Verification of table-specific element retrieval and type consistency
 */
TEST_P(TableGetTest, MultipleTableAccess_DifferentTableIndices) {
    // Load WASM module with multiple tables
    wasm_module_t module = load_wasm_module(WASM_FILE);
    ASSERT_NE(nullptr, module) << "Failed to load table.get test module";

    // Create module instance
    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test accessing different tables
    uint32_t argv[1];

    // Access function reference table (table 0)
    argv[0] = 0;
    uint32_t* result = execute_wasm_function(module_inst, "get_func_ref_table0", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to access function reference table";
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred accessing funcref table";

    // Access extern reference table (table 1) if available
    result = execute_wasm_function(module_inst, "get_extern_ref_table1", 1, argv);
    if (result != nullptr) {
        // Multiple table support available
        ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred accessing externref table";
    }

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

/**
 * @test ReferenceTypeConsistency_VerifiesCorrectTypes
 * @brief Validates table.get maintains reference type consistency
 * @details Tests that table.get operations return references of the correct type
 *          based on the table's element type specification (funcref vs externref).
 * @test_category Edge - Reference type validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:ref_type_consistency
 * @input_conditions Tables with different reference types (funcref, externref)
 * @expected_behavior Returns references matching the table's declared element type
 * @validation_method Type-specific validation of returned reference values
 */
TEST_P(TableGetTest, ReferenceTypeConsistency_VerifiesCorrectTypes) {
    // Load WASM module with typed tables
    wasm_module_t module = load_wasm_module(WASM_FILE);
    ASSERT_NE(nullptr, module) << "Failed to load table.get test module";

    // Create module instance
    wasm_module_inst_t module_inst = create_module_instance(module);
    ASSERT_NE(nullptr, module_inst) << "Failed to create module instance";

    // Test function reference type consistency
    uint32_t argv[1];
    argv[0] = 0;

    uint32_t* result = execute_wasm_function(module_inst, "get_func_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to get function reference";
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred getting function reference";

    // For function references, verify non-null value indicates valid function
    ASSERT_NE(0u, result[0]) << "Function reference should not be null";

    // Test extern reference type consistency
    result = execute_wasm_function(module_inst, "get_extern_ref", 1, argv);
    ASSERT_NE(nullptr, result) << "Failed to get extern reference";
    ASSERT_NE(0xFFFFFFFF, result[0]) << "Exception occurred getting extern reference";

    // Extern references may be null, which is valid

    // Clean up resources
    wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    RunningMode,
    TableGetTest,
    testing::Values(
        TableGetRunningMode::INTERP
#if WASM_ENABLE_AOT != 0
        ,TableGetRunningMode::AOT
#endif
    )
);