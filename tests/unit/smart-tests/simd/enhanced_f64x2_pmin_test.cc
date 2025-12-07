/**
 * @file enhanced_f64x2_pmin_test.cc
 * @brief Comprehensive unit tests for f64x2.pmin SIMD opcode
 * @details Tests f64x2.pmin functionality across interpreter and AOT execution modes
 *          with focus on element-wise pseudo-minimum operation of two 64-bit double-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmin_test.cc
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
 * @class F64x2PminTestSuite
 * @brief Test fixture class for f64x2.pmin opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F64x2PminTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f64x2.pmin testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmin_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.pmin test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_pmin_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.pmin tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Ensures proper cleanup of WAMR runtime and execution environment.
     *          Uses RAII pattern for automatic resource management.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmin_test.cc:TearDown
     */
    void TearDown() override
    {
        // RAII handles cleanup automatically
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f64x2.pmin function with two vector inputs
     * @param func_name Name of the WASM function to execute
     * @param d1_lane0 First double value for lane 0 of first vector
     * @param d1_lane1 First double value for lane 1 of first vector
     * @param d2_lane0 Second double value for lane 0 of second vector
     * @param d2_lane1 Second double value for lane 1 of second vector
     * @param result_bytes 16-byte array to store the pseudo-minimum operation result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmin_test.cc:call_f64x2_pmin
     */
    bool call_f64x2_pmin(const char* func_name, double d1_lane0, double d1_lane1, double d2_lane0, double d2_lane1,
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
        EXPECT_TRUE(success) << "Failed to execute f64x2.pmin WASM function: " << func_name;

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
     * @param result_bytes 16-byte array containing the v128 result
     * @param lane0_result Reference to store the lane 0 double value
     * @param lane1_result Reference to store the lane 1 double value
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f64x2_pmin_test.cc:extract_f64x2_result
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

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicPseudoMinimum_ReturnsCorrectResults
 * @brief Validates f64x2.pmin produces correct pseudo-minimum results for typical inputs
 * @details Tests fundamental pseudo-minimum operation with regular double-precision values.
 *          Verifies that f64x2.pmin correctly computes element-wise pseudo-minimum for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f64x2_pmin_operation
 * @input_conditions Regular f64 pairs: ([1.5, 3.2], [2.1, 1.8]), ([5.0, -2.5], [-1.0, 4.0]), ([2.5, 7.1], [2.5, 7.1])
 * @expected_behavior Returns element-wise mathematical minimum: [1.5, 1.8], [-1.0, -2.5], [2.5, 7.1]
 * @validation_method Direct comparison of WASM function result with expected values using ASSERT_DOUBLE_EQ
 */
TEST_F(F64x2PminTestSuite, BasicPseudoMinimum_ReturnsCorrectResults)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test case 1: Mixed regular values
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_basic", 1.5, 3.2, 2.1, 1.8, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(1.5, lane0_result) << "First lane pseudo-minimum failed for mixed regular values";
    ASSERT_DOUBLE_EQ(1.8, lane1_result) << "Second lane pseudo-minimum failed for mixed regular values";

    // Test case 2: Positive and negative combinations
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_basic", 5.0, -2.5, -1.0, 4.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(-1.0, lane0_result) << "First lane pseudo-minimum failed for positive/negative mix";
    ASSERT_DOUBLE_EQ(-2.5, lane1_result) << "Second lane pseudo-minimum failed for positive/negative mix";

    // Test case 3: Identical values (identity property)
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_basic", 2.5, 7.1, 2.5, 7.1, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(2.5, lane0_result) << "First lane pseudo-minimum failed for identical values";
    ASSERT_DOUBLE_EQ(7.1, lane1_result) << "Second lane pseudo-minimum failed for identical values";
}

/**
 * @test NaNPropagation_FollowsPseudoMinRules
 * @brief Validates f64x2.pmin follows regular minimum NaN propagation (NaN propagation like min operation)
 * @details Tests NaN handling behavior in f64x2.pmin operation.
 *          Verifies that NaN values propagate through the minimum operation (any NaN operand produces NaN result).
 * @test_category Edge - Special NaN handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f64x2_pmin_nan_handling
 * @input_conditions NaN combinations: ([NaN, 1.0], [2.0, 3.0]), ([1.0, 2.0], [NaN, 3.0]), ([NaN, NaN], [5.0, 6.0])
 * @expected_behavior NaN propagation: [NaN, 1.0], [NaN, 3.0], [NaN, NaN] respectively
 * @validation_method NaN detection with isnan() for NaN results and value assertions for non-NaN results
 */
TEST_F(F64x2PminTestSuite, NaNPropagation_FollowsPseudoMinRules)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;
    double nan_val = std::numeric_limits<double>::quiet_NaN();

    // Test case 1: First operand has NaN → returns NaN (regular min behavior)
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_nan", nan_val, 1.0, 2.0, 3.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_TRUE(std::isnan(lane0_result)) << "First lane should return NaN when first operand is NaN";
    ASSERT_DOUBLE_EQ(1.0, lane1_result) << "Second lane failed for first operand NaN";

    // Test case 2: Second operand has NaN → returns first operand (IEEE 754: NaN < x is false)
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_nan", 1.0, 2.0, nan_val, 3.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(1.0, lane0_result) << "First lane should return first operand when second operand is NaN";
    ASSERT_DOUBLE_EQ(2.0, lane1_result) << "Second lane failed for second operand NaN";

    // Test case 3: Both operands have NaN in first lane → returns NaN (regular min behavior)
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_nan", nan_val, nan_val, 5.0, 6.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_TRUE(std::isnan(lane0_result)) << "First lane should return NaN when both operands are NaN";
    ASSERT_TRUE(std::isnan(lane1_result)) << "Second lane should return NaN when both operands are NaN";

    // Test case 4: Mixed NaN scenarios in different lanes
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_nan", nan_val, 1.0, nan_val, 2.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_TRUE(std::isnan(lane0_result)) << "First lane should be NaN when both operands are NaN";
    ASSERT_DOUBLE_EQ(1.0, lane1_result) << "Second lane failed for mixed NaN scenario";
}

/**
 * @test SpecialValues_HandledCorrectly
 * @brief Validates f64x2.pmin handles IEEE 754 special values correctly
 * @details Tests pseudo-minimum behavior with infinities, signed zeros, and boundary values.
 *          Verifies IEEE 754 compliance for special floating-point value handling.
 * @test_category Edge - IEEE 754 special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f64x2_pmin_special_values
 * @input_conditions Special values: infinities (±∞), signed zeros (±0.0), mixed combinations
 * @expected_behavior IEEE 754 compliant pseudo-minimum results for all special value combinations
 * @validation_method Direct comparison using ASSERT_DOUBLE_EQ and infinity/zero checks
 */
TEST_F(F64x2PminTestSuite, SpecialValues_HandledCorrectly)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;
    double pos_inf = std::numeric_limits<double>::infinity();
    double neg_inf = -std::numeric_limits<double>::infinity();

    // Test case 1: Infinity handling
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_special", pos_inf, neg_inf, neg_inf, pos_inf, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(neg_inf, lane0_result) << "First lane infinity pseudo-minimum failed";
    ASSERT_DOUBLE_EQ(neg_inf, lane1_result) << "Second lane infinity pseudo-minimum failed";

    // Test case 2: Infinity with regular values
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_special", pos_inf, 1.0, 2.0, pos_inf, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(2.0, lane0_result) << "First lane failed with positive infinity";
    ASSERT_DOUBLE_EQ(1.0, lane1_result) << "Second lane failed with positive infinity";

    // Test case 3: Negative infinity behavior
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_special", neg_inf, neg_inf, 100.0, 200.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(neg_inf, lane0_result) << "First lane negative infinity pseudo-minimum failed";
    ASSERT_DOUBLE_EQ(neg_inf, lane1_result) << "Second lane negative infinity pseudo-minimum failed";

    // Test case 4: Signed zero handling (+0.0 vs -0.0)
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_special", +0.0, +0.0, -0.0, -0.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(-0.0, lane0_result) << "First lane signed zero pseudo-minimum failed";
    ASSERT_DOUBLE_EQ(-0.0, lane1_result) << "Second lane signed zero pseudo-minimum failed";
}

/**
 * @test BoundaryConditions_ProcessCorrectly
 * @brief Validates f64x2.pmin handles boundary values and extreme magnitudes correctly
 * @details Tests pseudo-minimum operation with maximum/minimum double values, denormalized numbers,
 *          and extreme magnitude differences to ensure robust boundary condition handling.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f64x2_pmin_boundaries
 * @input_conditions Boundary values: DBL_MAX, DBL_MIN, denormalized numbers, extreme magnitude differences
 * @expected_behavior Correct pseudo-minimum results for all boundary value combinations
 * @validation_method Precise boundary value result verification with ASSERT_DOUBLE_EQ
 */
TEST_F(F64x2PminTestSuite, BoundaryConditions_ProcessCorrectly)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;

    // Test case 1: Maximum double values
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_boundary", DBL_MAX, DBL_MAX, DBL_MAX - 1.0, DBL_MAX - 1.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(DBL_MAX - 1.0, lane0_result) << "First lane maximum value pseudo-minimum failed";
    ASSERT_DOUBLE_EQ(DBL_MAX - 1.0, lane1_result) << "Second lane maximum value pseudo-minimum failed";

    // Test case 2: Minimum positive normalized values
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_boundary", DBL_MIN, DBL_MIN, DBL_MIN * 2, DBL_MIN * 2, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(DBL_MIN, lane0_result) << "First lane minimum normalized pseudo-minimum failed";
    ASSERT_DOUBLE_EQ(DBL_MIN, lane1_result) << "Second lane minimum normalized pseudo-minimum failed";

    // Test case 3: Large magnitude differences
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_boundary", 1e308, 1e-308, 1e-308, 1e308, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(1e-308, lane0_result) << "First lane large magnitude difference failed";
    ASSERT_DOUBLE_EQ(1e-308, lane1_result) << "Second lane large magnitude difference failed";

    // Test case 4: Positive/negative extremes
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_boundary", DBL_MAX, -DBL_MAX, -DBL_MAX, DBL_MAX, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(-DBL_MAX, lane0_result) << "First lane positive/negative extreme failed";
    ASSERT_DOUBLE_EQ(-DBL_MAX, lane1_result) << "Second lane positive/negative extreme failed";
}

/**
 * @test CrossLaneIndependence_ValidatedSeparately
 * @brief Validates that f64x2.pmin processes each lane independently without cross-lane interference
 * @details Tests lane independence by using different scenarios in each lane to ensure
 *          proper SIMD vector processing without lane crosstalk or interference.
 * @test_category Main - Lane independence validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_f64x2_pmin_lane_processing
 * @input_conditions Mixed scenarios: different value types per lane (regular, NaN, infinity, boundary)
 * @expected_behavior Each lane processed independently with correct individual results
 * @validation_method Extract and validate each lane result separately with appropriate assertions
 */
TEST_F(F64x2PminTestSuite, CrossLaneIndependence_ValidatedSeparately)
{
    uint8_t result_bytes[16];
    double lane0_result, lane1_result;
    double nan_val = std::numeric_limits<double>::quiet_NaN();
    double pos_inf = std::numeric_limits<double>::infinity();

    // Test case 1: NaN in first lane, regular values in second lane
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_independence", nan_val, 10.0, 5.0, 20.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_TRUE(std::isnan(lane0_result)) << "First lane should return NaN when operand is NaN";
    ASSERT_DOUBLE_EQ(10.0, lane1_result) << "Second lane regular value processing failed";

    // Test case 2: Regular value in first lane, infinity in second lane
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_independence", 3.5, pos_inf, 7.2, 15.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(3.5, lane0_result) << "First lane regular value processing failed";
    ASSERT_DOUBLE_EQ(15.0, lane1_result) << "Second lane infinity handling failed";

    // Test case 3: Boundary value in first lane, signed zero in second lane
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_independence", DBL_MAX, +0.0, 1.0, -0.0, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(1.0, lane0_result) << "First lane boundary value processing failed";
    ASSERT_DOUBLE_EQ(-0.0, lane1_result) << "Second lane signed zero handling failed";

    // Test case 4: Extreme magnitude difference per lane validation
    ASSERT_TRUE(call_f64x2_pmin("f64x2_pmin_independence", 1e-100, 1e100, 1e100, 1e-100, result_bytes));
    extract_f64x2_result(result_bytes, lane0_result, lane1_result);
    ASSERT_DOUBLE_EQ(1e-100, lane0_result) << "First lane extreme magnitude handling failed";
    ASSERT_DOUBLE_EQ(1e-100, lane1_result) << "Second lane extreme magnitude handling failed";
}