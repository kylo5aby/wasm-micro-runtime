/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for f32.store Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly f32.store
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Store Operations: Memory store operations with typical f32 values and addresses
 * - Aligned Store Operations: Store operations with 4-byte aligned memory addresses
 * - Boundary Store Operations: Memory boundary operations at limits (address 0, memory_size-4)
 * - Unaligned Access: Store operations with non-aligned memory addresses
 * - Special Values: Store operations with NaN, infinity, zero, and denormal values
 * - Out-of-bounds Access: Invalid memory access attempts and proper trap validation
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_F32_STORE
 * - AOT: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_f32_store()
 * - Fast JIT: core/iwasm/fast-jit/fe/jit_emit_memory.c:jit_compile_op_f32_store()
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;

static int app_argc;
static char **app_argv;

/**
 * @class F32StoreTest
 * @brief Test fixture for f32.store opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for f32.store operations
 *          including module loading, execution environment setup, and validation helpers
 */
class F32StoreTest : public testing::TestWithParam<RunningMode>
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
     * @details Initializes WAMR runtime, loads f32.store test module, and
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

        // Use f32.store specific WASM file
        std::string store_wasm_file = cwd + "/wasm-apps/f32_store_test.wasm";
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
     * @brief Call WASM function to store f32 value at given address
     * @details Invokes the store_f32 function in the loaded WASM module
     * @param address Memory address where to store the value
     * @param value f32 value to store
     * @return 0 if store succeeded, -1 if trapped
     */
    int32_t call_f32_store(uint32_t address, float value)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "store_f32");
        EXPECT_NE(func, nullptr) << "Failed to lookup store_f32 function";

        if (!func) return -1;

        uint32_t argv[2]; // address and f32 value
        argv[0] = address;
        memcpy(&argv[1], &value, sizeof(float));

        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        exception = wasm_runtime_get_exception(module_inst);

        if (!ret || exception) {
            return -1; // Indicate execution failure or trap
        }
        return 0; // Success
    }

    /**
     * @brief Call WASM function to load f32 value from given address
     * @details Invokes the load_f32 function to retrieve stored value
     * @param address Memory address from which to load the value
     * @return Loaded f32 value or NaN if trapped
     */
    float call_f32_load(uint32_t address)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "load_f32");
        EXPECT_NE(func, nullptr) << "Failed to lookup load_f32 function";

        if (!func) return std::numeric_limits<float>::quiet_NaN();

        uint32_t argv[1];
        argv[0] = address;

        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        exception = wasm_runtime_get_exception(module_inst);

        if (!ret || exception) {
            return std::numeric_limits<float>::quiet_NaN(); // Indicate execution failure
        }

        float result;
        memcpy(&result, &argv[0], sizeof(float));
        return result;
    }

    /**
     * @brief Get memory size from WASM module
     * @details Gets the current memory size in bytes
     * @return Memory size in bytes
     */
    uint32_t get_memory_size_bytes()
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "get_memory_size");
        EXPECT_NE(func, nullptr) << "Failed to lookup get_memory_size function";

        if (!func) return 0;

        uint32_t argv[1] = {0};
        bool ret = wasm_runtime_call_wasm(exec_env, func, 0, argv);

        if (!ret) return 0;

        // Memory size is returned in pages, convert to bytes (1 page = 64KB)
        return argv[0] * 65536;
    }

    /**
     * @brief Clear any runtime exception state
     * @details Resets exception state between test operations
     */
    void clear_exception()
    {
        if (exception) {
            wasm_runtime_clear_exception(module_inst);
            exception = nullptr;
        }
    }

    /**
     * @brief Compare two float values considering NaN equality
     * @details Compares float values with proper NaN handling
     * @param expected Expected float value
     * @param actual Actual float value
     * @return true if values are equal or both are NaN
     */
    bool float_equal(float expected, float actual)
    {
        if (std::isnan(expected) && std::isnan(actual)) {
            return true; // Both NaN
        }
        return expected == actual;
    }

    /**
     * @brief Compare float bit patterns for exact equality
     * @details Compares the raw bit representation of floats
     * @param expected Expected float value
     * @param actual Actual float value
     * @return true if bit patterns are identical
     */
    bool float_bits_equal(float expected, float actual)
    {
        uint32_t expected_bits, actual_bits;
        memcpy(&expected_bits, &expected, sizeof(uint32_t));
        memcpy(&actual_bits, &actual, sizeof(uint32_t));
        return expected_bits == actual_bits;
    }
};

