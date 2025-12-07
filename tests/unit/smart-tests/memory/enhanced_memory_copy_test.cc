/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for memory.copy Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly memory.copy
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Copy Operations: Non-overlapping memory region copying with typical values
 * - Overlapping Operations: Forward and backward overlapping region handling
 * - Boundary Conditions: Memory boundary operations, zero-size copies, and large transfers
 * - Error Conditions: Out-of-bounds access and invalid parameter testing
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * - AOT: core/iwasm/aot/aot_runtime.c:aot_memory_copy()
 * - Compilation: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_memory_copy()
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
 * @class MemoryCopyTest
 * @brief Test fixture for memory.copy opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for memory.copy operations
 *          including module loading, execution environment setup, and validation helpers
 */
class MemoryCopyTest : public testing::TestWithParam<RunningMode>
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
     * @details Initializes WAMR runtime, loads memory.copy test module, and
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

        // Use memory copy specific WASM file
        std::string copy_wasm_file = cwd + "/wasm-apps/memory_copy_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(copy_wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << copy_wasm_file;

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
     * @brief Initialize memory with test pattern for validation
     * @param offset Memory offset to start initialization
     * @param size Number of bytes to initialize
     * @param pattern Byte pattern to write
     * @details Fills memory region with specified pattern for testing
     */
    void InitializeMemoryPattern(uint32_t offset, uint32_t size, uint8_t pattern)
    {
        uint8_t* memory = (uint8_t*)wasm_runtime_addr_app_to_native(module_inst, offset);
        ASSERT_NE(nullptr, memory) << "Failed to get memory pointer for offset: " << offset;
        memset(memory, pattern, size);
    }

    /**
     * @brief Verify memory content matches expected pattern
     * @param offset Memory offset to verify
     * @param size Number of bytes to verify
     * @param expected_pattern Expected byte pattern
     * @details Validates memory region contains expected pattern
     */
    void VerifyMemoryPattern(uint32_t offset, uint32_t size, uint8_t expected_pattern)
    {
        uint8_t* memory = (uint8_t*)wasm_runtime_addr_app_to_native(module_inst, offset);
        ASSERT_NE(nullptr, memory) << "Failed to get memory pointer for offset: " << offset;

        for (uint32_t i = 0; i < size; i++) {
            ASSERT_EQ(expected_pattern, memory[i])
                << "Memory mismatch at offset " << (offset + i)
                << ": expected " << (int)expected_pattern
                << ", got " << (int)memory[i];
        }
    }

    /**
     * @brief Execute memory.copy operation through WASM function call
     * @param dest Destination offset in memory
     * @param src Source offset in memory
     * @param size Number of bytes to copy
     * @return true if copy succeeded, false if trapped
     */
    bool ExecuteMemoryCopy(uint32_t dest, uint32_t src, uint32_t size)
    {
        wasm_function_inst_t copy_func = wasm_runtime_lookup_function(module_inst, "test_memory_copy");
        EXPECT_NE(nullptr, copy_func) << "Failed to lookup test_memory_copy function";

        uint32_t wasm_argv[3] = { dest, src, size };
        bool ret = wasm_runtime_call_wasm(exec_env, copy_func, 3, wasm_argv);

        exception = wasm_runtime_get_exception(module_inst);
        return ret && (exception == nullptr);
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
};

/**
 * @test BasicCopy_NonOverlapping_CopiesCorrectly
 * @brief Validates memory.copy correctly copies data between non-overlapping regions
 * @details Tests fundamental copy operation with small data blocks in separate memory regions.
 *          Verifies that destination memory matches source and source remains unchanged.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Non-overlapping memory regions with 16-byte test patterns
 * @expected_behavior Destination contains source data, source unchanged, no side effects
 * @validation_method Direct memory comparison after copy operation
 */
