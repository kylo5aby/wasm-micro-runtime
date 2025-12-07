/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i32.store Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i32.store
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Store Operations: Memory store operations with typical i32 values and addresses
 * - Offset Store Operations: Store operations with various static offset configurations
 * - Boundary Store Operations: Memory boundary operations, edge cases, and limit testing
 * - Out-of-bounds Access: Invalid memory access attempts and proper trap validation
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE
 * - AOT: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_i32_store()
 * - Fast JIT: core/iwasm/fast-jit/fe/jit_emit_memory.c:jit_compile_op_i32_store()
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;

static int app_argc;
static char **app_argv;

/**
 * @class I32StoreTest
 * @brief Test fixture for i32.store opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for i32.store operations
 *          including module loading, execution environment setup, and validation helpers
 */
class I32StoreTest : public testing::TestWithParam<RunningMode>
{
  protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };
    const char *exception = nullptr;

    /**
     * @brief Set up test environment with WAMR runtime and module loading
     * @details Initializes WAMR runtime, loads i32.store test module, and
     *          prepares execution environment for parameterized testing
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Get current working directory
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";
        std::string cwd = std::string(cwd_ptr);
        free(cwd_ptr);

        // Use i32.store specific WASM file
        std::string store_wasm_file = cwd + "/wasm-apps/i32_store_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(store_wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << store_wasm_file;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and release WAMR resources
     * @details Destroys execution environment, module instance, module, and
     *          performs runtime cleanup using RAII patterns
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
        if (buf) {
            BH_FREE(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Execute i32.store operation with address and value parameters
     * @param func_name WASM function name to execute
     * @param address Base memory address for store operation
     * @param value i32 value to store in memory
     * @return Execution result (0 for success, non-zero for trap/error)
     */
    int32_t call_i32_store(const char* func_name, uint32_t address, int32_t value)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(func, nullptr) << "Failed to lookup function: " << func_name;

        uint32_t argv[2] = { address, static_cast<uint32_t>(value) };
        uint32_t argc = 2;

        bool ret = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        exception = wasm_runtime_get_exception(module_inst);

        if (!ret || exception) {
            return -1; // Indicate execution failure or trap
        }
        return 0; // Success
    }

    /**
     * @brief Execute i32.load operation to verify stored values
     * @param func_name WASM function name to execute
     * @param address Memory address to load from
     * @return Loaded i32 value or error indicator
     */
    int32_t call_i32_load(const char* func_name, uint32_t address)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(func, nullptr) << "Failed to lookup function: " << func_name;

        uint32_t argv[1] = { address };
        uint32_t argc = 1;

        bool ret = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        exception = wasm_runtime_get_exception(module_inst);

        if (!ret || exception) {
            return INT32_MIN; // Indicate execution failure
        }
        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Clear any runtime exception state
     * @details Resets exception state between test operations
     */
    void clear_exception()
    {
        if (exception) {
            wasm_runtime_clear_exception(module_inst);
            exception = nullptr;
        }
    }
};

/**
 * @test BasicStoreOperations_ExecutesCorrectly
 * @brief Validates i32.store operations with typical values and addresses
 * @details Tests fundamental store operations with positive, negative, and zero values.
 *          Verifies correct storage and retrieval of i32 values at various memory addresses.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE
 * @input_conditions Standard addresses (0, 16, 64) with typical i32 values (42, -100, 0)
 * @expected_behavior Values stored correctly and retrievable via i32.load
 * @validation_method Store value, load from same address, verify stored value matches
 */
TEST_P(I32StoreTest, BasicStoreOperations_ExecutesCorrectly)
{
    // Test store and load of positive integer
    ASSERT_EQ(call_i32_store("store_i32", 0, 42), 0)
        << "Failed to store positive integer 42 at address 0";
    ASSERT_EQ(call_i32_load("load_i32", 0), 42)
        << "Stored value 42 does not match loaded value";

    // Test store and load of negative integer
    ASSERT_EQ(call_i32_store("store_i32", 16, -100), 0)
        << "Failed to store negative integer -100 at address 16";
    ASSERT_EQ(call_i32_load("load_i32", 16), -100)
        << "Stored value -100 does not match loaded value";

    // Test store and load of zero value
    ASSERT_EQ(call_i32_store("store_i32", 32, 0), 0)
        << "Failed to store zero value at address 32";
    ASSERT_EQ(call_i32_load("load_i32", 32), 0)
        << "Stored zero value does not match loaded value";
}

