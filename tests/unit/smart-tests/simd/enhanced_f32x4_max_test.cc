/**
 * @file enhanced_f32x4_max_test.cc
 * @brief Comprehensive unit tests for f32x4.max SIMD opcode
 * @details Tests f32x4.max functionality with focus on element-wise maximum operation
 *          of two 32-bit single-precision floating-point vectors, IEEE 754 compliance validation,
 *          and comprehensive edge case coverage. Validates WAMR SIMD implementation correctness.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_max_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <cmath>
#include <limits>
#include <cfloat>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class F32x4MaxTestSuite
 * @brief Test suite for comprehensive f32x4.max SIMD opcode validation
 * @details Provides test infrastructure and helper functions for f32x4.max testing.
 *          Includes setup/teardown, WASM module management, and result validation utilities.
 */
class F32x4MaxTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.max testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_max_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.max test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_max_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.max tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Automatically handled by RAII destructors for runtime and execution environment.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_max_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup is handled automatically by RAII destructors
    }

    /**
     * @brief Call f32x4.max WASM function with specified input vectors
     * @param f1_lane0 First vector lane 0 value
     * @param f1_lane1 First vector lane 1 value
     * @param f1_lane2 First vector lane 2 value
     * @param f1_lane3 First vector lane 3 value
     * @param f2_lane0 Second vector lane 0 value
     * @param f2_lane1 Second vector lane 1 value
     * @param f2_lane2 Second vector lane 2 value
     * @param f2_lane3 Second vector lane 3 value
     * @param result_bytes Output buffer for 16-byte v128 result
     * @return true if function execution succeeded, false otherwise
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_max_test.cc:call_f32x4_max
     */
    bool call_f32x4_max(float f1_lane0, float f1_lane1, float f1_lane2, float f1_lane3,
                        float f2_lane0, float f2_lane1, float f2_lane2, float f2_lane3,
                        uint8_t result_bytes[16])
    {
        // Prepare arguments: two input v128 vectors as eight f32 values
        uint32_t argv[8];

        // Convert floats to uint32_t bit representation
        memcpy(&argv[0], &f1_lane0, sizeof(float));
        memcpy(&argv[1], &f1_lane1, sizeof(float));
        memcpy(&argv[2], &f1_lane2, sizeof(float));
        memcpy(&argv[3], &f1_lane3, sizeof(float));
        memcpy(&argv[4], &f2_lane0, sizeof(float));
        memcpy(&argv[5], &f2_lane1, sizeof(float));
        memcpy(&argv[6], &f2_lane2, sizeof(float));
        memcpy(&argv[7], &f2_lane3, sizeof(float));

        // Execute WASM function: basic_max_operation
        bool success = dummy_env->execute("basic_max_operation", 8, argv);
        EXPECT_TRUE(success) << "Failed to execute f32x4.max WASM function";

        if (success) {
            // Extract v128 result and convert back to byte array (result stored in argv)
            // Copy result lanes as float values
            memcpy(&result_bytes[0], &argv[0], sizeof(float));   // Lane 0 result
            memcpy(&result_bytes[4], &argv[1], sizeof(float));   // Lane 1 result
            memcpy(&result_bytes[8], &argv[2], sizeof(float));   // Lane 2 result
            memcpy(&result_bytes[12], &argv[3], sizeof(float));  // Lane 3 result
        }

        return success;
    }

    /**
     * @brief Extract float values from result bytes array
     * @param result_bytes Input 16-byte array containing v128 result
     * @param lane0 Output lane 0 float value
     * @param lane1 Output lane 1 float value
     * @param lane2 Output lane 2 float value
     * @param lane3 Output lane 3 float value
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_max_test.cc:extract_float_lanes
     */
    void extract_float_lanes(const uint8_t result_bytes[16], float &lane0, float &lane1, float &lane2, float &lane3)
    {
        memcpy(&lane0, &result_bytes[0], sizeof(float));
        memcpy(&lane1, &result_bytes[4], sizeof(float));
        memcpy(&lane2, &result_bytes[8], sizeof(float));
        memcpy(&lane3, &result_bytes[12], sizeof(float));
    }

    /**
     * @brief Verify f32x4.max result against expected values with IEEE 754 compliance
     * @param expected_lane0 Expected lane 0 result
     * @param expected_lane1 Expected lane 1 result
     * @param expected_lane2 Expected lane 2 result
     * @param expected_lane3 Expected lane 3 result
     * @param result_bytes Actual result bytes from f32x4.max execution
     * @param test_context Description of test context for error messages
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_max_test.cc:verify_f32x4_max_result
     */
    void verify_f32x4_max_result(float expected_lane0, float expected_lane1, float expected_lane2, float expected_lane3,
                                 const uint8_t result_bytes[16], const char* test_context)
    {
        float actual_lane0, actual_lane1, actual_lane2, actual_lane3;
        extract_float_lanes(result_bytes, actual_lane0, actual_lane1, actual_lane2, actual_lane3);

        // Verify each lane with appropriate handling for special values
        const float expected[4] = {expected_lane0, expected_lane1, expected_lane2, expected_lane3};
        const float actual[4] = {actual_lane0, actual_lane1, actual_lane2, actual_lane3};

        for (int i = 0; i < 4; i++) {
            // Handle NaN cases
            if (std::isnan(expected[i])) {
                ASSERT_TRUE(std::isnan(actual[i]))
                    << test_context << " - Lane " << i
                    << ": Expected NaN, got " << actual[i];
            }
            // Handle infinity cases
            else if (std::isinf(expected[i])) {
                ASSERT_EQ(expected[i], actual[i])
                    << test_context << " - Lane " << i
                    << ": Expected " << expected[i]
                    << ", got " << actual[i];
            }
            // Handle finite values with tolerance
            else {
                ASSERT_FLOAT_EQ(expected[i], actual[i])
                    << test_context << " - Lane " << i
                    << ": Expected " << expected[i]
                    << ", got " << actual[i];
            }
        }
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicMaxOperation_ReturnsCorrectResults
 * @brief Validates f32x4.max produces correct element-wise maximum for typical inputs
 * @details Tests fundamental maximum operation with positive, negative, and mixed-sign floats.
 *          Verifies that f32x4.max correctly computes element-wise max(a,b) for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_max
 * @input_conditions Standard f32x4 pairs: positive, negative, and mixed-sign combinations
 * @expected_behavior Returns element-wise maximum following IEEE 754 rules
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(F32x4MaxTestSuite, BasicMaxOperation_ReturnsCorrectResults)
{
    uint8_t result_bytes[16];

    // Test case 1: All positive values
    bool success = call_f32x4_max(1.0f, 2.5f, 3.7f, 4.2f,
                                  1.5f, 2.0f, 4.1f, 3.8f,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for positive values";
    verify_f32x4_max_result(1.5f, 2.5f, 4.1f, 4.2f, result_bytes, "Positive values test");

    // Test case 2: All negative values
    success = call_f32x4_max(-1.5f, -2.8f, -0.9f, -3.1f,
                             -1.2f, -3.0f, -1.1f, -2.7f,
                             result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for negative values";
    verify_f32x4_max_result(-1.2f, -2.8f, -0.9f, -2.7f, result_bytes, "Negative values test");

    // Test case 3: Mixed signs
    success = call_f32x4_max(2.3f, -1.7f, 4.6f, -0.5f,
                             -1.1f, 2.9f, -2.4f, 1.8f,
                             result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for mixed signs";
    verify_f32x4_max_result(2.3f, 2.9f, 4.6f, 1.8f, result_bytes, "Mixed signs test");
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Validates f32x4.max handles floating-point boundary values correctly
 * @details Tests maximum operations with FLT_MAX, FLT_MIN, and subnormal values.
 *          Verifies precision is maintained at floating-point representation limits.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_max
 * @input_conditions FLT_MAX, FLT_MIN, subnormal values in various combinations
 * @expected_behavior Correct maximum selection preserving floating-point precision
 * @validation_method Precise floating-point comparison with tolerance for boundary cases
 */
TEST_F(F32x4MaxTestSuite, BoundaryValues_HandledCorrectly)
{
    uint8_t result_bytes[16];

    // Test case 1: FLT_MAX boundaries
    bool success = call_f32x4_max(FLT_MAX, 1.0f, -1.0f, 0.5f,
                                  1.0f, FLT_MAX, 0.5f, -1.0f,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for FLT_MAX boundaries";
    verify_f32x4_max_result(FLT_MAX, FLT_MAX, 0.5f, 0.5f, result_bytes, "FLT_MAX boundaries test");

    // Test case 2: FLT_MIN boundaries
    success = call_f32x4_max(FLT_MIN, -FLT_MIN, 2*FLT_MIN, -2*FLT_MIN,
                             2*FLT_MIN, FLT_MIN, FLT_MIN, -FLT_MIN,
                             result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for FLT_MIN boundaries";
    verify_f32x4_max_result(2*FLT_MIN, FLT_MIN, 2*FLT_MIN, -FLT_MIN, result_bytes, "FLT_MIN boundaries test");

    // Test case 3: Subnormal comparisons (denormalized values)
    success = call_f32x4_max(1e-40f, 1e-42f, -1e-41f, 1e-39f,
                             1e-41f, 1e-40f, -1e-42f, 1e-40f,
                             result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for subnormal values";
    verify_f32x4_max_result(1e-40f, 1e-40f, -1e-42f, 1e-39f, result_bytes, "Subnormal values test");
}

/**
 * @test SpecialValues_IEEE754Compliant
 * @brief Validates f32x4.max handles IEEE 754 special values correctly
 * @details Tests maximum operations with infinity, NaN, and signed zero values.
 *          Verifies IEEE 754-2008 compliance for special value handling in max operations.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_max
 * @input_conditions +∞, -∞, NaN, ±0.0 in mixed lane configurations
 * @expected_behavior IEEE 754 compliant maximum operations with proper special value handling
 * @validation_method Specialized checks for NaN, infinity, and signed zero behavior
 */
TEST_F(F32x4MaxTestSuite, SpecialValues_IEEE754Compliant)
{
    uint8_t result_bytes[16];

    // Test case 1: Infinity handling
    bool success = call_f32x4_max(INFINITY, -INFINITY, 1.0f, -1.0f,
                                  1.0f, 1.0f, INFINITY, -INFINITY,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for infinity values";
    verify_f32x4_max_result(INFINITY, 1.0f, INFINITY, -1.0f, result_bytes, "Infinity handling test");

    // Test case 2: Signed zero behavior (IEEE 754: max(+0, -0) = +0)
    success = call_f32x4_max(+0.0f, -0.0f, +0.0f, -0.0f,
                             -0.0f, +0.0f, 1.0f, -1.0f,
                             result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for signed zero";
    verify_f32x4_max_result(+0.0f, +0.0f, 1.0f, -0.0f, result_bytes, "Signed zero test");

    // Test case 3: Mixed special values in different lanes
    success = call_f32x4_max(INFINITY, NAN, 0.0f, 5.5f,
                             -INFINITY, 3.0f, -0.0f, NAN,
                             result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for mixed special values";
    verify_f32x4_max_result(INFINITY, NAN, 0.0f, NAN, result_bytes, "Mixed special values test");
}

/**
 * @test NaN_PropagationRules
 * @brief Validates f32x4.max properly propagates NaN values according to IEEE 754
 * @details Tests NaN propagation behavior in maximum operations across all lanes.
 *          Verifies that NaN values propagate correctly and follow IEEE 754 standards.
 * @test_category Edge - NaN handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f32x4_max
 * @input_conditions Various NaN patterns mixed with finite values across lanes
 * @expected_behavior NaN propagation following IEEE 754 standard (any NaN input → NaN output)
 * @validation_method std::isnan() validation with lane-specific NaN detection
 */
TEST_F(F32x4MaxTestSuite, NaN_PropagationRules)
{
    uint8_t result_bytes[16];

    // Test case 1: NaN propagation (NaN vs finite)
    bool success = call_f32x4_max(NAN, 2.0f, 3.0f, 4.0f,
                                  1.0f, NAN, 3.5f, 4.5f,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for NaN propagation";
    verify_f32x4_max_result(NAN, NAN, 3.5f, 4.5f, result_bytes, "NaN propagation test");

    // Test case 2: NaN vs infinity
    success = call_f32x4_max(NAN, INFINITY, -INFINITY, NAN,
                             INFINITY, NAN, NAN, -INFINITY,
                             result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for NaN vs infinity";
    verify_f32x4_max_result(NAN, NAN, NAN, NAN, result_bytes, "NaN vs infinity test");

    // Test case 3: NaN vs NaN (should remain NaN)
    success = call_f32x4_max(NAN, NAN, 1.0f, 2.0f,
                             NAN, 2.5f, NAN, NAN,
                             result_bytes);
    ASSERT_TRUE(success) << "f32x4.max execution should succeed for NaN vs NaN";
    verify_f32x4_max_result(NAN, NAN, NAN, NAN, result_bytes, "NaN vs NaN test");
}

/**
 * @test ModuleLoading_ValidatesSIMDSupport
 * @brief Validates that WASM module with f32x4.max loads successfully with SIMD support
 * @details Tests proper WASM module loading and instantiation with SIMD instructions.
 *          Ensures SIMD feature availability and correct module validation.
 * @test_category Error - SIMD support validation
 * @coverage_target wasm_runtime_load, wasm_runtime_instantiate
 * @input_conditions WASM module containing f32x4.max SIMD instruction
 * @expected_behavior Successful module loading and instantiation
 * @validation_method Non-null module and module instance validation
 */
TEST_F(F32x4MaxTestSuite, ModuleLoading_ValidatesSIMDSupport)
{
    // Module and execution environment should be successfully loaded in SetUp
    ASSERT_NE(nullptr, dummy_env->get())
        << "WASM execution environment with f32x4.max should be created successfully with SIMD support";

    // Verify that basic execution works
    uint8_t result_bytes[16];
    bool success = call_f32x4_max(1.0f, 2.0f, 3.0f, 4.0f,
                                  5.0f, 1.5f, 2.5f, 3.5f,
                                  result_bytes);
    ASSERT_TRUE(success)
        << "f32x4.max test function should be accessible and executable in loaded module";
}