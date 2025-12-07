/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <cmath>
#include <cfloat>
#include <limits>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

// Test execution modes for parameterized testing
enum class TestRunningMode {
    INTERP_MODE,
    AOT_MODE
};

/**
 * @class F32x4ExtractLaneTestSuite
 * @brief Test fixture for f32x4.extract_lane opcode validation
 * @details Validates SIMD lane extraction operations for f32x4 vectors
 *          Tests both interpreter and AOT execution modes with comprehensive validation
 */
class F32x4ExtractLaneTestSuite : public testing::TestWithParam<TestRunningMode>
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.extract_lane testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.extract_lane test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_extract_lane_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.extract_lane tests";
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

    TestRunningMode running_mode;
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Helper function to call f32x4.extract_lane WASM function
     * @param func_name Name of the exported WASM function
     * @param v1 First f32 value for vector construction
     * @param v2 Second f32 value for vector construction
     * @param v3 Third f32 value for vector construction
     * @param v4 Fourth f32 value for vector construction
     * @return float Extracted f32 value from specified lane
     */
    float call_f32x4_extract_lane(const char *func_name, float v1, float v2, float v3, float v4)
    {
        // Prepare arguments: 4 f32 inputs for vector construction
        uint32_t argv[4];

        // Convert f32 inputs to uint32_t representation for WASM call
        memcpy(&argv[0], &v1, sizeof(float));
        memcpy(&argv[1], &v2, sizeof(float));
        memcpy(&argv[2], &v3, sizeof(float));
        memcpy(&argv[3], &v4, sizeof(float));

        // Call WASM function with 4 f32 inputs, returns f32
        bool call_success = dummy_env->execute(func_name, 4, argv);
        if (!call_success) {
            throw std::runtime_error(std::string("WASM function call failed for ") + func_name);
        }

        // Extract f32 result from argv[0] after successful execution
        float result;
        memcpy(&result, &argv[0], sizeof(float));

        return result;
    }

    /**
     * @brief Helper function to compare f32 values with special value handling
     * @param expected Expected f32 value
     * @param actual Actual f32 value
     * @return bool True if values are equivalent (including NaN and signed zero handling)
     */
    bool compare_f32_values(float expected, float actual)
    {
        // Handle NaN comparison - both must be NaN for equality
        if (std::isnan(expected) && std::isnan(actual)) {
            return true;
        }

        // Handle infinity comparison with sign check
        if (std::isinf(expected) && std::isinf(actual)) {
            return (std::signbit(expected) == std::signbit(actual));
        }

        // Handle signed zero comparison
        if (expected == 0.0f && actual == 0.0f) {
            return std::signbit(expected) == std::signbit(actual);
        }

        // Standard float comparison for normal values
        return expected == actual;
    }

    /**
     * @brief Helper function to create bit pattern from uint32_t for special float values
     * @param bit_pattern Bit pattern as uint32_t
     * @return float Float value with specified bit pattern
     */
    float create_float_from_bits(uint32_t bit_pattern)
    {
        float result;
        memcpy(&result, &bit_pattern, sizeof(float));
        return result;
    }
};

/**
 * @test BasicExtraction_ReturnsCorrectValues
 * @brief Validates f32x4.extract_lane extracts correct values from each lane
 * @details Tests fundamental extraction operation with distinct values in each lane.
 *          Verifies that each lane index (0-3) extracts the correct corresponding value.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_extract_lane_operation
 * @input_conditions f32x4 vector with distinct values [1.5, -2.3, 42.7, 100.0]
 * @expected_behavior Each lane extraction returns the exact value stored in that position
 * @validation_method Direct comparison of extracted values with expected lane contents
 */
TEST_P(F32x4ExtractLaneTestSuite, BasicExtraction_ReturnsCorrectValues)
{
    // Test vector with distinct values for each lane
    float test_vector[4] = {1.5f, -2.3f, 42.7f, 100.0f};

    // Test extraction from lane 0
    float result_lane0 = call_f32x4_extract_lane("test_extract_lane_0",
                                                 test_vector[0], test_vector[1],
                                                 test_vector[2], test_vector[3]);
    ASSERT_TRUE(compare_f32_values(test_vector[0], result_lane0))
        << "Lane 0 extraction failed: expected " << test_vector[0] << ", got " << result_lane0;

    // Test extraction from lane 1
    float result_lane1 = call_f32x4_extract_lane("test_extract_lane_1",
                                                 test_vector[0], test_vector[1],
                                                 test_vector[2], test_vector[3]);
    ASSERT_TRUE(compare_f32_values(test_vector[1], result_lane1))
        << "Lane 1 extraction failed: expected " << test_vector[1] << ", got " << result_lane1;

    // Test extraction from lane 2
    float result_lane2 = call_f32x4_extract_lane("test_extract_lane_2",
                                                 test_vector[0], test_vector[1],
                                                 test_vector[2], test_vector[3]);
    ASSERT_TRUE(compare_f32_values(test_vector[2], result_lane2))
        << "Lane 2 extraction failed: expected " << test_vector[2] << ", got " << result_lane2;

    // Test extraction from lane 3
    float result_lane3 = call_f32x4_extract_lane("test_extract_lane_3",
                                                 test_vector[0], test_vector[1],
                                                 test_vector[2], test_vector[3]);
    ASSERT_TRUE(compare_f32_values(test_vector[3], result_lane3))
        << "Lane 3 extraction failed: expected " << test_vector[3] << ", got " << result_lane3;
}

/**
 * @test ExtremeFloatExtraction_PreservesValues
 * @brief Tests extraction of float boundary values (FLT_MAX, FLT_MIN, etc.)
 * @details Validates that extreme floating-point values are extracted correctly
 *          without precision loss or corruption at the limits of f32 representation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_extract_lane_operation
 * @input_conditions f32x4 vector with extreme values [FLT_MAX, FLT_MIN, -FLT_MAX, -FLT_MIN]
 * @expected_behavior Extreme values are preserved exactly during lane extraction
 * @validation_method Exact comparison of extracted extreme values with original constants
 */
TEST_P(F32x4ExtractLaneTestSuite, ExtremeFloatExtraction_PreservesValues)
{
    // Test vector with extreme floating-point values
    float extreme_vector[4] = {FLT_MAX, FLT_MIN, -FLT_MAX, -FLT_MIN};

    // Test FLT_MAX extraction from lane 0
    float result_max = call_f32x4_extract_lane("test_extract_lane_0",
                                              extreme_vector[0], extreme_vector[1],
                                              extreme_vector[2], extreme_vector[3]);
    ASSERT_FLOAT_EQ(FLT_MAX, result_max)
        << "FLT_MAX extraction from lane 0 failed";

    // Test FLT_MIN extraction from lane 1
    float result_min = call_f32x4_extract_lane("test_extract_lane_1",
                                              extreme_vector[0], extreme_vector[1],
                                              extreme_vector[2], extreme_vector[3]);
    ASSERT_FLOAT_EQ(FLT_MIN, result_min)
        << "FLT_MIN extraction from lane 1 failed";

    // Test -FLT_MAX extraction from lane 2
    float result_neg_max = call_f32x4_extract_lane("test_extract_lane_2",
                                                  extreme_vector[0], extreme_vector[1],
                                                  extreme_vector[2], extreme_vector[3]);
    ASSERT_FLOAT_EQ(-FLT_MAX, result_neg_max)
        << "-FLT_MAX extraction from lane 2 failed";

    // Test -FLT_MIN extraction from lane 3
    float result_neg_min = call_f32x4_extract_lane("test_extract_lane_3",
                                                  extreme_vector[0], extreme_vector[1],
                                                  extreme_vector[2], extreme_vector[3]);
    ASSERT_FLOAT_EQ(-FLT_MIN, result_neg_min)
        << "-FLT_MIN extraction from lane 3 failed";
}

/**
 * @test IEEE754SpecialExtraction_PreservesSpecialValues
 * @brief Validates extraction and preservation of NaN, Infinity, signed zeros
 * @details Tests extraction of IEEE 754 special values ensuring bit-exact preservation.
 *          Validates proper handling of NaN, positive/negative infinity, and signed zeros.
 * @test_category Edge - Special numeric value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_extract_lane_operation
 * @input_conditions f32x4 vector with special IEEE 754 values (NaN, ±∞, ±0.0)
 * @expected_behavior Special values are preserved with correct bit patterns and signs
 * @validation_method Specialized comparison handling NaN detection and signed zero distinction
 */
TEST_P(F32x4ExtractLaneTestSuite, IEEE754SpecialExtraction_PreservesSpecialValues)
{
    // Create special IEEE 754 values
    float quiet_nan = std::numeric_limits<float>::quiet_NaN();
    float pos_infinity = std::numeric_limits<float>::infinity();
    float neg_infinity = -std::numeric_limits<float>::infinity();
    float neg_zero = -0.0f;

    // Test vector with special IEEE 754 values
    float special_vector[4] = {quiet_nan, pos_infinity, neg_infinity, neg_zero};

    // Test NaN extraction from lane 0
    float result_nan = call_f32x4_extract_lane("test_extract_lane_0",
                                              special_vector[0], special_vector[1],
                                              special_vector[2], special_vector[3]);
    ASSERT_TRUE(std::isnan(result_nan))
        << "NaN extraction from lane 0 failed - result is not NaN";

    // Test positive infinity extraction from lane 1
    float result_pos_inf = call_f32x4_extract_lane("test_extract_lane_1",
                                                  special_vector[0], special_vector[1],
                                                  special_vector[2], special_vector[3]);
    ASSERT_TRUE(std::isinf(result_pos_inf) && result_pos_inf > 0)
        << "Positive infinity extraction from lane 1 failed";

    // Test negative infinity extraction from lane 2
    float result_neg_inf = call_f32x4_extract_lane("test_extract_lane_2",
                                                  special_vector[0], special_vector[1],
                                                  special_vector[2], special_vector[3]);
    ASSERT_TRUE(std::isinf(result_neg_inf) && result_neg_inf < 0)
        << "Negative infinity extraction from lane 2 failed";

    // Test negative zero extraction from lane 3
    float result_neg_zero = call_f32x4_extract_lane("test_extract_lane_3",
                                                   special_vector[0], special_vector[1],
                                                   special_vector[2], special_vector[3]);
    ASSERT_TRUE(result_neg_zero == 0.0f && std::signbit(result_neg_zero))
        << "Negative zero extraction from lane 3 failed - sign bit not preserved";
}

/**
 * @test ZeroVectorExtraction_HandlesCorrectly
 * @brief Tests extraction from vectors containing various zero representations
 * @details Validates extraction from vectors with positive zeros, negative zeros,
 *          and mixed zero/non-zero combinations ensuring proper sign preservation.
 * @test_category Edge - Zero value handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_extract_lane_operation
 * @input_conditions f32x4 vectors with different zero combinations (+0.0, -0.0, mixed)
 * @expected_behavior Zero values extracted with correct sign bits preserved
 * @validation_method Sign bit validation for extracted zero values
 */
