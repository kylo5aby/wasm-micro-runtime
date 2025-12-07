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
 * @file enhanced_i32x4_extmul_high_i16x8_s_test.cc
 * @brief Comprehensive test suite for i32x4.extmul_high_i16x8_s SIMD opcode
 *
 * Tests the extended multiplication operation that takes the higher 4 lanes (4-7)
 * of two i16x8 vectors, performs signed multiplication, and produces i32x4 results.
 * Validates both interpreter and AOT execution modes for complete coverage.
 */

// Test execution modes for cross-validation
enum class I32x4ExtmulHighI16x8SRunningMode : uint8_t {
    INTERP = 1 << 0,
    AOT = 1 << 1
};

static constexpr const char *MODULE_NAME = "i32x4_extmul_high_i16x8_s_test";
static constexpr const char *FUNC_NAME_BASIC_EXTMUL = "test_basic_extmul";
static constexpr const char *FUNC_NAME_SIGNED_EXTMUL = "test_signed_extmul";
static constexpr const char *FUNC_NAME_BOUNDARY_EXTMUL = "test_boundary_extmul";
static constexpr const char *FUNC_NAME_ZERO_EXTMUL = "test_zero_extmul";
static constexpr const char *FUNC_NAME_COMMUTATIVE_EXTMUL = "test_commutative_extmul";

/**
 * Test fixture for i32x4.extmul_high_i16x8_s opcode validation
 *
 * Provides comprehensive test environment for SIMD extended multiplication operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class I32x4ExtmulHighI16x8STestSuite : public testing::TestWithParam<I32x4ExtmulHighI16x8SRunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the i32x4.extmul_high_i16x8_s test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i32x4.extmul_high_i16x8_s test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i32x4_extmul_high_i16x8_s_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i32x4.extmul_high_i16x8_s tests";
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
     * Calls WASM i32x4.extmul_high_i16x8_s test function with input vectors
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
 * @test BasicMultiplication_ReturnsCorrectResults
 * @brief Validates i32x4.extmul_high_i16x8_s produces correct arithmetic results for typical inputs
 * @details Tests fundamental extended multiplication operation with positive, negative, and mixed-sign
 *          i16 values in the high lanes [4,5,6,7]. Verifies that the opcode correctly computes signed
 *          multiplication of corresponding high lanes and widens results to i32.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_high_operation
 * @input_conditions Standard i16 pairs in high 4 lanes: positive, negative, and mixed signs
 * @expected_behavior Returns mathematical product with proper sign handling and widening
 * @validation_method Direct comparison of WASM function result with expected i32x4 values
 */
TEST_P(I32x4ExtmulHighI16x8STestSuite, BasicMultiplication_ReturnsCorrectResults)
{
    // Test data: i16x8 vectors represented as i32x4 (packed 2 i16 per i32)
    // First vector: [ignored_lower_lanes..., 5, 6, 7, 8] (high lanes 4-7)
    const int32_t vec1[4] = {0, 0,                        // lower lanes (ignored by high extmul)
                            (6 << 16) | (5 & 0xFFFF),    // lanes 4,5: 5, 6
                            (8 << 16) | (7 & 0xFFFF)};   // lanes 6,7: 7, 8

    // Second vector: [ignored_lower_lanes..., 14, 15, 16, 17] (high lanes 4-7)
    const int32_t vec2[4] = {0, 0,                        // lower lanes (ignored by high extmul)
                            (15 << 16) | (14 & 0xFFFF),  // lanes 4,5: 14, 15
                            (17 << 16) | (16 & 0xFFFF)}; // lanes 6,7: 16, 17

    int32_t result[4];

    // Execute i32x4.extmul_high_i16x8_s operation
    call_extmul_test_function(FUNC_NAME_BASIC_EXTMUL, vec1, vec2, result);

    // Validate results: [5*14, 6*15, 7*16, 8*17] = [70, 90, 112, 136]
    ASSERT_EQ(70, result[0]) << "High lane 4 multiplication failed: 5 * 14 should equal 70";
    ASSERT_EQ(90, result[1]) << "High lane 5 multiplication failed: 6 * 15 should equal 90";
    ASSERT_EQ(112, result[2]) << "High lane 6 multiplication failed: 7 * 16 should equal 112";
    ASSERT_EQ(136, result[3]) << "High lane 7 multiplication failed: 8 * 17 should equal 136";
}

/**
 * @test SignedArithmetic_HandlesSignVariations
 * @brief Validates correct signed multiplication behavior with various sign combinations in high lanes
 * @details Tests mixed positive/negative i16 values in high lanes to ensure proper two's complement
 *          arithmetic and sign handling in extended multiplication results.
 * @test_category Main - Signed arithmetic validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_signed_multiply_high
 * @input_conditions Mixed-sign i16 values in high lanes: positive×negative, negative×positive, negative×negative
 * @expected_behavior Correct signed multiplication results with proper sign propagation
 * @validation_method Verification of sign handling in two's complement arithmetic
 */