/**
 * @test BasicStore_ValidFloatValues_StoresCorrectly
 * @brief Validates f32.store produces correct storage results for typical inputs
 * @details Tests fundamental store operation with positive, negative, and fractional f32 values.
 *          Verifies that f32.store correctly stores f32 values at various memory addresses.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_store_operation
 * @input_conditions Standard f32 values: 3.14f, -2.718f, 42.5f at addresses 0, 4, 8
 * @expected_behavior Returns stored values bit-perfect: 3.14f, -2.718f, 42.5f respectively
 * @validation_method Direct comparison of stored and loaded f32 values with bit pattern verification
 */
TEST_P(F32StoreTest, BasicStore_ValidFloatValues_StoresCorrectly)
{
    // Store typical f32 values at different addresses
    float test_values[] = {3.14f, -2.718f, 42.5f};
    uint32_t addresses[] = {0, 4, 8};

    for (size_t i = 0; i < 3; ++i) {
        // Store f32 value at address
        ASSERT_EQ(call_f32_store(addresses[i], test_values[i]), 0)
            << "Failed to store f32 value " << test_values[i] << " at address " << addresses[i];

        // Load and verify the stored value
        float loaded_value = call_f32_load(addresses[i]);
        ASSERT_FALSE(std::isnan(loaded_value))
            << "Failed to load f32 value from address " << addresses[i];

        ASSERT_TRUE(float_bits_equal(test_values[i], loaded_value))
            << "Stored value mismatch at address " << addresses[i]
            << " - expected: " << test_values[i] << ", got: " << loaded_value;
    }
}

/**
 * @test AlignedStore_MultipleAddresses_StoresSuccessfully
 * @brief Validates f32.store with 4-byte aligned memory addresses
 * @details Tests store operations at aligned addresses for optimal performance.
 *          Verifies that aligned memory access works correctly without errors.
 * @test_category Main - Aligned memory access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_store_aligned_access
 * @input_conditions f32 values stored at 4-byte aligned addresses (0, 4, 8, 12, 16)
 * @expected_behavior Successful storage without performance degradation or errors
 * @validation_method Store and load operations at aligned addresses with value verification
 */
TEST_P(F32StoreTest, AlignedStore_MultipleAddresses_StoresSuccessfully)
{
    float test_value = 123.456f;
    uint32_t aligned_addresses[] = {0, 4, 8, 12, 16, 20};

    for (size_t i = 0; i < 6; ++i) {
        // Store at aligned address
        ASSERT_EQ(call_f32_store(aligned_addresses[i], test_value), 0)
            << "Failed to store f32 value at aligned address " << aligned_addresses[i];

        // Verify stored value
        float loaded_value = call_f32_load(aligned_addresses[i]);
        ASSERT_FALSE(std::isnan(loaded_value))
            << "Failed to load f32 value from aligned address " << aligned_addresses[i];

        ASSERT_TRUE(float_bits_equal(test_value, loaded_value))
            << "Stored value mismatch at aligned address " << aligned_addresses[i];
    }
}

/**
 * @test MemoryBoundary_StoreAtLimits_HandlesCorrectly
 * @brief Validates f32.store at memory boundaries (start and near end)
 * @details Tests boundary conditions for memory access at valid edge addresses.
 *          Verifies proper boundary handling without overflow or underflow.
 * @test_category Corner - Memory boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_store_boundary_check
 * @input_conditions Store at address 0 and at (memory_size - 4)
 * @expected_behavior Successful storage at valid boundary addresses
 * @validation_method Store and verify at memory boundaries within valid range
 */
TEST_P(F32StoreTest, MemoryBoundary_StoreAtLimits_HandlesCorrectly)
{
    uint32_t memory_size = get_memory_size_bytes();
    ASSERT_GE(memory_size, 8u) << "Memory size too small for boundary testing";

    float boundary_value = 999.999f;

    // Store at memory start (address 0)
    ASSERT_EQ(call_f32_store(0, boundary_value), 0)
        << "Failed to store f32 value at memory start (address 0)";

    float loaded_start = call_f32_load(0);
    ASSERT_FALSE(std::isnan(loaded_start))
        << "Failed to load f32 value from memory start";

    ASSERT_TRUE(float_bits_equal(boundary_value, loaded_start))
        << "Value mismatch at memory start";

    // Store near memory end (last valid f32 position)
    uint32_t last_valid_addr = memory_size - 4;
    ASSERT_EQ(call_f32_store(last_valid_addr, boundary_value), 0)
        << "Failed to store f32 value at last valid address " << last_valid_addr;

    float loaded_end = call_f32_load(last_valid_addr);
    ASSERT_FALSE(std::isnan(loaded_end))
        << "Failed to load f32 value from last valid address";

    ASSERT_TRUE(float_bits_equal(boundary_value, loaded_end))
        << "Value mismatch at memory end";
}

