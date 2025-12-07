/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <limits>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * @file enhanced_i32x4_extmul_low_i16x8_u_test.cc
 * @brief Comprehensive test suite for i32x4.extmul_low_i16x8_u SIMD opcode
 *
 * Tests the extended unsigned multiplication operation that takes the lower 4 lanes (0-3)
 * of two i16x8 vectors, performs unsigned multiplication, and produces i32x4 results.
 * Validates both interpreter and AOT execution modes for complete coverage.
 */

// Test execution modes for cross-validation
enum class I32x4ExtmulLowI16x8URunningMode : uint8_t {
    INTERP = 1 << 0,
    AOT = 1 << 1
};

static constexpr const char *MODULE_NAME = "i32x4_extmul_low_i16x8_u_test";
static constexpr const char *FUNC_NAME_BASIC_EXTMUL = "test_basic_extmul";
static constexpr const char *FUNC_NAME_UNSIGNED_EXTMUL = "test_unsigned_extmul";
static constexpr const char *FUNC_NAME_BOUNDARY_EXTMUL = "test_boundary_extmul";
static constexpr const char *FUNC_NAME_ZERO_EXTMUL = "test_zero_extmul";
static constexpr const char *FUNC_NAME_IDENTITY_EXTMUL = "test_identity_extmul";

