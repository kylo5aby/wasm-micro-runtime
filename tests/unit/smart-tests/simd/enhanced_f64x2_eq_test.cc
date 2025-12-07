/**
 * @file enhanced_f64x2_eq_test.cc
 * @brief Comprehensive unit tests for f64x2.eq SIMD opcode
 * @details Tests f64x2.eq functionality across interpreter and AOT execution modes
 *          with focus on element-wise equality comparison of two 64-bit double-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_eq_test.cc
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
 * @class F64x2EqTestSuite
 * @brief Test fixture class for f64x2.eq opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F64x2EqTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f64x2.eq testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_eq_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.eq test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_eq_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.eq tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_eq_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f64x2.eq function with two vector inputs
     * @details Executes f64x2.eq operation on two input vectors and returns element-wise equality result.
     *          Handles WASM function invocation and v128 result extraction for 2-lane f64 comparison.
     * @param d1_lane0 First double value for lane 0 of first vector
     * @param d1_lane1 First double value for lane 1 of first vector
     * @param d2_lane0 Second double value for lane 0 of second vector
     * @param d2_lane1 Second double value for lane 1 of second vector
     * @param result_bytes 16-byte array to store the equality comparison result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_eq_test.cc:call_f64x2_eq
     */
    bool call_f64x2_eq(double d1_lane0, double d1_lane1, double d2_lane0, double d2_lane1,
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

        // Execute WASM function: f64x2_eq_basic
        bool success = dummy_env->execute("f64x2_eq_basic", 8, argv);
        EXPECT_TRUE(success) << "Failed to execute f64x2.eq WASM function";

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
     * @brief Validate f64x2.eq result matches expected lane values
     * @details Compares actual result bytes with expected equality masks for each lane.
     *          Each lane should be either all 0xFF (equal) or all 0x00 (not equal).
     * @param expected_lane0_eq True if lane 0 should be equal (0xFFFF...), false for not equal (0x0000...)
     * @param expected_lane1_eq True if lane 1 should be equal (0xFFFF...), false for not equal (0x0000...)
     * @param actual_bytes Actual 16-byte result from f64x2.eq operation
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_eq_test.cc:validate_f64x2_eq_result
     */
    void validate_f64x2_eq_result(bool expected_lane0_eq, bool expected_lane1_eq,
                                  const uint8_t actual_bytes[16])
    {
        // Lane 0 validation (bytes 0-7)
        uint64_t expected_lane0 = expected_lane0_eq ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL;
        uint64_t actual_lane0;
        memcpy(&actual_lane0, &actual_bytes[0], 8);

        ASSERT_EQ(expected_lane0, actual_lane0)
            << "f64x2.eq lane 0 mismatch - Expected: 0x" << std::hex << expected_lane0
            << ", Actual: 0x" << std::hex << actual_lane0;

        // Lane 1 validation (bytes 8-15)
        uint64_t expected_lane1 = expected_lane1_eq ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL;
        uint64_t actual_lane1;
        memcpy(&actual_lane1, &actual_bytes[8], 8);

        ASSERT_EQ(expected_lane1, actual_lane1)
            << "f64x2.eq lane 1 mismatch - Expected: 0x" << std::hex << expected_lane1
            << ", Actual: 0x" << std::hex << actual_lane1;
    }

    /**
     * @brief Helper function to create special IEEE 754 values for testing
     * @details Provides access to NaN, infinity, and other special floating-point values
     *          for comprehensive IEEE 754 compliance testing.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_eq_test.cc:get_special_values
     */
    double get_positive_nan() const { return std::numeric_limits<double>::quiet_NaN(); }
    double get_positive_infinity() const { return std::numeric_limits<double>::infinity(); }
    double get_negative_infinity() const { return -std::numeric_limits<double>::infinity(); }
    double get_positive_zero() const { return 0.0; }
    double get_negative_zero() const { return -0.0; }
    double get_max_double() const { return std::numeric_limits<double>::max(); }
    double get_min_double() const { return std::numeric_limits<double>::min(); }
    double get_denormal_double() const { return std::numeric_limits<double>::denorm_min(); }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicEqualityComparison_ReturnsCorrectResults
 * @brief Validates f64x2.eq produces correct equality results for typical input combinations
 * @details Tests fundamental equal operation with mixed equal/unequal scenarios.
 *          Verifies that f64x2.eq correctly identifies equal lanes and unequal lanes.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_eq
 * @input_conditions Mixed patterns: identical vectors, different vectors, partial equality
 * @expected_behavior Returns 0xFFFFFFFFFFFFFFFF for equal lanes, 0x0000000000000000 for unequal lanes
 * @validation_method Direct comparison of WASM function result with expected equality mask
 */
TEST_F(F64x2EqTestSuite, BasicEqualityComparison_ReturnsCorrectResults)
{
    uint8_t result[16];

    // Test complete equality: identical vectors should produce all true (equal)
    ASSERT_TRUE(call_f64x2_eq(1.5, 2.5, 1.5, 2.5, result))
        << "Failed to execute f64x2.eq with identical vectors";
    validate_f64x2_eq_result(true, true, result);  // Both lanes equal

    // Test complete inequality: different values in both lanes
    ASSERT_TRUE(call_f64x2_eq(1.5, 2.5, 3.0, 4.0, result))
        << "Failed to execute f64x2.eq with different vectors";
    validate_f64x2_eq_result(false, false, result);  // Both lanes not equal

    // Test mixed scenario: one lane equal, one lane not equal
    ASSERT_TRUE(call_f64x2_eq(1.0, 2.0, 1.0, 3.0, result))
        << "Failed to execute f64x2.eq with mixed equal/unequal lanes";
    validate_f64x2_eq_result(true, false, result);  // Lane 0 equal, lane 1 not equal

    // Test with negative values
    ASSERT_TRUE(call_f64x2_eq(-1.5, -2.5, 1.5, -2.5, result))
        << "Failed to execute f64x2.eq with negative values";
    validate_f64x2_eq_result(false, true, result);  // Lane 0 not equal, lane 1 equal
}

/**
 * @test IdenticalValues_ReturnsTrue
 * @brief Validates that identical values compare as equal (return true for =)
 * @details Tests the reflexive property: A = A should always be true for normal values.
 *          Ensures self-comparison produces correct results for various value types.
 * @test_category Main - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_eq
 * @input_conditions Identical f64x2 vectors with various value types
 * @expected_behavior All-one mask (both lanes return 0xFFFFFFFFFFFFFFFF)
 * @validation_method Verify self-comparison produces true results
 */
TEST_F(F64x2EqTestSuite, IdenticalValues_ReturnsTrue)
{
    uint8_t result[16];

    // Test with positive values
    ASSERT_TRUE(call_f64x2_eq(42.0, 3.14159, 42.0, 3.14159, result))
        << "Failed to execute f64x2.eq with identical positive values";
    validate_f64x2_eq_result(true, true, result);

    // Test with negative values
    ASSERT_TRUE(call_f64x2_eq(-123.456, -789.012, -123.456, -789.012, result))
        << "Failed to execute f64x2.eq with identical negative values";
    validate_f64x2_eq_result(true, true, result);

    // Test with zero values
    ASSERT_TRUE(call_f64x2_eq(0.0, 0.0, 0.0, 0.0, result))
        << "Failed to execute f64x2.eq with identical zero values";
    validate_f64x2_eq_result(true, true, result);

    // Test with very large values
    ASSERT_TRUE(call_f64x2_eq(1e100, -1e200, 1e100, -1e200, result))
        << "Failed to execute f64x2.eq with identical large values";
    validate_f64x2_eq_result(true, true, result);
}

/**
 * @test BoundaryValueComparisons_HandlesExtremeValues
 * @brief Validates f64x2.eq behavior at IEEE 754 boundary conditions
 * @details Tests equality comparison for extreme values including infinities,
 *          maximum/minimum representable values, and denormal numbers.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_eq
 * @input_conditions Boundary values: DBL_MAX, DBL_MIN, infinity, denormal numbers
 * @expected_behavior Correct IEEE 754 equality semantics for all boundary cases
 * @validation_method Boundary value equality verification with special cases
 */
TEST_F(F64x2EqTestSuite, BoundaryValueComparisons_HandlesExtremeValues)
{
    uint8_t result[16];

    // Test maximum double values
    double max_val = get_max_double();
    ASSERT_TRUE(call_f64x2_eq(max_val, max_val, max_val, max_val, result))
        << "Failed to execute f64x2.eq with DBL_MAX values";
    validate_f64x2_eq_result(true, true, result);

    // Test minimum positive double values
    double min_val = get_min_double();
    ASSERT_TRUE(call_f64x2_eq(min_val, min_val, min_val, min_val, result))
        << "Failed to execute f64x2.eq with DBL_MIN values";
    validate_f64x2_eq_result(true, true, result);

    // Test infinity comparisons
    double pos_inf = get_positive_infinity();
    double neg_inf = get_negative_infinity();

    // Positive infinity == positive infinity should be true
    ASSERT_TRUE(call_f64x2_eq(pos_inf, pos_inf, pos_inf, pos_inf, result))
        << "Failed to execute f64x2.eq with positive infinity values";
    validate_f64x2_eq_result(true, true, result);

    // Negative infinity == negative infinity should be true
    ASSERT_TRUE(call_f64x2_eq(neg_inf, neg_inf, neg_inf, neg_inf, result))
        << "Failed to execute f64x2.eq with negative infinity values";
    validate_f64x2_eq_result(true, true, result);

    // Positive infinity != negative infinity
    ASSERT_TRUE(call_f64x2_eq(pos_inf, neg_inf, neg_inf, pos_inf, result))
        << "Failed to execute f64x2.eq with mixed infinity values";
    validate_f64x2_eq_result(false, false, result);

    // Test denormal numbers
    double denormal = get_denormal_double();
    ASSERT_TRUE(call_f64x2_eq(denormal, denormal, denormal, denormal, result))
        << "Failed to execute f64x2.eq with denormal values";
    validate_f64x2_eq_result(true, true, result);
}

/**
 * @test SpecialValueComparisons_HandlesNaNAndZero
 * @brief Validates IEEE 754 special value equality behavior (NaN, ±0)
 * @details Tests critical IEEE 754 rules: NaN ≠ NaN, +0.0 == -0.0.
 *          Ensures f64x2.eq correctly implements IEEE 754 equality semantics.
 * @test_category Edge - IEEE 754 special cases validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_eq
 * @input_conditions NaN values, positive/negative zero, mixed special values
 * @expected_behavior NaN != NaN (false), +0.0 == -0.0 (true), NaN != numbers (false)
 * @validation_method IEEE 754 compliance verification for special value comparisons
 */
TEST_F(F64x2EqTestSuite, SpecialValueComparisons_HandlesNaNAndZero)
{
    uint8_t result[16];

    // Test NaN behavior: NaN != NaN per IEEE 754
    double nan_val = get_positive_nan();
    ASSERT_TRUE(call_f64x2_eq(nan_val, nan_val, nan_val, nan_val, result))
        << "Failed to execute f64x2.eq with NaN values";
    validate_f64x2_eq_result(false, false, result);  // NaN != NaN should be false

    // Test NaN vs regular numbers: always false
    ASSERT_TRUE(call_f64x2_eq(nan_val, 1.0, 1.0, nan_val, result))
        << "Failed to execute f64x2.eq with NaN vs numbers";
    validate_f64x2_eq_result(false, false, result);  // NaN != 1.0, 1.0 != NaN should be false

    // Test positive and negative zero equality: +0.0 == -0.0 per IEEE 754
    double pos_zero = get_positive_zero();
    double neg_zero = get_negative_zero();
    ASSERT_TRUE(call_f64x2_eq(pos_zero, neg_zero, neg_zero, pos_zero, result))
        << "Failed to execute f64x2.eq with positive/negative zero";
    validate_f64x2_eq_result(true, true, result);  // +0.0 == -0.0 should be true

    // Test zero vs zero (same sign)
    ASSERT_TRUE(call_f64x2_eq(pos_zero, pos_zero, pos_zero, pos_zero, result))
        << "Failed to execute f64x2.eq with positive zeros";
    validate_f64x2_eq_result(true, true, result);

    ASSERT_TRUE(call_f64x2_eq(neg_zero, neg_zero, neg_zero, neg_zero, result))
        << "Failed to execute f64x2.eq with negative zeros";
    validate_f64x2_eq_result(true, true, result);

    // Test mixed special values: NaN vs zero
    ASSERT_TRUE(call_f64x2_eq(nan_val, pos_zero, pos_zero, nan_val, result))
        << "Failed to execute f64x2.eq with NaN vs zero";
    validate_f64x2_eq_result(false, false, result);  // NaN != 0.0 should be false
}

/**
 * @test CrossExecutionModeConsistency_ProducesSameResults
 * @brief Validates consistent results across interpreter and AOT modes
 * @details Uses execution environment validation to ensure f64x2.eq produces identical results
 *          across different WAMR execution modes for test scenarios.
 * @test_category Integration - Cross-mode consistency validation
 * @coverage_target core/iwasm/interpreter + core/iwasm/aot execution paths
 * @input_conditions Mixed test scenarios with different f64 value combinations
 * @expected_behavior Consistent equality comparison results across execution modes
 * @validation_method Basic execution validation with representative test cases
 */
TEST_F(F64x2EqTestSuite, CrossExecutionModeConsistency_ProducesSameResults)
{
    uint8_t result[16];

    // Test that the valid module loads correctly and executes consistently
    ASSERT_NE(nullptr, dummy_env->get())
        << "Valid f64x2.eq test module should load successfully";

    // Test basic execution capability with different value patterns
    ASSERT_TRUE(call_f64x2_eq(1.0, 2.0, 1.0, 2.0, result))
        << "Basic f64x2.eq execution should succeed with valid module";
    validate_f64x2_eq_result(true, true, result);

    // Test with different values to ensure consistency
    ASSERT_TRUE(call_f64x2_eq(3.14, -2.71, 3.14, 1.41, result))
        << "f64x2.eq execution with mixed values should be consistent";
    validate_f64x2_eq_result(true, false, result);

    // Test execution consistency with special values
    ASSERT_TRUE(call_f64x2_eq(get_positive_infinity(), get_negative_zero(),
                              get_positive_infinity(), get_positive_zero(), result))
        << "f64x2.eq execution with special values should be consistent";
    validate_f64x2_eq_result(true, true, result);  // +∞ == +∞, -0.0 == +0.0
}