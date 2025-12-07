/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i64.store16 Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i64.store16
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Store16 Operations: 16-bit store operations with typical i64 values and addresses
 * - Value Truncation Operations: Testing proper truncation of i64 values to 16-bit storage
 * - Memory Boundary Operations: Store operations at memory boundaries and edge cases
 * - Out-of-bounds Access: Invalid memory access attempts and proper trap validation
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I64_STORE16
 * - AOT: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_i64_store16()
 * - Fast JIT: core/iwasm/fast-jit/fe/jit_emit_memory.c:jit_compile_op_i64_store16()
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

/**
 * @class I64Store16Test
 * @brief Test fixture for i64.store16 opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for i64.store16 operations
 *          including module loading, execution environment setup, and validation helpers
 */
class I64Store16Test : public testing::TestWithParam<RunningMode>
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
     * @brief Set up test environment and load WASM module for i64.store16 testing
     * @details Initializes WAMR runtime, loads test module, and prepares execution environment
     *          for comprehensive i64.store16 opcode validation across execution modes
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

        // Use i64.store16 specific WASM file
        std::string store16_wasm_file = cwd + "/wasm-apps/i64_store16_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(store16_wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << store16_wasm_file;

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
     * @brief Helper function to call i64.store16 WASM function and retrieve stored 16-bit value
     * @param addr Memory address for store operation
     * @param value i64 value to store (lower 16 bits will be stored)
     * @return Stored 16-bit value read back from memory
     */
    uint16_t call_i64_store16_and_read(uint32_t addr, uint64_t value) {
        // Call store function
        func_inst = wasm_runtime_lookup_function(module_inst, "store_i64_short");
        EXPECT_NE(func_inst, nullptr) << "Failed to lookup store_i64_short function";

        uint32_t argv[3] = { addr, (uint32_t)value, (uint32_t)(value >> 32) };
        bool ret = wasm_runtime_call_wasm(exec_env, func_inst, 3, argv);
        EXPECT_TRUE(ret) << "Failed to call store_i64_short function";

        // Read back the stored 16-bit value
        func_inst = wasm_runtime_lookup_function(module_inst, "load_short");
        EXPECT_NE(func_inst, nullptr) << "Failed to lookup load_short function";

        uint32_t read_argv[1] = { addr };
        ret = wasm_runtime_call_wasm(exec_env, func_inst, 1, read_argv);
        EXPECT_TRUE(ret) << "Failed to call load_short function";

        return (uint16_t)read_argv[0];
    }

    /**
     * @brief Helper function to test out-of-bounds store operations
     * @param addr Memory address for store operation (expected to be out-of-bounds)
     * @param value i64 value to attempt storing
     * @return true if operation caused a trap (expected behavior), false otherwise
     */
    bool test_out_of_bounds_store(uint32_t addr, uint64_t value) {
        func_inst = wasm_runtime_lookup_function(module_inst, "store_i64_short_oob");
        EXPECT_NE(func_inst, nullptr) << "Failed to lookup store_i64_short_oob function";

        uint32_t argv[3] = { addr, (uint32_t)value, (uint32_t)(value >> 32) };
        bool ret = wasm_runtime_call_wasm(exec_env, func_inst, 3, argv);

        // Check if execution failed due to trap (expected for out-of-bounds)
        return !ret;
    }

    /**
     * @brief Helper function to get memory size from WASM module
     * @return Current memory size in bytes
     */
    uint32_t get_memory_size() {
        func_inst = wasm_runtime_lookup_function(module_inst, "get_memory_size");
        EXPECT_NE(func_inst, nullptr) << "Failed to lookup get_memory_size function";

        uint32_t argv[1] = { 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func_inst, 0, argv);
        EXPECT_TRUE(ret) << "Failed to call get_memory_size function";

        return argv[0];
    }
};

/**
 * @test BasicStore_VariousValues_CorrectMemoryContent
 * @brief Validates i64.store16 produces correct 16-bit storage for typical inputs
 * @details Tests fundamental store16 operation with positive, negative, and mixed-value i64 integers.
 *          Verifies that i64.store16 correctly stores lower 16 bits for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store16_operation
 * @input_conditions Various i64 values: large positive, negative, small values
 * @expected_behavior Returns lower 16 bits of i64 value stored in memory
 * @validation_method Direct comparison of stored value with expected 16-bit truncation
 */
TEST_P(I64Store16Test, BasicStore_VariousValues_CorrectMemoryContent) {
    // Test with large positive value - should truncate to lower 16 bits
    uint64_t large_positive = 0x123456789ABCDEF0ULL;
    uint16_t expected_16bit = (uint16_t)(large_positive & 0xFFFF);
    uint16_t stored_value = call_i64_store16_and_read(0, large_positive);
    ASSERT_EQ(stored_value, expected_16bit)
        << "Large positive value truncation failed: expected 0x" << std::hex << expected_16bit
        << ", got 0x" << stored_value;

    // Test with negative value - verify signed behavior in truncation
    uint64_t negative_value = (uint64_t)(-1234567890LL);
    expected_16bit = (uint16_t)(negative_value & 0xFFFF);
    stored_value = call_i64_store16_and_read(8, negative_value);
    ASSERT_EQ(stored_value, expected_16bit)
        << "Negative value truncation failed: expected 0x" << std::hex << expected_16bit
        << ", got 0x" << stored_value;

    // Test with small value that fits within 16 bits
    uint64_t small_value = 0x1234ULL;
    stored_value = call_i64_store16_and_read(16, small_value);
    ASSERT_EQ(stored_value, 0x1234)
        << "Small value storage failed: expected 0x1234, got 0x" << std::hex << stored_value;
}

/**
 * @test BoundaryValues_ExtremeNumbers_ProperTruncation
 * @brief Validates i64.store16 handles boundary values with correct truncation behavior
 * @details Tests extreme numeric values including I64_MIN, I64_MAX, and 16-bit boundaries.
 *          Verifies proper truncation behavior for all boundary conditions.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store16_truncation
 * @input_conditions Boundary values: I64_MIN, I64_MAX, 0xFFFF, 0x8000
 * @expected_behavior Correct 16-bit truncation regardless of high-order bits
 * @validation_method Bitwise AND operation validation with 0xFFFF mask
 */
TEST_P(I64Store16Test, BoundaryValues_ExtremeNumbers_ProperTruncation) {
    // Test I64_MIN - should store as 0x0000 (truncated)
    uint64_t i64_min = 0x8000000000000000ULL; // I64_MIN as unsigned
    uint16_t stored_value = call_i64_store16_and_read(0, i64_min);
    ASSERT_EQ(stored_value, 0x0000)
        << "I64_MIN truncation failed: expected 0x0000, got 0x" << std::hex << stored_value;

    // Test I64_MAX - should store as 0xFFFF (truncated)
    uint64_t i64_max = 0x7FFFFFFFFFFFFFFFULL; // I64_MAX
    uint16_t expected = (uint16_t)(i64_max & 0xFFFF);
    stored_value = call_i64_store16_and_read(8, i64_max);
    ASSERT_EQ(stored_value, expected)
        << "I64_MAX truncation failed: expected 0x" << std::hex << expected
        << ", got 0x" << stored_value;

    // Test maximum 16-bit positive value
    uint64_t max_16bit = 0x000000000000FFFFULL;
    stored_value = call_i64_store16_and_read(16, max_16bit);
    ASSERT_EQ(stored_value, 0xFFFF)
        << "Max 16-bit value storage failed: expected 0xFFFF, got 0x" << std::hex << stored_value;

    // Test 16-bit sign boundary (0x8000)
    uint64_t sign_boundary = 0xFFFFFFFFFFFF8000ULL;
    expected = (uint16_t)(sign_boundary & 0xFFFF);
    stored_value = call_i64_store16_and_read(24, sign_boundary);
    ASSERT_EQ(stored_value, expected)
        << "Sign boundary truncation failed: expected 0x" << std::hex << expected
        << ", got 0x" << stored_value;
}

