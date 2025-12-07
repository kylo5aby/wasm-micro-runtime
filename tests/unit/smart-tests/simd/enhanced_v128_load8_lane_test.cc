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
 * @class V128Load8LaneTestSuite
 * @brief Test fixture for v128.load8_lane opcode validation
 * @details Validates SIMD memory load operations with lane insertion for 8-bit values
 *          Tests both interpreter and AOT execution modes with comprehensive validation
 */
class V128Load8LaneTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.load8_lane testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.load8_lane test module using absolute path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_load8_lane_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.load8_lane tests";
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
     * @brief Helper function to call v128.load8_lane WASM function
     * @param func_name Name of the exported WASM function
     * @param address Memory address for load operation
     * @param result_hi Reference to store high 64 bits of result v128 vector
     * @param result_lo Reference to store low 64 bits of result v128 vector
     * @return bool True if operation succeeded, false on error
     */
    bool call_v128_load8_lane(const char *func_name, uint32_t address,
                             uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: one i32 address input + space for v128 result (4 uint32_t)
        uint32_t argv[5];  // 1 for input i32 + 4 for v128 result
        argv[0] = address;

        // Call WASM function with one i32 input, returns v128
        bool call_success = dummy_env->execute(func_name, 1, argv);
        if (!call_success) {
            return false;
        }

        // Extract v128 result from argv (returned as 4 uint32_t values)
        // Following the pattern from existing v128 tests
        result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];

        return true;
    }

    /**
     * @brief Helper function to extract byte from v128 at specified lane
     * @param v128_hi High 64 bits of v128 vector
     * @param v128_lo Low 64 bits of v128 vector
     * @param lane_index Lane index (0-15)
     * @return The byte value at the specified lane
     */
    uint8_t extract_byte_from_v128(uint64_t v128_hi, uint64_t v128_lo, uint32_t lane_index)
    {
        // Convert v128 to byte array for easy access
        uint8_t bytes[16];
        memcpy(&bytes[0], &v128_lo, 8);  // First 8 bytes (lanes 0-7)
        memcpy(&bytes[8], &v128_hi, 8);  // Next 8 bytes (lanes 8-15)

        return bytes[lane_index];
    }

    /**
     * @brief Helper function to validate v128 lane values
     * @param v128_hi High 64 bits of result v128 vector
     * @param v128_lo Low 64 bits of result v128 vector
     * @param target_lane Lane that should contain the loaded value
     * @param expected_value Expected byte value in target lane
     * @param other_lanes_zero Whether other lanes should be zero (default true)
     */
    void validate_v128_lanes(uint64_t v128_hi, uint64_t v128_lo, uint32_t target_lane,
                            uint8_t expected_value, bool other_lanes_zero = true)
    {
        // Validate target lane contains expected value
        uint8_t actual_value = extract_byte_from_v128(v128_hi, v128_lo, target_lane);
        ASSERT_EQ(actual_value, expected_value)
            << "Lane " << target_lane << " should contain " << (int)expected_value
            << " but got " << (int)actual_value;

        // Optionally validate other lanes are zero
        if (other_lanes_zero) {
            for (uint32_t lane = 0; lane < 16; lane++) {
                if (lane != target_lane) {
                    uint8_t lane_value = extract_byte_from_v128(v128_hi, v128_lo, lane);
                    ASSERT_EQ(lane_value, 0)
                        << "Lane " << lane << " should be zero but got " << (int)lane_value;
                }
            }
        }
    }
};

/**
 * @test BasicLaneLoading_ReturnsCorrectValues
 * @brief Validates v128.load8_lane correctly loads 8-bit values into specified lanes
 * @details Tests fundamental load operation with various byte patterns into different lanes.
 *          Verifies that the loaded byte is correctly placed in the target lane.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:v128_load8_lane_operation
 * @input_conditions Memory with byte patterns (0x42, 0x85, 0xFF), lanes 0/8/15, zero-initialized v128
 * @expected_behavior Byte values correctly placed in target lanes, other lanes remain zero
 * @validation_method Direct comparison of lane values with expected byte patterns
 */