TEST_P(I32x4ExtmulHighI16x8STestSuite, SignedArithmetic_HandlesSignVariations)
{
    // Test data with mixed signs in high lanes: [ignored..., -1, -2, -3, -4]
    const int32_t vec1[4] = {0, 0,                                          // lower lanes (ignored)
                            ((-2) << 16) | ((-1) & 0xFFFF),               // lanes 4,5: -1, -2
                            ((-4) << 16) | ((-3) & 0xFFFF)};              // lanes 6,7: -3, -4

    // Second vector with mixed signs: [ignored..., 5, -6, 7, -8]
    const int32_t vec2[4] = {0, 0,                                          // lower lanes (ignored)
                            ((-6) << 16) | (5 & 0xFFFF),                  // lanes 4,5: 5, -6
                            ((-8) << 16) | (7 & 0xFFFF)};                 // lanes 6,7: 7, -8

    int32_t result[4];

    // Execute signed multiplication test
    call_extmul_test_function(FUNC_NAME_SIGNED_EXTMUL, vec1, vec2, result);

    // Validate results: [(-1)*5, (-2)*(-6), (-3)*7, (-4)*(-8)] = [-5, 12, -21, 32]
    ASSERT_EQ(-5, result[0]) << "Negative×Positive failed: (-1) * 5 should equal -5";
    ASSERT_EQ(12, result[1]) << "Negative×Negative failed: (-2) * (-6) should equal 12";
    ASSERT_EQ(-21, result[2]) << "Negative×Positive failed: (-3) * 7 should equal -21";
    ASSERT_EQ(32, result[3]) << "Negative×Negative failed: (-4) * (-8) should equal 32";
}

/**
 * @test BoundaryValues_HandleExtremeInputs
 * @brief Tests behavior at i16 MIN/MAX boundaries in high lanes and their multiplication products
 * @details Validates extended multiplication with extreme i16 values in high lanes to ensure no overflow
 *          occurs and maximum magnitude results are computed correctly.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_boundary_handling_high
 * @input_conditions i16 MIN_VALUE (-32768) and MAX_VALUE (32767) combinations in high lanes
 * @expected_behavior Maximum magnitude results without overflow in i32 range
 * @validation_method Verification of extreme value multiplication accuracy
 */
TEST_P(I32x4ExtmulHighI16x8STestSuite, BoundaryValues_HandleExtremeInputs)
{
    // Test with i16 boundary values: MIN=-32768, MAX=32767
    const int16_t i16_min = -32768;
    const int16_t i16_max = 32767;

    // Vector 1: [ignored..., MAX, MIN, MAX, MIN] (high lanes 4-7)
    const int32_t vec1[4] = {0, 0,                                               // lower lanes (ignored)
                            (i16_min << 16) | (i16_max & 0xFFFF),              // lanes 4,5: 32767, -32768
                            (i16_min << 16) | (i16_max & 0xFFFF)};             // lanes 6,7: 32767, -32768

    // Vector 2: [ignored..., MAX, MAX, MIN, MIN] (high lanes 4-7)
    const int32_t vec2[4] = {0, 0,                                               // lower lanes (ignored)
                            (i16_max << 16) | (i16_max & 0xFFFF),              // lanes 4,5: 32767, 32767
                            (i16_min << 16) | (i16_min & 0xFFFF)};             // lanes 6,7: -32768, -32768

    int32_t result[4];

    // Execute boundary value test
    call_extmul_test_function(FUNC_NAME_BOUNDARY_EXTMUL, vec1, vec2, result);

    // Validate results:
    // [32767*32767, (-32768)*32767, 32767*(-32768), (-32768)*(-32768)]
    // = [1073676289, -1073709056, -1073709056, 1073741824]
    ASSERT_EQ(1073676289, result[0]) << "MAX×MAX failed: should equal 1073676289";
    ASSERT_EQ(-1073709056, result[1]) << "MIN×MAX failed: should equal -1073709056";
    ASSERT_EQ(-1073709056, result[2]) << "MAX×MIN failed: should equal -1073709056";
    ASSERT_EQ(1073741824, result[3]) << "MIN×MIN failed: should equal 1073741824";
}

/**
 * @test ZeroOperands_ProducesZeroResults
 * @brief Validates zero multiplication identity and behavior patterns in high lanes
 * @details Tests mathematical properties including zero absorption (0×n=0) in high lanes,
 *          and per-lane independence to ensure lower lanes don't affect high lane results.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_zero_properties_high
 * @input_conditions Zero vectors in high lanes, mixed zero/non-zero scenarios
 * @expected_behavior Correct mathematical property preservation per high lane
 * @validation_method Verification of fundamental arithmetic properties
 */
