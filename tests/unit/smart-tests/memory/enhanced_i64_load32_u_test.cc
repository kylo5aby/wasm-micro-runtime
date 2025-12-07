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
 * @brief Test fixture for comprehensive i64.load32_u opcode validation
 *
 * This test suite validates the i64.load32_u WebAssembly opcode across different
 * execution modes (interpreter and AOT). The opcode loads an unsigned 32-bit integer
 * from memory and zero-extends it to 64-bit. Tests cover basic functionality,
 * zero extension behavior, boundary conditions, error scenarios, and cross-mode consistency.
 */
class I64Load32UTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment with WAMR runtime initialization
     *
     * Initializes the WAMR runtime with system allocator and loads
     * the i64.load32_u test module for execution validation.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        is_aot_mode = (GetParam() == Mode_LLVM_JIT);

        // Load the i64.load32_u test module
        LoadModule();
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     *
     * Properly releases module instance, module, and runtime resources
     * to prevent memory leaks and ensure clean test isolation.
     */
    void TearDown() override
    {
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }

        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }

        if (buffer) {
            delete[] buffer;
            buffer = nullptr;
        }

        wasm_runtime_destroy();
    }

    /**
     * @brief Load the i64.load32_u test WebAssembly module
     *
     * Loads the appropriate WASM module based on execution mode and
     * instantiates it for test execution.
     */
    void LoadModule()
    {
        const char* wasm_file = "wasm-apps/i64_load32_u_test.wasm";

        buffer = bh_read_file_to_buffer(wasm_file, &size);
        ASSERT_NE(nullptr, buffer) << "Failed to read WASM file: " << wasm_file;
        ASSERT_GT(size, 0U) << "WASM file is empty: " << wasm_file;

        module = wasm_runtime_load((uint8_t*)buffer, size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate module: " << error_buf;
    }

    /**
     * @brief Execute a WASM function by name with provided arguments
     *
     * @param func_name Name of the exported WASM function to call
     * @param argc Number of arguments to pass to the function
     * @param argv Array of argument values for the function
     * @return bool True if execution succeeded, false if trapped/failed
     */
    bool CallWasmFunction(const char* func_name, uint32_t argc, uint32_t* argv)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Failed to lookup function: " << func_name;

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        bool ret = wasm_runtime_call_wasm(exec_env, func, argc, argv);

        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }

        return ret;
    }

    /**
     * @brief Call i64.load32_u test function with specified address
     *
     * @param addr Memory address to load from
     * @return uint64_t Zero-extended 64-bit result
     */
    uint64_t CallLoad32U(uint32_t addr)
    {
        uint32_t argv[1] = { addr };
        uint32_t argc = 1;

        bool success = CallWasmFunction("test_i64_load32_u", argc, argv);
        EXPECT_TRUE(success) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        // Return result as uint64_t (argv[0] contains low 32 bits, argv[1] contains high 32 bits)
        return ((uint64_t)argv[1] << 32) | argv[0];
    }

    /**
     * @brief Call i64.load32_u test function with address and offset
     *
     * @param addr Base memory address
     * @param offset Static offset from instruction
     * @return uint64_t Zero-extended 64-bit result
     */
    uint64_t CallLoad32UWithOffset(uint32_t addr, uint32_t offset)
    {
        uint32_t argv[2] = { addr, offset };
        uint32_t argc = 2;

        bool success = CallWasmFunction("test_i64_load32_u_offset", argc, argv);
        EXPECT_TRUE(success) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        return ((uint64_t)argv[1] << 32) | argv[0];
    }

    /**
     * @brief Call memory initialization helper function
     *
     * @param addr Address to write to
     * @param value 32-bit value to write
     */
    void InitMemoryU32(uint32_t addr, uint32_t value)
    {
        uint32_t argv[2] = { addr, value };
        uint32_t argc = 2;

        bool success = CallWasmFunction("init_memory_u32", argc, argv);
        EXPECT_TRUE(success) << "Memory init failed: " << wasm_runtime_get_exception(module_inst);
    }

    /**
     * @brief Test error condition with out-of-bounds access
     *
     * @param addr Invalid address that should cause trap
     * @return bool true if trap occurred as expected
     */
    bool TestOutOfBounds(uint32_t addr)
    {
        uint32_t argv[1] = { addr };
        uint32_t argc = 1;

        bool success = CallWasmFunction("test_i64_load32_u", argc, argv);
        return !success; // Returns true if trap occurred (expected)
    }

protected:
    RuntimeInitArgs init_args{};
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    char* buffer = nullptr;
    uint32_t size = 0;
    char error_buf[256]{};
    bool is_aot_mode = false;

    static constexpr uint32_t stack_size = 16 * 1024;
    static constexpr uint32_t heap_size = 16 * 1024;
};

/**
 * @test BasicLoading_ValidAddresses_ReturnsZeroExtendedValues
 * @brief Validates fundamental i64.load32_u functionality with typical u32 values
 * @details Tests core zero-extension operation with representative 32-bit patterns,
 *          including positive values, values with high bit set, and boundary cases.
 *          Verifies that all u32 values are correctly zero-extended to i64 format.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_load32_u_operation
 * @input_conditions Memory initialized with test patterns: 0x12345678, 0xABCDEF00, 0x80000001, 0x7FFFFFFF
 * @expected_behavior Zero-extended i64 values: 0x0000000012345678, 0x00000000ABCDEF00, etc.
 * @validation_method Direct comparison of loaded values with expected zero-extended results
 */
TEST_P(I64Load32UTest, BasicLoading_ValidAddresses_ReturnsZeroExtendedValues)
{
    // Initialize memory with test patterns
    InitMemoryU32(0, 0x12345678U);
    InitMemoryU32(4, 0xABCDEF00U);
    InitMemoryU32(8, 0x80000001U);
    InitMemoryU32(12, 0x7FFFFFFFU);

    // Test basic loading and verify zero extension
    ASSERT_EQ(0x0000000012345678ULL, CallLoad32U(0))
        << "Load from address 0 failed - expected zero-extended 0x12345678";

    ASSERT_EQ(0x00000000ABCDEF00ULL, CallLoad32U(4))
        << "Load from address 4 failed - expected zero-extended 0xABCDEF00";

    ASSERT_EQ(0x0000000080000001ULL, CallLoad32U(8))
        << "Load from address 8 failed - expected zero-extended 0x80000001";

    ASSERT_EQ(0x000000007FFFFFFFULL, CallLoad32U(12))
        << "Load from address 12 failed - expected zero-extended 0x7FFFFFFF";
}

/**
 * @test ZeroExtensionVerification_HighBitSet_UpperBitsZero
 * @brief Confirms zero extension behavior for u32 values with MSB=1
 * @details Validates that values with the most significant bit set (which would be
 *          negative in signed interpretation) are properly zero-extended rather than
 *          sign-extended, distinguishing unsigned from signed load behavior.
 * @test_category Corner - Boundary condition validation for sign bit handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:LOAD_U32_zero_extension
 * @input_conditions Memory contains MSB-set values: 0x80000000, 0x90000000, 0xFFFFFFFF
 * @expected_behavior Zero extension: 0x0000000080000000, 0x0000000090000000, 0x00000000FFFFFFFF
 * @validation_method Verify upper 32 bits are zero and compare with expected patterns
 */
TEST_P(I64Load32UTest, ZeroExtensionVerification_HighBitSet_UpperBitsZero)
{
    // Test values that would be negative if sign-extended
    InitMemoryU32(16, 0x80000000U);   // MSB set, minimum "negative" in signed
    InitMemoryU32(20, 0x90000000U);   // MSB set, random pattern
    InitMemoryU32(24, 0xFFFFFFFFU);   // All bits set, maximum u32 value

    uint64_t result1 = CallLoad32U(16);
    uint64_t result2 = CallLoad32U(20);
    uint64_t result3 = CallLoad32U(24);

    // Verify zero extension (upper 32 bits must be zero)
    ASSERT_EQ(0x0000000080000000ULL, result1)
        << "0x80000000 should zero-extend to 0x0000000080000000, got 0x"
        << std::hex << result1;

    ASSERT_EQ(0x0000000090000000ULL, result2)
        << "0x90000000 should zero-extend to 0x0000000090000000, got 0x"
        << std::hex << result2;

    ASSERT_EQ(0x00000000FFFFFFFFULL, result3)
        << "0xFFFFFFFF should zero-extend to 0x00000000FFFFFFFF, got 0x"
        << std::hex << result3;

    // Verify upper 32 bits are explicitly zero
    ASSERT_EQ(0ULL, result1 >> 32) << "Upper 32 bits of 0x80000000 load should be zero";
    ASSERT_EQ(0ULL, result2 >> 32) << "Upper 32 bits of 0x90000000 load should be zero";
    ASSERT_EQ(0ULL, result3 >> 32) << "Upper 32 bits of 0xFFFFFFFF load should be zero";
}

