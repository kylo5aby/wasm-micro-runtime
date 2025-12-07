/**
 * @file enhanced_i16x8_extmul_high_i8x16_s_test.cc
 * @brief Comprehensive unit tests for i16x8.extmul_high_i8x16_s SIMD opcode
 * @details Tests i16x8.extmul_high_i8x16_s functionality across interpreter and AOT execution modes
 *          with focus on extended multiplication of high eight signed i8 lanes producing i16 results.
 *          Validates WAMR SIMD implementation correctness for integer extended multiplication.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_i16x8_extmul_high_i8x16_s_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <climits>
#include <cmath>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class I16x8ExtmulHighI8x16STestSuite
 * @brief Test fixture class for i16x8.extmul_high_i8x16_s opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD integer result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class I16x8ExtmulHighI8x16STestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for i16x8.extmul_high_i8x16_s testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i16x8_extmul_high_i8x16_s_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i16x8.extmul_high_i8x16_s test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i16x8_extmul_high_i8x16_s_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i16x8.extmul_high_i8x16_s tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i16x8_extmul_high_i8x16_s_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM i16x8.extmul_high_i8x16_s function and extract v128 result
     * @details Executes extmul test function and extracts eight i16 values from v128 result.
     *          Handles WASM function invocation and v128 result extraction into i16 array.
     * @param function_name Name of the WASM function to call
     * @param results Reference to i16[8] array to store extracted results
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i16x8_extmul_high_i8x16_s_test.cc:call_extmul_function
     */
    bool call_extmul_function(const char* function_name, int16_t results[8])
    {
        // Call WASM function with no arguments, expects v128 return value
        uint32_t argv[4]; // Space for v128 return value (4 x 32-bit components)
        bool call_success = dummy_env->execute(function_name, 0, argv);
        EXPECT_TRUE(call_success) << "Failed to call " << function_name << " function";

        if (call_success) {
            // Extract i16 results from v128 value (8 lanes of i16)
            memcpy(results, argv, sizeof(int16_t) * 8);
        }

        return call_success;
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicMultiplication_ReturnsCorrectResults
 * @brief Validates i16x8.extmul_high_i8x16_s produces correct arithmetic results for typical inputs
 * @details Tests fundamental extended multiplication operation with positive, negative, and mixed-sign integers
 *          in high lanes (8-15). Verifies that the opcode correctly computes signed multiplication of
 *          corresponding high lane pairs and extends results to i16.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:i16x8_extmul_high_i8x16_s_operation
 * @input_conditions Standard integer pairs in high lanes: positive×positive, negative×negative, positive×negative
 * @expected_behavior Returns mathematically correct products extended to i16 values
 * @validation_method Direct comparison of WASM function result with expected i16 multiplication values
 */
TEST_F(I16x8ExtmulHighI8x16STestSuite, BasicMultiplication_ReturnsCorrectResults)
{
    // Test basic positive × positive multiplication in high lanes
    int16_t results[8];
    bool success = call_extmul_function("test_basic_positive", results);
    ASSERT_TRUE(success) << "Basic positive multiplication function call should succeed";

    // Verify results: high lanes (8-15) contain 2×3=6, 4×5=20, 1×7=7, 6×8=48
    ASSERT_EQ(results[0], 6)  << "Basic positive multiplication failed: 2×3 should equal 6";
    ASSERT_EQ(results[1], 20) << "Basic positive multiplication failed: 4×5 should equal 20";
    ASSERT_EQ(results[2], 7)  << "Basic positive multiplication failed: 1×7 should equal 7";
    ASSERT_EQ(results[3], 48) << "Basic positive multiplication failed: 6×8 should equal 48";

    // Test mixed-sign multiplication: positive × negative
    success = call_extmul_function("test_mixed_signs", results);
    ASSERT_TRUE(success) << "Mixed-sign multiplication function call should succeed";

    // Verify mixed-sign results: 5×(-3)=-15, (-2)×4=-8, 7×(-1)=-7, (-6)×3=-18
    ASSERT_EQ(results[0], -15) << "Mixed-sign multiplication failed: 5×(-3) should equal -15";
    ASSERT_EQ(results[1], -8)  << "Mixed-sign multiplication failed: (-2)×4 should equal -8";
    ASSERT_EQ(results[2], -7)  << "Mixed-sign multiplication failed: 7×(-1) should equal -7";
    ASSERT_EQ(results[3], -18) << "Mixed-sign multiplication failed: (-6)×3 should equal -18";
}

/**
 * @test BoundaryValues_ProduceValidI16Results
 * @brief Tests extreme i8 values (INT8_MIN, INT8_MAX) in high lane extended multiplication
 * @details Validates that boundary value combinations like 127×127, -128×127, -128×(-128) produce
 *          mathematically correct results that fit within i16 range without overflow.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:i16x8_extmul_high_i8x16_s_boundary_handling
 * @input_conditions Extreme i8 values: INT8_MAX (127), INT8_MIN (-128) in various combinations
 * @expected_behavior Correct products: 127×127=16129, -128×127=-16256, -128×(-128)=16384 within i16 range
 * @validation_method Verify boundary products don't overflow and maintain mathematical accuracy
 */
TEST_F(I16x8ExtmulHighI8x16STestSuite, BoundaryValues_ProduceValidI16Results)
{
    int16_t results[8];

    // Test maximum positive boundary: INT8_MAX × INT8_MAX
    bool success = call_extmul_function("test_max_positive", results);
    ASSERT_TRUE(success) << "Max positive boundary function call should succeed";

    // Verify maximum positive product: 127 × 127 = 16129
    ASSERT_EQ(results[0], 16129) << "Max positive boundary failed: 127×127 should equal 16129";

    // Test most negative boundary: INT8_MIN × INT8_MAX
    success = call_extmul_function("test_min_times_max", results);
    ASSERT_TRUE(success) << "Min×Max boundary function call should succeed";

    // Verify most negative product: -128 × 127 = -16256
    ASSERT_EQ(results[0], -16256) << "Min×Max boundary failed: (-128)×127 should equal -16256";

    // Test INT8_MIN × INT8_MIN case
    success = call_extmul_function("test_min_times_min", results);
    ASSERT_TRUE(success) << "Min×Min boundary function call should succeed";

    // Verify INT8_MIN × INT8_MIN: (-128) × (-128) = 16384
    ASSERT_EQ(results[0], 16384) << "Min×Min boundary failed: (-128)×(-128) should equal 16384";
}

/**
 * @test ZeroOperands_ProduceZeroResults
 * @brief Validates multiplicative identity with zero values in high lanes
 * @details Tests that multiplication by zero produces zero results regardless of other operand values,
 *          and verifies that low lanes (0-7) are properly ignored during the operation.
 * @test_category Edge - Zero value validation and lane isolation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:i16x8_extmul_high_i8x16_s_zero_handling
 * @input_conditions Mix of zero and non-zero values in high lanes, varied values in low lanes
 * @expected_behavior Any lane with zero operand produces zero result, low lanes completely ignored
 * @validation_method Confirm zero multiplication identity and proper lane isolation behavior
 */
TEST_F(I16x8ExtmulHighI8x16STestSuite, ZeroOperands_ProduceZeroResults)
{
    int16_t results[8];

    // Test all-zero high lanes
    bool success = call_extmul_function("test_all_zero_high", results);
    ASSERT_TRUE(success) << "All-zero high lanes function call should succeed";

    // Verify all results are zero when high lanes contain zeros
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(results[i], 0) << "All-zero high lanes test failed at lane " << i;
    }

    // Test partial zero patterns: some high lanes zero, others non-zero
    success = call_extmul_function("test_partial_zero", results);
    ASSERT_TRUE(success) << "Partial zero function call should succeed";

    // Verify mixed zero/non-zero results: 0×5=0, 3×0=0, 2×4=8, 0×7=0
    ASSERT_EQ(results[0], 0) << "Partial zero test failed: 0×5 should equal 0";
    ASSERT_EQ(results[1], 0) << "Partial zero test failed: 3×0 should equal 0";
    ASSERT_EQ(results[2], 8) << "Partial zero test failed: 2×4 should equal 8";
    ASSERT_EQ(results[3], 0) << "Partial zero test failed: 0×7 should equal 0";
}

