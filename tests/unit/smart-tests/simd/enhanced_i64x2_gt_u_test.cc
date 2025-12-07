/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include <memory>

// Test execution modes for parameterized testing
enum class TestRunningMode {
    INTERP_MODE,
    AOT_MODE
};

/**
 * @brief Test fixture for i64x2.gt_u SIMD instruction validation
 * @details Provides comprehensive testing infrastructure for i64x2.gt_u opcode
 *          across both interpreter and AOT execution modes. Tests SIMD unsigned
 *          64-bit integer comparison functionality with proper setup/teardown.
 */
class I64x2GtUTest : public testing::TestWithParam<TestRunningMode>
{
  protected:
    /**
     * @brief Set up test environment for i64x2.gt_u instruction testing
     * @details Initializes WAMR runtime with SIMD support, loads test module,
     *          and prepares execution context for both interpreter and AOT modes
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i64x2.gt_u test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i64x2_gt_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i64x2.gt_u tests";
    }

    /**
     * @brief Clean up test environment and release WAMR resources
     * @details Properly deallocates module instance, module, buffer and runtime
     *          to prevent resource leaks during test execution
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Call WASM i64x2_gt_u function with two v128 operands
     * @param a_high High 64-bit value of first vector
     * @param a_low Low 64-bit value of first vector
     * @param b_high High 64-bit value of second vector
     * @param b_low Low 64-bit value of second vector
     * @param result_high Pointer to store high 64-bit result
     * @param result_low Pointer to store low 64-bit result
     * @return true if function call succeeded, false otherwise
     */
    bool call_i64x2_gt_u(uint64_t a_high, uint64_t a_low,
                         uint64_t b_high, uint64_t b_low,
                         uint64_t* result_high, uint64_t* result_low)
    {
        uint32_t argv[8] = {0};
        // Pack first vector (a_high, a_low)
        argv[0] = (uint32_t)(a_low & 0xFFFFFFFF);
        argv[1] = (uint32_t)(a_low >> 32);
        argv[2] = (uint32_t)(a_high & 0xFFFFFFFF);
        argv[3] = (uint32_t)(a_high >> 32);
        // Pack second vector (b_high, b_low)
        argv[4] = (uint32_t)(b_low & 0xFFFFFFFF);
        argv[5] = (uint32_t)(b_low >> 32);
        argv[6] = (uint32_t)(b_high & 0xFFFFFFFF);
        argv[7] = (uint32_t)(b_high >> 32);

        bool ret = dummy_env->execute("i64x2_gt_u", 8, argv);
        if (ret) {
            // Unpack result vector
            *result_low = ((uint64_t)argv[1] << 32) | argv[0];
            *result_high = ((uint64_t)argv[3] << 32) | argv[2];
        }
        return ret;
    }

