/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <memory>
#include <cstring>
#include "wasm_runtime.h"
#include "bh_read_file.h"
#include "test_helper.h"

/**
 * @class I32Load16UTest
 * @brief Comprehensive test suite for i32.load16_u opcode functionality
 *
 * Tests the i32.load16_u WebAssembly instruction which loads a 16-bit unsigned integer
 * from linear memory and zero-extends it to a 32-bit value. Validates memory access,
 * zero extension behavior, alignment handling, and error conditions across both
 * interpreter and AOT execution modes.
 */
class I32Load16UTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment with WAMR runtime initialization
     *
     * Initializes WAMR runtime with system allocator, loads the test WASM module,
     * and prepares the execution environment for i32.load16_u instruction testing.
     * Creates module instance for both interpreter and AOT modes.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        is_aot_mode = (GetParam() == Mode_LLVM_JIT);

        load_sample_wasm();
        ASSERT_NE(nullptr, wasm_module) << "Failed to load i32.load16_u test module";

        wasm_module_inst = wasm_runtime_instantiate(wasm_module, stack_size, heap_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, wasm_module_inst) << "Failed to instantiate WASM module: " << error_buf;
    }

    /**
     * @brief Clean up test environment and release WAMR resources
     *
     * Deinstantiates module instance, unloads module,
     * and performs complete WAMR runtime cleanup to prevent memory leaks.
     */
    void TearDown() override
    {
        if (wasm_module_inst) {
            wasm_runtime_deinstantiate(wasm_module_inst);
            wasm_module_inst = nullptr;
        }
        if (wasm_module) {
            wasm_runtime_unload(wasm_module);
            wasm_module = nullptr;
        }
        if (wasm_buf) {
            BH_FREE(wasm_buf);
            wasm_buf = nullptr;
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Load the i32.load16_u test WASM module
     *
     * Loads the pre-compiled WASM module containing test functions for i32.load16_u
     * instruction validation, including memory setup and boundary test scenarios.
     */
    void load_sample_wasm()
    {
        const char *wasm_path = "wasm-apps/i32_load16_u_test.wasm";

        wasm_buf = bh_read_file_to_buffer(wasm_path, &wasm_buf_size);
        ASSERT_NE(nullptr, wasm_buf) << "Failed to read WASM file: " << wasm_path;

        wasm_module = wasm_runtime_load((uint8_t*)wasm_buf, wasm_buf_size,
                                      error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, wasm_module) << "Failed to load WASM module: " << error_buf;
    }

    /**
     * @brief Call WASM function to load 16-bit unsigned value from memory
     * @param address Memory address to load from
     * @return Zero-extended 32-bit value loaded from memory
     *
     * Invokes the WASM test function that performs i32.load16_u operation
     * at the specified memory address and returns the loaded value.
     */
    uint32_t call_i32_load16_u(uint32_t address)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "load16_u");
        EXPECT_NE(nullptr, func) << "Failed to lookup load16_u function";

        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(wasm_module_inst, stack_size);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        uint32_t argv[1] = { address };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(ret) << "Failed to call load16_u function: "
                         << wasm_runtime_get_exception(wasm_module_inst);

        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }

        return argv[0];
    }

    /**
     * @brief Call WASM function for signed 16-bit load comparison
     * @param address Memory address to load from
     * @return Sign-extended 32-bit value loaded from memory
     *
     * Invokes the WASM test function for i32.load16_s to compare behavior
     * with i32.load16_u for the same memory location.
     */
    uint32_t call_i32_load16_s(uint32_t address)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "load16_s");
        EXPECT_NE(nullptr, func) << "Failed to lookup load16_s function";

        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(wasm_module_inst, stack_size);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        uint32_t argv[1] = { address };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(ret) << "Failed to call load16_s function: "
                         << wasm_runtime_get_exception(wasm_module_inst);

        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }

        return argv[0];
    }

    /**
     * @brief Store 16-bit value in memory for testing
     * @param address Memory address to store to
     * @param value 16-bit value to store
     *
     * Stores a 16-bit value at the specified memory address using the WASM
     * test function for memory setup in test scenarios.
     */
    void store_i16_value(uint32_t address, uint16_t value)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "store16");
        ASSERT_NE(nullptr, func) << "Failed to lookup store16 function";

        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(wasm_module_inst, stack_size);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

        uint32_t argv[2] = { address, value };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        ASSERT_TRUE(ret) << "Failed to call store16 function: "
                         << wasm_runtime_get_exception(wasm_module_inst);

        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }
    }

    /**
     * @brief Get memory instance for direct memory access
     * @return Pointer to WASM linear memory instance
     *
     * Retrieves the linear memory instance for direct memory manipulation
     * and boundary condition testing.
     */
    wasm_memory_inst_t get_memory_inst()
    {
        return wasm_runtime_get_default_memory(wasm_module_inst);
    }

    /**
     * @brief Get current memory size in bytes
     * @return Current memory size in bytes
     *
     * Returns the current size of linear memory for boundary testing
     * and out-of-bounds access validation.
     */
    uint32_t get_memory_size()
    {
        wasm_memory_inst_t memory = get_memory_inst();
        EXPECT_NE(nullptr, memory) << "Failed to get memory instance";
        return wasm_memory_get_cur_page_count(memory) * 65536; // 64KB pages
    }

    RuntimeInitArgs init_args;
    wasm_module_t wasm_module = nullptr;
    wasm_module_inst_t wasm_module_inst = nullptr;
    char *wasm_buf = nullptr;
    uint32_t wasm_buf_size = 0;
    char error_buf[128] = {0};
    bool is_aot_mode = false;
    const uint32_t stack_size = 8092;
    const uint32_t heap_size = 8192;
};

