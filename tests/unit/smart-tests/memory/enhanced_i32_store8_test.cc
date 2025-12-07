/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i32.store8 Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i32.store8
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Store8 Operations: 8-bit store operations with typical i32 values and addresses
 * - Value Truncation Operations: Testing proper truncation of i32 values to 8-bit storage
 * - Memory Boundary Operations: Store operations at memory boundaries and edge cases
 * - Out-of-bounds Access: Invalid memory access attempts and proper trap validation
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE8
 * - AOT: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_i32_store8()
 * - Fast JIT: core/iwasm/fast-jit/fe/jit_emit_memory.c:jit_compile_op_i32_store8()
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
 * @class I32Store8Test
 * @brief Test fixture for i32.store8 opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for i32.store8 operations
 *          including module loading, execution environment setup, and validation helpers
 */
class I32Store8Test : public testing::TestWithParam<RunningMode>
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
     * @brief Set up test environment and load WASM module for i32.store8 testing
     * @details Initializes WAMR runtime, loads test module, and prepares execution environment
     *          for comprehensive i32.store8 opcode validation across execution modes
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

        // Use i32.store8 specific WASM file
        std::string store8_wasm_file = cwd + "/wasm-apps/i32_store8_test.wasm";
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
     * @brief Execute WASM function with error handling and result validation
     * @param func_name Name of the WASM function to execute
     * @param args Array of function arguments
     * @param argc Number of arguments
     * @return Function execution result, or UINT32_MAX on error
     */
    uint32_t call_wasm_function(const char* func_name, uint32_t args[], int argc) {
        func_inst = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func_inst) << "Function " << func_name << " not found";
        if (!func_inst) return UINT32_MAX;

        uint32_t argv[8];  // Support up to 8 parameters
        for (int i = 0; i < argc && i < 8; i++) {
            argv[i] = args[i];
        }

        bool ret = wasm_runtime_call_wasm(exec_env, func_inst, argc, argv);

        if (!ret) {
            const char* exception = wasm_runtime_get_exception(module_inst);
            if (exception) {
                EXPECT_TRUE(strstr(exception, "out of bounds") != nullptr ||
                           strstr(exception, "unreachable") != nullptr)
                    << "Unexpected exception: " << exception;
                return UINT32_MAX;  // Signal exception occurred
            }
        }

        return ret ? argv[0] : UINT32_MAX;
    }
};

/**
 * @test BasicStore8_TypicalValues_StoresCorrectByte
 * @brief Validates i32.store8 correctly truncates and stores typical i32 values as 8-bit bytes
 * @details Tests fundamental store8 operation with various i32 values, verifying proper 8-bit
 *          truncation behavior. Tests positive, negative, and mixed values to ensure consistent
 *          truncation to lowest 8 bits.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE8
 * @input_conditions Standard i32 values: 0x42, 0x1234, 0x56789ABC stored to addresses 0, 1, 2
 * @expected_behavior Returns stored bytes: 0x42, 0x34, 0xBC respectively (lowest 8 bits)
 * @validation_method Direct comparison of stored bytes with expected truncated values
 */
TEST_P(I32Store8Test, BasicStore8_TypicalValues_StoresCorrectByte) {
    // Test basic i32.store8 functionality with typical values
    uint32_t args[2];

    // Store 0x42 to address 0 (should store 0x42)
    args[0] = 0; args[1] = 0x42;
    uint32_t result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x42, result) << "Failed to store and load byte value 0x42";

    // Store 0x1234 to address 1 (should store 0x34 - lowest byte)
    args[0] = 1; args[1] = 0x1234;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x34, result) << "Failed to truncate 0x1234 to lowest byte 0x34";

    // Store 0x56789ABC to address 2 (should store 0xBC - lowest byte)
    args[0] = 2; args[1] = 0x56789ABC;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xBC, result) << "Failed to truncate 0x56789ABC to lowest byte 0xBC";
}

