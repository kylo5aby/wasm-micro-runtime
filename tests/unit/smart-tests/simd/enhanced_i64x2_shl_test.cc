/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"

/**
 * @brief Test class for i64x2.shl opcode functionality validation
 * @details This test class validates the i64x2.shl SIMD opcode which performs
 *          left shift operations on each 64-bit lane of a 128-bit vector.
 *          Tests cover basic functionality, boundary conditions, edge cases,
 *          and cross-execution mode validation using comprehensive WAMR testing framework.
 */
class I64x2ShlTestSuite : public testing::Test {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     * @details Initializes WAMR runtime using RAII helper and loads the
     *          i64x2.shl test WASM module for execution testing.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i64x2.shl test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i64x2_shl_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i64x2.shl tests";
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     * @details Properly releases all allocated resources using RAII pattern
     *          to prevent resource leaks.
     */
    void TearDown() override {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute i64x2.shl operation with vector and scalar shift count
     * @param lane0 First 64-bit lane value (uint64_t)
     * @param lane1 Second 64-bit lane value (uint64_t)
     * @param shift_count Scalar shift amount (i32)
     * @param result_lane0 Output for first lane result
     * @param result_lane1 Output for second lane result
     * @return bool True if operation succeeded, false on error
     * @details Calls the WASM test function to perform i64x2.shl operation
     */
    bool call_i64x2_shl(uint64_t lane0, uint64_t lane1, int32_t shift_count,
                        uint64_t &result_lane0, uint64_t &result_lane1) {
        // Prepare arguments: 2 lanes (split into high/low 32-bit parts) + 1 shift count
        uint32_t argv[5];

        // Set up lane 0 (low 32 bits, high 32 bits)
        argv[0] = static_cast<uint32_t>(lane0 & 0xFFFFFFFF);
        argv[1] = static_cast<uint32_t>(lane0 >> 32);

        // Set up lane 1 (low 32 bits, high 32 bits)
        argv[2] = static_cast<uint32_t>(lane1 & 0xFFFFFFFF);
        argv[3] = static_cast<uint32_t>(lane1 >> 32);

        // Set shift count
        argv[4] = static_cast<uint32_t>(shift_count);

        // Call WASM function with 5 arguments
        bool call_success = dummy_env->execute("test_i64x2_shl", 5, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_i64x2_shl function";

        if (call_success) {
            // Extract result values - function returns 4 32-bit values (2 lanes split)
            result_lane0 = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            result_lane1 = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];
        }

        return call_success;
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicShift_TypicalValues_ReturnsCorrectResults
 * @brief Validates i64x2.shl produces correct left shift results for standard inputs
 * @details Tests fundamental left shift operation with typical shift amounts (1, 4, 8, 16).
 *          Verifies that each lane shifts independently and produces mathematically
 *          correct results for common use cases.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64x2_shl_operation
 * @input_conditions Standard 64-bit values with shift amounts 1, 4, 8, 16
 * @expected_behavior Each lane shifts left by shift_count with correct bit patterns
 * @validation_method Direct comparison of each lane result with expected values
 */
TEST_F(I64x2ShlTestSuite, BasicShift_TypicalValues_ReturnsCorrectResults) {
    uint64_t result_lane0, result_lane1;

    // Test input values
    uint64_t input_lane0 = 0x0123456789ABCDEF;
    uint64_t input_lane1 = 0xFEDCBA9876543210;

    // Test shift by 1
    ASSERT_TRUE(call_i64x2_shl(input_lane0, input_lane1, 1, result_lane0, result_lane1))
        << "Failed to execute shift by 1";
    ASSERT_EQ(0x02468ACF13579BDE, result_lane0)
        << "Shift 0x0123456789ABCDEF << 1 failed";
    ASSERT_EQ(0xFDB97530ECA86420, result_lane1)
        << "Shift 0xFEDCBA9876543210 << 1 failed";

    // Test shift by 4
    ASSERT_TRUE(call_i64x2_shl(input_lane0, input_lane1, 4, result_lane0, result_lane1))
        << "Failed to execute shift by 4";
    ASSERT_EQ(0x123456789ABCDEF0, result_lane0)
        << "Shift 0x0123456789ABCDEF << 4 failed";
    ASSERT_EQ(0xEDCBA9876543210 << 4, result_lane1)
        << "Shift 0xFEDCBA9876543210 << 4 failed";

    // Test shift by 8
    ASSERT_TRUE(call_i64x2_shl(input_lane0, input_lane1, 8, result_lane0, result_lane1))
        << "Failed to execute shift by 8";
    ASSERT_EQ(0x23456789ABCDEF00, result_lane0)
        << "Shift 0x0123456789ABCDEF << 8 failed";
    ASSERT_EQ(0xDCBA987654321000, result_lane1)
        << "Shift 0xFEDCBA9876543210 << 8 failed";

    // Test shift by 16
    ASSERT_TRUE(call_i64x2_shl(input_lane0, input_lane1, 16, result_lane0, result_lane1))
        << "Failed to execute shift by 16";
    ASSERT_EQ(0x456789ABCDEF0000, result_lane0)
        << "Shift 0x0123456789ABCDEF << 16 failed";
    ASSERT_EQ(0xBA98765432100000, result_lane1)
        << "Shift 0xFEDCBA9876543210 << 16 failed";
}

/**
 * @test BoundaryShifts_MaximumValidShift_ReturnsCorrectResults
 * @brief Validates i64x2.shl handles boundary conditions and maximum shifts correctly
 * @details Tests identity operation (shift 0), half-width shift (32), and maximum
 *          valid shift (63). Verifies proper bit positioning at boundaries.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64x2_shl_boundary_handling
 * @input_conditions Single-bit and sign-bit patterns with boundary shift amounts
 * @expected_behavior Correct bit positioning at shift boundaries (0, 32, 63)
 * @validation_method Verification of specific bit patterns after boundary shifts
 */
TEST_F(I64x2ShlTestSuite, BoundaryShifts_MaximumValidShift_ReturnsCorrectResults) {
    uint64_t result_lane0, result_lane1;

    // Test with single bit and sign bit patterns
    uint64_t single_bit = 0x0000000000000001;
    uint64_t sign_bit = 0x8000000000000000;

    // Test zero shift (identity operation)
    ASSERT_TRUE(call_i64x2_shl(single_bit, sign_bit, 0, result_lane0, result_lane1))
        << "Failed to execute zero shift";
    ASSERT_EQ(single_bit, result_lane0)
        << "Zero shift should preserve input for single bit";
    ASSERT_EQ(sign_bit, result_lane1)
        << "Zero shift should preserve input for sign bit";

    // Test shift by 32 (half-width)
    ASSERT_TRUE(call_i64x2_shl(single_bit, sign_bit, 32, result_lane0, result_lane1))
        << "Failed to execute shift by 32";
    ASSERT_EQ(0x0000000100000000, result_lane0)
        << "Shift 0x0000000000000001 << 32 failed";
    ASSERT_EQ(0x0000000000000000, result_lane1)
        << "Shift 0x8000000000000000 << 32 should overflow to 0";

    // Test shift by 63 (maximum valid shift)
    ASSERT_TRUE(call_i64x2_shl(single_bit, sign_bit, 63, result_lane0, result_lane1))
        << "Failed to execute shift by 63";
    ASSERT_EQ(0x8000000000000000, result_lane0)
        << "Shift 0x0000000000000001 << 63 should move to sign bit";
    ASSERT_EQ(0x0000000000000000, result_lane1)
        << "Shift 0x8000000000000000 << 63 should overflow to 0";
}

/**
 * @test LargeShiftCounts_ModuloWrapping_ReturnsCorrectResults
 * @brief Validates i64x2.shl correctly masks large shift counts using modulo 64
 * @details Tests shift counts >= 64 to verify they wrap correctly (shift_count & 63).
 *          Ensures proper modulo arithmetic implementation for 64-bit values.
 * @test_category Corner - Shift count wrapping validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64x2_shl_mask_handling
 * @input_conditions Single-bit pattern with large shift counts (64, 65, 127, 128)
 * @expected_behavior Results equivalent to (shift_count & 63) modulo behavior
 * @validation_method Verification of modulo arithmetic for large shift values
 */
TEST_F(I64x2ShlTestSuite, LargeShiftCounts_ModuloWrapping_ReturnsCorrectResults) {
    uint64_t result_lane0, result_lane1;

    // Test input - single bit pattern for easy verification
    uint64_t test_pattern = 0x0000000000000001;

    // Test shift by 64 (should wrap to 0)
    ASSERT_TRUE(call_i64x2_shl(test_pattern, test_pattern, 64, result_lane0, result_lane1))
        << "Failed to execute shift by 64";
    ASSERT_EQ(test_pattern, result_lane0)
        << "Shift by 64 should equal shift by 0 (64 & 63 = 0)";
    ASSERT_EQ(test_pattern, result_lane1)
        << "Shift by 64 should equal shift by 0 (64 & 63 = 0)";

    // Test shift by 65 (should wrap to 1)
    ASSERT_TRUE(call_i64x2_shl(test_pattern, test_pattern, 65, result_lane0, result_lane1))
        << "Failed to execute shift by 65";
    ASSERT_EQ(0x0000000000000002, result_lane0)
        << "Shift by 65 should equal shift by 1 (65 & 63 = 1)";
    ASSERT_EQ(0x0000000000000002, result_lane1)
        << "Shift by 65 should equal shift by 1 (65 & 63 = 1)";

    // Test shift by 127 (should wrap to 63)
    ASSERT_TRUE(call_i64x2_shl(test_pattern, test_pattern, 127, result_lane0, result_lane1))
        << "Failed to execute shift by 127";
    ASSERT_EQ(0x8000000000000000, result_lane0)
        << "Shift by 127 should equal shift by 63 (127 & 63 = 63)";
    ASSERT_EQ(0x8000000000000000, result_lane1)
        << "Shift by 127 should equal shift by 63 (127 & 63 = 63)";

    // Test shift by 128 (should wrap to 0)
    ASSERT_TRUE(call_i64x2_shl(test_pattern, test_pattern, 128, result_lane0, result_lane1))
        << "Failed to execute shift by 128";
    ASSERT_EQ(test_pattern, result_lane0)
        << "Shift by 128 should equal shift by 0 (128 & 63 = 0)";
    ASSERT_EQ(test_pattern, result_lane1)
        << "Shift by 128 should equal shift by 0 (128 & 63 = 0)";
}

/**
 * @test ZeroOperands_IdentityAndZero_ReturnsExpectedResults
 * @brief Validates i64x2.shl behavior with zero operands and identity operations
 * @details Tests mathematical properties: zero shifted remains zero, and zero shift
 *          is identity. Verifies fundamental mathematical properties of shift operation.
 * @test_category Edge - Zero operand and identity validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64x2_shl_zero_handling
 * @input_conditions Zero shift amounts, zero vector values, mixed scenarios
 * @expected_behavior Mathematical properties of zero in shift operations preserved
 * @validation_method Verification of identity and zero properties
 */
TEST_F(I64x2ShlTestSuite, ZeroOperands_IdentityAndZero_ReturnsExpectedResults) {
    uint64_t result_lane0, result_lane1;

    // Test zero vector with various shift counts
    uint64_t zero_value = 0x0000000000000000;
    ASSERT_TRUE(call_i64x2_shl(zero_value, zero_value, 5, result_lane0, result_lane1))
        << "Failed to execute zero vector shift";
    ASSERT_EQ(zero_value, result_lane0)
        << "Zero shifted by any amount should remain zero (lane 0)";
    ASSERT_EQ(zero_value, result_lane1)
        << "Zero shifted by any amount should remain zero (lane 1)";

    // Test zero shift with non-zero vector (identity operation)
    uint64_t non_zero_0 = 0x123456789ABCDEF0;
    uint64_t non_zero_1 = 0xFEDCBA0987654321;
    ASSERT_TRUE(call_i64x2_shl(non_zero_0, non_zero_1, 0, result_lane0, result_lane1))
        << "Failed to execute identity operation (zero shift)";
    ASSERT_EQ(non_zero_0, result_lane0)
        << "Identity operation should preserve input (lane 0)";
    ASSERT_EQ(non_zero_1, result_lane1)
        << "Identity operation should preserve input (lane 1)";

    // Test mixed case: one zero lane, one non-zero lane
    ASSERT_TRUE(call_i64x2_shl(zero_value, non_zero_1, 4, result_lane0, result_lane1))
        << "Failed to execute mixed zero/non-zero shift";
    ASSERT_EQ(zero_value, result_lane0)
        << "Zero lane should remain zero regardless of shift";
    ASSERT_EQ(non_zero_1 << 4, result_lane1)
        << "Non-zero lane should shift correctly";
}

/**
 * @test ExtremeValues_SpecialPatterns_ReturnsCorrectBitPatterns
 * @brief Validates i64x2.shl handles extreme values and special bit patterns correctly
 * @details Tests all-ones patterns, alternating bits, and sign bits with various shifts.
 *          Verifies complex bit manipulation for extreme patterns and edge cases.
 * @test_category Edge - Extreme value and pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64x2_shl_pattern_handling
 * @input_conditions All-ones, alternating bits, sign bits with various shift counts
 * @expected_behavior Correct bit manipulation for extreme patterns and edge cases
 * @validation_method Pattern-specific validations and bit transformation checks
 */
TEST_F(I64x2ShlTestSuite, ExtremeValues_SpecialPatterns_ReturnsCorrectBitPatterns) {
    uint64_t result_lane0, result_lane1;

    // Test all-ones pattern
    uint64_t all_ones = 0xFFFFFFFFFFFFFFFF;
    ASSERT_TRUE(call_i64x2_shl(all_ones, all_ones, 1, result_lane0, result_lane1))
        << "Failed to execute all-ones shift by 1";
    ASSERT_EQ(0xFFFFFFFFFFFFFFFE, result_lane0)
        << "All-ones << 1 should be 0xFFFFFFFFFFFFFFFE";
    ASSERT_EQ(0xFFFFFFFFFFFFFFFE, result_lane1)
        << "All-ones << 1 should be 0xFFFFFFFFFFFFFFFE";

    ASSERT_TRUE(call_i64x2_shl(all_ones, all_ones, 4, result_lane0, result_lane1))
        << "Failed to execute all-ones shift by 4";
    ASSERT_EQ(0xFFFFFFFFFFFFFFF0, result_lane0)
        << "All-ones << 4 should be 0xFFFFFFFFFFFFFFF0";
    ASSERT_EQ(0xFFFFFFFFFFFFFFF0, result_lane1)
        << "All-ones << 4 should be 0xFFFFFFFFFFFFFFF0";

    // Test alternating bit patterns
    uint64_t alternating_1 = 0xAAAAAAAAAAAAAAAA; // 10101010...
    uint64_t alternating_2 = 0x5555555555555555; // 01010101...
    ASSERT_TRUE(call_i64x2_shl(alternating_1, alternating_2, 1, result_lane0, result_lane1))
        << "Failed to execute alternating pattern shift";
    ASSERT_EQ(0x5555555555555554, result_lane0)
        << "0xAAAAAAAAAAAAAAAA << 1 should be 0x5555555555555554";
    ASSERT_EQ(0xAAAAAAAAAAAAAAAA, result_lane1)
        << "0x5555555555555555 << 1 should be 0xAAAAAAAAAAAAAAAA";

    // Test sign bit patterns
    uint64_t sign_bit = 0x8000000000000000;
    uint64_t not_sign_bit = 0x7FFFFFFFFFFFFFFF;
    ASSERT_TRUE(call_i64x2_shl(sign_bit, not_sign_bit, 1, result_lane0, result_lane1))
        << "Failed to execute sign bit pattern shift";
    ASSERT_EQ(0x0000000000000000, result_lane0)
        << "Sign bit << 1 should overflow to 0";
    ASSERT_EQ(0xFFFFFFFFFFFFFFFE, result_lane1)
        << "Max positive << 1 should be 0xFFFFFFFFFFFFFFFE";
}