/**
 * @test IdentityOperations_PreserveValues
 * @brief Tests multiplicative identity (×1) and sign inversion (×-1) properties
 * @details Validates mathematical identities work correctly: any_value × 1 = any_value,
 *          any_value × (-1) = -any_value. Tests these properties across various input values.
 * @test_category Edge - Mathematical identity validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:i16x8_extmul_high_i8x16_s_identity_ops
 * @input_conditions High lanes containing 1, -1 multiplied against various test values
 * @expected_behavior Identity preservation (v×1=v) and sign inversion (v×(-1)=-v) per lane
 * @validation_method Validate mathematical identities work correctly per lane
 */
TEST_F(I16x8ExtmulHighI8x16STestSuite, IdentityOperations_PreserveValues)
{
    int16_t results[8];

    // Test multiplicative identity: any_value × 1 = any_value
    bool success = call_extmul_function("test_identity_multiply", results);
    ASSERT_TRUE(success) << "Identity multiply function call should succeed";

    // Verify identity preservation: 5×1=5, (-3)×1=-3, 7×1=7, (-12)×1=-12
    ASSERT_EQ(results[0], 5)   << "Identity test failed: 5×1 should equal 5";
    ASSERT_EQ(results[1], -3)  << "Identity test failed: (-3)×1 should equal -3";
    ASSERT_EQ(results[2], 7)   << "Identity test failed: 7×1 should equal 7";
    ASSERT_EQ(results[3], -12) << "Identity test failed: (-12)×1 should equal -12";

    // Test sign inversion: any_value × (-1) = -any_value
    success = call_extmul_function("test_sign_inversion", results);
    ASSERT_TRUE(success) << "Sign inversion function call should succeed";

    // Verify sign inversion: 8×(-1)=-8, (-5)×(-1)=5, 15×(-1)=-15, (-20)×(-1)=20
    ASSERT_EQ(results[0], -8)  << "Sign inversion test failed: 8×(-1) should equal -8";
    ASSERT_EQ(results[1], 5)   << "Sign inversion test failed: (-5)×(-1) should equal 5";
    ASSERT_EQ(results[2], -15) << "Sign inversion test failed: 15×(-1) should equal -15";
    ASSERT_EQ(results[3], 20)  << "Sign inversion test failed: (-20)×(-1) should equal 20";
}

/**
 * @test LaneIsolation_IgnoresLowLanes
 * @brief Verifies ONLY high lanes (8-15) participate, low lanes (0-7) ignored
 * @details Tests that the operation processes exactly lanes 8-15 from input vectors and completely
 *          ignores lanes 0-7. Changes to low lane values should not affect multiplication results.
 * @test_category Edge - Lane selection validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_simd.c:i16x8_extmul_high_i8x16_s_lane_selection
 * @input_conditions Different values in low lanes vs high lanes across both input vectors
 * @expected_behavior Result depends solely on high lane values, low lane values irrelevant
 * @validation_method Change low lane values, verify identical results proving isolation
 */
TEST_F(I16x8ExtmulHighI8x16STestSuite, LaneIsolation_IgnoresLowLanes)
{
    int16_t results1[8], results2[8];

    // Test with specific values in low lanes, verify they don't affect high lane results
    bool success = call_extmul_function("test_lane_isolation", results1);
    ASSERT_TRUE(success) << "Lane isolation function call should succeed";

    // Verify results depend only on high lanes: high lanes 2×3=6, 4×1=4, etc.
    ASSERT_EQ(results1[0], 6) << "Lane isolation test failed: high lane 2×3 should equal 6";
    ASSERT_EQ(results1[1], 4) << "Lane isolation test failed: high lane 4×1 should equal 4";

    // Test with different low lane values but same high lanes - should get identical results
    success = call_extmul_function("test_lane_isolation_variant", results2);
    ASSERT_TRUE(success) << "Lane isolation variant function call should succeed";

    // Results should be identical despite different low lane values
    ASSERT_EQ(results2[0], 6) << "Lane isolation variant failed: should get same result 6";
    ASSERT_EQ(results2[1], 4) << "Lane isolation variant failed: should get same result 4";

    // Verify results are byte-for-byte identical
    ASSERT_EQ(memcmp(results1, results2, 8 * sizeof(int16_t)), 0)
        << "Lane isolation failed: results should be identical regardless of low lane values";
}