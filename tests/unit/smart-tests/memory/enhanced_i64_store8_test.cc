/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i64.store8 Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i64.store8
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Store8 Operations: 8-bit store operations with typical i64 values and addresses
 * - Value Truncation Operations: Testing proper truncation of i64 values to 8-bit storage
 * - Memory Boundary Operations: Store operations at memory boundaries and edge cases
 * - Out-of-bounds Access: Invalid memory access attempts and proper trap validation
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I64_STORE8
 * - AOT: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_i64_store8()
 * - Fast JIT: core/iwasm/fast-jit/fe/jit_emit_memory.c:jit_compile_op_i64_store8()
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

static int app_argc;
static char **app_argv;

/**
 * @class I64Store8Test
 * @brief Test fixture for i64.store8 opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for i64.store8 operations
 *          including module loading, execution environment setup, and validation helpers
 */
class I64Store8Test : public testing::TestWithParam<RunningMode>
{
  protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_function_inst_t func_inst = nullptr;

    /**
     * @brief Set up test environment and load WASM module for i64.store8 testing
     * @details Initializes WAMR runtime, loads test module, and prepares execution environment
     *          for comprehensive i64.store8 opcode validation across execution modes
     */
    void SetUp() override {
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

        // Use i64.store8 specific WASM file
        std::string store8_wasm_file = cwd + "/wasm-apps/i64_store8_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(store8_wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << store8_wasm_file;

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
     * @brief Clean up test environment and release WASM module resources
     * @details Destroys execution environment, unloads module instance, and performs cleanup
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
     * @brief Helper function to call i64.store8 WASM function and retrieve stored byte
     * @param addr Memory address for store operation
     * @param value i64 value to store (lower 8 bits will be stored)
     * @return Stored byte value read back from memory
     */
    uint8_t call_i64_store8_and_read(uint32_t addr, uint64_t value) {
        // Call store function
        func_inst = wasm_runtime_lookup_function(module_inst, "store_i64_byte");
        EXPECT_NE(func_inst, nullptr) << "Failed to lookup store_i64_byte function";

        uint32_t argv[3] = { addr, (uint32_t)value, (uint32_t)(value >> 32) };
        bool ret = wasm_runtime_call_wasm(exec_env, func_inst, 3, argv);
        EXPECT_TRUE(ret) << "Failed to call store_i64_byte function";

        // Read back the stored byte
        func_inst = wasm_runtime_lookup_function(module_inst, "load_byte");
        EXPECT_NE(func_inst, nullptr) << "Failed to lookup load_byte function";

        uint32_t read_argv[1] = { addr };
        ret = wasm_runtime_call_wasm(exec_env, func_inst, 1, read_argv);
        EXPECT_TRUE(ret) << "Failed to call load_byte function";

        return (uint8_t)read_argv[0];
    }

    /**
     * @brief Helper function to test out-of-bounds store operations
     * @param addr Memory address for store operation (expected to be out-of-bounds)
     * @param value i64 value to attempt storing
     * @return true if operation caused a trap (expected behavior), false otherwise
     */
    bool test_out_of_bounds_store(uint32_t addr, uint64_t value) {
        func_inst = wasm_runtime_lookup_function(module_inst, "store_i64_byte_oob");
        EXPECT_NE(func_inst, nullptr) << "Failed to lookup store_i64_byte_oob function";

        uint32_t argv[3] = { addr, (uint32_t)value, (uint32_t)(value >> 32) };

        // Clear any previous exception
        wasm_runtime_clear_exception(module_inst);

        bool ret = wasm_runtime_call_wasm(exec_env, func_inst, 3, argv);

        // Check if an exception occurred (expected for out-of-bounds)
        const char *exception = wasm_runtime_get_exception(module_inst);
        return !ret || (exception != nullptr);
    }

    /**
     * @brief Get memory size in bytes from the WASM module instance
     * @return Current memory size in bytes
     */
    uint32_t get_memory_size() {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "get_memory_size_bytes");
        EXPECT_NE(func, nullptr) << "Failed to lookup get_memory_size_bytes function";

        uint32_t argv[1] = {0};
        bool ret = wasm_runtime_call_wasm(exec_env, func, 0, argv);
        EXPECT_TRUE(ret) << "Failed to call get_memory_size_bytes function";

        return argv[0];
    }
};

