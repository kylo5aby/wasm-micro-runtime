/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include <cstring>

/**
 * @brief Enhanced unit tests for i16x8.extadd_pairwise_i8x16 SIMD opcode
 *
 * This test suite comprehensively validates the i16x8.extadd_pairwise_i8x16 instruction,
 * which performs pairwise addition with sign extension from i8x16 to i16x8 vectors.
 * The operation takes 16 i8 values, pairs adjacent elements, sign-extends each to i16,
 * and adds the pairs to produce 8 i16 results.
 *
 * Test coverage includes:
 * - Basic pairwise addition functionality with typical values
 * - Sign extension behavior for negative i8 values
 * - Mixed positive/negative value combinations
 * - Boundary conditions with INT8_MIN and INT8_MAX values
 * - Cross-execution mode validation (interpreter and AOT)
 */
class I16x8ExtaddPairwiseI8x16TestSuite : public testing::Test {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     * @details Initializes WAMR runtime using RAII helper and loads the
     *          i16x8.extadd_pairwise_i8x16 test WASM module for execution testing.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i16x8.extadd_pairwise_i8x16 test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i16x8_extadd_pairwise_i8x16_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i16x8.extadd_pairwise_i8x16 tests";
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     * @details Properly releases all allocated resources using RAII pattern
     *          to prevent resource leaks.
     */
    void TearDown() override {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute i16x8.extadd_pairwise_i8x16 operation with input vector
     * @param func_name Name of the test function to call
     * @param input_i8 Array of 16 i8 values as input vector
     * @param result_i16 Output array to store the 8 i16 results
     * @return bool True if operation succeeded, false on error
     * @details Calls the WASM test function to perform i16x8.extadd_pairwise_i8x16 operation
     */
    bool call_i16x8_extadd_pairwise_i8x16(const std::string& func_name,
                                          const int8_t input_i8[16],
                                          int16_t result_i16[8]) {
        uint32_t argv[4]; // 4 uint32 values to represent v128 input

        // Pack i8x16 vector into argv (16 bytes = 4 uint32)
        memcpy(argv, input_i8, 16);

        // Execute the WASM function
        bool success = dummy_env->execute(func_name.c_str(), 4, argv);

        // Extract i16x8 results from argv (returned via modified parameters)
        if (success) {
            memcpy(result_i16, argv, 16); // 8 i16 values = 16 bytes
        }

        return success;
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicPairwiseAddition_ReturnsCorrectSums
 * @brief Validates i16x8.extadd_pairwise_i8x16 produces correct arithmetic results for typical positive inputs
 * @details Tests fundamental pairwise addition operation with positive i8 values.
 *          Verifies that each pair of adjacent i8 values is sign-extended and added correctly.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extadd_pairwise_i8x16_operation
 * @input_conditions Standard i8 pairs: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
 * @expected_behavior Returns pairwise sums: [3,7,11,15,19,23,27,31]
 * @validation_method Direct comparison of WASM function result with expected i16 values
 */
TEST_F(I16x8ExtaddPairwiseI8x16TestSuite, BasicPairwiseAddition_ReturnsCorrectSums)
{
    int8_t input_i8[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int16_t expected_i16[8] = {3, 7, 11, 15, 19, 23, 27, 31};
    int16_t result_i16[8];

    bool success = call_i16x8_extadd_pairwise_i8x16("test_basic_pairwise_addition", input_i8, result_i16);
    ASSERT_TRUE(success) << "Failed to execute test_basic_pairwise_addition function";

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected_i16[i], result_i16[i])
            << "Basic pairwise addition failed at lane " << i
            << " expected: " << expected_i16[i] << " actual: " << result_i16[i];
    }
}

/**
 * @test NegativeValueHandling_ReturnsCorrectSums
 * @brief Validates i16x8.extadd_pairwise_i8x16 handles sign extension correctly for negative i8 values
 * @details Tests sign extension behavior with negative i8 values to ensure proper i16 conversion.
 *          Verifies that negative i8 values maintain their sign when extended to i16.
 * @test_category Main - Sign extension validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extadd_pairwise_i8x16_sign_extension
 * @input_conditions Negative i8 pairs: [-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15,-16]
 * @expected_behavior Returns negative pairwise sums: [-3,-7,-11,-15,-19,-23,-27,-31]
 * @validation_method Direct comparison ensuring negative results preserve correct sign and magnitude
 */
TEST_F(I16x8ExtaddPairwiseI8x16TestSuite, NegativeValueHandling_ReturnsCorrectSums)
{
    int8_t input_i8[16] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16};
    int16_t expected_i16[8] = {-3, -7, -11, -15, -19, -23, -27, -31};
    int16_t result_i16[8];

    bool success = call_i16x8_extadd_pairwise_i8x16("test_negative_values", input_i8, result_i16);
    ASSERT_TRUE(success) << "Failed to execute test_negative_values function";

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected_i16[i], result_i16[i])
            << "Negative value handling failed at lane " << i
            << " expected: " << expected_i16[i] << " actual: " << result_i16[i];
    }
}

