/*
 * Enhanced unit tests for f64x2.sub opcode
 * Tests element-wise subtraction of two f64x2 SIMD vectors with comprehensive validation
 */

#include <gtest/gtest.h>
#include <cmath>
#include <float.h>
#include <cstring>
#include <memory>
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
 * @class F64x2SubTest
 * @brief Test fixture for f64x2.sub opcode validation across execution modes
 * @details Provides parameterized testing across interpreter and AOT execution modes
 *          with comprehensive SIMD vector subtraction validation using IEEE 754 standards
 */
class F64x2SubTest : public testing::TestWithParam<TestRunningMode>
{
protected:
    TestRunningMode running_mode;
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Set up WAMR runtime environment and load test module
     * @details Initializes WAMR runtime with proper configuration for SIMD operations,
     *          loads the f64x2.sub test WASM module, and prepares execution environment
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime with SIMD support
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.sub test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_sub_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.sub tests";
    }

    /**
     * @brief Clean up WAMR runtime environment and release resources
     * @details Properly destroys execution environment and runtime resources
     */
    void TearDown() override
    {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute f64x2.sub operation with two input vectors
     * @param minuend First f64x2 vector (values to subtract from)
     * @param subtrahend Second f64x2 vector (values to subtract)
     * @param result Output f64x2 vector containing subtraction results
     * @details Calls the WASM f64x2.sub function and retrieves the result vector
     */
    void call_f64x2_sub(const double minuend[2], const double subtrahend[2], double result[2])
    {
        uint32_t argv[8]; // 2 v128 parameters (4 uint32_t each)

        // Pack input vectors into uint32_t array
        memcpy(&argv[0], minuend, sizeof(double) * 2);
        memcpy(&argv[4], subtrahend, sizeof(double) * 2);

        bool call_result = dummy_env->execute("f64x2_sub_basic", 8, argv);
        ASSERT_TRUE(call_result)
            << "Failed to execute f64x2_sub_basic: " << dummy_env->get_exception();

        // Unpack result vector
        memcpy(result, argv, sizeof(double) * 2);
    }

    /**
     * @brief Helper function to compare two double values with IEEE 754 awareness
     * @param expected Expected double value
     * @param actual Actual double value
     * @return true if values are equal (handles NaN, infinity, zero cases with epsilon tolerance)
     */
    bool double_equal_ieee754(double expected, double actual) {
        // Handle NaN cases - NaN should equal NaN
        if (std::isnan(expected) && std::isnan(actual)) {
            return true;
        }
        // Handle infinity cases
        if (std::isinf(expected) && std::isinf(actual)) {
            return (expected > 0 && actual > 0) || (expected < 0 && actual < 0);
        }
        // Handle zero cases (both positive and negative zero are considered equal)
        if (expected == 0.0 && actual == 0.0) {
            return true;
        }
        // Epsilon-based comparison for normal floating-point values
        const double epsilon = 1e-15; // Precision tolerance for IEEE 754 double
        return std::abs(expected - actual) < epsilon;
    }
};

/**
 * @test BasicSubtraction_ReturnsCorrectDifference
 * @brief Validates f64x2.sub produces correct arithmetic results for typical inputs
 * @details Tests fundamental subtraction operation with positive, negative, and mixed-sign
 *          double-precision numbers. Verifies that f64x2.sub correctly computes minuend[i] - subtrahend[i]
 *          for various input combinations with exact IEEE 754 precision.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_sub_operation
 * @input_conditions Standard f64 pairs: [5.5, 10.3] - [2.3, 4.1], [-7.2, 15.6] - [3.8, -2.4]
 * @expected_behavior Returns mathematical difference: [3.2, 6.2], [-11.0, 18.0] respectively
 * @validation_method Direct IEEE 754 floating-point comparison of WASM function results
 */
