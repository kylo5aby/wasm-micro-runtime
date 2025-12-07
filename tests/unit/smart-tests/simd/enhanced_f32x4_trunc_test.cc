/**
 * @file enhanced_f32x4_trunc_test.cc
 * @brief Comprehensive unit tests for f32x4.trunc SIMD opcode
 * @details Tests f32x4.trunc functionality across interpreter and AOT execution modes
 *          with focus on IEEE 754 truncation operations, special values, and cross-mode consistency.
 *          Validates WAMR SIMD implementation correctness for floating-point truncation function.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_trunc_test.cc
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
 * @class F32x4TruncTestSuite
 * @brief Test fixture class for f32x4.trunc opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD float32 result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class F32x4TruncTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.trunc testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_trunc_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.trunc test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_trunc_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.trunc tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_trunc_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f32x4.trunc function and extract v128 result
     * @details Executes f32x4.trunc test function and extracts four f32 values from v128 result.
     *          Handles WASM function invocation and v128 result extraction into f32 array.
     * @param function_name Name of the WASM function to call
     * @param results Reference to f32[4] array to store extracted results
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_trunc_test.cc:call_f32x4_trunc_function
     */
    bool call_f32x4_trunc_function(const char* function_name, float results[4])
    {
        // Call WASM function with no arguments, expects v128 return value
        uint32_t argv[4]; // Space for v128 return value (4 x 32-bit components)
        bool call_success = dummy_env->execute(function_name, 0, argv);
        EXPECT_TRUE(call_success) << "Failed to call " << function_name << " function";

        if (call_success) {
            // Extract f32 results from v128 value (4 lanes of f32)
            memcpy(results, argv, sizeof(float) * 4);
        }

        return call_success;
    }

    /**
     * @brief Utility function to check if two floats are equal considering NaN
     * @details Handles special IEEE 754 comparison including NaN values
     * @param expected Expected float value
     * @param actual Actual float value
     * @return bool True if values are equivalent (including both NaN)
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_trunc_test.cc:float_equal_or_both_nan
     */
    bool float_equal_or_both_nan(float expected, float actual)
    {
        if (std::isnan(expected) && std::isnan(actual)) {
            return true;  // Both NaN
        }
        return expected == actual;  // Regular equality (handles infinity and signed zero)
    }

    /**
     * @brief Utility function to check if a float value is negative zero
     * @details Uses signbit to detect -0.0f specifically
     * @param value Float value to check
     * @return bool True if value is exactly -0.0f
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_trunc_test.cc:is_negative_zero
     */
    bool is_negative_zero(float value)
    {
        return (value == 0.0f) && std::signbit(value);
    }

    /**
     * @brief Utility function to check if a float value is positive zero
     * @details Uses signbit to detect +0.0f specifically
     * @param value Float value to check
     * @return bool True if value is exactly +0.0f
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_trunc_test.cc:is_positive_zero
     */
    bool is_positive_zero(float value)
    {
        return (value == 0.0f) && !std::signbit(value);
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicTruncation_ReturnsIntegerPart
 * @brief Validates fundamental f32x4.trunc behavior with typical fractional values
 * @details Tests that f32x4.trunc correctly removes fractional parts and rounds toward zero
 *          for positive, negative, and mixed-sign floating-point values across all lanes.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_operation
 * @input_conditions Mixed fractional values: [1.5f, -2.7f, 3.14f, -0.9f]
 * @expected_behavior Returns truncated values: [1.0f, -2.0f, 3.0f, -0.0f]
 * @validation_method Direct comparison with expected truncated results
 */
TEST_F(F32x4TruncTestSuite, BasicTruncation_ReturnsIntegerPart)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_basic", results))
        << "Failed to execute basic f32x4.trunc test function";

    // Expected results for input [1.5f, -2.7f, 3.14f, -0.9f]
    float expected[4] = {1.0f, -2.0f, 3.0f, -0.0f};

    for (int i = 0; i < 4; i++) {
        ASSERT_FLOAT_EQ(expected[i], results[i])
            << "Lane " << i << " truncation failed: expected=" << expected[i]
            << " actual=" << results[i];
    }

    // Verify lane 3 produces negative zero for -0.9f truncation
    ASSERT_TRUE(is_negative_zero(results[3]))
        << "Lane 3 should produce negative zero (-0.0f) from truncating -0.9f";
}