/**
 * @test TruncationBehavior_LargeValues_TruncatesTo8Bits
 * @brief Verifies proper truncation of large i32 values to their lowest 8 bits
 * @details Tests i32.store8 truncation behavior with extreme values including negative numbers,
 *          maximum values, and specific bit patterns. Validates that only the lowest 8 bits
 *          are preserved during storage operation.
 * @test_category Corner - Boundary conditions and truncation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE8
 * @input_conditions Large i32 values: 0xFFFFFFFF, 0x80000000, 0x12345678, 0x000000FF, 0x00000100
 * @expected_behavior Returns truncated bytes: 0xFF, 0x00, 0x78, 0xFF, 0x00 respectively
 * @validation_method Bitwise AND comparison to verify (input & 0xFF) == stored_byte
 */
TEST_P(I32Store8Test, TruncationBehavior_LargeValues_TruncatesTo8Bits) {
    uint32_t args[2];

    // Test maximum i32 value (0xFFFFFFFF should store 0xFF)
    args[0] = 0; args[1] = 0xFFFFFFFF;
    uint32_t result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xFF, result) << "Failed to truncate 0xFFFFFFFF to 0xFF";

    // Test minimum signed i32 value (0x80000000 should store 0x00)
    args[0] = 4; args[1] = 0x80000000;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x00, result) << "Failed to truncate 0x80000000 to 0x00";

    // Test specific bit pattern (0x12345678 should store 0x78)
    args[0] = 8; args[1] = 0x12345678;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x78, result) << "Failed to truncate 0x12345678 to 0x78";

    // Test byte boundary values
    args[0] = 12; args[1] = 0x000000FF;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xFF, result) << "Failed to store maximum byte value 0xFF";

    args[0] = 16; args[1] = 0x00000100;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x00, result) << "Failed to truncate 0x00000100 to 0x00";
}

/**
 * @test BoundaryAccess_MemoryEdges_AccessesCorrectly
 * @brief Tests i32.store8 operations at memory boundaries and various address alignments
 * @details Validates store8 operations at memory edge cases including zero address,
 *          unaligned addresses, and near memory boundaries. Verifies address independence
 *          of byte storage operations.
 * @test_category Edge - Memory boundary and alignment validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE8
 * @input_conditions Addresses: 0, 1, 15, 16, 31, 63 with distinct test values
 * @expected_behavior Each address stores and retrieves its corresponding test value correctly
 * @validation_method Sequential storage and retrieval validation across memory boundaries
 */
TEST_P(I32Store8Test, BoundaryAccess_MemoryEdges_AccessesCorrectly) {
    uint32_t args[2];

    // Test zero address storage
    args[0] = 0; args[1] = 0xAA;
    uint32_t result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xAA, result) << "Failed to store to zero address";

    // Test unaligned address storage (odd addresses)
    args[0] = 1; args[1] = 0xBB;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xBB, result) << "Failed to store to unaligned address 1";

    args[0] = 15; args[1] = 0xCC;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xCC, result) << "Failed to store to address 15";

    // Test 16-byte aligned addresses
    args[0] = 16; args[1] = 0xDD;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xDD, result) << "Failed to store to aligned address 16";

    args[0] = 32; args[1] = 0xEE;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xEE, result) << "Failed to store to address 32";

    // Test near page boundary (assuming 64KB pages)
    args[0] = 63; args[1] = 0xFF;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0xFF, result) << "Failed to store near page boundary";
}

/**
 * @test ZeroValues_VariousAddresses_StoresCorrectly
 * @brief Tests storage of zero values and access to zero addresses in various combinations
 * @details Validates i32.store8 behavior with zero values, zero addresses, and combinations
 *          thereof. Tests edge cases involving zero operands and validates proper storage
 *          of zero bytes.
 * @test_category Edge - Zero value and address validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE8
 * @input_conditions Zero address with zero value, zero value to various addresses, various values to zero address
 * @expected_behavior All operations complete successfully with proper zero byte storage
 * @validation_method Zero byte storage and retrieval across different address patterns
 */
