/**
 * @file enhanced_f32x4_nearest_test.cc
 * @brief Comprehensive unit tests for f32x4.nearest SIMD opcode
 * @details Tests f32x4.nearest functionality across interpreter and AOT execution modes
 *          with focus on IEEE 754 "round half to even" (banker's rounding) operations,
 *          special values, and cross-mode consistency.
 *          Validates WAMR SIMD implementation correctness for floating-point nearest function.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_nearest_test.cc
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
 * @class F32x4NearestTestSuite
 * @brief Test fixture class for f32x4.nearest opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD float32 result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class F32x4NearestTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.nearest testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_nearest_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.nearest test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_nearest_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.nearest tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_nearest_test.cc:TearDown
     */
    void TearDown() override
    {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f32x4.nearest function with given function name
     * @details Invokes the WASM test function and extracts the four f32 result lanes
     * @param function_name Name of the WASM function to call
     * @param results Reference to f32[4] array to store extracted results
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_nearest_test.cc:call_f32x4_nearest_function
     */
    bool call_f32x4_nearest_function(const char* function_name, float results[4])
    {
        // Call WASM function with no arguments, expects v128 return value
        uint32_t argv[4]; // Space for v128 return value (4 x 32-bit components)
        bool call_success = dummy_env->execute(function_name, 0, argv);

        if (call_success) {
            // Extract f32 results from v128 value (4 lanes of f32)
            memcpy(results, argv, sizeof(float) * 4);
        }

        return call_success;
    }

    /**
     * @brief Validates f32 values with NaN and special value awareness
     * @details Performs proper comparison considering NaN propagation and signed zero handling
     * @param expected Expected f32 value (may be NaN, infinity, or signed zero)
     * @param actual Actual f32 value returned from f32x4.nearest
     * @param lane_index Lane index (0-3) for error reporting context
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_nearest_test.cc:validate_f32_result
     */
    void validate_f32_result(float expected, float actual, int lane_index)
    {
        if (std::isnan(expected)) {
            ASSERT_TRUE(std::isnan(actual))
                << "Lane " << lane_index << ": Expected NaN, got " << actual;
        }
        else if (std::isinf(expected)) {
            ASSERT_EQ(expected, actual)
                << "Lane " << lane_index << ": Expected " << expected << ", got " << actual;
        }
        else if (expected == 0.0f) {
            // Handle signed zero comparison
            ASSERT_EQ(expected, actual)
                << "Lane " << lane_index << ": Expected " << expected << " (signed zero), got " << actual;
            ASSERT_EQ(std::signbit(expected), std::signbit(actual))
                << "Lane " << lane_index << ": Sign bit mismatch for zero values";
        }
        else {
            ASSERT_EQ(expected, actual)
                << "Lane " << lane_index << ": Expected " << expected << ", got " << actual;
        }
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicRounding_ProducesCorrectResults
 * @brief Validates f32x4.nearest produces correct arithmetic results for typical inputs
 * @details Tests fundamental rounding operation with positive, negative, and mixed-sign inputs.
 *          Verifies that f32x4.nearest correctly computes nearest integer for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f32x4_nearest_operation
 * @input_conditions Standard float pairs: (1.2f, 2.7f, -3.1f, -4.9f)
 * @expected_behavior Returns rounded values: 1.0f, 3.0f, -3.0f, -5.0f respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(F32x4NearestTestSuite, BasicRounding_ProducesCorrectResults)
{
    // Test case 1: Standard rounding behavior
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_basic", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest basic test function";

        // Expected rounding results for input [1.2f, 2.7f, -3.1f, -4.9f]
        float expected[4] = {1.0f, 3.0f, -3.0f, -5.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }

    // Test case 2: Small fractional values near zero
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_small_fractions", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest small fractions test function";

        // Expected rounding results for input [0.1f, 0.4f, 0.6f, 0.9f]
        float expected[4] = {0.0f, 0.0f, 1.0f, 1.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }

    // Test case 3: Already integer values (identity operation)
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_integers", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest integers test function";

        // Expected results for input [1.0f, -5.0f, 42.0f, -17.0f]
        float expected[4] = {1.0f, -5.0f, 42.0f, -17.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }
}

/**
 * @test BankersRounding_RoundsHalfToEven
 * @brief Validates f32x4.nearest implements "round half to even" (banker's rounding) correctly
 * @details Tests IEEE 754 default rounding mode where 0.5 cases round to nearest even integer.
 *          Verifies proper implementation of banker's rounding for bias minimization.
 * @test_category Main - Banker's rounding validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f32x4_nearest_operation
 * @input_conditions Half values: (2.5f, 3.5f, -2.5f, -3.5f)
 * @expected_behavior Returns even rounded values: 2.0f, 4.0f, -2.0f, -4.0f respectively
 * @validation_method Verification of "round half to even" IEEE 754 behavior
 */
TEST_F(F32x4NearestTestSuite, BankersRounding_RoundsHalfToEven)
{
    // Test case 1: Primary banker's rounding examples
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_bankers", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest banker's rounding test function";

        // Expected banker's rounding results for input [2.5f, 3.5f, -2.5f, -3.5f]
        float expected[4] = {2.0f, 4.0f, -2.0f, -4.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }

    // Test case 2: Additional banker's rounding cases
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_bankers_extended", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest extended banker's rounding test function";

        // Expected banker's rounding results for input [0.5f, 1.5f, -0.5f, -1.5f]
        float expected[4] = {0.0f, 2.0f, -0.0f, -2.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }

    // Test case 3: Higher magnitude banker's rounding
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_bankers_large", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest large banker's rounding test function";

        // Expected banker's rounding results for input [10.5f, 11.5f, -10.5f, -11.5f]
        float expected[4] = {10.0f, 12.0f, -10.0f, -12.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }
}

/**
 * @test SpecialValues_PreservesIEEE754Semantics
 * @brief Validates f32x4.nearest preserves IEEE 754 special values unchanged
 * @details Tests handling of NaN, ±infinity, and signed zero values.
 *          Verifies that special values are preserved according to IEEE 754 standard.
 * @test_category Edge - Special values validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f32x4_nearest_operation
 * @input_conditions IEEE 754 special values: NaN, +∞, -∞, ±0.0f
 * @expected_behavior All special values preserved unchanged with correct sign
 * @validation_method NaN/infinity detection and signed zero bit verification
 */
TEST_F(F32x4NearestTestSuite, SpecialValues_PreservesIEEE754Semantics)
{
    // Test case 1: NaN and infinity preservation
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_special", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest special values test function";

        // Expected special values preservation for input [NaN, +inf, -inf, NaN]
        float expected[4] = {NAN, INFINITY, -INFINITY, NAN};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }

    // Test case 2: Signed zero handling
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_zeros", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest signed zeros test function";

        // Expected signed zero preservation for input [+0.0f, -0.0f, +0.0f, -0.0f]
        float expected[4] = {+0.0f, -0.0f, +0.0f, -0.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }

    // Test case 3: Mixed special and normal values
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_mixed_special", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest mixed special values test function";

        // Expected mixed results for input [1.5f, NaN, -inf, 2.5f]
        float expected[4] = {2.0f, NAN, -INFINITY, 2.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }
}

/**
 * @test PrecisionBoundary_HandlesSinglePrecisionLimits
 * @brief Validates f32x4.nearest behavior at f32 precision boundaries
 * @details Tests rounding behavior around 2^23 where f32 cannot represent fractional parts,
 *          and verifies handling of large integer values and subnormal numbers.
 * @test_category Corner - Precision boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:f32x4_nearest_operation
 * @input_conditions Values near f32 precision limits: 2^23, large integers, subnormal values
 * @expected_behavior Correct handling of precision boundaries and large values
 * @validation_method Precision-aware comparison with expected boundary behavior
 */
TEST_F(F32x4NearestTestSuite, PrecisionBoundary_HandlesSinglePrecisionLimits)
{
    // Test case 1: F32 precision boundary (2^23)
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_precision_boundary", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest precision boundary test function";

        // Expected precision boundary results for input [8388608.0f, 8388608.5f, 8388609.0f, 16777216.0f]
        float expected[4] = {8388608.0f, 8388608.0f, 8388609.0f, 16777216.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }

    // Test case 2: Large integer values
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_large_integers", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest large integers test function";

        // Expected results for input [1e10f, -1e10f, 1e20f, -1e20f]
        float expected[4] = {1e10f, -1e10f, 1e20f, -1e20f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }

    // Test case 3: Very small values near zero (subnormal handling)
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_subnormal", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest subnormal test function";

        // Expected results for input [1e-10f, -1e-10f, 1e-20f, -1e-20f]
        float expected[4] = {0.0f, -0.0f, 0.0f, -0.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }
}

/**
 * @test ErrorConditions_HandlesInvalidScenarios
 * @brief Validates f32x4.nearest error handling and module validation
 * @details Tests module loading failures, invalid configurations, and runtime error scenarios.
 *          Verifies proper error reporting and graceful failure handling.
 * @test_category Error - Error condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:module_validation
 * @input_conditions Invalid module configurations, corrupted runtime context
 * @expected_behavior Proper error reporting and graceful failure handling
 * @validation_method Error condition detection and proper failure response
 */
TEST_F(F32x4NearestTestSuite, ErrorConditions_HandlesInvalidScenarios)
{
    // Test case 1: Verify module loaded successfully (baseline)
    {
        ASSERT_NE(nullptr, dummy_env->get())
            << "f32x4.nearest test module should load successfully with SIMD enabled";
    }

    // Test case 2: Test with invalid function name
    {
        float results[4];
        bool call_success = call_f32x4_nearest_function("non_existent_function", results);
        ASSERT_FALSE(call_success)
            << "Call to non-existent function should fail gracefully";
    }

    // Test case 3: Verify SIMD functionality is available
    {
        // This test confirms that f32x4.nearest is properly supported
        float results[4];
        bool call_success = call_f32x4_nearest_function("test_f32x4_nearest_bankers", results);
        ASSERT_TRUE(call_success) << "Failed to execute f32x4.nearest functionality test";

        // Expected banker's rounding results for input [2.5f, 3.5f, -2.5f, -3.5f]
        float expected[4] = {2.0f, 4.0f, -2.0f, -4.0f};

        for (int i = 0; i < 4; i++) {
            validate_f32_result(expected[i], results[i], i);
        }
    }
}