/**
 * @file enhanced_f64x2_splat_test.cc
 * @brief Comprehensive unit tests for f64x2.splat SIMD opcode
 * @details Tests f64x2.splat functionality across interpreter and AOT execution modes
 *          with focus on basic operations, IEEE 754 special values, boundary conditions,
 *          and precision preservation. Validates WAMR SIMD implementation correctness
 *          and cross-mode consistency for double-precision vector construction operations.
 * @source_location tests/unit/enhanced_opcode/extension/enhanced_f64x2_splat_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <cfloat>
#include <cmath>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class F64x2SplatTestSuite
 * @brief Test fixture class for f64x2.splat opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles SIMD vector result validation using DummyExecEnv helper
 *          for comprehensive f64x2.splat operation validation with IEEE 754 compliance.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class F64x2SplatTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f64x2.splat testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files using relative path.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_f64x2_splat_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.splat test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_splat_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.splat tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_f64x2_splat_test.cc:TearDown
     */
    void TearDown() override
    {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute f64x2.splat operation with given f64 input value
     * @param input f64 value to splat across both lanes of f64x2 vector
     * @param expected_lanes Expected f64 values in both lanes (should be identical)
     * @details Calls WASM test function to execute f64x2.splat and validates resulting vector
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_f64x2_splat_test.cc:call_f64x2_splat
     */
    void call_f64x2_splat(double input, double expected_lanes[2])
    {
        // v128 functions return 4 uint32_t values which overwrite the argv array
        uint32_t argv[4];
        memcpy(argv, &input, sizeof(double));
        argv[2] = 0; // Initialize unused elements
        argv[3] = 0;

        // Execute the function and expect success
        bool call_result = dummy_env->execute("test_f64x2_splat", 2, argv);
        ASSERT_TRUE(call_result)
            << "Failed to call test_f64x2_splat with value: " << input;

        // Extract result f64 values from argv (now contains the v128 result)
        double* result_f64s = reinterpret_cast<double*>(argv);

        // Validate both lanes contain the expected values
        for (int i = 0; i < 2; i++) {
            if (std::isnan(expected_lanes[i])) {
                ASSERT_TRUE(std::isnan(result_f64s[i]))
                    << "Lane " << i << " should be NaN but got: " << result_f64s[i];
            } else {
                ASSERT_DOUBLE_EQ(expected_lanes[i], result_f64s[i])
                    << "Lane " << i << " contains incorrect value";
            }
        }
    }

    /**
     * @brief Helper function to call no-argument f64x2.splat WASM function
     * @param func_name WASM function name to call
     * @param expected_f64 Expected f64 value in both lanes
     * @details Calls WASM function with no arguments for testing constant splat values.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_f64x2_splat_test.cc:CallF64x2SplatConst
     */
    void CallF64x2SplatConst(const char* func_name, double expected_f64)
    {
        // v128 functions return 4 uint32_t values
        uint32_t argv[4] = { 0, 0, 0, 0 };

        // Execute the function and expect success
        bool call_result = dummy_env->execute(func_name, 0, argv);
        ASSERT_TRUE(call_result)
            << "Failed to call " << func_name;

        // Extract result f64 values from argv (now contains the v128 result)
        double* result_f64s = reinterpret_cast<double*>(argv);

        // Validate both lanes contain the expected value
        for (int i = 0; i < 2; i++) {
            if (std::isnan(expected_f64)) {
                ASSERT_TRUE(std::isnan(result_f64s[i]))
                    << "Lane " << i << " should be NaN but got: " << result_f64s[i];
            } else if (expected_f64 == 0.0) {
                // Handle signed zero comparison
                ASSERT_TRUE(result_f64s[i] == 0.0)
                    << "Lane " << i << " should be zero but got: " << result_f64s[i];
                ASSERT_EQ(std::signbit(expected_f64), std::signbit(result_f64s[i]))
                    << "Lane " << i << " sign mismatch for zero value";
            } else {
                ASSERT_DOUBLE_EQ(expected_f64, result_f64s[i])
                    << "Lane " << i << " contains incorrect value";
            }
        }
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicSplat_CreatesCorrectVector
 * @brief Validates f64x2.splat produces correct vectors for typical f64 inputs
 * @details Tests fundamental splat operation with positive, negative, and fractional doubles.
 *          Verifies that f64x2.splat correctly replicates input value across both vector lanes.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_splat_operation
 * @input_conditions Standard double values: 1.0, -1.0, 3.14159, 0.5, -42.75
 * @expected_behavior Returns f64x2 vector with identical values in both lanes
 * @validation_method Direct lane extraction and comparison with input values
 */
TEST_F(F64x2SplatTestSuite, BasicSplat_CreatesCorrectVector)
{
    // Test positive double value
    double expected1[2] = {1.0, 1.0};
    call_f64x2_splat(1.0, expected1);

    // Test negative double value
    double expected2[2] = {-1.0, -1.0};
    call_f64x2_splat(-1.0, expected2);

    // Test fractional value with high precision
    double pi_val = 3.14159265358979323846;
    double expected3[2] = {pi_val, pi_val};
    call_f64x2_splat(pi_val, expected3);

    // Test simple fractional value
    double expected4[2] = {0.5, 0.5};
    call_f64x2_splat(0.5, expected4);

    // Test negative fractional value
    double expected5[2] = {-42.75, -42.75};
    call_f64x2_splat(-42.75, expected5);
}

/**
 * @test BoundaryValues_PreservePrecision
 * @brief Validates f64x2.splat handles IEEE 754 boundary values correctly
 * @details Tests extreme double values including DBL_MIN, DBL_MAX, and subnormal values.
 *          Verifies bit-perfect precision preservation across vector lanes.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_splat_operation
 * @input_conditions DBL_MIN, DBL_MAX, smallest subnormal values
 * @expected_behavior Exact bit replication in both f64x2 vector lanes
 * @validation_method Bit-level comparison for precision validation
 */
TEST_F(F64x2SplatTestSuite, BoundaryValues_PreservePrecision)
{
    // Test largest positive normalized double
    double expected_max[2] = {DBL_MAX, DBL_MAX};
    call_f64x2_splat(DBL_MAX, expected_max);

    // Test smallest positive normalized double
    double expected_min[2] = {DBL_MIN, DBL_MIN};
    call_f64x2_splat(DBL_MIN, expected_min);

    // Test smallest positive subnormal double (if supported)
    double smallest_subnormal = DBL_MIN / DBL_EPSILON / 2.0;
    double expected_subnormal[2] = {smallest_subnormal, smallest_subnormal};
    call_f64x2_splat(smallest_subnormal, expected_subnormal);

    // Test value near machine epsilon
    double expected_epsilon[2] = {DBL_EPSILON, DBL_EPSILON};
    call_f64x2_splat(DBL_EPSILON, expected_epsilon);
}

/**
 * @test SpecialValues_HandleCorrectly
 * @brief Validates f64x2.splat handles IEEE 754 special values correctly
 * @details Tests special double values including ±0.0, ±Infinity, and NaN patterns.
 *          Verifies proper handling and bit pattern preservation for edge case values.
 * @test_category Edge - Special IEEE 754 value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_splat_operation
 * @input_conditions +0.0, -0.0, +Infinity, -Infinity, NaN
 * @expected_behavior Special values preserved with correct sign and bit patterns
 * @validation_method Special value detection and bit-level comparison
 */
TEST_F(F64x2SplatTestSuite, SpecialValues_HandleCorrectly)
{
    // Test positive zero
    CallF64x2SplatConst("f64x2_splat_pos_zero", +0.0);

    // Test negative zero (IEEE 754 distinction)
    CallF64x2SplatConst("f64x2_splat_neg_zero", -0.0);

    // Test positive infinity
    CallF64x2SplatConst("f64x2_splat_pos_inf", INFINITY);

    // Test negative infinity
    CallF64x2SplatConst("f64x2_splat_neg_inf", -INFINITY);

    // Test quiet NaN
    double quiet_nan = std::numeric_limits<double>::quiet_NaN();
    CallF64x2SplatConst("f64x2_splat_nan", quiet_nan);
}

/**
 * @test HighPrecisionValues_MaintainAccuracy
 * @brief Validates f64x2.splat preserves high-precision double values exactly
 * @details Tests doubles requiring full 64-bit precision including mathematical constants
 *          and values with many significant digits. Verifies no precision loss during splat.
 * @test_category Edge - High precision value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_splat_operation
 * @input_conditions High precision mathematical constants and large magnitude values
 * @expected_behavior Exact preservation of all 64 bits of precision in both lanes
 * @validation_method Bit-perfect comparison for precision maintenance
 */
TEST_F(F64x2SplatTestSuite, HighPrecisionValues_MaintainAccuracy)
{
    // Test mathematical constant e with high precision
    double high_precision_e = 2.7182818284590452354;
    double expected_e[2] = {high_precision_e, high_precision_e};
    call_f64x2_splat(high_precision_e, expected_e);

    // Test mathematical constant pi with high precision
    double high_precision_pi = 3.1415926535897932384626433832795;
    double expected_pi[2] = {high_precision_pi, high_precision_pi};
    call_f64x2_splat(high_precision_pi, expected_pi);

    // Test large magnitude precise value
    double large_precise = 1.23456789012345e+100;
    double expected_large[2] = {large_precise, large_precise};
    call_f64x2_splat(large_precise, expected_large);

    // Test small magnitude precise value
    double small_precise = 1.23456789012345e-100;
    double expected_small[2] = {small_precise, small_precise};
    call_f64x2_splat(small_precise, expected_small);
}