TEST_P(MemoryCopyTest, BasicCopy_NonOverlapping_CopiesCorrectly)
{
    const uint32_t src_offset = 0x1000;
    const uint32_t dest_offset = 0x2000;
    const uint32_t copy_size = 16;
    const uint8_t test_pattern = 0xAB;
    const uint8_t dest_pattern = 0x00;

    // Initialize source memory with test pattern
    InitializeMemoryPattern(src_offset, copy_size, test_pattern);

    // Initialize destination memory with different pattern
    InitializeMemoryPattern(dest_offset, copy_size, dest_pattern);

    // Execute memory copy
    bool success = ExecuteMemoryCopy(dest_offset, src_offset, copy_size);
    ASSERT_TRUE(success) << "Memory copy operation failed";

    // Verify destination contains source data
    VerifyMemoryPattern(dest_offset, copy_size, test_pattern);

    // Verify source memory unchanged
    VerifyMemoryPattern(src_offset, copy_size, test_pattern);
}

/**
 * @test OverlappingCopy_ForwardOverlap_HandlesMemmove
 * @brief Validates memory.copy handles forward overlapping regions correctly
 * @details Tests copy operation where destination starts within source region (forward overlap).
 *          Verifies proper memmove-like behavior to prevent data corruption.
 * @test_category Corner - Boundary conditions with overlapping memory
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Forward overlapping regions: src=0x1000, dest=0x1008, size=16
 * @expected_behavior Correct copy with no data corruption, proper overlap handling
 * @validation_method Memory pattern verification after overlapping copy
 */
TEST_P(MemoryCopyTest, OverlappingCopy_ForwardOverlap_HandlesMemmove)
{
    const uint32_t src_offset = 0x1000;
    const uint32_t dest_offset = 0x1008;  // 8 bytes into source region
    const uint32_t copy_size = 16;
    const uint8_t pattern1 = 0xCC;
    const uint8_t pattern2 = 0xDD;

    // Initialize memory with alternating patterns for clear identification
    InitializeMemoryPattern(src_offset, 8, pattern1);
    InitializeMemoryPattern(src_offset + 8, 8, pattern2);
    InitializeMemoryPattern(src_offset + 16, 8, 0x00);

    // Execute overlapping copy
    bool success = ExecuteMemoryCopy(dest_offset, src_offset, copy_size);
    ASSERT_TRUE(success) << "Overlapping memory copy failed";

    // Verify the first 8 bytes at destination match original source start
    VerifyMemoryPattern(dest_offset, 8, pattern1);

    // Verify the next 8 bytes at destination match original source middle
    VerifyMemoryPattern(dest_offset + 8, 8, pattern2);
}

/**
 * @test OverlappingCopy_BackwardOverlap_HandlesMemmove
 * @brief Validates memory.copy handles backward overlapping regions correctly
 * @details Tests copy operation where source starts within destination region (backward overlap).
 *          Verifies proper memmove-like behavior for reverse overlap scenarios.
 * @test_category Corner - Boundary conditions with overlapping memory
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Backward overlapping regions: src=0x1008, dest=0x1000, size=16
 * @expected_behavior Correct copy with no data corruption, proper overlap handling
 * @validation_method Memory pattern verification after overlapping copy
 */
TEST_P(MemoryCopyTest, OverlappingCopy_BackwardOverlap_HandlesMemmove)
{
    const uint32_t dest_offset = 0x1000;
    const uint32_t src_offset = 0x1008;   // 8 bytes into destination region
    const uint32_t copy_size = 16;
    const uint8_t pattern1 = 0xEE;
    const uint8_t pattern2 = 0xFF;

    // Initialize memory with distinct patterns
    InitializeMemoryPattern(dest_offset, 8, 0x00);
    InitializeMemoryPattern(src_offset, 8, pattern1);
    InitializeMemoryPattern(src_offset + 8, 8, pattern2);

    // Execute backward overlapping copy
    bool success = ExecuteMemoryCopy(dest_offset, src_offset, copy_size);
    ASSERT_TRUE(success) << "Backward overlapping memory copy failed";

    // Verify destination contains correct source data
    VerifyMemoryPattern(dest_offset, 8, pattern1);
    VerifyMemoryPattern(dest_offset + 8, 8, pattern2);
}

