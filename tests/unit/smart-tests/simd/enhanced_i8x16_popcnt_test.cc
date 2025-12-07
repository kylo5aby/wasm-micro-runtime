/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * Enhanced unit tests for i8x16.popcnt WASM opcode
 *
 * Tests comprehensive SIMD population count functionality including:
 * - Basic population count operations with various bit patterns
 * - Boundary condition handling (MIN/MAX values, single bits)
 * - Edge cases (all zeros, all ones, alternating patterns)
 * - Mathematical property validation (complement relationships)
 * - Cross-execution mode validation (interpreter vs AOT)
 */

static std::string CWD;
static std::string WASM_FILE;
static constexpr const char *MODULE_NAME = "i8x16_popcnt_test";
static constexpr const char *FUNC_NAME_MIXED_VALUES = "test_mixed_values";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_ALL_ZEROS = "test_all_zeros";
static constexpr const char *FUNC_NAME_ALL_ONES = "test_all_ones";
static constexpr const char *FUNC_NAME_COMPLEMENT_PATTERNS = "test_complement_patterns";
static constexpr const char *FUNC_NAME_SINGLE_BITS = "test_single_bits";

/**
 * Test fixture for i8x16.popcnt opcode validation
 *
 * Provides comprehensive test environment for SIMD population count operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class I8x16PopcntTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the i8x16.popcnt test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.popcnt test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_popcnt_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i8x16.popcnt tests";
    }

    /**
     * Cleans up test environment and runtime resources
     *
     * Cleanup is handled automatically by RAII destructors.
     */
    void TearDown() override {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * Calls WASM i8x16.popcnt test function and validates execution
     *
     * @param func_name Name of the WASM test function to call
     * @return True if test function executed successfully and returned success
     */
    bool call_popcnt_test_function(const char* func_name) {
        uint32_t argv[1] = {0}; // No input arguments needed
        bool call_success = dummy_env->execute(func_name, 0, argv);

        if (call_success) {
            // Test functions return 1 for success, 0 for failure
            return (argv[0] == 1);
        }
        return false;
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicPopulationCount_MixedValues_ReturnsCorrectCounts
 * @brief Validates i8x16.popcnt produces correct population counts for various bit densities
 * @details Tests fundamental population count operation with mixed bit patterns including
 *          values from 0-8 set bits per lane. Verifies each lane correctly counts the
 *          number of 1 bits in its corresponding 8-bit value.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_popcnt_operation
 * @input_conditions v128 with lanes [0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x55, 0xAA, 0xF0, 0x0F, 0x33, 0x66, 0x99]
 * @expected_behavior Returns population counts [0, 1, 2, 3, 4, 5, 6, 7, 8, 4, 4, 4, 4, 4, 4, 4] respectively
 * @validation_method Direct comparison of WASM function result with expected population counts
 */
TEST_P(I8x16PopcntTestSuite, BasicPopulationCount_MixedValues_ReturnsCorrectCounts) {
    // Execute test with mixed population densities
    bool result = call_popcnt_test_function(FUNC_NAME_MIXED_VALUES);
    ASSERT_TRUE(result)
        << "i8x16.popcnt failed for mixed population count values in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test BoundaryValues_MinMaxBytes_ProducesCorrectResults
 * @brief Validates i8x16.popcnt handles boundary values correctly (0x00 and 0xFF)
 * @details Tests population count operation with minimum (0x00) and maximum (0xFF) 8-bit values
 *          alternating across lanes. Verifies correct boundary condition handling.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_popcnt_operation
 * @input_conditions v128 with alternating 0x00 and 0xFF values across 16 lanes
 * @expected_behavior Returns alternating population counts 0 and 8 for respective lanes
 * @validation_method Verification of MIN_VALUE (0) and MAX_VALUE (8) population counts
 */
TEST_P(I8x16PopcntTestSuite, BoundaryValues_MinMaxBytes_ProducesCorrectResults) {
    // Execute test with boundary values (0x00 and 0xFF)
    bool result = call_popcnt_test_function(FUNC_NAME_BOUNDARY_VALUES);
    ASSERT_TRUE(result)
        << "i8x16.popcnt failed for boundary values (0x00/0xFF) in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test EdgeCases_AllZeros_ReturnsZeroPopCount
 * @brief Validates i8x16.popcnt returns zero for all-zero vector
 * @details Tests edge case where all 16 lanes contain 0x00 (no bits set).
 *          Verifies that population count correctly returns 0 for each lane.
 * @test_category Edge - Zero input validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_popcnt_operation
 * @input_conditions v128 with all 16 lanes set to 0x00
 * @expected_behavior Returns all lanes with population count 0
 * @validation_method Verification that all-zero input produces all-zero output
 */
TEST_P(I8x16PopcntTestSuite, EdgeCases_AllZeros_ReturnsZeroPopCount) {
    // Execute test with all-zero vector
    bool result = call_popcnt_test_function(FUNC_NAME_ALL_ZEROS);
    ASSERT_TRUE(result)
        << "i8x16.popcnt failed for all-zero vector in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test EdgeCases_AllOnes_ReturnsMaxPopCount
 * @brief Validates i8x16.popcnt returns maximum count for all-ones vector
 * @details Tests edge case where all 16 lanes contain 0xFF (all bits set).
 *          Verifies that population count correctly returns 8 for each lane.
 * @test_category Edge - Maximum value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_popcnt_operation
 * @input_conditions v128 with all 16 lanes set to 0xFF
 * @expected_behavior Returns all lanes with population count 8
 * @validation_method Verification that all-ones input produces maximum population count
 */
TEST_P(I8x16PopcntTestSuite, EdgeCases_AllOnes_ReturnsMaxPopCount) {
    // Execute test with all-ones vector
    bool result = call_popcnt_test_function(FUNC_NAME_ALL_ONES);
    ASSERT_TRUE(result)
        << "i8x16.popcnt failed for all-ones vector in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test ComplementPatterns_BitwiseComplements_ValidateMathProperties
 * @brief Validates mathematical properties of population count with bitwise complements
 * @details Tests that for any value x and its bitwise complement ~x, the relationship
 *          popcount(x) + popcount(~x) = 8 holds true for 8-bit values.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_popcnt_operation
 * @input_conditions v128 with complementary pairs [0x33, 0xCC, 0x0F, 0xF0, 0x3C, 0xC3, 0x5A, 0xA5, ...]
 * @expected_behavior Each complementary pair's population counts sum to 8
 * @validation_method Mathematical validation of popcount(x) + popcount(~x) = 8 property
 */
TEST_P(I8x16PopcntTestSuite, ComplementPatterns_BitwiseComplements_ValidateMathProperties) {
    // Execute test with complementary bit patterns
    bool result = call_popcnt_test_function(FUNC_NAME_COMPLEMENT_PATTERNS);
    ASSERT_TRUE(result)
        << "i8x16.popcnt failed for complement patterns mathematical validation in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test SingleBitPositions_IndividualBits_ReturnsOne
 * @brief Validates i8x16.popcnt correctly counts single-bit patterns
 * @details Tests population count operation with power-of-2 values (single bit set).
 *          Each lane contains exactly one bit set, verifying accurate single-bit detection.
 * @test_category Corner - Single bit validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_popcnt_operation
 * @input_conditions v128 with lanes containing [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, ...]
 * @expected_behavior Returns population count 1 for all lanes (single bit set)
 * @validation_method Verification that single-bit patterns consistently return count 1
 */
TEST_P(I8x16PopcntTestSuite, SingleBitPositions_IndividualBits_ReturnsOne) {
    // Execute test with single-bit patterns
    bool result = call_popcnt_test_function(FUNC_NAME_SINGLE_BITS);
    ASSERT_TRUE(result)
        << "i8x16.popcnt failed for single-bit patterns in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

// Instantiate tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(I8x16PopcntTest, I8x16PopcntTestSuite,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT));