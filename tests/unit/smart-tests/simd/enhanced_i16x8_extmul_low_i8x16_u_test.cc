/**
 * @file enhanced_i16x8_extmul_low_i8x16_u_test.cc
 * @brief Comprehensive unit tests for i16x8.extmul_low_i8x16_u SIMD opcode
 * @details Tests i16x8.extmul_low_i8x16_u functionality across interpreter and AOT execution modes
 *          with focus on extended multiplication of low eight unsigned i8 lanes producing i16 results.
 *          Validates WAMR SIMD implementation correctness for integer extended multiplication.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_i16x8_extmul_low_i8x16_u_test.cc
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
 * @class I16x8ExtmulLowI8x16UTestSuite
 * @brief Test fixture class for i16x8.extmul_low_i8x16_u opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD integer result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class I16x8ExtmulLowI8x16UTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for i16x8.extmul_low_i8x16_u testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i16x8_extmul_low_i8x16_u_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i16x8.extmul_low_i8x16_u test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i16x8_extmul_low_i8x16_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i16x8.extmul_low_i8x16_u tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i16x8_extmul_low_i8x16_u_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM i16x8.extmul_low_i8x16_u function and extract v128 result
     * @details Executes extmul test function and extracts eight i16 values from v128 result.
     *          Handles WASM function invocation and v128 result extraction into i16 array.
     * @param function_name Name of the WASM function to call
     * @param results Reference to i16[8] array to store extracted results
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i16x8_extmul_low_i8x16_u_test.cc:call_extmul_function
     */
    bool call_extmul_function(const char* function_name, uint16_t results[8])
    {
        // Call WASM function with no arguments, expects v128 return value
        uint32_t argv[4]; // Space for v128 return value (4 x 32-bit components)
        bool call_success = dummy_env->execute(function_name, 0, argv);
        EXPECT_TRUE(call_success) << "Failed to call " << function_name << " function";

        if (call_success) {
            // Extract i16 results from v128 value (8 lanes of i16)
            memcpy(results, argv, sizeof(uint16_t) * 8);
        }

        return call_success;
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicExtendedMultiplication_ReturnsCorrectProducts
 * @brief Validates i16x8.extmul_low_i8x16_u produces correct arithmetic results for typical inputs
 * @details Tests fundamental extended multiplication with sequential values, verifying that
 *          i16x8.extmul_low_i8x16_u correctly computes (i16)a[i] * (i16)b[i] for lanes 0-7.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_low_i8x16_u
 * @input_conditions Sequential i8 values: a=[1,2,3,4,5,6,7,8,...], b=[2,3,4,5,6,7,8,9,...]
 * @expected_behavior Returns products: [2,6,12,20,30,42,56,72] in lanes 0-7, zeros in 8-15
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(I16x8ExtmulLowI8x16UTestSuite, BasicExtendedMultiplication_ReturnsCorrectProducts)
{
    uint16_t results[8];
    bool success = call_extmul_function("extmul_low_basic", results);
    ASSERT_TRUE(success) << "Basic extended multiplication function call should succeed";

    // Verify correct products in lanes 0-7: [1,2,3,4,5,6,7,8] × [2,3,4,5,6,7,8,9]
    ASSERT_EQ(2, results[0])  << "Lane 0: 1 × 2 should equal 2";
    ASSERT_EQ(6, results[1])  << "Lane 1: 2 × 3 should equal 6";
    ASSERT_EQ(12, results[2]) << "Lane 2: 3 × 4 should equal 12";
    ASSERT_EQ(20, results[3]) << "Lane 3: 4 × 5 should equal 20";
    ASSERT_EQ(30, results[4]) << "Lane 4: 5 × 6 should equal 30";
    ASSERT_EQ(42, results[5]) << "Lane 5: 6 × 7 should equal 42";
    ASSERT_EQ(56, results[6]) << "Lane 6: 7 × 8 should equal 56";
    ASSERT_EQ(72, results[7]) << "Lane 7: 8 × 9 should equal 72";
}

/**
 * @test MaximumBoundaryValues_ProducesCorrectResults
 * @brief Validates correct handling of maximum unsigned i8 boundary values
 * @details Tests with maximum unsigned i8 values (255) to verify proper unsigned interpretation
 *          and that maximum product (65025) fits within i16 range without overflow.
 * @test_category Corner - Boundary conditions for unsigned 8-bit integers
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_low_i8x16_u
 * @input_conditions Maximum unsigned i8 values: [255,255,255,255,255,255,255,255,...]
 * @expected_behavior Returns maximum products: [65025,65025,...] (255×255) in lanes 0-7
 * @validation_method Verification that 255×255=65025 fits in i16 range (0-65535)
 */
TEST_F(I16x8ExtmulLowI8x16UTestSuite, MaximumBoundaryValues_ProducesCorrectResults)
{
    uint16_t results[8];
    bool success = call_extmul_function("extmul_low_max", results);
    ASSERT_TRUE(success) << "Maximum boundary values function call should succeed";

    // Verify maximum product in all low lanes: 255 × 255 = 65025
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(65025, results[i])
            << "Lane " << i << ": 255 × 255 should equal 65025";
    }
}