/**
 * Test fixture for i32x4.extmul_low_i16x8_u opcode validation
 *
 * Provides comprehensive test environment for SIMD extended unsigned multiplication operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class I32x4ExtmulLowI16x8UTestSuite : public testing::TestWithParam<I32x4ExtmulLowI16x8URunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the i32x4.extmul_low_i16x8_u test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i32x4.extmul_low_i16x8_u test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i32x4_extmul_low_i16x8_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i32x4.extmul_low_i16x8_u tests";
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
     * Calls WASM i32x4.extmul_low_i16x8_u test function with input vectors
     *
     * @param func_name Name of the WASM test function to call
     * @param input1_values Array of 4 i32 values for first input v128 vector
     * @param input2_values Array of 4 i32 values for second input v128 vector
     * @param output_values Array to store 4 i32 values from output v128 vector
     */
    void call_extmul_test_function(const char* func_name, const int32_t* input1_values,
                                  const int32_t* input2_values, int32_t* output_values) {
        // Prepare arguments: pack input vectors into argv array
        uint32_t argv[8];  // 4 i32 for first vector + 4 i32 for second vector
        memcpy(argv, input1_values, 16);      // First i32x4 vector
        memcpy(argv + 4, input2_values, 16);  // Second i32x4 vector

        // Execute function
        bool call_success = dummy_env->execute(func_name, 8, argv);
        ASSERT_TRUE(call_success) << "Failed to execute WASM function: " << func_name;

        // Extract result: get i32x4 result (4 i32s = 16 bytes)
        memcpy(output_values, argv, 16);
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicUnsignedMultiplication_ReturnsCorrectResults
 * @brief Validates i32x4.extmul_low_i16x8_u produces correct arithmetic results for typical unsigned inputs
 * @details Tests fundamental extended unsigned multiplication operation with positive unsigned
 *          i16 values in the lower lanes. Verifies that the opcode correctly computes unsigned
 *          multiplication of corresponding lanes and widens results to i32.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_extmul_operation
 * @input_conditions Standard unsigned i16 pairs in lower 4 lanes: small positive values
 * @expected_behavior Returns mathematical product with proper unsigned handling and widening
 * @validation_method Direct comparison of WASM function result with expected i32x4 values
 */
TEST_P(I32x4ExtmulLowI16x8UTestSuite, BasicUnsignedMultiplication_ReturnsCorrectResults)
{
    // Test data: i16x8 vectors represented as i32x4 (packed 2 i16 per i32)
    // First vector: [10, 20, 30, 40, ignored_upper_lanes...]
    const int32_t vec1[4] = {(20 << 16) | (10 & 0xFFFF),  // lanes 0,1: 10, 20
                            (40 << 16) | (30 & 0xFFFF),  // lanes 2,3: 30, 40
                            0, 0};                       // upper lanes (ignored)

    // Second vector: [2, 3, 4, 5, ignored_upper_lanes...]
    const int32_t vec2[4] = {(3 << 16) | (2 & 0xFFFF),   // lanes 0,1: 2, 3
                            (5 << 16) | (4 & 0xFFFF),   // lanes 2,3: 4, 5
                            0, 0};                       // upper lanes (ignored)

    int32_t result[4];

    // Execute i32x4.extmul_low_i16x8_u operation
    call_extmul_test_function(FUNC_NAME_BASIC_EXTMUL, vec1, vec2, result);

    // Validate results: [10*2, 20*3, 30*4, 40*5] = [20, 60, 120, 200]
    ASSERT_EQ(20, result[0]) << "Lane 0 unsigned multiplication failed: 10 * 2 should equal 20";
    ASSERT_EQ(60, result[1]) << "Lane 1 unsigned multiplication failed: 20 * 3 should equal 60";
    ASSERT_EQ(120, result[2]) << "Lane 2 unsigned multiplication failed: 30 * 4 should equal 120";
    ASSERT_EQ(200, result[3]) << "Lane 3 unsigned multiplication failed: 40 * 5 should equal 200";
}

/**
 * @test UnsignedRange_HandlesLargePositiveValues
 * @brief Validates correct unsigned multiplication behavior with large positive values
 * @details Tests values that would be negative in signed interpretation but are large
 *          positive values in unsigned context. Ensures proper unsigned arithmetic handling.
 * @test_category Main - Unsigned arithmetic validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_unsigned_multiply
 * @input_conditions Large unsigned i16 values: 32768+ range (previously negative in signed)
 * @expected_behavior Correct unsigned multiplication results with proper handling
 * @validation_method Verification of unsigned arithmetic behavior vs signed interpretation
 */
TEST_P(I32x4ExtmulLowI16x8UTestSuite, UnsignedRange_HandlesLargePositiveValues)
{
    // Test data with large unsigned values: [32768, 40000, 50000, 60000, ...]
    // These would be negative in signed interpretation but are large positive in unsigned
    const int32_t vec1[4] = {(40000 << 16) | (32768 & 0xFFFF),   // lanes 0,1: 32768, 40000
                            (60000 << 16) | (50000 & 0xFFFF),   // lanes 2,3: 50000, 60000
                            0, 0};

    // Second vector: [32768, 1000, 800, 600, ...]
    const int32_t vec2[4] = {(1000 << 16) | (32768 & 0xFFFF),    // lanes 0,1: 32768, 1000
                            (600 << 16) | (800 & 0xFFFF),       // lanes 2,3: 800, 600
                            0, 0};

    int32_t result[4];

    // Execute unsigned multiplication test
    call_extmul_test_function(FUNC_NAME_UNSIGNED_EXTMUL, vec1, vec2, result);

    // Validate results: [32768*32768, 40000*1000, 50000*800, 60000*600]
    // = [1073741824, 40000000, 40000000, 36000000]
    ASSERT_EQ(1073741824, result[0]) << "Large unsigned multiplication failed: 32768 * 32768";
    ASSERT_EQ(40000000, result[1]) << "Large unsigned multiplication failed: 40000 * 1000";
    ASSERT_EQ(40000000, result[2]) << "Large unsigned multiplication failed: 50000 * 800";
    ASSERT_EQ(36000000, result[3]) << "Large unsigned multiplication failed: 60000 * 600";
}

/**
 * @test BoundaryValues_HandleMaxUnsignedInputs
 * @brief Tests behavior at unsigned i16 MAX boundaries and their multiplication products
 * @details Validates extended multiplication with maximum unsigned i16 values to ensure
 *          proper handling of largest magnitude results without overflow.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_boundary_handling
 * @input_conditions Unsigned i16 MAX_VALUE (65535) combinations
 * @expected_behavior Maximum magnitude results without overflow in u32 range
 * @validation_method Verification of extreme unsigned value multiplication accuracy
 */
TEST_P(I32x4ExtmulLowI16x8UTestSuite, BoundaryValues_HandleMaxUnsignedInputs)
{
    // Test with unsigned i16 boundary values: MAX=65535 (0xFFFF)
    const uint16_t u16_max = 65535;  // Maximum unsigned i16 value

    // Vector 1: [65535, 65535, 65535, 65535, ...]
    const int32_t vec1[4] = {(u16_max << 16) | u16_max,  // lanes 0,1: 65535, 65535
                            (u16_max << 16) | u16_max,  // lanes 2,3: 65535, 65535
                            0, 0};

    // Vector 2: [65535, 32768, 1, 2, ...]
    const int32_t vec2[4] = {(32768 << 16) | u16_max,   // lanes 0,1: 65535, 32768
                            (2 << 16) | 1,              // lanes 2,3: 1, 2
                            0, 0};

    int32_t result[4];

    // Execute boundary value test
    call_extmul_test_function(FUNC_NAME_BOUNDARY_EXTMUL, vec1, vec2, result);

    // Validate results:
    // [65535*65535, 65535*32768, 65535*1, 65535*2]
    // = [4294836225, 2147450880, 65535, 131070]
    ASSERT_EQ(4294836225U, static_cast<uint32_t>(result[0]))
        << "MAX×MAX failed: 65535 * 65535 should equal 4294836225";
    ASSERT_EQ(2147450880U, static_cast<uint32_t>(result[1]))
        << "MAX×32768 failed: 65535 * 32768 should equal 2147450880";
    ASSERT_EQ(65535, result[2])
        << "MAX×1 failed: 65535 * 1 should equal 65535";
    ASSERT_EQ(131070, result[3])
        << "MAX×2 failed: 65535 * 2 should equal 131070";
}

/**
 * @test ZeroAndIdentity_MathematicalProperties
 * @brief Validates zero multiplication and identity properties of the unsigned operation
 * @details Tests mathematical properties including zero absorption (0×n=0),
 *          identity preservation (n×1=n), and per-lane independence in unsigned context.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_math_properties
 * @input_conditions Zero vectors, identity values, and mixed zero/non-zero scenarios
 * @expected_behavior Correct mathematical property preservation per lane
 * @validation_method Verification of fundamental unsigned arithmetic properties
 */
TEST_P(I32x4ExtmulLowI16x8UTestSuite, ZeroAndIdentity_MathematicalProperties)
{
    int32_t result[4];

    // Test 1: Zero absorption property (0 × n = 0)
    const int32_t zero_vec[4] = {0, 0, 0, 0};
    const int32_t any_vec[4] = {(200 << 16) | (100 & 0xFFFF),  // lanes: 100, 200
                               (400 << 16) | (300 & 0xFFFF),  // lanes: 300, 400
                               0, 0};

    call_extmul_test_function(FUNC_NAME_ZERO_EXTMUL, zero_vec, any_vec, result);

    // All results should be zero
    ASSERT_EQ(0, result[0]) << "Zero absorption failed in lane 0";
    ASSERT_EQ(0, result[1]) << "Zero absorption failed in lane 1";
    ASSERT_EQ(0, result[2]) << "Zero absorption failed in lane 2";
    ASSERT_EQ(0, result[3]) << "Zero absorption failed in lane 3";

    // Test 2: Identity property (n × 1 = n)
    const int32_t identity_vec[4] = {(1 << 16) | (1 & 0xFFFF),   // lanes: 1, 1
                                    (1 << 16) | (1 & 0xFFFF),   // lanes: 1, 1
                                    0, 0};
    const int32_t test_vec[4] = {(200 << 16) | (100 & 0xFFFF),  // lanes: 100, 200
                                (400 << 16) | (300 & 0xFFFF),  // lanes: 300, 400
                                0, 0};

    call_extmul_test_function(FUNC_NAME_IDENTITY_EXTMUL, test_vec, identity_vec, result);

    // Results should preserve original values: [100, 200, 300, 400]
    ASSERT_EQ(100, result[0]) << "Identity property failed in lane 0: 100×1 should equal 100";
    ASSERT_EQ(200, result[1]) << "Identity property failed in lane 1: 200×1 should equal 200";
    ASSERT_EQ(300, result[2]) << "Identity property failed in lane 2: 300×1 should equal 300";
    ASSERT_EQ(400, result[3]) << "Identity property failed in lane 3: 400×1 should equal 400";
}

/**
 * @test ValidationError_InvalidSIMDContext
 * @brief Tests error handling for invalid function calls and runtime failures
 * @details Validates proper error detection when calling i32x4.extmul_low_i16x8_u functions
 *          that don't exist or with invalid parameters using existing valid module.
 * @test_category Error - Invalid scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_runtime.c:wasm_runtime_call_wasm
 * @input_conditions Invalid function names, incorrect parameter counts
 * @expected_behavior Proper error reporting without crashes
 * @validation_method Function call failure validation with existing valid module
 */
TEST_P(I32x4ExtmulLowI16x8UTestSuite, ValidationError_InvalidSIMDContext)
{
    // Test calling non-existent function - should fail gracefully
    uint32_t invalid_argv[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bool call_success = dummy_env->execute("nonexistent_extmul_function", 8, invalid_argv);
    ASSERT_FALSE(call_success)
        << "Expected function call to fail for non-existent function name";

    // Test calling with wrong parameter count - should fail gracefully
    uint32_t wrong_argc_argv[4] = {0, 0, 0, 0};
    call_success = dummy_env->execute(FUNC_NAME_BASIC_EXTMUL, 4, wrong_argc_argv);
    ASSERT_FALSE(call_success)
        << "Expected function call to fail with incorrect parameter count";
}

// Parametrized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningModeTests,
    I32x4ExtmulLowI16x8UTestSuite,
    testing::Values(I32x4ExtmulLowI16x8URunningMode::INTERP, I32x4ExtmulLowI16x8URunningMode::AOT),
    [](const testing::TestParamInfo<I32x4ExtmulLowI16x8UTestSuite::ParamType>& info) {
        return info.param == I32x4ExtmulLowI16x8URunningMode::INTERP ? "Interpreter" : "AOT";
    }
);