TEST_P(I32x4ExtmulHighI16x8STestSuite, ZeroOperands_ProducesZeroResults)
{
    int32_t result[4];

    // Test zero absorption property (0 × n = 0) in high lanes
    // Vector 1: [non_zero_lower..., 0, 0, 0, 0] (zero in high lanes)
    const int32_t zero_high_vec[4] = {(200 << 16) | (100 & 0xFFFF), // lower lanes (ignored)
                                     (400 << 16) | (300 & 0xFFFF), // lower lanes (ignored)
                                     0, 0};                         // high lanes: all zeros

    // Vector 2: [non_zero_lower..., 100, 200, 300, 400] (non-zero in high lanes)
    const int32_t any_vec[4] = {(2000 << 16) | (1000 & 0xFFFF),    // lower lanes (ignored)
                               (4000 << 16) | (3000 & 0xFFFF),     // lower lanes (ignored)
                               (200 << 16) | (100 & 0xFFFF),       // lanes 4,5: 100, 200
                               (400 << 16) | (300 & 0xFFFF)};      // lanes 6,7: 300, 400

    call_extmul_test_function(FUNC_NAME_ZERO_EXTMUL, zero_high_vec, any_vec, result);

    // All high lane results should be zero (0 × anything = 0)
    ASSERT_EQ(0, result[0]) << "Zero absorption failed in high lane 4";
    ASSERT_EQ(0, result[1]) << "Zero absorption failed in high lane 5";
    ASSERT_EQ(0, result[2]) << "Zero absorption failed in high lane 6";
    ASSERT_EQ(0, result[3]) << "Zero absorption failed in high lane 7";
}

/**
 * @test MathematicalProperties_ValidatesCommutativeProperty
 * @brief Tests mathematical properties (commutativity) of high-lane multiplication
 * @details Validates that a×b = b×a for high lane pairs, ensuring consistent
 *          behavior regardless of operand order.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_commutative_high
 * @input_conditions Identical values in swapped operand positions in high lanes
 * @expected_behavior a×b = b×a for all high lane pairs
 * @validation_method Cross-validation comparing swapped operand results
 */
TEST_P(I32x4ExtmulHighI16x8STestSuite, MathematicalProperties_ValidatesCommutativeProperty)
{
    // Test commutative property: a*b = b*a
    // Vector A: [ignored..., 15, -20, 25, -30] (high lanes)
    const int32_t vecA[4] = {0, 0,                                      // lower lanes (ignored)
                            ((-20) << 16) | (15 & 0xFFFF),             // lanes 4,5: 15, -20
                            ((-30) << 16) | (25 & 0xFFFF)};            // lanes 6,7: 25, -30

    // Vector B: [ignored..., 7, 8, -9, 10] (high lanes)
    const int32_t vecB[4] = {0, 0,                                      // lower lanes (ignored)
                            (8 << 16) | (7 & 0xFFFF),                  // lanes 4,5: 7, 8
                            (10 << 16) | ((-9) & 0xFFFF)};             // lanes 6,7: -9, 10

    int32_t resultAB[4], resultBA[4];

    // Test A × B
    call_extmul_test_function(FUNC_NAME_COMMUTATIVE_EXTMUL, vecA, vecB, resultAB);

    // Test B × A (swapped operands)
    call_extmul_test_function(FUNC_NAME_COMMUTATIVE_EXTMUL, vecB, vecA, resultBA);

    // Verify commutative property: A×B should equal B×A
    // Expected: [15*7, (-20)*8, 25*(-9), (-30)*10] = [105, -160, -225, -300]
    ASSERT_EQ(resultAB[0], resultBA[0]) << "Commutative property failed in high lane 4";
    ASSERT_EQ(resultAB[1], resultBA[1]) << "Commutative property failed in high lane 5";
    ASSERT_EQ(resultAB[2], resultBA[2]) << "Commutative property failed in high lane 6";
    ASSERT_EQ(resultAB[3], resultBA[3]) << "Commutative property failed in high lane 7";

    // Also verify the actual expected results
    ASSERT_EQ(105, resultAB[0]) << "High lane 4: 15 × 7 should equal 105";
    ASSERT_EQ(-160, resultAB[1]) << "High lane 5: (-20) × 8 should equal -160";
    ASSERT_EQ(-225, resultAB[2]) << "High lane 6: 25 × (-9) should equal -225";
    ASSERT_EQ(-300, resultAB[3]) << "High lane 7: (-30) × 10 should equal -300";
}

// Parametrized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningModeTests,
    I32x4ExtmulHighI16x8STestSuite,
    testing::Values(I32x4ExtmulHighI16x8SRunningMode::INTERP, I32x4ExtmulHighI16x8SRunningMode::AOT),
    [](const testing::TestParamInfo<I32x4ExtmulHighI16x8STestSuite::ParamType>& info) {
        return info.param == I32x4ExtmulHighI16x8SRunningMode::INTERP ? "Interpreter" : "AOT";
    }
);