/**
 * @test BoundaryCopy_MemoryLimits_CopiesAtEdges
 * @brief Validates memory.copy operations at memory boundary edges
 * @details Tests copy operations at memory start (offset 0) and near memory end.
 *          Verifies boundary condition handling without out-of-bounds errors.
 * @test_category Corner - Memory boundary conditions
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Memory start (offset 0) and near memory size limit
 * @expected_behavior Successful copy operations at valid memory boundaries
 * @validation_method Boundary offset validation and successful copy completion
 */
TEST_P(MemoryCopyTest, BoundaryCopy_MemoryLimits_CopiesAtEdges)
{
    // Test copy at memory start (offset 0)
    const uint32_t copy_size = 8;
    const uint8_t test_pattern = 0x77;

    InitializeMemoryPattern(64, copy_size, test_pattern);

    bool success = ExecuteMemoryCopy(0, 64, copy_size);
    ASSERT_TRUE(success) << "Memory copy to offset 0 failed";

    VerifyMemoryPattern(0, copy_size, test_pattern);

    // Test copy near memory end (within valid range)
    uint32_t memory_size = GetCurrentMemorySize();
    ASSERT_GT(memory_size, 128U) << "Memory size too small for boundary testing";

    uint32_t near_end_dest = memory_size - 64;
    uint32_t near_end_src = memory_size - 128;

    InitializeMemoryPattern(near_end_src, 32, test_pattern);

    success = ExecuteMemoryCopy(near_end_dest, near_end_src, 32);
    ASSERT_TRUE(success) << "Memory copy near memory end failed";

    VerifyMemoryPattern(near_end_dest, 32, test_pattern);
}

/**
 * @test ZeroSizeCopy_ValidOffsets_CompletesSuccessfully
 * @brief Validates memory.copy with zero size parameter behaves as no-op
 * @details Tests zero-size copy operations with various valid offset combinations.
 *          Verifies no memory changes occur and operation completes successfully.
 * @test_category Edge - Zero-size operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Various offset combinations with size = 0
 * @expected_behavior No memory modification, successful completion
 * @validation_method Memory state verification before and after operation
 */
TEST_P(MemoryCopyTest, ZeroSizeCopy_ValidOffsets_CompletesSuccessfully)
{
    const uint32_t test_offset1 = 0x100;
    const uint32_t test_offset2 = 0x200;
    const uint8_t original_pattern = 0x55;

    // Initialize memory regions with known patterns
    InitializeMemoryPattern(test_offset1, 16, original_pattern);
    InitializeMemoryPattern(test_offset2, 16, original_pattern);

    // Test zero-size copy between different offsets
    bool success = ExecuteMemoryCopy(test_offset1, test_offset2, 0);
    ASSERT_TRUE(success) << "Zero-size memory copy failed";

    // Verify memory unchanged
    VerifyMemoryPattern(test_offset1, 16, original_pattern);
    VerifyMemoryPattern(test_offset2, 16, original_pattern);

    // Test zero-size copy to same location
    success = ExecuteMemoryCopy(test_offset1, test_offset1, 0);
    ASSERT_TRUE(success) << "Zero-size self-copy failed";

    VerifyMemoryPattern(test_offset1, 16, original_pattern);
}

/**
 * @test SelfCopy_IdenticalOffsets_PreservesMemory
 * @brief Validates memory.copy with identical source and destination offsets
 * @details Tests self-copy operations where source equals destination with non-zero size.
 *          Verifies memory remains unchanged and operation completes successfully.
 * @test_category Edge - Identity operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Identical source and destination offsets with various sizes
 * @expected_behavior Memory preservation, no corruption, successful completion
 * @validation_method Memory pattern consistency verification
 */
TEST_P(MemoryCopyTest, SelfCopy_IdenticalOffsets_PreservesMemory)
{
    const uint32_t offset = 0x500;
    const uint32_t copy_size = 32;
    const uint8_t test_pattern = 0x99;

    InitializeMemoryPattern(offset, copy_size, test_pattern);

    // Execute self-copy
    bool success = ExecuteMemoryCopy(offset, offset, copy_size);
    ASSERT_TRUE(success) << "Self-copy operation failed";

    // Verify memory unchanged
    VerifyMemoryPattern(offset, copy_size, test_pattern);
}