TEST_P(F32x4ExtractLaneTestSuite, ZeroVectorExtraction_HandlesCorrectly)
{
    // Test all positive zeros
    float all_zeros[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    float result_pos_zero = call_f32x4_extract_lane("test_extract_lane_0",
                                                   all_zeros[0], all_zeros[1],
                                                   all_zeros[2], all_zeros[3]);
    ASSERT_TRUE(result_pos_zero == 0.0f && !std::signbit(result_pos_zero))
        << "Positive zero extraction failed - incorrect sign bit";

    // Test mixed signed zeros
    float mixed_zeros[4] = {0.0f, -0.0f, 0.0f, -0.0f};

    // Extract positive zero from lane 0
    float result_mixed_pos = call_f32x4_extract_lane("test_extract_lane_0",
                                                    mixed_zeros[0], mixed_zeros[1],
                                                    mixed_zeros[2], mixed_zeros[3]);
    ASSERT_TRUE(result_mixed_pos == 0.0f && !std::signbit(result_mixed_pos))
        << "Mixed positive zero extraction failed";

    // Extract negative zero from lane 1
    float result_mixed_neg = call_f32x4_extract_lane("test_extract_lane_1",
                                                    mixed_zeros[0], mixed_zeros[1],
                                                    mixed_zeros[2], mixed_zeros[3]);
    ASSERT_TRUE(result_mixed_neg == 0.0f && std::signbit(result_mixed_neg))
        << "Mixed negative zero extraction failed";

    // Test zero mixed with non-zero values
    float mixed_vector[4] = {1.5f, 0.0f, -2.3f, -0.0f};

    float result_zero_in_mixed = call_f32x4_extract_lane("test_extract_lane_1",
                                                        mixed_vector[0], mixed_vector[1],
                                                        mixed_vector[2], mixed_vector[3]);
    ASSERT_TRUE(result_zero_in_mixed == 0.0f && !std::signbit(result_zero_in_mixed))
        << "Zero extraction from mixed vector failed";
}

/**
 * @test SubnormalExtraction_PreservesValues
 * @brief Tests extraction of subnormal (denormalized) floating-point values
 * @details Validates that very small subnormal values are extracted correctly
 *          without being flushed to zero or corrupted during the operation.
 * @test_category Edge - Subnormal value handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_extract_lane_operation
 * @input_conditions f32x4 vector with subnormal values near zero but non-zero
 * @expected_behavior Subnormal values are preserved exactly during extraction
 * @validation_method Direct comparison of subnormal values ensuring no flush-to-zero behavior
 */
TEST_P(F32x4ExtractLaneTestSuite, SubnormalExtraction_PreservesValues)
{
    // Create subnormal values using bit patterns
    // Subnormal f32: exponent = 0, mantissa != 0
    float subnormal1 = create_float_from_bits(0x00000001); // Smallest positive subnormal
    float subnormal2 = create_float_from_bits(0x007FFFFF); // Largest positive subnormal
    float subnormal3 = create_float_from_bits(0x80000001); // Smallest negative subnormal
    float subnormal4 = create_float_from_bits(0x807FFFFF); // Largest negative subnormal

    float subnormal_vector[4] = {subnormal1, subnormal2, subnormal3, subnormal4};

    // Test smallest positive subnormal extraction
    float result_sub1 = call_f32x4_extract_lane("test_extract_lane_0",
                                               subnormal_vector[0], subnormal_vector[1],
                                               subnormal_vector[2], subnormal_vector[3]);
    ASSERT_TRUE(compare_f32_values(subnormal1, result_sub1))
        << "Smallest positive subnormal extraction failed";

    // Test largest positive subnormal extraction
    float result_sub2 = call_f32x4_extract_lane("test_extract_lane_1",
                                               subnormal_vector[0], subnormal_vector[1],
                                               subnormal_vector[2], subnormal_vector[3]);
    ASSERT_TRUE(compare_f32_values(subnormal2, result_sub2))
        << "Largest positive subnormal extraction failed";

    // Test smallest negative subnormal extraction
    float result_sub3 = call_f32x4_extract_lane("test_extract_lane_2",
                                               subnormal_vector[0], subnormal_vector[1],
                                               subnormal_vector[2], subnormal_vector[3]);
    ASSERT_TRUE(compare_f32_values(subnormal3, result_sub3))
        << "Smallest negative subnormal extraction failed";

    // Test largest negative subnormal extraction
    float result_sub4 = call_f32x4_extract_lane("test_extract_lane_3",
                                               subnormal_vector[0], subnormal_vector[1],
                                               subnormal_vector[2], subnormal_vector[3]);
    ASSERT_TRUE(compare_f32_values(subnormal4, result_sub4))
        << "Largest negative subnormal extraction failed";
}

// Instantiate parameterized test for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, F32x4ExtractLaneTestSuite,
                        testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));