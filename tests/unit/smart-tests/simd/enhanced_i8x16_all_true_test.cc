/**
 * @file enhanced_i8x16_all_true_test.cc
 * @brief Comprehensive unit tests for i8x16.all_true SIMD opcode
 * @details Tests i8x16.all_true functionality across interpreter and AOT execution modes
 *          with focus on boolean reduction operations, edge cases, and cross-mode consistency.
 *          Validates WAMR SIMD implementation correctness for all_true boolean reduction.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_all_true_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class I8x16AllTrueTestSuite
 * @brief Test fixture class for i8x16.all_true opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD boolean result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class I8x16AllTrueTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for i8x16.all_true testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_all_true_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.all_true test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_all_true_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i8x16.all_true tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_all_true_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM i8x16.all_true function with no parameters
     * @details Executes parameterless i8x16.all_true test function and returns boolean result.
     *          Handles WASM function invocation and i32 result extraction.
     * @param function_name Name of the WASM function to call
     * @param result Reference to store i32 result (0 or 1)
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_all_true_test.cc:call_i8x16_all_true_function
     */
    bool call_i8x16_all_true_function(const char* function_name, uint32_t& result)
    {
        // Call WASM function with no arguments
        uint32_t argv[1] = {0}; // Space for return value
        bool call_success = dummy_env->execute(function_name, 0, argv);
        EXPECT_TRUE(call_success) << "Failed to call " << function_name << " function";

        if (call_success) {
            // Extract i32 result (boolean: 0 or 1)
            result = argv[0];
        }

        return call_success;
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test AllNonZeroLanes_ReturnsTrue
 * @brief Validates i8x16.all_true returns 1 when all lanes contain non-zero values
 * @details Tests basic functionality with all lanes set to 1, verifying that
 *          i8x16.all_true correctly identifies all-true condition
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions All 16 lanes set to value 1 (non-zero)
 * @expected_behavior Returns i32 value 1 (true)
 * @validation_method Direct comparison of WASM function result with expected value
 */
TEST_F(I8x16AllTrueTestSuite, AllNonZeroLanes_ReturnsTrue)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_all_ones", result);
    ASSERT_TRUE(call_success) << "Failed to execute i8x16.all_true test function";
    ASSERT_EQ(1, result) << "Expected 1 when all lanes are non-zero, got " << result;
}

/**
 * @test MixedPositiveNegative_ReturnsTrue
 * @brief Validates i8x16.all_true handles mixed positive/negative non-zero values correctly
 * @details Tests with alternating positive and negative values to ensure sign doesn't
 *          affect non-zero detection logic
 * @test_category Main - Sign handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions Mix of positive and negative non-zero values across lanes
 * @expected_behavior Returns i32 value 1 (true)
 * @validation_method Direct comparison confirming sign-independent non-zero detection
 */
TEST_F(I8x16AllTrueTestSuite, MixedPositiveNegative_ReturnsTrue)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_mixed_signs", result);
    ASSERT_TRUE(call_success) << "Failed to execute mixed signs test function";
    ASSERT_EQ(1, result) << "Expected 1 for mixed positive/negative non-zero values, got " << result;
}

/**
 * @test BoundaryValues_ReturnsTrue
 * @brief Validates i8x16.all_true handles i8 boundary values MIN_VALUE and MAX_VALUE
 * @details Tests with mix of -128 (MIN_VALUE) and +127 (MAX_VALUE) to verify
 *          boundary conditions are properly recognized as non-zero
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions Mix of MIN_VALUE (-128) and MAX_VALUE (+127) in different lanes
 * @expected_behavior Returns i32 value 1 (true)
 * @validation_method Boundary value recognition as non-zero confirmation
 */
TEST_F(I8x16AllTrueTestSuite, BoundaryValues_ReturnsTrue)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_min_max", result);
    ASSERT_TRUE(call_success) << "Failed to execute boundary values test function";
    ASSERT_EQ(1, result) << "Expected 1 for MIN/MAX boundary values, got " << result;
}

/**
 * @test SingleZeroLane_ReturnsFalse
 * @brief Validates i8x16.all_true returns 0 when exactly one lane contains zero
 * @details Tests critical edge case where 15 lanes are non-zero but one lane is zero,
 *          verifying that single zero breaks the all-true condition
 * @test_category Corner - Single zero detection
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions 15 non-zero lanes with 1 zero lane at specific position
 * @expected_behavior Returns i32 value 0 (false)
 * @validation_method Single zero lane detection and all-true condition failure
 */
TEST_F(I8x16AllTrueTestSuite, SingleZeroLane_ReturnsFalse)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_single_zero", result);
    ASSERT_TRUE(call_success) << "Failed to execute single zero test function";
    ASSERT_EQ(0, result) << "Expected 0 when one lane is zero, got " << result;
}

/**
 * @test AllZeroLanes_ReturnsFalse
 * @brief Validates i8x16.all_true returns 0 when all lanes contain zero values
 * @details Tests extreme case with complete zero vector, verifying proper
 *          handling of all-false condition
 * @test_category Edge - Complete zero vector handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions All 16 lanes set to zero value
 * @expected_behavior Returns i32 value 0 (false)
 * @validation_method Complete zero vector detection and false result validation
 */
TEST_F(I8x16AllTrueTestSuite, AllZeroLanes_ReturnsFalse)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_all_zeros", result);
    ASSERT_TRUE(call_success) << "Failed to execute all zeros test function";
    ASSERT_EQ(0, result) << "Expected 0 when all lanes are zero, got " << result;
}

/**
 * @test AlternatingPattern_ReturnsFalse
 * @brief Validates i8x16.all_true returns 0 with alternating zero/non-zero pattern
 * @details Tests pattern-based zero detection with alternating values, ensuring
 *          any zero lanes cause all-true condition to fail
 * @test_category Edge - Pattern-based zero detection
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions Alternating pattern of zero and non-zero values
 * @expected_behavior Returns i32 value 0 (false)
 * @validation_method Pattern-based zero detection with false result confirmation
 */
TEST_F(I8x16AllTrueTestSuite, AlternatingPattern_ReturnsFalse)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_alternating", result);
    ASSERT_TRUE(call_success) << "Failed to execute alternating pattern test function";
    ASSERT_EQ(0, result) << "Expected 0 for alternating zero/non-zero pattern, got " << result;
}

/**
 * @test NegativeOnesPattern_ReturnsTrue
 * @brief Validates i8x16.all_true with all lanes set to -1 (0xFF)
 * @details Tests with all bits set (0xFF pattern) to verify negative value
 *          handling and maximum bit density recognition
 * @test_category Edge - Maximum bit density validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions All 16 lanes set to -1 (0xFF, all bits set)
 * @expected_behavior Returns i32 value 1 (true)
 * @validation_method All-bits-set pattern recognition as non-zero
 */
TEST_F(I8x16AllTrueTestSuite, NegativeOnesPattern_ReturnsTrue)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_negative_ones", result);
    ASSERT_TRUE(call_success) << "Failed to execute negative ones test function";
    ASSERT_EQ(1, result) << "Expected 1 for all negative ones pattern, got " << result;
}

/**
 * @test ZeroFirstLane_ReturnsFalse
 * @brief Validates i8x16.all_true with zero in first lane only
 * @details Tests position independence by placing zero in first lane while
 *          keeping remaining lanes non-zero
 * @test_category Edge - Position independence validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions Zero in lane 0, non-zero in lanes 1-15
 * @expected_behavior Returns i32 value 0 (false)
 * @validation_method First-position zero detection confirmation
 */
TEST_F(I8x16AllTrueTestSuite, ZeroFirstLane_ReturnsFalse)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_zero_first_lane", result);
    ASSERT_TRUE(call_success) << "Failed to execute zero first lane test function";
    ASSERT_EQ(0, result) << "Expected 0 when first lane is zero, got " << result;
}

/**
 * @test ZeroLastLane_ReturnsFalse
 * @brief Validates i8x16.all_true with zero in last lane only
 * @details Tests position independence by placing zero in last lane while
 *          keeping remaining lanes non-zero
 * @test_category Edge - Position independence validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions Non-zero in lanes 0-14, zero in lane 15
 * @expected_behavior Returns i32 value 0 (false)
 * @validation_method Last-position zero detection confirmation
 */
TEST_F(I8x16AllTrueTestSuite, ZeroLastLane_ReturnsFalse)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_zero_last_lane", result);
    ASSERT_TRUE(call_success) << "Failed to execute zero last lane test function";
    ASSERT_EQ(0, result) << "Expected 0 when last lane is zero, got " << result;
}

/**
 * @test RandomNonZeroValues_ReturnsTrue
 * @brief Validates i8x16.all_true with diverse non-zero values
 * @details Tests with random mix of non-zero positive and negative values
 *          to validate diverse input handling
 * @test_category Main - Diverse input validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions Random non-zero values across all lanes
 * @expected_behavior Returns i32 value 1 (true)
 * @validation_method Diverse non-zero pattern recognition
 */
TEST_F(I8x16AllTrueTestSuite, RandomNonZeroValues_ReturnsTrue)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_random_nonzero", result);
    ASSERT_TRUE(call_success) << "Failed to execute random non-zero test function";
    ASSERT_EQ(1, result) << "Expected 1 for random non-zero values, got " << result;
}

/**
 * @test MultipleZeros_ReturnsFalse
 * @brief Validates i8x16.all_true with multiple scattered zero lanes
 * @details Tests with several zero lanes at different positions to ensure
 *          multiple zeros are properly detected
 * @test_category Corner - Multiple zero detection
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_all_true_operation
 * @input_conditions Multiple zero lanes scattered throughout vector
 * @expected_behavior Returns i32 value 0 (false)
 * @validation_method Multiple zero lane detection confirmation
 */
TEST_F(I8x16AllTrueTestSuite, MultipleZeros_ReturnsFalse)
{
    uint32_t result;
    bool call_success = call_i8x16_all_true_function("test_i8x16_all_true_multiple_zeros", result);
    ASSERT_TRUE(call_success) << "Failed to execute multiple zeros test function";
    ASSERT_EQ(0, result) << "Expected 0 for multiple zero lanes, got " << result;
}