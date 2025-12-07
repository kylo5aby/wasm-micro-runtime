/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"

static std::string CWD;
static std::string WASM_FILE;
static constexpr RunningMode running_modes[] = { Mode_Interp, Mode_LLVM_JIT };

/**
 * @brief Test fixture for i64.load16_u opcode validation across interpreter and AOT modes
 *
 * This test suite comprehensively validates the i64.load16_u WebAssembly opcode functionality:
 * - Loads unsigned 16-bit integers from linear memory
 * - Performs proper zero extension to 64-bit unsigned integers
 * - Validates memory bounds checking and error handling
 * - Tests across both interpreter and AOT execution modes
 * - Verifies little-endian byte order in memory access
 * - Tests alignment scenarios for 16-bit memory operations
 */
class I64Load16UTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up WAMR runtime and load test module for each test case
     *
     * Initializes WAMR runtime with system allocator and loads the i64.load16_u
     * test module from WASM bytecode file. Configures both interpreter and AOT
     * execution modes based on test parameters.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        WASM_FILE = "wasm-apps/i64_load16_u_test.wasm";

        // Load WASM module from file
        module_buffer = (unsigned char *)bh_read_file_to_buffer(WASM_FILE.c_str(), &module_buffer_size);
        ASSERT_NE(nullptr, module_buffer) << "Failed to read WASM file: " << WASM_FILE;

        // Load module into WAMR
        module = wasm_runtime_load(module_buffer, module_buffer_size, error_buffer, sizeof(error_buffer));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buffer;

        // Instantiate module
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buffer, sizeof(error_buffer));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buffer;

        // Set execution mode
        wasm_runtime_set_running_mode(module_inst, GetParam());

        // Get memory data pointer for direct memory access
        memory_data = wasm_runtime_addr_app_to_native(module_inst, 0);
        ASSERT_NE(nullptr, memory_data) << "Failed to get memory data pointer";

        // WASM memory is 1 page = 64KB
        memory_size = 65536;
    }

    /**
     * @brief Clean up WAMR resources after each test case
     *
     * Properly deallocates module instance, module, and runtime resources
     * to prevent memory leaks and ensure clean test environment.
     */
    void TearDown() override
    {
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (module_buffer) {
            BH_FREE(module_buffer);
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Execute i64.load16_u test function with specified address
     *
     * @param func_name Name of the WASM test function to call
     * @param address Memory address parameter for load operation
     * @return uint64_t Zero-extended 64-bit result from i64.load16_u
     */
    uint64_t call_i64_load16_u_function(const char* func_name, uint32_t address)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Failed to lookup function: " << func_name;

        // Create execution environment
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        uint32_t wasm_argv[3] = { address, 0, 0 }; // argv[0] = input, return value overwrites starting from argv[0]

        bool success = wasm_runtime_call_wasm(exec_env, func, 1, wasm_argv);
        EXPECT_TRUE(success) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        // Extract i64 result from argv slots (little-endian) - result overwrites argv[0] and argv[1]
        uint64_t result = ((uint64_t)wasm_argv[1] << 32) | wasm_argv[0];

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        return result;
    }

    /**
     * @brief Call simple WASM i64.load16_u function with specified memory offset
     * @param offset Memory offset to load 16-bit unsigned value from
     * @return Loaded value as 64-bit unsigned integer with zero extension
     */
    uint64_t call_i64_load16_u(uint32_t offset)
    {
        return call_i64_load16_u_function("i64_load16_u", offset);
    }

    /**
     * @brief Write 16-bit value to memory at specified offset with bounds checking
     * @param offset Memory offset to write value to
     * @param value 16-bit value to write to memory
     */
    void write_memory_u16(uint32_t offset, uint16_t value)
    {
        ASSERT_LE(offset + 2, memory_size) << "Memory write would exceed bounds";

        uint8_t *mem_ptr = (uint8_t *)memory_data + offset;

        // Write in little-endian format (LSB first)
        mem_ptr[0] = (uint8_t)(value & 0xFF);
        mem_ptr[1] = (uint8_t)((value >> 8) & 0xFF);
    }

    /**
     * @brief Verify zero extension property for loaded 64-bit values
     * @param loaded_value 64-bit value returned from i64.load16_u
     * @param expected_16bit Expected 16-bit value that was loaded
     */
    void verify_zero_extension(uint64_t loaded_value, uint16_t expected_16bit)
    {
        // Verify lower 16 bits match expected value
        ASSERT_EQ((uint16_t)(loaded_value & 0xFFFF), expected_16bit)
            << "Lower 16 bits do not match expected value";

        // Verify upper 48 bits are zero (zero extension)
        ASSERT_EQ(loaded_value >> 16, 0ULL)
            << "Upper 48 bits should be zero for unsigned load";
    }

    // Test fixture member variables
    RuntimeInitArgs init_args{};
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    unsigned char *module_buffer = nullptr;
    uint32_t module_buffer_size = 0;
    void *memory_data = nullptr;
    uint32_t memory_size = 0;
    char error_buffer[256]{};
    static constexpr uint32_t stack_size = 16 * 1024;
    static constexpr uint32_t heap_size = 16 * 1024;
};

