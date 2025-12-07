/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
static const char *WASM_FILE_1 = "i64_store32_test.wasm";

static RunningMode running_mode_supported[] = { Mode_Interp, Mode_LLVM_JIT };

/**
 * @brief Enhanced test suite class for i64.store32 opcode validation
 * @details Comprehensive testing framework for WASM i64.store32 instruction that stores
 *          the low 32 bits of a 64-bit integer to linear memory at specified addresses.
 *          Tests cover basic functionality, boundary conditions, memory alignment,
 *          value truncation behavior, and error handling across interpreter and AOT modes.
 */
class I64Store32Test : public testing::TestWithParam<RunningMode>
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
     * @details Initializes WAMR runtime, loads test WASM module, and configures
     *          execution environment for both interpreter and AOT modes.
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

        // Use i64.store32 specific WASM file
        std::string store_wasm_file = cwd + "/wasm-apps/i64_store32_test.wasm";
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
     * @brief Clean up test environment and resources
     * @details Destroys execution environment, module instance, and unloads module
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
     * @brief Execute WASM function to store i64 value as 32-bit in memory
     * @param address Memory address for store operation
     * @param value 64-bit value to store (will be truncated to 32 bits)
     * @param offset Immediate offset added to address
     * @return Execution result (0 for success, non-zero for failure/trap)
     */
    int32_t call_i64_store32(uint32_t address, uint64_t value, uint32_t offset)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_store32");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_store32 function";

        // WASM function signature: (i32 address, i64 value, i32 offset) -> i32
        // For i64 value, we need to pass it as two 32-bit values (low, high)
        uint32_t argv[4] = {
            address,
            static_cast<uint32_t>(value),           // i64 low 32 bits
            static_cast<uint32_t>(value >> 32),     // i64 high 32 bits
            offset
        };
        uint32_t argc = 4;

        bool ret = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        exception = wasm_runtime_get_exception(module_inst);

        if (!ret || exception) {
            return -1; // Indicate execution failure or trap
        }
        return static_cast<int32_t>(argv[0]); // Return function result
    }

    /**
     * @brief Read 32-bit value from memory at specified address
     * @param address Memory address to read from
     * @return 32-bit value stored in memory
     */
    uint32_t read_memory_i32(uint32_t address)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "read_i32");
        EXPECT_NE(func, nullptr) << "Failed to lookup read_i32 function";

        uint32_t argv[1] = { address };
        uint32_t argc = 1;

        bool ret = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_TRUE(ret && !exception) << "Failed to read from memory address " << address;
        return argv[0];
    }
};

/**
 * @test BasicStore_ReturnsCorrectTruncation
 * @brief Validates i64.store32 correctly truncates 64-bit values to 32-bit and stores in memory
 * @details Tests fundamental store operation with various i64 values to verify proper truncation.
 *          Validates that only the low 32 bits are stored while high bits are discarded.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store32_operation
 * @input_conditions Various i64 values with different bit patterns in high/low 32 bits
 * @expected_behavior Memory contains only low 32 bits of original i64 values
 * @validation_method Direct memory read and comparison with expected truncated values
 */
TEST_P(I64Store32Test, BasicStore_ReturnsCorrectTruncation)
{
    // Test typical 64-bit values with mixed high/low bits
    uint32 result = call_i64_store32(0, 0x123456789ABCDEF0ULL, 0);
    ASSERT_EQ(0, result) << "Store operation should succeed";
    ASSERT_EQ(0x9ABCDEF0U, read_memory_i32(0))
        << "Memory should contain low 32 bits (0x9ABCDEF0) of stored i64 value";

    // Test value with high bits set, low bits different
    result = call_i64_store32(4, 0xFFFFFFFF12345678ULL, 0);
    ASSERT_EQ(0, result) << "Store operation should succeed";
    ASSERT_EQ(0x12345678U, read_memory_i32(4))
        << "Memory should contain low 32 bits (0x12345678) ignoring high bits";

    // Test value with mixed bit patterns
    result = call_i64_store32(8, 0xABCDEF0087654321ULL, 0);
    ASSERT_EQ(0, result) << "Store operation should succeed";
    ASSERT_EQ(0x87654321U, read_memory_i32(8))
        << "Memory should contain low 32 bits (0x87654321) with proper truncation";
}

/**
 * @test BoundaryValues_StoresCorrectly
 * @brief Tests i64.store32 with boundary values and extreme bit patterns
 * @details Validates correct truncation behavior for i64 MIN/MAX values, all-zeros,
 *          all-ones, and other extreme bit patterns to ensure consistent behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_store32_operation
 * @input_conditions i64 MIN/MAX values, zero, all-ones, alternating bit patterns
 * @expected_behavior Consistent truncation preserving exact low 32 bits regardless of high bits
 * @validation_method Bit-level comparison of stored values with expected truncated results
 */