/**
 * @test MemoryBoundary_ValidAddresses_SuccessfulStore
 * @brief Validates i64.store16 operations succeed at valid memory boundary addresses
 * @details Tests store operations at memory boundaries including last valid addresses.
 *          Verifies boundary checking allows valid addresses while preventing overruns.
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/common/wasm_memory.c:wasm_check_app_addr_and_convert
 * @input_conditions Addresses at memory boundaries: start, near end, last valid
 * @expected_behavior Successful store operations at all valid addresses
 * @validation_method Exception-free execution and correct value storage
 */
TEST_P(I64Store16Test, MemoryBoundary_ValidAddresses_SuccessfulStore) {
    uint32_t memory_size = get_memory_size();
    uint64_t test_value = 0xABCDULL;

    // Test store at memory start (address 0)
    uint16_t stored_value = call_i64_store16_and_read(0, test_value);
    ASSERT_EQ(stored_value, 0xABCD)
        << "Store at memory start failed: expected 0xABCD, got 0x" << std::hex << stored_value;

    // Test store at last valid address (memory_size - 2, since store16 needs 2 bytes)
    ASSERT_GE(memory_size, 2u) << "Memory too small for boundary testing";
    uint32_t last_valid_addr = memory_size - 2;
    stored_value = call_i64_store16_and_read(last_valid_addr, test_value);
    ASSERT_EQ(stored_value, 0xABCD)
        << "Store at last valid address failed: expected 0xABCD, got 0x" << std::hex << stored_value;

    // Test store at second-to-last valid address
    if (memory_size >= 4) {
        uint32_t second_last_addr = memory_size - 4;
        stored_value = call_i64_store16_and_read(second_last_addr, test_value);
        ASSERT_EQ(stored_value, 0xABCD)
            << "Store at second-last address failed: expected 0xABCD, got 0x" << std::hex << stored_value;
    }
}

/**
 * @test ZeroOperands_VariousCombinations_CorrectBehavior
 * @brief Validates i64.store16 handles zero values and addresses correctly
 * @details Tests zero address, zero value, and combined zero scenarios.
 *          Verifies proper handling of zero operands in all combinations.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store16_zero_handling
 * @input_conditions Zero combinations: zero address, zero value, both zero
 * @expected_behavior Correct storage of zero values and operations at zero address
 * @validation_method Zero value verification and successful execution
 */
TEST_P(I64Store16Test, ZeroOperands_VariousCombinations_CorrectBehavior) {
    // Test zero value at various addresses
    uint16_t stored_value = call_i64_store16_and_read(0, 0ULL);
    ASSERT_EQ(stored_value, 0x0000)
        << "Zero value storage at address 0 failed: expected 0x0000, got 0x" << std::hex << stored_value;

    stored_value = call_i64_store16_and_read(8, 0ULL);
    ASSERT_EQ(stored_value, 0x0000)
        << "Zero value storage at address 8 failed: expected 0x0000, got 0x" << std::hex << stored_value;

    // Test non-zero value at zero address (already tested above, but explicit)
    uint64_t non_zero_value = 0x5555ULL;
    stored_value = call_i64_store16_and_read(0, non_zero_value);
    ASSERT_EQ(stored_value, 0x5555)
        << "Non-zero value at zero address failed: expected 0x5555, got 0x" << std::hex << stored_value;

    // Test extreme values with zero in different bit positions
    uint64_t alternating_pattern = 0xAAAAAAAAAAAA0000ULL;
    stored_value = call_i64_store16_and_read(16, alternating_pattern);
    ASSERT_EQ(stored_value, 0x0000)
        << "Alternating pattern with zero lower bits failed: expected 0x0000, got 0x" << std::hex << stored_value;
}

/**
 * @test ExtremeValues_BitPatterns_ProperTruncation
 * @brief Validates i64.store16 preserves correct bit patterns during truncation
 * @details Tests extreme values and specific bit patterns to verify truncation consistency.
 *          Validates mathematical property that stored_value == (input_value & 0xFFFF).
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store16_bit_operations
 * @input_conditions Extreme bit patterns: all set, alternating, specific combinations
 * @expected_behavior Bit-perfect truncation preserving lower 16 bits exactly
 * @validation_method Bitwise comparison with expected truncated values
 */
