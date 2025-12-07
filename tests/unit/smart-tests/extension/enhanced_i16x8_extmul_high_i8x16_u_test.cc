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
 * @brief Test suite for i16x8.extmul_high_i8x16_u opcode
 * @details Comprehensive testing of extended unsigned multiplication of high 8 lanes
 *          from two i8x16 vectors producing i16x8 results. Tests cover basic functionality,
 *          boundary conditions, edge cases, and cross-execution mode validation.
 */
class I16x8ExtmulHighI8x16UTestSuite : public testing::Test
{
protected:
    /**
     * @brief Set up test environment for each test case
     * @details Initializes WAMR runtime using WAMRRuntimeRAII helper and prepares
     *          test module for i16x8.extmul_high_i8x16_u opcode validation
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<512 * 1024>>();

        // Load the i16x8.extmul_high_i8x16_u test module
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i16x8_extmul_high_i8x16_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i16x8.extmul_high_i8x16_u tests";
    }

    /**
     * @brief Clean up test environment after each test case
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks
     */
    void TearDown() override
    {
        // RAII handles cleanup automatically
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call i16x8.extmul_high_i8x16_u test function
     * @param a_bytes Array of 16 i8 values for first v128 vector
     * @param b_bytes Array of 16 i8 values for second v128 vector
     * @param result_i16 Output array of 8 i16 values for result
     * @details Calls WASM function that performs i16x8.extmul_high_i8x16_u on high lanes (8-15)
     */
    void call_i16x8_extmul_high_i8x16_u(const int8_t a_bytes[16], const int8_t b_bytes[16],
                                         int16_t result_i16[8])
    {
        // v128 functions take two v128 parameters (8 uint32_t values total)
        // and return one v128 (4 uint32_t values)
        uint32_t argv[8]; // 8 uint32_t = 2 v128 inputs

        // Pack input vectors into argv - each v128 is 4 uint32_t values
        // First v128 (argv[0-3])
        memcpy(&argv[0], a_bytes, 16);
        // Second v128 (argv[4-7])
        memcpy(&argv[4], b_bytes, 16);

        bool call_result = dummy_env->execute("test_i16x8_extmul_high_i8x16_u", 8, argv);
        ASSERT_TRUE(call_result) << "Failed to call WASM function: " << dummy_env->get_exception();

        // Extract v128 result - the return value overwrites argv starting from index 0
        // Cast the argv array directly to get the result bytes
        uint8_t *result_bytes = reinterpret_cast<uint8_t*>(argv);

        memcpy(result_i16, result_bytes, 8 * sizeof(int16_t));
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<512 * 1024>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicUnsignedMultiplication_ReturnsCorrectResults
 * @brief Validates i16x8.extmul_high_i8x16_u produces correct arithmetic results for typical inputs
 * @details Tests fundamental extended multiplication operation with various unsigned integer combinations.
 *          Verifies that only high lanes (8-15) are processed and results are correctly computed.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extmul_high_i8x16_u_operation
 * @input_conditions Standard unsigned integer pairs in high lanes of v128 vectors
 * @expected_behavior Returns correct extended multiplication results as i16 values
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(I16x8ExtmulHighI8x16UTestSuite, BasicUnsignedMultiplication_ReturnsCorrectResults)
{
    // Test vectors with typical unsigned values in high lanes
    int8_t vec_a[16] = {99, 88, 77, 66, 55, 44, 33, 22,  // low lanes (ignored)
                        10, 20, 30, 40, 50, 60, 70, 80}; // high lanes (processed)
    int8_t vec_b[16] = {11, 12, 13, 14, 15, 16, 17, 18,  // low lanes (ignored)
                        2, 3, 4, 5, 6, 7, 8, 9};         // high lanes (processed)

    int16_t result[8];
    call_i16x8_extmul_high_i8x16_u(vec_a, vec_b, result);

    // Verify correct multiplication of high lanes: (unsigned)a[8+i] * (unsigned)b[8+i]
    ASSERT_EQ(result[0], 20)  << "Lane 0: 10 * 2 should equal 20";
    ASSERT_EQ(result[1], 60)  << "Lane 1: 20 * 3 should equal 60";
    ASSERT_EQ(result[2], 120) << "Lane 2: 30 * 4 should equal 120";
    ASSERT_EQ(result[3], 200) << "Lane 3: 40 * 5 should equal 200";
    ASSERT_EQ(result[4], 300) << "Lane 4: 50 * 6 should equal 300";
    ASSERT_EQ(result[5], 420) << "Lane 5: 60 * 7 should equal 420";
    ASSERT_EQ(result[6], 560) << "Lane 6: 70 * 8 should equal 560";
    ASSERT_EQ(result[7], 720) << "Lane 7: 80 * 9 should equal 720";
}

/**
 * @test HighLaneIsolation_IgnoresLowLanes
 * @brief Verifies only high lanes (8-15) affect results, low lanes (0-7) are ignored
 * @details Tests isolation of high lanes by using different values in low vs high lanes.
 *          Confirms that low lane values have no impact on the computation results.
 * @test_category Main - Lane isolation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extmul_high_i8x16_u_lane_selection
 * @input_conditions Different values in low vs high lanes to test isolation
 * @expected_behavior Results based only on high lanes, low lanes ignored
 * @validation_method Compare results with expected values based only on high lanes
 */
TEST_F(I16x8ExtmulHighI8x16UTestSuite, HighLaneIsolation_IgnoresLowLanes)
{
    // Use extreme values in low lanes to test they are properly ignored
    int8_t vec_a[16] = {-1, -1, -1, -1, -1, -1, -1, -1,  // low lanes (should be ignored)
                        1, 2, 3, 4, 5, 6, 7, 8};         // high lanes (processed)
    int8_t vec_b[16] = {127, 127, 127, 127, 127, 127, 127, 127,  // low lanes (should be ignored)
                        2, 3, 4, 5, 6, 7, 8, 9};                 // high lanes (processed)

    int16_t result[8];
    call_i16x8_extmul_high_i8x16_u(vec_a, vec_b, result);

    // Results should be based only on high lanes, ignoring extreme low lane values
    ASSERT_EQ(result[0], 2)  << "Lane 0: 1 * 2 should equal 2 (low lanes ignored)";
    ASSERT_EQ(result[1], 6)  << "Lane 1: 2 * 3 should equal 6 (low lanes ignored)";
    ASSERT_EQ(result[2], 12) << "Lane 2: 3 * 4 should equal 12 (low lanes ignored)";
    ASSERT_EQ(result[3], 20) << "Lane 3: 4 * 5 should equal 20 (low lanes ignored)";
    ASSERT_EQ(result[4], 30) << "Lane 4: 5 * 6 should equal 30 (low lanes ignored)";
    ASSERT_EQ(result[5], 42) << "Lane 5: 6 * 7 should equal 42 (low lanes ignored)";
    ASSERT_EQ(result[6], 56) << "Lane 6: 7 * 8 should equal 56 (low lanes ignored)";
    ASSERT_EQ(result[7], 72) << "Lane 7: 8 * 9 should equal 72 (low lanes ignored)";
}

/**
 * @test BoundaryValues_HandlesMinMaxCorrectly
 * @brief Tests boundary conditions with 0 and 255 (max unsigned i8) values
 * @details Validates correct handling of minimum and maximum unsigned i8 values,
 *          including maximum possible product (255 * 255 = 65025).
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extmul_high_i8x16_u_boundary_handling
 * @input_conditions Mix of minimum (0) and maximum (255) unsigned values
 * @expected_behavior Correct handling of boundary multiplications including max product
 * @validation_method Verify boundary value multiplication results
 */
TEST_F(I16x8ExtmulHighI8x16UTestSuite, BoundaryValues_HandlesMinMaxCorrectly)
{
    // Test with boundary values: 0 and 255 (max unsigned i8)
    int8_t vec_a[16] = {0, 0, 0, 0, 0, 0, 0, 0,                      // low lanes (ignored)
                        0, 1, -1, -1, 0, -1, 127, -128};             // high lanes: 0,1,255,255,0,255,127,128
    int8_t vec_b[16] = {0, 0, 0, 0, 0, 0, 0, 0,                      // low lanes (ignored)
                        -1, -1, -1, -2, 100, 50, 2, 2};              // high lanes: 255,255,255,254,100,50,2,2

    int16_t result[8];
    call_i16x8_extmul_high_i8x16_u(vec_a, vec_b, result);

    // Verify boundary value multiplications (treating i8 as unsigned)
    // Note: Results are stored as i16, so values > 32767 will appear negative when cast to signed
    ASSERT_EQ(result[0], 0)     << "Lane 0: 0 * 255 should equal 0";
    ASSERT_EQ(result[1], 255)   << "Lane 1: 1 * 255 should equal 255";
    ASSERT_EQ((uint16_t)result[2], 65025) << "Lane 2: 255 * 255 should equal 65025 (max product)";
    ASSERT_EQ((uint16_t)result[3], 64770) << "Lane 3: 255 * 254 should equal 64770";
    ASSERT_EQ(result[4], 0)     << "Lane 4: 0 * 100 should equal 0";
    ASSERT_EQ((uint16_t)result[5], 12750) << "Lane 5: 255 * 50 should equal 12750";
    ASSERT_EQ(result[6], 254)   << "Lane 6: 127 * 2 should equal 254";
    ASSERT_EQ(result[7], 256)   << "Lane 7: 128 * 2 should equal 256";
}

/**
 * @test ZeroMultiplication_ProducesZeroResults
 * @brief Validates multiplication by zero scenarios produce zero results
 * @details Tests various zero patterns in high lanes to verify that any multiplication
 *          involving zero produces zero result regardless of the other operand value.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extmul_high_i8x16_u_zero_handling
 * @input_conditions Various zero patterns mixed with non-zero values
 * @expected_behavior Zero results for any multiplication involving zero operand
 * @validation_method Verify all zero products produce zero results
 */
TEST_F(I16x8ExtmulHighI8x16UTestSuite, ZeroMultiplication_ProducesZeroResults)
{
    // Test zero multiplication patterns
    int8_t vec_a[16] = {1, 1, 1, 1, 1, 1, 1, 1,              // low lanes (ignored)
                        0, 0, 0, 0, -1, -1, -1, -1};          // high lanes: 0,0,0,0,255,255,255,255
    int8_t vec_b[16] = {2, 2, 2, 2, 2, 2, 2, 2,              // low lanes (ignored)
                        -1, 100, 50, 25, 0, 0, 0, 0};         // high lanes: 255,100,50,25,0,0,0,0

    int16_t result[8];
    call_i16x8_extmul_high_i8x16_u(vec_a, vec_b, result);

    // All results should be zero when either operand is zero
    ASSERT_EQ(result[0], 0) << "Lane 0: 0 * 255 should equal 0";
    ASSERT_EQ(result[1], 0) << "Lane 1: 0 * 100 should equal 0";
    ASSERT_EQ(result[2], 0) << "Lane 2: 0 * 50 should equal 0";
    ASSERT_EQ(result[3], 0) << "Lane 3: 0 * 25 should equal 0";
    ASSERT_EQ(result[4], 0) << "Lane 4: 255 * 0 should equal 0";
    ASSERT_EQ(result[5], 0) << "Lane 5: 255 * 0 should equal 0";
    ASSERT_EQ(result[6], 0) << "Lane 6: 255 * 0 should equal 0";
    ASSERT_EQ(result[7], 0) << "Lane 7: 255 * 0 should equal 0";
}

/**
 * @test IdentityMultiplication_PreservesValues
 * @brief Tests multiplication by 1 (identity element) preserves original values
 * @details Verifies that multiplying by 1 produces the original value zero-extended to i16.
 *          Tests the mathematical identity property for multiplication.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extmul_high_i8x16_u_identity
 * @input_conditions One vector with all 1s in high lanes, various values in other vector
 * @expected_behavior Other vector's values preserved (zero-extended to i16)
 * @validation_method Verify identity property holds for all high lanes
 */
TEST_F(I16x8ExtmulHighI8x16UTestSuite, IdentityMultiplication_PreservesValues)
{
    // Test identity multiplication (multiply by 1)
    int8_t vec_a[16] = {0, 0, 0, 0, 0, 0, 0, 0,                      // low lanes (ignored)
                        1, 1, 1, 1, 1, 1, 1, 1};                     // high lanes: all 1s
    int8_t vec_b[16] = {99, 88, 77, 66, 55, 44, 33, 22,              // low lanes (ignored)
                        37, 89, 123, -56, -1, -128, 64, 15};         // high lanes: mixed values

    int16_t result[8];
    call_i16x8_extmul_high_i8x16_u(vec_a, vec_b, result);

    // Results should equal the original values (treated as unsigned, zero-extended to i16)
    ASSERT_EQ(result[0], 37)  << "Lane 0: 1 * 37 should equal 37";
    ASSERT_EQ(result[1], 89)  << "Lane 1: 1 * 89 should equal 89";
    ASSERT_EQ(result[2], 123) << "Lane 2: 1 * 123 should equal 123";
    ASSERT_EQ(result[3], 200) << "Lane 3: 1 * (unsigned)200 should equal 200"; // -56 as unsigned = 200
    ASSERT_EQ(result[4], 255) << "Lane 4: 1 * (unsigned)255 should equal 255"; // -1 as unsigned = 255
    ASSERT_EQ(result[5], 128) << "Lane 5: 1 * (unsigned)128 should equal 128"; // -128 as unsigned = 128
    ASSERT_EQ(result[6], 64)  << "Lane 6: 1 * 64 should equal 64";
    ASSERT_EQ(result[7], 15)  << "Lane 7: 1 * 15 should equal 15";
}

/**
 * @test CommutativeProperty_ProducesIdenticalResults
 * @brief Validates commutative property (a×b = b×a) produces identical results
 * @details Tests mathematical commutative property by swapping operand order and
 *          verifying results are identical regardless of operand order.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extmul_high_i8x16_u_commutativity
 * @input_conditions Two different test vectors, swapped for second test
 * @expected_behavior Identical results regardless of operand order
 * @validation_method Compare results of both orders for equivalence
 */
TEST_F(I16x8ExtmulHighI8x16UTestSuite, CommutativeProperty_ProducesIdenticalResults)
{
    // First test: A * B
    int8_t vec_a[16] = {0, 0, 0, 0, 0, 0, 0, 0,                      // low lanes (ignored)
                        12, 34, 56, 78, 90, 111, -123, -101};        // high lanes
    int8_t vec_b[16] = {1, 1, 1, 1, 1, 1, 1, 1,                      // low lanes (ignored)
                        5, 7, 9, 11, 13, 15, 17, 19};                // high lanes

    int16_t result_ab[8];
    call_i16x8_extmul_high_i8x16_u(vec_a, vec_b, result_ab);

    // Second test: B * A (swapped operands)
    int16_t result_ba[8];
    call_i16x8_extmul_high_i8x16_u(vec_b, vec_a, result_ba);

    // Results should be identical due to commutative property
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(result_ab[i], result_ba[i])
            << "Lane " << i << ": Commutative property failed - A*B != B*A";
    }

    // Verify specific expected results for completeness
    ASSERT_EQ(result_ab[0], 60)   << "Lane 0: 12 * 5 should equal 60";
    ASSERT_EQ(result_ab[1], 238)  << "Lane 1: 34 * 7 should equal 238";
    ASSERT_EQ(result_ab[2], 504)  << "Lane 2: 56 * 9 should equal 504";
    ASSERT_EQ(result_ab[3], 858)  << "Lane 3: 78 * 11 should equal 858";
    ASSERT_EQ(result_ab[4], 1170) << "Lane 4: 90 * 13 should equal 1170";
    ASSERT_EQ(result_ab[5], 1665) << "Lane 5: 111 * 15 should equal 1665";
    ASSERT_EQ(result_ab[6], 2261) << "Lane 6: (unsigned)133 * 17 should equal 2261"; // -123 as unsigned = 133
    ASSERT_EQ(result_ab[7], 2945) << "Lane 7: (unsigned)155 * 19 should equal 2945"; // -101 as unsigned = 155
}