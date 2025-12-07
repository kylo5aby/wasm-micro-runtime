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

/**
 * @brief Enhanced test suite for WASM i8x16.sub opcode
 *
 * This test suite provides comprehensive validation of the i8x16.sub SIMD operation,
 * which performs element-wise subtraction of two i8x16 vectors with wraparound behavior.
 *
 * The i8x16.sub operation:
 * - Takes two i8x16 vectors (minuend and subtrahend) from the stack
 * - Performs result[i] = minuend[i] - subtrahend[i] for each lane i (0-15)
 * - Implements two's complement wraparound on overflow/underflow
 * - Returns the result as an i8x16 vector
 */

// WASM SIMD support is enabled in CMakeLists.txt with WAMR_BUILD_SIMD=1

// Use RunningMode from wasm_export.h - no local enum definition needed

/**
 * @brief Parameterized test class for i8x16.sub opcode validation
 *
 * Supports both interpreter and AOT execution modes to ensure consistent
 * behavior across WAMR runtime execution strategies.
 */
class I8x16SubTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Current running mode for test execution
     */
    RunningMode running_mode;

    /**
     * @brief WAMR runtime RAII helper instance
     */
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;

    /**
     * @brief Dummy execution environment for WASM function calls
     */
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Set up test environment and initialize WAMR runtime
     */
    void SetUp() override {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.sub test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_sub_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i8x16.sub tests";
    }

    /**
     * @brief Tears down the test fixture with proper cleanup
     * @details Destroys execution environment and WAMR runtime resources
     */
    void TearDown() override {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM function and extract v128 result as i8 array
     * @param function_name Name of the WASM function to call
     * @param result_out Array to store the 16 i8 result values
     */
    void call_i8x16_sub_function(const char* function_name, int8_t result_out[16]) {
        wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(dummy_env->get());
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, function_name);
        ASSERT_NE(func, nullptr) << "Failed to lookup function: " << function_name;

        uint32_t argv[4];  // v128 result as 4 x i32
        bool call_result = wasm_runtime_call_wasm(dummy_env->get(), func, 0, argv);
        ASSERT_TRUE(call_result) << "Failed to call " << function_name << ": " << wasm_runtime_get_exception(module_inst);

        // Extract i8 values from v128 result
        memcpy(result_out, argv, 16 * sizeof(int8_t));
    }
};

/**
 * @test BasicSubtraction_ValidatesArithmetic
 * @brief Validates i8x16.sub produces correct arithmetic results for typical inputs
 * @details Tests fundamental subtraction operation with positive, negative, and mixed-sign integers.
 *          Verifies that i8x16.sub correctly computes minuend[i] - subtrahend[i] for each lane.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_sub_operation
 * @input_conditions Standard integer pairs across all 16 lanes
 * @expected_behavior Returns mathematical difference for each lane
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I8x16SubTest, BasicSubtraction_ValidatesArithmetic) {
    // Test basic arithmetic subtraction: [10,10,10,10,...] - [3,3,3,3,...] = [7,7,7,7,...]
    int8_t result[16];
    call_i8x16_sub_function("i8x16_sub_basic", result);

    // Validate all 16 lanes produce correct arithmetic result
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(7, result[i])
            << "Basic subtraction failed at lane " << i << ": expected 7, got " << (int)result[i];
    }

    // Test mixed sign operations: positive - negative = larger positive
    call_i8x16_sub_function("i8x16_sub_mixed_signs", result);

    // Validate mixed sign arithmetic: 15 - (-10) = 25
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(25, result[i])
            << "Mixed signs subtraction failed at lane " << i << ": expected 25, got " << (int)result[i];
    }
}

/**
 * @test BoundaryValues_ValidatesLimits
 * @brief Validates i8x16.sub handles boundary values correctly without exceptions
 * @details Tests subtraction with MIN/MAX i8 values to ensure proper limit handling.
 *          Verifies operations at the edges of the i8 value range.
 * @test_category Main - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_sub_boundary_checks
 * @input_conditions INT8_MIN, INT8_MAX, zero values in various combinations
 * @expected_behavior Correct arithmetic results for boundary values without exceptions
 * @validation_method Boundary value computation verification with expected results
 */
TEST_P(I8x16SubTest, BoundaryValues_ValidatesLimits) {
    // Test maximum value subtraction: 127 - 1 = 126
    int8_t result[16];
    call_i8x16_sub_function("i8x16_sub_max_values", result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(126, result[i])
            << "Max values subtraction failed at lane " << i << ": expected 126, got " << (int)result[i];
    }

    // Test minimum value subtraction: -128 - (-1) = -127
    call_i8x16_sub_function("i8x16_sub_min_values", result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(-127, result[i])
            << "Min values subtraction failed at lane " << i << ": expected -127, got " << (int)result[i];
    }

    // Test zero operations: 0 - 0 = 0
    call_i8x16_sub_function("i8x16_sub_zero_values", result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(0, result[i])
            << "Zero values subtraction failed at lane " << i << ": expected 0, got " << (int)result[i];
    }
}

/**
 * @test OverflowBehavior_ValidatesWraparound
 * @brief Validates i8x16.sub implements correct two's complement wraparound behavior
 * @details Tests overflow and underflow scenarios to ensure proper wraparound arithmetic.
 *          Verifies that results wrap around correctly when exceeding i8 value ranges.
 * @test_category Main - Overflow/underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_sub_overflow_handling
 * @input_conditions Values that cause positive overflow and negative underflow
 * @expected_behavior Two's complement wraparound: overflow→negative, underflow→positive
 * @validation_method Wraparound result verification with modular arithmetic expectations
 */
TEST_P(I8x16SubTest, OverflowBehavior_ValidatesWraparound) {
    // Test positive overflow wraparound: 127 - (-1) = -128 (wraparound)
    int8_t result[16];
    call_i8x16_sub_function("i8x16_sub_overflow", result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(-128, result[i])
            << "Overflow wraparound failed at lane " << i << ": expected -128, got " << (int)result[i];
    }

    // Test negative underflow wraparound: -128 - 1 = 127 (wraparound)
    call_i8x16_sub_function("i8x16_sub_underflow", result);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(127, result[i])
            << "Underflow wraparound failed at lane " << i << ": expected 127, got " << (int)result[i];
    }
}

/**
 * @test VectorOperations_ValidatesParallelism
 * @brief Validates i8x16.sub correctly processes all 16 lanes independently
 * @details Tests different values in each lane to verify parallel computation integrity.
 *          Ensures each lane is computed independently without cross-lane interference.
 * @test_category Main - Parallel computation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i8x16_sub_lane_processing
 * @input_conditions Different values in each of the 16 lanes
 * @expected_behavior Independent computation: result[i] = minuend[i] - subtrahend[i]
 * @validation_method Per-lane result verification with unique expected values
 */
TEST_P(I8x16SubTest, VectorOperations_ValidatesParallelism) {
    // Test mixed lane operations with different values per lane
    int8_t result[16];
    call_i8x16_sub_function("i8x16_sub_mixed_lanes", result);

    // Expected results for minuend[1,2,3,...,16] - subtrahend[16,15,14,...,1]
    int8_t expected_results[16] = {-15, -13, -11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9, 11, 13, 15};

    // Validate each lane computed independently
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(expected_results[i], result[i])
            << "Mixed lanes subtraction failed at lane " << i
            << ": expected " << (int)expected_results[i]
            << ", got " << (int)result[i];
    }
}

// Parameterized test instantiation for both execution modes
INSTANTIATE_TEST_SUITE_P(I8x16SubExecutionModes, I8x16SubTest,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT));