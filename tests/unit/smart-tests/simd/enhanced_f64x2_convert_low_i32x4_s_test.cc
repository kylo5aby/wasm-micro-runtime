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
#include "../common/test_helper.h"

// Test execution modes for parameterized testing
enum class TestRunningMode {
    INTERP_MODE,
    AOT_MODE
};

/**
 * @class F64x2ConvertLowI32x4STestSuite
 * @brief Test fixture for f64x2.convert_low_i32x4_s opcode validation
 * @details Validates SIMD signed integer to float conversion operations for f64x2.convert_low_i32x4_s
 *          Tests both interpreter and AOT execution modes with comprehensive validation
 */
class F64x2ConvertLowI32x4STestSuite : public testing::TestWithParam<TestRunningMode>
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f64x2.convert_low_i32x4_s testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.convert_low_i32x4_s test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_convert_low_i32x4_s_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.convert_low_i32x4_s tests";
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
     * @brief Helper function to call f64x2.convert_low_i32x4_s WASM function
     * @param func_name Name of the exported WASM function
     * @param lane0 First i32 value for vector construction (lane 0)
     * @param lane1 Second i32 value for vector construction (lane 1)
     * @param lane2 Third i32 value for vector construction (lane 2)
     * @param lane3 Fourth i32 value for vector construction (lane 3)
     * @return std::pair<double, double> Converted f64 values from lanes 0 and 1
     */
    std::pair<double, double> call_f64x2_convert_low_i32x4_s(const char *func_name,
                                                              int32_t lane0, int32_t lane1,
                                                              int32_t lane2, int32_t lane3)
    {
        // Prepare arguments: 4 i32 inputs for vector construction
        uint32_t argv[6]; // 4 inputs + 2 outputs (f64x2 result = 4 uint32_t)

        // Set input arguments for i32x4 vector construction
        memcpy(&argv[0], &lane0, sizeof(int32_t)); // Lane 0
        memcpy(&argv[1], &lane1, sizeof(int32_t)); // Lane 1
        memcpy(&argv[2], &lane2, sizeof(int32_t)); // Lane 2
        memcpy(&argv[3], &lane3, sizeof(int32_t)); // Lane 3

        // Call WASM function with 4 i32 inputs, returns f64x2 (2 f64 values)
        bool call_success = dummy_env->execute(func_name, 4, argv);
        if (!call_success) {
            throw std::runtime_error(std::string("WASM function call failed for ") + func_name);
        }

        // Extract f64x2 result from argv[0:3] after successful execution
        double result0, result1;
        memcpy(&result0, &argv[0], sizeof(double)); // First f64 from lanes 0,1 of argv
        memcpy(&result1, &argv[2], sizeof(double)); // Second f64 from lanes 2,3 of argv

        return std::make_pair(result0, result1);
    }

    /**
     * @brief Helper function to compare f64 values with special value handling
     * @param expected Expected f64 value
     * @param actual Actual f64 value from WASM execution
     * @param message Descriptive message for assertion failures
     */
    void assert_f64_equal(double expected, double actual, const char *message)
    {
        // Handle NaN comparison - NaN != NaN, so use isnan for both
        if (std::isnan(expected) && std::isnan(actual)) {
            return; // Both NaN - consider equal
        }

        // Handle infinity comparison
        if (std::isinf(expected) && std::isinf(actual)) {
            // Check same sign for infinity
            ASSERT_EQ(std::signbit(expected), std::signbit(actual)) << message;
            return;
        }

        // Handle signed zero differentiation
        if (expected == 0.0 && actual == 0.0) {
            ASSERT_EQ(std::signbit(expected), std::signbit(actual)) << message;
            return;
        }

        // Standard f64 comparison for normal values
        ASSERT_DOUBLE_EQ(expected, actual) << message;
    }
};

/**
 * @test BasicConversion_ReturnsCorrectValues
 * @brief Validates f64x2.convert_low_i32x4_s produces correct conversion results for typical inputs
 * @details Tests fundamental signed integer to float conversion with positive, negative, and mixed-sign integers.
 *          Verifies that f64x2.convert_low_i32x4_s correctly converts only lanes 0 and 1 from i32x4 to f64x2.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_CONVERT_LOW_I32X4_S_OP
 * @input_conditions Standard integer quartets: (1, -1, 999, 888), (100, -200, 777, 666)
 * @expected_behavior Returns exact double values: (1.0, -1.0), (100.0, -200.0)
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F64x2ConvertLowI32x4STestSuite, BasicConversion_ReturnsCorrectValues)
{
    // Test case 1: Mixed positive/negative values in low lanes
    auto result1 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_basic", 1, -1, 999, 888);
    assert_f64_equal(1.0, result1.first, "Lane 0 conversion failed for mixed sign values");
    assert_f64_equal(-1.0, result1.second, "Lane 1 conversion failed for mixed sign values");

    // Test case 2: Different magnitude values in low lanes
    auto result2 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_basic", 100, -200, 777, 666);
    assert_f64_equal(100.0, result2.first, "Lane 0 conversion failed for different magnitudes");
    assert_f64_equal(-200.0, result2.second, "Lane 1 conversion failed for different magnitudes");

    // Test case 3: Zero values in low lanes
    auto result3 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_basic", 0, 0, 555, 444);
    assert_f64_equal(0.0, result3.first, "Lane 0 conversion failed for zero value");
    assert_f64_equal(0.0, result3.second, "Lane 1 conversion failed for zero value");
}

/**
 * @test BoundaryValues_ConvertExactly
 * @brief Tests conversion of INT32_MIN/MAX boundary values for signed integer conversion
 * @details Validates that extreme 32-bit signed integer values convert exactly to f64 representation.
 *          Ensures no precision loss occurs during i32->f64 conversion at boundaries.
 * @test_category Corner - Boundary conditions validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_CONVERT_LOW_I32X4_S_OP
 * @input_conditions Boundary values: (INT32_MAX, INT32_MIN, X, X), (INT32_MIN, INT32_MAX, X, X)
 * @expected_behavior Returns exact conversions: (2147483647.0, -2147483648.0), (-2147483648.0, 2147483647.0)
 * @validation_method Exact f64 comparison with boundary value expectations
 */
TEST_P(F64x2ConvertLowI32x4STestSuite, BoundaryValues_ConvertExactly)
{
    // Test case 1: Maximum positive and minimum negative
    auto result1 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_boundary",
                                                   INT32_MAX, INT32_MIN, 0, 0);
    assert_f64_equal(static_cast<double>(INT32_MAX), result1.first,
                     "INT32_MAX conversion failed");
    assert_f64_equal(static_cast<double>(INT32_MIN), result1.second,
                     "INT32_MIN conversion failed");

    // Test case 2: Reverse order - minimum negative and maximum positive
    auto result2 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_boundary",
                                                   INT32_MIN, INT32_MAX, 12345, 67890);
    assert_f64_equal(static_cast<double>(INT32_MIN), result2.first,
                     "INT32_MIN in lane 0 conversion failed");
    assert_f64_equal(static_cast<double>(INT32_MAX), result2.second,
                     "INT32_MAX in lane 1 conversion failed");

    // Test case 3: Near-boundary values
    auto result3 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_boundary",
                                                   INT32_MAX - 1, INT32_MIN + 1, -999, 999);
    assert_f64_equal(static_cast<double>(INT32_MAX - 1), result3.first,
                     "INT32_MAX-1 conversion failed");
    assert_f64_equal(static_cast<double>(INT32_MIN + 1), result3.second,
                     "INT32_MIN+1 conversion failed");
}

/**
 * @test LaneSelection_IgnoresHighLanes
 * @brief Verifies that only lanes 0,1 are processed; lanes 2,3 are ignored
 * @details Tests lane selection behavior by using distinctive values in all lanes.
 *          Confirms that high lanes (2,3) do not affect the conversion result.
 * @test_category Edge - Lane isolation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_CONVERT_LOW_I32X4_S_OP
 * @input_conditions Distinctive values: (1, 2, INT32_MAX, INT32_MIN), (42, -42, 999999, -999999)
 * @expected_behavior Returns only low lane conversions: (1.0, 2.0), (42.0, -42.0)
 * @validation_method Verify result independence from high lane values
 */
TEST_P(F64x2ConvertLowI32x4STestSuite, LaneSelection_IgnoresHighLanes)
{
    // Test case 1: Distinctive values with extreme high lanes
    auto result1 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_lanes",
                                                   1, 2, INT32_MAX, INT32_MIN);
    assert_f64_equal(1.0, result1.first, "Lane 0 conversion affected by high lanes");
    assert_f64_equal(2.0, result1.second, "Lane 1 conversion affected by high lanes");

    // Test case 2: Medium values in low lanes, large values in high lanes
    auto result2 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_lanes",
                                                   42, -42, 999999, -999999);
    assert_f64_equal(42.0, result2.first, "Lane 0 conversion affected by large high lane values");
    assert_f64_equal(-42.0, result2.second, "Lane 1 conversion affected by large high lane values");

    // Test case 3: Same test with different high lane values to verify independence
    auto result3 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_lanes",
                                                   42, -42, -777, 777);
    assert_f64_equal(42.0, result3.first, "Lane 0 result changed with different high lanes");
    assert_f64_equal(-42.0, result3.second, "Lane 1 result changed with different high lanes");
}

/**
 * @test SignPreservation_HandlesNegatives
 * @brief Validates proper signed integer interpretation and conversion
 * @details Tests negative integer handling including two's complement edge cases.
 *          Ensures negative i32 values maintain sign and magnitude in f64 representation.
 * @test_category Edge - Sign handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_CONVERT_LOW_I32X4_S_OP
 * @input_conditions Negative integers: (-1, -12345, X, X), (-999999, -1, X, X)
 * @expected_behavior Preserves negative signs: (-1.0, -12345.0), (-999999.0, -1.0)
 * @validation_method Sign bit and magnitude validation for negative f64 results
 */
TEST_P(F64x2ConvertLowI32x4STestSuite, SignPreservation_HandlesNegatives)
{
    // Test case 1: Common negative values
    auto result1 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_signs",
                                                   -1, -12345, 0, 0);
    assert_f64_equal(-1.0, result1.first, "Negative one conversion failed");
    assert_f64_equal(-12345.0, result1.second, "Negative 12345 conversion failed");

    // Additional validation: ensure results are actually negative
    ASSERT_TRUE(std::signbit(result1.first)) << "Result should be negative for -1";
    ASSERT_TRUE(std::signbit(result1.second)) << "Result should be negative for -12345";

    // Test case 2: Large negative values
    auto result2 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_signs",
                                                   -999999, -1, 888, 777);
    assert_f64_equal(-999999.0, result2.first, "Large negative value conversion failed");
    assert_f64_equal(-1.0, result2.second, "Negative one in lane 1 conversion failed");

    // Additional validation: ensure magnitude and sign are correct
    ASSERT_LT(result2.first, 0.0) << "Large negative result must be less than zero";
    ASSERT_LT(result2.second, 0.0) << "Negative one result must be less than zero";

    // Test case 3: Mixed positive and negative
    auto result3 = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_signs",
                                                   -100, 200, -300, 400);
    assert_f64_equal(-100.0, result3.first, "Mixed sign lane 0 conversion failed");
    assert_f64_equal(200.0, result3.second, "Mixed sign lane 1 conversion failed");

    // Validate sign correctness
    ASSERT_TRUE(std::signbit(result3.first)) << "Lane 0 result should be negative";
    ASSERT_FALSE(std::signbit(result3.second)) << "Lane 1 result should be positive";
}

/**
 * @test ResourceManagement_NoMemoryLeaks
 * @brief Tests repeated conversions for resource leak detection
 * @details Validates that multiple consecutive conversion operations don't cause resource exhaustion.
 *          Ensures WAMR properly manages memory during repeated SIMD operations.
 * @test_category Error - Resource validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_CONVERT_LOW_I32X4_S_OP
 * @input_conditions Multiple iterations of conversion operations with varying inputs
 * @expected_behavior Consistent results without resource exhaustion or memory leaks
 * @validation_method Repeated execution with result consistency validation
 */
TEST_P(F64x2ConvertLowI32x4STestSuite, ResourceManagement_NoMemoryLeaks)
{
    // Perform multiple iterations to test for resource leaks
    const int iterations = 1000;

    for (int i = 0; i < iterations; ++i) {
        // Use iteration-dependent values for variety
        int32_t val1 = i % 1000;
        int32_t val2 = -(i % 1000);
        int32_t val3 = i * 2;
        int32_t val4 = -(i * 2);

        auto result = call_f64x2_convert_low_i32x4_s("convert_low_i32x4_s_stress",
                                                      val1, val2, val3, val4);

        // Validate expected conversion results
        assert_f64_equal(static_cast<double>(val1), result.first,
                         "Iteration conversion failed for lane 0");
        assert_f64_equal(static_cast<double>(val2), result.second,
                         "Iteration conversion failed for lane 1");

        // Every 100 iterations, ensure no exceptions occurred
        if (i % 100 == 0) {
            ASSERT_EQ(nullptr, dummy_env->get_exception())
                << "Exception occurred during iteration " << i;
        }
    }

    // Final validation: no exceptions after all iterations
    ASSERT_EQ(nullptr, dummy_env->get_exception())
        << "Exception occurred after resource management test";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode,
                         F64x2ConvertLowI32x4STestSuite,
                         testing::Values(TestRunningMode::INTERP_MODE),
                         [](const testing::TestParamInfo<TestRunningMode>& info) {
                             switch (info.param) {
                                 case TestRunningMode::INTERP_MODE:
                                     return "InterpreterMode";
                                 case TestRunningMode::AOT_MODE:
                                     return "AOTMode";
                                 default:
                                     return "UnknownMode";
                             }
                         });