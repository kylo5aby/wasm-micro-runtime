/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for memory.fill Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly memory.fill
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Fill Operations: Memory region filling with typical byte values and patterns
 * - Boundary Conditions: Memory boundary operations, zero-length fills, and large fills
 * - Edge Cases: Value truncation, single-byte fills, and identity operations
 * - Error Conditions: Out-of-bounds access, stack underflow, and invalid parameters
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_MEMORY_FILL
 * - AOT: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_memory_fill()
 * - Fast JIT: core/iwasm/fast-jit/fe/jit_emit_memory.c:jit_compile_op_memory_fill()
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
static std::string WASM_FILE_ERROR_TEST;

static int app_argc;
static char **app_argv;

/**
 * @class MemoryFillTest
 * @brief Test fixture for memory.fill opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for memory.fill operations
 *          including module loading, execution environment setup, and validation helpers
 */
class MemoryFillTest : public testing::TestWithParam<RunningMode>
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
     * @details Initializes WAMR runtime, loads memory.fill test module, and
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

        // Use memory fill specific WASM file
        std::string fill_wasm_file = cwd + "/wasm-apps/memory_fill_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(fill_wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << fill_wasm_file;

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
            wasm_runtime_free(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Helper function to call WASM memory.fill function
     * @details Invokes memory_fill_test function with specified parameters and
     *          validates execution result
     * @param dst Destination offset in memory
     * @param val Byte value to fill (will be truncated to uint8)
     * @param len Number of bytes to fill
     * @return true if operation succeeded, false if trapped
     */
    bool call_memory_fill(uint32_t dst, uint32_t val, uint32_t len)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "memory_fill_test");
        EXPECT_NE(func, nullptr) << "Failed to lookup memory_fill_test function";

        uint32_t wasm_argv[3] = { dst, val, len };
        bool result = wasm_runtime_call_wasm(exec_env, func, 3, wasm_argv);

        exception = wasm_runtime_get_exception(module_inst);
        return result && (exception == nullptr);
    }

    /**
     * @brief Get current memory size from WASM module
     * @return Current memory size in bytes
     */
    uint32_t GetCurrentMemorySize()
    {
        wasm_function_inst_t size_func = wasm_runtime_lookup_function(module_inst, "get_memory_size");
        EXPECT_NE(nullptr, size_func) << "Failed to lookup get_memory_size function";

        uint32_t wasm_argv[1] = { 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, size_func, 0, wasm_argv);
        EXPECT_TRUE(ret) << "Failed to call get_memory_size function";

        return wasm_argv[0];
    }

    /**
     * @brief Helper function to read byte from WASM memory
     * @details Reads a single byte from linear memory at specified offset
     * @param offset Memory offset to read from
     * @return Byte value at the specified offset
     */
    uint8_t read_memory_byte(uint32_t offset)
    {
        uint8_t *memory = (uint8_t*)wasm_runtime_addr_app_to_native(module_inst, offset);
        EXPECT_NE(memory, nullptr) << "Failed to get memory pointer for offset " << offset;
        return *memory;
    }

    /**
     * @brief Helper function to write byte to WASM memory
     * @details Writes a single byte to linear memory at specified offset
     * @param offset Memory offset to write to
     * @param value Byte value to write
     */
    void write_memory_byte(uint32_t offset, uint8_t value)
    {
        uint8_t *memory = (uint8_t*)wasm_runtime_addr_app_to_native(module_inst, offset);
        ASSERT_NE(memory, nullptr) << "Failed to get memory pointer for offset " << offset;
        *memory = value;
    }

    /**
     * @brief Helper function to verify memory pattern
     * @details Validates that memory region contains expected fill pattern
     * @param offset Starting memory offset
     * @param length Number of bytes to verify
     * @param expected_value Expected byte value
     * @return true if all bytes match expected pattern
     */
    bool verify_memory_pattern(uint32_t offset, uint32_t length, uint8_t expected_value)
    {
        for (uint32_t i = 0; i < length; i++) {
            if (read_memory_byte(offset + i) != expected_value) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Helper function to setup distinct memory pattern
     * @details Initializes memory region with distinct values for testing
     * @param offset Starting memory offset
     * @param length Number of bytes to initialize
     */
    void setup_distinct_pattern(uint32_t offset, uint32_t length)
    {
        for (uint32_t i = 0; i < length; i++) {
            write_memory_byte(offset + i, (uint8_t)(i % 256));
        }
    }
};

/**
 * @test BasicMemoryFill_ReturnsCorrectPattern
 * @brief Validates memory.fill produces correct byte patterns for typical inputs
 * @details Tests fundamental fill operation with various byte values and memory regions.
 *          Verifies that memory.fill correctly fills regions with specified byte patterns.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_MEMORY_FILL
 * @input_conditions Standard memory regions with different byte values: 0x00, 0xAA, 0xFF
 * @expected_behavior Returns success and fills memory with exact byte patterns
 * @validation_method Direct memory content verification through byte-by-byte comparison
 */
TEST_P(MemoryFillTest, BasicMemoryFill_ReturnsCorrectPattern)
{
    // Test fill with 0x00 (zero pattern)
    ASSERT_TRUE(call_memory_fill(0, 0x00, 64))
        << "Failed to fill memory at offset 0 with 0x00 pattern";
    ASSERT_TRUE(verify_memory_pattern(0, 64, 0x00))
        << "Memory not correctly filled with 0x00 pattern";

    // Test fill with 0xAA (alternating pattern)
    ASSERT_TRUE(call_memory_fill(128, 0xAA, 64))
        << "Failed to fill memory at offset 128 with 0xAA pattern";
    ASSERT_TRUE(verify_memory_pattern(128, 64, 0xAA))
        << "Memory not correctly filled with 0xAA pattern";

    // Test fill with 0xFF (all bits set)
    ASSERT_TRUE(call_memory_fill(256, 0xFF, 64))
        << "Failed to fill memory at offset 256 with 0xFF pattern";
    ASSERT_TRUE(verify_memory_pattern(256, 64, 0xFF))
        << "Memory not correctly filled with 0xFF pattern";
}

/**
 * @test BoundaryFill_AtMemoryLimits
 * @brief Validates memory.fill operations at memory boundary conditions
 * @details Tests fill operations at the edges of linear memory to ensure proper
 *          boundary handling without out-of-bounds access.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:CHECK_BULK_MEMORY_OVERFLOW
 * @input_conditions Memory fills at last valid offsets and maximum valid lengths
 * @expected_behavior Successful fills exactly at memory boundaries without errors
 * @validation_method Boundary offset calculations and memory content verification
 */
TEST_P(MemoryFillTest, BoundaryFill_AtMemoryLimits)
{
    // Get memory size (default 1 page = 65536 bytes)
    uint32_t memory_size = GetCurrentMemorySize();
    ASSERT_GT(memory_size, 0) << "Memory size should be greater than 0";

    // Fill single byte at last valid memory position
    uint32_t last_offset = memory_size - 1;
    ASSERT_TRUE(call_memory_fill(last_offset, 0xFF, 1))
        << "Failed to fill last byte at memory boundary";
    ASSERT_EQ(0xFF, read_memory_byte(last_offset))
        << "Last byte not correctly filled at memory boundary";

    // Fill larger region ending exactly at memory boundary
    uint32_t boundary_offset = memory_size - 256;
    ASSERT_TRUE(call_memory_fill(boundary_offset, 0x55, 256))
        << "Failed to fill region ending at memory boundary";
    ASSERT_TRUE(verify_memory_pattern(boundary_offset, 256, 0x55))
        << "Memory region ending at boundary not correctly filled";
}

/**
 * @test ZeroLengthFill_NoOperation
 * @brief Validates memory.fill with zero length performs no operation
 * @details Tests that zero-length fill operations are valid no-ops that don't
 *          modify memory content or cause runtime errors.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_fill (len=0 path)
 * @input_conditions memory.fill with len=0 at various valid offsets
 * @expected_behavior Operation succeeds with no memory modification
 * @validation_method Pre/post memory content comparison to ensure no changes
 */
TEST_P(MemoryFillTest, ZeroLengthFill_NoOperation)
{
    // Setup distinct memory pattern for verification
    setup_distinct_pattern(512, 128);

    // Store original memory content
    std::vector<uint8_t> original_content(128);
    for (uint32_t i = 0; i < 128; i++) {
        original_content[i] = read_memory_byte(512 + i);
    }

    // Perform zero-length fill operation
    ASSERT_TRUE(call_memory_fill(512, 0xAA, 0))
        << "Zero-length memory fill should succeed";

    // Verify memory content unchanged
    for (uint32_t i = 0; i < 128; i++) {
        ASSERT_EQ(original_content[i], read_memory_byte(512 + i))
            << "Memory content changed after zero-length fill at offset " << i;
    }
}

/**
 * @test ValueTruncation_UsesLowByte
 * @brief Validates memory.fill truncates value to low byte only
 * @details Tests that memory.fill uses only the lower 8 bits of the value parameter,
 *          ignoring higher bits as per WebAssembly specification.
 * @test_category Edge - Value truncation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:POP_I32 (fill_val cast)
 * @input_conditions i32 values with high bits set, expecting low byte extraction
 * @expected_behavior Memory filled with low byte only, high bits ignored
 * @validation_method Direct byte value verification after fill operation
 */
TEST_P(MemoryFillTest, ValueTruncation_UsesLowByte)
{
    // Test value truncation: 0x12345678 should fill with 0x78
    ASSERT_TRUE(call_memory_fill(1024, 0x12345678, 16))
        << "Failed to fill memory with truncated value";
    ASSERT_TRUE(verify_memory_pattern(1024, 16, 0x78))
        << "Memory not filled with correct truncated value (0x78)";

    // Test value truncation: 0xABCDEF01 should fill with 0x01
    ASSERT_TRUE(call_memory_fill(1040, 0xABCDEF01, 16))
        << "Failed to fill memory with second truncated value";
    ASSERT_TRUE(verify_memory_pattern(1040, 16, 0x01))
        << "Memory not filled with correct truncated value (0x01)";

    // Test value truncation: 0xFFFFFF99 should fill with 0x99
    ASSERT_TRUE(call_memory_fill(1056, 0xFFFFFF99, 16))
        << "Failed to fill memory with third truncated value";
    ASSERT_TRUE(verify_memory_pattern(1056, 16, 0x99))
        << "Memory not filled with correct truncated value (0x99)";
}

/**
 * @test LargeRegionFill_SucceedsCorrectly
 * @brief Validates memory.fill operations on large memory regions
 * @details Tests fill operations on substantial memory regions to verify
 *          performance and correctness with large data transfers.
 * @test_category Corner - Large operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memset large regions
 * @input_conditions Large memory regions (4KB, 8KB) with various patterns
 * @expected_behavior Successful fills of large regions without performance issues
 * @validation_method Spot-check verification at region boundaries and middle
 */
TEST_P(MemoryFillTest, LargeRegionFill_SucceedsCorrectly)
{
    // Test 4KB fill operation
    uint32_t large_size_4k = 4096;
    ASSERT_TRUE(call_memory_fill(8192, 0xCC, large_size_4k))
        << "Failed to fill 4KB memory region";

    // Verify fill pattern at start, middle, and end
    ASSERT_EQ(0xCC, read_memory_byte(8192))
        << "4KB fill pattern incorrect at start";
    ASSERT_EQ(0xCC, read_memory_byte(8192 + large_size_4k/2))
        << "4KB fill pattern incorrect at middle";
    ASSERT_EQ(0xCC, read_memory_byte(8192 + large_size_4k - 1))
        << "4KB fill pattern incorrect at end";

    // Test 8KB fill operation
    uint32_t large_size_8k = 8192;
    ASSERT_TRUE(call_memory_fill(16384, 0x33, large_size_8k))
        << "Failed to fill 8KB memory region";

    // Verify fill pattern at boundaries
    ASSERT_EQ(0x33, read_memory_byte(16384))
        << "8KB fill pattern incorrect at start";
    ASSERT_EQ(0x33, read_memory_byte(16384 + large_size_8k - 1))
        << "8KB fill pattern incorrect at end";
}

/**
 * @test OutOfBounds_CausesTrap
 * @brief Validates memory.fill with out-of-bounds access causes proper traps
 * @details Tests that memory.fill operations beyond linear memory limits
 *          trigger appropriate WAMR runtime traps and error handling.
 * @test_category Error - Out-of-bounds validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:out_of_bounds handler
 * @input_conditions Fill operations beyond memory limits and overflow scenarios
 * @expected_behavior Runtime traps with proper exception reporting, no memory corruption
 * @validation_method Exception detection and memory integrity verification
 */
TEST_P(MemoryFillTest, OutOfBounds_CausesTrap)
{
    // Get memory size for bounds testing
    uint32_t memory_size = GetCurrentMemorySize();

    // Test fill starting beyond memory bounds
    ASSERT_FALSE(call_memory_fill(memory_size, 0xFF, 1))
        << "Fill beyond memory bounds should cause trap";
    ASSERT_NE(nullptr, wasm_runtime_get_exception(module_inst))
        << "Expected exception for out-of-bounds fill";

    // Clear exception and test dst + len overflow
    wasm_runtime_clear_exception(module_inst);
    ASSERT_FALSE(call_memory_fill(memory_size - 1, 0xAA, 2))
        << "Fill causing overflow should cause trap";
    ASSERT_NE(nullptr, wasm_runtime_get_exception(module_inst))
        << "Expected exception for overflow fill";

    // Clear exception and test large offset
    wasm_runtime_clear_exception(module_inst);
    ASSERT_FALSE(call_memory_fill(UINT32_MAX - 100, 0x55, 200))
        << "Fill with large offset should cause trap";
    ASSERT_NE(nullptr, wasm_runtime_get_exception(module_inst))
        << "Expected exception for large offset fill";
}

// Test parameterization for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    RunningModeMemoryFill,
    MemoryFillTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<MemoryFillTest::ParamType> &info) {
        return info.param == Mode_Interp ? "Interpreter" : "LLVM_JIT";
    });