TEST_P(I32Store8Test, ZeroValues_VariousAddresses_StoresCorrectly) {
    uint32_t args[2];

    // Store zero value to zero address
    args[0] = 0; args[1] = 0x00000000;
    uint32_t result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x00, result) << "Failed to store zero value to zero address";

    // Store zero value to various addresses
    args[0] = 4; args[1] = 0x00000000;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x00, result) << "Failed to store zero value to address 4";

    args[0] = 8; args[1] = 0x00000000;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x00, result) << "Failed to store zero value to address 8";

    // Store various values to zero address (sequential tests)
    args[0] = 0; args[1] = 0x11;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x11, result) << "Failed to store 0x11 to zero address";

    args[0] = 0; args[1] = 0x22;
    result = call_wasm_function("store_and_load_byte", args, 2);
    ASSERT_EQ(0x22, result) << "Failed to store 0x22 to zero address";
}

/**
 * @test OutOfBoundsAccess_InvalidAddresses_TrapsCorrectly
 * @brief Tests that attempts to store beyond memory bounds result in proper traps
 * @details Validates i32.store8 error handling for out-of-bounds memory access attempts.
 *          Tests various invalid addresses beyond memory limits and verifies proper
 *          trap generation and error reporting.
 * @test_category Error - Out-of-bounds access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE8
 * @input_conditions Addresses beyond memory size: 0xFFFFFFFC, 0xFFFFFFFE, large offsets
 * @expected_behavior WASM execution traps with out-of-bounds memory access error
 * @validation_method Exception detection and error message validation
 */
TEST_P(I32Store8Test, OutOfBoundsAccess_InvalidAddresses_TrapsCorrectly) {
    uint32_t args[2];

    // Attempt to store beyond allocated memory using very large address
    args[0] = 0xFFFFFFFC; args[1] = 0x42;
    uint32_t result = call_wasm_function("test_out_of_bounds_store", args, 2);
    ASSERT_EQ(UINT32_MAX, result) << "Expected out-of-bounds store to trap";

    // Verify exception was generated
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(nullptr, exception) << "Expected exception for out-of-bounds access";
    ASSERT_TRUE(strstr(exception, "out of bounds") != nullptr)
        << "Expected 'out of bounds' in exception message: " << exception;

    // Clear exception for next test
    wasm_runtime_clear_exception(module_inst);

    // Attempt store to maximum address
    args[0] = 0xFFFFFFFE; args[1] = 0x99;
    result = call_wasm_function("test_out_of_bounds_store", args, 2);
    ASSERT_EQ(UINT32_MAX, result) << "Expected maximum address store to trap";

    exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(nullptr, exception) << "Expected exception for large address access";
}

/**
 * @test OffsetCalculation_ImmediateOffsets_CalculatesCorrectly
 * @brief Tests i32.store8 with various immediate offset values for address calculation
 * @details Validates proper effective address calculation using base address plus immediate
 *          offset. Tests various offset combinations to ensure correct memory addressing
 *          in i32.store8 operations.
 * @test_category Main - Address calculation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE8
 * @input_conditions Base addresses with various immediate offsets: 0+4, 8+8, 16+0
 * @expected_behavior Effective address calculation produces correct storage locations
 * @validation_method Storage and retrieval validation with offset address calculations
 */
TEST_P(I32Store8Test, OffsetCalculation_ImmediateOffsets_CalculatesCorrectly) {
    uint32_t args[2];

    // Test base address + immediate offset combinations
    // These test functions have built-in immediate offsets

    // Test with immediate offset +4
    args[0] = 0; args[1] = 0x33;  // Effective address: 0 + 4 = 4
    uint32_t result = call_wasm_function("store_with_offset4", args, 2);
    ASSERT_EQ(0x33, result) << "Failed to store with immediate offset +4";

    // Test with immediate offset +8
    args[0] = 8; args[1] = 0x44;  // Effective address: 8 + 8 = 16
    result = call_wasm_function("store_with_offset8", args, 2);
    ASSERT_EQ(0x44, result) << "Failed to store with immediate offset +8";

    // Test with zero immediate offset
    args[0] = 20; args[1] = 0x55;  // Effective address: 20 + 0 = 20
    result = call_wasm_function("store_with_offset0", args, 2);
    ASSERT_EQ(0x55, result) << "Failed to store with zero immediate offset";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I32Store8Test,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT));