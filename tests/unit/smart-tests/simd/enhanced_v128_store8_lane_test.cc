/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @brief Test fixture for v128.store8_lane opcode validation
 * @details Comprehensive testing of v128.store8_lane instruction.
 *          Validates lane extraction, memory storage, boundary conditions, and error handling.
 */
class V128Store8LaneTestSuite : public testing::Test {
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.store8_lane testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.store8_lane test module using absolute path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_store8_lane_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.store8_lane tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Helper function to call v128.store8_lane WASM function
     * @param func_name Name of the exported WASM function
     * @param address Memory address for store operation
     * @param lane Lane index (0-15)
     * @param expected Expected value to store
     * @return bool True if operation succeeded, false on error
     */
    bool call_v128_store8_lane(const char *func_name, uint32_t address, uint32_t lane, uint32_t expected)
    {
        // Prepare arguments: address, lane, expected_value
        uint32_t argv[4] = {address, lane, expected, 0};

        // Call WASM function with 3 inputs, returns i32 (success indicator)
        bool call_success = dummy_env->execute(func_name, 3, argv);
        if (!call_success) {
            return false;
        }

        // Return result (1 for success, 0 for failure)
        return argv[0] == 1;
    }
};

/**
 * @test BasicStore_MultipleLanes_StoresCorrectValues
 * @brief Validates v128.store8_lane correctly stores 8-bit values from different vector lanes to memory
 * @details Tests basic store functionality with various lane indices and memory addresses.
 *          Verifies that the correct 8-bit value is extracted from each specified lane and stored to memory.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:v128_store8_lane_operation
 * @input_conditions v128 vector with known lane values, various memory addresses and lane indices
 * @expected_behavior Extracts correct 8-bit value from specified lane and stores to target memory address
 * @validation_method Direct memory comparison of stored values with expected lane values
 */
TEST_F(V128Store8LaneTestSuite, BasicStore_MultipleLanes_StoresCorrectValues) {
    // Test lane 0 with value 0x42 to memory address 0
    bool result = call_v128_store8_lane("test_store8_lane", 0, 0, 0x42);
    ASSERT_TRUE(result) << "Store to lane 0 with value 0x42 failed";

    // Test lane 7 with value 0x55 to memory address 64
    result = call_v128_store8_lane("test_store8_lane", 64, 7, 0x55);
    ASSERT_TRUE(result) << "Store to lane 7 with value 0x55 failed";

    // Test lane 8 with value 0xAA to memory address 128
    result = call_v128_store8_lane("test_store8_lane", 128, 8, 0xAA);
    ASSERT_TRUE(result) << "Store to lane 8 with value 0xAA failed";

    // Test lane 15 with value 0xFF to memory address 256
    result = call_v128_store8_lane("test_store8_lane", 256, 15, 0xFF);
    ASSERT_TRUE(result) << "Store to lane 15 with value 0xFF failed";
}

/**
 * @test MemoryBoundaries_ValidAddresses_StoresSuccessfully
 * @brief Validates v128.store8_lane operations at memory boundary locations
 * @details Tests store operations at first byte, last valid byte, and addresses with large offsets.
 *          Ensures boundary memory addresses are handled correctly without traps.
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/common/wasm_memory.c:memory_boundary_validation
 * @input_conditions Memory addresses at 0, memory_size-1, and large offset scenarios
 * @expected_behavior Successful stores without memory access violations or traps
 * @validation_method Verify store operations complete successfully at boundary addresses
 */
TEST_F(V128Store8LaneTestSuite, MemoryBoundaries_ValidAddresses_StoresSuccessfully) {
    // Store to memory address 0 (first byte)
    bool result = call_v128_store8_lane("test_store8_lane", 0, 0, 0x42);
    ASSERT_TRUE(result) << "Store to memory address 0 failed";

    // Store to high memory address within bounds
    result = call_v128_store8_lane("test_store8_lane", 65535, 1, 0x55);
    ASSERT_TRUE(result) << "Store to high memory address failed";

    // Test with memory offset scenarios
    result = call_v128_store8_lane("test_store8_lane_with_offset", 100, 3, 0xAA);
    ASSERT_TRUE(result) << "Store with memory offset failed";
}

/**
 * @test EdgePatterns_ExtremeValues_HandlesCorrectly
 * @brief Validates v128.store8_lane with extreme 8-bit values and special bit patterns
 * @details Tests zero values, maximum values, bit patterns, and incremental patterns.
 *          Verifies accurate storage of edge case 8-bit values from vector lanes.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_lane_extraction
 * @input_conditions Zero values (0x00), maximum values (0xFF), bit patterns (0x55, 0xAA, 0x01, 0x80)
 * @expected_behavior Accurate extraction and storage of all 8-bit value patterns
 * @validation_method Memory verification of stored patterns matches expected lane values
 */
TEST_F(V128Store8LaneTestSuite, EdgePatterns_ExtremeValues_HandlesCorrectly) {
    // Test zero value (0x00)
    bool result = call_v128_store8_lane("test_store8_lane", 0, 0, 0x00);
    ASSERT_TRUE(result) << "Store of zero value (0x00) failed";

    // Test maximum value (0xFF)
    result = call_v128_store8_lane("test_store8_lane", 16, 1, 0xFF);
    ASSERT_TRUE(result) << "Store of maximum value (0xFF) failed";

    // Test alternating bit patterns
    result = call_v128_store8_lane("test_store8_lane", 32, 2, 0x55); // 01010101
    ASSERT_TRUE(result) << "Store of bit pattern 0x55 failed";

    result = call_v128_store8_lane("test_store8_lane", 48, 3, 0xAA); // 10101010
    ASSERT_TRUE(result) << "Store of bit pattern 0xAA failed";

    // Test power of 2 values
    result = call_v128_store8_lane("test_store8_lane", 64, 4, 0x01);
    ASSERT_TRUE(result) << "Store of power of 2 value 0x01 failed";

    result = call_v128_store8_lane("test_store8_lane", 80, 5, 0x80);
    ASSERT_TRUE(result) << "Store of power of 2 value 0x80 failed";
}

/**
 * @test MemoryAlignment_UnalignedAccess_WorksCorrectly
 * @brief Validates v128.store8_lane operations with unaligned memory addresses
 * @details Tests store operations to odd memory addresses and various offset combinations.
 *          Ensures 8-bit stores work correctly regardless of address alignment.
 * @test_category Corner - Memory alignment validation
 * @coverage_target core/iwasm/common/wasm_memory.c:unaligned_memory_access
 * @input_conditions Odd memory addresses (1, 3, 5, 7) and various memarg offset values
 * @expected_behavior Successful 8-bit stores regardless of address alignment
 * @validation_method Verify store operations complete successfully for all alignment scenarios
 */
TEST_F(V128Store8LaneTestSuite, MemoryAlignment_UnalignedAccess_WorksCorrectly) {
    // Test unaligned addresses (odd addresses)
    bool result = call_v128_store8_lane("test_store8_lane", 1, 0, 0x42);
    ASSERT_TRUE(result) << "Store to unaligned address 1 failed";

    result = call_v128_store8_lane("test_store8_lane", 3, 1, 0x55);
    ASSERT_TRUE(result) << "Store to unaligned address 3 failed";

    result = call_v128_store8_lane("test_store8_lane", 5, 2, 0xAA);
    ASSERT_TRUE(result) << "Store to unaligned address 5 failed";

    result = call_v128_store8_lane("test_store8_lane", 7, 3, 0xFF);
    ASSERT_TRUE(result) << "Store to unaligned address 7 failed";

    // Test with memarg offset combinations
    result = call_v128_store8_lane("test_store8_lane_with_offset", 100, 4, 0x80);
    ASSERT_TRUE(result) << "Store with offset to unaligned base failed";
}

/**
 * @test OutOfBounds_InvalidAddresses_TriggersTraps
 * @brief Validates v128.store8_lane properly handles out-of-bounds memory access
 * @details Tests store attempts beyond memory limits and address overflow scenarios.
 *          Verifies proper trap behavior and error handling for invalid memory access.
 * @test_category Error - Out-of-bounds validation
 * @coverage_target core/iwasm/common/wasm_memory.c:memory_bounds_checking
 * @input_conditions Memory addresses beyond memory_size and large offset causing overflow
 * @expected_behavior Memory access traps triggered for invalid addresses, proper error handling
 * @validation_method Verify trap conditions are properly detected and handled
 */
TEST_F(V128Store8LaneTestSuite, OutOfBounds_InvalidAddresses_TriggersTraps) {
    // Test store beyond memory size (should trigger trap)
    bool result = call_v128_store8_lane("test_store8_lane_bounds", 100000, 0, 0x42);
    ASSERT_FALSE(result) << "Expected trap for out-of-bounds memory access";

    // Test large offset causing overflow
    result = call_v128_store8_lane("test_store8_lane_large_offset", 65530, 1, 0x55);
    ASSERT_FALSE(result) << "Expected trap for address overflow scenario";
}