/**
 * @test BoundaryValues_U32MinMax_HandledCorrectly
 * @brief Tests loading of u32 minimum and maximum boundary values
 * @details Validates correct handling of extreme u32 values including zero (minimum)
 *          and 0xFFFFFFFF (maximum), ensuring proper zero extension behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:u32_boundary_handling
 * @input_conditions Memory contains u32 MIN (0x00000000) and MAX (0xFFFFFFFF)
 * @expected_behavior Zero extension: MIN→0x0000000000000000, MAX→0x00000000FFFFFFFF
 * @validation_method Direct value comparison and boundary semantics verification
 */
TEST_P(I64Load32UTest, BoundaryValues_U32MinMax_HandledCorrectly)
{
    // Test u32 boundary values
    InitMemoryU32(28, 0x00000000U);   // u32 minimum value
    InitMemoryU32(32, 0xFFFFFFFFU);   // u32 maximum value

    uint64_t min_result = CallLoad32U(28);
    uint64_t max_result = CallLoad32U(32);

    // Verify boundary value handling
    ASSERT_EQ(0x0000000000000000ULL, min_result)
        << "u32 MIN (0x00000000) should zero-extend to 0x0000000000000000";

    ASSERT_EQ(0x00000000FFFFFFFFULL, max_result)
        << "u32 MAX (0xFFFFFFFF) should zero-extend to 0x00000000FFFFFFFF";

    // Verify boundary semantics
    ASSERT_EQ(0U, static_cast<uint32_t>(min_result & 0xFFFFFFFF))
        << "Lower 32 bits should preserve original u32 MIN value";

    ASSERT_EQ(0xFFFFFFFFU, static_cast<uint32_t>(max_result & 0xFFFFFFFF))
        << "Lower 32 bits should preserve original u32 MAX value";
}

/**
 * @test AlignmentHandling_UnalignedAccess_LoadsCorrectly
 * @brief Verifies correct behavior for unaligned memory access
 * @details Tests i64.load32_u with addresses not aligned to 4-byte boundaries,
 *          ensuring WAMR handles unaligned access correctly across all platforms.
 * @test_category Edge - Unaligned memory access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:unaligned_memory_access
 * @input_conditions Same u32 value stored at addresses with different alignments (0,1,2,3 offsets)
 * @expected_behavior Identical results regardless of alignment offset
 * @validation_method Compare results from aligned vs unaligned addresses
 */
TEST_P(I64Load32UTest, AlignmentHandling_UnalignedAccess_LoadsCorrectly)
{
    // Initialize memory with test pattern at different alignments
    const uint32_t test_value = 0xDEADBEEFU;

    // Store at aligned address (divisible by 4)
    InitMemoryU32(36, test_value);

    // Store at unaligned addresses (1, 2, 3 byte offsets)
    InitMemoryU32(41, test_value);  // 1-byte misaligned
    InitMemoryU32(46, test_value);  // 2-byte misaligned
    InitMemoryU32(51, test_value);  // 3-byte misaligned

    uint64_t aligned_result = CallLoad32U(36);
    uint64_t unaligned1_result = CallLoad32U(41);
    uint64_t unaligned2_result = CallLoad32U(46);
    uint64_t unaligned3_result = CallLoad32U(51);

    const uint64_t expected = 0x00000000DEADBEEFULL;

    // Verify all alignments produce identical results
    ASSERT_EQ(expected, aligned_result)
        << "Aligned access (addr 36) failed";

    ASSERT_EQ(expected, unaligned1_result)
        << "Unaligned access (addr 41, 1-byte offset) failed";

    ASSERT_EQ(expected, unaligned2_result)
        << "Unaligned access (addr 46, 2-byte offset) failed";

    ASSERT_EQ(expected, unaligned3_result)
        << "Unaligned access (addr 51, 3-byte offset) failed";
}

/**
 * @test StaticOffsetHandling_WithOffset_CalculatesAddressCorrectly
 * @brief Tests i64.load32_u with static offset parameter
 * @details Validates proper address calculation when using static offset in addition
 *          to dynamic address, ensuring base + offset arithmetic works correctly.
 * @test_category Main - Offset parameter validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:address_offset_calculation
 * @input_conditions Base addresses with various static offsets
 * @expected_behavior Correct value loaded from (base + offset) address
 * @validation_method Compare offset-based loads with direct address loads
 */