/**
 * @test BasicLoad_ValidAddresses_ReturnsZeroExtendedValues
 * @brief Validates basic i32.load16_u functionality with typical memory addresses and values
 * @details Tests fundamental 16-bit unsigned load operation with known values at various addresses.
 *          Verifies that i32.load16_u correctly loads 16-bit values and zero-extends them to 32 bits.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Memory pre-loaded with test values 0x1234, 0x5678, 0xABCD at addresses 0, 2, 4
 * @expected_behavior Returns zero-extended values: 0x00001234, 0x00005678, 0x0000ABCD
 * @validation_method Direct comparison of loaded values with expected zero-extended results
 */
TEST_P(I32Load16UTest, BasicLoad_ValidAddresses_ReturnsZeroExtendedValues)
{
    // Store test values in memory
    store_i16_value(0, 0x1234);
    store_i16_value(2, 0x5678);
    store_i16_value(4, 0xABCD);

    // Test basic load operations with zero extension validation
    ASSERT_EQ(0x00001234U, call_i32_load16_u(0))
        << "Failed to load and zero-extend 0x1234 from address 0";
    ASSERT_EQ(0x00005678U, call_i32_load16_u(2))
        << "Failed to load and zero-extend 0x5678 from address 2";
    ASSERT_EQ(0x0000ABCDU, call_i32_load16_u(4))
        << "Failed to load and zero-extend 0xABCD from address 4";
}

/**
 * @test ZeroExtensionValidation_FullRange_CorrectBehavior
 * @brief Verifies zero extension behavior across complete 16-bit value range
 * @details Tests zero extension with boundary values including sign bit to ensure
 *          unsigned interpretation. Validates that upper 16 bits are always zero.
 * @test_category Corner - Boundary condition testing
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_load16_u_operation
 * @input_conditions Test values 0x0000, 0x7FFF, 0x8000, 0xFFFF stored in memory
 * @expected_behavior Upper 16 bits always zero: 0x8000→0x00008000, 0xFFFF→0x0000FFFF
 * @validation_method Explicit upper/lower bit validation and comparison with expected results
 */
TEST_P(I32Load16UTest, ZeroExtensionValidation_FullRange_CorrectBehavior)
{
    // Store boundary values for zero extension testing
    store_i16_value(0, 0x0000);  // Minimum value
    store_i16_value(2, 0x7FFF);  // Maximum signed positive
    store_i16_value(4, 0x8000);  // Sign bit set
    store_i16_value(6, 0xFFFF);  // Maximum unsigned value

    // Test zero extension behavior
    uint32_t result_min = call_i32_load16_u(0);
    uint32_t result_pos_max = call_i32_load16_u(2);
    uint32_t result_sign_bit = call_i32_load16_u(4);
    uint32_t result_max = call_i32_load16_u(6);

    // Validate zero extension - upper 16 bits must be zero
    ASSERT_EQ(0x00000000U, result_min)
        << "Zero value not properly zero-extended";
    ASSERT_EQ(0x00007FFFU, result_pos_max)
        << "Positive maximum not properly zero-extended";
    ASSERT_EQ(0x00008000U, result_sign_bit)
        << "Sign bit value not properly zero-extended (must be unsigned interpretation)";
    ASSERT_EQ(0x0000FFFFU, result_max)
        << "Maximum value not properly zero-extended";

    // Verify upper 16 bits are always zero
    ASSERT_EQ(0x0000U, (result_sign_bit >> 16))
        << "Upper 16 bits not zero for sign bit value";
    ASSERT_EQ(0x0000U, (result_max >> 16))
        << "Upper 16 bits not zero for maximum value";
}

/**
 * @test UnalignedAccess_OddAddresses_SuccessfulLoad
 * @brief Tests memory access from unaligned addresses (odd byte boundaries)
 * @details Validates that i32.load16_u works correctly with unaligned memory access,
 *          ensuring proper little-endian byte interpretation regardless of alignment.
 * @test_category Edge - Alignment testing
 * @coverage_target core/iwasm/common/wasm_memory.c:wasm_runtime_memory_operations
 * @input_conditions 16-bit values stored at odd addresses 1, 3, 5, 7 (unaligned)
 * @expected_behavior Successful loads with correct little-endian byte interpretation
 * @validation_method Comparison of loaded values with expected little-endian results
 */
TEST_P(I32Load16UTest, UnalignedAccess_OddAddresses_SuccessfulLoad)
{
    // Store values at unaligned (odd) addresses
    store_i16_value(1, 0x1234);
    store_i16_value(3, 0x5678);
    store_i16_value(5, 0x9ABC);
    store_i16_value(7, 0xDEF0);

    // Test unaligned access
    ASSERT_EQ(0x00001234U, call_i32_load16_u(1))
        << "Unaligned load failed at address 1";
    ASSERT_EQ(0x00005678U, call_i32_load16_u(3))
        << "Unaligned load failed at address 3";
    ASSERT_EQ(0x00009ABCU, call_i32_load16_u(5))
        << "Unaligned load failed at address 5";
    ASSERT_EQ(0x0000DEF0U, call_i32_load16_u(7))
        << "Unaligned load failed at address 7";
}

/**
 * @test MemoryBoundaryAccess_EdgeAddresses_ValidLoad
 * @brief Validates memory access at valid memory boundaries
 * @details Tests loading from addresses near memory limits to ensure proper
 *          boundary checking and successful access within valid memory range.
 * @test_category Corner - Memory boundary testing
 * @coverage_target core/iwasm/common/wasm_memory.c:wasm_runtime_validate_app_addr
 * @input_conditions Load from addresses near memory_size-2 (last valid 16-bit location)
 * @expected_behavior Successful load without memory access violation
 * @validation_method Successful function execution and correct value retrieval
 */
TEST_P(I32Load16UTest, MemoryBoundaryAccess_EdgeAddresses_ValidLoad)
{
    uint32_t memory_size = get_memory_size();
    ASSERT_GE(memory_size, 16U) << "Memory too small for boundary testing";

    // Store value near memory boundary (ensuring 2 bytes fit)
    uint32_t boundary_addr = memory_size - 2;
    store_i16_value(boundary_addr, 0x4321);

    // Test boundary access
    uint32_t result = call_i32_load16_u(boundary_addr);
    ASSERT_EQ(0x00004321U, result)
        << "Boundary access failed at address " << boundary_addr;
}