TEST_F(V128Load8LaneTestSuite, BasicLaneLoading_ReturnsCorrectValues)
{
    uint64_t result_hi, result_lo;

    // Test loading 0x42 into lane 0 (address 0x10 contains 0x42)
    ASSERT_TRUE(call_v128_load8_lane("test_load_lane_0", 0x10, result_hi, result_lo))
        << "Failed to call v128.load8_lane test function for lane 0";
    validate_v128_lanes(result_hi, result_lo, 0, 0x42);

    // Test loading 0x85 into lane 8 (address 0x18 contains 0x85)
    ASSERT_TRUE(call_v128_load8_lane("test_load_lane_8", 0x18, result_hi, result_lo))
        << "Failed to call v128.load8_lane test function for lane 8";
    validate_v128_lanes(result_hi, result_lo, 8, 0x85);

    // Test loading 0xFF into lane 15 (address 0x1F contains 0xFF)
    ASSERT_TRUE(call_v128_load8_lane("test_load_lane_15", 0x1F, result_hi, result_lo))
        << "Failed to call v128.load8_lane test function for lane 15";
    validate_v128_lanes(result_hi, result_lo, 15, 0xFF);
}

/**
 * @test AllLanesCoverage_ValidatesCompleteLaneAccess
 * @brief Test loading into all 16 possible lanes (0-15) with different byte values
 * @details Validates that v128.load8_lane can successfully access all 16 byte lanes
 *          within a v128 vector with sequential byte patterns.
 * @test_category Main - Comprehensive lane access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:v128_load8_lane_lane_validation
 * @input_conditions Sequential byte values (0x00-0x0F), corresponding lane indices
 * @expected_behavior Each lane contains its corresponding byte value
 * @validation_method Loop-based validation of all 16 lanes with descriptive messages
 */
TEST_F(V128Load8LaneTestSuite, AllLanesCoverage_ValidatesCompleteLaneAccess)
{
    uint64_t result_hi, result_lo;

    // Test comprehensive lane loading using the test_all_lanes function
    // This function loads sequential bytes (0x00-0x0F) into lanes 0-15
    ASSERT_TRUE(call_v128_load8_lane("test_all_lanes", 0x00, result_hi, result_lo))
        << "Failed to call v128.load8_lane comprehensive test function";

    // Verify each lane contains its corresponding byte value
    for (uint32_t lane = 0; lane < 16; lane++) {
        uint8_t expected_byte = (uint8_t)lane;
        uint8_t actual_byte = extract_byte_from_v128(result_hi, result_lo, lane);

        ASSERT_EQ(actual_byte, expected_byte)
            << "Lane " << lane << " should contain byte value " << (int)expected_byte
            << " but got " << (int)actual_byte;
    }
}

/**
 * @test MemoryBoundaryAccess_HandlesBoundaryConditions
 * @brief Test memory access at boundaries (address 0, last valid address)
 * @details Validates that v128.load8_lane correctly handles memory access at boundary
 *          conditions including the start and near the end of linear memory.
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_boundary_validation
 * @input_conditions Memory at boundaries with known patterns, valid lane indices
 * @expected_behavior Successful loads without access violations
 * @validation_method Boundary access validation with known memory patterns
 */
TEST_F(V128Load8LaneTestSuite, MemoryBoundaryAccess_HandlesBoundaryConditions)
{
    uint64_t result_hi, result_lo;

    // Test loading from address 0 (start of memory) - should load 0x00
    ASSERT_TRUE(call_v128_load8_lane("test_boundary", 0x00, result_hi, result_lo))
        << "Failed to load from memory start boundary (address 0)";
    validate_v128_lanes(result_hi, result_lo, 0, 0x00);

    // Test loading from near end of available memory - should load 0xFF
    ASSERT_TRUE(call_v128_load8_lane("test_boundary", 0xFF, result_hi, result_lo))
        << "Failed to load from memory near boundary";
    validate_v128_lanes(result_hi, result_lo, 0, 0xFF);

    // Test loading from page boundary (address 0x100) - should load 0x42
    ASSERT_TRUE(call_v128_load8_lane("test_boundary", 0x100, result_hi, result_lo))
        << "Failed to load from page boundary address";
    validate_v128_lanes(result_hi, result_lo, 0, 0x42);
}

/**
 * @test LanePreservation_MaintainsNonTargetLanes
 * @brief Verify that loading into one lane doesn't affect other lanes
 * @details Tests lane independence by loading into a target lane while ensuring
 *          all other lanes in the vector preserve their original values.
 * @test_category Edge - Lane independence validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:lane_isolation_logic
 * @input_conditions Pre-initialized v128 with pattern, load into middle lane
 * @expected_behavior Target lane updated, all other lanes preserve original values
 * @validation_method Individual lane comparison with preserved value verification
 */
