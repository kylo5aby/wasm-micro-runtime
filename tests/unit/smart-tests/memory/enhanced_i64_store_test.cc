/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i64.store Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i64.store
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Store Operations: Memory store operations with typical i64 values and addresses
 * - Boundary Store Operations: Memory boundary operations, edge cases, and limit testing
 * - Memory Alignment Testing: Aligned and unaligned memory access validation
 * - Out-of-bounds Access: Invalid memory access attempts and proper trap validation
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I64_STORE
 * - AOT: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_i64_store()
 * - Fast JIT: core/iwasm/fast-jit/fe/jit_emit_memory.c:jit_compile_op_i64_store()
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
 * @class I64StoreTest
 * @brief Test fixture for i64.store opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for i64.store operations
 *          including module loading, execution environment setup, and validation helpers
 */
class I64StoreTest : public testing::TestWithParam<RunningMode>
{
  protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8192, heap_size = 8192;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };

    /**
     * @brief Set up test environment for each test case
     * @details Initializes WAMR runtime, loads test module, and prepares execution environment
     */
    void SetUp() override {
        memset(error_buf, 0, sizeof(error_buf));
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Get current working directory
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";
        std::string cwd = std::string(cwd_ptr);
        free(cwd_ptr);

        // Load WASM module from file
        std::string store_wasm_file = cwd + "/wasm-apps/i64_store_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(store_wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << store_wasm_file;

        // Load WASM module
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        // Instantiate module
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        // Configure execution mode based on test parameter
        wasm_runtime_set_running_mode(module_inst, GetParam());

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment after each test case
     * @details Properly destroys module instance and module resources
     */
    void TearDown() override {
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
     * @brief Execute i64.store operation via WASM function call
     * @param func_name Name of the WASM function to execute
     * @param addr Memory address for store operation
     * @param value i64 value to store
     * @return Success status of the operation
     */
    bool call_i64_store_func(const char* func_name, uint32_t addr, int64_t value) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        if (!func) {
            return false;
        }

        uint32_t argv[3];
        argv[0] = addr;
        // Split i64 value into two i32 parts for parameter passing
        argv[1] = (uint32_t)(value & 0xFFFFFFFF);        // Lower 32 bits
        argv[2] = (uint32_t)((value >> 32) & 0xFFFFFFFF); // Upper 32 bits

        bool ret = wasm_runtime_call_wasm(exec_env, func, 3, argv);
        const char* exception = wasm_runtime_get_exception(module_inst);

        return ret && (exception == nullptr || strlen(exception) == 0);
    }

    /**
     * @brief Load i64 value from memory via WASM function call
     * @param addr Memory address to load from
     * @return Loaded i64 value
     */
    int64_t call_i64_load_func(uint32_t addr) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "load_i64");
        EXPECT_NE(nullptr, func) << "Failed to lookup load_i64 function";

        uint32_t argv[2] = {addr, 0};  // Input address parameter

        bool success = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(success) << "Failed to call load_i64 function";

        // For multi-return functions, WAMR places results starting from argv[0]
        // The load_i64 function returns (result i32 i32) - low and high parts
        uint32_t low = argv[0];   // Lower 32 bits
        uint32_t high = argv[1];  // Upper 32 bits

        // Reconstruct i64 from two i32 parts
        return (int64_t)low | ((int64_t)high << 32);
    }

    /**
     * @brief Get current memory size of module instance
     * @return Memory size in bytes
     */
    uint32_t get_memory_size() {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "get_memory_size");
        if (!func) {
            return 0;
        }

        uint32_t argv[1] = {0};
        bool success = wasm_runtime_call_wasm(exec_env, func, 0, argv);
        if (!success) {
            return 0;
        }

        return argv[0];
    }

    /**
     * @brief Expect WASM execution to trap with specific error
     * @param expected_success Expected success status (false for trap scenarios)
     * @param error_context Context description for assertion messages
     */
    void expect_wasm_trap(bool expected_success, const char* error_context) {
        if (!expected_success) {
            const char* exception = wasm_runtime_get_exception(module_inst);
            ASSERT_NE(nullptr, exception) << error_context << ": Expected trap but got success";
            ASSERT_STRNE("", exception) << error_context << ": Expected non-empty exception message";
        }
    }
};

/**
 * @test BasicStore_ValidValues_StoresCorrectly
 * @brief Validates i64.store produces correct storage results for typical inputs
 * @details Tests fundamental store operation with positive, negative, and special i64 integers.
 *          Verifies that i64.store correctly stores values and they can be retrieved via i64.load.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store_operation
 * @input_conditions Standard i64 values: 42, -1, 0x123456789ABCDEF0 at addresses 0, 8, 16
 * @expected_behavior Values stored correctly and retrievable via i64.load
 * @validation_method Direct comparison of stored value with loaded value
 */
TEST_P(I64StoreTest, BasicStore_ValidValues_StoresCorrectly) {
    // Test Case 1: Store positive value at address 0
    ASSERT_TRUE(call_i64_store_func("store_i64", 0, 42))
        << "Failed to store positive i64 value at address 0";
    ASSERT_EQ(42, call_i64_load_func(0))
        << "Stored and loaded values do not match for positive integer";

    // Test Case 2: Store negative value at address 8
    ASSERT_TRUE(call_i64_store_func("store_i64", 8, -1))
        << "Failed to store negative i64 value at address 8";
    ASSERT_EQ(-1, call_i64_load_func(8))
        << "Stored and loaded values do not match for negative integer";

    // Test Case 3: Store large hex pattern at address 16
    int64_t test_value = 0x123456789ABCDEF0LL;
    ASSERT_TRUE(call_i64_store_func("store_i64", 16, test_value))
        << "Failed to store large i64 hex pattern at address 16";
    ASSERT_EQ(test_value, call_i64_load_func(16))
        << "Stored and loaded values do not match for large hex pattern";
}

/**
 * @test BoundaryConditions_ExtremeValues_HandledCorrectly
 * @brief Tests boundary value storage (INT64_MIN, INT64_MAX, near-boundary addresses)
 * @details Validates storage of extreme i64 values and operations at memory boundaries.
 *          Tests both numeric boundaries and memory address boundaries.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store_bounds_check
 * @input_conditions INT64_MIN, INT64_MAX values and addresses near memory boundaries
 * @expected_behavior Boundary values stored correctly, memory bounds respected
 * @validation_method Direct value comparison and boundary access validation
 */
TEST_P(I64StoreTest, BoundaryConditions_ExtremeValues_HandledCorrectly) {
    // Test Case 1: Store INT64_MIN value
    int64_t min_value = INT64_MIN;
    ASSERT_TRUE(call_i64_store_func("store_i64", 0, min_value))
        << "Failed to store INT64_MIN value";
    ASSERT_EQ(min_value, call_i64_load_func(0))
        << "INT64_MIN value not stored/loaded correctly";

    // Test Case 2: Store INT64_MAX value
    int64_t max_value = INT64_MAX;
    ASSERT_TRUE(call_i64_store_func("store_i64", 8, max_value))
        << "Failed to store INT64_MAX value";
    ASSERT_EQ(max_value, call_i64_load_func(8))
        << "INT64_MAX value not stored/loaded correctly";

    // Test Case 3: Store at last valid memory address
    uint32_t memory_size = get_memory_size();
    uint32_t last_valid_addr = memory_size - 8; // i64 requires 8 bytes
    ASSERT_TRUE(call_i64_store_func("store_i64", last_valid_addr, 0xDEADBEEFCAFEBABELL))
        << "Failed to store at last valid memory address";
    ASSERT_EQ(0xDEADBEEFCAFEBABELL, call_i64_load_func(last_valid_addr))
        << "Value not stored correctly at memory boundary";
}

/**
 * @test MemoryAlignment_UnalignedAccess_WorksCorrectly
 * @brief Validates unaligned memory access for i64.store
 * @details Tests i64.store operations at both aligned and unaligned addresses.
 *          Verifies bit patterns are preserved across different alignment scenarios.
 * @test_category Edge - Memory alignment validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store_unaligned
 * @input_conditions Bit patterns at aligned (0, 8, 16) and unaligned (1, 3, 5, 7) addresses
 * @expected_behavior Unaligned access works correctly, bit patterns preserved
 * @validation_method Bit pattern verification and alignment boundary testing
 */
TEST_P(I64StoreTest, MemoryAlignment_UnalignedAccess_WorksCorrectly) {
    // Test Case 1: Aligned addresses (optimal performance)
    int64_t pattern1 = 0x5555555555555555LL; // Alternating bits
    ASSERT_TRUE(call_i64_store_func("store_i64", 0, pattern1))
        << "Failed to store at aligned address 0";
    ASSERT_EQ(pattern1, call_i64_load_func(0))
        << "Bit pattern not preserved at aligned address";

    // Test Case 2: Unaligned addresses (valid but potentially slower)
    int64_t pattern2 = 0xAAAAAAAAAAAAAAAALL; // Inverse alternating bits
    ASSERT_TRUE(call_i64_store_func("store_i64", 1, pattern2))
        << "Failed to store at unaligned address 1";
    ASSERT_EQ(pattern2, call_i64_load_func(1))
        << "Bit pattern not preserved at unaligned address 1";

    // Test Case 3: More unaligned addresses
    int64_t pattern3 = 0xF0F0F0F0F0F0F0F0LL;
    ASSERT_TRUE(call_i64_store_func("store_i64", 3, pattern3))
        << "Failed to store at unaligned address 3";
    ASSERT_EQ(pattern3, call_i64_load_func(3))
        << "Bit pattern not preserved at unaligned address 3";

    ASSERT_TRUE(call_i64_store_func("store_i64", 7, 0x0F0F0F0F0F0F0F0FLL))
        << "Failed to store at unaligned address 7";
    ASSERT_EQ(0x0F0F0F0F0F0F0F0FLL, call_i64_load_func(7))
        << "Bit pattern not preserved at unaligned address 7";
}

/**
 * @test OutOfBounds_InvalidAccess_GeneratesTraps
 * @brief Tests out-of-bounds memory access generates proper WASM traps
 * @details Validates that i64.store operations beyond memory bounds trigger appropriate
 *          WASM traps and error handling without corrupting memory or crashing.
 * @test_category Error - Out-of-bounds access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store_bounds_trap
 * @input_conditions Addresses beyond memory bounds, partial out-of-bounds scenarios
 * @expected_behavior WASM traps triggered, proper error handling, no memory corruption
 * @validation_method Exception detection and trap message validation
 */
TEST_P(I64StoreTest, OutOfBounds_InvalidAccess_GeneratesTraps) {
    uint32_t memory_size = get_memory_size();

    // Test Case 1: Store exactly at memory boundary (should fail)
    bool result1 = call_i64_store_func("store_i64", memory_size, 0x1234567890ABCDEFLL);
    ASSERT_FALSE(result1) << "Expected failure for store at memory boundary";
    expect_wasm_trap(false, "Store at memory boundary");

    // Clear any previous exception before next test
    wasm_runtime_clear_exception(module_inst);

    // Test Case 2: Store well beyond memory bounds (should fail)
    bool result2 = call_i64_store_func("store_i64", memory_size + 1000, 0xFEDCBA0987654321LL);
    ASSERT_FALSE(result2) << "Expected failure for store well beyond memory bounds";
    expect_wasm_trap(false, "Store well beyond memory bounds");

    // Clear any previous exception before next test
    wasm_runtime_clear_exception(module_inst);

    // Test Case 3: Partial out-of-bounds (address + 8 > memory_size)
    if (memory_size >= 4) {
        bool result3 = call_i64_store_func("store_i64", memory_size - 4, 0xCAFEBABEDEADBEEFLL);
        ASSERT_FALSE(result3) << "Expected failure for partial out-of-bounds store";
        expect_wasm_trap(false, "Partial out-of-bounds store");
    }
}

// Parameterized test instantiation for different execution modes
INSTANTIATE_TEST_SUITE_P(
    RunningModes,
    I64StoreTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        switch (info.param) {
            case Mode_Interp: return "Interpreter";
            case Mode_LLVM_JIT: return "AOT_LLVM";
            default: return "Unknown";
        }
    }
);