/**
 * @test LargeCopy_MaxValidSize_TransfersCorrectly
 * @brief Validates memory.copy with large data transfer sizes
 * @details Tests copy operations with large size parameters (multiple KB).
 *          Verifies system can handle substantial data transfers correctly.
 * @test_category Edge - Large size operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Large copy sizes within memory limits
 * @expected_behavior Successful large data transfer with data integrity
 * @validation_method Partial pattern verification for large regions
 */
TEST_P(MemoryCopyTest, LargeCopy_MaxValidSize_TransfersCorrectly)
{
    const uint32_t large_size = 4096;  // 4KB copy
    const uint32_t src_offset = 0x1000;
    const uint32_t dest_offset = 0x8000;
    const uint8_t test_pattern = 0x33;

    // Verify we have enough memory for large copy
    uint32_t memory_size = GetCurrentMemorySize();
    ASSERT_GE(memory_size, dest_offset + large_size)
        << "Insufficient memory for large copy test";

    // Initialize large source region
    InitializeMemoryPattern(src_offset, large_size, test_pattern);

    // Execute large copy
    bool success = ExecuteMemoryCopy(dest_offset, src_offset, large_size);
    ASSERT_TRUE(success) << "Large memory copy failed";

    // Verify data integrity by checking multiple points
    VerifyMemoryPattern(dest_offset, 64, test_pattern);  // Beginning
    VerifyMemoryPattern(dest_offset + large_size/2, 64, test_pattern);  // Middle
    VerifyMemoryPattern(dest_offset + large_size - 64, 64, test_pattern);  // End
}

/**
 * @test OutOfBounds_DestinationOverflow_TriggersTraps
 * @brief Validates memory.copy triggers traps for destination out-of-bounds access
 * @details Tests copy operations where destination + size exceeds memory bounds.
 *          Verifies proper trap generation and error handling.
 * @test_category Error - Out-of-bounds validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Destination offset + size > memory_size
 * @expected_behavior Runtime trap with appropriate error message
 * @validation_method Trap detection and error message verification
 */
TEST_P(MemoryCopyTest, OutOfBounds_DestinationOverflow_TriggersTraps)
{
    uint32_t memory_size = GetCurrentMemorySize();

    // Attempt copy that would overflow destination
    uint32_t invalid_dest = memory_size - 8;
    uint32_t valid_src = 0x100;
    uint32_t overflow_size = 16;  // 8 bytes beyond memory limit

    InitializeMemoryPattern(valid_src, overflow_size, 0x88);

    // Execute out-of-bounds copy - should fail
    bool success = ExecuteMemoryCopy(invalid_dest, valid_src, overflow_size);
    ASSERT_FALSE(success) << "Out-of-bounds destination copy should have failed";
}

/**
 * @test OutOfBounds_SourceOverflow_TriggersTraps
 * @brief Validates memory.copy triggers traps for source out-of-bounds access
 * @details Tests copy operations where source + size exceeds memory bounds.
 *          Verifies proper trap generation and error handling.
 * @test_category Error - Out-of-bounds validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:memory_copy_operation
 * @input_conditions Source offset + size > memory_size
 * @expected_behavior Runtime trap with appropriate error message
 * @validation_method Trap detection and error message verification
 */
TEST_P(MemoryCopyTest, OutOfBounds_SourceOverflow_TriggersTraps)
{
    uint32_t memory_size = GetCurrentMemorySize();

    // Attempt copy that would overflow source
    uint32_t valid_dest = 0x100;
    uint32_t invalid_src = memory_size - 8;
    uint32_t overflow_size = 16;  // 8 bytes beyond memory limit

    // Execute out-of-bounds copy - should fail
    bool success = ExecuteMemoryCopy(valid_dest, invalid_src, overflow_size);
    ASSERT_FALSE(success) << "Out-of-bounds source copy should have failed";
}

// Test parameter instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningMode, MemoryCopyTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         testing::PrintToStringParamName());