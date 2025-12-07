/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"

/**
 * @brief Test class for i8x16.avgr_u opcode functionality validation
 * @details This test class validates the i8x16.avgr_u SIMD opcode which performs
 *          unsigned average with rounding operations on packed 8-bit integers.
 *          The operation computes (a[i] + b[i] + 1) >> 1 for each lane i using
 *          9-bit arithmetic to prevent overflow, then rounds up.
 *          Tests cover basic functionality, boundary conditions, edge cases,
 *          and cross-execution mode validation using comprehensive WAMR testing framework.
 */
class I8x16AvgrUTestSuite : public testing::Test {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     * @details Initializes WAMR runtime using RAII helper and loads the
     *          i8x16.avgr_u test WASM module for execution testing.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.avgr_u test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_avgr_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i8x16.avgr_u tests";
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
     * @brief Execute i8x16.avgr_u operation with two input vectors
     * @param vec_a Array of 16 bytes representing the first input vector
     * @param vec_b Array of 16 bytes representing the second input vector
     * @param result_values Output array to store the 16 result bytes
     * @return bool True if operation succeeded, false on error
     * @details Calls the WASM test function to perform i8x16.avgr_u operation
     *          using the formula: (vec_a[i] + vec_b[i] + 1) >> 1 for each lane i
     */
    bool call_i8x16_avgr_u(const uint8_t vec_a[16], const uint8_t vec_b[16], uint8_t result_values[16]) {
        uint32_t argv[34]; // 32 inputs + 2 result parameters

        // Pack input vectors into argv (each byte as uint32_t)
        for (int i = 0; i < 16; i++) {
            argv[i] = static_cast<uint32_t>(vec_a[i]);      // First vector lanes 0-15
            argv[i + 16] = static_cast<uint32_t>(vec_b[i]); // Second vector lanes 0-15
        }

        // Result pointer parameters
        argv[32] = 0; // Unused
        argv[33] = 0; // Unused

        // Execute the WASM function
        bool success = dummy_env->execute("test_i8x16_avgr_u", 34, argv);

        // Extract results from argv (returned via modified parameters)
        if (success) {
            for (int i = 0; i < 16; i++) {
                result_values[i] = static_cast<uint8_t>(argv[i]);
            }
        }

        return success;
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicAveraging_ReturnsCorrectResults
 * @brief Validates i8x16.avgr_u produces correct unsigned average results for typical inputs
 * @details Tests fundamental average operation with typical value pairs across all 16 lanes.
 *          Verifies that i8x16.avgr_u correctly computes (a[i] + b[i] + 1) >> 1 for various
 *          input combinations, ensuring proper rounding behavior.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_avgr_u_operation
 * @input_conditions Standard byte value pairs: (10,20), (50,100), (25,75), (1,1)
 * @expected_behavior Returns mathematical average with rounding: 15, 75, 50, 1 respectively
 * @validation_method Direct comparison of WASM function result with expected average values
 */
TEST_F(I8x16AvgrUTestSuite, BasicAveraging_ReturnsCorrectResults) {
    uint8_t vec_a[16] = {10, 50, 25, 1, 30, 60, 15, 8,
                         40, 70, 35, 5, 20, 80, 45, 12};
    uint8_t vec_b[16] = {20, 100, 75, 1, 40, 80, 25, 16,
                         60, 90, 55, 15, 30, 120, 65, 28};
    uint8_t result[16];

    // Execute the average operation
    ASSERT_TRUE(call_i8x16_avgr_u(vec_a, vec_b, result))
        << "Failed to execute i8x16.avgr_u with typical values";

    // Verify each lane result: (a[i] + b[i] + 1) >> 1
    ASSERT_EQ(15, result[0]) << "Average of (10+20+1)>>1 should be 15";
    ASSERT_EQ(75, result[1]) << "Average of (50+100+1)>>1 should be 75";
    ASSERT_EQ(50, result[2]) << "Average of (25+75+1)>>1 should be 50";
    ASSERT_EQ(1, result[3]) << "Average of (1+1+1)>>1 should be 1";
    ASSERT_EQ(35, result[4]) << "Average of (30+40+1)>>1 should be 35";
    ASSERT_EQ(70, result[5]) << "Average of (60+80+1)>>1 should be 70";
    ASSERT_EQ(20, result[6]) << "Average of (15+25+1)>>1 should be 20";
    ASSERT_EQ(12, result[7]) << "Average of (8+16+1)>>1 should be 12";
    ASSERT_EQ(50, result[8]) << "Average of (40+60+1)>>1 should be 50";
    ASSERT_EQ(80, result[9]) << "Average of (70+90+1)>>1 should be 80";
    ASSERT_EQ(45, result[10]) << "Average of (35+55+1)>>1 should be 45";
    ASSERT_EQ(10, result[11]) << "Average of (5+15+1)>>1 should be 10";
    ASSERT_EQ(25, result[12]) << "Average of (20+30+1)>>1 should be 25";
    ASSERT_EQ(100, result[13]) << "Average of (80+120+1)>>1 should be 100";
    ASSERT_EQ(55, result[14]) << "Average of (45+65+1)>>1 should be 55";
    ASSERT_EQ(20, result[15]) << "Average of (12+28+1)>>1 should be 20";
}

/**
 * @test BoundaryValues_HandlesMinMaxCorrectly
 * @brief Validates i8x16.avgr_u handles boundary conditions correctly
 * @details Tests boundary value combinations including (0,0), (255,255), and (0,255).
 *          Verifies proper handling of minimum and maximum unsigned 8-bit values
 *          and their averages with correct overflow prevention using 9-bit arithmetic.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_avgr_u_boundary_handling
 * @input_conditions Boundary combinations: (0,0), (255,255), (0,255), (1,254)
 * @expected_behavior Proper boundary handling: 0, 255, 128, 127 respectively
 * @validation_method Verification of boundary value arithmetic and overflow prevention
 */
TEST_F(I8x16AvgrUTestSuite, BoundaryValues_HandlesMinMaxCorrectly) {
    uint8_t vec_a[16] = {0, 255, 0, 1, 0, 255, 128, 64,
                         255, 0, 127, 200, 50, 150, 100, 175};
    uint8_t vec_b[16] = {0, 255, 255, 254, 1, 254, 127, 192,
                         0, 255, 128, 55, 205, 105, 155, 80};
    uint8_t result[16];

    // Execute the average operation
    ASSERT_TRUE(call_i8x16_avgr_u(vec_a, vec_b, result))
        << "Failed to execute i8x16.avgr_u with boundary values";

    // Verify boundary cases: (a + b + 1) >> 1
    ASSERT_EQ(0, result[0]) << "Average of (0+0+1)>>1 should be 0";
    ASSERT_EQ(255, result[1]) << "Average of (255+255+1)>>1 should be 255";
    ASSERT_EQ(128, result[2]) << "Average of (0+255+1)>>1 should be 128";
    ASSERT_EQ(128, result[3]) << "Average of (1+254+1)>>1 should be 128";
    ASSERT_EQ(1, result[4]) << "Average of (0+1+1)>>1 should be 1";
    ASSERT_EQ(255, result[5]) << "Average of (255+254+1)>>1 should be 255";
    ASSERT_EQ(128, result[6]) << "Average of (128+127+1)>>1 should be 128";
    ASSERT_EQ(128, result[7]) << "Average of (64+192+1)>>1 should be 128";
}

/**
 * @test RoundingBehavior_RoundsUpCorrectly
 * @brief Validates i8x16.avgr_u performs correct rounding for odd sums
 * @details Tests scenarios where sum is odd to verify the +1 rounding behavior.
 *          Ensures proper rounding up for fractional averages according to the
 *          WebAssembly specification for unsigned average with rounding.
 * @test_category Edge - Rounding validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_avgr_u_rounding
 * @input_conditions Value pairs that produce odd sums: (1,2), (3,4), (5,6), etc.
 * @expected_behavior Proper rounding up: 2, 4, 6 respectively (not 1, 3, 5)
 * @validation_method Verification of rounding behavior for fractional results
 */
TEST_F(I8x16AvgrUTestSuite, RoundingBehavior_RoundsUpCorrectly) {
    // Test values that create odd sums to verify rounding
    uint8_t vec_a[16] = {1, 3, 5, 7, 9, 11, 13, 15,
                         17, 19, 21, 23, 25, 27, 29, 31};
    uint8_t vec_b[16] = {2, 4, 6, 8, 10, 12, 14, 16,
                         18, 20, 22, 24, 26, 28, 30, 32};
    uint8_t result[16];

    // Execute the average operation
    ASSERT_TRUE(call_i8x16_avgr_u(vec_a, vec_b, result))
        << "Failed to execute i8x16.avgr_u with rounding test values";

    // Verify rounding behavior: (odd_sum + 1) >> 1 rounds up
    ASSERT_EQ(2, result[0]) << "Average of (1+2+1)>>1 should round up to 2";
    ASSERT_EQ(4, result[1]) << "Average of (3+4+1)>>1 should round up to 4";
    ASSERT_EQ(6, result[2]) << "Average of (5+6+1)>>1 should round up to 6";
    ASSERT_EQ(8, result[3]) << "Average of (7+8+1)>>1 should round up to 8";
    ASSERT_EQ(10, result[4]) << "Average of (9+10+1)>>1 should round up to 10";
    ASSERT_EQ(12, result[5]) << "Average of (11+12+1)>>1 should round up to 12";
    ASSERT_EQ(14, result[6]) << "Average of (13+14+1)>>1 should round up to 14";
    ASSERT_EQ(16, result[7]) << "Average of (15+16+1)>>1 should round up to 16";
    ASSERT_EQ(18, result[8]) << "Average of (17+18+1)>>1 should round up to 18";
    ASSERT_EQ(20, result[9]) << "Average of (19+20+1)>>1 should round up to 20";
    ASSERT_EQ(22, result[10]) << "Average of (21+22+1)>>1 should round up to 22";
    ASSERT_EQ(24, result[11]) << "Average of (23+24+1)>>1 should round up to 24";
    ASSERT_EQ(26, result[12]) << "Average of (25+26+1)>>1 should round up to 26";
    ASSERT_EQ(28, result[13]) << "Average of (27+28+1)>>1 should round up to 28";
    ASSERT_EQ(30, result[14]) << "Average of (29+30+1)>>1 should round up to 30";
    ASSERT_EQ(32, result[15]) << "Average of (31+32+1)>>1 should round up to 32";
}

/**
 * @test ZeroOperands_HandlesZeroValuesCorrectly
 * @brief Validates i8x16.avgr_u handles zero operands correctly
 * @details Tests scenarios with zero values in various lane combinations
 *          to ensure proper handling of zero inputs and mathematical correctness.
 * @test_category Edge - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_avgr_u_zero_handling
 * @input_conditions Zero combinations: (0,x), (x,0), (0,0) patterns
 * @expected_behavior Correct zero handling: (0+x+1)>>1, (x+0+1)>>1, (0+0+1)>>1
 * @validation_method Verification of zero value arithmetic and edge case handling
 */
TEST_F(I8x16AvgrUTestSuite, ZeroOperands_HandlesZeroValuesCorrectly) {
    uint8_t vec_a[16] = {0, 10, 0, 20, 0, 30, 0, 40,
                         0, 50, 0, 60, 0, 70, 0, 80};
    uint8_t vec_b[16] = {5, 0, 15, 0, 25, 0, 35, 0,
                         45, 0, 55, 0, 65, 0, 75, 0};
    uint8_t result[16];

    // Execute the average operation
    ASSERT_TRUE(call_i8x16_avgr_u(vec_a, vec_b, result))
        << "Failed to execute i8x16.avgr_u with zero values";

    // Verify zero handling: (0+x+1)>>1 and (x+0+1)>>1
    ASSERT_EQ(3, result[0]) << "Average of (0+5+1)>>1 should be 3";
    ASSERT_EQ(5, result[1]) << "Average of (10+0+1)>>1 should be 5";
    ASSERT_EQ(8, result[2]) << "Average of (0+15+1)>>1 should be 8";
    ASSERT_EQ(10, result[3]) << "Average of (20+0+1)>>1 should be 10";
    ASSERT_EQ(13, result[4]) << "Average of (0+25+1)>>1 should be 13";
    ASSERT_EQ(15, result[5]) << "Average of (30+0+1)>>1 should be 15";
    ASSERT_EQ(18, result[6]) << "Average of (0+35+1)>>1 should be 18";
    ASSERT_EQ(20, result[7]) << "Average of (40+0+1)>>1 should be 20";
}

/**
 * @test SIMDFeatureValidation_ModuleLoadsSuccessfully
 * @brief Validates SIMD feature support and module loading functionality
 * @details Tests that the WASM module with i8x16.avgr_u instruction loads successfully
 *          and that SIMD features are properly supported in the current WAMR build.
 * @test_category Main - Feature validation
 * @coverage_target core/iwasm/common/wasm_loader.c:simd_feature_validation
 * @input_conditions Valid WASM module with SIMD instructions
 * @expected_behavior Successful module loading and instantiation
 * @validation_method Module loading verification and execution environment setup
 */
TEST_F(I8x16AvgrUTestSuite, SIMDFeatureValidation_ModuleLoadsSuccessfully) {
    // Verify module loaded successfully in SetUp
    ASSERT_NE(nullptr, dummy_env->get())
        << "SIMD module should load successfully when SIMD support is enabled";

    // Test a simple operation to verify functionality
    uint8_t vec_a[16] = {100, 100, 100, 100, 100, 100, 100, 100,
                         100, 100, 100, 100, 100, 100, 100, 100};
    uint8_t vec_b[16] = {200, 200, 200, 200, 200, 200, 200, 200,
                         200, 200, 200, 200, 200, 200, 200, 200};
    uint8_t result[16];

    // Execute to verify SIMD functionality works
    ASSERT_TRUE(call_i8x16_avgr_u(vec_a, vec_b, result))
        << "SIMD feature validation should execute successfully";

    // Verify expected result for all lanes
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(150, result[i]) << "Lane " << i << " should have average 150";
    }
}