TEST_F(V128Load8LaneTestSuite, LanePreservation_MaintainsNonTargetLanes)
{
    uint64_t result_hi, result_lo;

    // Test lane preservation using the test_load_lane_preserve function
    // This function creates a pattern vector (0xA0-0xAF) and loads 0x99 into lane 5
    ASSERT_TRUE(call_v128_load8_lane("test_load_lane_preserve", 0x25, result_hi, result_lo))
        << "Failed to call v128.load8_lane preservation test";

    // Verify lane 5 contains the new loaded value (0x99 from memory address 0x25)
    uint8_t lane5_value = extract_byte_from_v128(result_hi, result_lo, 5);
    ASSERT_EQ(lane5_value, 0x99)
        << "Target lane 5 should contain loaded value 0x99 but got "
        << (int)lane5_value;

    // Verify all other lanes preserve their original pattern values
    for (int i = 0; i < 16; i++) {
        if (i != 5) {
            uint8_t expected_value = (uint8_t)(0xA0 + i);
            uint8_t actual_value = extract_byte_from_v128(result_hi, result_lo, i);
            ASSERT_EQ(actual_value, expected_value)
                << "Lane " << i << " should preserve original value "
                << (int)expected_value << " but got " << (int)actual_value;
        }
    }
}

/**
 * @test ExtremeByteValues_HandlesMinMaxValues
 * @brief Test loading extreme byte values (0x00, 0xFF) and bit patterns
 * @details Validates that v128.load8_lane correctly handles extreme 8-bit values
 *          including minimum (0x00), maximum (0xFF), and alternating bit patterns.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:byte_value_handling
 * @input_conditions Memory containing extreme byte values, various lane positions
 * @expected_behavior Correct loading and preservation of all bit patterns
 * @validation_method Exact bit pattern verification for extreme values
 */
TEST_F(V128Load8LaneTestSuite, ExtremeByteValues_HandlesMinMaxValues)
{
    uint64_t result_hi, result_lo;

    // Test minimum byte value (0x00) - offset 0 from address 0x200
    ASSERT_TRUE(call_v128_load8_lane("test_extreme_values", 0x00, result_hi, result_lo))
        << "Failed to load minimum byte value (0x00)";
    validate_v128_lanes(result_hi, result_lo, 0, 0x00);

    // Test maximum byte value (0xFF) - offset 1 from address 0x200
    ASSERT_TRUE(call_v128_load8_lane("test_extreme_values", 0x01, result_hi, result_lo))
        << "Failed to load maximum byte value (0xFF)";
    validate_v128_lanes(result_hi, result_lo, 0, 0xFF);

    // Test alternating bit pattern (0xAA = 10101010) - offset 2 from address 0x200
    ASSERT_TRUE(call_v128_load8_lane("test_extreme_values", 0x02, result_hi, result_lo))
        << "Failed to load alternating bit pattern (0xAA)";
    validate_v128_lanes(result_hi, result_lo, 0, 0xAA);

    // Test inverse alternating pattern (0x55 = 01010101) - offset 3 from address 0x200
    ASSERT_TRUE(call_v128_load8_lane("test_extreme_values", 0x03, result_hi, result_lo))
        << "Failed to load inverse alternating bit pattern (0x55)";
    validate_v128_lanes(result_hi, result_lo, 0, 0x55);
}

/**
 * @test OutOfBoundsAccess_TriggersAppropriateTraps
 * @brief Test memory access beyond allocated limits triggers WebAssembly traps
 * @details Validates that v128.load8_lane properly handles out-of-bounds memory
 *          access by triggering appropriate WebAssembly traps or error conditions.
 * @test_category Error - Out-of-bounds validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_bounds_checking
 * @input_conditions Memory addresses exceeding linear memory size
 * @expected_behavior WAMR should trap with appropriate error handling
 * @validation_method Exception handling validation for out-of-bounds access
 */
TEST_F(V128Load8LaneTestSuite, OutOfBoundsAccess_TriggersAppropriateTraps)
{
    uint64_t result_hi, result_lo;

    // Test access beyond allocated memory (should fail with address 0x10000 = 64KB)
    bool result = call_v128_load8_lane("test_out_of_bounds", 0x10000, result_hi, result_lo);

    // The function call should fail due to out-of-bounds memory access
    ASSERT_FALSE(result)
        << "Out-of-bounds memory access should fail but succeeded";

    // Note: Exception handling details depend on the DummyExecEnv implementation
    // The test validates that the function call returns false for out-of-bounds access
}

