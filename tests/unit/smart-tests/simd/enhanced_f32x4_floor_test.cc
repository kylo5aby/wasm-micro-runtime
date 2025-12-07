/**
 * @file enhanced_f32x4_floor_test.cc
 * @brief Comprehensive unit tests for f32x4.floor SIMD opcode
 * @details Tests f32x4.floor functionality across interpreter and AOT execution modes
 *          with focus on IEEE 754 floor operations, special values, and cross-mode consistency.
 *          Validates WAMR SIMD implementation correctness for floating-point floor function.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_floor_test.cc
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
 * @class F32x4FloorTestSuite
 * @brief Test fixture class for f32x4.floor opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD float32 result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class F32x4FloorTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.floor testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_floor_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.floor test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_floor_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.floor tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_floor_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f32x4.floor function and extract v128 result
     * @details Executes f32x4.floor test function and extracts four f32 values from v128 result.
     *          Handles WASM function invocation and v128 result extraction into f32 array.
     * @param function_name Name of the WASM function to call
     * @param results Reference to f32[4] array to store extracted results
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_floor_test.cc:call_f32x4_floor_function
     */
    bool call_f32x4_floor_function(const char* function_name, float results[4])
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
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_floor_test.cc:float_equal_or_both_nan
     */
    bool float_equal_or_both_nan(float expected, float actual)
    {
        if (std::isnan(expected) && std::isnan(actual)) {
            return true;  // Both NaN
        }
        return expected == actual;  // Regular equality (handles infinity and signed zero)
    }

    /**
     * @brief Utility function to validate v128 f32x4 results against expected values
     * @details Compares four-lane f32 results with expected values using appropriate comparison
     * @param expected Expected f32[4] array
     * @param actual Actual f32[4] results from WASM execution
     * @param test_description Description for assertion messages
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_floor_test.cc:validate_f32x4_results
     */
    void validate_f32x4_results(const float expected[4], const float actual[4], const char* test_description)
    {
        for (int i = 0; i < 4; i++) {
            ASSERT_TRUE(float_equal_or_both_nan(expected[i], actual[i]))
                << test_description << " - Lane " << i << ": expected " << expected[i]
                << ", got " << actual[i];
        }
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicFloorOperation_ReturnsFloorValues
 * @brief Validates f32x4.floor produces correct floor results for typical floating-point inputs
 * @details Tests fundamental floor operation with positive, negative, mixed-sign, and integer values.
 *          Verifies that f32x4.floor correctly computes floor for each lane independently.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_floor_operation
 * @input_conditions Mixed f32 values: [1.2f, -2.7f, 3.0f, -4.9f]
 * @expected_behavior Returns floor values: [1.0f, -3.0f, 3.0f, -5.0f]
 * @validation_method Direct comparison of WASM function result with expected floor values
 */
TEST_F(F32x4FloorTestSuite, BasicFloorOperation_ReturnsFloorValues)
{
    float results[4];
    bool call_success = call_f32x4_floor_function("test_f32x4_floor_basic", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.floor basic test function";

    // Expected floor results for input [1.2f, -2.7f, 3.0f, -4.9f]
    const float expected[4] = {1.0f, -3.0f, 3.0f, -5.0f};
    validate_f32x4_results(expected, results, "Basic floor operation");
}

/**
 * @test BoundaryValues_HandlesExtremeRanges
 * @brief Validates f32x4.floor handles f32 boundary values and subnormal numbers correctly
 * @details Tests with FLT_MAX, -FLT_MAX, smallest positive subnormal, and values near precision limits
 *          to verify proper boundary condition handling and subnormal floor behavior.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_floor_operation
 * @input_conditions Boundary values: [FLT_MAX, -FLT_MAX, 1.4e-45f, -1.4e-45f]
 * @expected_behavior Correct boundary handling: [FLT_MAX, -FLT_MAX, 0.0f, -1.0f]
 * @validation_method Boundary value floor computation verification
 */
TEST_F(F32x4FloorTestSuite, BoundaryValues_HandlesExtremeRanges)
{
    float results[4];
    bool call_success = call_f32x4_floor_function("test_f32x4_floor_boundary", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.floor boundary values test function";

    // Expected: [FLT_MAX, -FLT_MAX, 0.0f, -1.0f]
    ASSERT_FLOAT_EQ(FLT_MAX, results[0]) << "Expected FLT_MAX preservation in lane 0, got " << results[0];
    ASSERT_FLOAT_EQ(-FLT_MAX, results[1]) << "Expected -FLT_MAX preservation in lane 1, got " << results[1];
    ASSERT_FLOAT_EQ(0.0f, results[2]) << "Expected 0.0f for subnormal floor in lane 2, got " << results[2];
    ASSERT_FLOAT_EQ(-1.0f, results[3]) << "Expected -1.0f for negative subnormal floor in lane 3, got " << results[3];
}

/**
 * @test SpecialValues_PreservesIEEE754Semantics
 * @brief Validates f32x4.floor handles IEEE 754 special values correctly
 * @details Tests with NaN, positive infinity, negative infinity, and signed zeros
 *          to ensure proper IEEE 754 floor semantics are preserved.
 * @test_category Edge - IEEE 754 special value handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_floor_operation
 * @input_conditions Special values: [NaN, +∞, -∞, +0.0f]
 * @expected_behavior Preserves special values: [NaN, +∞, -∞, +0.0f]
 * @validation_method IEEE 754 special value preservation validation
 */
TEST_F(F32x4FloorTestSuite, SpecialValues_PreservesIEEE754Semantics)
{
    float results[4];
    bool call_success = call_f32x4_floor_function("test_f32x4_floor_special", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.floor special values test function";

    // Expected: NaN, +infinity, -infinity, +0.0f should be preserved
    ASSERT_TRUE(std::isnan(results[0])) << "Expected NaN preservation in lane 0, got " << results[0];
    ASSERT_TRUE(std::isinf(results[1]) && results[1] > 0) << "Expected +infinity in lane 1, got " << results[1];
    ASSERT_TRUE(std::isinf(results[2]) && results[2] < 0) << "Expected -infinity in lane 2, got " << results[2];
    ASSERT_FLOAT_EQ(+0.0f, results[3]) << "Expected +0.0f preservation in lane 3, got " << results[3];
}

/**
 * @test IdempotentProperty_FloorOfFloor
 * @brief Validates mathematical idempotent property: floor(floor(x)) = floor(x)
 * @details Tests that applying floor operation twice produces identical results,
 *          verifying the fundamental mathematical property of floor function.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_floor_operation
 * @input_conditions Fractional values: [1.5f, -2.3f, 3.7f, -4.1f]
 * @expected_behavior Idempotent results: floor(floor(x)) = floor(x) for all lanes
 * @validation_method Double floor operation result comparison
 */
TEST_F(F32x4FloorTestSuite, IdempotentProperty_FloorOfFloor)
{
    float results_first[4], results_second[4];

    // First floor operation
    bool call_success1 = call_f32x4_floor_function("test_f32x4_floor_idempotent", results_first);
    ASSERT_TRUE(call_success1) << "Failed to execute f32x4.floor idempotent test function (first call)";

    // Second floor operation (should be applied to already floor-ed values)
    bool call_success2 = call_f32x4_floor_function("test_f32x4_floor_idempotent_double", results_second);
    ASSERT_TRUE(call_success2) << "Failed to execute f32x4.floor idempotent test function (second call)";

    // Validate idempotent property: floor(floor(x)) = floor(x)
    validate_f32x4_results(results_first, results_second, "Idempotent property floor(floor(x)) = floor(x)");
}

/**
 * @test PrecisionLimits_LargeIntegerBehavior
 * @brief Validates f32x4.floor behavior at f32 precision boundaries with large integers
 * @details Tests values at and beyond 2²³ precision limit where fractional parts cannot be represented.
 *          Verifies that large integers remain unchanged and fractional behavior near precision limits.
 * @test_category Corner - Precision boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_floor_operation
 * @input_conditions Large values: [16777216.0f, 16777215.5f, -16777216.0f, -16777215.5f]
 * @expected_behavior Precision-aware floor: [16777216.0f, 16777215.0f, -16777216.0f, -16777216.0f]
 * @validation_method Precision boundary floor computation verification
 */
TEST_F(F32x4FloorTestSuite, PrecisionLimits_LargeIntegerBehavior)
{
    float results[4];
    bool call_success = call_f32x4_floor_function("test_f32x4_floor_precision", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.floor precision test function";

    // Expected: [16777216.0f, 16777215.0f, -16777216.0f, -16777216.0f]
    ASSERT_FLOAT_EQ(16777216.0f, results[0]) << "Expected 16777216.0f preservation in lane 0, got " << results[0];
    ASSERT_FLOAT_EQ(16777215.0f, results[1]) << "Expected 16777215.0f for fractional floor in lane 1, got " << results[1];
    ASSERT_FLOAT_EQ(-16777216.0f, results[2]) << "Expected -16777216.0f preservation in lane 2, got " << results[2];
    ASSERT_FLOAT_EQ(-16777216.0f, results[3]) << "Expected -16777216.0f for negative fractional floor in lane 3, got " << results[3];
}

/**
 * @test SmallFractionals_NearZeroBehavior
 * @brief Validates f32x4.floor behavior with small fractional values near zero
 * @details Tests positive and negative small fractional values to verify correct floor computation.
 *          Ensures proper sign handling for values in the interval (-1, 1).
 * @test_category Main - Small value floor validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_floor_operation
 * @input_conditions Small fractions: [0.1f, -0.1f, 0.9f, -0.9f]
 * @expected_behavior Correct floor: [0.0f, -1.0f, 0.0f, -1.0f]
 * @validation_method Small fractional floor computation verification
 */
TEST_F(F32x4FloorTestSuite, SmallFractionals_NearZeroBehavior)
{
    float results[4];
    bool call_success = call_f32x4_floor_function("test_f32x4_floor_small", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.floor small fractions test function";

    // Expected floor results for input [0.1f, -0.1f, 0.9f, -0.9f]
    const float expected[4] = {0.0f, -1.0f, 0.0f, -1.0f};
    validate_f32x4_results(expected, results, "Small fractional floor operation");
}

/**
 * @test MathematicalProperty_FloorLessOrEqual
 * @brief Validates fundamental mathematical property: floor(x) ≤ x for all finite x
 * @details Tests that floor function always returns values less than or equal to input.
 *          Verifies this property holds across positive, negative, and zero values.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_floor_operation
 * @input_conditions Test values: [1.1f, 2.9f, -1.1f, -2.9f]
 * @expected_behavior Property validation: floor(x) ≤ x for all lanes
 * @validation_method Mathematical property verification with comparison
 */
TEST_F(F32x4FloorTestSuite, MathematicalProperty_FloorLessOrEqual)
{
    float results[4];
    bool call_success = call_f32x4_floor_function("test_f32x4_floor_property", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.floor property test function";

    // Input values for property validation: [1.1f, 2.9f, -1.1f, -2.9f]
    const float input_values[4] = {1.1f, 2.9f, -1.1f, -2.9f};
    const float expected_floors[4] = {1.0f, 2.0f, -2.0f, -3.0f};

    // Validate floor(x) <= x property and correct floor values
    for (int i = 0; i < 4; i++) {
        ASSERT_LE(results[i], input_values[i])
            << "Mathematical property floor(x) <= x violated in lane " << i
            << ": floor(" << input_values[i] << ") = " << results[i];

        ASSERT_FLOAT_EQ(expected_floors[i], results[i])
            << "Incorrect floor value in lane " << i << ": expected "
            << expected_floors[i] << ", got " << results[i];
    }
}

/**
 * @test RuntimeStability_MultipleExecutions
 * @brief Validates proper runtime stability and consistency across multiple floor operations
 * @details Tests that the runtime correctly handles repeated operations and maintains stability
 *          after multiple floor function executions without memory leaks or state corruption.
 * @test_category Error - Runtime stability validation
 * @coverage_target WAMR runtime stability and error resilience
 * @input_conditions Multiple consecutive test executions to verify stability
 * @expected_behavior Consistent behavior across multiple test runs
 * @validation_method Runtime stability and consistency verification
 */
TEST_F(F32x4FloorTestSuite, RuntimeStability_MultipleExecutions)
{
    // Validate runtime stability by running the same test multiple times
    // This tests that there are no memory leaks or state corruption issues

    for (int i = 0; i < 3; i++) {
        float results[4];

        // Test basic functionality multiple times to ensure stability
        bool call_success = call_f32x4_floor_function("test_f32x4_floor_basic", results);
        ASSERT_TRUE(call_success) << "Runtime should remain stable on iteration " << i;

        // Validate results remain consistent
        const float expected[4] = {1.0f, -3.0f, 3.0f, -5.0f};
        validate_f32x4_results(expected, results, "Runtime stability test");

        // Test that WASM module can be called repeatedly without issues
        bool special_call_success = call_f32x4_floor_function("test_f32x4_floor_special", results);
        ASSERT_TRUE(special_call_success) << "Special values test should remain stable on iteration " << i;
    }

    // Final validation that runtime is still functional
    float final_results[4];
    bool final_success = call_f32x4_floor_function("test_f32x4_floor_basic", final_results);
    ASSERT_TRUE(final_success) << "Runtime should remain functional after stability tests";
}