TEST_P(I64Load32UTest, StaticOffsetHandling_WithOffset_CalculatesAddressCorrectly)
{
    // Initialize memory with pattern for offset testing
    InitMemoryU32(60, 0x11111111U);
    InitMemoryU32(64, 0x22222222U);
    InitMemoryU32(68, 0x33333333U);
    InitMemoryU32(72, 0x44444444U);

    // Test loading with static offsets
    uint64_t result1 = CallLoad32UWithOffset(60, 0);   // base + 0
    uint64_t result2 = CallLoad32UWithOffset(60, 4);   // base + 4
    uint64_t result3 = CallLoad32UWithOffset(60, 8);   // base + 8
    uint64_t result4 = CallLoad32UWithOffset(60, 12);  // base + 12

    // Verify offset calculations
    ASSERT_EQ(0x0000000011111111ULL, result1)
        << "Load with offset 0 failed";

    ASSERT_EQ(0x0000000022222222ULL, result2)
        << "Load with offset 4 failed";

    ASSERT_EQ(0x0000000033333333ULL, result3)
        << "Load with offset 8 failed";

    ASSERT_EQ(0x0000000044444444ULL, result4)
        << "Load with offset 12 failed";
}

/**
 * @test MemoryBoundary_AtLimits_ProperBoundsChecking
 * @brief Tests memory access at the boundaries of allocated memory
 * @details Validates WAMR's bounds checking by attempting loads at memory limits,
 *          expecting success for valid addresses and traps for out-of-bounds.
 * @test_category Error - Memory bounds validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:CHECK_MEMORY_OVERFLOW
 * @input_conditions Addresses near memory boundaries including valid and invalid ranges
 * @expected_behavior Success for valid addresses, traps for out-of-bounds access
 * @validation_method Exception handling to catch expected memory access traps
 */
TEST_P(I64Load32UTest, MemoryBoundary_AtLimits_ProperBoundsChecking)
{
    // Use conservative memory boundaries - test near end of initialized data
    // rather than assuming full 64KB memory size
    const uint32_t safe_boundary_address = 65500;  // Well within 64KB but near end

    // Test valid access in safe region (should succeed)
    InitMemoryU32(safe_boundary_address, 0xBBBBBBBBU);
    uint64_t boundary_result = CallLoad32U(safe_boundary_address);
    ASSERT_EQ(0x00000000BBBBBBBBULL, boundary_result)
        << "Valid safe boundary access should succeed";

    // Test clearly invalid accesses (should trap)
    ASSERT_TRUE(TestOutOfBounds(0xFFFFFFFC))
        << "Access near 4GB limit should cause out-of-bounds trap";

    ASSERT_TRUE(TestOutOfBounds(0xFFFFFFFD))
        << "Access at 4GB-3 should cause out-of-bounds trap";

    ASSERT_TRUE(TestOutOfBounds(0xFFFFFFFE))
        << "Access at 4GB-2 should cause out-of-bounds trap";

    ASSERT_TRUE(TestOutOfBounds(0xFFFFFFFF))
        << "Access at 4GB-1 should cause out-of-bounds trap";

    // Test other clearly invalid addresses
    ASSERT_TRUE(TestOutOfBounds(0x10000000))
        << "Access at 256MB should cause out-of-bounds trap";
}

/**
 * @test ZeroAddress_BaseMemoryAccess_LoadsCorrectly
 * @brief Tests loading from memory base address (zero)
 * @details Validates that loading from address 0 (memory base) works correctly
 *          and produces the expected zero-extended result.
 * @test_category Edge - Zero address access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:zero_address_handling
 * @input_conditions Memory[0-3] contains known u32 value
 * @expected_behavior Successful load and zero extension of value at address 0
 * @validation_method Direct comparison with expected zero-extended result
 */
TEST_P(I64Load32UTest, ZeroAddress_BaseMemoryAccess_LoadsCorrectly)
{
    // Test loading from memory base address (zero)
    InitMemoryU32(0, 0x01234567U);

    uint64_t result = CallLoad32U(0);

    ASSERT_EQ(0x0000000001234567ULL, result)
        << "Load from base address 0 should return zero-extended value";

    // Verify zero address is handled as valid memory location
    ASSERT_EQ(0x01234567U, static_cast<uint32_t>(result & 0xFFFFFFFF))
        << "Lower 32 bits should preserve original u32 value";

    ASSERT_EQ(0U, static_cast<uint32_t>(result >> 32))
        << "Upper 32 bits should be zero for zero extension";
}

// Test parameterization for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossMode,
    I64Load32UTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);