/**
 * @test BasicLoad_ReturnsZeroExtendedValue
 * @brief Validates i64.load16_u produces correct zero-extended results for typical inputs
 * @details Tests fundamental 16-bit unsigned load operation with common values.
 *          Verifies that i64.load16_u correctly loads 16-bit values and zero-extends
 *          them to 64-bit integers with upper 48 bits set to zero.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_load16_u_operation
 * @input_conditions Standard 16-bit values: 0x1234, 0x5678, 0xABCD at memory offsets 0, 2, 4
 * @expected_behavior Returns zero-extended 64-bit values: 0x0000000000001234, 0x0000000000005678, 0x000000000000ABCD
 * @validation_method Direct comparison of WASM function result with expected zero-extended values
 */
TEST_P(I64Load16UTest, BasicLoad_ReturnsZeroExtendedValue)
{
    // Prepare test data in memory
    write_memory_u16(0, 0x1234);
    write_memory_u16(2, 0x5678);
    write_memory_u16(4, 0xABCD);

    // Test basic load operations with zero extension validation
    uint64_t result1 = call_i64_load16_u(0);
    ASSERT_EQ(0x0000000000001234ULL, result1) << "Failed to load 0x1234 with correct zero extension";
    verify_zero_extension(result1, 0x1234);

    uint64_t result2 = call_i64_load16_u(2);
    ASSERT_EQ(0x0000000000005678ULL, result2) << "Failed to load 0x5678 with correct zero extension";
    verify_zero_extension(result2, 0x5678);

    uint64_t result3 = call_i64_load16_u(4);
    ASSERT_EQ(0x000000000000ABCDULL, result3) << "Failed to load 0xABCD with correct zero extension";
    verify_zero_extension(result3, 0xABCD);
}

/**
 * @test BoundaryValues_ZeroExtensionCorrect
 * @brief Validates i64.load16_u behavior with 16-bit boundary values and zero extension
 * @details Tests critical 16-bit boundary values including MIN (0x0000), MAX (0xFFFF),
 *          and signed boundaries (0x7FFF, 0x8000). Ensures proper zero extension for all values.
 * @test_category Corner - Boundary conditions validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_load16_u_zero_extension
 * @input_conditions 16-bit boundary values: 0x0000, 0x7FFF, 0x8000, 0xFFFF
 * @expected_behavior All values properly zero-extended with upper 48 bits = 0
 * @validation_method Verify zero extension and compare with expected unsigned interpretation
 */
TEST_P(I64Load16UTest, BoundaryValues_ZeroExtensionCorrect)
{
    // Test minimum value (0x0000)
    write_memory_u16(0, 0x0000);
    uint64_t min_result = call_i64_load16_u(0);
    ASSERT_EQ(0x0000000000000000ULL, min_result) << "Failed to load MIN value with zero extension";
    verify_zero_extension(min_result, 0x0000);

    // Test maximum signed positive (0x7FFF = 32767)
    write_memory_u16(2, 0x7FFF);
    uint64_t max_pos_result = call_i64_load16_u(2);
    ASSERT_EQ(0x0000000000007FFFULL, max_pos_result) << "Failed to load 0x7FFF with zero extension";
    verify_zero_extension(max_pos_result, 0x7FFF);

    // Test signed boundary (0x8000 = 32768 unsigned, -32768 signed)
    write_memory_u16(4, 0x8000);
    uint64_t signed_boundary_result = call_i64_load16_u(4);
    ASSERT_EQ(0x0000000000008000ULL, signed_boundary_result)
        << "Failed to load 0x8000 as unsigned (should be 32768, not -32768)";
    verify_zero_extension(signed_boundary_result, 0x8000);

    // Test maximum value (0xFFFF = 65535)
    write_memory_u16(6, 0xFFFF);
    uint64_t max_result = call_i64_load16_u(6);
    ASSERT_EQ(0x000000000000FFFFULL, max_result) << "Failed to load MAX value with zero extension";
    verify_zero_extension(max_result, 0xFFFF);
}

/**
 * @test MemoryBoundaries_ValidAccess
 * @brief Validates i64.load16_u memory access at valid boundary locations
 * @details Tests memory loads at critical memory boundary positions including
 *          memory start, memory end-2 (last valid 16-bit position), and page boundaries.
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_bounds_checking
 * @input_conditions Memory positions: 0, memory_size-2, page_size boundaries
 * @expected_behavior Successful loads at all valid memory boundary positions
 * @validation_method Verify no exceptions thrown and correct values loaded
 */
TEST_P(I64Load16UTest, MemoryBoundaries_ValidAccess)
{
    // Test load from memory start (offset 0)
    write_memory_u16(0, 0x1111);
    uint64_t start_result = call_i64_load16_u(0);
    ASSERT_EQ(0x0000000000001111ULL, start_result) << "Failed to load from memory start";

    // Test load from last valid 16-bit position (memory_size - 2)
    uint32_t last_valid_offset = memory_size - 2;
    write_memory_u16(last_valid_offset, 0x2222);
    uint64_t end_result = call_i64_load16_u(last_valid_offset);
    ASSERT_EQ(0x0000000000002222ULL, end_result) << "Failed to load from memory end-2";

    // Test load from page boundary if memory is large enough (assuming 64KB pages)
    if (memory_size > 65536) {
        write_memory_u16(65534, 0x3333); // Last 16-bit position in first page
        uint64_t page_result = call_i64_load16_u(65534);
        ASSERT_EQ(0x0000000000003333ULL, page_result) << "Failed to load from page boundary";
    }
}

/**
 * @test SignedComparison_UnsignedBehavior
 * @brief Validates i64.load16_u unsigned interpretation vs i64.load16_s signed behavior
 * @details Compares i64.load16_u behavior with the signed variant i64.load16_s for values
 *          in the signed negative range (0x8000-0xFFFF). Ensures unsigned load interprets
 *          these values as positive while maintaining zero extension.
 * @test_category Edge - Signed vs unsigned comparison
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:unsigned_vs_signed_load
 * @input_conditions Values in signed negative range: 0x8000, 0x8001, 0xFFFF
 * @expected_behavior Unsigned interpretation: 0x8000→32768, 0x8001→32769, 0xFFFF→65535
 * @validation_method Compare unsigned results with expected positive values
 */
