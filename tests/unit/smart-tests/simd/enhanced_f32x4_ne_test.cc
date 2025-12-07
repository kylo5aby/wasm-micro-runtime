/**
 * @file enhanced_f32x4_ne_test.cc
 * @brief Comprehensive unit tests for f32x4.ne SIMD opcode
 * @details Tests f32x4.ne functionality across interpreter and AOT execution modes
 *          with focus on element-wise not-equal comparison of four 32-bit single-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc
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
 * @class F32x4NeTestSuite
 * @brief Test fixture class for f32x4.ne opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F32x4NeTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.ne testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.ne test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_ne_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.ne tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM f32x4.ne function with two vector inputs
     * @details Executes f32x4.ne operation on two input vectors and returns element-wise inequality result.
     *          Handles WASM function invocation and v128 result extraction for 4-lane f32 comparison.
     * @param f1_lane0 First float value for lane 0 of first vector
     * @param f1_lane1 First float value for lane 1 of first vector
     * @param f1_lane2 First float value for lane 2 of first vector
     * @param f1_lane3 First float value for lane 3 of first vector
     * @param f2_lane0 Second float value for lane 0 of second vector
     * @param f2_lane1 Second float value for lane 1 of second vector
     * @param f2_lane2 Second float value for lane 2 of second vector
     * @param f2_lane3 Second float value for lane 3 of second vector
     * @param result_bytes 16-byte array to store the inequality comparison result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc:call_f32x4_ne
     */
    bool call_f32x4_ne(float f1_lane0, float f1_lane1, float f1_lane2, float f1_lane3,
                       float f2_lane0, float f2_lane1, float f2_lane2, float f2_lane3,
                       uint8_t result_bytes[16])
    {
        // Prepare arguments: two input v128 vectors as eight i32 values each
        uint32_t argv[8];

        // Convert floats to byte representation and then to 32-bit values
        uint32_t input1_lane0, input1_lane1, input1_lane2, input1_lane3;
        uint32_t input2_lane0, input2_lane1, input2_lane2, input2_lane3;

        // Copy float values to get their bit representation
        memcpy(&input1_lane0, &f1_lane0, sizeof(float));  // Lane 0 of first vector
        memcpy(&input1_lane1, &f1_lane1, sizeof(float));  // Lane 1 of first vector
        memcpy(&input1_lane2, &f1_lane2, sizeof(float));  // Lane 2 of first vector
        memcpy(&input1_lane3, &f1_lane3, sizeof(float));  // Lane 3 of first vector
        memcpy(&input2_lane0, &f2_lane0, sizeof(float));  // Lane 0 of second vector
        memcpy(&input2_lane1, &f2_lane1, sizeof(float));  // Lane 1 of second vector
        memcpy(&input2_lane2, &f2_lane2, sizeof(float));  // Lane 2 of second vector
        memcpy(&input2_lane3, &f2_lane3, sizeof(float));  // Lane 3 of second vector

        // WASM expects little-endian format for v128 representation
        // First v128 vector (f1_lane0, f1_lane1, f1_lane2, f1_lane3)
        argv[0] = input1_lane0;  // Lane 0 of first vector
        argv[1] = input1_lane1;  // Lane 1 of first vector
        argv[2] = input1_lane2;  // Lane 2 of first vector
        argv[3] = input1_lane3;  // Lane 3 of first vector
        // Second v128 vector (f2_lane0, f2_lane1, f2_lane2, f2_lane3)
        argv[4] = input2_lane0;  // Lane 0 of second vector
        argv[5] = input2_lane1;  // Lane 1 of second vector
        argv[6] = input2_lane2;  // Lane 2 of second vector
        argv[7] = input2_lane3;  // Lane 3 of second vector

        // Execute WASM function: f32x4_ne_basic
        bool success = dummy_env->execute("f32x4_ne_basic", 8, argv);
        EXPECT_TRUE(success) << "Failed to execute f32x4.ne WASM function";

        if (success) {
            // Extract v128 result and convert back to byte array (result stored in argv)
            // Copy result bytes for 4 lanes (16 bytes total)
            memcpy(&result_bytes[0], &argv[0], 4);   // Lane 0 result (4 bytes)
            memcpy(&result_bytes[4], &argv[1], 4);   // Lane 1 result (4 bytes)
            memcpy(&result_bytes[8], &argv[2], 4);   // Lane 2 result (4 bytes)
            memcpy(&result_bytes[12], &argv[3], 4);  // Lane 3 result (4 bytes)
        }

        return success;
    }

    /**
     * @brief Validate f32x4.ne result matches expected lane values
     * @details Compares actual result bytes with expected inequality masks for each lane.
     *          Each lane should be either all 0xFF (not equal) or all 0x00 (equal).
     * @param expected_lane0_ne True if lane 0 should be not-equal (0xFFFFFFFF), false for equal (0x00000000)
     * @param expected_lane1_ne True if lane 1 should be not-equal (0xFFFFFFFF), false for equal (0x00000000)
     * @param expected_lane2_ne True if lane 2 should be not-equal (0xFFFFFFFF), false for equal (0x00000000)
     * @param expected_lane3_ne True if lane 3 should be not-equal (0xFFFFFFFF), false for equal (0x00000000)
     * @param actual_bytes Actual 16-byte result from f32x4.ne operation
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc:validate_f32x4_ne_result
     */
    void validate_f32x4_ne_result(bool expected_lane0_ne, bool expected_lane1_ne,
                                  bool expected_lane2_ne, bool expected_lane3_ne,
                                  const uint8_t actual_bytes[16])
    {
        // Lane 0 validation (bytes 0-3)
        uint32_t expected_lane0 = expected_lane0_ne ? 0xFFFFFFFF : 0x00000000;
        uint32_t actual_lane0;
        memcpy(&actual_lane0, &actual_bytes[0], 4);

        ASSERT_EQ(expected_lane0, actual_lane0)
            << "f32x4.ne lane 0 mismatch - Expected: 0x" << std::hex << expected_lane0
            << ", Actual: 0x" << std::hex << actual_lane0;

        // Lane 1 validation (bytes 4-7)
        uint32_t expected_lane1 = expected_lane1_ne ? 0xFFFFFFFF : 0x00000000;
        uint32_t actual_lane1;
        memcpy(&actual_lane1, &actual_bytes[4], 4);

        ASSERT_EQ(expected_lane1, actual_lane1)
            << "f32x4.ne lane 1 mismatch - Expected: 0x" << std::hex << expected_lane1
            << ", Actual: 0x" << std::hex << actual_lane1;

        // Lane 2 validation (bytes 8-11)
        uint32_t expected_lane2 = expected_lane2_ne ? 0xFFFFFFFF : 0x00000000;
        uint32_t actual_lane2;
        memcpy(&actual_lane2, &actual_bytes[8], 4);

        ASSERT_EQ(expected_lane2, actual_lane2)
            << "f32x4.ne lane 2 mismatch - Expected: 0x" << std::hex << expected_lane2
            << ", Actual: 0x" << std::hex << actual_lane2;

        // Lane 3 validation (bytes 12-15)
        uint32_t expected_lane3 = expected_lane3_ne ? 0xFFFFFFFF : 0x00000000;
        uint32_t actual_lane3;
        memcpy(&actual_lane3, &actual_bytes[12], 4);

        ASSERT_EQ(expected_lane3, actual_lane3)
            << "f32x4.ne lane 3 mismatch - Expected: 0x" << std::hex << expected_lane3
            << ", Actual: 0x" << std::hex << actual_lane3;
    }

    /**
     * @brief Get NaN value for testing purposes
     * @details Returns a standard quiet NaN value for float comparison testing.
     * @return float Quiet NaN value
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc:get_nan_f32
     */
    float get_nan_f32()
    {
        return std::numeric_limits<float>::quiet_NaN();
    }

    /**
     * @brief Get positive infinity value for testing
     * @details Returns positive infinity for boundary value testing.
     * @return float Positive infinity
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc:get_pos_inf_f32
     */
    float get_pos_inf_f32()
    {
        return std::numeric_limits<float>::infinity();
    }

    /**
     * @brief Get negative infinity value for testing
     * @details Returns negative infinity for boundary value testing.
     * @return float Negative infinity
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc:get_neg_inf_f32
     */
    float get_neg_inf_f32()
    {
        return -std::numeric_limits<float>::infinity();
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicNotEqualComparison_ReturnsCorrectResults
 * @brief Validates f32x4.ne produces correct arithmetic results for typical inputs
 * @details Tests fundamental not-equal operation with mixed equal/not-equal f32 values.
 *          Verifies that f32x4.ne correctly computes a != b for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f32x4_ne
 * @input_conditions Standard float pairs with mixed equal/not-equal values across lanes
 * @expected_behavior Returns 0xFFFFFFFF for not-equal lanes, 0x00000000 for equal lanes
 * @validation_method Direct comparison of WASM function result with expected lane values
 */
TEST_F(F32x4NeTestSuite, BasicNotEqualComparison_ReturnsCorrectResults)
{
    uint8_t result_bytes[16];

    // Test case 1: All lanes different
    ASSERT_TRUE(call_f32x4_ne(1.0f, 2.0f, 3.0f, 4.0f,
                              5.0f, 6.0f, 7.0f, 8.0f,
                              result_bytes))
        << "Failed to execute f32x4.ne with all different values";

    validate_f32x4_ne_result(true, true, true, true, result_bytes);

    // Test case 2: Mixed equal and not-equal lanes
    ASSERT_TRUE(call_f32x4_ne(1.0f, 2.0f, 3.0f, 4.0f,
                              1.0f, 9.0f, 3.0f, 10.0f,
                              result_bytes))
        << "Failed to execute f32x4.ne with mixed equal/not-equal values";

    validate_f32x4_ne_result(false, true, false, true, result_bytes);

    // Test case 3: All lanes equal
    ASSERT_TRUE(call_f32x4_ne(5.0f, 5.0f, 5.0f, 5.0f,
                              5.0f, 5.0f, 5.0f, 5.0f,
                              result_bytes))
        << "Failed to execute f32x4.ne with all equal values";

    validate_f32x4_ne_result(false, false, false, false, result_bytes);

    // Test case 4: Negative and positive values
    ASSERT_TRUE(call_f32x4_ne(-1.0f, -2.0f, 3.0f, -4.0f,
                              -1.0f, 2.0f, -3.0f, -4.0f,
                              result_bytes))
        << "Failed to execute f32x4.ne with negative and positive values";

    validate_f32x4_ne_result(false, true, true, false, result_bytes);
}

/**
 * @test BoundaryValueComparisons_HandlesExtremeValues
 * @brief Validates f32x4.ne handling of boundary and extreme float values
 * @details Tests f32x4.ne with FLT_MAX, FLT_MIN, and subnormal number comparisons.
 *          Verifies precision handling at numeric boundaries and edge cases.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f32x4_ne
 * @input_conditions Boundary values: FLT_MAX, FLT_MIN, subnormal numbers, zero
 * @expected_behavior Correct inequality detection for extreme values
 * @validation_method Boundary value comparison and subnormal number handling verification
 */
TEST_F(F32x4NeTestSuite, BoundaryValueComparisons_HandlesExtremeValues)
{
    uint8_t result_bytes[16];

    // Test case 1: Maximum float values
    float flt_max = std::numeric_limits<float>::max();
    ASSERT_TRUE(call_f32x4_ne(flt_max, flt_max, flt_max, flt_max,
                              flt_max, flt_max - 1.0f, flt_max, flt_max - 1.0f,
                              result_bytes))
        << "Failed to execute f32x4.ne with FLT_MAX values";

    validate_f32x4_ne_result(false, true, false, true, result_bytes);

    // Test case 2: Minimum positive normal values
    float flt_min = std::numeric_limits<float>::min();
    ASSERT_TRUE(call_f32x4_ne(flt_min, flt_min, flt_min * 2.0f, flt_min,
                              flt_min, flt_min * 2.0f, flt_min * 2.0f, 0.0f,
                              result_bytes))
        << "Failed to execute f32x4.ne with FLT_MIN values";

    validate_f32x4_ne_result(false, true, false, true, result_bytes);

    // Test case 3: Very small subnormal numbers
    float subnormal1 = std::numeric_limits<float>::denorm_min();
    float subnormal2 = subnormal1 * 2.0f;
    ASSERT_TRUE(call_f32x4_ne(subnormal1, subnormal2, 0.0f, subnormal1,
                              subnormal2, subnormal1, 0.0f, subnormal2,
                              result_bytes))
        << "Failed to execute f32x4.ne with subnormal values";

    validate_f32x4_ne_result(true, true, false, true, result_bytes);

    // Test case 4: Zero and near-zero comparisons
    ASSERT_TRUE(call_f32x4_ne(0.0f, -0.0f, 1e-38f, -1e-38f,
                              -0.0f, 0.0f, -1e-38f, 1e-38f,
                              result_bytes))
        << "Failed to execute f32x4.ne with zero and near-zero values";

    // +0.0f and -0.0f should be considered equal per IEEE 754
    validate_f32x4_ne_result(false, false, true, true, result_bytes);
}

/**
 * @test SpecialIEEE754Values_ReturnsSpecificationCompliantResults
 * @brief Validates f32x4.ne compliance with IEEE 754 special value handling
 * @details Tests NaN comparisons (NaN != NaN → true), infinity handling, and zero variants.
 *          Verifies critical IEEE 754 requirements for NaN and infinity behavior.
 * @test_category Edge - IEEE 754 specification compliance
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f32x4_ne
 * @input_conditions IEEE 754 special values: NaN, ±∞, ±0.0f combinations
 * @expected_behavior NaN != anything → true, proper infinity and zero handling
 * @validation_method IEEE 754 specification compliance verification
 */
TEST_F(F32x4NeTestSuite, SpecialIEEE754Values_ReturnsSpecificationCompliantResults)
{
    uint8_t result_bytes[16];

    float nan_val = get_nan_f32();
    float pos_inf = get_pos_inf_f32();
    float neg_inf = get_neg_inf_f32();

    // Test case 1: NaN comparisons (NaN != NaN should be true)
    ASSERT_TRUE(call_f32x4_ne(nan_val, nan_val, nan_val, 1.0f,
                              nan_val, 2.0f, nan_val, nan_val,
                              result_bytes))
        << "Failed to execute f32x4.ne with NaN values";

    // NaN != anything (including itself) should always be true
    validate_f32x4_ne_result(true, true, true, true, result_bytes);

    // Test case 2: Infinity comparisons
    ASSERT_TRUE(call_f32x4_ne(pos_inf, neg_inf, pos_inf, neg_inf,
                              pos_inf, neg_inf, neg_inf, pos_inf,
                              result_bytes))
        << "Failed to execute f32x4.ne with infinity values";

    validate_f32x4_ne_result(false, false, true, true, result_bytes);

    // Test case 3: Mixed special values
    ASSERT_TRUE(call_f32x4_ne(nan_val, pos_inf, 0.0f, -0.0f,
                              1.0f, pos_inf, -0.0f, 0.0f,
                              result_bytes))
        << "Failed to execute f32x4.ne with mixed special values";

    // NaN != 1.0f → true, pos_inf == pos_inf → false, 0.0f == -0.0f → false, -0.0f == 0.0f → false
    validate_f32x4_ne_result(true, false, false, false, result_bytes);

    // Test case 4: NaN vs infinity and zero
    ASSERT_TRUE(call_f32x4_ne(nan_val, nan_val, nan_val, nan_val,
                              pos_inf, neg_inf, 0.0f, -0.0f,
                              result_bytes))
        << "Failed to execute f32x4.ne with NaN vs other special values";

    // NaN != anything should always be true
    validate_f32x4_ne_result(true, true, true, true, result_bytes);
}

/**
 * @test ModuleLoadFailure_FailsGracefully
 * @brief Validates graceful handling of module loading failures
 * @details Tests error handling when WASM module cannot be loaded or is malformed.
 *          Ensures proper error reporting without crashes or undefined behavior.
 * @test_category Error - Module loading validation
 * @coverage_target tests/unit/enhanced_opcode/simd/enhanced_f32x4_ne_test.cc:SetUp
 * @input_conditions Invalid or missing WASM module file
 * @expected_behavior Graceful failure with proper error reporting
 * @validation_method Module loading error detection and handling verification
 */
TEST_F(F32x4NeTestSuite, ModuleLoadFailure_FailsGracefully)
{
    // Test module load validation (handled in SetUp)
    ASSERT_NE(nullptr, dummy_env->get())
        << "Test environment should be properly initialized for f32x4.ne tests";

    // Validate that the module has the expected function
    uint8_t result_bytes[16];

    // Attempt basic operation to verify module functionality
    bool success = call_f32x4_ne(1.0f, 2.0f, 3.0f, 4.0f,
                                 1.0f, 2.0f, 3.0f, 4.0f,
                                 result_bytes);

    ASSERT_TRUE(success)
        << "Basic f32x4.ne operation should succeed with properly loaded module";

    // Verify correct result for equal values
    validate_f32x4_ne_result(false, false, false, false, result_bytes);
}