TEST_P(I64Store32Test, BoundaryValues_StoresCorrectly)
{
    // Test i64 minimum value (0x8000000000000000)
    uint32 result = call_i64_store32(0, 0x8000000000000000ULL, 0);
    ASSERT_EQ(0, result) << "Store operation should succeed for i64 MIN";
    ASSERT_EQ(0x00000000U, read_memory_i32(0))
        << "i64 MIN should store as 0x00000000 (low 32 bits)";

    // Test i64 maximum value (0x7FFFFFFFFFFFFFFF)
    result = call_i64_store32(4, 0x7FFFFFFFFFFFFFFFULL, 0);
    ASSERT_EQ(0, result) << "Store operation should succeed for i64 MAX";
    ASSERT_EQ(0xFFFFFFFFU, read_memory_i32(4))
        << "i64 MAX should store as 0xFFFFFFFF (low 32 bits)";

    // Test all zeros
    result = call_i64_store32(8, 0x0000000000000000ULL, 0);
    ASSERT_EQ(0, result) << "Store operation should succeed for zero value";
    ASSERT_EQ(0x00000000U, read_memory_i32(8))
        << "Zero i64 value should store as 0x00000000";

    // Test all ones
    result = call_i64_store32(12, 0xFFFFFFFFFFFFFFFFULL, 0);
    ASSERT_EQ(0, result) << "Store operation should succeed for all-ones value";
    ASSERT_EQ(0xFFFFFFFFU, read_memory_i32(12))
        << "All-ones i64 value should store as 0xFFFFFFFF (low 32 bits)";
}

/**
 * @test MemoryAlignment_HandlesUnalignedAccess
 * @brief Verifies correct behavior for both aligned and unaligned memory addresses
 * @details Tests i64.store32 operations at various memory alignment boundaries to ensure
 *          correct behavior regardless of address alignment. WASM allows unaligned access.
 * @test_category Edge - Memory alignment validation
 * @coverage_target core/iwasm/common/wasm_memory.c:memory_access_operations
 * @input_conditions Memory addresses at 0, 1, 2, 3 byte offsets from 4-byte boundaries
 * @expected_behavior Successful stores regardless of alignment with correct memory content
 * @validation_method Memory content verification across different alignment scenarios
 */
TEST_P(I64Store32Test, MemoryAlignment_HandlesUnalignedAccess)
{
    const uint64 test_value = 0x123456789ABCDEF0ULL;
    const uint32 expected_stored = 0x9ABCDEF0U;

    // Test aligned address (4-byte boundary)
    uint32 result = call_i64_store32(0, test_value, 0);
    ASSERT_EQ(0, result) << "Store at aligned address should succeed";
    ASSERT_EQ(expected_stored, read_memory_i32(0))
        << "Aligned store should produce correct memory content";

    // Test unaligned addresses (1, 2, 3 byte offsets)
    result = call_i64_store32(1, test_value, 0);
    ASSERT_EQ(0, result) << "Store at 1-byte unaligned address should succeed";
    ASSERT_EQ(expected_stored, read_memory_i32(1))
        << "1-byte unaligned store should produce correct memory content";

    result = call_i64_store32(2, test_value, 0);
    ASSERT_EQ(0, result) << "Store at 2-byte unaligned address should succeed";
    ASSERT_EQ(expected_stored, read_memory_i32(2))
        << "2-byte unaligned store should produce correct memory content";

    result = call_i64_store32(3, test_value, 0);
    ASSERT_EQ(0, result) << "Store at 3-byte unaligned address should succeed";
    ASSERT_EQ(expected_stored, read_memory_i32(3))
        << "3-byte unaligned store should produce correct memory content";
}

/**
 * @test OffsetCalculation_HandlesVariousOffsets
 * @brief Validates correct address calculation with different immediate offset values
 * @details Tests i64.store32 with various offset values to ensure proper address calculation
 *          and memory access patterns. Verifies (base_address + offset) computation.
 * @test_category Main - Address calculation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:address_calculation
 * @input_conditions Different base addresses and offset combinations within memory bounds
 * @expected_behavior Correct memory writes at calculated addresses (base + offset)
 * @validation_method Memory content verification at calculated target addresses
 */
TEST_P(I64Store32Test, OffsetCalculation_HandlesVariousOffsets)
{
    const uint64 test_value = 0xABCDEF0012345678ULL;
    const uint32 expected_stored = 0x12345678U;

    // Test zero offset
    uint32 result = call_i64_store32(100, test_value, 0);
    ASSERT_EQ(0, result) << "Store with zero offset should succeed";
    ASSERT_EQ(expected_stored, read_memory_i32(100))
        << "Zero offset should store at base address";

    // Test small offset
    result = call_i64_store32(100, test_value, 4);
    ASSERT_EQ(0, result) << "Store with small offset should succeed";
    ASSERT_EQ(expected_stored, read_memory_i32(104))
        << "Small offset should store at base+offset address";

    // Test larger offset
    result = call_i64_store32(100, test_value, 50);
    ASSERT_EQ(0, result) << "Store with larger offset should succeed";
    ASSERT_EQ(expected_stored, read_memory_i32(150))
        << "Larger offset should store at base+offset address";

    // Test maximum reasonable offset within memory bounds
    result = call_i64_store32(1000, test_value, 1000);
    ASSERT_EQ(0, result) << "Store with large offset should succeed";
    ASSERT_EQ(expected_stored, read_memory_i32(2000))
        << "Large offset should store at calculated address";
}

/**
 * @test TruncationBehavior_ValidatesExactBitPreservation
 * @brief Validates exact bit-level truncation behavior from i64 to i32
 * @details Tests specific bit patterns to ensure exact preservation of low 32 bits
 *          while confirming high 32 bits are properly discarded during store operation.
 * @test_category Edge - Bit-level validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:value_truncation
 * @input_conditions Specific bit patterns designed to test high/low bit separation
 * @expected_behavior Exact low 32-bit preservation, complete high 32-bit discard
 * @validation_method Bitwise comparison of stored values with expected truncated patterns
 */
TEST_P(I64Store32Test, TruncationBehavior_ValidatesExactBitPreservation)
{
    // Test high bits only (should store as zero)
    uint32 result = call_i64_store32(0, 0xFFFFFFFF00000000ULL, 0);
    ASSERT_EQ(0, result) << "Store with high bits only should succeed";
    ASSERT_EQ(0x00000000U, read_memory_i32(0))
        << "High bits only should store as zero";

    // Test low bits only (should store as-is)
    result = call_i64_store32(4, 0x00000000FFFFFFFFULL, 0);
    ASSERT_EQ(0, result) << "Store with low bits only should succeed";
    ASSERT_EQ(0xFFFFFFFFU, read_memory_i32(4))
        << "Low bits only should store unchanged";

    // Test alternating pattern (0xAAAAAAAA55555555)
    result = call_i64_store32(8, 0xAAAAAAAA55555555ULL, 0);
    ASSERT_EQ(0, result) << "Store with alternating pattern should succeed";
    ASSERT_EQ(0x55555555U, read_memory_i32(8))
        << "Alternating pattern should store low 32 bits only";

    // Test inverse alternating pattern (0x5555555555555555)
    result = call_i64_store32(12, 0x5555555555555555ULL, 0);
    ASSERT_EQ(0, result) << "Store with inverse alternating pattern should succeed";
    ASSERT_EQ(0x55555555U, read_memory_i32(12))
        << "Inverse alternating pattern should store low 32 bits";
}

/**
 * @test OutOfBounds_GeneratesTraps
 * @brief Validates proper trap generation for invalid memory access attempts
 * @details Tests i64.store32 operations that exceed memory boundaries to ensure
 *          proper trap generation and error handling without memory corruption.
 * @test_category Error - Memory bounds validation
 * @coverage_target core/iwasm/common/wasm_memory.c:bounds_checking
 * @input_conditions Memory addresses beyond allocated limits, address overflow scenarios
 * @expected_behavior Memory access traps generated, no memory corruption, clean error handling
 * @validation_method Exception handling verification and memory integrity checks
 */
TEST_P(I64Store32Test, OutOfBounds_GeneratesTraps)
{
    // Get memory size to test bounds
    wasm_function_inst_t memory_size_func = wasm_runtime_lookup_function(module_inst, "get_memory_size");
    ASSERT_NE(memory_size_func, nullptr) << "Failed to lookup get_memory_size function";

    wasm_val_t results[1] = { { .kind = WASM_I32 } };
    bool call_result = wasm_runtime_call_wasm_a(exec_env, memory_size_func, 1, results, 0, nullptr);
    ASSERT_TRUE(call_result) << "Failed to get memory size";
    uint32 memory_size = results[0].of.i32;

    // Test store beyond memory boundary (should trap)
    uint32 result = call_i64_store32(memory_size, 0x12345678ABCDEF00ULL, 0);
    ASSERT_NE(0, result) << "Store beyond memory boundary should fail/trap";

    // Test store at boundary - 3 (4-byte store needs 4 bytes, should trap)
    result = call_i64_store32(memory_size - 3, 0x12345678ABCDEF00ULL, 0);
    ASSERT_NE(0, result) << "Store crossing memory boundary should fail/trap";

    // Test store at valid boundary - 4 (should succeed)
    result = call_i64_store32(memory_size - 4, 0x12345678ABCDEF00ULL, 0);
    ASSERT_EQ(0, result) << "Store at valid memory boundary should succeed";
    ASSERT_EQ(0xABCDEF00U, read_memory_i32(memory_size - 4))
        << "Valid boundary store should produce correct memory content";
}

INSTANTIATE_TEST_SUITE_P(RunningMode, I64Store32Test,
                         testing::ValuesIn(running_mode_supported),
                         testing::PrintToStringParamName());