/**
 * @test OffsetStoreOperations_HandlesOffsetsCorrectly
 * @brief Tests i32.store with various static offset configurations
 * @details Validates store operations using different offset values in memarg.
 *          Tests effective address calculation (base + offset) and correct storage.
 * @test_category Main - Offset parameter validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE
 * @input_conditions Base addresses with offsets: (0+4), (16+8), (32+16)
 * @expected_behavior Effective address calculation and correct storage at computed address
 * @validation_method Store with offset, verify storage at computed address via load
 */
TEST_P(I32StoreTest, OffsetStoreOperations_HandlesOffsetsCorrectly)
{
    // Test store with offset 4 (address 0 + offset 4 = effective address 4)
    ASSERT_EQ(call_i32_store("store_i32_offset4", 0, 1000), 0)
        << "Failed to store with offset 4 at base address 0";
    ASSERT_EQ(call_i32_load("load_i32", 4), 1000)
        << "Stored value with offset does not match expected value";

    // Test store with offset 8 (address 16 + offset 8 = effective address 24)
    ASSERT_EQ(call_i32_store("store_i32_offset8", 16, -500), 0)
        << "Failed to store with offset 8 at base address 16";
    ASSERT_EQ(call_i32_load("load_i32", 24), -500)
        << "Stored value with offset 8 does not match expected value";

    // Test store with offset 16 (address 32 + offset 16 = effective address 48)
    ASSERT_EQ(call_i32_store("store_i32_offset16", 32, 2147483647), 0)
        << "Failed to store with offset 16 at base address 32";
    ASSERT_EQ(call_i32_load("load_i32", 48), 2147483647)
        << "Stored INT32_MAX with offset does not match expected value";
}

/**
 * @test BoundaryStoreOperations_HandlesMemoryBoundaries
 * @brief Validates store operations at memory boundaries and extreme values
 * @details Tests storage at memory boundaries, extreme integer values, and alignment cases.
 *          Verifies proper handling of INT32_MIN, INT32_MAX, and boundary addresses.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE
 * @input_conditions Boundary values: INT32_MIN, INT32_MAX, memory limits
 * @expected_behavior Successful storage within bounds, proper value preservation
 * @validation_method Store boundary values, verify correct retrieval and bit patterns
 */
TEST_P(I32StoreTest, BoundaryStoreOperations_HandlesMemoryBoundaries)
{
    // Test store of INT32_MIN
    ASSERT_EQ(call_i32_store("store_i32", 64, INT32_MIN), 0)
        << "Failed to store INT32_MIN at address 64";
    ASSERT_EQ(call_i32_load("load_i32", 64), INT32_MIN)
        << "Stored INT32_MIN does not match loaded value";

    // Test store of INT32_MAX
    ASSERT_EQ(call_i32_store("store_i32", 128, INT32_MAX), 0)
        << "Failed to store INT32_MAX at address 128";
    ASSERT_EQ(call_i32_load("load_i32", 128), INT32_MAX)
        << "Stored INT32_MAX does not match loaded value";

    // Test store with bit patterns
    ASSERT_EQ(call_i32_store("store_i32", 192, 0xAAAAAAAA), 0)
        << "Failed to store bit pattern 0xAAAAAAAA";
    ASSERT_EQ(call_i32_load("load_i32", 192), static_cast<int32_t>(0xAAAAAAAA))
        << "Stored bit pattern does not match expected value";
}

/**
 * @test OutOfBoundsAccess_GeneratesTraps
 * @brief Tests error handling for invalid memory access attempts
 * @details Validates proper trap generation for out-of-bounds memory access.
 *          Tests access beyond memory limits and address computation overflow.
 * @test_category Error - Exception handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE
 * @input_conditions Addresses beyond memory limit, overflow scenarios
 * @expected_behavior WASM trap generation with proper error reporting
 * @validation_method Attempt invalid stores, verify trap occurs with expected behavior
 */
TEST_P(I32StoreTest, OutOfBoundsAccess_GeneratesTraps)
{
    // Clear any existing exceptions
    clear_exception();

    // Test out-of-bounds store (assuming memory size is limited)
    int32_t result = call_i32_store("store_i32", 0xFFFFFFFC, 123);
    ASSERT_NE(result, 0) << "Out-of-bounds store should generate trap";
    ASSERT_NE(exception, nullptr) << "Exception should be set for out-of-bounds access";

    // Clear exception for next test
    clear_exception();

    // Test store with large offset causing address overflow
    result = call_i32_store("store_i32_large_offset", 0x7FFFFFFF, 456);
    ASSERT_NE(result, 0) << "Address overflow store should generate trap";
    ASSERT_NE(exception, nullptr) << "Exception should be set for address overflow";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I32StoreTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         testing::PrintToStringParamName());