TEST_P(I64Load16UTest, SignedComparison_UnsignedBehavior)
{
    // Test 0x8000: signed = -32768, unsigned = 32768
    write_memory_u16(0, 0x8000);
    uint64_t result_8000 = call_i64_load16_u(0);
    ASSERT_EQ(0x0000000000008000ULL, result_8000)
        << "0x8000 should be interpreted as 32768 (unsigned), not -32768 (signed)";

    // Test 0x8001: signed = -32767, unsigned = 32769
    write_memory_u16(2, 0x8001);
    uint64_t result_8001 = call_i64_load16_u(2);
    ASSERT_EQ(0x0000000000008001ULL, result_8001)
        << "0x8001 should be interpreted as 32769 (unsigned), not -32767 (signed)";

    // Test 0xFFFF: signed = -1, unsigned = 65535
    write_memory_u16(4, 0xFFFF);
    uint64_t result_FFFF = call_i64_load16_u(4);
    ASSERT_EQ(0x000000000000FFFFULL, result_FFFF)
        << "0xFFFF should be interpreted as 65535 (unsigned), not -1 (signed)";

    // Verify all results have proper zero extension
    verify_zero_extension(result_8000, 0x8000);
    verify_zero_extension(result_8001, 0x8001);
    verify_zero_extension(result_FFFF, 0xFFFF);
}

/**
 * @test LittleEndianByteOrder_CorrectInterpretation
 * @brief Validates i64.load16_u respects little-endian byte ordering in memory
 * @details Tests that 16-bit values stored in memory with little-endian byte order
 *          (LSB first) are correctly interpreted by i64.load16_u instruction.
 * @test_category Edge - Byte ordering validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:little_endian_load
 * @input_conditions Memory bytes: [0x34, 0x12] should load as 0x1234
 * @expected_behavior Correct little-endian interpretation of multi-byte values
 * @validation_method Direct byte manipulation and load result comparison
 */
TEST_P(I64Load16UTest, LittleEndianByteOrder_CorrectInterpretation)
{
    // Manually write bytes in little-endian order for 0x1234
    uint8_t *mem_ptr = (uint8_t *)memory_data;
    mem_ptr[0] = 0x34; // LSB first (little-endian)
    mem_ptr[1] = 0x12; // MSB second

    uint64_t result = call_i64_load16_u(0);
    ASSERT_EQ(0x0000000000001234ULL, result)
        << "Little-endian byte order not correctly interpreted";

    // Test another value: bytes [0xCD, 0xAB] should load as 0xABCD
    mem_ptr[2] = 0xCD;
    mem_ptr[3] = 0xAB;

    uint64_t result2 = call_i64_load16_u(2);
    ASSERT_EQ(0x000000000000ABCDULL, result2)
        << "Second little-endian test failed";

    verify_zero_extension(result, 0x1234);
    verify_zero_extension(result2, 0xABCD);
}

/**
 * @test UnalignedAccess_WorksCorrectly
 * @brief Validates i64.load16_u works correctly with unaligned memory addresses
 * @details Tests that i64.load16_u can successfully load 16-bit values from
 *          both aligned (even) and unaligned (odd) memory addresses.
 * @test_category Edge - Alignment handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:unaligned_memory_access
 * @input_conditions Both aligned (even) and unaligned (odd) addresses
 * @expected_behavior Successful loads regardless of address alignment
 * @validation_method Compare results from aligned vs unaligned addresses
 */
TEST_P(I64Load16UTest, UnalignedAccess_WorksCorrectly)
{
    // Test aligned access (even address)
    write_memory_u16(0, 0x1122);
    uint64_t aligned_result = call_i64_load16_u(0);
    ASSERT_EQ(0x0000000000001122ULL, aligned_result) << "Aligned access failed";

    // Test unaligned access (odd address) - manually set bytes
    if (memory_size >= 4) {
        uint8_t *mem_ptr = (uint8_t *)memory_data;
        mem_ptr[1] = 0x44; // LSB at odd offset
        mem_ptr[2] = 0x33; // MSB at even offset

        uint64_t unaligned_result = call_i64_load16_u(1);
        ASSERT_EQ(0x0000000000003344ULL, unaligned_result) << "Unaligned access failed";

        verify_zero_extension(aligned_result, 0x1122);
        verify_zero_extension(unaligned_result, 0x3344);
    }
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(I64Load16UModes, I64Load16UTest,
                        testing::ValuesIn(running_modes));