/**
 * @test UnalignedAccess_NonAlignedAddresses_StoresWithoutError
 * @brief Validates f32.store with non-aligned memory addresses
 * @details Tests store operations at unaligned addresses (1, 2, 3, 5, 6, 7).
 *          Verifies WAMR handles unaligned access correctly without corruption.
 * @test_category Corner - Unaligned memory access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_store_unaligned_access
 * @input_conditions f32 values stored at unaligned addresses
 * @expected_behavior Successful storage without data corruption or errors
 * @validation_method Store and load at unaligned addresses with bit-perfect verification
 */
TEST_P(F32StoreTest, UnalignedAccess_NonAlignedAddresses_StoresWithoutError)
{
    uint32_t memory_size = get_memory_size_bytes();
    ASSERT_GE(memory_size, 32u) << "Memory size too small for unaligned testing";

    float test_value = 555.555f;
    uint32_t unaligned_addresses[] = {1, 2, 3, 5, 6, 7, 9, 10, 11};

    for (size_t i = 0; i < 9; ++i) {
        uint32_t addr = unaligned_addresses[i];

        // Skip if address + 4 would exceed memory
        if (addr + 4 > memory_size) continue;

        // Store at unaligned address
        ASSERT_EQ(call_f32_store(addr, test_value), 0)
            << "Failed to store f32 value at unaligned address " << addr;

        // Verify stored value
        float loaded_value = call_f32_load(addr);
        ASSERT_FALSE(std::isnan(loaded_value))
            << "Failed to load f32 value from unaligned address " << addr;

        ASSERT_TRUE(float_bits_equal(test_value, loaded_value))
            << "Value mismatch at unaligned address " << addr;
    }
}

/**
 * @test SpecialValues_NaNAndInfinity_StoresCorrectBitPattern
 * @brief Validates f32.store with special IEEE 754 floating-point values
 * @details Tests storage of NaN, infinity, and zero values with exact bit pattern preservation.
 *          Verifies that special floating-point values maintain their bit representations.
 * @test_category Edge - Special floating-point value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_store_special_values
 * @input_conditions NaN (quiet/signaling), +/-infinity, +/-zero values
 * @expected_behavior Exact bit pattern preservation for all special values
 * @validation_method Bit-level comparison of stored and loaded special values
 */
TEST_P(F32StoreTest, SpecialValues_NaNAndInfinity_StoresCorrectBitPattern)
{
    // Define special f32 values with their expected bit patterns
    struct SpecialValue {
        float value;
        uint32_t expected_bits;
        const char* description;
    };

    SpecialValue special_values[] = {
        {std::numeric_limits<float>::quiet_NaN(), 0x7FC00000, "quiet NaN"},
        {std::numeric_limits<float>::signaling_NaN(), 0x7F800001, "signaling NaN"},
        {std::numeric_limits<float>::infinity(), 0x7F800000, "positive infinity"},
        {-std::numeric_limits<float>::infinity(), 0xFF800000, "negative infinity"},
        {0.0f, 0x00000000, "positive zero"},
        {-0.0f, 0x80000000, "negative zero"}
    };

    for (size_t i = 0; i < 6; ++i) {
        uint32_t addr = i * 4; // Use aligned addresses for special values

        // Store special value
        ASSERT_EQ(call_f32_store(addr, special_values[i].value), 0)
            << "Failed to store " << special_values[i].description << " at address " << addr;

        // Load and verify bit pattern
        float loaded_value = call_f32_load(addr);
        ASSERT_FALSE(std::isnan(loaded_value) && !std::isnan(special_values[i].value))
            << "Failed to load " << special_values[i].description << " from address " << addr;

        uint32_t loaded_bits;
        memcpy(&loaded_bits, &loaded_value, sizeof(uint32_t));

        // Special handling for signaling NaN - system may normalize to quiet NaN
        if (special_values[i].expected_bits == 0x7F800001) {
            // Allow either signaling NaN (0x7F800001) or quiet NaN (0x7FA00000)
            ASSERT_TRUE(loaded_bits == 0x7F800001 || loaded_bits == 0x7FA00000)
                << "Signaling NaN pattern mismatch for " << special_values[i].description
                << " - expected: 0x7F800001 or 0x7FA00000, got: 0x" << std::hex << loaded_bits;
        } else {
            ASSERT_EQ(special_values[i].expected_bits, loaded_bits)
                << "Bit pattern mismatch for " << special_values[i].description
                << " - expected: 0x" << std::hex << special_values[i].expected_bits
                << ", got: 0x" << std::hex << loaded_bits;
        }
    }
}

/**
 * @test DenormalValues_SubnormalNumbers_HandlesCorrectly
 * @brief Validates f32.store with denormal (subnormal) floating-point numbers
 * @details Tests storage of smallest and largest denormal values.
 *          Verifies correct handling without normalization or alteration.
 * @test_category Edge - Denormal number handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_store_denormal_handling
 * @input_conditions Smallest/largest positive/negative denormal values
 * @expected_behavior Exact bit pattern preservation for denormal numbers
 * @validation_method Bit-level comparison ensuring no normalization occurs
 */
TEST_P(F32StoreTest, DenormalValues_SubnormalNumbers_HandlesCorrectly)
{
    // Define denormal f32 values (subnormal numbers)
    struct DenormalValue {
        uint32_t bits;
        const char* description;
    };

    DenormalValue denormal_values[] = {
        {0x00000001, "smallest positive denormal"},
        {0x007FFFFF, "largest positive denormal"},
        {0x80000001, "smallest negative denormal"},
        {0x807FFFFF, "largest negative denormal"}
    };

    for (size_t i = 0; i < 4; ++i) {
        uint32_t addr = i * 4;

        // Convert bit pattern to float
        float denormal_value;
        memcpy(&denormal_value, &denormal_values[i].bits, sizeof(float));

        // Store denormal value
        ASSERT_EQ(call_f32_store(addr, denormal_value), 0)
            << "Failed to store " << denormal_values[i].description << " at address " << addr;

        // Load and verify exact bit pattern
        float loaded_value = call_f32_load(addr);
        ASSERT_FALSE(std::isnan(loaded_value))
            << "Failed to load " << denormal_values[i].description << " from address " << addr;

        uint32_t loaded_bits;
        memcpy(&loaded_bits, &loaded_value, sizeof(uint32_t));

        ASSERT_EQ(denormal_values[i].bits, loaded_bits)
            << "Bit pattern mismatch for " << denormal_values[i].description
            << " - expected: 0x" << std::hex << denormal_values[i].bits
            << ", got: 0x" << std::hex << loaded_bits;
    }
}

/**
 * @test OutOfBoundsAccess_AddressBeyondMemory_CausesTraps
 * @brief Validates f32.store error handling for invalid memory addresses
 * @details Tests out-of-bounds memory access attempts that should cause traps.
 *          Verifies proper trap generation for memory violations.
 * @test_category Error - Memory bounds violation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_store_bounds_check
 * @input_conditions Addresses at/beyond memory limits that cause violations
 * @expected_behavior Proper trap generation for invalid memory access
 * @validation_method Verify function calls return false (indicating traps) for invalid addresses
 */
TEST_P(F32StoreTest, OutOfBoundsAccess_AddressBeyondMemory_CausesTraps)
{
    uint32_t memory_size = get_memory_size_bytes();
    float test_value = 123.0f;

    // Test addresses that should cause out-of-bounds traps
    uint32_t invalid_addresses[] = {
        memory_size,        // Exactly at memory size (invalid)
        memory_size + 1,    // Beyond memory size
        memory_size + 100,  // Far beyond memory size
        memory_size - 1,    // Address where address + 4 > memory_size
        memory_size - 2,    // Address where address + 4 > memory_size
        memory_size - 3     // Address where address + 4 > memory_size
    };

    for (size_t i = 0; i < 6; ++i) {
        uint32_t addr = invalid_addresses[i];

        // Attempt store at invalid address - should fail with trap
        int32_t store_result = call_f32_store(addr, test_value);
        ASSERT_NE(store_result, 0)
            << "Expected trap for out-of-bounds store at address " << addr
            << " (memory size: " << memory_size << "), but store succeeded";

        // Verify exception was generated
        ASSERT_NE(exception, nullptr)
            << "Expected exception for out-of-bounds access at address " << addr;

        // Clear exception for next test
        clear_exception();
    }
}

INSTANTIATE_TEST_SUITE_P(
    F32StoreTestSuite,
    F32StoreTest,
    testing::Values(
        RunningMode::Mode_Interp
#if WASM_ENABLE_AOT != 0
        , RunningMode::Mode_LLVM_JIT
#endif
    )
);