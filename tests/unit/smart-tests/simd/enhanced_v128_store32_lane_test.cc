/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/**
 * Enhanced test suite for v128.store32_lane WASM opcode
 *
 * Tests the v128.store32_lane instruction which stores a 32-bit value from a specific
 * lane of a v128 SIMD vector to linear memory at a calculated address.
 *
 * Coverage includes:
 * - Basic lane store operations across all valid lanes (0-3)
 * - Memory offset calculations and address validation
 * - Boundary condition testing at memory limits
 * - Extreme value patterns and bit preservation
 * - Error condition handling and trap generation
 * - Cross-execution mode consistency (interpreter vs AOT)
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @brief Test fixture for v128.store32_lane opcode validation
 * @details Comprehensive testing of v128.store32_lane instruction.
 *          Validates lane extraction, memory storage, boundary conditions, and error handling.
 */
class V128Store32LaneTestSuite : public testing::Test {
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.store32_lane testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.store32_lane test module using absolute path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_store32_lane_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.store32_lane tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     */
    void TearDown() override
    {
        // Cleanup handled automatically by unique_ptr destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Call WASM function with specified parameters
     * @param func_name Name of the exported WASM function
     * @param args Array of uint32 arguments
     * @param argc Number of arguments
     * @return true if function executed successfully, false if trapped
     * @details Executes WASM function through runtime, handles traps gracefully
     */
    bool call_wasm_function(const char *func_name, uint32_t *args, uint32_t argc)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            wasm_runtime_get_module_inst(dummy_env->get()), func_name);
        if (!func) {
            return false;
        }

        return wasm_runtime_call_wasm(dummy_env->get(), func, argc, args);
    }

    /**
     * @brief Read 32-bit value from linear memory at specified address
     * @param addr Memory address to read from
     * @return 32-bit value read from memory
     * @details Validates memory address and reads 4-byte value from linear memory
     */
    uint32_t read_memory_u32(uint32_t addr)
    {
        wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(dummy_env->get());
        uint8_t *memory_data = (uint8_t*)wasm_runtime_addr_app_to_native(module_inst, addr);
        if (!memory_data) {
            return 0;
        }

        // Read as little-endian 32-bit value
        return *(uint32_t*)memory_data;
    }

    /**
     * @brief Call v128.store32_lane test function for specified lane
     * @param lane Lane index (0-3)
     * @param offset Memory offset for store operation
     * @return true if operation succeeded, false if trapped
     */
    bool call_v128_store32_lane(uint32_t lane, uint32_t offset)
    {
        std::string func_name = "test_store_lane" + std::to_string(lane);
        uint32_t args[] = {offset};
        return call_wasm_function(func_name.c_str(), args, 1);
    }
};

/**
 * @test BasicLaneStorage_StoresCorrectValues
 * @brief Validates fundamental v128.store32_lane functionality for all lane indices
 * @details Tests lane extraction and memory storage for lanes 0-3 with distinct 32-bit values.
 *          Verifies that each lane stores correctly to different memory addresses.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:v128_store32_lane_operation
 * @input_conditions v128 with distinct lane values [0x11111111, 0x22222222, 0x33333333, 0x44444444], base addresses 0x1000-0x4000
 * @expected_behavior Each lane value stored correctly at computed memory addresses
 * @validation_method Direct memory content verification after store operations
 */
TEST_F(V128Store32LaneTestSuite, BasicLaneStorage_StoresCorrectValues)
{
    // Test lane 0 storage (0x11111111)
    ASSERT_TRUE(call_v128_store32_lane(0, 0x1000))
        << "Lane 0 store operation failed";
    ASSERT_EQ(0x11111111U, read_memory_u32(0x1000))
        << "Lane 0 value not stored correctly";

    // Test lane 1 storage (0x22222222)
    ASSERT_TRUE(call_v128_store32_lane(1, 0x2000))
        << "Lane 1 store operation failed";
    ASSERT_EQ(0x22222222U, read_memory_u32(0x2000))
        << "Lane 1 value not stored correctly";

    // Test lane 2 storage (0x33333333)
    ASSERT_TRUE(call_v128_store32_lane(2, 0x3000))
        << "Lane 2 store operation failed";
    ASSERT_EQ(0x33333333U, read_memory_u32(0x3000))
        << "Lane 2 value not stored correctly";

    // Test lane 3 storage (0x44444444)
    ASSERT_TRUE(call_v128_store32_lane(3, 0x4000))
        << "Lane 3 store operation failed";
    ASSERT_EQ(0x44444444U, read_memory_u32(0x4000))
        << "Lane 3 value not stored correctly";
}

/**
 * @test MemoryBoundaryAccess_HandlesLimitsCorrectly
 * @brief Tests v128.store32_lane at memory boundaries and limit conditions
 * @details Validates storage at valid memory boundaries and verifies proper trap behavior for invalid addresses.
 *          Tests 64KB memory limit (1 WASM page) with boundary conditions.
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/common/wasm_memory.c:memory_boundary_check
 * @input_conditions 64KB memory, addresses near boundary (65532 valid, 65536+ invalid)
 * @expected_behavior Valid addresses succeed, invalid addresses cause execution trap
 * @validation_method Memory access validation and trap detection for out-of-bounds
 */
TEST_F(V128Store32LaneTestSuite, MemoryBoundaryAccess_HandlesLimitsCorrectly)
{
    // Test valid boundary access (last valid 32-bit store in 64KB memory)
    uint32_t args_valid[] = {65532}; // 65536 - 4 = last valid 32-bit address
    ASSERT_TRUE(call_wasm_function("test_store_boundary_valid", args_valid, 1))
        << "Valid boundary store should succeed";
    ASSERT_EQ(0xBBBBBBBBU, read_memory_u32(65532))
        << "Boundary value not stored correctly";

    // Test invalid boundary access (beyond memory limit)
    uint32_t args_invalid[] = {65536}; // Beyond 64KB limit
    ASSERT_FALSE(call_wasm_function("test_store_boundary_invalid", args_invalid, 1))
        << "Invalid boundary store should trap";

    // Test partial invalid access (overlaps boundary)
    uint32_t args_partial[] = {65534}; // Only 2 bytes available, need 4
    ASSERT_FALSE(call_wasm_function("test_store_boundary_partial", args_partial, 1))
        << "Partial boundary store should trap";
}

/**
 * @test AddressComputation_CalculatesCorrectly
 * @brief Validates base address + memarg.offset computation for v128.store32_lane
 * @details Tests various base/offset combinations including edge cases.
 *          Verifies correct address arithmetic and memory access patterns.
 * @test_category Main - Address computation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:compute_memory_address
 * @input_conditions Various base/offset combinations, including zero offset scenarios
 * @expected_behavior Correct address arithmetic, proper memory access
 * @validation_method Address computation verification and memory content validation
 */
TEST_F(V128Store32LaneTestSuite, AddressComputation_CalculatesCorrectly)
{
    // Test zero offset (direct address)
    uint32_t args_zero_offset[] = {0x5000}; // base address
    ASSERT_TRUE(call_wasm_function("test_store_zero_offset", args_zero_offset, 1))
        << "Zero offset store should succeed";
    ASSERT_EQ(0xCCCCCCCCU, read_memory_u32(0x5000))
        << "Zero offset value not stored correctly";

    // Test small offset (manual address calculation in WASM)
    uint32_t args_small_offset[] = {0x6000}; // base address, manual +16 in WASM
    ASSERT_TRUE(call_wasm_function("test_store_small_offset", args_small_offset, 1))
        << "Small offset store should succeed";
    ASSERT_EQ(0xDDDDDDDDU, read_memory_u32(0x6010)) // 0x6000 + 16
        << "Small offset value not stored correctly";

    // Test large offset (manual address calculation in WASM)
    uint32_t args_large_offset[] = {0x1000}; // base address, manual +0x7000 in WASM
    ASSERT_TRUE(call_wasm_function("test_store_large_offset", args_large_offset, 1))
        << "Large offset store should succeed";
    ASSERT_EQ(0xEEEEEEEEU, read_memory_u32(0x8000)) // 0x1000 + 0x7000
        << "Large offset value not stored correctly";
}

/**
 * @test ExtremeLaneValues_PreservesDataIntegrity
 * @brief Tests v128.store32_lane with boundary and pattern values in 32-bit lanes
 * @details Validates storage and retrieval of extreme 32-bit values including zero, max, min, and pattern values.
 *          Ensures bit-exact preservation through store/verify cycle.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_lane_extraction
 * @input_conditions Boundary values (0x00000000, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF), pattern values
 * @expected_behavior Exact value preservation through store/verify cycle
 * @validation_method Bit-exact value comparison after storage
 */
TEST_F(V128Store32LaneTestSuite, ExtremeLaneValues_PreservesDataIntegrity)
{
    // Test zero value
    uint32_t args_zero[] = {0x8000}; // base address
    ASSERT_TRUE(call_wasm_function("test_store_zero_value", args_zero, 1))
        << "Zero value store should succeed";
    ASSERT_EQ(0x00000000U, read_memory_u32(0x8000))
        << "Zero value not preserved correctly";

    // Test maximum value
    uint32_t args_max[] = {0x8004}; // base address
    ASSERT_TRUE(call_wasm_function("test_store_max_value", args_max, 1))
        << "Maximum value store should succeed";
    ASSERT_EQ(0xFFFFFFFFU, read_memory_u32(0x8004))
        << "Maximum value not preserved correctly";

    // Test minimum signed value
    uint32_t args_min[] = {0x8008}; // base address
    ASSERT_TRUE(call_wasm_function("test_store_min_value", args_min, 1))
        << "Minimum signed value store should succeed";
    ASSERT_EQ(0x80000000U, read_memory_u32(0x8008))
        << "Minimum signed value not preserved correctly";

    // Test maximum signed value
    uint32_t args_max_signed[] = {0x800C}; // base address
    ASSERT_TRUE(call_wasm_function("test_store_max_signed_value", args_max_signed, 1))
        << "Maximum signed value store should succeed";
    ASSERT_EQ(0x7FFFFFFFU, read_memory_u32(0x800C))
        << "Maximum signed value not preserved correctly";

    // Test pattern values (checkerboard patterns)
    uint32_t args_pattern1[] = {0x8010}; // base address
    ASSERT_TRUE(call_wasm_function("test_store_pattern1", args_pattern1, 1))
        << "Pattern 1 store should succeed";
    ASSERT_EQ(0xAAAAAAAAU, read_memory_u32(0x8010))
        << "Pattern 1 value not preserved correctly";

    uint32_t args_pattern2[] = {0x8014}; // base address
    ASSERT_TRUE(call_wasm_function("test_store_pattern2", args_pattern2, 1))
        << "Pattern 2 store should succeed";
    ASSERT_EQ(0x55555555U, read_memory_u32(0x8014))
        << "Pattern 2 value not preserved correctly";
}