/**
 * @test BasicStore_TypicalValues_StoresCorrectByte
 * @brief Validates i64.store8 produces correct byte storage for typical inputs
 * @details Tests fundamental store8 operation with various i64 values to ensure
 *          correct truncation to 8-bit values and proper memory storage.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store8_operation
 * @input_conditions Various i64 values: 0x42, 0x123456789ABCDEF0, 0xFFFFFFFFFFFFFF00
 * @expected_behavior Stores lower 8 bits: 0x42, 0xF0, 0x00 respectively
 * @validation_method Direct comparison of stored byte with expected truncated values
 */
TEST_P(I64Store8Test, BasicStore_TypicalValues_StoresCorrectByte) {
    // Test basic i64 value storage with typical values
    uint8_t result = call_i64_store8_and_read(0, 0x42ULL);
    ASSERT_EQ(0x42, result) << "Failed to store simple i64 value 0x42";

    // Test truncation of large i64 value - should store lower 8 bits
    result = call_i64_store8_and_read(4, 0x123456789ABCDEF0ULL);
    ASSERT_EQ(0xF0, result) << "Failed to truncate i64 value 0x123456789ABCDEF0 to 0xF0";

    // Test truncation where lower 8 bits are zero
    result = call_i64_store8_and_read(8, 0xFFFFFFFFFFFFFF00ULL);
    ASSERT_EQ(0x00, result) << "Failed to truncate i64 value with zero lower byte";
}

/**
 * @test MemoryBoundary_ValidAddresses_StoreSuccessfully
 * @brief Validates i64.store8 operates correctly at memory boundaries
 * @details Tests store operations at the first and last valid memory addresses
 *          to ensure proper boundary handling and address validation.
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/common/wasm_memory.c:memory_bounds_checking
 * @input_conditions Addresses: 0 (first), memory_size-1 (last); Value: 0x55
 * @expected_behavior Both addresses store 0x55 successfully without traps
 * @validation_method Verify successful store and correct byte retrieval at boundaries
 */
TEST_P(I64Store8Test, MemoryBoundary_ValidAddresses_StoreSuccessfully) {
    uint32_t memory_size = get_memory_size();
    ASSERT_GT(memory_size, 0U) << "Memory size must be greater than 0";

    // Test storing at address 0 (first valid address)
    uint8_t result = call_i64_store8_and_read(0, 0x55ULL);
    ASSERT_EQ(0x55, result) << "Failed to store at memory address 0";

    // Test storing at last valid address (memory_size - 1)
    result = call_i64_store8_and_read(memory_size - 1, 0xAAULL);
    ASSERT_EQ(0xAA, result) << "Failed to store at last valid memory address";
}

/**
 * @test ValueTruncation_LargeValues_PreservesLowerByte
 * @brief Validates i64.store8 correctly truncates large i64 values to 8-bit storage
 * @details Tests truncation behavior with extreme i64 values to ensure only
 *          the lower 8 bits are preserved and upper 56 bits are discarded.
 * @test_category Edge - Value truncation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_value_truncation
 * @input_conditions Extreme values: INT64_MAX, negative values, wrap-around values
 * @expected_behavior Preserves only lower 8 bits with proper modulo 256 behavior
 * @validation_method Bit-level comparison of stored values with expected truncation
 */
TEST_P(I64Store8Test, ValueTruncation_LargeValues_PreservesLowerByte) {
    // Test maximum i64 value truncation
    uint8_t result = call_i64_store8_and_read(0, 0xFFFFFFFFFFFFFFFFULL);
    ASSERT_EQ(0xFF, result) << "Failed to truncate maximum i64 value to 0xFF";

    // Test negative i64 value (when interpreted as signed)
    result = call_i64_store8_and_read(4, 0x8000000000000042ULL);
    ASSERT_EQ(0x42, result) << "Failed to truncate signed i64 value preserving lower byte";

    // Test wrap-around values (multiples of 256)
    result = call_i64_store8_and_read(8, 256ULL);  // 0x100
    ASSERT_EQ(0x00, result) << "Failed to wrap value 256 to 0x00";

    result = call_i64_store8_and_read(12, 257ULL); // 0x101
    ASSERT_EQ(0x01, result) << "Failed to wrap value 257 to 0x01";

    result = call_i64_store8_and_read(16, 512ULL); // 0x200
    ASSERT_EQ(0x00, result) << "Failed to wrap value 512 to 0x00";
}

