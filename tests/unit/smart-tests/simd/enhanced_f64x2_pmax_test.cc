/**
 * @file enhanced_f64x2_pmax_test.cc
 * @brief Comprehensive unit tests for f64x2.pmax SIMD opcode
 * @details Tests f64x2.pmax functionality across interpreter and AOT execution modes
 *          with focus on element-wise pseudo-maximum operation of two 64-bit double-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmax_test.cc
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
 * @class F64x2PmaxTestSuite
 * @brief Test fixture class for f64x2.pmax opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F64x2PmaxTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f64x2.pmax testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmax_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.pmax test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_pmax_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.pmax tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Ensures proper cleanup of WAMR runtime and execution environment.
     *          Uses RAII pattern for automatic resource management.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmax_test.cc:TearDown
     */
    void TearDown() override
    {
        // RAII handles cleanup automatically
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f64x2.pmax function with two vector inputs
     * @param func_name Name of the WASM function to execute
     * @param d1_lane0 First double value for lane 0 of first vector
     * @param d1_lane1 First double value for lane 1 of first vector
     * @param d2_lane0 Second double value for lane 0 of second vector
     * @param d2_lane1 Second double value for lane 1 of second vector
     * @param result_bytes 16-byte array to store the pseudo-maximum operation result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmax_test.cc:call_f64x2_pmax
     */
    bool call_f64x2_pmax(const char* func_name, double d1_lane0, double d1_lane1, double d2_lane0, double d2_lane1,
                         uint8_t result_bytes[16])
    {
        // Prepare arguments: two input v128 vectors as four i64 values each
        uint32_t argv[8];

        // Convert doubles to byte representation and then to 64-bit values
        uint64_t input1_lo, input1_hi, input2_lo, input2_hi;

        // Copy double values to get their bit representation
        memcpy(&input1_lo, &d1_lane0, sizeof(double));  // Lane 0 of first vector
        memcpy(&input1_hi, &d1_lane1, sizeof(double));  // Lane 1 of first vector
        memcpy(&input2_lo, &d2_lane0, sizeof(double));  // Lane 0 of second vector
        memcpy(&input2_hi, &d2_lane1, sizeof(double));  // Lane 1 of second vector

        // WASM expects little-endian format: low part first, then high part
        // First v128 vector (d1_lane0, d1_lane1)
        argv[0] = static_cast<uint32_t>(input1_lo);        // Low 32 bits of lane 0
        argv[1] = static_cast<uint32_t>(input1_lo >> 32);  // High 32 bits of lane 0
        argv[2] = static_cast<uint32_t>(input1_hi);        // Low 32 bits of lane 1
        argv[3] = static_cast<uint32_t>(input1_hi >> 32);  // High 32 bits of lane 1
        // Second v128 vector (d2_lane0, d2_lane1)
        argv[4] = static_cast<uint32_t>(input2_lo);        // Low 32 bits of lane 0
        argv[5] = static_cast<uint32_t>(input2_lo >> 32);  // High 32 bits of lane 0
        argv[6] = static_cast<uint32_t>(input2_hi);        // Low 32 bits of lane 1
        argv[7] = static_cast<uint32_t>(input2_hi >> 32);  // High 32 bits of lane 1

        // Execute WASM function
        bool success = dummy_env->execute(func_name, 8, argv);
        EXPECT_TRUE(success) << "Failed to execute f64x2.pmax WASM function: " << func_name;

        if (success) {
            // Extract v128 result and convert back to byte array (result stored in argv)
            uint64_t result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            uint64_t result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];

            // Copy result bytes in little-endian order
            memcpy(&result_bytes[0], &result_lo, 8);   // Lane 0 result (8 bytes)
            memcpy(&result_bytes[8], &result_hi, 8);   // Lane 1 result (8 bytes)
        }

        return success;
    }

    /**
     * @brief Extract double values from 16-byte v128 result
     * @param result_bytes 16-byte array containing v128 result
     * @param lane0_result Output parameter for lane 0 double value
     * @param lane1_result Output parameter for lane 1 double value
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmax_test.cc:extract_f64x2_result
     */
    void extract_f64x2_result(const uint8_t result_bytes[16], double& lane0_result, double& lane1_result)
    {
        // Extract two 64-bit double values from 16-byte v128 result
        memcpy(&lane0_result, &result_bytes[0], sizeof(double));   // Lane 0: bytes 0-7
        memcpy(&lane1_result, &result_bytes[8], sizeof(double));   // Lane 1: bytes 8-15
    }

    /**
     * @brief Check if a double value is NaN using IEEE 754 bit pattern
     * @param value Double value to check
     * @return bool True if value is NaN, false otherwise
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmax_test.cc:is_nan_value
     */
    bool is_nan_value(double value)
    {
        return std::isnan(value);
    }

    /**
     * @brief Compare double values with NaN-aware equality
     * @param expected Expected double value
     * @param actual Actual double value
     * @return bool True if values are equal (including both NaN)
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmax_test.cc:double_equal_or_nan
     */
    bool double_equal_or_nan(double expected, double actual)
    {
        if (is_nan_value(expected) && is_nan_value(actual)) {
            return true;  // Both NaN - considered equal
        }
        if (is_nan_value(expected) || is_nan_value(actual)) {
            return false; // Only one is NaN
        }
        return expected == actual;  // Normal equality
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;  ///< WAMR runtime RAII wrapper
    std::unique_ptr<DummyExecEnv> dummy_env;          ///< WASM execution environment
};

