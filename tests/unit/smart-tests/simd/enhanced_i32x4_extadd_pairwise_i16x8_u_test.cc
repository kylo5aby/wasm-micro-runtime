/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <climits>
#include <cstring>
#include <memory>

#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @brief Test fixture for i32x4.extadd_pairwise_i16x8_u opcode tests
 *
 * This test suite validates the i32x4.extadd_pairwise_i16x8_u SIMD instruction which performs
 * pairwise addition of adjacent unsigned 16-bit integers in an i16x8 vector, extending each
 * sum result to 32-bit integers to form an i32x4 vector. The operation processes pairs
 * (lane0+lane1), (lane2+lane3), (lane4+lane5), (lane6+lane7) and extends results to prevent overflow.
 */
class I32x4ExtaddPairwiseI16x8UTest : public testing::Test
{
  protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     *
     * Initializes WAMR with support for interpreter, AOT, and SIMD operations.
     * Loads the test WASM module containing i32x4.extadd_pairwise_i16x8_u test functions.
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i32x4.extadd_pairwise_i16x8_u test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i32x4_extadd_pairwise_i16x8_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i32x4.extadd_pairwise_i16x8_u tests";
    }

    /**
     * @brief Clean up test environment and release WAMR resources
     *
     * Properly destroys execution environment and WAMR runtime
     * using RAII pattern to prevent memory leaks.
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM test functions with i16x8 vector
     *
     * @param func_name Name of the WASM function to call
     * @param input i16x8 vector as array of 8 uint16_t values
     * @param result Output i32x4 vector as array of 4 uint32_t values
     * @return true if function execution succeeded, false otherwise
     */
    bool call_wasm_function(const char* func_name, const uint16_t input[8], uint32_t result[4])
    {
        wasm_exec_env_t exec_env = dummy_env->get();
        wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);

        wasm_function_inst_t func_inst =
            wasm_runtime_lookup_function(module_inst, func_name);
        if (!func_inst) {
            return false;
        }

        // Pack i16x8 vector into v128 parameter
        uint32_t argv[4];
        memcpy(argv, input, 16);      // 16 bytes for input vector

        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 4, argv);
        if (!call_result) {
            return false;
        }

        // Extract i32x4 result from return value
        memcpy(result, argv, 16);

        return call_result;
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicPairwiseAddition_ProducesCorrectSums
 * @brief Validates i32x4.extadd_pairwise_i16x8_u produces correct pairwise sums for typical inputs
 * @details Tests fundamental pairwise addition operation with sequential and mixed values.
 *          Verifies that adjacent lanes are correctly paired: (lane0+lane1), (lane2+lane3), etc.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extadd_pairwise_i16x8_u
 * @input_conditions Sequential i16x8 vector [1,2,3,4,5,6,7,8] and mixed values
 * @expected_behavior Returns i32x4 vector [3,7,11,15] for sequential input
 * @validation_method Direct comparison of WASM function result with expected pairwise sums
 */
TEST_F(I32x4ExtaddPairwiseI16x8UTest, BasicPairwiseAddition_ProducesCorrectSums)
{
    // Test sequential values: [1,2,3,4,5,6,7,8] → [3,7,11,15]
    uint16_t input_sequential[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint32_t result_sequential[4];
    ASSERT_TRUE(call_wasm_function("basic_pairwise", input_sequential, result_sequential))
        << "Failed to execute basic_pairwise function";

    ASSERT_EQ(3u, result_sequential[0]) << "Lane 0: (1+2) should equal 3";
    ASSERT_EQ(7u, result_sequential[1]) << "Lane 1: (3+4) should equal 7";
    ASSERT_EQ(11u, result_sequential[2]) << "Lane 2: (5+6) should equal 11";
    ASSERT_EQ(15u, result_sequential[3]) << "Lane 3: (7+8) should equal 15";

    // Test mixed values: [100,200,1000,2000,5,15,300,700] → [300,3000,20,1000]
    uint16_t input_mixed[] = {100, 200, 1000, 2000, 5, 15, 300, 700};
    uint32_t result_mixed[4];
    ASSERT_TRUE(call_wasm_function("basic_pairwise", input_mixed, result_mixed))
        << "Failed to execute basic_pairwise function with mixed values";

    ASSERT_EQ(300u, result_mixed[0]) << "Lane 0: (100+200) should equal 300";
    ASSERT_EQ(3000u, result_mixed[1]) << "Lane 1: (1000+2000) should equal 3000";
    ASSERT_EQ(20u, result_mixed[2]) << "Lane 2: (5+15) should equal 20";
    ASSERT_EQ(1000u, result_mixed[3]) << "Lane 3: (300+700) should equal 1000";

    // Test repeated patterns: [10,10,20,20,30,30,40,40] → [20,40,60,80]
    uint16_t input_repeated[] = {10, 10, 20, 20, 30, 30, 40, 40};
    uint32_t result_repeated[4];
    ASSERT_TRUE(call_wasm_function("basic_pairwise", input_repeated, result_repeated))
        << "Failed to execute basic_pairwise function with repeated patterns";

    ASSERT_EQ(20u, result_repeated[0]) << "Lane 0: (10+10) should equal 20";
    ASSERT_EQ(40u, result_repeated[1]) << "Lane 1: (20+20) should equal 40";
    ASSERT_EQ(60u, result_repeated[2]) << "Lane 2: (30+30) should equal 60";
    ASSERT_EQ(80u, result_repeated[3]) << "Lane 3: (40+40) should equal 80";
}

/**
 * @test BoundaryValues_HandleExtremeInputs
 * @brief Validates boundary condition handling with minimum and maximum 16-bit unsigned values
 * @details Tests operation behavior at numeric boundaries including max sums and mixed min/max pairs.
 *          Verifies proper extension to 32-bit without overflow issues.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extadd_pairwise_i16x8_u
 * @input_conditions Maximum values (65535), minimum values (0), and mixed boundary patterns
 * @expected_behavior Correct sums: max pair = 131070, min pair = 0, mixed = 65535
 * @validation_method Verification of proper 16-bit to 32-bit extension without overflow
 */
TEST_F(I32x4ExtaddPairwiseI16x8UTest, BoundaryValues_HandleExtremeInputs)
{
    // Test maximum values: [65535,65535,65535,65535,65535,65535,65535,65535] → [131070,131070,131070,131070]
    uint16_t input_max[] = {65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535};
    uint32_t result_max[4];
    ASSERT_TRUE(call_wasm_function("boundary_values", input_max, result_max))
        << "Failed to execute boundary_values function with maximum values";

    ASSERT_EQ(131070u, result_max[0]) << "Lane 0: (65535+65535) should equal 131070";
    ASSERT_EQ(131070u, result_max[1]) << "Lane 1: (65535+65535) should equal 131070";
    ASSERT_EQ(131070u, result_max[2]) << "Lane 2: (65535+65535) should equal 131070";
    ASSERT_EQ(131070u, result_max[3]) << "Lane 3: (65535+65535) should equal 131070";

    // Test minimum values: [0,0,0,0,0,0,0,0] → [0,0,0,0]
    uint16_t input_min[] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t result_min[4];
    ASSERT_TRUE(call_wasm_function("boundary_values", input_min, result_min))
        << "Failed to execute boundary_values function with minimum values";

    ASSERT_EQ(0u, result_min[0]) << "Lane 0: (0+0) should equal 0";
    ASSERT_EQ(0u, result_min[1]) << "Lane 1: (0+0) should equal 0";
    ASSERT_EQ(0u, result_min[2]) << "Lane 2: (0+0) should equal 0";
    ASSERT_EQ(0u, result_min[3]) << "Lane 3: (0+0) should equal 0";

    // Test mixed min/max: [0,65535,65535,0,0,65535,65535,0] → [65535,65535,65535,65535]
    uint16_t input_mixed[] = {0, 65535, 65535, 0, 0, 65535, 65535, 0};
    uint32_t result_mixed[4];
    ASSERT_TRUE(call_wasm_function("boundary_values", input_mixed, result_mixed))
        << "Failed to execute boundary_values function with mixed min/max values";

    ASSERT_EQ(65535u, result_mixed[0]) << "Lane 0: (0+65535) should equal 65535";
    ASSERT_EQ(65535u, result_mixed[1]) << "Lane 1: (65535+0) should equal 65535";
    ASSERT_EQ(65535u, result_mixed[2]) << "Lane 2: (0+65535) should equal 65535";
    ASSERT_EQ(65535u, result_mixed[3]) << "Lane 3: (65535+0) should equal 65535";
}

/**
 * @test ZeroOperands_ReturnExpectedResults
 * @brief Validates handling of zero values in various lane positions
 * @details Tests behavior with all-zero inputs, selective zeros, and zero-sum pairs.
 *          Verifies that zero values are properly handled without affecting adjacent lanes.
 * @test_category Edge - Zero value handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extadd_pairwise_i16x8_u
 * @input_conditions All zeros, selective zero positions, and zero within pairs
 * @expected_behavior Correct zero propagation and non-zero lane preservation
 * @validation_method Verification of zero handling across different lane configurations
 */
TEST_F(I32x4ExtaddPairwiseI16x8UTest, ZeroOperands_ReturnExpectedResults)
{
    // Test all zeros: [0,0,0,0,0,0,0,0] → [0,0,0,0]
    uint16_t input_all_zero[] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t result_all_zero[4];
    ASSERT_TRUE(call_wasm_function("zero_operands", input_all_zero, result_all_zero))
        << "Failed to execute zero_operands function with all zero input";

    ASSERT_EQ(0u, result_all_zero[0]) << "Lane 0: (0+0) should equal 0";
    ASSERT_EQ(0u, result_all_zero[1]) << "Lane 1: (0+0) should equal 0";
    ASSERT_EQ(0u, result_all_zero[2]) << "Lane 2: (0+0) should equal 0";
    ASSERT_EQ(0u, result_all_zero[3]) << "Lane 3: (0+0) should equal 0";

    // Test selective zeros: [0,1,2,0,0,3,4,0] → [1,2,3,4]
    uint16_t input_selective[] = {0, 1, 2, 0, 0, 3, 4, 0};
    uint32_t result_selective[4];
    ASSERT_TRUE(call_wasm_function("zero_operands", input_selective, result_selective))
        << "Failed to execute zero_operands function with selective zeros";

    ASSERT_EQ(1u, result_selective[0]) << "Lane 0: (0+1) should equal 1";
    ASSERT_EQ(2u, result_selective[1]) << "Lane 1: (2+0) should equal 2";
    ASSERT_EQ(3u, result_selective[2]) << "Lane 2: (0+3) should equal 3";
    ASSERT_EQ(4u, result_selective[3]) << "Lane 3: (4+0) should equal 4";

    // Test zero sum pairs: [5,0,0,7,3,0,0,2] → [5,7,3,2]
    uint16_t input_zero_pairs[] = {5, 0, 0, 7, 3, 0, 0, 2};
    uint32_t result_zero_pairs[4];
    ASSERT_TRUE(call_wasm_function("zero_operands", input_zero_pairs, result_zero_pairs))
        << "Failed to execute zero_operands function with zero pair patterns";

    ASSERT_EQ(5u, result_zero_pairs[0]) << "Lane 0: (5+0) should equal 5";
    ASSERT_EQ(7u, result_zero_pairs[1]) << "Lane 1: (0+7) should equal 7";
    ASSERT_EQ(3u, result_zero_pairs[2]) << "Lane 2: (3+0) should equal 3";
    ASSERT_EQ(2u, result_zero_pairs[3]) << "Lane 3: (0+2) should equal 2";
}

/**
 * @test MaximumValues_ExtendProperly
 * @brief Validates maximum value handling and proper extension to 32-bit results
 * @details Tests extreme value scenarios including maximum sums, alternating extremes, and power-of-2 patterns.
 *          Verifies that the unsigned-to-signed extension maintains correct numeric values.
 * @test_category Edge - Extreme value and extension validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extadd_pairwise_i16x8_u
 * @input_conditions Maximum combinations, alternating patterns, and power-of-2 sequences
 * @expected_behavior Proper 16-bit to 32-bit extension without data loss or overflow
 * @validation_method Verification of extension correctness for extreme value combinations
 */
TEST_F(I32x4ExtaddPairwiseI16x8UTest, MaximumValues_ExtendProperly)
{
    // Test alternating extremes: [65535,0,0,65535,65535,0,0,65535] → [65535,65535,65535,65535]
    uint16_t input_alternating[] = {65535, 0, 0, 65535, 65535, 0, 0, 65535};
    uint32_t result_alternating[4];
    ASSERT_TRUE(call_wasm_function("maximum_values", input_alternating, result_alternating))
        << "Failed to execute maximum_values function with alternating patterns";

    ASSERT_EQ(65535u, result_alternating[0]) << "Lane 0: (65535+0) should equal 65535";
    ASSERT_EQ(65535u, result_alternating[1]) << "Lane 1: (0+65535) should equal 65535";
    ASSERT_EQ(65535u, result_alternating[2]) << "Lane 2: (65535+0) should equal 65535";
    ASSERT_EQ(65535u, result_alternating[3]) << "Lane 3: (0+65535) should equal 65535";

    // Test power of 2 patterns: [32768,32768,16384,16384,8192,8192,4096,4096] → [65536,32768,16384,8192]
    uint16_t input_powers[] = {32768, 32768, 16384, 16384, 8192, 8192, 4096, 4096};
    uint32_t result_powers[4];
    ASSERT_TRUE(call_wasm_function("maximum_values", input_powers, result_powers))
        << "Failed to execute maximum_values function with power of 2 patterns";

    ASSERT_EQ(65536u, result_powers[0]) << "Lane 0: (32768+32768) should equal 65536";
    ASSERT_EQ(32768u, result_powers[1]) << "Lane 1: (16384+16384) should equal 32768";
    ASSERT_EQ(16384u, result_powers[2]) << "Lane 2: (8192+8192) should equal 16384";
    ASSERT_EQ(8192u, result_powers[3]) << "Lane 3: (4096+4096) should equal 8192";

    // Test identity values: [1,0,0,1,1,0,0,1] → [1,1,1,1]
    uint16_t input_identity[] = {1, 0, 0, 1, 1, 0, 0, 1};
    uint32_t result_identity[4];
    ASSERT_TRUE(call_wasm_function("maximum_values", input_identity, result_identity))
        << "Failed to execute maximum_values function with identity patterns";

    ASSERT_EQ(1u, result_identity[0]) << "Lane 0: (1+0) should equal 1";
    ASSERT_EQ(1u, result_identity[1]) << "Lane 1: (0+1) should equal 1";
    ASSERT_EQ(1u, result_identity[2]) << "Lane 2: (1+0) should equal 1";
    ASSERT_EQ(1u, result_identity[3]) << "Lane 3: (0+1) should equal 1";
}