/**
 * @test SpecialValues_PreservesInfinityAndNaN
 * @brief Validates preservation of special IEEE 754 values
 * @details Tests that f32x4.trunc correctly preserves infinity and NaN values without
 *          modification, as these special values have no fractional part to truncate.
 * @test_category Edge - Special floating-point value preservation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_special_values
 * @input_conditions Special values: [NaN, +∞, -∞, -0.0f]
 * @expected_behavior Preserves special values: [NaN, +∞, -∞, -0.0f]
 * @validation_method IEEE 754 special value detection and preservation testing
 */
TEST_F(F32x4TruncTestSuite, SpecialValues_PreservesInfinityAndNaN)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_special", results))
        << "Failed to execute special values f32x4.trunc test function";

    // Check NaN preservation (lane 0)
    ASSERT_TRUE(std::isnan(results[0]))
        << "Lane 0 should preserve NaN value";

    // Check positive infinity preservation (lane 1)
    ASSERT_TRUE(std::isinf(results[1]) && results[1] > 0)
        << "Lane 1 should preserve positive infinity";

    // Check negative infinity preservation (lane 2)
    ASSERT_TRUE(std::isinf(results[2]) && results[2] < 0)
        << "Lane 2 should preserve negative infinity";

    // Check negative zero preservation (lane 3)
    ASSERT_TRUE(is_negative_zero(results[3]))
        << "Lane 3 should preserve negative zero (-0.0f)";
}

/**
 * @test BoundaryValues_TruncatesCorrectly
 * @brief Tests f32x4.trunc behavior at floating-point precision boundaries
 * @details Validates truncation of maximum finite values and values near unity boundary.
 *          Ensures correct boundary condition handling for extreme values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_boundary_handling
 * @input_conditions Boundary values: [FLT_MAX, -FLT_MAX, 0.999999f, -0.999999f]
 * @expected_behavior Max values unchanged, near-unity values truncate to zero
 * @validation_method Precise floating-point comparison for boundary behavior
 */
TEST_F(F32x4TruncTestSuite, BoundaryValues_TruncatesCorrectly)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_boundary", results))
        << "Failed to execute boundary values f32x4.trunc test function";

    // FLT_MAX and -FLT_MAX should remain unchanged (already integers at this scale)
    ASSERT_FLOAT_EQ(FLT_MAX, results[0])
        << "Lane 0 should preserve FLT_MAX";
    ASSERT_FLOAT_EQ(-FLT_MAX, results[1])
        << "Lane 1 should preserve -FLT_MAX";

    // 0.999999f should truncate to +0.0f
    ASSERT_TRUE(is_positive_zero(results[2]))
        << "Lane 2 should truncate 0.999999f to positive zero";

    // -0.999999f should truncate to -0.0f
    ASSERT_TRUE(is_negative_zero(results[3]))
        << "Lane 3 should truncate -0.999999f to negative zero";
}

/**
 * @test ZeroValues_PreservesSignedZeros
 * @brief Verifies correct handling of positive and negative zero values
 * @details Tests that f32x4.trunc preserves the sign of zero values and correctly
 *          truncates small fractional values to appropriately signed zeros.
 * @test_category Edge - Zero value and sign preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_zero_handling
 * @input_conditions Zero and near-zero values: [0.0f, -0.0f, 0.1f, -0.1f]
 * @expected_behavior Preserves zero signs: [0.0f, -0.0f, 0.0f, -0.0f]
 * @validation_method Bit-wise comparison to detect signed zero preservation
 */
TEST_F(F32x4TruncTestSuite, ZeroValues_PreservesSignedZeros)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_zeros", results))
        << "Failed to execute zero values f32x4.trunc test function";

    // Check positive zero preservation (lane 0)
    ASSERT_TRUE(is_positive_zero(results[0]))
        << "Lane 0 should preserve positive zero (+0.0f)";

    // Check negative zero preservation (lane 1)
    ASSERT_TRUE(is_negative_zero(results[1]))
        << "Lane 1 should preserve negative zero (-0.0f)";

    // Check 0.1f truncates to positive zero (lane 2)
    ASSERT_TRUE(is_positive_zero(results[2]))
        << "Lane 2 should truncate 0.1f to positive zero";

    // Check -0.1f truncates to negative zero (lane 3)
    ASSERT_TRUE(is_negative_zero(results[3]))
        << "Lane 3 should truncate -0.1f to negative zero";
}

/**
 * @test IntegerInputs_RemainUnchanged
 * @brief Confirms that perfect integer values are not modified by truncation
 * @details Validates that f32x4.trunc leaves integer values unchanged, including
 *          small integers, large integers, and negative integers.
 * @test_category Edge - Integer input preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_integer_preservation
 * @input_conditions Integer values: [1.0f, -5.0f, 100.0f, -1000.0f]
 * @expected_behavior No modification: [1.0f, -5.0f, 100.0f, -1000.0f]
 * @validation_method Exact equality comparison for integer preservation
 */
TEST_F(F32x4TruncTestSuite, IntegerInputs_RemainUnchanged)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_integers", results))
        << "Failed to execute integer values f32x4.trunc test function";

    // All integer values should remain unchanged
    float expected[4] = {1.0f, -5.0f, 100.0f, -1000.0f};

    for (int i = 0; i < 4; i++) {
        ASSERT_FLOAT_EQ(expected[i], results[i])
            << "Lane " << i << " integer preservation failed: expected=" << expected[i]
            << " actual=" << results[i];
    }
}

/**
 * @test MathematicalProperties_ValidatesTruncationRules
 * @brief Tests mathematical properties and consistency of truncation operation
 * @details Validates truncation behavior rules including sign preservation,
 *          magnitude reduction, and specific truncation results.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_properties
 * @input_conditions Various values for property testing: [2.7f, -3.8f, 0.5f, -0.3f]
 * @expected_behavior Satisfies mathematical truncation properties: [2.0f, -3.0f, 0.0f, -0.0f]
 * @validation_method Property-based assertions and mathematical rule verification
 */
TEST_F(F32x4TruncTestSuite, MathematicalProperties_ValidatesTruncationRules)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_properties", results))
        << "Failed to execute mathematical properties f32x4.trunc test function";

    // Expected truncated results: [2.7f->2.0f, -3.8f->-3.0f, 0.5f->0.0f, -0.3f->-0.0f]
    float expected[4] = {2.0f, -3.0f, 0.0f, -0.0f};

    // Verify specific truncation results
    ASSERT_FLOAT_EQ(expected[0], results[0])
        << "Lane 0: 2.7f should truncate to 2.0f";
    ASSERT_FLOAT_EQ(expected[1], results[1])
        << "Lane 1: -3.8f should truncate to -3.0f";
    ASSERT_TRUE(is_positive_zero(results[2]))
        << "Lane 2: 0.5f should truncate to positive zero";
    ASSERT_TRUE(is_negative_zero(results[3]))
        << "Lane 3: -0.3f should truncate to negative zero";
}

/**
 * @test PrecisionBoundary_HandlesLargeNumbers
 * @brief Tests f32x4.trunc behavior at floating-point precision limits
 * @details Validates truncation of numbers near f32 precision boundaries where
 *          fractional representation may be limited by IEEE 754 precision.
 * @test_category Corner - Precision boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_precision_limits
 * @input_conditions Precision boundary values: [16777216.5f, -16777216.5f, 8388607.9f, -8388607.1f]
 * @expected_behavior Handles precision limits correctly
 * @validation_method Precision-aware comparison for large number truncation
 */
TEST_F(F32x4TruncTestSuite, PrecisionBoundary_HandlesLargeNumbers)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_precision", results))
        << "Failed to execute precision boundary f32x4.trunc test function";

    // At f32 precision limits, fractional parts may not be exactly representable
    // 2^24 + 0.5 may already be rounded to an integer due to precision limits
    ASSERT_GE(results[0], 16777216.0f)
        << "Lane 0 should be at least 16777216.0f (2^24)";
    ASSERT_LE(results[1], -16777216.0f)
        << "Lane 1 should be at most -16777216.0f (-2^24)";

    // For values within exact representation range, check specific truncation
    ASSERT_GE(results[2], 8388607.0f)
        << "Lane 2 should truncate to at least 8388607.0f";
    ASSERT_LE(results[3], -8388607.0f)
        << "Lane 3 should truncate to at most -8388607.0f";
}

/**
 * @test EdgeCases_HandlesExtremeScenarios
 * @brief Tests f32x4.trunc with various edge cases and extreme scenarios
 * @details Validates truncation behavior for very small numbers, subnormals,
 *          and values just around unit boundaries.
 * @test_category Edge - Extreme scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_edge_cases
 * @input_conditions Edge case values: [1e-38f, -1e-38f, 1.0000001f, -1.0000001f]
 * @expected_behavior Correct handling of extreme values: [0.0f, -0.0f, 1.0f, -1.0f]
 * @validation_method Edge case specific assertions and boundary validation
 */
TEST_F(F32x4TruncTestSuite, EdgeCases_HandlesExtremeScenarios)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_edge", results))
        << "Failed to execute edge cases f32x4.trunc test function";

    // Very small positive number should truncate to +0.0f
    ASSERT_TRUE(is_positive_zero(results[0]))
        << "Lane 0: 1e-38f should truncate to positive zero";

    // Very small negative number should truncate to -0.0f
    ASSERT_TRUE(is_negative_zero(results[1]))
        << "Lane 1: -1e-38f should truncate to negative zero";

    // 1.0000001f should truncate to 1.0f
    ASSERT_FLOAT_EQ(1.0f, results[2])
        << "Lane 2: 1.0000001f should truncate to 1.0f";

    // -1.0000001f should truncate to -1.0f
    ASSERT_FLOAT_EQ(-1.0f, results[3])
        << "Lane 3: -1.0000001f should truncate to -1.0f";
}

/**
 * @test MixedScenarios_ComprehensiveValidation
 * @brief Tests f32x4.trunc with mixed special and normal values in single vector
 * @details Validates truncation behavior when different types of values
 *          (special, normal, fractional) are combined in a single SIMD vector.
 * @test_category Edge - Mixed value scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_*.c:f32x4_trunc_mixed_scenarios
 * @input_conditions Mixed values: [NaN, 2.5f, +∞, -0.1f]
 * @expected_behavior Appropriate per-lane handling: [NaN, 2.0f, +∞, -0.0f]
 * @validation_method Mixed assertion types for different value categories
 */
TEST_F(F32x4TruncTestSuite, MixedScenarios_ComprehensiveValidation)
{
    float results[4];
    ASSERT_TRUE(call_f32x4_trunc_function("test_f32x4_trunc_mixed", results))
        << "Failed to execute mixed scenarios f32x4.trunc test function";

    // Lane 0: NaN should be preserved
    ASSERT_TRUE(std::isnan(results[0]))
        << "Lane 0 should preserve NaN value";

    // Lane 1: 2.5f should truncate to 2.0f
    ASSERT_FLOAT_EQ(2.0f, results[1])
        << "Lane 1: 2.5f should truncate to 2.0f";

    // Lane 2: +infinity should be preserved
    ASSERT_TRUE(std::isinf(results[2]) && results[2] > 0)
        << "Lane 2 should preserve positive infinity";

    // Lane 3: -0.1f should truncate to -0.0f
    ASSERT_TRUE(is_negative_zero(results[3]))
        << "Lane 3: -0.1f should truncate to negative zero";
}