TEST_P(I64Store16Test, ExtremeValues_BitPatterns_ProperTruncation) {
    // Test all bits set
    uint64_t all_bits_set = 0xFFFFFFFFFFFFFFFFULL;
    uint16_t stored_value = call_i64_store16_and_read(0, all_bits_set);
    ASSERT_EQ(stored_value, 0xFFFF)
        << "All bits set truncation failed: expected 0xFFFF, got 0x" << std::hex << stored_value;

    // Test alternating bit pattern
    uint64_t alternating = 0xAAAAAAAAAAAAAAAAULL;
    uint16_t expected = (uint16_t)(alternating & 0xFFFF);
    stored_value = call_i64_store16_and_read(8, alternating);
    ASSERT_EQ(stored_value, expected)
        << "Alternating bit pattern failed: expected 0x" << std::hex << expected
        << ", got 0x" << stored_value;

    // Test inverted alternating pattern
    uint64_t inverted_alternating = 0x5555555555555555ULL;
    expected = (uint16_t)(inverted_alternating & 0xFFFF);
    stored_value = call_i64_store16_and_read(16, inverted_alternating);
    ASSERT_EQ(stored_value, expected)
        << "Inverted alternating pattern failed: expected 0x" << std::hex << expected
        << ", got 0x" << stored_value;

    // Test mathematical consistency: verify (value & 0xFFFF) == stored_value
    uint64_t test_values[] = {
        0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL, 0x0000000000001234ULL,
        0xFFFF000000000000ULL, 0x0000FFFF00000000ULL, 0x00000000FFFFFFFFULL
    };

    for (size_t i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++) {
        uint64_t value = test_values[i];
        expected = (uint16_t)(value & 0xFFFF);
        stored_value = call_i64_store16_and_read(24 + i * 8, value);
        ASSERT_EQ(stored_value, expected)
            << "Mathematical consistency failed for value 0x" << std::hex << value
            << ": expected 0x" << expected << ", got 0x" << stored_value;
    }
}

/**
 * @test OutOfBounds_InvalidAddresses_ProperTraps
 * @brief Validates i64.store16 generates proper traps for out-of-bounds memory access
 * @details Tests invalid memory addresses that should cause WAMR to generate traps.
 *          Verifies proper error handling and trap generation for memory violations.
 * @test_category Error - Out-of-bounds access validation
 * @coverage_target core/iwasm/common/wasm_memory.c:wasm_runtime_validate_app_addr
 * @input_conditions Invalid addresses: memory_size-1, memory_size, beyond bounds
 * @expected_behavior WAMR traps with out-of-bounds errors for invalid addresses
 * @validation_method Trap detection and proper error handling verification
 */
TEST_P(I64Store16Test, OutOfBounds_InvalidAddresses_ProperTraps) {
    uint32_t memory_size = get_memory_size();
    uint64_t test_value = 0x1234ULL;

    // Test store at memory_size - 1 (invalid: needs 2 bytes, only 1 available)
    bool trapped = test_out_of_bounds_store(memory_size - 1, test_value);
    ASSERT_TRUE(trapped)
        << "Expected trap for store at address " << (memory_size - 1)
        << " (needs 2 bytes, only 1 available)";

    // Test store at memory_size (completely out of bounds)
    trapped = test_out_of_bounds_store(memory_size, test_value);
    ASSERT_TRUE(trapped)
        << "Expected trap for store at address " << memory_size << " (completely out of bounds)";

    // Test store far beyond memory bounds
    trapped = test_out_of_bounds_store(memory_size + 1000, test_value);
    ASSERT_TRUE(trapped)
        << "Expected trap for store at address " << (memory_size + 1000) << " (far out of bounds)";

    // Test store with maximum uint32 address
    trapped = test_out_of_bounds_store(UINT32_MAX, test_value);
    ASSERT_TRUE(trapped)
        << "Expected trap for store at maximum uint32 address";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I64Store16Test,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT));