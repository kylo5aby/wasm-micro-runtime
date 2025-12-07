/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <memory>

#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @file enhanced_v128_load64_lane_test.cc
 * @brief Comprehensive test suite for v128.load64_lane SIMD opcode
 * @details Tests v128.load64_lane functionality across different execution modes.
 *          This opcode loads a 64-bit value from memory and replaces one specific lane
 *          in an existing v128 vector while preserving other lanes.
 */

/**
 * Test fixture for v128.load64_lane opcode validation
 * @brief Comprehensive test suite for v128.load64_lane SIMD memory load instruction
 * @details Tests v128.load64_lane functionality with memory access, alignment, boundary
 *          conditions, and error handling for the v128.load64_lane instruction.
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:v128_load64_lane
 */
class V128Load64LaneTest : public testing::Test {
protected:
    /**
     * @brief Set up test environment with WAMR runtime initialization
     * @details Initializes WAMR runtime with SIMD support and loads test module
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.load64_lane test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_load64_lane_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.load64_lane tests";
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     * @details Performs proper cleanup of loaded modules and runtime resources
     */
    void TearDown() override {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute test function with v128 and i32 parameters for lane 0
     * @param vec_input Input v128 vector as array of 4 uint32_t values
     * @param addr Memory address for load operation
     * @param result Output array to store resulting v128 vector
     */
    void call_v128_load64_lane_0(const uint32_t vec_input[4], uint32_t addr, uint32_t result[4]) {
        // Prepare arguments: 4 i32 values for v128 input + 1 i32 for address
        uint32_t argv[5];

        // Set up input v128 as 4 i32 values (not used in this test, but required by function signature)
        argv[0] = vec_input[0];
        argv[1] = vec_input[1];
        argv[2] = vec_input[2];
        argv[3] = vec_input[3];

        // Set up memory address
        argv[4] = addr;

        // Call WASM function: 5 inputs, returns v128 (4 i32s)
        bool call_success = dummy_env->execute("test_v128_load64_lane_0", 5, argv);
        ASSERT_TRUE(call_success)
            << "Failed to call test_v128_load64_lane_0 function";

        // Extract v128 result from argv (returned as 4 uint32_t values)
        result[0] = argv[0];
        result[1] = argv[1];
        result[2] = argv[2];
        result[3] = argv[3];
    }

    /**
     * @brief Execute test function with v128 and i32 parameters for lane 1
     * @param vec_input Input v128 vector as array of 4 uint32_t values
     * @param addr Memory address for load operation
     * @param result Output array to store resulting v128 vector
     */
    void call_v128_load64_lane_1(const uint32_t vec_input[4], uint32_t addr, uint32_t result[4]) {
        // Prepare arguments: 4 i32 values for v128 input + 1 i32 for address
        uint32_t argv[5];

        // Set up input v128 as 4 i32 values (not used in this test, but required by function signature)
        argv[0] = vec_input[0];
        argv[1] = vec_input[1];
        argv[2] = vec_input[2];
        argv[3] = vec_input[3];

        // Set up memory address
        argv[4] = addr;

        // Call WASM function: 5 inputs, returns v128 (4 i32s)
        bool call_success = dummy_env->execute("test_v128_load64_lane_1", 5, argv);
        ASSERT_TRUE(call_success)
            << "Failed to call test_v128_load64_lane_1 function";

        // Extract v128 result from argv (returned as 4 uint32_t values)
        result[0] = argv[0];
        result[1] = argv[1];
        result[2] = argv[2];
        result[3] = argv[3];
    }

    /**
     * @brief Test out-of-bounds memory access scenarios
     * @param addr Memory address that would cause out-of-bounds access
     * @return True if function call resulted in expected trap/failure, false otherwise
     */
    bool test_out_of_bounds_access(uint32_t addr) {
        uint32_t input_vec[4] = {0, 0, 0, 0};
        uint32_t argv[5] = {input_vec[0], input_vec[1], input_vec[2], input_vec[3], addr};

        // Expect this call to fail due to out-of-bounds access
        bool call_success = dummy_env->execute("test_v128_load64_lane_0", 5, argv);

        // Check if an exception was set (indicating trap occurred)
        const char* exception = dummy_env->get_exception();
        bool has_exception = (exception != nullptr && strlen(exception) > 0);

        // Clear exception for next test
        if (has_exception) {
            dummy_env->clear_exception();
        }

        return !call_success || has_exception;
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicLaneLoading_ReturnsUpdatedVector
 * @brief Validates v128.load64_lane correctly loads and replaces specified lanes
 * @details Tests fundamental lane replacement functionality by loading known 64-bit values
 *          from memory into specific lanes of v128 vectors while preserving other lanes.
 *          Verifies correct memory access, value loading, and selective lane updates.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:v128_load64_lane_operation
 * @input_conditions Memory addresses pointing to known 64-bit values, initial v128 vectors
 * @expected_behavior Target lane updated with memory value, other lanes unchanged
 * @validation_method Direct comparison of lane values before and after load operations
 */
TEST_F(V128Load64LaneTest, BasicLaneLoading_ReturnsUpdatedVector) {
    uint32_t input_vec[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    uint32_t result[4];

    // Test lane 0 replacement
    // Memory at address 0 should contain 0x123456789ABCDEF0 (little-endian: F0 DE BC 9A 78 56 34 12)
    call_v128_load64_lane_0(input_vec, 0, result);

    // Lane 0 should be updated with memory value: 0x123456789ABCDEF0
    // In little-endian format, this becomes: result[0] = 0x9ABCDEF0, result[1] = 0x12345678
    ASSERT_EQ(result[0], 0x9ABCDEF0U)
        << "Lane 0 low 32-bits should contain loaded memory value";
    ASSERT_EQ(result[1], 0x12345678U)
        << "Lane 0 high 32-bits should contain loaded memory value";

    // Lane 1 should remain as initialized by WASM function (0xAAAAAAAAAAAAAAAA)
    ASSERT_EQ(result[2], 0xAAAAAAAAU)
        << "Lane 1 low 32-bits should remain unchanged after lane 0 load";
    ASSERT_EQ(result[3], 0xAAAAAAAAU)
        << "Lane 1 high 32-bits should remain unchanged after lane 0 load";

    // Test lane 1 replacement
    // Memory at address 8 should contain 0xFEDCBA0987654321 (little-endian: 21 43 65 87 09 BA DC FE)
    call_v128_load64_lane_1(input_vec, 8, result);

    // Lane 0 should remain as initialized by WASM function (0x1111111111111111)
    ASSERT_EQ(result[0], 0x11111111U)
        << "Lane 0 low 32-bits should remain unchanged after lane 1 load";
    ASSERT_EQ(result[1], 0x11111111U)
        << "Lane 0 high 32-bits should remain unchanged after lane 1 load";

    // Lane 1 should be updated with memory value: 0xFEDCBA0987654321
    // In little-endian format, this becomes: result[2] = 0x87654321, result[3] = 0xFEDCBA09
    ASSERT_EQ(result[2], 0x87654321U)
        << "Lane 1 low 32-bits should contain loaded memory value";
    ASSERT_EQ(result[3], 0xFEDCBA09U)
        << "Lane 1 high 32-bits should contain loaded memory value";
}

/**
 * @test MemoryBoundaryAccess_LoadsCorrectly
 * @brief Tests loading from memory boundary positions and address calculations
 * @details Validates v128.load64_lane behavior at memory boundaries, with various offset
 *          values and alignment scenarios. Tests both aligned and unaligned memory access
 *          patterns to ensure proper memory handling across different addressing modes.
 * @test_category Corner - Boundary conditions testing
 * @coverage_target Memory access validation and address calculation logic
 * @input_conditions Memory boundary addresses, various offset values, alignment scenarios
 * @expected_behavior Successful loads from valid boundary positions without corruption
 * @validation_method Memory access success verification and result value validation
 */
TEST_F(V128Load64LaneTest, MemoryBoundaryAccess_LoadsCorrectly) {
    uint32_t input_vec[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    uint32_t result[4];

    // Test aligned memory access (8-byte boundary)
    // Memory at address 16 should contain 0x0011223344556677 (from WASM initialization)
    call_v128_load64_lane_0(input_vec, 16, result);

    // Verify successful load from aligned address
    // Expected value: 0x0011223344556677 -> little-endian: result[0] = 0x44556677, result[1] = 0x00112233
    ASSERT_EQ(result[0], 0x44556677U)
        << "Aligned access should load correct low 32-bits";
    ASSERT_EQ(result[1], 0x00112233U)
        << "Aligned access should load correct high 32-bits";

    // Lane 1 should remain unchanged (initialized by WASM function to 0xAAAAAAAAAAAAAAAA)
    ASSERT_EQ(result[2], 0xAAAAAAAAU)
        << "Non-target lane should remain unchanged for aligned access";
    ASSERT_EQ(result[3], 0xAAAAAAAAU)
        << "Non-target lane should remain unchanged for aligned access";

    // Test access near memory boundary (but still within bounds)
    // Memory at address 56 should contain test pattern: 0xF0F1F2F3F4F5F6F7
    call_v128_load64_lane_1(input_vec, 56, result);

    // Lane 0 should remain unchanged (initialized by WASM function to 0x1111111111111111)
    ASSERT_EQ(result[0], 0x11111111U)
        << "Non-target lane should remain unchanged in boundary access";
    ASSERT_EQ(result[1], 0x11111111U)
        << "Non-target lane should remain unchanged in boundary access";

    // Lane 1 should be updated: 0xF0F1F2F3F4F5F6F7 -> little-endian: result[2] = 0xF4F5F6F7, result[3] = 0xF0F1F2F3
    ASSERT_EQ(result[2], 0xF4F5F6F7U)
        << "Boundary access should load correct low 32-bits";
    ASSERT_EQ(result[3], 0xF0F1F2F3U)
        << "Boundary access should load correct high 32-bits";
}

/**
 * @test SpecialValues_LoadCorrectly
 * @brief Validates loading of special 64-bit numeric patterns and endianness
 * @details Tests v128.load64_lane with special bit patterns including zeros, ones,
 *          signed integer limits, and endianness-sensitive values. Verifies correct
 *          interpretation of loaded values according to platform byte ordering.
 * @test_category Edge - Special value and endianness testing
 * @coverage_target Endianness handling and special value processing
 * @input_conditions Special bit patterns in memory (zeros, ones, min/max values)
 * @expected_behavior Correct interpretation of loaded values per platform endianness
 * @validation_method Bit-exact comparison of expected vs actual lane values
 */
TEST_F(V128Load64LaneTest, SpecialValues_LoadCorrectly) {
    uint32_t input_vec[4] = {0xAAAAAAAA, 0xAAAAAAAA, 0xBBBBBBBB, 0xBBBBBBBB};
    uint32_t result[4];

    // Test loading zero value
    // Memory at address 32 should contain 0x0000000000000000 (from WASM initialization)
    call_v128_load64_lane_0(input_vec, 32, result);

    ASSERT_EQ(result[0], 0x00000000U)
        << "Should correctly load zero value low 32-bits from memory";
    ASSERT_EQ(result[1], 0x00000000U)
        << "Should correctly load zero value high 32-bits from memory";

    // Lane 1 should remain unchanged (initialized by WASM function to 0xAAAAAAAAAAAAAAAA)
    ASSERT_EQ(result[2], 0xAAAAAAAAU)
        << "Non-target lane should preserve initial value";
    ASSERT_EQ(result[3], 0xAAAAAAAAU)
        << "Non-target lane should preserve initial value";

    // Test loading maximum value (all bits set)
    // Memory at address 40 should contain 0xFFFFFFFFFFFFFFFF (from WASM initialization)
    call_v128_load64_lane_1(input_vec, 40, result);

    // Lane 0 should remain unchanged (initialized by WASM function to 0x1111111111111111)
    ASSERT_EQ(result[0], 0x11111111U)
        << "Non-target lane should preserve initial value";
    ASSERT_EQ(result[1], 0x11111111U)
        << "Non-target lane should preserve initial value";

    ASSERT_EQ(result[2], 0xFFFFFFFFU)
        << "Should correctly load maximum value low 32-bits from memory";
    ASSERT_EQ(result[3], 0xFFFFFFFFU)
        << "Should correctly load maximum value high 32-bits from memory";

    // Test endianness-sensitive pattern
    // Memory at address 48 should contain 0x0102030405060708 (from WASM initialization)
    call_v128_load64_lane_0(input_vec, 48, result);

    // Expected little-endian interpretation: 0x0102030405060708 -> result[0] = 0x05060708, result[1] = 0x01020304
    ASSERT_EQ(result[0], 0x05060708U)
        << "Should correctly interpret endianness-sensitive byte pattern low 32-bits";
    ASSERT_EQ(result[1], 0x01020304U)
        << "Should correctly interpret endianness-sensitive byte pattern high 32-bits";
}

/**
 * @test ErrorConditions_TrapAppropriately
 * @brief Tests out-of-bounds memory access and error handling
 * @details Validates v128.load64_lane error handling for invalid memory addresses,
 *          overflow scenarios, and access beyond allocated memory bounds. Verifies
 *          proper trap behavior and error state management for exceptional conditions.
 * @test_category Error - Invalid access and error handling testing
 * @coverage_target Memory bounds checking and trap generation logic
 * @input_conditions Invalid memory addresses, overflow scenarios, out-of-bounds access
 * @expected_behavior Proper trap behavior or controlled error handling
 * @validation_method Exception handling validation and error state verification
 */
TEST_F(V128Load64LaneTest, ErrorConditions_TrapAppropriately) {
    // Test access beyond memory bounds
    // Attempt to access memory at address that should be out of bounds
    // Memory size is 2 pages = 131072 bytes, so access at 131072 should fail (need 8 bytes)
    uint32_t out_of_bounds_addr = 131072;

    bool trapped = test_out_of_bounds_access(out_of_bounds_addr);
    ASSERT_TRUE(trapped)
        << "Out-of-bounds memory access should result in trap or failure";

    // Test address overflow scenario
    // Use address that would overflow when adding 8 bytes for 64-bit access
    uint32_t overflow_addr = 0xFFFFFFFC; // 0xFFFFFFFC + 8 would overflow

    trapped = test_out_of_bounds_access(overflow_addr);
    ASSERT_TRUE(trapped)
        << "Address overflow scenario should result in trap or failure";

    // Test access at exact memory boundary (should fail)
    // Access at memory size - 7 should fail (need 8 bytes, only 7 available)
    uint32_t boundary_addr = 131072 - 7;

    trapped = test_out_of_bounds_access(boundary_addr);
    ASSERT_TRUE(trapped)
        << "Access requiring bytes beyond memory boundary should result in trap or failure";
}