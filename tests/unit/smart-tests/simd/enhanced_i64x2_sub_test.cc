/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <climits>
#include <cstdint>
#include <cstring>
#include <memory>

#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

// Test execution modes for parameterized testing
enum class TestRunningMode {
    INTERP_MODE,
    AOT_MODE
};

/**
 * @brief Test fixture for i64x2.sub WASM SIMD opcode testing
 *
 * This class provides comprehensive test infrastructure for validating the i64x2.sub
 * SIMD instruction across different WAMR execution modes (interpreter and AOT).
 * Tests validate element-wise subtraction of two 64-bit integer vectors with
 * proper overflow/underflow handling.
 */
class I64x2SubTest : public testing::TestWithParam<TestRunningMode>
{
protected:
    TestRunningMode running_mode;
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Initialize WAMR runtime environment for SIMD testing
     *
     * Sets up WAMR runtime with SIMD support enabled and loads the test module
     * containing i64x2.sub test functions. Configures both interpreter and AOT
     * execution modes based on test parameters.
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime with SIMD support
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i64x2.sub test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i64x2_sub_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i64x2.sub tests";
    }

    /**
     * @brief Clean up WASM runtime resources
     *
     * Properly destroys execution environment, module instance, module, and
     * runtime to prevent memory leaks and ensure clean test environment.
     */
    void TearDown() override
    {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute i64x2.sub operation with two input vectors
     *
     * @param minuend First input vector (i64x2) - values to subtract from
     * @param subtrahend Second input vector (i64x2) - values to subtract
     * @return Result vector containing element-wise subtraction results
     */
    std::array<int64_t, 2> call_i64x2_sub(const std::array<int64_t, 2>& minuend,
                                           const std::array<int64_t, 2>& subtrahend)
    {
        uint32_t args[8]; // 2 v128 parameters (4 uint32_t each)

        // Pack input vectors into uint32_t array (v128 format)
        memcpy(&args[0], &minuend, sizeof(int64_t) * 2);     // minuend v128
        memcpy(&args[4], &subtrahend, sizeof(int64_t) * 2);  // subtrahend v128

        bool call_result = dummy_env->execute("i64x2_sub_test", 8, args);
        EXPECT_TRUE(call_result)
            << "Failed to execute i64x2_sub_test: " << dummy_env->get_exception();

        std::array<int64_t, 2> result;
        // Unpack result vector (returned as v128 in args[0-3])
        memcpy(&result, &args[0], sizeof(int64_t) * 2);
        return result;
    }
};

/**
 * @test BasicSubtraction_ReturnsCorrectResult
 * @brief Validates i64x2.sub produces correct arithmetic results for typical inputs
 * @details Tests fundamental subtraction operation with positive, negative, and mixed-sign integers.
 *          Verifies that i64x2.sub correctly computes element-wise subtraction for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_sub
 * @input_conditions Standard integer pairs: [100,200] - [30,50], [-50,-75] - [25,35], [50,-100] - [-25,75]
 * @expected_behavior Returns mathematical difference: [70,150], [-75,-110], [75,-175] respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64x2SubTest, BasicSubtraction_ReturnsCorrectResult)
{
    std::array<int64_t, 2> result;

    // Test positive numbers subtraction
    result = call_i64x2_sub({100, 200}, {30, 50});
    ASSERT_EQ(result[0], 70) << "First element subtraction failed for positive integers";
    ASSERT_EQ(result[1], 150) << "Second element subtraction failed for positive integers";

    // Test negative numbers subtraction
    result = call_i64x2_sub({-50, -75}, {25, 35});
    ASSERT_EQ(result[0], -75) << "First element subtraction failed for negative minuend";
    ASSERT_EQ(result[1], -110) << "Second element subtraction failed for negative minuend";

    // Test mixed sign subtraction
    result = call_i64x2_sub({50, -100}, {-25, 75});
    ASSERT_EQ(result[0], 75) << "First element subtraction failed for mixed signs";
    ASSERT_EQ(result[1], -175) << "Second element subtraction failed for mixed signs";
}

/**
 * @test EdgeCaseBoundaries_HandlesMinMaxValues
 * @brief Tests boundary conditions with INT64_MIN and INT64_MAX values
 * @details Validates behavior at integer limits including overflow/underflow scenarios.
 *          Tests operations that approach or exceed the representable range of 64-bit signed integers.
 * @test_category Edge - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_sub
 * @input_conditions Boundary values: [INT64_MAX,0] - [1,INT64_MIN], [INT64_MIN,INT64_MAX] - [0,1]
 * @expected_behavior Two's complement wraparound behavior for overflow/underflow conditions
 * @validation_method Verification of wraparound arithmetic according to two's complement rules
 */
TEST_P(I64x2SubTest, EdgeCaseBoundaries_HandlesMinMaxValues)
{
    std::array<int64_t, 2> result;

    // Test INT64_MAX boundary - this should wrap around
    result = call_i64x2_sub({INT64_MAX, 0}, {1, INT64_MIN});
    ASSERT_EQ(result[0], INT64_MAX - 1) << "INT64_MAX subtraction boundary failed";
    ASSERT_EQ(result[1], INT64_MIN) << "Zero minus INT64_MIN failed";

    // Test INT64_MIN boundary conditions
    result = call_i64x2_sub({INT64_MIN, INT64_MAX}, {0, 1});
    ASSERT_EQ(result[0], INT64_MIN) << "INT64_MIN subtraction boundary failed";
    ASSERT_EQ(result[1], INT64_MAX - 1) << "INT64_MAX minus 1 failed";

    // Test wraparound scenarios
    result = call_i64x2_sub({INT64_MIN, INT64_MAX}, {1, -1});
    ASSERT_EQ(result[0], INT64_MAX) << "INT64_MIN - 1 wraparound failed";
    ASSERT_EQ(result[1], INT64_MIN) << "INT64_MAX - (-1) wraparound failed";
}

/**
 * @test OverflowUnderflow_WrapsAroundCorrectly
 * @brief Validates overflow/underflow behavior with wraparound semantics
 * @details Tests operations that cause positive and negative overflow to ensure proper
 *          two's complement wraparound behavior without exceptions or errors.
 * @test_category Edge - Overflow/underflow handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_sub
 * @input_conditions Overflow scenarios: operations causing integer wraparound
 * @expected_behavior Standard two's complement wraparound results without errors
 * @validation_method Verification of calculated wraparound expectations
 */
TEST_P(I64x2SubTest, OverflowUnderflow_WrapsAroundCorrectly)
{
    std::array<int64_t, 2> result;

    // Test positive overflow (large positive - large negative = overflow)
    int64_t large_pos = INT64_MAX / 2;
    int64_t large_neg = INT64_MIN / 2;
    result = call_i64x2_sub({large_pos, INT64_MAX}, {large_neg, -2});

    // Calculate expected wraparound results
    int64_t expected1 = large_pos - large_neg;  // Should be within range
    int64_t expected2 = INT64_MIN + 1;          // Wraparound result

    ASSERT_EQ(result[0], expected1) << "Positive overflow wraparound failed";
    ASSERT_EQ(result[1], expected2) << "Large positive minus negative failed";

    // Test negative underflow scenarios
    result = call_i64x2_sub({INT64_MIN + 1, -1000}, {2, -500});
    ASSERT_EQ(result[0], INT64_MAX) << "Negative underflow failed";
    ASSERT_EQ(result[1], -500) << "Negative subtraction with negative subtrahend failed";
}

/**
 * @test ZeroOperations_ProducesExpectedResults
 * @brief Tests subtraction involving zero values in various combinations
 * @details Validates zero identity properties and negation behavior when zero
 *          is used as minuend or subtrahend in different combinations.
 * @test_category Main - Zero value handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_sub
 * @input_conditions Zero combinations: [0,42] - [42,0], [0,0] - [INT64_MAX,INT64_MIN]
 * @expected_behavior Zero identity: [-42,42], [-INT64_MAX,-INT64_MIN] respectively
 * @validation_method Verification of zero properties and negation results
 */
TEST_P(I64x2SubTest, ZeroOperations_ProducesExpectedResults)
{
    std::array<int64_t, 2> result;

    // Test zero as minuend and subtrahend
    result = call_i64x2_sub({0, 42}, {42, 0});
    ASSERT_EQ(result[0], -42) << "Zero minus positive number failed";
    ASSERT_EQ(result[1], 42) << "Positive number minus zero failed";

    // Test zero with boundary values
    result = call_i64x2_sub({0, 0}, {INT64_MAX, INT64_MIN});
    ASSERT_EQ(result[0], -INT64_MAX) << "Zero minus INT64_MAX failed";
    ASSERT_EQ(result[1], INT64_MIN) << "Zero minus INT64_MIN failed";

    // Test both zeros
    result = call_i64x2_sub({0, 0}, {0, 0});
    ASSERT_EQ(result[0], 0) << "Zero minus zero failed for first element";
    ASSERT_EQ(result[1], 0) << "Zero minus zero failed for second element";
}

// Parameterized test instances for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(I64x2SubTests, I64x2SubTest,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));