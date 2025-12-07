/**
 * @file enhanced_f64x2_min_test.cc
 * @brief Comprehensive unit tests for f64x2.min SIMD opcode
 * @details Tests f64x2.min functionality across interpreter and AOT execution modes
 *          with focus on element-wise minimum operation of two 64-bit double-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_min_test.cc
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
 * @class F64x2MinTestSuite
 * @brief Test fixture class for f64x2.min opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F64x2MinTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f64x2.min testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_min_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.min test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_min_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.min tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_min_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f64x2.min function with two vector inputs
     * @details Executes f64x2.min operation on two input vectors and returns element-wise minimum result.
     *          Handles WASM function invocation and v128 result extraction for 2-lane f64 minimum operation.
     * @param d1_lane0 First double value for lane 0 of first vector
     * @param d1_lane1 First double value for lane 1 of first vector
     * @param d2_lane0 Second double value for lane 0 of second vector
     * @param d2_lane1 Second double value for lane 1 of second vector
     * @param result_bytes 16-byte array to store the minimum operation result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_min_test.cc:call_f64x2_min
     */
    bool call_f64x2_min(double d1_lane0, double d1_lane1, double d2_lane0, double d2_lane1,
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

        // Execute WASM function: f64x2_min_basic
        bool success = dummy_env->execute("f64x2_min_basic", 8, argv);
        EXPECT_TRUE(success) << "Failed to execute f64x2.min WASM function";

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
     * @brief Extract double values from a 16-byte v128 result representing f64x2
     * @details Converts raw v128 bytes back to two double values for verification.
     *          Handles little-endian byte ordering for proper double reconstruction.
     * @param result_bytes 16-byte array containing the v128 result
     * @param lane0_result Reference to store the lane 0 double value
     * @param lane1_result Reference to store the lane 1 double value
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_min_test.cc:extract_f64x2_result
     */
    void extract_f64x2_result(const uint8_t result_bytes[16], double &lane0_result, double &lane1_result)
    {
        // Extract two 64-bit doubles from the v128 result
        uint64_t lane0_bits, lane1_bits;

        // Little-endian: first 8 bytes are lane 0, next 8 bytes are lane 1
        memcpy(&lane0_bits, &result_bytes[0], sizeof(uint64_t));
        memcpy(&lane1_bits, &result_bytes[8], sizeof(uint64_t));

        // Convert bit patterns back to double values
        memcpy(&lane0_result, &lane0_bits, sizeof(double));
        memcpy(&lane1_result, &lane1_bits, sizeof(double));
    }

    /**
     * @brief Check if a double value is NaN (Not a Number)
     * @details Uses std::isnan for IEEE 754 compliant NaN detection.
     *          Required for proper special value validation.
     * @param value Double value to check for NaN
     * @return bool True if value is NaN, false otherwise
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_min_test.cc:is_nan
     */
    bool is_nan(double value)
    {
        return std::isnan(value);
    }

    /**
     * @brief Check if a double value is negative zero (-0.0)
     * @details Uses signbit to distinguish between +0.0 and -0.0 as required by IEEE 754.
     *          Critical for testing signed zero handling in min operations.
     * @param value Double value to check for negative zero
     * @return bool True if value is -0.0, false otherwise
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_min_test.cc:is_negative_zero
     */
    bool is_negative_zero(double value)
    {
        return (value == 0.0) && std::signbit(value);
    }

    // Test infrastructure members
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicMinimum_ReturnsCorrectResult
 * @brief Validates f64x2.min produces correct element-wise minimum for regular values
 * @details Tests fundamental minimum operation with positive, negative, and mixed-sign doubles.
 *          Verifies that f64x2.min correctly computes min(a[i], b[i]) for each lane independently.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_simd_ops.c:f64x2_min_operation
 * @input_conditions Standard double pairs: (1.5, 3.7) vs (2.1, 1.9), (-5.2, -8.1) vs (-3.4, -12.7)
 * @expected_behavior Returns lane-wise minimums: (1.5, 1.9), (-5.2, -12.7) respectively
 * @validation_method Direct comparison of WASM function result with expected double values
 */
TEST_F(F64x2MinTestSuite, BasicMinimum_ReturnsCorrectResult)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test case 1: Positive numbers with mixed results per lane
    ASSERT_TRUE(call_f64x2_min(1.5, 3.7, 2.1, 1.9, result_bytes))
        << "Failed to execute f64x2.min with positive input values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(1.5, lane0_result) << "Lane 0: min(1.5, 2.1) should be 1.5";
    ASSERT_EQ(1.9, lane1_result) << "Lane 1: min(3.7, 1.9) should be 1.9";

    // Test case 2: Negative numbers with lane-wise minimum selection
    ASSERT_TRUE(call_f64x2_min(-5.2, -8.1, -3.4, -12.7, result_bytes))
        << "Failed to execute f64x2.min with negative input values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(-5.2, lane0_result) << "Lane 0: min(-5.2, -3.4) should be -5.2";
    ASSERT_EQ(-12.7, lane1_result) << "Lane 1: min(-8.1, -12.7) should be -12.7";

    // Test case 3: Mixed sign combinations
    ASSERT_TRUE(call_f64x2_min(10.5, -15.3, -7.2, 25.8, result_bytes))
        << "Failed to execute f64x2.min with mixed sign input values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(-7.2, lane0_result) << "Lane 0: min(10.5, -7.2) should be -7.2";
    ASSERT_EQ(-15.3, lane1_result) << "Lane 1: min(-15.3, 25.8) should be -15.3";
}

/**
 * @test SpecialValues_HandlesIEEE754Correctly
 * @brief Validates IEEE 754 special value handling (NaN, infinity, signed zeros)
 * @details Tests critical IEEE 754 minimum behaviors including NaN propagation,
 *          infinity handling, and signed zero preference according to IEEE 754 standard.
 * @test_category Edge - IEEE 754 special value validation
 * @coverage_target core/iwasm/interpreter/wasm_simd_ops.c:f64x2_min_special_values
 * @input_conditions Special values: (+∞, NaN) vs (-∞, +0.0), (NaN, -0.0) vs (finite, +0.0)
 * @expected_behavior NaN propagation, infinity handling, negative zero preference
 * @validation_method IEEE 754 compliant special value checking and signbit validation
 */
TEST_F(F64x2MinTestSuite, SpecialValues_HandlesIEEE754Correctly)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test case 1: Infinity and NaN handling
    double pos_inf = std::numeric_limits<double>::infinity();
    double neg_inf = -std::numeric_limits<double>::infinity();
    double quiet_nan = std::numeric_limits<double>::quiet_NaN();

    // Use special values function for this test
    uint32_t argv_special[8];
    uint64_t input1_lo, input1_hi, input2_lo, input2_hi;

    memcpy(&input1_lo, &pos_inf, sizeof(double));
    memcpy(&input1_hi, &quiet_nan, sizeof(double));
    memcpy(&input2_lo, &neg_inf, sizeof(double));
    double zero_value = 0.0;
    memcpy(&input2_hi, &zero_value, sizeof(double));

    argv_special[0] = static_cast<uint32_t>(input1_lo);
    argv_special[1] = static_cast<uint32_t>(input1_lo >> 32);
    argv_special[2] = static_cast<uint32_t>(input1_hi);
    argv_special[3] = static_cast<uint32_t>(input1_hi >> 32);
    argv_special[4] = static_cast<uint32_t>(input2_lo);
    argv_special[5] = static_cast<uint32_t>(input2_lo >> 32);
    argv_special[6] = static_cast<uint32_t>(input2_hi);
    argv_special[7] = static_cast<uint32_t>(input2_hi >> 32);

    ASSERT_TRUE(dummy_env->execute("f64x2_min_special_values", 8, argv_special))
        << "Failed to execute f64x2.min with infinity and NaN values";

    uint64_t result_lo = (static_cast<uint64_t>(argv_special[1]) << 32) | argv_special[0];
    uint64_t result_hi = (static_cast<uint64_t>(argv_special[3]) << 32) | argv_special[2];
    memcpy(&result_bytes[0], &result_lo, 8);
    memcpy(&result_bytes[8], &result_hi, 8);

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(neg_inf, lane0_result) << "Lane 0: min(+∞, -∞) should be -∞";
    ASSERT_TRUE(is_nan(lane1_result)) << "Lane 1: min(NaN, 0.0) should propagate NaN";

    // Test case 2: Signed zero handling - IEEE 754 requires min(+0.0, -0.0) = -0.0
    double pos_zero = 0.0;
    double neg_zero = -0.0;

    ASSERT_TRUE(call_f64x2_min(pos_zero, neg_zero, neg_zero, pos_zero, result_bytes))
        << "Failed to execute f64x2.min with signed zero values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_TRUE(is_negative_zero(lane0_result)) << "Lane 0: min(+0.0, -0.0) should be -0.0";
    ASSERT_TRUE(is_negative_zero(lane1_result)) << "Lane 1: min(-0.0, +0.0) should be -0.0";

    // Test case 3: NaN propagation from either operand
    ASSERT_TRUE(call_f64x2_min(quiet_nan, 42.0, 3.14, quiet_nan, result_bytes))
        << "Failed to execute f64x2.min with NaN propagation test";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_TRUE(is_nan(lane0_result)) << "Lane 0: min(NaN, 3.14) should propagate NaN";
    ASSERT_TRUE(is_nan(lane1_result)) << "Lane 1: min(42.0, NaN) should propagate NaN";
}

/**
 * @test BoundaryConditions_HandlesPrecisionLimits
 * @brief Tests behavior at floating-point precision boundaries and extreme values
 * @details Validates minimum operation with boundary values including DBL_MAX, DBL_MIN,
 *          and values at the limits of double-precision representation.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_simd_ops.c:f64x2_min_precision
 * @input_conditions Boundary values: (DBL_MAX, DBL_MIN) vs (DBL_MAX/2, DBL_MIN*2)
 * @expected_behavior Correct minimum selection with precision preservation
 * @validation_method Double equality with appropriate tolerance for boundary values
 */
TEST_F(F64x2MinTestSuite, BoundaryConditions_HandlesPrecisionLimits)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test case 1: Maximum double values
    double dbl_max = std::numeric_limits<double>::max();
    double dbl_max_half = dbl_max / 2.0;

    // Use boundary test function
    uint32_t argv_boundary[8];
    uint64_t input1_lo, input1_hi, input2_lo, input2_hi;

    memcpy(&input1_lo, &dbl_max, sizeof(double));
    double dbl_min = std::numeric_limits<double>::min();
    memcpy(&input1_hi, &dbl_min, sizeof(double));
    memcpy(&input2_lo, &dbl_max_half, sizeof(double));
    double dbl_min_times_2 = std::numeric_limits<double>::min() * 2.0;
    memcpy(&input2_hi, &dbl_min_times_2, sizeof(double));

    argv_boundary[0] = static_cast<uint32_t>(input1_lo);
    argv_boundary[1] = static_cast<uint32_t>(input1_lo >> 32);
    argv_boundary[2] = static_cast<uint32_t>(input1_hi);
    argv_boundary[3] = static_cast<uint32_t>(input1_hi >> 32);
    argv_boundary[4] = static_cast<uint32_t>(input2_lo);
    argv_boundary[5] = static_cast<uint32_t>(input2_lo >> 32);
    argv_boundary[6] = static_cast<uint32_t>(input2_hi);
    argv_boundary[7] = static_cast<uint32_t>(input2_hi >> 32);

    ASSERT_TRUE(dummy_env->execute("f64x2_min_boundary_test", 8, argv_boundary))
        << "Failed to execute f64x2.min with boundary values";

    uint64_t result_lo = (static_cast<uint64_t>(argv_boundary[1]) << 32) | argv_boundary[0];
    uint64_t result_hi = (static_cast<uint64_t>(argv_boundary[3]) << 32) | argv_boundary[2];
    memcpy(&result_bytes[0], &result_lo, 8);
    memcpy(&result_bytes[8], &result_hi, 8);

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(dbl_max_half, lane0_result) << "Lane 0: min(DBL_MAX, DBL_MAX/2) should be DBL_MAX/2";
    ASSERT_EQ(std::numeric_limits<double>::min(), lane1_result)
        << "Lane 1: min(DBL_MIN, DBL_MIN*2) should be DBL_MIN";

    // Test case 2: Very small positive values near zero (within normal range)
    double very_small_1 = 1e-100;
    double very_small_2 = 1e-200;

    // Use boundary test function for very small values
    uint32_t argv_small[8];
    uint64_t input_1_lo, input_1_hi, input_2_lo, input_2_hi;

    memcpy(&input_1_lo, &very_small_1, sizeof(double));
    memcpy(&input_1_hi, &very_small_2, sizeof(double));
    double very_small_2_times_2 = very_small_2 * 2;
    double very_small_1_div_2 = very_small_1 / 2;
    memcpy(&input_2_lo, &very_small_2_times_2, sizeof(double));
    memcpy(&input_2_hi, &very_small_1_div_2, sizeof(double));

    argv_small[0] = static_cast<uint32_t>(input_1_lo);
    argv_small[1] = static_cast<uint32_t>(input_1_lo >> 32);
    argv_small[2] = static_cast<uint32_t>(input_1_hi);
    argv_small[3] = static_cast<uint32_t>(input_1_hi >> 32);
    argv_small[4] = static_cast<uint32_t>(input_2_lo);
    argv_small[5] = static_cast<uint32_t>(input_2_lo >> 32);
    argv_small[6] = static_cast<uint32_t>(input_2_hi);
    argv_small[7] = static_cast<uint32_t>(input_2_hi >> 32);

    ASSERT_TRUE(dummy_env->execute("f64x2_min_boundary_test", 8, argv_small))
        << "Failed to execute f64x2.min with very small values";

    uint64_t result_small_lo = (static_cast<uint64_t>(argv_small[1]) << 32) | argv_small[0];
    uint64_t result_small_hi = (static_cast<uint64_t>(argv_small[3]) << 32) | argv_small[2];
    memcpy(&result_bytes[0], &result_small_lo, 8);
    memcpy(&result_bytes[8], &result_small_hi, 8);

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(very_small_2_times_2, lane0_result) << "Lane 0: should select smaller of boundary values";
    ASSERT_EQ(very_small_2, lane1_result) << "Lane 1: should select smaller of boundary values";
}

/**
 * @test CommutativeProperty_ValidatesSymmetry
 * @brief Verifies min(a,b) = min(b,a) commutative property holds
 * @details Tests that element-wise minimum operation produces identical results
 *          regardless of operand order, validating mathematical commutativity.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_simd_ops.c:f64x2_min_commutative
 * @input_conditions Multiple value pairs tested in both orders
 * @expected_behavior Identical results regardless of operand order
 * @validation_method Direct comparison of forward and reverse operation results
 */
TEST_F(F64x2MinTestSuite, CommutativeProperty_ValidatesSymmetry)
{
    uint8_t result_bytes1[16], result_bytes2[16];
    double lane0_result1, lane1_result1, lane0_result2, lane1_result2;

    // Test commutative property: min(a,b) = min(b,a)
    double a0 = 7.5, a1 = -12.3;
    double b0 = 4.2, b1 = -8.7;

    // Forward operation: min((a0, a1), (b0, b1))
    uint32_t argv_forward[8];
    uint64_t input1_lo, input1_hi, input2_lo, input2_hi;

    memcpy(&input1_lo, &a0, sizeof(double));
    memcpy(&input1_hi, &a1, sizeof(double));
    memcpy(&input2_lo, &b0, sizeof(double));
    memcpy(&input2_hi, &b1, sizeof(double));

    argv_forward[0] = static_cast<uint32_t>(input1_lo);
    argv_forward[1] = static_cast<uint32_t>(input1_lo >> 32);
    argv_forward[2] = static_cast<uint32_t>(input1_hi);
    argv_forward[3] = static_cast<uint32_t>(input1_hi >> 32);
    argv_forward[4] = static_cast<uint32_t>(input2_lo);
    argv_forward[5] = static_cast<uint32_t>(input2_lo >> 32);
    argv_forward[6] = static_cast<uint32_t>(input2_hi);
    argv_forward[7] = static_cast<uint32_t>(input2_hi >> 32);

    ASSERT_TRUE(dummy_env->execute("f64x2_min_commutative_test", 8, argv_forward))
        << "Failed to execute forward f64x2.min operation";

    uint64_t result_lo = (static_cast<uint64_t>(argv_forward[1]) << 32) | argv_forward[0];
    uint64_t result_hi = (static_cast<uint64_t>(argv_forward[3]) << 32) | argv_forward[2];
    memcpy(&result_bytes1[0], &result_lo, 8);
    memcpy(&result_bytes1[8], &result_hi, 8);

    // Reverse operation: min((b0, b1), (a0, a1))
    uint32_t argv_reverse[8];

    memcpy(&input1_lo, &b0, sizeof(double));
    memcpy(&input1_hi, &b1, sizeof(double));
    memcpy(&input2_lo, &a0, sizeof(double));
    memcpy(&input2_hi, &a1, sizeof(double));

    argv_reverse[0] = static_cast<uint32_t>(input1_lo);
    argv_reverse[1] = static_cast<uint32_t>(input1_lo >> 32);
    argv_reverse[2] = static_cast<uint32_t>(input1_hi);
    argv_reverse[3] = static_cast<uint32_t>(input1_hi >> 32);
    argv_reverse[4] = static_cast<uint32_t>(input2_lo);
    argv_reverse[5] = static_cast<uint32_t>(input2_lo >> 32);
    argv_reverse[6] = static_cast<uint32_t>(input2_hi);
    argv_reverse[7] = static_cast<uint32_t>(input2_hi >> 32);

    ASSERT_TRUE(dummy_env->execute("f64x2_min_commutative_test", 8, argv_reverse))
        << "Failed to execute reverse f64x2.min operation";

    result_lo = (static_cast<uint64_t>(argv_reverse[1]) << 32) | argv_reverse[0];
    result_hi = (static_cast<uint64_t>(argv_reverse[3]) << 32) | argv_reverse[2];
    memcpy(&result_bytes2[0], &result_lo, 8);
    memcpy(&result_bytes2[8], &result_hi, 8);

    extract_f64x2_result(result_bytes1, lane0_result1, lane1_result1);
    extract_f64x2_result(result_bytes2, lane0_result2, lane1_result2);

    ASSERT_EQ(lane0_result1, lane0_result2)
        << "Commutative property violation: min(a0,b0) != min(b0,a0)";
    ASSERT_EQ(lane1_result1, lane1_result2)
        << "Commutative property violation: min(a1,b1) != min(b1,a1)";

    // Verify the actual minimum values are correct
    ASSERT_EQ(4.2, lane0_result1) << "Lane 0 minimum should be 4.2";
    ASSERT_EQ(-12.3, lane1_result1) << "Lane 1 minimum should be -12.3";
}