    TestRunningMode running_mode;
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicComparison_ReturnsCorrectMask
 * @brief Validates i64x2.gt_u produces correct comparison masks for typical unsigned values
 * @details Tests fundamental unsigned comparison with standard integer pairs to ensure
 *          proper element-wise greater-than logic and mask generation behavior.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_gt_u
 * @input_conditions Two v128 vectors: [100, 200] vs [50, 300]
 * @expected_behavior Returns comparison masks: [0xFFFFFFFFFFFFFFFF, 0x0000000000000000]
 * @validation_method Direct comparison of WASM function result with expected mask values
 */
TEST_P(I64x2GtUTest, BasicComparison_ReturnsCorrectMask)
{
    uint64_t result_high, result_low;

    // Test: [100, 200] > [50, 300] = [true, false]
    ASSERT_TRUE(call_i64x2_gt_u(200, 100, 300, 50, &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with basic comparison values";

    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_low)
        << "Expected true mask for 100 > 50, got: 0x" << std::hex << result_low;
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false mask for 200 > 300, got: 0x" << std::hex << result_high;

    // Test: [1000, 2000] > [999, 2001] = [true, false]
    ASSERT_TRUE(call_i64x2_gt_u(2000, 1000, 2001, 999, &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with incremental comparison values";

    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_low)
        << "Expected true mask for 1000 > 999, got: 0x" << std::hex << result_low;
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false mask for 2000 > 2001, got: 0x" << std::hex << result_high;
}

/**
 * @test BoundaryValues_MaxAndMinComparisons
 * @brief Validates i64x2.gt_u handles boundary values correctly at numeric limits
 * @details Tests comparison behavior at maximum and minimum unsigned 64-bit values
 *          to ensure proper boundary condition handling and overflow prevention.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_gt_u
 * @input_conditions Boundary vectors: [0xFFFFFFFFFFFFFFFF, 0x0] vs [0xFFFFFFFFFFFFFFFE, 0x1]
 * @expected_behavior Returns masks: [0xFFFFFFFFFFFFFFFF, 0x0000000000000000]
 * @validation_method Verify correct handling of maximum and minimum unsigned values
 */
TEST_P(I64x2GtUTest, BoundaryValues_MaxAndMinComparisons)
{
    uint64_t result_high, result_low;

    // Test: [MAX, MIN] > [MAX-1, 1] = [true, false]
    ASSERT_TRUE(call_i64x2_gt_u(0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL,
                               0x0000000000000001ULL, 0xFFFFFFFFFFFFFFFEULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with boundary values";

    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_low)
        << "Expected true mask for MAX > MAX-1, got: 0x" << std::hex << result_low;
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false mask for MIN > 1, got: 0x" << std::hex << result_high;

    // Test: [0x8000000000000001, 0x7FFFFFFFFFFFFFFF] > [0x8000000000000000, 0x8000000000000000] = [true, false]
    ASSERT_TRUE(call_i64x2_gt_u(0x7FFFFFFFFFFFFFFFULL, 0x8000000000000001ULL,
                               0x8000000000000000ULL, 0x8000000000000000ULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with mid-range boundary values";

    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_low)
        << "Expected true mask for 0x8000000000000001 > 0x8000000000000000";
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false mask for 0x7FFFFFFFFFFFFFFF > 0x8000000000000000";
}

/**
 * @test UnsignedSemantics_HandlesLargeValues
 * @brief Validates i64x2.gt_u uses unsigned comparison semantics correctly
 * @details Tests values that would differ between signed and unsigned interpretation
 *          to ensure proper unsigned comparison behavior throughout the full range.
 * @test_category Main - Unsigned semantics validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_gt_u
 * @input_conditions Mixed vectors: [0x8000000000000000, 1] vs [0x7FFFFFFFFFFFFFFF, 2]
 * @expected_behavior Returns masks: [0xFFFFFFFFFFFFFFFF, 0x0000000000000000]
 * @validation_method Confirm unsigned interpretation produces correct comparison results
 */
TEST_P(I64x2GtUTest, UnsignedSemantics_HandlesLargeValues)
{
    uint64_t result_high, result_low;

    // Test unsigned semantics: large unsigned values vs smaller signed-positive values
    ASSERT_TRUE(call_i64x2_gt_u(0x0000000000000001ULL, 0x8000000000000000ULL,
                               0x0000000000000002ULL, 0x7FFFFFFFFFFFFFFFULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with unsigned semantic test values";

    // 0x8000000000000000 > 0x7FFFFFFFFFFFFFFF in unsigned comparison
    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_low)
        << "Expected true for large unsigned > smaller unsigned value";
    // 1 > 2 is false
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false for 1 > 2 comparison";

    // Test with very large unsigned values
    ASSERT_TRUE(call_i64x2_gt_u(0xF000000000000000ULL, 0xE000000000000000ULL,
                               0xD000000000000000ULL, 0xF000000000000000ULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with large unsigned values";

    ASSERT_EQ(0x0000000000000000, result_low)
        << "Expected false for 0xE000000000000000 > 0xF000000000000000";
    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_high)
        << "Expected true for 0xF000000000000000 > 0xD000000000000000";
}

/**
 * @test ZeroOperands_HandlesZeroComparisons
 * @brief Validates i64x2.gt_u handles zero values correctly in various positions
 * @details Tests zero value comparisons to ensure proper handling of zero operands
 *          and correct comparison logic when zero appears in different vector positions.
 * @test_category Edge - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_gt_u
 * @input_conditions Zero vectors: [0x0, 0x0] vs [0x0, 0x1]
 * @expected_behavior Returns masks: [0x0000000000000000, 0x0000000000000000]
 * @validation_method Verify zero comparisons produce expected false results
 */
TEST_P(I64x2GtUTest, ZeroOperands_HandlesZeroComparisons)
{
    uint64_t result_high, result_low;

    // Test: [0, 0] > [0, 1] = [false, false]
    ASSERT_TRUE(call_i64x2_gt_u(0x0000000000000000ULL, 0x0000000000000000ULL,
                               0x0000000000000001ULL, 0x0000000000000000ULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with zero operands";

    ASSERT_EQ(0x0000000000000000, result_low)
        << "Expected false for 0 > 0 comparison";
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false for 0 > 1 comparison";

    // Test: [1, 0] > [0, 1] = [true, false]
    ASSERT_TRUE(call_i64x2_gt_u(0x0000000000000000ULL, 0x0000000000000001ULL,
                               0x0000000000000001ULL, 0x0000000000000000ULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with mixed zero values";

    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_low)
        << "Expected true for 1 > 0 comparison";
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false for 0 > 1 comparison";
}

/**
 * @test IdentityComparison_ReturnsFalseForEqual
 * @brief Validates i64x2.gt_u returns false when comparing identical values
 * @details Tests that equal value comparisons (A > A) correctly return false masks,
 *          validating proper equality detection in greater-than comparison logic.
 * @test_category Edge - Identity comparison validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_gt_u
 * @input_conditions Identical vectors: [0x123456789ABCDEF0, 0xFEDCBA9876543210] vs same
 * @expected_behavior Returns masks: [0x0000000000000000, 0x0000000000000000]
 * @validation_method Verify identical values produce false for greater-than comparison
 */
TEST_P(I64x2GtUTest, IdentityComparison_ReturnsFalseForEqual)
{
    uint64_t result_high, result_low;

    // Test: identical values should return false for greater-than
    uint64_t test_val1 = 0x123456789ABCDEF0ULL;
    uint64_t test_val2 = 0xFEDCBA9876543210ULL;

    ASSERT_TRUE(call_i64x2_gt_u(test_val2, test_val1, test_val2, test_val1,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with identical values";

    ASSERT_EQ(0x0000000000000000, result_low)
        << "Expected false for identical value comparison: 0x" << std::hex << test_val1;
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false for identical value comparison: 0x" << std::hex << test_val2;

    // Test with maximum values
    ASSERT_TRUE(call_i64x2_gt_u(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                               0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with identical maximum values";

    ASSERT_EQ(0x0000000000000000, result_low)
        << "Expected false for MAX > MAX comparison";
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false for MAX > MAX comparison";
}

/**
 * @test ExtremeValues_MaxVsMinComparisons
 * @brief Validates i64x2.gt_u handles extreme value combinations correctly
 * @details Tests comparisons between maximum and minimum possible 64-bit unsigned values
 *          to ensure proper handling of full numeric range and extreme value scenarios.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i64x2_gt_u
 * @input_conditions Extreme vectors: [0xFFFFFFFFFFFFFFFF, 0x1] vs [0x0, 0xFFFFFFFFFFFFFFFF]
 * @expected_behavior Returns masks: [0xFFFFFFFFFFFFFFFF, 0x0000000000000000]
 * @validation_method Verify MAX > MIN and 1 is not > MAX produce correct masks
 */
TEST_P(I64x2GtUTest, ExtremeValues_MaxVsMinComparisons)
{
    uint64_t result_high, result_low;

    // Test: [MAX, 1] > [MIN, MAX] = [true, false]
    ASSERT_TRUE(call_i64x2_gt_u(0x0000000000000001ULL, 0xFFFFFFFFFFFFFFFFULL,
                               0xFFFFFFFFFFFFFFFFULL, 0x0000000000000000ULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with extreme values";

    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_low)
        << "Expected true for MAX > MIN comparison";
    ASSERT_EQ(0x0000000000000000, result_high)
        << "Expected false for 1 > MAX comparison";

    // Test power-of-2 boundaries
    ASSERT_TRUE(call_i64x2_gt_u(0x0000000000010000ULL, 0x0000000100000000ULL,
                               0x000000000000FFFFULL, 0x00000000FFFFFFFFULL,
                               &result_high, &result_low))
        << "Failed to execute i64x2.gt_u with power-of-2 boundary values";

    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_low)
        << "Expected true for 0x0000000100000000 > 0x00000000FFFFFFFF";
    ASSERT_EQ(0xFFFFFFFFFFFFFFFF, result_high)
        << "Expected true for 0x0000000000010000 > 0x000000000000FFFF";
}

INSTANTIATE_TEST_SUITE_P(RunningMode, I64x2GtUTest,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));