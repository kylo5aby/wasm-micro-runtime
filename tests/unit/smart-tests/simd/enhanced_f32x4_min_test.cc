/**
 * @file enhanced_f32x4_min_test.cc
 * @brief Comprehensive unit tests for f32x4.min SIMD opcode
 * @details Tests f32x4.min functionality with focus on element-wise minimum operation
 *          of two 32-bit single-precision floating-point vectors, IEEE 754 compliance validation,
 *          and comprehensive edge case coverage. Validates WAMR SIMD implementation correctness.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_min_test.cc
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
 * @class F32x4MinTestSuite
 * @brief Test fixture class for f32x4.min opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F32x4MinTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.min testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_min_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.min test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_min_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.min tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Automatically handled by RAII destructors for runtime and execution environment.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_min_test.cc:TearDown
     */
    void TearDown() override
    {
        // Automatic cleanup via RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute f32x4.min test with two input vectors and get result
     * @details Calls WASM f32x4_min_basic function with f32 input values and extracts results.
     *          Handles v128 data conversion and proper argument marshaling for WAMR execution.
     * @param f1_lane0 First vector lane 0 value
     * @param f1_lane1 First vector lane 1 value
     * @param f1_lane2 First vector lane 2 value
     * @param f1_lane3 First vector lane 3 value
     * @param f2_lane0 Second vector lane 0 value
     * @param f2_lane1 Second vector lane 1 value
     * @param f2_lane2 Second vector lane 2 value
     * @param f2_lane3 Second vector lane 3 value
     * @param result_bytes 16-byte array to store the minimum operation result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_min_test.cc:call_f32x4_min
     */
    bool call_f32x4_min(float f1_lane0, float f1_lane1, float f1_lane2, float f1_lane3,
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

        // Execute WASM function: f32x4_min_basic
        bool success = dummy_env->execute("test_f32x4_min", 8, argv);
        EXPECT_TRUE(success) << "Failed to execute f32x4.min WASM function";

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
     * @brief Extract float values from a 16-byte v128 result representing f32x4
     * @details Converts raw v128 bytes back to four float values for verification.
     *          Handles little-endian byte ordering for proper float reconstruction.
     * @param result_bytes 16-byte array containing the v128 result
     * @param lane0_result Reference to store the lane 0 float value
     * @param lane1_result Reference to store the lane 1 float value
     * @param lane2_result Reference to store the lane 2 float value
     * @param lane3_result Reference to store the lane 3 float value
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_min_test.cc:extract_f32x4_result
     */
    void extract_f32x4_result(const uint8_t result_bytes[16],
                              float &lane0_result, float &lane1_result,
                              float &lane2_result, float &lane3_result)
    {
        // Extract four 32-bit floats from the v128 result
        memcpy(&lane0_result, &result_bytes[0], sizeof(float));   // Lane 0
        memcpy(&lane1_result, &result_bytes[4], sizeof(float));   // Lane 1
        memcpy(&lane2_result, &result_bytes[8], sizeof(float));   // Lane 2
        memcpy(&lane3_result, &result_bytes[12], sizeof(float));  // Lane 3
    }

    /**
     * @brief Check if two float values are bitwise equal (handles NaN and signed zero)
     * @details Performs bitwise comparison to distinguish between +0.0/-0.0 and different NaN values.
     *          Essential for proper IEEE 754 compliance testing in minimum operations.
     * @param a First float value
     * @param b Second float value
     * @return true if bitwise equal, false otherwise
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_min_test.cc:FloatBitwiseEqual
     */
    bool FloatBitwiseEqual(float a, float b)
    {
        union { float f; uint32_t i; } ua, ub;
        ua.f = a;
        ub.f = b;
        return ua.i == ub.i;
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicMinimum_ReturnsCorrectResults
 * @brief Validates f32x4.min produces correct minimum values for typical inputs
 * @details Tests fundamental minimum operation with positive, negative, and mixed-sign values.
 *          Verifies that f32x4.min correctly computes min(a[i], b[i]) for each lane.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Standard f32 vectors with various value combinations
 * @expected_behavior Returns element-wise minimum values in result vector
 * @validation_method Direct comparison of WASM function result with expected minimum values
 */
TEST_F(F32x4MinTestSuite, BasicMinimum_ReturnsCorrectResults)
{
    uint8_t result_bytes[16];
    float lane0, lane1, lane2, lane3;

    // Test: min([1.5, -2.5, 3.0, -4.0], [2.0, -1.5, 2.5, -5.0])
    // Expected result: [1.5, -2.5, 2.5, -5.0]
    bool success = call_f32x4_min(1.5f, -2.5f, 3.0f, -4.0f,
                                  2.0f, -1.5f, 2.5f, -5.0f,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.min basic test execution failed";

    extract_f32x4_result(result_bytes, lane0, lane1, lane2, lane3);

    ASSERT_FLOAT_EQ(1.5f, lane0) << "Basic minimum failed at lane 0";
    ASSERT_FLOAT_EQ(-2.5f, lane1) << "Basic minimum failed at lane 1";
    ASSERT_FLOAT_EQ(2.5f, lane2) << "Basic minimum failed at lane 2";
    ASSERT_FLOAT_EQ(-5.0f, lane3) << "Basic minimum failed at lane 3";
}

/**
 * @test MixedSignValues_ProducesCorrectMinimum
 * @brief Validates f32x4.min with combinations of positive and negative values
 * @details Tests minimum selection across sign boundaries and various magnitude combinations.
 *          Ensures proper handling of mixed positive/negative value scenarios.
 * @test_category Main - Sign handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Vectors with mixed positive/negative values
 * @expected_behavior Correct minimum selection regardless of sign combinations
 * @validation_method Comparison of results with mathematically expected minimum values
 */
TEST_F(F32x4MinTestSuite, MixedSignValues_ProducesCorrectMinimum)
{
    uint8_t result_bytes[16];
    float lane0, lane1, lane2, lane3;

    // Test: min([-1.0, 2.0, -3.0, 4.0], [1.0, -2.0, 3.0, -4.0])
    // Expected result: [-1.0, -2.0, -3.0, -4.0]
    bool success = call_f32x4_min(-1.0f, 2.0f, -3.0f, 4.0f,
                                  1.0f, -2.0f, 3.0f, -4.0f,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.min mixed sign test execution failed";

    extract_f32x4_result(result_bytes, lane0, lane1, lane2, lane3);

    ASSERT_FLOAT_EQ(-1.0f, lane0) << "Mixed sign minimum failed at lane 0";
    ASSERT_FLOAT_EQ(-2.0f, lane1) << "Mixed sign minimum failed at lane 1";
    ASSERT_FLOAT_EQ(-3.0f, lane2) << "Mixed sign minimum failed at lane 2";
    ASSERT_FLOAT_EQ(-4.0f, lane3) << "Mixed sign minimum failed at lane 3";
}

/**
 * @test IdenticalValues_ReturnsIdenticalResult
 * @brief Validates f32x4.min idempotent property: min(x, x) = x
 * @details Tests that minimum of identical values returns the identical value.
 *          Validates mathematical property of minimum operation.
 * @test_category Main - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Two identical f32x4 vectors
 * @expected_behavior Result vector identical to both input vectors
 * @validation_method Direct equality comparison with input values
 */
TEST_F(F32x4MinTestSuite, IdenticalValues_ReturnsIdenticalResult)
{
    uint8_t result_bytes[16];
    float lane0, lane1, lane2, lane3;

    // Test: min([1.25, -3.5, 0.0, 42.0], [1.25, -3.5, 0.0, 42.0])
    // Expected result: [1.25, -3.5, 0.0, 42.0]
    bool success = call_f32x4_min(1.25f, -3.5f, 0.0f, 42.0f,
                                  1.25f, -3.5f, 0.0f, 42.0f,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.min identical values test execution failed";

    extract_f32x4_result(result_bytes, lane0, lane1, lane2, lane3);

    ASSERT_FLOAT_EQ(1.25f, lane0) << "Identical value minimum failed at lane 0";
    ASSERT_FLOAT_EQ(-3.5f, lane1) << "Identical value minimum failed at lane 1";
    ASSERT_FLOAT_EQ(0.0f, lane2) << "Identical value minimum failed at lane 2";
    ASSERT_FLOAT_EQ(42.0f, lane3) << "Identical value minimum failed at lane 3";
}

/**
 * @test BoundaryValues_HandlesExtremeRanges
 * @brief Validates f32x4.min with boundary and extreme floating-point values
 * @details Tests minimum operation with FLT_MAX, FLT_MIN, and boundary transitions.
 *          Ensures correct handling of extreme value ranges and boundary conditions.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Vectors containing FLT_MAX, FLT_MIN, and boundary values
 * @expected_behavior Correct minimum selection for extreme value ranges
 * @validation_method Comparison with expected boundary value minimums
 */
TEST_F(F32x4MinTestSuite, BoundaryValues_HandlesExtremeRanges)
{
    uint8_t result_bytes[16];
    float lane0, lane1, lane2, lane3;

    // Test: min([FLT_MAX, FLT_MIN, -FLT_MAX, 1.0], [1.0, -1.0, FLT_MIN, FLT_MAX])
    // Expected result: [1.0, -1.0, -FLT_MAX, 1.0]
    bool success = call_f32x4_min(FLT_MAX, FLT_MIN, -FLT_MAX, 1.0f,
                                  1.0f, -1.0f, FLT_MIN, FLT_MAX,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.min boundary values test execution failed";

    extract_f32x4_result(result_bytes, lane0, lane1, lane2, lane3);

    ASSERT_FLOAT_EQ(1.0f, lane0) << "Boundary value minimum failed at lane 0";
    ASSERT_FLOAT_EQ(-1.0f, lane1) << "Boundary value minimum failed at lane 1";
    ASSERT_FLOAT_EQ(-FLT_MAX, lane2) << "Boundary value minimum failed at lane 2";
    ASSERT_FLOAT_EQ(1.0f, lane3) << "Boundary value minimum failed at lane 3";
}

/**
 * @test SubnormalNumbers_ComparesCorrectly
 * @brief Validates f32x4.min with subnormal (denormalized) floating-point values
 * @details Tests minimum operation with subnormal numbers and normal number comparisons.
 *          Ensures correct IEEE 754 compliant handling of subnormal values.
 * @test_category Corner - Subnormal value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Vectors containing subnormal and normal floating-point values
 * @expected_behavior Correct minimum selection for subnormal value scenarios
 * @validation_method Comparison with IEEE 754 compliant minimum results
 */
TEST_F(F32x4MinTestSuite, SubnormalNumbers_ComparesCorrectly)
{
    uint8_t result_bytes[16];
    float lane0, lane1, lane2, lane3;

    // Create subnormal numbers (very small denormalized values)
    float subnormal = 1.0e-40f; // Subnormal value
    float normal = 1.0e-38f;    // Normal value (larger than FLT_MIN)

    // Test: min([subnormal, -subnormal, normal, -normal], [normal, -normal, subnormal, -subnormal])
    // Expected result: [subnormal, -normal, subnormal, -normal]
    bool success = call_f32x4_min(subnormal, -subnormal, normal, -normal,
                                  normal, -normal, subnormal, -subnormal,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.min subnormal numbers test execution failed";

    extract_f32x4_result(result_bytes, lane0, lane1, lane2, lane3);

    ASSERT_FLOAT_EQ(subnormal, lane0) << "Subnormal number minimum failed at lane 0";
    ASSERT_FLOAT_EQ(-normal, lane1) << "Subnormal number minimum failed at lane 1";
    ASSERT_FLOAT_EQ(subnormal, lane2) << "Subnormal number minimum failed at lane 2";
    ASSERT_FLOAT_EQ(-normal, lane3) << "Subnormal number minimum failed at lane 3";
}

/**
 * @test SignedZeroHandling_FollowsIEEE754Standard
 * @brief Validates f32x4.min IEEE 754 signed zero handling: min(-0.0, +0.0) = -0.0
 * @details Tests that minimum operation correctly handles signed zero according to IEEE 754.
 *          Validates that negative zero is considered smaller than positive zero.
 * @test_category Edge - IEEE 754 signed zero validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Vectors with positive and negative zero combinations
 * @expected_behavior Negative zero returned as minimum in signed zero comparisons
 * @validation_method Bitwise comparison to distinguish signed zeros
 */
TEST_F(F32x4MinTestSuite, SignedZeroHandling_FollowsIEEE754Standard)
{
    uint8_t result_bytes[16];
    float lane0, lane1, lane2, lane3;

    float pos_zero = 0.0f;
    float neg_zero = -0.0f;

    // Test: min([+0.0, -0.0, +0.0, 1.0], [-0.0, +0.0, +0.0, -0.0])
    // Expected result: [-0.0, -0.0, +0.0, -0.0]
    bool success = call_f32x4_min(pos_zero, neg_zero, pos_zero, 1.0f,
                                  neg_zero, pos_zero, pos_zero, -0.0f,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.min signed zero test execution failed";

    extract_f32x4_result(result_bytes, lane0, lane1, lane2, lane3);

    // Check that negative zero is returned in both mixed cases
    ASSERT_TRUE(FloatBitwiseEqual(neg_zero, lane0))
        << "min(+0.0, -0.0) should return -0.0";
    ASSERT_TRUE(FloatBitwiseEqual(neg_zero, lane1))
        << "min(-0.0, +0.0) should return -0.0";
    ASSERT_TRUE(FloatBitwiseEqual(pos_zero, lane2))
        << "min(+0.0, +0.0) should return +0.0";
    ASSERT_TRUE(FloatBitwiseEqual(neg_zero, lane3))
        << "min(1.0, -0.0) should return -0.0";
}

/**
 * @test InfinityOperations_ProducesCorrectResults
 * @brief Validates f32x4.min with positive and negative infinity values
 * @details Tests minimum operation with infinity combinations and finite value comparisons.
 *          Ensures correct IEEE 754 infinity ordering: -∞ < finite < +∞.
 * @test_category Edge - Infinity value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Vectors containing positive/negative infinity and finite values
 * @expected_behavior Correct minimum selection following infinity ordering rules
 * @validation_method Comparison with IEEE 754 infinity comparison results
 */
TEST_F(F32x4MinTestSuite, InfinityOperations_ProducesCorrectResults)
{
    uint8_t result_bytes[16];
    float lane0, lane1, lane2, lane3;

    float pos_inf = INFINITY;
    float neg_inf = -INFINITY;

    // Test: min([+inf, -inf, +inf, 1.0], [1.0, 1.0, -inf, +inf])
    // Expected result: [1.0, -inf, -inf, 1.0]
    bool success = call_f32x4_min(pos_inf, neg_inf, pos_inf, 1.0f,
                                  1.0f, 1.0f, neg_inf, pos_inf,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.min infinity test execution failed";

    extract_f32x4_result(result_bytes, lane0, lane1, lane2, lane3);

    ASSERT_FLOAT_EQ(1.0f, lane0) << "Infinity minimum failed at lane 0";
    ASSERT_FLOAT_EQ(neg_inf, lane1) << "Infinity minimum failed at lane 1";
    ASSERT_FLOAT_EQ(neg_inf, lane2) << "Infinity minimum failed at lane 2";
    ASSERT_FLOAT_EQ(1.0f, lane3) << "Infinity minimum failed at lane 3";
}

/**
 * @test NaNPropagation_HandlesNaNInputsCorrectly
 * @brief Validates f32x4.min NaN propagation: min(NaN, value) = NaN
 * @details Tests that NaN values are correctly propagated in minimum operations.
 *          Ensures IEEE 754 compliant NaN handling in all scenarios.
 * @test_category Edge - NaN propagation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Vectors with NaN values in various positions and combinations
 * @expected_behavior NaN propagation in all cases involving NaN inputs
 * @validation_method isnan() check for proper NaN result propagation
 */
TEST_F(F32x4MinTestSuite, NaNPropagation_HandlesNaNInputsCorrectly)
{
    uint8_t result_bytes[16];
    float lane0, lane1, lane2, lane3;

    float nan_val = NAN;

    // Test: min([NaN, 1.0, NaN, 2.0], [1.0, NaN, NaN, NaN])
    // Expected result: [NaN, NaN, NaN, NaN]
    bool success = call_f32x4_min(nan_val, 1.0f, nan_val, 2.0f,
                                  1.0f, nan_val, nan_val, nan_val,
                                  result_bytes);
    ASSERT_TRUE(success) << "f32x4.min NaN propagation test execution failed";

    extract_f32x4_result(result_bytes, lane0, lane1, lane2, lane3);

    // All results should be NaN when any input is NaN
    ASSERT_TRUE(std::isnan(lane0))
        << "NaN propagation failed at lane 0: expected NaN, got " << lane0;
    ASSERT_TRUE(std::isnan(lane1))
        << "NaN propagation failed at lane 1: expected NaN, got " << lane1;
    ASSERT_TRUE(std::isnan(lane2))
        << "NaN propagation failed at lane 2: expected NaN, got " << lane2;
    ASSERT_TRUE(std::isnan(lane3))
        << "NaN propagation failed at lane 3: expected NaN, got " << lane3;
}

/**
 * @test MathematicalProperties_ValidatesCommutativeProperty
 * @brief Validates f32x4.min commutative property: min(a, b) = min(b, a)
 * @details Tests that minimum operation is commutative for all value types.
 *          Ensures order independence of operands in minimum computation.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_min_operation
 * @input_conditions Various value combinations tested in both operand orders
 * @expected_behavior Identical results regardless of operand order
 * @validation_method Direct comparison of min(a,b) with min(b,a) results
 */
TEST_F(F32x4MinTestSuite, MathematicalProperties_ValidatesCommutativeProperty)
{
    uint8_t result_bytes1[16], result_bytes2[16];
    float lane0_1, lane1_1, lane2_1, lane3_1;
    float lane0_2, lane1_2, lane2_2, lane3_2;

    // Test min([3.14, -2.71, +inf, +0.0], [2.71, -3.14, -inf, -0.0])
    bool success1 = call_f32x4_min(3.14f, -2.71f, INFINITY, 0.0f,
                                   2.71f, -3.14f, -INFINITY, -0.0f,
                                   result_bytes1);
    ASSERT_TRUE(success1) << "f32x4.min commutative test 1 execution failed";

    // Test min([2.71, -3.14, -inf, -0.0], [3.14, -2.71, +inf, +0.0]) - should be identical
    bool success2 = call_f32x4_min(2.71f, -3.14f, -INFINITY, -0.0f,
                                   3.14f, -2.71f, INFINITY, 0.0f,
                                   result_bytes2);
    ASSERT_TRUE(success2) << "f32x4.min commutative test 2 execution failed";

    extract_f32x4_result(result_bytes1, lane0_1, lane1_1, lane2_1, lane3_1);
    extract_f32x4_result(result_bytes2, lane0_2, lane1_2, lane2_2, lane3_2);

    // Check commutative property for each lane
    ASSERT_TRUE(FloatBitwiseEqual(lane0_1, lane0_2))
        << "Commutative property failed at lane 0";
    ASSERT_TRUE(FloatBitwiseEqual(lane1_1, lane1_2))
        << "Commutative property failed at lane 1";
    ASSERT_TRUE(FloatBitwiseEqual(lane2_1, lane2_2))
        << "Commutative property failed at lane 2";
    ASSERT_TRUE(FloatBitwiseEqual(lane3_1, lane3_2))
        << "Commutative property failed at lane 3";
}

/**
 * @test ModuleLoading_ValidatesSIMDSupport
 * @brief Validates that WASM module with f32x4.min loads successfully with SIMD support
 * @details Tests proper WASM module loading and instantiation with SIMD instructions.
 *          Ensures SIMD feature availability and correct module validation.
 * @test_category Error - SIMD support validation
 * @coverage_target wasm_runtime_load, wasm_runtime_instantiate
 * @input_conditions WASM module containing f32x4.min SIMD instruction
 * @expected_behavior Successful module loading and instantiation
 * @validation_method Non-null module and module instance validation
 */
TEST_F(F32x4MinTestSuite, ModuleLoading_ValidatesSIMDSupport)
{
    // Module and execution environment should be successfully loaded in SetUp
    ASSERT_NE(nullptr, dummy_env->get())
        << "WASM execution environment with f32x4.min should be created successfully with SIMD support";

    // Verify that basic execution works
    uint8_t result_bytes[16];
    bool success = call_f32x4_min(1.0f, 2.0f, 3.0f, 4.0f,
                                  5.0f, 1.5f, 2.5f, 3.5f,
                                  result_bytes);
    ASSERT_TRUE(success)
        << "f32x4.min test function should be accessible and executable in loaded module";
}