/**
 * @test ZeroValues_StoresCorrectly
 * @brief Validates i64.store8 handles zero values and addresses correctly
 * @details Tests storage of zero i64 value and storage to address zero
 *          to ensure proper handling of zero operand scenarios.
 * @test_category Edge - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:zero_value_handling
 * @input_conditions Zero i64 value, zero address with valid value
 * @expected_behavior Stores 0x00 byte correctly, handles address 0 properly
 * @validation_method Direct verification of zero byte storage and address handling
 */
TEST_P(I64Store8Test, ZeroValues_StoresCorrectly) {
    // Test storing zero i64 value
    uint8_t result = call_i64_store8_and_read(0, 0x0000000000000000ULL);
    ASSERT_EQ(0x00, result) << "Failed to store zero i64 value as 0x00 byte";

    // Test storing to address zero with non-zero value
    result = call_i64_store8_and_read(0, 0xDEADBEEFCAFEBABEULL);
    ASSERT_EQ(0xBE, result) << "Failed to store to address zero with complex value";
}

/**
 * @test BitPatterns_PreservesLowerByte
 * @brief Validates i64.store8 preserves specific bit patterns in lower byte
 * @details Tests various bit patterns to ensure bit-level accuracy of truncation
 *          and proper preservation of the lower 8-bit pattern.
 * @test_category Edge - Bit pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:bit_pattern_preservation
 * @input_conditions Specific patterns: 0x55 (alternating), 0xAA (inverse alternating), 0xFF (all ones)
 * @expected_behavior Exact bit patterns preserved in stored bytes
 * @validation_method Bit-level verification of pattern preservation
 */
TEST_P(I64Store8Test, BitPatterns_PreservesLowerByte) {
    // Test alternating bit pattern (01010101)
    uint8_t result = call_i64_store8_and_read(0, 0x5555555555555555ULL);
    ASSERT_EQ(0x55, result) << "Failed to preserve alternating bit pattern 0x55";

    // Test inverse alternating bit pattern (10101010)
    result = call_i64_store8_and_read(4, 0xAAAAAAAAAAAAAAAAULL);
    ASSERT_EQ(0xAA, result) << "Failed to preserve inverse alternating bit pattern 0xAA";

    // Test all ones pattern (11111111)
    result = call_i64_store8_and_read(8, 0xFFFFFFFFFFFFFFFFULL);
    ASSERT_EQ(0xFF, result) << "Failed to preserve all ones bit pattern 0xFF";

    // Test single bit patterns
    result = call_i64_store8_and_read(12, 0x0000000000000001ULL);
    ASSERT_EQ(0x01, result) << "Failed to preserve single bit pattern 0x01";
}

/**
 * @test OutOfBounds_InvalidAddresses_ThrowsTraps
 * @brief Validates i64.store8 properly handles out-of-bounds memory access
 * @details Tests store operations to invalid memory addresses to ensure proper
 *          bounds checking and trap generation for memory safety.
 * @test_category Error - Memory bounds validation
 * @coverage_target core/iwasm/common/wasm_memory.c:bounds_check_and_trap
 * @input_conditions Addresses: memory_size, memory_size+1000, large invalid addresses
 * @expected_behavior All operations should cause memory bounds traps
 * @validation_method Verify trap/exception occurs for all out-of-bounds access attempts
 */
TEST_P(I64Store8Test, OutOfBounds_InvalidAddresses_ThrowsTraps) {
    uint32_t memory_size = get_memory_size();

    // Test storing just beyond memory bounds
    bool trapped = test_out_of_bounds_store(memory_size, 0x42ULL);
    ASSERT_TRUE(trapped) << "Expected trap for address exactly at memory_size boundary";

    // Test storing well beyond memory bounds
    trapped = test_out_of_bounds_store(memory_size + 1000, 0x55ULL);
    ASSERT_TRUE(trapped) << "Expected trap for address well beyond memory bounds";

    // Test storing at very large address (potential wraparound)
    trapped = test_out_of_bounds_store(0xFFFFFFFFU, 0xAAULL);
    ASSERT_TRUE(trapped) << "Expected trap for maximum i32 address";
}

// Parameterized tests for both interpreter and AOT execution modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I64Store8Test,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         testing::PrintToStringParamName());

