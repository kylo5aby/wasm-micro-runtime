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
 * @brief Test fixture for i32x4.extend_low_i16x8 opcode comprehensive testing
 * @details This class provides setup and teardown for WAMR runtime environment,
 *          supporting both interpreter and AOT execution modes for validation
 *          of the i32x4.extend_low_i16x8 SIMD operation.
 */
class I32x4ExtendLowI16x8Test : public testing::TestWithParam<TestRunningMode>
{
protected:
    /**
     * @brief Set up test environment for i32x4.extend_low_i16x8 instruction testing
     * @details Initializes WAMR runtime with SIMD support, loads test module,
     *          and prepares execution context for both interpreter and AOT modes
     */
    void SetUp() override
    {
        // Get current test mode (INTERP_MODE or AOT_MODE)
        running_mode = GetParam();

        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i32x4.extend_low_i16x8 test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i32x4_extend_low_i16x8_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i32x4.extend_low_i16x8 tests";
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
     * @brief Call WASM function for basic extension test
     * @return true if test function call succeeded, false otherwise
     */
    bool call_basic_extension(int32_t* result)
    {
        uint32_t argv[4] = {0};
        bool ret = dummy_env->execute("test_basic_extension", 0, argv);
        if (ret) {
            // Extract the 4 i32 results returned by the function
            result[0] = static_cast<int32_t>(argv[0]);
            result[1] = static_cast<int32_t>(argv[1]);
            result[2] = static_cast<int32_t>(argv[2]);
            result[3] = static_cast<int32_t>(argv[3]);
        }
        return ret;
    }

    /**
     * @brief Call WASM function for boundary values test
     * @return true if test function call succeeded, false otherwise
     */
    bool call_boundary_values(int32_t* result)
    {
        uint32_t argv[4] = {0};
        bool ret = dummy_env->execute("test_boundary_values", 0, argv);
        if (ret) {
            // Extract the 4 i32 results returned by the function
            result[0] = static_cast<int32_t>(argv[0]);
            result[1] = static_cast<int32_t>(argv[1]);
            result[2] = static_cast<int32_t>(argv[2]);
            result[3] = static_cast<int32_t>(argv[3]);
        }
        return ret;
    }

    /**
     * @brief Call WASM function for zero values test
     * @return true if test function call succeeded, false otherwise
     */
    bool call_zero_values(int32_t* result)
    {
        uint32_t argv[4] = {0};
        bool ret = dummy_env->execute("test_zero_values", 0, argv);
        if (ret) {
            // Extract the 4 i32 results returned by the function
            result[0] = static_cast<int32_t>(argv[0]);
            result[1] = static_cast<int32_t>(argv[1]);
            result[2] = static_cast<int32_t>(argv[2]);
            result[3] = static_cast<int32_t>(argv[3]);
        }
        return ret;
    }

    /**
     * @brief Call WASM function for sign extension test
     * @return true if test function call succeeded, false otherwise
     */
    bool call_sign_extension(int32_t* result)
    {
        uint32_t argv[4] = {0};
        bool ret = dummy_env->execute("test_sign_extension", 0, argv);
        if (ret) {
            // Extract the 4 i32 results returned by the function
            result[0] = static_cast<int32_t>(argv[0]);
            result[1] = static_cast<int32_t>(argv[1]);
            result[2] = static_cast<int32_t>(argv[2]);
            result[3] = static_cast<int32_t>(argv[3]);
        }
        return ret;
    }

    TestRunningMode running_mode;
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicExtension_ReturnsCorrectI32Values
 * @brief Validates i32x4.extend_low_i16x8 produces correct arithmetic results for typical inputs
 * @details Tests fundamental extension operation with positive, negative, and mixed-sign integers.
 *          Verifies that i32x4.extend_low_i16x8 correctly extends the low 4 i16 lanes to i32 values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32x4_extend_low_i16x8_operation
 * @input_conditions i16x8 vector with values [100, -200, 300, -400, 500, -600, 700, -800]
 * @expected_behavior Returns i32x4 with values [100, -200, 300, -400] (low 4 lanes extended)
 * @validation_method Direct comparison of each lane in resulting i32x4 vector with expected values
 */
TEST_P(I32x4ExtendLowI16x8Test, BasicExtension_ReturnsCorrectI32Values)
{
    // Expected results for basic extension test: [100, -200, 300, -400]
    int32_t expected[] = {100, -200, 300, -400};
    int32_t result[4];

    ASSERT_TRUE(call_basic_extension(result))
        << "Failed to call test_basic_extension function";

    for (size_t i = 0; i < 4; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << " mismatch: expected " << expected[i]
            << ", got " << result[i];
    }
}

/**
 * @test BoundaryValues_HandlesMinMaxCorrectly
 * @brief Tests extreme boundary value handling and sign extension limits
 * @details Validates correct handling of INT16_MIN, INT16_MAX, zero, and -1 values.
 *          Ensures proper sign extension behavior at data type boundaries.
 * @test_category Main - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32x4_extend_low_i16x8_operation
 * @input_conditions i16x8 vector with [INT16_MIN(-32768), INT16_MAX(32767), 0, -1, 1, 2, 3, 4]
 * @expected_behavior Returns i32x4 with [-32768, 32767, 0, -1] maintaining correct signs
 * @validation_method Individual lane comparison ensuring boundary values are correctly extended
 */
TEST_P(I32x4ExtendLowI16x8Test, BoundaryValues_HandlesMinMaxCorrectly)
{
    // Expected boundary values: [INT16_MIN, INT16_MAX, 0, -1]
    int32_t expected[] = {-32768, 32767, 0, -1};
    int32_t result[4];

    ASSERT_TRUE(call_boundary_values(result))
        << "Failed to call test_boundary_values function";

    ASSERT_EQ(expected[0], result[0])
        << "INT16_MIN extension failed: expected " << expected[0]
        << ", got " << result[0];
    ASSERT_EQ(expected[1], result[1])
        << "INT16_MAX extension failed: expected " << expected[1]
        << ", got " << result[1];
    ASSERT_EQ(expected[2], result[2])
        << "Zero value extension failed: expected " << expected[2]
        << ", got " << result[2];
    ASSERT_EQ(expected[3], result[3])
        << "Negative one extension failed: expected " << expected[3]
        << ", got " << result[3];
}

/**
 * @test ZeroValues_ReturnsZero
 * @brief Verifies correct handling of zero values in all lanes
 * @details Validates that zero i16 values are correctly extended to zero i32 values.
 *          Tests the identity property of sign extension for zero values.
 * @test_category Main - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32x4_extend_low_i16x8_operation
 * @input_conditions i16x8 vector with all lanes set to 0
 * @expected_behavior Returns i32x4 vector with all lanes set to 0
 * @validation_method Verify each lane in result is zero
 */
TEST_P(I32x4ExtendLowI16x8Test, ZeroValues_ReturnsZero)
{
    // Test zero value handling - all lanes should remain zero after extension
    int32_t result[4];

    ASSERT_TRUE(call_zero_values(result))
        << "Failed to call test_zero_values function";

    for (size_t i = 0; i < 4; i++) {
        ASSERT_EQ(0, result[i])
            << "Lane " << i << " should be zero, got " << result[i];
    }
}

/**
 * @test SignExtension_PreservesSignBit
 * @brief Validates correct sign bit preservation during extension
 * @details Tests that negative i16 values maintain their negative status when extended to i32.
 *          Verifies the sign extension algorithm preserves magnitude and sign correctly.
 * @test_category Main - Sign preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32x4_extend_low_i16x8_operation
 * @input_conditions i16x8 vector with [-1, -32768, -100, -50, 10, 20, 30, 40]
 * @expected_behavior Returns i32x4 with [-1, -32768, -100, -50] preserving negative signs
 * @validation_method Verify each negative value maintains correct sign and magnitude after extension
 */
TEST_P(I32x4ExtendLowI16x8Test, SignExtension_PreservesSignBit)
{
    // Expected sign extension results: [-1, -32768, -100, -50]
    int32_t expected[] = {-1, -32768, -100, -50};
    int32_t result[4];

    ASSERT_TRUE(call_sign_extension(result))
        << "Failed to call test_sign_extension function";

    for (size_t i = 0; i < 4; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Sign extension failed for lane " << i
            << ": expected " << expected[i] << ", got " << result[i];

        // Additional verification that negative values remain negative
        if (expected[i] < 0) {
            ASSERT_LT(result[i], 0)
                << "Negative sign not preserved in lane " << i
                << ": got " << result[i];
        }
    }
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I32x4ExtendLowI16x8Test,
                         testing::Values(TestRunningMode::INTERP_MODE, TestRunningMode::AOT_MODE));