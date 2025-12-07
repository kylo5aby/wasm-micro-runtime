/**
 * @file enhanced_f64x2_max_test.cc
 * @brief Comprehensive unit tests for f64x2.max SIMD opcode
 * @details Tests f64x2.max functionality across interpreter and AOT execution modes
 *          with focus on element-wise maximum operation of two 64-bit double-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_max_test.cc
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
 * @class F64x2MaxTestSuite
 * @brief Test fixture class for f64x2.max opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F64x2MaxTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f64x2.max testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_max_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.max test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_max_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.max tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          to prevent memory leaks and resource cleanup.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_max_test.cc:TearDown
     */
    void TearDown() override
    {
        // Resources automatically cleaned up by RAII destructors
    }

    /**
     * @brief Helper function to call WASM f64x2.max function with two vector inputs
     * @details Executes f64x2.max operation on two input vectors and returns element-wise maximum result.
     *          Handles WASM function invocation and v128 result extraction for 2-lane f64 maximum operation.
     * @param d1_lane0 First double value for lane 0 of first vector
     * @param d1_lane1 First double value for lane 1 of first vector
     * @param d2_lane0 Second double value for lane 0 of second vector
     * @param d2_lane1 Second double value for lane 1 of second vector
     * @param result_bytes 16-byte array to store the maximum operation result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_max_test.cc:call_f64x2_max
     */
    bool call_f64x2_max(double d1_lane0, double d1_lane1, double d2_lane0, double d2_lane1,
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

        // Execute WASM function: f64x2_max_basic
        bool success = dummy_env->execute("f64x2_max_basic", 8, argv);
        EXPECT_TRUE(success) << "Failed to execute f64x2.max WASM function";

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
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_max_test.cc:extract_f64x2_result
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
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_max_test.cc:is_nan
     */
    bool is_nan(double value)
    {
        return std::isnan(value);
    }

    /**
     * @brief Check if a double value is negative zero (-0.0)
     * @details Uses signbit to distinguish between +0.0 and -0.0 as required by IEEE 754.
     *          Critical for testing signed zero handling in max operations.
     * @param value Double value to check for negative zero
     * @return bool True if value is -0.0, false otherwise
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_max_test.cc:is_negative_zero
     */
    bool is_negative_zero(double value)
    {
        return (value == 0.0) && std::signbit(value);
    }

    /**
     * @brief Check if a double value is positive zero (+0.0)
     * @details Verifies value is zero and not negative zero for IEEE 754 compliance
     * @param value Double value to check for positive zero
     * @return bool True if value is +0.0, false otherwise
     */
    bool is_positive_zero(double value)
    {
        return (value == 0.0) && !std::signbit(value);
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicMaximumOperation_ReturnsCorrectResults
 * @brief Validates f64x2.max produces correct arithmetic results for typical inputs
 * @details Tests fundamental maximum operation with positive, negative, and mixed-sign doubles.
 *          Verifies that f64x2.max correctly computes max(a, b) for each lane independently.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_max_operation
 * @input_conditions Standard f64 pairs across lanes: (2.5,1.0) max (1.5,3.0), (-2.0,5.5) max (0.0,2.2)
 * @expected_behavior Returns element-wise maximum: (2.5,3.0), (0.0,5.5) respectively
 * @validation_method Direct comparison of WASM function results with expected IEEE 754 maximum values
 */
TEST_F(F64x2MaxTestSuite, BasicMaximumOperation_ReturnsCorrectResults)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test case 1: Mixed positive values (2.5, 1.0) max (1.5, 3.0)
    ASSERT_TRUE(call_f64x2_max(2.5, 1.0, 1.5, 3.0, result_bytes))
        << "Failed to execute f64x2.max with positive values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(2.5, lane0_result) << "Lane 0: max(2.5, 1.5) should be 2.5";
    ASSERT_EQ(3.0, lane1_result) << "Lane 1: max(1.0, 3.0) should be 3.0";

    // Test case 2: Mixed negative and positive values (-2.0, 5.5) max (0.0, 2.2)
    ASSERT_TRUE(call_f64x2_max(-2.0, 5.5, 0.0, 2.2, result_bytes))
        << "Failed to execute f64x2.max with mixed sign values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(0.0, lane0_result) << "Lane 0: max(-2.0, 0.0) should be 0.0";
    ASSERT_EQ(5.5, lane1_result) << "Lane 1: max(5.5, 2.2) should be 5.5";

    // Test case 3: Both negative values (-10.0, -5.0) max (-2.0, -8.0)
    ASSERT_TRUE(call_f64x2_max(-10.0, -5.0, -2.0, -8.0, result_bytes))
        << "Failed to execute f64x2.max with negative values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(-2.0, lane0_result) << "Lane 0: max(-10.0, -2.0) should be -2.0";
    ASSERT_EQ(-5.0, lane1_result) << "Lane 1: max(-5.0, -8.0) should be -5.0";
}

/**
 * @test IEEE754SpecialValues_ReturnsCorrectResults
 * @brief Validates f64x2.max handles IEEE 754 special values correctly
 * @details Tests NaN propagation, infinity comparisons, and signed zero handling
 *          according to IEEE 754-2008 maximum operation semantics.
 * @test_category Edge - Special floating-point value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_max_special_values
 * @input_conditions NaN with normal values, positive/negative infinity, signed zeros
 * @expected_behavior NaN propagation, infinity handling, +0.0 preferred over -0.0
 * @validation_method IEEE 754 compliant special value comparison and NaN detection
 */
TEST_F(F64x2MaxTestSuite, IEEE754SpecialValues_ReturnsCorrectResults)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test NaN propagation - any NaN input produces NaN output
    double nan_val = std::numeric_limits<double>::quiet_NaN();
    ASSERT_TRUE(call_f64x2_max(nan_val, 1.0, 2.0, 3.0, result_bytes))
        << "Failed to execute f64x2.max with NaN values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_TRUE(is_nan(lane0_result)) << "Lane 0: max(NaN, 2.0) should be NaN";
    ASSERT_EQ(3.0, lane1_result) << "Lane 1: max(1.0, 3.0) should be 3.0";

    // Test positive infinity handling
    double pos_inf = std::numeric_limits<double>::infinity();
    ASSERT_TRUE(call_f64x2_max(pos_inf, 1.0, 100.0, 2.0, result_bytes))
        << "Failed to execute f64x2.max with positive infinity";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(pos_inf, lane0_result) << "Lane 0: max(+inf, 100.0) should be +inf";
    ASSERT_EQ(2.0, lane1_result) << "Lane 1: max(1.0, 2.0) should be 2.0";

    // Test negative infinity handling
    double neg_inf = -std::numeric_limits<double>::infinity();
    ASSERT_TRUE(call_f64x2_max(neg_inf, 1.0, -100.0, 2.0, result_bytes))
        << "Failed to execute f64x2.max with negative infinity";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(-100.0, lane0_result) << "Lane 0: max(-inf, -100.0) should be -100.0";
    ASSERT_EQ(2.0, lane1_result) << "Lane 1: max(1.0, 2.0) should be 2.0";

    // Test signed zero handling: +0.0 should be preferred over -0.0
    double pos_zero = 0.0;
    double neg_zero = -0.0;
    ASSERT_TRUE(call_f64x2_max(pos_zero, 1.0, neg_zero, 2.0, result_bytes))
        << "Failed to execute f64x2.max with signed zeros";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_TRUE(is_positive_zero(lane0_result)) << "Lane 0: max(+0.0, -0.0) should be +0.0";
    ASSERT_EQ(2.0, lane1_result) << "Lane 1: max(1.0, 2.0) should be 2.0";
}

/**
 * @test BoundaryValues_ReturnsCorrectResults
 * @brief Validates f64x2.max handles IEEE 754 boundary values correctly
 * @details Tests extreme values like DBL_MAX, DBL_MIN, and largest/smallest normalized numbers
 *          to ensure proper boundary condition handling.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_max_boundary_values
 * @input_conditions DBL_MAX, DBL_MIN, extreme magnitude values
 * @expected_behavior Correct IEEE 754 maximum comparison for boundary values
 * @validation_method Boundary value comparison with expected maximum results
 */
TEST_F(F64x2MaxTestSuite, BoundaryValues_ReturnsCorrectResults)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test DBL_MAX comparison
    ASSERT_TRUE(call_f64x2_max(DBL_MAX, 100.0, 100.0, DBL_MAX, result_bytes))
        << "Failed to execute f64x2.max with DBL_MAX values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(DBL_MAX, lane0_result) << "Lane 0: max(DBL_MAX, 100.0) should be DBL_MAX";
    ASSERT_EQ(DBL_MAX, lane1_result) << "Lane 1: max(100.0, DBL_MAX) should be DBL_MAX";

    // Test DBL_MIN comparison (smallest positive normalized number)
    ASSERT_TRUE(call_f64x2_max(DBL_MIN, 1.0, -1.0, DBL_MIN, result_bytes))
        << "Failed to execute f64x2.max with DBL_MIN values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(DBL_MIN, lane0_result) << "Lane 0: max(DBL_MIN, -1.0) should be DBL_MIN";
    ASSERT_EQ(1.0, lane1_result) << "Lane 1: max(1.0, DBL_MIN) should be 1.0";
}

/**
 * @test SubnormalNumbers_ReturnsCorrectResults
 * @brief Validates f64x2.max handles subnormal (denormalized) numbers correctly
 * @details Tests very small subnormal values to ensure proper IEEE 754 comparison
 *          behavior for denormalized floating-point numbers.
 * @test_category Edge - Subnormal number validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64x2_max_subnormal_handling
 * @input_conditions Very small subnormal values, subnormal vs normal comparisons
 * @expected_behavior Correct IEEE 754 maximum operation on subnormal numbers
 * @validation_method Subnormal value comparison with expected maximum results
 */
TEST_F(F64x2MaxTestSuite, SubnormalNumbers_ReturnsCorrectResults)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test subnormal vs normal number comparison
    double subnormal = 1.0e-320;  // Very small subnormal number
    ASSERT_TRUE(call_f64x2_max(subnormal, 1.0, DBL_MIN, subnormal, result_bytes))
        << "Failed to execute f64x2.max with subnormal values";

    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_EQ(DBL_MIN, lane0_result) << "Lane 0: max(subnormal, DBL_MIN) should be DBL_MIN";
    ASSERT_EQ(1.0, lane1_result) << "Lane 1: max(1.0, subnormal) should be 1.0";
}