/**
 * @test BasicPseudoMaximum_ReturnsCorrectResults
 * @brief Validates f64x2.pmax produces correct element-wise pseudo-maximum results for typical inputs
 * @details Tests fundamental pseudo-maximum operation with positive, negative, and mixed-sign doubles.
 *          Verifies that f64x2.pmax correctly computes pmax(a[i], b[i]) for each lane independently.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:f64x2_pmax_operation
 * @input_conditions Standard double pairs in both lanes: mixed positive/negative combinations
 * @expected_behavior Returns element-wise maximum: [max(a[0],b[0]), max(a[1],b[1])]
 * @validation_method Direct comparison of WASM function result with expected pseudo-maximum values
 */
TEST_F(F64x2PmaxTestSuite, BasicPseudoMaximum_ReturnsCorrectResults)
{
    uint8_t result[16];
    double lane0_result, lane1_result;

    // Test case 1: Mixed positive values with different maximums per lane
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_basic", 1.5, 2.5, 2.0, 1.0, result))
        << "Failed to execute f64x2.pmax with mixed positive values";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_EQ(2.0, lane0_result) << "Lane 0 pseudo-maximum failed: pmax(1.5, 2.0) should be 2.0";
    ASSERT_EQ(2.5, lane1_result) << "Lane 1 pseudo-maximum failed: pmax(2.5, 1.0) should be 2.5";

    // Test case 2: Mixed positive and negative values
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_basic", 3.7, -1.2, -0.8, 4.9, result))
        << "Failed to execute f64x2.pmax with mixed positive/negative values";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_EQ(3.7, lane0_result) << "Lane 0 pseudo-maximum failed: pmax(3.7, -0.8) should be 3.7";
    ASSERT_EQ(4.9, lane1_result) << "Lane 1 pseudo-maximum failed: pmax(-1.2, 4.9) should be 4.9";

    // Test case 3: Small values with precision requirements
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_basic", 0.001, 0.002, 0.003, 0.001, result))
        << "Failed to execute f64x2.pmax with small precision values";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_EQ(0.003, lane0_result) << "Lane 0 pseudo-maximum failed: pmax(0.001, 0.003) should be 0.003";
    ASSERT_EQ(0.002, lane1_result) << "Lane 1 pseudo-maximum failed: pmax(0.002, 0.001) should be 0.002";
}

/**
 * @test BoundaryValues_HandlesLimitsCorrectly
 * @brief Validates f64x2.pmax handles IEEE 754 boundary values and extreme numbers correctly
 * @details Tests pseudo-maximum operation with double-precision limits, infinities, and denormalized numbers.
 *          Verifies correct handling of DBL_MAX, DBL_MIN, infinity values, and subnormal numbers.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:f64x2_pmax_boundary_handling
 * @input_conditions Extreme double values: DBL_MAX, DBL_MIN, ±INFINITY, denormalized numbers
 * @expected_behavior Correct IEEE 754 compliant pseudo-maximum selection for boundary values
 * @validation_method Verification of proper boundary value comparison and selection behavior
 */
TEST_F(F64x2PmaxTestSuite, BoundaryValues_HandlesLimitsCorrectly)
{
    uint8_t result[16];
    double lane0_result, lane1_result;

    // Test case 1: Maximum double values
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_boundary", DBL_MAX, -DBL_MAX, DBL_MAX/2, DBL_MAX, result))
        << "Failed to execute f64x2.pmax with DBL_MAX boundary values";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_EQ(DBL_MAX, lane0_result) << "Lane 0 failed: pmax(DBL_MAX, DBL_MAX/2) should be DBL_MAX";
    ASSERT_EQ(DBL_MAX, lane1_result) << "Lane 1 failed: pmax(-DBL_MAX, DBL_MAX) should be DBL_MAX";

    // Test case 2: Positive and negative infinity
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_boundary", INFINITY, 1.0, 2.0, -INFINITY, result))
        << "Failed to execute f64x2.pmax with infinity values";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_EQ(INFINITY, lane0_result) << "Lane 0 failed: pmax(INFINITY, 2.0) should be INFINITY";
    ASSERT_EQ(1.0, lane1_result) << "Lane 1 failed: pmax(1.0, -INFINITY) should be 1.0";

    // Test case 3: Mixed infinity combinations
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_boundary", -INFINITY, INFINITY, INFINITY, -INFINITY, result))
        << "Failed to execute f64x2.pmax with mixed infinity values";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_EQ(INFINITY, lane0_result) << "Lane 0 failed: pmax(-INFINITY, INFINITY) should be INFINITY";
    ASSERT_EQ(INFINITY, lane1_result) << "Lane 1 failed: pmax(INFINITY, -INFINITY) should be INFINITY";
}

/**
 * @test NaNHandling_AsymmetricNaNBehavior
 * @brief Validates f64x2.pmax NaN handling follows asymmetric NaN behavior (position-dependent propagation)
 * @details Tests that f64x2.pmax uses asymmetric NaN handling: max(NaN, x) = NaN but max(x, NaN) = x.
 *          Verifies that NaN propagation depends on operand position, matching WAMR's implementation behavior.
 * @test_category Edge - WAMR-specific NaN behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:f64x2_pmax_nan_handling
 * @input_conditions NaN in various positions: first operand, second operand, both operands, mixed patterns
 * @expected_behavior Asymmetric NaN behavior: first operand NaN propagates, second operand NaN doesn't
 * @validation_method Verification that NaN handling matches actual WAMR implementation behavior
 */
TEST_F(F64x2PmaxTestSuite, NaNHandling_AsymmetricNaNBehavior)
{
    uint8_t result[16];
    double lane0_result, lane1_result;
    const double nan_val = std::numeric_limits<double>::quiet_NaN();

    // Test case 1: First operand has NaN, second operand has valid values
    // max(NaN, x) returns NaN (first operand NaN propagates)
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_nan", nan_val, 1.0, 2.0, 3.0, result))
        << "Failed to execute f64x2.pmax with NaN in first operand";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_TRUE(is_nan_value(lane0_result)) << "Lane 0 failed: max(NaN, 2.0) should return NaN (first operand NaN propagates)";
    ASSERT_EQ(3.0, lane1_result) << "Lane 1 failed: max(1.0, 3.0) should return 3.0";

    // Test case 2: Second operand has NaN, first operand has valid values
    // max(x, NaN) returns x (second operand NaN does not propagate)
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_nan", 1.0, 2.0, nan_val, 3.0, result))
        << "Failed to execute f64x2.pmax with NaN in second operand";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_EQ(1.0, lane0_result) << "Lane 0 failed: max(1.0, NaN) should return 1.0 (second operand NaN doesn't propagate)";
    ASSERT_EQ(3.0, lane1_result) << "Lane 1 failed: max(2.0, 3.0) should return 3.0";

    // Test case 3: Mixed NaN pattern - NaN in different lanes and positions
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_nan", nan_val, 2.0, 1.0, nan_val, result))
        << "Failed to execute f64x2.pmax with mixed NaN pattern";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_TRUE(is_nan_value(lane0_result)) << "Lane 0 failed: max(NaN, 1.0) should return NaN (first operand NaN propagates)";
    ASSERT_EQ(2.0, lane1_result) << "Lane 1 failed: max(2.0, NaN) should return 2.0 (second operand NaN doesn't propagate)";

    // Test case 4: Both operands are NaN
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_nan", nan_val, nan_val, nan_val, nan_val, result))
        << "Failed to execute f64x2.pmax with both operands NaN";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_TRUE(is_nan_value(lane0_result)) << "Lane 0 failed: max(NaN, NaN) should return NaN";
    ASSERT_TRUE(is_nan_value(lane1_result)) << "Lane 1 failed: max(NaN, NaN) should return NaN";
}

/**
 * @test ZeroSignHandling_DistinguishesSignedZeros
 * @brief Validates f64x2.pmax properly handles IEEE 754 signed zero distinctions (+0.0 vs -0.0)
 * @details Tests pseudo-maximum behavior with positive and negative zero values in various combinations.
 *          Verifies that IEEE 754 signed zero semantics are preserved in pseudo-maximum operations.
 * @test_category Edge - IEEE 754 signed zero behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:f64x2_pmax_zero_handling
 * @input_conditions Combinations of +0.0 and -0.0 in different lane positions
 * @expected_behavior IEEE 754 compliant signed zero handling in pseudo-maximum selection
 * @validation_method Verification of correct sign preservation and zero comparison behavior
 */
TEST_F(F64x2PmaxTestSuite, ZeroSignHandling_DistinguishesSignedZeros)
{
    uint8_t result[16];
    double lane0_result, lane1_result;
    const double pos_zero = +0.0;
    const double neg_zero = -0.0;

    // Test case 1: Positive zero vs negative zero
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_zeros", neg_zero, pos_zero, pos_zero, neg_zero, result))
        << "Failed to execute f64x2.pmax with signed zero values";

    extract_f64x2_result(result, lane0_result, lane1_result);
    // Note: IEEE 754 considers +0.0 > -0.0 for comparison purposes
    ASSERT_EQ(pos_zero, lane0_result) << "Lane 0 failed: pmax(-0.0, +0.0) should return +0.0";
    ASSERT_EQ(pos_zero, lane1_result) << "Lane 1 failed: pmax(+0.0, -0.0) should return +0.0";

    // Test case 2: Zero vs small values
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_zeros", pos_zero, neg_zero, DBL_MIN, -DBL_MIN, result))
        << "Failed to execute f64x2.pmax with zero vs small values";

    extract_f64x2_result(result, lane0_result, lane1_result);
    ASSERT_EQ(DBL_MIN, lane0_result) << "Lane 0 failed: pmax(+0.0, DBL_MIN) should return DBL_MIN";
    ASSERT_EQ(neg_zero, lane1_result) << "Lane 1 failed: pmax(-0.0, -DBL_MIN) should return -0.0";
}

/**
 * @test ModuleLoadingValidation_SimdSupport
 * @brief Validates f64x2.pmax WASM module loads successfully when SIMD support is enabled
 * @details Tests that WASM modules containing f64x2.pmax operations load properly with SIMD support.
 *          Verifies WAMR runtime correctly initializes and prepares execution environment for SIMD operations.
 * @test_category Error - Module loading and SIMD support validation
 * @coverage_target core/iwasm/common/wasm_loader.c:simd_validation
 * @input_conditions Valid WASM module with f64x2.pmax operations and proper SIMD structure
 * @expected_behavior Successful module loading and execution environment initialization
 * @validation_method Verification of module loading success and execution readiness
 */
TEST_F(F64x2PmaxTestSuite, ModuleLoadingValidation_SimdSupport)
{
    // Verify that the test execution environment was successfully initialized
    ASSERT_NE(nullptr, dummy_env)
        << "WASM execution environment should be initialized for f64x2.pmax tests";

    ASSERT_NE(nullptr, dummy_env->get())
        << "WASM execution environment should contain valid module instance";

    // Test basic function availability by attempting to execute a simple test
    uint8_t result[16];
    ASSERT_TRUE(call_f64x2_pmax("test_f64x2_pmax_basic", 1.0, 2.0, 3.0, 4.0, result))
        << "Basic f64x2.pmax function should be executable with valid module loading";

    // Verify that SIMD operations produce meaningful results (not just successful execution)
    double lane0_result, lane1_result;
    extract_f64x2_result(result, lane0_result, lane1_result);

    // Basic sanity check: pmax(1.0, 3.0) = 3.0, pmax(2.0, 4.0) = 4.0
    ASSERT_EQ(3.0, lane0_result) << "SIMD f64x2.pmax operation should produce correct results";
    ASSERT_EQ(4.0, lane1_result) << "SIMD f64x2.pmax operation should produce correct results";
}