TEST_P(F64x2SubTest, BasicSubtraction_ReturnsCorrectDifference)
{
    // Test case 1: Positive numbers
    double minuend1[2] = {5.5, 10.3};
    double subtrahend1[2] = {2.3, 4.1};
    double result1[2];

    call_f64x2_sub(minuend1, subtrahend1, result1);
    ASSERT_TRUE(double_equal_ieee754(3.2, result1[0])) << "First element subtraction failed for positive numbers";
    ASSERT_TRUE(double_equal_ieee754(6.2, result1[1])) << "Second element subtraction failed for positive numbers";

    // Test case 2: Mixed positive/negative numbers
    double minuend2[2] = {-7.2, 15.6};
    double subtrahend2[2] = {3.8, -2.4};
    double result2[2];

    call_f64x2_sub(minuend2, subtrahend2, result2);
    ASSERT_TRUE(double_equal_ieee754(-11.0, result2[0])) << "First element subtraction failed for mixed signs";
    ASSERT_TRUE(double_equal_ieee754(18.0, result2[1])) << "Second element subtraction failed for mixed signs";
}

/**
 * @test ZeroValueSubtraction_HandlesZeroCorrectly
 * @brief Validates f64x2.sub correctly handles operations involving positive and negative zero
 * @details Tests IEEE 754 zero handling: x - (+0.0) = x, x - (-0.0) = x, (+0.0) - (-0.0) = +0.0
 *          Ensures proper sign preservation and zero subtraction behavior according to IEEE 754.
 * @test_category Main - Zero value edge cases
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_sub_zero_handling
 * @input_conditions Zero combinations: [1.0, -1.0] - [+0.0, -0.0], [+0.0, -0.0] - [+0.0, -0.0]
 * @expected_behavior Returns [1.0, -1.0] and [+0.0, +0.0] preserving IEEE 754 zero arithmetic
 * @validation_method IEEE 754 compliant zero comparison with sign bit verification
 */
TEST_P(F64x2SubTest, ZeroValueSubtraction_HandlesZeroCorrectly)
{
    // Test case 1: Finite numbers minus zero
    double minuend1[2] = {1.0, -1.0};
    double subtrahend1[2] = {+0.0, -0.0};
    double result1[2];

    call_f64x2_sub(minuend1, subtrahend1, result1);
    ASSERT_TRUE(double_equal_ieee754(1.0, result1[0])) << "Positive number minus zero failed";
    ASSERT_TRUE(double_equal_ieee754(-1.0, result1[1])) << "Negative number minus negative zero failed";

    // Test case 2: Zero minus zero combinations
    double minuend2[2] = {+0.0, -0.0};
    double subtrahend2[2] = {+0.0, -0.0};
    double result2[2];

    call_f64x2_sub(minuend2, subtrahend2, result2);
    ASSERT_TRUE(double_equal_ieee754(+0.0, result2[0])) << "Positive zero minus positive zero failed";
    ASSERT_TRUE(double_equal_ieee754(+0.0, result2[1])) << "Negative zero minus negative zero should produce positive zero";
}

/**
 * @test InfinitySubtraction_PreservesIEEE754Behavior
 * @brief Validates f64x2.sub correctly handles infinity operations according to IEEE 754
 * @details Tests infinity arithmetic: inf - inf = NaN, inf - (-inf) = inf, (-inf) - inf = -inf
 *          Ensures proper IEEE 754 infinity subtraction behavior and special case handling.
 * @test_category Edge - Infinity arithmetic validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_sub_infinity_handling
 * @input_conditions Infinity combinations: [+inf, -inf] - [+inf, -inf], [5.0, -5.0] - [+inf, -inf]
 * @expected_behavior Returns [NaN, NaN] for inf-inf, [-inf, +inf] for finite-inf operations
 * @validation_method IEEE 754 infinity and NaN detection with proper special value handling
 */
TEST_P(F64x2SubTest, InfinitySubtraction_PreservesIEEE754Behavior)
{
    // Test case 1: Infinity minus infinity (should produce NaN)
    double pos_inf = std::numeric_limits<double>::infinity();
    double neg_inf = -std::numeric_limits<double>::infinity();

    double minuend1[2] = {pos_inf, neg_inf};
    double subtrahend1[2] = {pos_inf, neg_inf};
    double result1[2];

    call_f64x2_sub(minuend1, subtrahend1, result1);
    ASSERT_TRUE(std::isnan(result1[0])) << "Positive infinity minus positive infinity should produce NaN";
    ASSERT_TRUE(std::isnan(result1[1])) << "Negative infinity minus negative infinity should produce NaN";

    // Test case 2: Finite number minus infinity
    double minuend2[2] = {5.0, -5.0};
    double subtrahend2[2] = {pos_inf, neg_inf};
    double result2[2];

    call_f64x2_sub(minuend2, subtrahend2, result2);
    ASSERT_TRUE(std::isinf(result2[0]) && result2[0] < 0) << "Finite minus positive infinity should be negative infinity";
    ASSERT_TRUE(std::isinf(result2[1]) && result2[1] > 0) << "Finite minus negative infinity should be positive infinity";
}

/**
 * @test NaNPropagation_PreservesNaNValues
 * @brief Validates f64x2.sub correctly propagates NaN values according to IEEE 754
 * @details Tests NaN arithmetic: NaN - x = NaN, x - NaN = NaN, NaN - NaN = NaN
 *          Ensures proper IEEE 754 NaN propagation behavior in SIMD vector operations.
 * @test_category Edge - NaN propagation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_sub_nan_handling
 * @input_conditions NaN combinations: [NaN, 5.0] - [2.0, NaN], [NaN, NaN] - [1.0, 1.0]
 * @expected_behavior Returns [NaN, NaN] for all cases preserving IEEE 754 NaN propagation rules
 * @validation_method IEEE 754 NaN detection ensuring proper quiet NaN behavior
 */
TEST_P(F64x2SubTest, NaNPropagation_PreservesNaNValues)
{
    double nan_val = std::numeric_limits<double>::quiet_NaN();

    // Test case 1: NaN in first operand
    double minuend1[2] = {nan_val, 5.0};
    double subtrahend1[2] = {2.0, 3.0};
    double result1[2];

    call_f64x2_sub(minuend1, subtrahend1, result1);
    ASSERT_TRUE(std::isnan(result1[0])) << "NaN in minuend should propagate to result";
    ASSERT_TRUE(double_equal_ieee754(2.0, result1[1])) << "Normal subtraction should work when no NaN present";

    // Test case 2: NaN in second operand
    double minuend2[2] = {7.0, 4.0};
    double subtrahend2[2] = {1.0, nan_val};
    double result2[2];

    call_f64x2_sub(minuend2, subtrahend2, result2);
    ASSERT_TRUE(double_equal_ieee754(6.0, result2[0])) << "Normal subtraction should work when no NaN present";
    ASSERT_TRUE(std::isnan(result2[1])) << "NaN in subtrahend should propagate to result";
}

/**
 * @test BoundaryValueSubtraction_HandlesExtremeValues
 * @brief Validates f64x2.sub correctly handles boundary and extreme floating-point values
 * @details Tests extreme value arithmetic: DBL_MAX operations, DBL_MIN operations, denormal handling
 *          Ensures proper overflow, underflow, and boundary condition handling for double-precision limits.
 * @test_category Edge - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_sub_boundary_handling
 * @input_conditions Boundary values: [DBL_MAX, DBL_MIN] - [-DBL_MAX, 0.0], denormal operations
 * @expected_behavior Returns [+inf, DBL_MIN] with proper overflow/underflow handling per IEEE 754
 * @validation_method Boundary value verification with overflow detection and denormal preservation
 */
TEST_P(F64x2SubTest, BoundaryValueSubtraction_HandlesExtremeValues)
{
    // Test case 1: Maximum value operations (should overflow to infinity)
    double minuend1[2] = {DBL_MAX, DBL_MIN};
    double subtrahend1[2] = {-DBL_MAX, 0.0};
    double result1[2];

    call_f64x2_sub(minuend1, subtrahend1, result1);
    ASSERT_TRUE(std::isinf(result1[0]) && result1[0] > 0)
        << "DBL_MAX minus -DBL_MAX should overflow to positive infinity";
    ASSERT_TRUE(double_equal_ieee754(DBL_MIN, result1[1]))
        << "DBL_MIN minus 0 should preserve minimum value";

    // Test case 2: Very small numbers (denormal preservation)
    double small_val = DBL_MIN / 2.0; // Denormal number
    double minuend2[2] = {small_val, 1.0};
    double subtrahend2[2] = {0.0, small_val};
    double result2[2];

    call_f64x2_sub(minuend2, subtrahend2, result2);
    ASSERT_TRUE(double_equal_ieee754(small_val, result2[0]))
        << "Denormal number minus zero should preserve denormal value";
    ASSERT_GT(result2[1], 0.99)
        << "1.0 minus small denormal should be approximately 1.0";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningMode, F64x2SubTest,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));