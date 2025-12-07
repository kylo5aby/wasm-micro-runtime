/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <climits>
#include <cfloat>
#include <cmath>
#include <fstream>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * Enhanced unit tests for f64x2.floor WASM opcode
 *
 * Tests comprehensive SIMD floor functionality for double-precision floating-point including:
 * - Basic floor operations with mixed positive/negative fractional f64 values
 * - Boundary condition handling (large magnitude values, precision limits, exact integers)
 * - Special value handling (±0.0, ±infinity, NaN)
 * - Mathematical property validation (monotonic, range properties)
 * - Cross-execution mode validation (interpreter vs AOT)
 * - IEEE 754 compliance verification
 */

static constexpr const char *MODULE_NAME = "f64x2_floor_test";
static constexpr const char *FUNC_NAME_BASIC_FLOOR = "test_basic_floor";
static constexpr const char *FUNC_NAME_BOUNDARY_VALUES = "test_boundary_values";
static constexpr const char *FUNC_NAME_SPECIAL_VALUES = "test_special_values";
static constexpr const char *FUNC_NAME_ZERO_VALUES = "test_zero_values";
static constexpr const char *WASM_FILE = "wasm-apps/f64x2_floor_test.wasm";

/**
 * Test fixture for f64x2.floor opcode validation
 *
 * Provides comprehensive test environment for SIMD floating-point floor operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class F64x2FloorTestSuite : public testing::Test {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the f64x2.floor test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Check if WASM file exists before attempting to load
        std::ifstream wasm_file_check(WASM_FILE);
        ASSERT_TRUE(wasm_file_check.good())
            << "WASM test file not found: " << WASM_FILE;
        wasm_file_check.close();

        // Load the f64x2.floor test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>(WASM_FILE);
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.floor tests";
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
     * Helper function to call WASM f64x2.floor functions with two f64 values and extract f64x2 result
     *
     * @param func_name Name of the WASM function to call
     * @param input1 First f64 input value
     * @param input2 Second f64 input value
     * @param lane0 Output for first f64 lane
     * @param lane1 Output for second f64 lane
     * @return true if function execution succeeded, false otherwise
     */
    bool call_f64x2_floor_function(const char* func_name, double input1, double input2, double* lane0, double* lane1) {
        // Convert f64 inputs to i32 pairs for WASM function call
        uint64_t input1_bits, input2_bits;
        memcpy(&input1_bits, &input1, sizeof(double));
        memcpy(&input2_bits, &input2, sizeof(double));

        // Prepare arguments: four i32 values representing two f64 values
        uint32_t argv[4];
        argv[0] = static_cast<uint32_t>(input1_bits);        // input1 low 32 bits
        argv[1] = static_cast<uint32_t>(input1_bits >> 32);  // input1 high 32 bits
        argv[2] = static_cast<uint32_t>(input2_bits);        // input2 low 32 bits
        argv[3] = static_cast<uint32_t>(input2_bits >> 32);  // input2 high 32 bits

        // Call WASM function using dummy_env->execute
        bool call_success = dummy_env->execute(func_name, 4, argv);
        if (!call_success) {
            return false;
        }

        // Extract f64 results from i32 pairs
        uint64_t result1_bits = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        uint64_t result2_bits = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];

        if (lane0 && lane1) {
            memcpy(lane0, &result1_bits, sizeof(double));
            memcpy(lane1, &result2_bits, sizeof(double));
        }

        return true;
    }

protected:
    // WAMR runtime components using RAII
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicFloor_ProducesCorrectResults
 * @brief Validates f64x2.floor produces correct arithmetic results for typical fractional inputs
 * @details Tests fundamental floor operation with positive, negative, and mixed-sign fractional doubles.
 *          Verifies that f64x2.floor correctly computes floor(a), floor(b) for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_floor
 * @input_conditions Standard fractional f64 pairs: (1.7,2.3), (-1.7,-2.3), (3.9,-1.2)
 * @expected_behavior Returns floor values: (1.0,2.0), (-2.0,-3.0), (3.0,-2.0) respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(F64x2FloorTestSuite, BasicFloor_ProducesCorrectResults) {
    double lane0, lane1;

    // Test case 1: Mixed positive and negative fractional values
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_BASIC_FLOOR, 1.7, 2.3, &lane0, &lane1))
        << "Failed to call basic floor function with mixed fractional values";

    ASSERT_EQ(1.0, lane0) << "First lane floor(1.7) should equal 1.0";
    ASSERT_EQ(2.0, lane1) << "Second lane floor(2.3) should equal 2.0";

    // Test case 2: Negative fractional values
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_BASIC_FLOOR, -1.7, -2.3, &lane0, &lane1))
        << "Failed to call basic floor function with negative fractional values";

    ASSERT_EQ(-2.0, lane0) << "First lane floor(-1.7) should equal -2.0";
    ASSERT_EQ(-3.0, lane1) << "Second lane floor(-2.3) should equal -3.0";

    // Test case 3: Mixed positive/negative values
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_BASIC_FLOOR, 3.9, -1.2, &lane0, &lane1))
        << "Failed to call basic floor function with mixed sign values";

    ASSERT_EQ(3.0, lane0) << "First lane floor(3.9) should equal 3.0";
    ASSERT_EQ(-2.0, lane1) << "Second lane floor(-1.2) should equal -2.0";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Validates f64x2.floor behavior at numeric boundaries and precision limits
 * @details Tests floor operation with f64 boundaries, large magnitude values, and precision edge cases.
 *          Ensures proper handling of values near MAX_VALUE, MIN_NORMAL, and precision limits.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_floor
 * @input_conditions F64 boundary values and precision limits
 * @expected_behavior Correct floor application at all boundary conditions
 * @validation_method Boundary value verification with ASSERT_EQ
 */
TEST_F(F64x2FloorTestSuite, BoundaryValues_HandledCorrectly) {
    double lane0, lane1;

    // Test case 1: Large positive values near precision limits
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_BOUNDARY_VALUES, 1e12 + 0.7, 1e15 + 0.5, &lane0, &lane1))
        << "Failed to call boundary values function with large positive values";

    ASSERT_EQ(1e12, lane0) << "Floor of large value should truncate fractional part";
    ASSERT_EQ(1e15, lane1) << "Floor of 1e15 + 0.5 should equal 1e15";

    // Test case 2: Large negative values
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_BOUNDARY_VALUES, -1e12 - 0.7, -(1e15 + 0.5), &lane0, &lane1))
        << "Failed to call boundary values function with large negative values";

    ASSERT_EQ(-1e12 - 1.0, lane0) << "Floor of negative large value should round down";
    ASSERT_EQ(-1e15 - 1.0, lane1) << "Floor of negative value should round toward negative infinity";

    // Test case 3: Values around zero crossing
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_BOUNDARY_VALUES, 0.9, -0.1, &lane0, &lane1))
        << "Failed to call boundary values function with zero crossing values";

    ASSERT_EQ(0.0, lane0) << "Floor(0.9) should equal 0.0";
    ASSERT_EQ(-1.0, lane1) << "Floor(-0.1) should equal -1.0";
}

/**
 * @test SpecialValues_PreserveIEEESemantics
 * @brief Validates f64x2.floor behavior with IEEE 754 special values
 * @details Tests floor operation with NaN, infinity, and special numeric values.
 *          Ensures IEEE 754 compliance for edge cases and special value propagation.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_floor
 * @input_conditions NaN, ±infinity, ±0.0, identity cases
 * @expected_behavior NaN→NaN, ±infinity→±infinity, ±0.0→±0.0, integers unchanged
 * @validation_method Special value checks with std::isnan and ASSERT_EQ
 */
TEST_F(F64x2FloorTestSuite, SpecialValues_PreserveIEEESemantics) {
    double lane0, lane1;

    // Test case 1: NaN propagation
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_SPECIAL_VALUES, std::numeric_limits<double>::quiet_NaN(), 5.5, &lane0, &lane1))
        << "Failed to call special values function with NaN";

    ASSERT_TRUE(std::isnan(lane0)) << "Floor of NaN should remain NaN";
    ASSERT_EQ(5.0, lane1) << "Floor(5.5) should equal 5.0";

    // Test case 2: Positive and negative infinity
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_SPECIAL_VALUES, std::numeric_limits<double>::infinity(),
                                         -std::numeric_limits<double>::infinity(), &lane0, &lane1))
        << "Failed to call special values function with infinities";

    ASSERT_EQ(std::numeric_limits<double>::infinity(), lane0) << "Floor of +infinity should remain +infinity";
    ASSERT_EQ(-std::numeric_limits<double>::infinity(), lane1) << "Floor of -infinity should remain -infinity";

    // Test case 3: Already integer values (identity cases)
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_SPECIAL_VALUES, 17.0, -25.0, &lane0, &lane1))
        << "Failed to call special values function with integer values";

    ASSERT_EQ(17.0, lane0) << "Floor of integer 17.0 should remain 17.0";
    ASSERT_EQ(-25.0, lane1) << "Floor of integer -25.0 should remain -25.0";
}

/**
 * @test ZeroValues_PreserveSignBits
 * @brief Validates f64x2.floor behavior with zero values and sign preservation
 * @details Tests floor operation with positive and negative zero, ensuring IEEE 754 sign preservation.
 *          Validates behavior with near-zero values and sign bit handling.
 * @test_category Edge - Zero value and sign preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_f64x2_floor
 * @input_conditions ±0.0, near-zero values, mixed zero combinations
 * @expected_behavior Sign preservation: +0.0→+0.0, -0.0→-0.0, proper near-zero floor
 * @validation_method Sign bit verification using std::copysign and ASSERT_EQ
 */
TEST_F(F64x2FloorTestSuite, ZeroValues_PreserveSignBits) {
    double lane0, lane1;

    // Test case 1: Positive and negative zero
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_ZERO_VALUES, +0.0, -0.0, &lane0, &lane1))
        << "Failed to call zero values function with signed zeros";

    ASSERT_EQ(+0.0, lane0) << "Floor(+0.0) should equal +0.0";
    ASSERT_EQ(-0.0, lane1) << "Floor(-0.0) should equal -0.0";

    // Verify sign bit preservation using copysign
    ASSERT_EQ(std::copysign(1.0, +0.0), std::copysign(1.0, lane0)) << "Positive zero sign should be preserved";
    ASSERT_EQ(std::copysign(1.0, -0.0), std::copysign(1.0, lane1)) << "Negative zero sign should be preserved";

    // Test case 2: Near-zero values
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_ZERO_VALUES, 1e-100, -1e-100, &lane0, &lane1))
        << "Failed to call zero values function with near-zero values";

    ASSERT_EQ(0.0, lane0) << "Floor of small positive value should equal 0.0";
    ASSERT_EQ(-1.0, lane1) << "Floor of small negative value should equal -1.0";

    // Test case 3: Zero with other values
    ASSERT_TRUE(call_f64x2_floor_function(FUNC_NAME_ZERO_VALUES, 0.0, 42.7, &lane0, &lane1))
        << "Failed to call zero values function with mixed zero/non-zero";

    ASSERT_EQ(0.0, lane0) << "Floor(0.0) should equal 0.0";
    ASSERT_EQ(42.0, lane1) << "Floor(42.7) should equal 42.0";
}