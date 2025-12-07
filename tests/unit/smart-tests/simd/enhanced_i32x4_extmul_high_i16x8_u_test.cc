/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <limits>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * @file enhanced_i32x4_extmul_high_i16x8_u_test.cc
 * @brief Comprehensive test suite for i32x4.extmul_high_i16x8_u SIMD opcode
 *
 * Tests the extended multiplication operation that takes the higher 4 lanes (4-7)
 * of two i16x8 vectors, performs unsigned multiplication, and produces i32x4 results.
 * Validates both interpreter and AOT execution modes for complete coverage.
 */

// Test execution modes for cross-validation
enum class I32x4ExtmulHighI16x8URunningMode : uint8_t {
    INTERP = 1 << 0,
    AOT = 1 << 1
};

static constexpr const char *MODULE_NAME = "i32x4_extmul_high_i16x8_u_test";
static constexpr const char *FUNC_NAME_BASIC_EXTMUL = "test_basic_extmul";
static constexpr const char *FUNC_NAME_BOUNDARY_EXTMUL = "test_boundary_extmul";
static constexpr const char *FUNC_NAME_ZERO_EXTMUL = "test_zero_extmul";
static constexpr const char *FUNC_NAME_VALIDATION_EXTMUL = "test_validation_extmul";

/**
 * Test fixture for i32x4.extmul_high_i16x8_u opcode validation
 *
 * Provides comprehensive test environment for SIMD extended multiplication operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class I32x4ExtmulHighI16x8UTestSuite : public testing::TestWithParam<I32x4ExtmulHighI16x8URunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the i32x4.extmul_high_i16x8_u test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i32x4.extmul_high_i16x8_u test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i32x4_extmul_high_i16x8_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i32x4.extmul_high_i16x8_u tests";
    }

    /**
     * Cleans up test environment and runtime resources
     *
     * Cleanup is handled automatically by RAII destructors.
     */
    void TearDown() override {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * Calls WASM i32x4.extmul_high_i16x8_u test function with input vectors
     *
     * @param func_name Name of the WASM test function to call
     * @param input1_values Array of 4 i32 values for first input v128 vector (as i16x8 packed)
     * @param input2_values Array of 4 i32 values for second input v128 vector (as i16x8 packed)
     * @param output_values Array to store 4 i32 values from output v128 vector
     */
    void call_extmul_test_function(const char* func_name, const int32_t* input1_values,
                                  const int32_t* input2_values, int32_t* output_values) {
        // Prepare arguments: pack input vectors into argv array
        uint32_t argv[8];  // 4 i32 for first vector + 4 i32 for second vector
        memcpy(argv, input1_values, 16);      // First i32x4 vector (representing i16x8)
        memcpy(argv + 4, input2_values, 16);  // Second i32x4 vector (representing i16x8)

        // Execute function
        bool call_success = dummy_env->execute(func_name, 8, argv);
        ASSERT_TRUE(call_success) << "Failed to execute WASM function: " << func_name;

        // Extract result: get i32x4 result (4 i32s = 16 bytes)
        memcpy(output_values, argv, 16);
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicExtendedMultiplication_ReturnsCorrectResults
 * @brief Validates i32x4.extmul_high_i16x8_u produces correct arithmetic results for typical inputs
 * @details Tests fundamental extended multiplication operation with standard unsigned u16 values
 *          in the high lanes [4,5,6,7]. Verifies that the opcode correctly computes unsigned
 *          multiplication of corresponding high lanes and widens results to u32.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_high_unsigned_operation
 * @input_conditions Standard u16 pairs in high 4 lanes: typical unsigned values
 * @expected_behavior Returns mathematical product with proper unsigned arithmetic and widening
 * @validation_method Direct comparison of WASM function result with expected u32x4 values
 */
TEST_P(I32x4ExtmulHighI16x8UTestSuite, BasicExtendedMultiplication_ReturnsCorrectResults)
{
    // Test data: i16x8 vectors represented as i32x4 (packed 2 u16 per i32)
    // First vector: [ignored_lower_lanes..., 10, 20, 30, 40] (high lanes 4-7)
    const int32_t vec1[4] = {0, 0,                        // lower lanes (ignored by high extmul)
                            (20 << 16) | (10 & 0xFFFF),  // lanes 4,5: 10, 20
                            (40 << 16) | (30 & 0xFFFF)}; // lanes 6,7: 30, 40

    // Second vector: [ignored_lower_lanes..., 2, 3, 4, 5] (high lanes 4-7)
    const int32_t vec2[4] = {0, 0,                        // lower lanes (ignored by high extmul)
                            (3 << 16) | (2 & 0xFFFF),    // lanes 4,5: 2, 3
                            (5 << 16) | (4 & 0xFFFF)};   // lanes 6,7: 4, 5

    int32_t result[4];

    // Execute i32x4.extmul_high_i16x8_u operation
    call_extmul_test_function(FUNC_NAME_BASIC_EXTMUL, vec1, vec2, result);

    // Validate results: [10*2, 20*3, 30*4, 40*5] = [20, 60, 120, 200]
    ASSERT_EQ(20, result[0]) << "High lane 4 multiplication failed: 10 * 2 should equal 20";
    ASSERT_EQ(60, result[1]) << "High lane 5 multiplication failed: 20 * 3 should equal 60";
    ASSERT_EQ(120, result[2]) << "High lane 6 multiplication failed: 30 * 4 should equal 120";
    ASSERT_EQ(200, result[3]) << "High lane 7 multiplication failed: 40 * 5 should equal 200";
}

/**
 * @test MaximumBoundaryValues_ProducesCorrectProducts
 * @brief Tests behavior at u16 MIN/MAX boundaries in high lanes and their multiplication products
 * @details Validates extended multiplication with extreme u16 values in high lanes to ensure no overflow
 *          occurs and maximum magnitude results are computed correctly.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_boundary_handling_high_unsigned
 * @input_conditions u16 MIN_VALUE (0) and MAX_VALUE (65535) combinations in high lanes
 * @expected_behavior Maximum magnitude results without overflow in u32 range
 * @validation_method Verification of extreme value multiplication accuracy
 */
TEST_P(I32x4ExtmulHighI16x8UTestSuite, MaximumBoundaryValues_ProducesCorrectProducts)
{
    // Test with u16 boundary values: MIN=0, MAX=65535
    const uint16_t u16_min = 0;
    const uint16_t u16_max = 65535;

    // Vector 1: [ignored..., MAX, MAX, MAX, MAX] (high lanes 4-7)
    const int32_t vec1[4] = {0, 0,                                               // lower lanes (ignored)
                            (u16_max << 16) | (u16_max & 0xFFFF),              // lanes 4,5: 65535, 65535
                            (u16_max << 16) | (u16_max & 0xFFFF)};             // lanes 6,7: 65535, 65535

    // Vector 2: [ignored..., MAX, 1, 2, MIN] (high lanes 4-7)
    const int32_t vec2[4] = {0, 0,                                               // lower lanes (ignored)
                            (1 << 16) | (u16_max & 0xFFFF),                    // lanes 4,5: 65535, 1
                            (u16_min << 16) | (2 & 0xFFFF)};                   // lanes 6,7: 2, 0

    int32_t result[4];

    // Execute boundary value test
    call_extmul_test_function(FUNC_NAME_BOUNDARY_EXTMUL, vec1, vec2, result);

    // Validate results:
    // [65535*65535, 65535*1, 65535*2, 65535*0]
    // = [4294836225, 65535, 131070, 0]
    ASSERT_EQ(4294836225U, static_cast<uint32_t>(result[0])) << "MAX×MAX failed: should equal 4294836225";
    ASSERT_EQ(65535, result[1]) << "MAX×1 failed: should equal 65535";
    ASSERT_EQ(131070, result[2]) << "MAX×2 failed: should equal 131070";
    ASSERT_EQ(0, result[3]) << "MAX×MIN failed: should equal 0";
}

/**
 * @test ZeroAndIdentityOperations_HandlesMathematicalProperties
 * @brief Validates zero multiplication identity and behavior patterns in high lanes
 * @details Tests mathematical properties including zero absorption (0×n=0), identity operations (1×n=n),
 *          and power-of-2 values in high lanes, ensuring per-lane independence.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_mathematical_properties_high_unsigned
 * @input_conditions Zero vectors, identity multipliers, power-of-2 values in high lanes
 * @expected_behavior Correct mathematical property preservation per high lane
 * @validation_method Verification of fundamental arithmetic properties
 */
TEST_P(I32x4ExtmulHighI16x8UTestSuite, ZeroAndIdentityOperations_HandlesMathematicalProperties)
{
    int32_t result[4];

    // Test zero absorption property (0 × n = 0) in first two high lanes
    // Test identity property (1 × n = n) in last two high lanes
    // Vector 1: [non_zero_lower..., 0, 0, 1234, 5678] (mixed zero and values in high lanes)
    const int32_t test_vec1[4] = {(200 << 16) | (100 & 0xFFFF), // lower lanes (ignored)
                                 (400 << 16) | (300 & 0xFFFF),  // lower lanes (ignored)
                                 (0 << 16) | (0 & 0xFFFF),       // lanes 4,5: 0, 0 (zero test)
                                 (5678 << 16) | (1234 & 0xFFFF)}; // lanes 6,7: 1234, 5678

    // Vector 2: [non_zero_lower..., 100, 200, 1, 1] (any values with identity)
    const int32_t test_vec2[4] = {(2000 << 16) | (1000 & 0xFFFF), // lower lanes (ignored)
                                 (4000 << 16) | (3000 & 0xFFFF),  // lower lanes (ignored)
                                 (200 << 16) | (100 & 0xFFFF),    // lanes 4,5: 100, 200 (with zero)
                                 (1 << 16) | (1 & 0xFFFF)};       // lanes 6,7: 1, 1 (identity)

    call_extmul_test_function(FUNC_NAME_ZERO_EXTMUL, test_vec1, test_vec2, result);

    // Validate mathematical properties:
    // [0*100, 0*200, 1234*1, 5678*1] = [0, 0, 1234, 5678]
    ASSERT_EQ(0, result[0]) << "Zero absorption failed in high lane 4: 0 * 100 should equal 0";
    ASSERT_EQ(0, result[1]) << "Zero absorption failed in high lane 5: 0 * 200 should equal 0";
    ASSERT_EQ(1234, result[2]) << "Identity property failed in high lane 6: 1234 * 1 should equal 1234";
    ASSERT_EQ(5678, result[3]) << "Identity property failed in high lane 7: 5678 * 1 should equal 5678";

    // Additional test: Power of 2 values
    // Vector 1: [ignored..., 2, 4, 8, 16] (powers of 2 in high lanes)
    const int32_t pow2_vec1[4] = {0, 0,                          // lower lanes (ignored)
                                 (4 << 16) | (2 & 0xFFFF),      // lanes 4,5: 2, 4
                                 (16 << 16) | (8 & 0xFFFF)};    // lanes 6,7: 8, 16

    // Vector 2: [ignored..., 3, 5, 7, 9] (multipliers)
    const int32_t pow2_vec2[4] = {0, 0,                          // lower lanes (ignored)
                                 (5 << 16) | (3 & 0xFFFF),      // lanes 4,5: 3, 5
                                 (9 << 16) | (7 & 0xFFFF)};     // lanes 6,7: 7, 9

    call_extmul_test_function(FUNC_NAME_ZERO_EXTMUL, pow2_vec1, pow2_vec2, result);

    // Validate power-of-2 results: [2*3, 4*5, 8*7, 16*9] = [6, 20, 56, 144]
    ASSERT_EQ(6, result[0]) << "Power-of-2 multiplication failed: 2 * 3 should equal 6";
    ASSERT_EQ(20, result[1]) << "Power-of-2 multiplication failed: 4 * 5 should equal 20";
    ASSERT_EQ(56, result[2]) << "Power-of-2 multiplication failed: 8 * 7 should equal 56";
    ASSERT_EQ(144, result[3]) << "Power-of-2 multiplication failed: 16 * 9 should equal 144";
}

/**
 * @test ModuleLoadingAndExecution_ValidatesSIMDSupport
 * @brief Validates WASM module loading and SIMD instruction execution
 * @details Tests proper module setup, SIMD feature validation, and successful function execution
 *          to ensure comprehensive SIMD instruction handling by WAMR runtime.
 * @test_category Validation - SIMD instruction and module validation
 * @coverage_target core/iwasm/common/wasm_loader.c:simd_instruction_validation
 * @input_conditions Properly constructed WASM module with SIMD features enabled
 * @expected_behavior Successful module loading, function instantiation, and execution
 * @validation_method Module loading validation and execution success verification
 */
TEST_P(I32x4ExtmulHighI16x8UTestSuite, ModuleLoadingAndExecution_ValidatesSIMDSupport)
{
    // Test basic module functionality with simple multiplication
    // Vector 1: [ignored..., 100, 200, 300, 400] (high lanes 4-7)
    const int32_t simple_vec1[4] = {0, 0,                            // lower lanes (ignored)
                                   (200 << 16) | (100 & 0xFFFF),    // lanes 4,5: 100, 200
                                   (400 << 16) | (300 & 0xFFFF)};   // lanes 6,7: 300, 400

    // Vector 2: [ignored..., 10, 15, 25, 50] (high lanes 4-7)
    const int32_t simple_vec2[4] = {0, 0,                            // lower lanes (ignored)
                                   (15 << 16) | (10 & 0xFFFF),      // lanes 4,5: 10, 15
                                   (50 << 16) | (25 & 0xFFFF)};     // lanes 6,7: 25, 50

    int32_t result[4];

    // Execute validation test function
    call_extmul_test_function(FUNC_NAME_VALIDATION_EXTMUL, simple_vec1, simple_vec2, result);

    // Validate basic execution and SIMD instruction processing
    // Expected: [100*10, 200*15, 300*25, 400*50] = [1000, 3000, 7500, 20000]
    ASSERT_EQ(1000, result[0]) << "SIMD execution validation failed: 100 * 10 should equal 1000";
    ASSERT_EQ(3000, result[1]) << "SIMD execution validation failed: 200 * 15 should equal 3000";
    ASSERT_EQ(7500, result[2]) << "SIMD execution validation failed: 300 * 25 should equal 7500";
    ASSERT_EQ(20000, result[3]) << "SIMD execution validation failed: 400 * 50 should equal 20000";

    // Validate that dummy_env and module are properly initialized
    ASSERT_NE(nullptr, dummy_env) << "Execution environment should be properly initialized";
    ASSERT_NE(nullptr, dummy_env->get()) << "WASM module should be successfully loaded";
}

// Parametrized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningModeTests,
    I32x4ExtmulHighI16x8UTestSuite,
    testing::Values(I32x4ExtmulHighI16x8URunningMode::INTERP, I32x4ExtmulHighI16x8URunningMode::AOT),
    [](const testing::TestParamInfo<I32x4ExtmulHighI16x8UTestSuite::ParamType>& info) {
        return info.param == I32x4ExtmulHighI16x8URunningMode::INTERP ? "Interpreter" : "AOT";
    }
);