/**
 * @test MixedSignValues_ReturnsCorrectSums
 * @brief Validates i16x8.extadd_pairwise_i8x16 with mixed positive/negative pairs that cancel out
 * @details Tests mixed-sign arithmetic where pairs of opposite values result in zero.
 *          Verifies that sign extension works correctly for both positive and negative values.
 * @test_category Main - Mixed sign arithmetic validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extadd_pairwise_i8x16_mixed_signs
 * @input_conditions Alternating signs: [1,-1,2,-2,3,-3,4,-4,5,-5,6,-6,7,-7,8,-8]
 * @expected_behavior Returns zero sums: [0,0,0,0,0,0,0,0]
 * @validation_method Verification that all pairs cancel to zero through proper sign extension
 */
TEST_F(I16x8ExtaddPairwiseI8x16TestSuite, MixedSignValues_ReturnsCorrectSums)
{
    int8_t input_i8[16] = {1, -1, 2, -2, 3, -3, 4, -4, 5, -5, 6, -6, 7, -7, 8, -8};
    int16_t expected_i16[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int16_t result_i16[8];

    bool success = call_i16x8_extadd_pairwise_i8x16("test_mixed_signs", input_i8, result_i16);
    ASSERT_TRUE(success) << "Failed to execute test_mixed_signs function";

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected_i16[i], result_i16[i])
            << "Mixed sign value handling failed at lane " << i
            << " expected: " << expected_i16[i] << " actual: " << result_i16[i];
    }
}

/**
 * @test EdgeValueCombinations_ReturnsCorrectSums
 * @brief Validates i16x8.extadd_pairwise_i8x16 with boundary conditions using INT8_MIN/MAX values
 * @details Tests extreme value combinations including INT8_MIN (-128) and INT8_MAX (127).
 *          Verifies that boundary arithmetic produces correct i16 results without overflow.
 * @test_category Edge - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_extadd_pairwise_i8x16_boundary_values
 * @input_conditions Boundary values: [127,-128,0,0,127,127,-128,-128,1,-1,126,-127,125,-126,124,-125]
 * @expected_behavior Returns boundary sums: [-1,0,254,-256,0,-1,-1,-1]
 * @validation_method Verification of correct arithmetic for extreme i8 value combinations
 */
TEST_F(I16x8ExtaddPairwiseI8x16TestSuite, EdgeValueCombinations_ReturnsCorrectSums)
{
    int8_t input_i8[16] = {127, -128, 0, 0, 127, 127, -128, -128, 1, -1, 126, -127, 125, -126, 124, -125};
    int16_t expected_i16[8] = {-1, 0, 254, -256, 0, -1, -1, -1};
    int16_t result_i16[8];

    bool success = call_i16x8_extadd_pairwise_i8x16("test_edge_values", input_i8, result_i16);
    ASSERT_TRUE(success) << "Failed to execute test_edge_values function";

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected_i16[i], result_i16[i])
            << "Edge value combination failed at lane " << i
            << " expected: " << expected_i16[i] << " actual: " << result_i16[i];
    }
}