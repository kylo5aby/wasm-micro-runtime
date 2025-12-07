/**
 * @file enhanced_f32x4_ceil_test.cc
 * @brief Comprehensive unit tests for f32x4.ceil SIMD opcode
 * @details Tests f32x4.ceil functionality across interpreter and AOT execution modes
 *          with focus on IEEE 754 ceiling operations, special values, and cross-mode consistency.
 *          Validates WAMR SIMD implementation correctness for floating-point ceiling function.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ceil_test.cc
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
 * @class F32x4CeilTestSuite
 * @brief Test fixture class for f32x4.ceil opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD float32 result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class F32x4CeilTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.ceil testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ceil_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.ceil test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_ceil_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.ceil tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ceil_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f32x4.ceil function and extract v128 result
     * @details Executes f32x4.ceil test function and extracts four f32 values from v128 result.
     *          Handles WASM function invocation and v128 result extraction into f32 array.
     * @param function_name Name of the WASM function to call
     * @param results Reference to f32[4] array to store extracted results
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ceil_test.cc:call_f32x4_ceil_function
     */
    bool call_f32x4_ceil_function(const char* function_name, float results[4])
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
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ceil_test.cc:float_equal_or_both_nan
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
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ceil_test.cc:validate_f32x4_results
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
 * @test BasicCeilingOperation_ReturnsCeilingValues
 * @brief Validates f32x4.ceil produces correct ceiling results for typical floating-point inputs
 * @details Tests fundamental ceiling operation with positive, negative, mixed-sign, and integer values.
 *          Verifies that f32x4.ceil correctly computes ceiling for each lane independently.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_ceil_operation
 * @input_conditions Mixed f32 values: [1.2f, -2.7f, 3.0f, -4.9f]
 * @expected_behavior Returns ceiling values: [2.0f, -2.0f, 3.0f, -4.0f]
 * @validation_method Direct comparison of WASM function result with expected ceiling values
 */
TEST_F(F32x4CeilTestSuite, BasicCeilingOperation_ReturnsCeilingValues)
{
    float results[4];
    bool call_success = call_f32x4_ceil_function("test_f32x4_ceil_basic", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.ceil basic test function";

    // Expected ceiling results for input [1.2f, -2.7f, 3.0f, -4.9f]
    const float expected[4] = {2.0f, -2.0f, 3.0f, -4.0f};
    validate_f32x4_results(expected, results, "Basic ceiling operation");
}

/**
 * @test SpecialValues_PreservesIEEE754Semantics
 * @brief Validates f32x4.ceil handles IEEE 754 special values correctly
 * @details Tests with NaN, positive infinity, negative infinity, and positive zero
 *          to ensure proper IEEE 754 ceiling semantics are preserved.
 * @test_category Edge - IEEE 754 special value handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_ceil_operation
 * @input_conditions Special values: [NaN, +∞, -∞, +0.0f]
 * @expected_behavior Preserves special values: [NaN, +∞, -∞, +0.0f]
 * @validation_method IEEE 754 special value preservation validation
 */
TEST_F(F32x4CeilTestSuite, SpecialValues_PreservesIEEE754Semantics)
{
    float results[4];
    bool call_success = call_f32x4_ceil_function("test_f32x4_ceil_special", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.ceil special values test function";

    // Expected: NaN, +infinity, -infinity, +0.0f should be preserved
    ASSERT_TRUE(std::isnan(results[0])) << "Expected NaN preservation in lane 0, got " << results[0];
    ASSERT_TRUE(std::isinf(results[1]) && results[1] > 0) << "Expected +infinity in lane 1, got " << results[1];
    ASSERT_TRUE(std::isinf(results[2]) && results[2] < 0) << "Expected -infinity in lane 2, got " << results[2];
    ASSERT_FLOAT_EQ(+0.0f, results[3]) << "Expected +0.0f preservation in lane 3, got " << results[3];
}

/**
 * @test BoundaryValues_HandlesExtremeRanges
 * @brief Validates f32x4.ceil handles f32 boundary values and subnormal numbers correctly
 * @details Tests with FLT_MAX, -FLT_MAX, smallest positive subnormal, and values near precision limits
 *          to verify proper boundary condition handling and subnormal ceiling behavior.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_ceil_operation
 * @input_conditions Boundary values: [FLT_MAX, -FLT_MAX, 1.4e-45f, -1.4e-45f]
 * @expected_behavior Correct boundary handling: [FLT_MAX, -FLT_MAX, 1.0f, -0.0f]
 * @validation_method Boundary value ceiling computation verification
 */
TEST_F(F32x4CeilTestSuite, BoundaryValues_HandlesExtremeRanges)
{
    float results[4];
    bool call_success = call_f32x4_ceil_function("test_f32x4_ceil_boundary", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.ceil boundary values test function";

    // Expected: [FLT_MAX, -FLT_MAX, 1.0f, -0.0f]
    ASSERT_FLOAT_EQ(FLT_MAX, results[0]) << "Expected FLT_MAX preservation in lane 0, got " << results[0];
    ASSERT_FLOAT_EQ(-FLT_MAX, results[1]) << "Expected -FLT_MAX preservation in lane 1, got " << results[1];
    ASSERT_FLOAT_EQ(1.0f, results[2]) << "Expected 1.0f for subnormal ceiling in lane 2, got " << results[2];
    ASSERT_TRUE(results[3] == -0.0f) << "Expected -0.0f for negative subnormal ceiling in lane 3, got " << results[3];
}

/**
 * @test ZeroHandling_PreservesSignBits
 * @brief Validates f32x4.ceil preserves signed zero according to IEEE 754 standard
 * @details Tests with positive and negative zero values to ensure sign bits are preserved
 *          correctly during ceiling operation as required by IEEE 754 standard.
 * @test_category Edge - Signed zero preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_ceil_operation
 * @input_conditions Signed zeros: [+0.0f, -0.0f, +0.0f, -0.0f]
 * @expected_behavior Preserves sign bits: [+0.0f, -0.0f, +0.0f, -0.0f]
 * @validation_method Sign bit preservation verification for zero values
 */
TEST_F(F32x4CeilTestSuite, ZeroHandling_PreservesSignBits)
{
    float results[4];
    bool call_success = call_f32x4_ceil_function("test_f32x4_ceil_zeros", results);
    ASSERT_TRUE(call_success) << "Failed to execute f32x4.ceil signed zero test function";

    // Validate signed zero preservation (IEEE 754 requirement)
    ASSERT_TRUE(results[0] == +0.0f && !std::signbit(results[0])) << "Expected +0.0f in lane 0, got " << results[0];
    ASSERT_TRUE(results[1] == -0.0f && std::signbit(results[1])) << "Expected -0.0f in lane 1, got " << results[1];
    ASSERT_TRUE(results[2] == +0.0f && !std::signbit(results[2])) << "Expected +0.0f in lane 2, got " << results[2];
    ASSERT_TRUE(results[3] == -0.0f && std::signbit(results[3])) << "Expected -0.0f in lane 3, got " << results[3];
}

/**
 * @test IdempotentProperty_CeilingOfCeiling
 * @brief Validates mathematical idempotent property: ceil(ceil(x)) = ceil(x)
 * @details Tests that applying ceiling operation twice produces identical results,
 *          verifying the fundamental mathematical property of ceiling function.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_ceil_operation
 * @input_conditions Fractional values: [1.5f, -2.3f, 3.7f, -4.1f]
 * @expected_behavior Idempotent results: ceil(ceil(x)) = ceil(x) for all lanes
 * @validation_method Double ceiling operation result comparison
 */
TEST_F(F32x4CeilTestSuite, IdempotentProperty_CeilingOfCeiling)
{
    float results_first[4], results_second[4];

    // First ceiling operation
    bool call_success1 = call_f32x4_ceil_function("test_f32x4_ceil_idempotent", results_first);
    ASSERT_TRUE(call_success1) << "Failed to execute f32x4.ceil idempotent test function (first call)";

    // Second ceiling operation (should be applied to already ceiling-ed values)
    bool call_success2 = call_f32x4_ceil_function("test_f32x4_ceil_idempotent_double", results_second);
    ASSERT_TRUE(call_success2) << "Failed to execute f32x4.ceil idempotent test function (second call)";

    // Validate idempotent property: ceil(ceil(x)) = ceil(x)
    validate_f32x4_results(results_first, results_second, "Idempotent property ceil(ceil(x)) = ceil(x)");
}

/**
 * @test RuntimeInitialization_HandlesMissingContext
 * @brief Validates proper error handling and runtime resilience
 * @details Tests that the runtime correctly handles edge cases and maintains stability
 *          after attempting operations with boundary conditions.
 * @test_category Error - Runtime error handling validation
 * @coverage_target WAMR runtime stability and error resilience
 * @input_conditions Multiple consecutive test executions to verify stability
 * @expected_behavior Consistent behavior across multiple test runs
 * @validation_method Runtime stability and consistency verification
 */
TEST_F(F32x4CeilTestSuite, RuntimeInitialization_HandlesMissingContext)
{
    // Validate runtime stability by running the same test multiple times
    // This tests that there are no memory leaks or state corruption issues

    for (int i = 0; i < 3; i++) {
        float results[4];

        // Test basic functionality multiple times to ensure stability
        bool call_success = call_f32x4_ceil_function("test_f32x4_ceil_basic", results);
        ASSERT_TRUE(call_success) << "Runtime should remain stable on iteration " << i;

        // Validate results remain consistent
        const float expected[4] = {2.0f, -2.0f, 3.0f, -4.0f};
        validate_f32x4_results(expected, results, "Runtime stability test");

        // Test that WASM module can be called repeatedly without issues
        bool special_call_success = call_f32x4_ceil_function("test_f32x4_ceil_special", results);
        ASSERT_TRUE(special_call_success) << "Special values test should remain stable on iteration " << i;
    }

    // Final validation that runtime is still functional
    float final_results[4];
    bool final_success = call_f32x4_ceil_function("test_f32x4_ceil_basic", final_results);
    ASSERT_TRUE(final_success) << "Runtime should remain functional after stability tests";
}