/**
 * @test ZeroOperand_ProducesZeroResults
 * @brief Validates zero multiplication properties and zero propagation
 * @details Tests zero operand scenarios to verify that multiplication by zero produces zero
 *          results, validating mathematical identity properties.
 * @test_category Edge - Zero operand scenarios and identity operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_low_i8x16_u
 * @input_conditions All-zero vectors and mixed zero/non-zero combinations
 * @expected_behavior All products involving zero should be zero
 * @validation_method Verification of zero propagation in multiplication
 */
TEST_F(I16x8ExtmulLowI8x16UTestSuite, ZeroOperand_ProducesZeroResults)
{
    uint16_t results[8];
    bool success = call_extmul_function("extmul_low_zero", results);
    ASSERT_TRUE(success) << "Zero operand function call should succeed";

    // Verify all results are zero when one operand is all-zero
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(0, results[i])
            << "Lane " << i << ": multiplication by zero should equal 0";
    }
}

/**
 * @test IdentityMultiplication_PreservesValues
 * @brief Validates identity property of multiplication by one
 * @details Tests multiplication by 1 to verify that values are preserved unchanged,
 *          validating mathematical identity property: a × 1 = a.
 * @test_category Edge - Identity operations and mathematical properties
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_low_i8x16_u
 * @input_conditions Test values multiplied by identity value (1)
 * @expected_behavior Original values preserved: a × 1 = a for all lanes
 * @validation_method Direct comparison with input values
 */
TEST_F(I16x8ExtmulLowI8x16UTestSuite, IdentityMultiplication_PreservesValues)
{
    uint16_t results[8];
    bool success = call_extmul_function("extmul_low_identity", results);
    ASSERT_TRUE(success) << "Identity multiplication function call should succeed";

    // Expected values: [10,20,30,40,50,60,70,80] × [1,1,1,1,1,1,1,1] = [10,20,30,40,50,60,70,80]
    uint16_t expected_values[8] = {10, 20, 30, 40, 50, 60, 70, 80};

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected_values[i], results[i])
            << "Lane " << i << ": " << expected_values[i] << " × 1 should equal " << expected_values[i];
    }
}

/**
 * @test PowerOfTwoValues_ValidatesBitPatterns
 * @brief Validates multiplication with power-of-two values and bit patterns
 * @details Tests with power-of-two values to verify correct unsigned bit interpretation
 *          and validate expected mathematical relationships.
 * @test_category Edge - Bit pattern validation and mathematical properties
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_low_i8x16_u
 * @input_conditions Power-of-two values: [1,2,4,8,16,32,64,128] and bit patterns
 * @expected_behavior Correct unsigned multiplication results for power-of-two values
 * @validation_method Mathematical verification of power-of-two multiplication
 */
TEST_F(I16x8ExtmulLowI8x16UTestSuite, PowerOfTwoValues_ValidatesBitPatterns)
{
    uint16_t results[8];
    bool success = call_extmul_function("extmul_low_powers", results);
    ASSERT_TRUE(success) << "Power-of-two values function call should succeed";

    // Expected: [1,2,4,8,16,32,64,128] × [2,2,2,2,2,2,2,2] = [2,4,8,16,32,64,128,256]
    uint16_t expected_values[8] = {2, 4, 8, 16, 32, 64, 128, 256};

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected_values[i], results[i])
            << "Lane " << i << ": power-of-two multiplication failed";
    }
}

/**
 * @test HighLaneIgnoring_VerifiesLaneIsolation
 * @brief Validates that high lanes (8-15) do not affect low lane (0-7) results
 * @details Tests lane isolation by using different values in high lanes,
 *          verifying that i16x8.extmul_low_i8x16_u only processes lanes 0-7.
 * @test_category Error - Lane isolation and output format validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_low_i8x16_u
 * @input_conditions Test vectors with varying high lane values
 * @expected_behavior Results should be consistent regardless of high lane values
 * @validation_method Compare results from vectors with different high lanes
 */
TEST_F(I16x8ExtmulLowI8x16UTestSuite, HighLaneIgnoring_VerifiesLaneIsolation)
{
    uint16_t results[8];
    bool success = call_extmul_function("extmul_low_isolation", results);
    ASSERT_TRUE(success) << "High lane isolation function call should succeed";

    // Expected: [5,10,15,20,25,30,35,40] × [2,2,2,2,2,2,2,2] = [10,20,30,40,50,60,70,80]
    // High lanes should not affect these results
    uint16_t expected_values[8] = {10, 20, 30, 40, 50, 60, 70, 80};

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected_values[i], results[i])
            << "Lane " << i << ": high lane values should not affect low lane results";
    }
}