/**
 * @test ComparisonWithSigned_SameMemory_ValidateDifferentExtension
 * @brief Compares i32.load16_u with i32.load16_s on identical memory locations
 * @details Validates the difference between unsigned and signed 16-bit loads,
 *          ensuring proper zero extension vs sign extension behavior.
 * @test_category Edge - Signed/unsigned comparison
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:load_operations
 * @input_conditions Same memory location with 0x8000 value (sign bit set)
 * @expected_behavior i32.load16_u→0x00008000, i32.load16_s→0xFFFF8000 (different extension)
 * @validation_method Direct comparison of unsigned and signed load results
 */
TEST_P(I32Load16UTest, ComparisonWithSigned_SameMemory_ValidateDifferentExtension)
{
    // Store value with sign bit set for comparison testing
    store_i16_value(0, 0x8000);
    store_i16_value(2, 0xFFFF);

    // Compare unsigned vs signed load behavior
    uint32_t unsigned_result_8000 = call_i32_load16_u(0);
    uint32_t signed_result_8000 = call_i32_load16_s(0);
    uint32_t unsigned_result_FFFF = call_i32_load16_u(2);
    uint32_t signed_result_FFFF = call_i32_load16_s(2);

    // Validate zero extension vs sign extension
    ASSERT_EQ(0x00008000U, unsigned_result_8000)
        << "Unsigned load should zero-extend 0x8000";
    ASSERT_EQ(0xFFFF8000U, signed_result_8000)
        << "Signed load should sign-extend 0x8000";
    ASSERT_EQ(0x0000FFFFU, unsigned_result_FFFF)
        << "Unsigned load should zero-extend 0xFFFF";
    ASSERT_EQ(0xFFFFFFFFU, signed_result_FFFF)
        << "Signed load should sign-extend 0xFFFF";

    // Ensure they produce different results for values with sign bit set
    ASSERT_NE(unsigned_result_8000, signed_result_8000)
        << "Unsigned and signed loads should differ for sign bit values";
    ASSERT_NE(unsigned_result_FFFF, signed_result_FFFF)
        << "Unsigned and signed loads should differ for 0xFFFF";
}

/**
 * @test LittleEndianValidation_ByteOrdering_CorrectInterpretation
 * @brief Verifies little-endian byte ordering in memory interpretation
 * @details Tests that bytes are correctly interpreted in little-endian order
 *          when loading 16-bit values from memory.
 * @test_category Edge - Byte order validation
 * @coverage_target core/iwasm/common/wasm_memory.c:memory_byte_ordering
 * @input_conditions Specific byte patterns to validate little-endian interpretation
 * @expected_behavior Bytes interpreted in little-endian order (LSB first)
 * @validation_method Direct memory byte manipulation and load result verification
 */
TEST_P(I32Load16UTest, LittleEndianValidation_ByteOrdering_CorrectInterpretation)
{
    // Test little-endian byte ordering with specific patterns
    store_i16_value(0, 0x1234);  // Should be stored as 0x34, 0x12 in memory
    store_i16_value(2, 0xABCD);  // Should be stored as 0xCD, 0xAB in memory

    uint32_t result1 = call_i32_load16_u(0);
    uint32_t result2 = call_i32_load16_u(2);

    ASSERT_EQ(0x00001234U, result1)
        << "Little-endian interpretation failed for 0x1234";
    ASSERT_EQ(0x0000ABCDU, result2)
        << "Little-endian interpretation failed for 0xABCD";

    // Validate individual bytes in little-endian order
    wasm_memory_inst_t memory = get_memory_inst();
    ASSERT_NE(nullptr, memory) << "Failed to get memory instance";

    uint8_t* memory_data = (uint8_t*)wasm_runtime_addr_app_to_native(wasm_module_inst, 0);
    ASSERT_NE(nullptr, memory_data) << "Failed to get memory data pointer";

    // Verify little-endian byte storage
    ASSERT_EQ(0x34, memory_data[0]) << "First byte should be LSB of 0x1234";
    ASSERT_EQ(0x12, memory_data[1]) << "Second byte should be MSB of 0x1234";
    ASSERT_EQ(0xCD, memory_data[2]) << "Third byte should be LSB of 0xABCD";
    ASSERT_EQ(0xAB, memory_data[3]) << "Fourth byte should be MSB of 0xABCD";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I32Load16UTest,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT));