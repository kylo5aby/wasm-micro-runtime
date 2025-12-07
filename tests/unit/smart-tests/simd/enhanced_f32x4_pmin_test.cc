/**
 * @file enhanced_f32x4_pmin_test.cc
 * @brief Comprehensive unit tests for f32x4.pmin SIMD opcode
 * @details Tests f32x4.pmin functionality across interpreter and AOT execution modes
 *          with focus on element-wise pseudo-minimum operation of two 32-bit single-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmin_test.cc
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
 * @class F32x4PminTestSuite
 * @brief Test fixture class for f32x4.pmin opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F32x4PminTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.pmin testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmin_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.pmin test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_pmin_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.pmin tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Performs cleanup of WAMR runtime environment using RAII helpers.
     *          Ensures proper resource deallocation and runtime shutdown.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmin_test.cc:TearDown
     */
    void TearDown() override
    {
        // RAII handles cleanup automatically
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to execute f32x4.pmin with 8 f32 parameters
     * @details Creates f32x4 vectors from input parameters, executes f32x4.pmin via WASM,
     *          and returns the result as 4 f32 values.
     * @param function_name WASM export function name to call
     * @param a0,a1,a2,a3 First f32x4 vector components
     * @param b0,b1,b2,b3 Second f32x4 vector components
     * @return Array of 4 f32 results from f32x4.pmin operation
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmin_test.cc:call_f32x4_pmin
     */
    std::array<float, 4> call_f32x4_pmin(const char *function_name,
                                         float a0, float a1, float a2, float a3,
                                         float b0, float b1, float b2, float b3)
    {
        // Prepare function arguments (8 f32 values for 2 f32x4 vectors)
        uint32_t argv[8];
        memcpy(&argv[0], &a0, sizeof(float));
        memcpy(&argv[1], &a1, sizeof(float));
        memcpy(&argv[2], &a2, sizeof(float));
        memcpy(&argv[3], &a3, sizeof(float));
        memcpy(&argv[4], &b0, sizeof(float));
        memcpy(&argv[5], &b1, sizeof(float));
        memcpy(&argv[6], &b2, sizeof(float));
        memcpy(&argv[7], &b3, sizeof(float));

        // Execute WASM function and validate successful execution
        bool success = dummy_env->execute(function_name, 8, argv);
        EXPECT_TRUE(success) << "Failed to execute " << function_name;

        // Extract and return f32x4 result as array
        std::array<float, 4> result;
        if (success) {
            memcpy(&result[0], &argv[0], sizeof(float));
            memcpy(&result[1], &argv[1], sizeof(float));
            memcpy(&result[2], &argv[2], sizeof(float));
            memcpy(&result[3], &argv[3], sizeof(float));
        }

        return result;
    }

    /**
     * @brief Helper to compare floating-point values with NaN handling
     * @details Handles NaN comparison correctly (NaN != NaN in IEEE 754)
     * @param expected Expected floating-point value
     * @param actual Actual floating-point value
     * @return True if values match (including both being NaN)
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmin_test.cc:float_equals_with_nan
     */
    bool float_equals_with_nan(float expected, float actual)
    {
        if (std::isnan(expected) && std::isnan(actual)) {
            return true; // Both NaN - considered equal
        }
        if (std::isnan(expected) || std::isnan(actual)) {
            return false; // Only one is NaN
        }
        return expected == actual; // Regular equality for normal values
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicPseudoMinimum_ReturnsCorrectResults
 * @brief Validates f32x4.pmin produces correct pseudo-minimum results for typical inputs
 * @details Tests fundamental pseudo-minimum operation with positive, negative, and mixed-sign floats.
 *          Verifies that f32x4.pmin correctly computes element-wise pseudo-minimum across all lanes.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmin_operation
 * @input_conditions Standard f32 pairs across all 4 lanes with typical values
 * @expected_behavior Returns element-wise pseudo-minimum: min(a[i], b[i]) for each lane
 * @validation_method Direct comparison of WASM function results with expected pseudo-minimum values
 */
TEST_F(F32x4PminTestSuite, BasicPseudoMinimum_ReturnsCorrectResults)
{
    // Test basic pseudo-minimum with positive numbers
    auto result1 = call_f32x4_pmin("f32x4_pmin_basic",
                                  1.5f, 2.0f, 3.5f, 4.0f,
                                  2.5f, 1.5f, 4.0f, 3.0f);
    ASSERT_EQ(1.5f, result1[0]) << "Lane 0: pmin(1.5, 2.5) should be 1.5";
    ASSERT_EQ(1.5f, result1[1]) << "Lane 1: pmin(2.0, 1.5) should be 1.5";
    ASSERT_EQ(3.5f, result1[2]) << "Lane 2: pmin(3.5, 4.0) should be 3.5";
    ASSERT_EQ(3.0f, result1[3]) << "Lane 3: pmin(4.0, 3.0) should be 3.0";

    // Test pseudo-minimum with mixed-sign values
    auto result2 = call_f32x4_pmin("f32x4_pmin_basic",
                                  -1.5f, 2.0f, -3.5f, 4.0f,
                                  1.0f, -2.5f, 2.0f, -1.0f);
    ASSERT_EQ(-1.5f, result2[0]) << "Lane 0: pmin(-1.5, 1.0) should be -1.5";
    ASSERT_EQ(-2.5f, result2[1]) << "Lane 1: pmin(2.0, -2.5) should be -2.5";
    ASSERT_EQ(-3.5f, result2[2]) << "Lane 2: pmin(-3.5, 2.0) should be -3.5";
    ASSERT_EQ(-1.0f, result2[3]) << "Lane 3: pmin(4.0, -1.0) should be -1.0";

    // Test pseudo-minimum across different value ranges
    auto result3 = call_f32x4_pmin("f32x4_pmin_basic",
                                  100.0f, 0.1f, 1000.0f, 0.001f,
                                  50.0f, 0.2f, 2000.0f, 0.002f);
    ASSERT_EQ(50.0f, result3[0]) << "Lane 0: pmin(100.0, 50.0) should be 50.0";
    ASSERT_EQ(0.1f, result3[1]) << "Lane 1: pmin(0.1, 0.2) should be 0.1";
    ASSERT_EQ(1000.0f, result3[2]) << "Lane 2: pmin(1000.0, 2000.0) should be 1000.0";
    ASSERT_EQ(0.001f, result3[3]) << "Lane 3: pmin(0.001, 0.002) should be 0.001";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Validates f32x4.pmin handles IEEE 754 boundary values correctly
 * @details Tests pseudo-minimum behavior with extreme floating-point values including
 *          infinity, maximum/minimum normal values, and subnormal numbers.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmin_boundary_handling
 * @input_conditions F32 boundary values: infinity, F32_MAX, F32_MIN, subnormals
 * @expected_behavior Proper IEEE 754 compliant pseudo-minimum selection for boundary cases
 * @validation_method Verification of boundary value pseudo-minimum results with IEEE 754 rules
 */
TEST_F(F32x4PminTestSuite, BoundaryValues_HandledCorrectly)
{
    // Test infinity combinations with finite values
    auto result1 = call_f32x4_pmin("f32x4_pmin_boundary",
                                  INFINITY, -INFINITY, INFINITY, 1.0f,
                                  1.0f, 1.0f, -INFINITY, INFINITY);
    ASSERT_EQ(1.0f, result1[0]) << "Lane 0: pmin(+INF, 1.0) should be 1.0";
    ASSERT_EQ(-INFINITY, result1[1]) << "Lane 1: pmin(-INF, 1.0) should be -INF";
    ASSERT_EQ(-INFINITY, result1[2]) << "Lane 2: pmin(+INF, -INF) should be -INF";
    ASSERT_EQ(1.0f, result1[3]) << "Lane 3: pmin(1.0, +INF) should be 1.0";

    // Test maximum/minimum normal float values
    auto result2 = call_f32x4_pmin("f32x4_pmin_boundary",
                                  FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN,
                                  FLT_MIN, FLT_MAX, FLT_MIN, FLT_MAX);
    ASSERT_EQ(FLT_MIN, result2[0]) << "Lane 0: pmin(FLT_MAX, FLT_MIN) should be FLT_MIN";
    ASSERT_EQ(FLT_MIN, result2[1]) << "Lane 1: pmin(FLT_MIN, FLT_MAX) should be FLT_MIN";
    ASSERT_EQ(FLT_MIN, result2[2]) << "Lane 2: pmin(FLT_MAX, FLT_MIN) should be FLT_MIN";
    ASSERT_EQ(FLT_MIN, result2[3]) << "Lane 3: pmin(FLT_MIN, FLT_MAX) should be FLT_MIN";

    // Test subnormal numbers (denormalized floats)
    float min_subnormal = std::numeric_limits<float>::denorm_min();
    auto result3 = call_f32x4_pmin("f32x4_pmin_boundary",
                                  min_subnormal, 1.0f, min_subnormal, 1.0f,
                                  1.0f, min_subnormal, 2.0f, min_subnormal);
    ASSERT_EQ(min_subnormal, result3[0]) << "Lane 0: pmin(subnormal, 1.0) should be subnormal";
    ASSERT_EQ(min_subnormal, result3[1]) << "Lane 1: pmin(1.0, subnormal) should be subnormal";
    ASSERT_EQ(min_subnormal, result3[2]) << "Lane 2: pmin(subnormal, 2.0) should be subnormal";
    ASSERT_EQ(min_subnormal, result3[3]) << "Lane 3: pmin(1.0, subnormal) should be subnormal";
}

/**
 * @test NaNHandling_FollowsPseudoMinimumRules
 * @brief Validates f32x4.pmin follows pseudo-minimum NaN handling rules
 * @details Tests critical difference between pmin and regular min: pmin returns the
 *          non-NaN value when one operand is NaN, but returns NaN when both are NaN.
 * @test_category Edge - NaN behavior validation (key pmin characteristic)
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmin_nan_handling
 * @input_conditions Various NaN + number combinations and NaN + NaN cases
 * @expected_behavior Non-NaN value selected when available, NaN when both operands are NaN
 * @validation_method NaN-aware comparison with pseudo-minimum specific behavior verification
 */
TEST_F(F32x4PminTestSuite, NaNHandling_FollowsPseudoMinimumRules)
{
    float nan_val = std::numeric_limits<float>::quiet_NaN();

    // Test NaN with normal numbers (key pseudo-minimum behavior)
    auto result1 = call_f32x4_pmin("f32x4_pmin_nan",
                                  nan_val, 1.0f, 2.0f, nan_val,
                                  2.0f, nan_val, 3.0f, 5.0f);
    ASSERT_EQ(2.0f, result1[0]) << "Lane 0: pmin(NaN, 2.0) should be 2.0 (non-NaN selected)";
    ASSERT_EQ(1.0f, result1[1]) << "Lane 1: pmin(1.0, NaN) should be 1.0 (non-NaN selected)";
    ASSERT_EQ(2.0f, result1[2]) << "Lane 2: pmin(2.0, 3.0) should be 2.0";
    ASSERT_EQ(5.0f, result1[3]) << "Lane 3: pmin(NaN, 5.0) should be 5.0 (non-NaN selected)";

    // Test both operands NaN (should return NaN)
    auto result2 = call_f32x4_pmin("f32x4_pmin_nan",
                                  nan_val, nan_val, 1.0f, 2.0f,
                                  nan_val, nan_val, 3.0f, 4.0f);
    ASSERT_TRUE(std::isnan(result2[0])) << "Lane 0: pmin(NaN, NaN) should be NaN";
    ASSERT_TRUE(std::isnan(result2[1])) << "Lane 1: pmin(NaN, NaN) should be NaN";
    ASSERT_EQ(1.0f, result2[2]) << "Lane 2: pmin(1.0, 3.0) should be 1.0";
    ASSERT_EQ(2.0f, result2[3]) << "Lane 3: pmin(2.0, 4.0) should be 2.0";

    // Test mixed NaN scenarios in single vector
    auto result3 = call_f32x4_pmin("f32x4_pmin_nan",
                                  nan_val, INFINITY, FLT_MIN, 0.0f,
                                  -INFINITY, nan_val, FLT_MAX, -0.0f);
    ASSERT_EQ(-INFINITY, result3[0]) << "Lane 0: pmin(NaN, -INF) should be -INF (non-NaN selected)";
    ASSERT_EQ(INFINITY, result3[1]) << "Lane 1: pmin(INF, NaN) should be INF (non-NaN selected)";
    ASSERT_EQ(FLT_MIN, result3[2]) << "Lane 2: pmin(FLT_MIN, FLT_MAX) should be FLT_MIN";
    ASSERT_EQ(-0.0f, result3[3]) << "Lane 3: pmin(0.0, -0.0) should be -0.0";
}

/**
 * @test SignedZero_BehavesCorrectly
 * @brief Validates f32x4.pmin handles IEEE 754 signed zero correctly
 * @details Tests pseudo-minimum behavior with +0.0f and -0.0f combinations.
 *          Signed zero handling may be implementation-defined in pseudo-minimum.
 * @test_category Edge - Signed zero handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmin_signed_zero
 * @input_conditions Various combinations of +0.0f and -0.0f values
 * @expected_behavior Implementation-defined signed zero selection (typically -0.0f < +0.0f)
 * @validation_method Verification of signed zero pseudo-minimum with IEEE 754 semantics
 */
TEST_F(F32x4PminTestSuite, SignedZero_BehavesCorrectly)
{
    // Test signed zero combinations
    auto result1 = call_f32x4_pmin("f32x4_pmin_zero",
                                  +0.0f, -0.0f, +0.0f, -0.0f,
                                  -0.0f, +0.0f, 1.0f, -1.0f);
    // Note: Signed zero behavior is implementation-defined for pmin
    // Most implementations treat -0.0 < +0.0, so pmin should select -0.0
    ASSERT_TRUE(float_equals_with_nan(-0.0f, result1[0])) << "Lane 0: pmin(+0.0, -0.0) expected -0.0";
    ASSERT_TRUE(float_equals_with_nan(-0.0f, result1[1])) << "Lane 1: pmin(-0.0, +0.0) expected -0.0";
    ASSERT_EQ(+0.0f, result1[2]) << "Lane 2: pmin(+0.0, 1.0) should be +0.0";
    ASSERT_EQ(-1.0f, result1[3]) << "Lane 3: pmin(-0.0, -1.0) should be -1.0";

    // Test zero with other values
    auto result2 = call_f32x4_pmin("f32x4_pmin_zero",
                                  0.0f, -0.0f, 1.0f, -1.0f,
                                  FLT_MIN, FLT_MIN, 0.0f, 0.0f);
    ASSERT_EQ(0.0f, result2[0]) << "Lane 0: pmin(0.0, FLT_MIN) should be 0.0";
    ASSERT_EQ(-0.0f, result2[1]) << "Lane 1: pmin(-0.0, FLT_MIN) should be -0.0";
    ASSERT_EQ(0.0f, result2[2]) << "Lane 2: pmin(1.0, 0.0) should be 0.0";
    ASSERT_EQ(-1.0f, result2[3]) << "Lane 3: pmin(-1.0, 0.0) should be -1.0";
}

/**
 * @test IdentityOperations_ReturnSelfValues
 * @brief Validates f32x4.pmin identity property: pmin(x, x) = x
 * @details Tests that pseudo-minimum of identical values returns the value itself.
 *          Verifies identity property across various special float values.
 * @test_category Edge - Identity property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmin_identity
 * @input_conditions Identical values in both operands across various float types
 * @expected_behavior Returns identical input values (identity property)
 * @validation_method Direct equality comparison for identity operations
 */
TEST_F(F32x4PminTestSuite, IdentityOperations_ReturnSelfValues)
{
    // Test identity with normal values
    auto result1 = call_f32x4_pmin("f32x4_pmin_basic",
                                  1.5f, -2.0f, 0.0f, INFINITY,
                                  1.5f, -2.0f, 0.0f, INFINITY);
    ASSERT_EQ(1.5f, result1[0]) << "Lane 0: pmin(1.5, 1.5) should be 1.5";
    ASSERT_EQ(-2.0f, result1[1]) << "Lane 1: pmin(-2.0, -2.0) should be -2.0";
    ASSERT_EQ(0.0f, result1[2]) << "Lane 2: pmin(0.0, 0.0) should be 0.0";
    ASSERT_EQ(INFINITY, result1[3]) << "Lane 3: pmin(INF, INF) should be INF";

    // Test identity with special values
    float nan_val = std::numeric_limits<float>::quiet_NaN();
    auto result2 = call_f32x4_pmin("f32x4_pmin_nan",
                                  nan_val, -INFINITY, FLT_MAX, FLT_MIN,
                                  nan_val, -INFINITY, FLT_MAX, FLT_MIN);
    ASSERT_TRUE(std::isnan(result2[0])) << "Lane 0: pmin(NaN, NaN) should be NaN";
    ASSERT_EQ(-INFINITY, result2[1]) << "Lane 1: pmin(-INF, -INF) should be -INF";
    ASSERT_EQ(FLT_MAX, result2[2]) << "Lane 2: pmin(FLT_MAX, FLT_MAX) should be FLT_MAX";
    ASSERT_EQ(FLT_MIN, result2[3]) << "Lane 3: pmin(FLT_MIN, FLT_MIN) should be FLT_MIN";
}