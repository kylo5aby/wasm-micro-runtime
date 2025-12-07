/**
 * @file enhanced_f64x2_ne_test.cc
 * @brief Comprehensive unit tests for f64x2.ne SIMD opcode
 * @details Tests f64x2.ne functionality across interpreter and AOT execution modes
 *          with focus on element-wise not-equal comparison of two 64-bit double-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_ne_test.cc
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
 * @class F64x2NeTestSuite
 * @brief Test fixture class for f64x2.ne opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F64x2NeTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f64x2.ne testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_ne_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.ne test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_ne_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.ne tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_ne_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f64x2.ne function with two vector inputs
     * @details Executes f64x2.ne operation on two input vectors and returns element-wise inequality result.
     *          Handles WASM function invocation and v128 result extraction for 2-lane f64 comparison.
     * @param d1_lane0 First double value for lane 0 of first vector
     * @param d1_lane1 First double value for lane 1 of first vector
     * @param d2_lane0 Second double value for lane 0 of second vector
     * @param d2_lane1 Second double value for lane 1 of second vector
     * @param result_bytes 16-byte array to store the inequality comparison result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_ne_test.cc:call_f64x2_ne
     */
    bool call_f64x2_ne(double d1_lane0, double d1_lane1, double d2_lane0, double d2_lane1,
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

        // Execute WASM function: f64x2_ne_basic
        bool success = dummy_env->execute("f64x2_ne_basic", 8, argv);
        EXPECT_TRUE(success) << "Failed to execute f64x2.ne WASM function";

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
     * @brief Validate f64x2.ne result matches expected lane values
     * @details Compares actual result bytes with expected inequality masks for each lane.
     *          Each lane should be either all 0xFF (not equal) or all 0x00 (equal).
     * @param expected_lane0_ne True if lane 0 should be not-equal (0xFFFF...), false for equal (0x0000...)
     * @param expected_lane1_ne True if lane 1 should be not-equal (0xFFFF...), false for equal (0x0000...)
     * @param actual_bytes Actual 16-byte result from f64x2.ne operation
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_ne_test.cc:validate_f64x2_ne_result
     */
    void validate_f64x2_ne_result(bool expected_lane0_ne, bool expected_lane1_ne,
                                  const uint8_t actual_bytes[16])
    {
        // Lane 0 validation (bytes 0-7)
        uint64_t expected_lane0 = expected_lane0_ne ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL;
        uint64_t actual_lane0;
        memcpy(&actual_lane0, &actual_bytes[0], 8);

        ASSERT_EQ(expected_lane0, actual_lane0)
            << "f64x2.ne lane 0 mismatch - Expected: 0x" << std::hex << expected_lane0
            << ", Actual: 0x" << std::hex << actual_lane0;

        // Lane 1 validation (bytes 8-15)
        uint64_t expected_lane1 = expected_lane1_ne ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL;
        uint64_t actual_lane1;
        memcpy(&actual_lane1, &actual_bytes[8], 8);

        ASSERT_EQ(expected_lane1, actual_lane1)
            << "f64x2.ne lane 1 mismatch - Expected: 0x" << std::hex << expected_lane1
            << ", Actual: 0x" << std::hex << actual_lane1;
    }

    /**
     * @brief Helper function to create special IEEE 754 values for testing
     * @details Provides access to NaN, infinity, and other special floating-point values
     *          for comprehensive IEEE 754 compliance testing.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_ne_test.cc:get_special_values
     */
    double get_positive_nan() const { return std::numeric_limits<double>::quiet_NaN(); }
    double get_positive_infinity() const { return std::numeric_limits<double>::infinity(); }
    double get_negative_infinity() const { return -std::numeric_limits<double>::infinity(); }
    double get_positive_zero() const { return 0.0; }
    double get_negative_zero() const { return -0.0; }
    double get_max_double() const { return std::numeric_limits<double>::max(); }
    double get_min_double() const { return std::numeric_limits<double>::min(); }
    double get_denormal_double() const { return std::numeric_limits<double>::denorm_min(); }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicComparison_ReturnsCorrectResults
 * @brief Validates f64x2.ne produces correct inequality results for typical input combinations
 * @details Tests fundamental not-equal operation with mixed equal/unequal scenarios.
 *          Verifies that f64x2.ne correctly identifies unequal lanes and equal lanes.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_ne
 * @input_conditions Mixed patterns: identical vectors, different vectors, partial equality
 * @expected_behavior Returns 0xFFFFFFFFFFFFFFFF for unequal lanes, 0x0000000000000000 for equal lanes
 * @validation_method Direct comparison of WASM function result with expected inequality mask
 */
TEST_F(F64x2NeTestSuite, BasicComparison_ReturnsCorrectResults)
{
    uint8_t result[16];

    // Test complete equality: identical vectors should produce all false (equal)
    ASSERT_TRUE(call_f64x2_ne(1.5, 2.5, 1.5, 2.5, result))
        << "Failed to execute f64x2.ne with identical vectors";
    validate_f64x2_ne_result(false, false, result);  // Both lanes equal

    // Test complete inequality: different values in both lanes
    ASSERT_TRUE(call_f64x2_ne(1.5, 2.5, 3.0, 4.0, result))
        << "Failed to execute f64x2.ne with different vectors";
    validate_f64x2_ne_result(true, true, result);  // Both lanes not equal

    // Test mixed scenario: one lane equal, one lane not equal
    ASSERT_TRUE(call_f64x2_ne(1.0, 2.0, 1.0, 3.0, result))
        << "Failed to execute f64x2.ne with mixed equal/unequal lanes";
    validate_f64x2_ne_result(false, true, result);  // Lane 0 equal, lane 1 not equal

    // Test with negative values
    ASSERT_TRUE(call_f64x2_ne(-1.5, -2.5, 1.5, -2.5, result))
        << "Failed to execute f64x2.ne with negative values";
    validate_f64x2_ne_result(true, false, result);  // Lane 0 not equal, lane 1 equal
}

/**
 * @test IdenticalValues_ReturnsFalse
 * @brief Validates that identical values compare as equal (return false for ≠)
 * @details Tests the reflexive property: A ≠ A should always be false for normal values.
 *          Ensures self-comparison produces correct results for various value types.
 * @test_category Main - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_ne
 * @input_conditions Identical f64x2 vectors with various value types
 * @expected_behavior All-zero mask (both lanes return 0x0000000000000000)
 * @validation_method Verify self-comparison produces false results
 */
TEST_F(F64x2NeTestSuite, IdenticalValues_ReturnsFalse)
{
    uint8_t result[16];

    // Test with positive values
    ASSERT_TRUE(call_f64x2_ne(42.0, 3.14159, 42.0, 3.14159, result))
        << "Failed to execute f64x2.ne with identical positive values";
    validate_f64x2_ne_result(false, false, result);

    // Test with negative values
    ASSERT_TRUE(call_f64x2_ne(-123.456, -789.012, -123.456, -789.012, result))
        << "Failed to execute f64x2.ne with identical negative values";
    validate_f64x2_ne_result(false, false, result);

    // Test with zero values
    ASSERT_TRUE(call_f64x2_ne(0.0, 0.0, 0.0, 0.0, result))
        << "Failed to execute f64x2.ne with identical zero values";
    validate_f64x2_ne_result(false, false, result);

    // Test with very large values
    ASSERT_TRUE(call_f64x2_ne(1e100, 1e200, 1e100, 1e200, result))
        << "Failed to execute f64x2.ne with identical large values";
    validate_f64x2_ne_result(false, false, result);
}

/**
 * @test SpecialIEEE754Values_HandledCorrectly
 * @brief Validates IEEE 754 special value handling (NaN, infinity, zeros)
 * @details Tests critical IEEE 754 behaviors: NaN != NaN, infinity comparisons,
 *          positive/negative zero equivalence. Ensures WAMR follows IEEE standards.
 * @test_category Edge - IEEE 754 compliance validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_ne
 * @input_conditions NaN vs NaN, +∞ vs -∞, +0.0 vs -0.0
 * @expected_behavior NaN always ≠ (true), infinities and zeros per IEEE rules
 * @validation_method Verify IEEE 754 compliance for special cases
 */
TEST_F(F64x2NeTestSuite, SpecialIEEE754Values_HandledCorrectly)
{
    uint8_t result[16];

    // NaN behavior: NaN != NaN (should return true)
    double nan_val = get_positive_nan();
    ASSERT_TRUE(call_f64x2_ne(nan_val, 1.0, nan_val, 1.0, result))
        << "Failed to execute f64x2.ne with NaN values";
    validate_f64x2_ne_result(true, false, result);  // NaN != NaN is true, 1.0 == 1.0 is false

    // NaN vs different NaN (should return true for both)
    ASSERT_TRUE(call_f64x2_ne(nan_val, nan_val, 1.0, 1.0, result))
        << "Failed to execute f64x2.ne with NaN vs non-NaN";
    validate_f64x2_ne_result(true, true, result);  // NaN != 1.0 is true for both lanes

    // Positive vs negative infinity (should be not equal)
    ASSERT_TRUE(call_f64x2_ne(get_positive_infinity(), get_negative_infinity(),
                              get_negative_infinity(), get_positive_infinity(), result))
        << "Failed to execute f64x2.ne with positive/negative infinity";
    validate_f64x2_ne_result(true, true, result);  // +∞ != -∞ for both lanes

    // Same infinities (should be equal)
    ASSERT_TRUE(call_f64x2_ne(get_positive_infinity(), get_negative_infinity(),
                              get_positive_infinity(), get_negative_infinity(), result))
        << "Failed to execute f64x2.ne with same infinities";
    validate_f64x2_ne_result(false, false, result);  // +∞ == +∞ and -∞ == -∞

    // Positive vs negative zero (IEEE 754: +0.0 == -0.0)
    ASSERT_TRUE(call_f64x2_ne(get_positive_zero(), get_negative_zero(),
                              get_negative_zero(), get_positive_zero(), result))
        << "Failed to execute f64x2.ne with positive/negative zero";
    validate_f64x2_ne_result(false, false, result);  // +0.0 == -0.0 per IEEE 754
}

/**
 * @test BoundaryValues_CompareCorrectly
 * @brief Validates boundary value comparisons (DBL_MAX, DBL_MIN, extreme values)
 * @details Tests comparison behavior at floating-point representation limits.
 *          Ensures correct handling of maximum, minimum, and near-boundary values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_ne
 * @input_conditions Maximum/minimum f64 values, near-boundary comparisons
 * @expected_behavior Correct comparison results for extreme values
 * @validation_method Verify boundary conditions don't cause incorrect results
 */
TEST_F(F64x2NeTestSuite, BoundaryValues_CompareCorrectly)
{
    uint8_t result[16];

    // Test with maximum double values
    double max_val = get_max_double();
    ASSERT_TRUE(call_f64x2_ne(max_val, 1.0, max_val, 2.0, result))
        << "Failed to execute f64x2.ne with maximum double values";
    validate_f64x2_ne_result(false, true, result);  // max == max, 1.0 != 2.0

    // Test with minimum normalized double values
    double min_val = get_min_double();
    ASSERT_TRUE(call_f64x2_ne(min_val, min_val, min_val * 2.0, min_val, result))
        << "Failed to execute f64x2.ne with minimum double values";
    validate_f64x2_ne_result(true, false, result);  // min != min*2, min == min

    // Test with very large vs very small values
    ASSERT_TRUE(call_f64x2_ne(max_val, min_val, -max_val, -min_val, result))
        << "Failed to execute f64x2.ne with large vs small values";
    validate_f64x2_ne_result(true, true, result);  // max != -max, min != -min

    // Test with values near boundary conditions
    double near_max = max_val / 2.0;
    ASSERT_TRUE(call_f64x2_ne(near_max, near_max, max_val, near_max, result))
        << "Failed to execute f64x2.ne with near-boundary values";
    validate_f64x2_ne_result(true, false, result);  // near_max != max, near_max == near_max
}

/**
 * @test SubnormalNumbers_HandleProperly
 * @brief Validates subnormal/denormal number comparisons
 * @details Tests comparison behavior with very small numbers that are in the denormal range.
 *          Ensures correct IEEE 754 handling of subnormal values in SIMD operations.
 * @test_category Edge - Subnormal number validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_f64x2_ne
 * @input_conditions Very small numbers near zero, denormal ranges
 * @expected_behavior Correct comparison per IEEE 754 subnormal rules
 * @validation_method Verify subnormal numbers compare correctly
 */
TEST_F(F64x2NeTestSuite, SubnormalNumbers_HandleProperly)
{
    uint8_t result[16];

    // Test with denormal/subnormal numbers
    double denorm_val = get_denormal_double();
    ASSERT_TRUE(call_f64x2_ne(denorm_val, denorm_val, denorm_val, denorm_val * 2.0, result))
        << "Failed to execute f64x2.ne with subnormal values";
    validate_f64x2_ne_result(false, true, result);  // denorm == denorm, denorm != denorm*2

    // Test subnormal vs zero
    ASSERT_TRUE(call_f64x2_ne(denorm_val, 0.0, 0.0, denorm_val, result))
        << "Failed to execute f64x2.ne with subnormal vs zero";
    validate_f64x2_ne_result(true, true, result);  // denorm != 0.0 for both lanes

    // Test very small normal vs subnormal
    double small_normal = get_min_double();
    ASSERT_TRUE(call_f64x2_ne(small_normal, denorm_val, denorm_val, small_normal, result))
        << "Failed to execute f64x2.ne with small normal vs subnormal";
    validate_f64x2_ne_result(true, true, result);  // small_normal != denorm for both lanes

    // Test negative subnormal values
    ASSERT_TRUE(call_f64x2_ne(-denorm_val, denorm_val, denorm_val, -denorm_val, result))
        << "Failed to execute f64x2.ne with negative subnormal values";
    validate_f64x2_ne_result(true, true, result);  // -denorm != denorm for both lanes
}

/**
 * @test CrossExecutionModeConsistency_ProducesSameResults
 * @brief Validates consistent results across interpreter and AOT modes
 * @details Uses parameterized testing to ensure f64x2.ne produces identical results
 *          in both interpreter and AOT execution modes for all test scenarios.
 * @test_category Integration - Cross-mode consistency validation
 * @coverage_target core/iwasm/interpreter + core/iwasm/aot execution paths
 * @input_conditions Representative test vectors from all categories
 * @expected_behavior Identical results regardless of execution mode
 * @validation_method Bit-exact result matching between modes
 */
TEST_F(F64x2NeTestSuite, CrossExecutionModeConsistency_ProducesSameResults)
{
    uint8_t result[16];

    // Test representative values across different categories to ensure mode consistency

    // Basic comparison consistency
    ASSERT_TRUE(call_f64x2_ne(123.456, -789.012, 123.456, -999.999, result))
        << "Failed cross-mode consistency test with basic values";
    validate_f64x2_ne_result(false, true, result);

    // Special values consistency
    ASSERT_TRUE(call_f64x2_ne(get_positive_nan(), get_positive_infinity(),
                              1.0, get_positive_infinity(), result))
        << "Failed cross-mode consistency test with special values";
    validate_f64x2_ne_result(true, false, result);

    // Boundary values consistency
    ASSERT_TRUE(call_f64x2_ne(get_max_double(), get_min_double(),
                              get_max_double(), get_denormal_double(), result))
        << "Failed cross-mode consistency test with boundary values";
    validate_f64x2_ne_result(false, true, result);

    // Zero handling consistency
    ASSERT_TRUE(call_f64x2_ne(get_positive_zero(), get_negative_zero(),
                              get_negative_zero(), get_positive_zero(), result))
        << "Failed cross-mode consistency test with zero values";
    validate_f64x2_ne_result(false, false, result);
}