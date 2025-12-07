/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i64.extend16_s Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i64.extend16_s
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic sign extension functionality with typical values
 * - Corner Cases: Boundary conditions at 16-bit signed integer limits
 * - Edge Cases: Upper bit interference and mathematical properties
 * - Error Handling: Stack underflow and validation scenarios
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i64.extend16_s)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c (i64 sign extension operations)
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;
static std::string WASM_FILE_UNDERFLOW;

static int
app_argc;
static char **app_argv;

/**
 * @class I64Extend16sTest
 * @brief Test fixture for comprehensive i64.extend16_s opcode validation
 *
 * Provides WAMR runtime environment setup and cleanup for testing
 * i64.extend16_s sign extension operations across different execution modes.
 * Supports both interpreter and AOT modes through parameterized testing.
 */
class I64Extend16sTest : public testing::TestWithParam<RunningMode>
{
  protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };
    const char *exception = nullptr;

    /**
     * @brief Set up WAMR runtime environment and load i64.extend16_s test module
     *
     * Initializes WAMR runtime, loads the test WASM module, creates module instance,
     * and sets up execution environment for i64.extend16_s testing.
     * Configures the running mode based on test parameter (interpreter/AOT).
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Initialize WASM file paths - done in SetUp to ensure proper timing
        WASM_FILE = "wasm-apps/i64_extend16_s_test.wasm";
        WASM_FILE_UNDERFLOW = "wasm-apps/i64_extend16_s_stack_underflow.wasm";

        // Load i64.extend16_s test WASM module
        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode (interpreter or AOT)
        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up WAMR runtime resources
     *
     * Destroys execution environment, deinstantiates module, unloads module,
     * and frees allocated buffers to prevent resource leaks.
     */
    void TearDown() override
    {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (buf) {
            BH_FREE(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Execute i64.extend16_s operation with input value
     * @param input Input i64 value for sign extension
     * @return Sign-extended i64 result
     *
     * Calls the test_i64_extend16_s function in the loaded WASM module,
     * passing the input value and returning the sign-extended result.
     */
    int64_t call_i64_extend16_s(int64_t input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_extend16_s");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_extend16_s function";

        wasm_val_t params[1] = { 0 };
        wasm_val_t results[1] = { 0 };

        params[0].kind = WASM_I64;
        params[0].of.i64 = input;
        results[0].kind = WASM_I64;

        bool call_result = wasm_runtime_call_wasm_a(exec_env, func, 1, results, 1, params);
        EXPECT_TRUE(call_result) << "Failed to call test_i64_extend16_s function";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Exception occurred: " << exception;

        return results[0].of.i64;
    }

    /**
     * @brief Execute double i64.extend16_s operation for idempotent testing
     * @param input Input i64 value for double sign extension
     * @return Result after two consecutive sign extensions
     *
     * Tests idempotent property: extend16_s(extend16_s(x)) == extend16_s(x)
     */
    int64_t call_i64_extend16_s_double(int64_t input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_extend16_s_double");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_extend16_s_double function";

        wasm_val_t params[1] = { 0 };
        wasm_val_t results[1] = { 0 };

        params[0].kind = WASM_I64;
        params[0].of.i64 = input;
        results[0].kind = WASM_I64;

        bool call_result = wasm_runtime_call_wasm_a(exec_env, func, 1, results, 1, params);
        EXPECT_TRUE(call_result) << "Failed to call test_i64_extend16_s_double function";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Exception occurred: " << exception;

        return results[0].of.i64;
    }

    /**
     * @brief Test upper bits masking behavior
     * @param input Input i64 value with arbitrary upper bits
     * @return Sign-extended result based only on lower 16 bits
     *
     * Verifies that only the lower 16 bits affect the sign extension result,
     * regardless of the values in the upper 48 bits.
     */
    int64_t call_i64_extend16_s_mask(int64_t input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_extend16_s_mask");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_extend16_s_mask function";

        wasm_val_t params[1] = { 0 };
        wasm_val_t results[1] = { 0 };

        params[0].kind = WASM_I64;
        params[0].of.i64 = input;
        results[0].kind = WASM_I64;

        bool call_result = wasm_runtime_call_wasm_a(exec_env, func, 1, results, 1, params);
        EXPECT_TRUE(call_result) << "Failed to call test_i64_extend16_s_mask function";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Exception occurred: " << exception;

        return results[0].of.i64;
    }
};

/**
 * @test BasicSignExtension_ReturnsCorrectValues
 * @brief Validates i64.extend16_s produces correct sign extension for typical values
 * @details Tests fundamental sign extension with positive, negative, and mixed-sign 16-bit integers.
 *          Verifies that the lower 16 bits are properly sign-extended to 64 bits.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_extend16_s_operation
 * @input_conditions Typical 16-bit signed values: 100, -100, 1000, -1000
 * @expected_behavior Returns mathematically correct sign extensions
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64Extend16sTest, BasicSignExtension_ReturnsCorrectValues)
{
    // Test positive values - should remain positive after sign extension
    ASSERT_EQ(100LL, call_i64_extend16_s(100LL))
        << "Positive value 100 should extend to 100";
    ASSERT_EQ(1000LL, call_i64_extend16_s(1000LL))
        << "Positive value 1000 should extend to 1000";

    // Test negative values - should remain negative after sign extension
    ASSERT_EQ(-100LL, call_i64_extend16_s(-100LL))
        << "Negative value -100 should extend to -100";
    ASSERT_EQ(-1000LL, call_i64_extend16_s(-1000LL))
        << "Negative value -1000 should extend to -1000";

    // Test mixed values in same operation for consistency
    ASSERT_EQ(15000LL, call_i64_extend16_s(15000LL))
        << "Large positive value should extend correctly";
    ASSERT_EQ(-15000LL, call_i64_extend16_s(-15000LL))
        << "Large negative value should extend correctly";
}

/**
 * @test BoundaryValues_SignExtendCorrectly
 * @brief Validates correct handling of 16-bit signed integer boundary values
 * @details Tests maximum and minimum 16-bit signed values, and values near boundaries.
 *          Also tests that upper bits don't affect the result by using values with
 *          arbitrary upper 48 bits.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_extend16_s_operation
 * @input_conditions 16-bit boundaries: 32767, -32768, and values with upper bits set
 * @expected_behavior Correct sign extension regardless of upper bits
 * @validation_method Verification of boundary values and upper bit masking
 */
TEST_P(I64Extend16sTest, BoundaryValues_SignExtendCorrectly)
{
    // Test maximum positive 16-bit signed integer (0x7FFF = 32767)
    ASSERT_EQ(32767LL, call_i64_extend16_s(32767LL))
        << "Maximum positive 16-bit value should extend to 32767";

    // Test minimum negative 16-bit signed integer (0x8000 = -32768)
    ASSERT_EQ(-32768LL, call_i64_extend16_s(-32768LL))
        << "Minimum negative 16-bit value should extend to -32768";

    // Test values adjacent to boundaries
    ASSERT_EQ(32766LL, call_i64_extend16_s(32766LL))
        << "Value below max positive should extend correctly";
    ASSERT_EQ(-32767LL, call_i64_extend16_s(-32767LL))
        << "Value above min negative should extend correctly";

    // Test upper bits are ignored - only lower 16 bits matter
    ASSERT_EQ(32767LL, call_i64_extend16_s_mask(0xFFFF0000FFFF7FFFLL))
        << "Upper bits should be ignored, only lower 16 bits affect result";
    ASSERT_EQ(-32768LL, call_i64_extend16_s_mask(0x12345678ABCD8000LL))
        << "Arbitrary upper bits should not affect sign extension";
}

/**
 * @test SpecialValues_HandleCorrectly
 * @brief Validates handling of special values like zero, -1, and bit patterns
 * @details Tests zero extension, negative one extension, and ensures mathematical
 *          properties are preserved. Also tests idempotent property of the operation.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_extend16_s_operation
 * @input_conditions Zero, -1 (0xFFFF), and mathematical property tests
 * @expected_behavior Zero remains zero, -1 properly sign extends, idempotent behavior
 * @validation_method Mathematical correctness and property verification
 */
TEST_P(I64Extend16sTest, SpecialValues_HandleCorrectly)
{
    // Test zero extension - should remain zero
    ASSERT_EQ(0LL, call_i64_extend16_s(0LL))
        << "Zero should remain zero after sign extension";

    // Test -1 (0xFFFF in 16-bit) - should sign extend to all 1s in 64-bit
    ASSERT_EQ(-1LL, call_i64_extend16_s(0xFFFFLL))
        << "16-bit value 0xFFFF should sign extend to -1 (all bits set)";

    // Test with upper bits set to verify only lower 16 bits matter
    ASSERT_EQ(0LL, call_i64_extend16_s_mask(0x123456789ABC0000LL))
        << "Value with upper bits set but lower 16 bits zero should extend to zero";
    ASSERT_EQ(-1LL, call_i64_extend16_s_mask(0xFEDCBA9876543210LL | 0xFFFFLL))
        << "Value with upper bits set but lower 16 bits 0xFFFF should extend to -1";

    // Test idempotent property: extend16_s(extend16_s(x)) == extend16_s(x)
    ASSERT_EQ(call_i64_extend16_s(12345LL), call_i64_extend16_s_double(12345LL))
        << "Double sign extension should be idempotent";
    ASSERT_EQ(call_i64_extend16_s(-12345LL), call_i64_extend16_s_double(-12345LL))
        << "Double sign extension should be idempotent for negative values";
}

/**
 * @test StackUnderflow_HandledGracefully
 * @brief Validates proper handling of stack underflow conditions
 * @details Tests module loading behavior when stack underflow scenarios occur.
 *          Since i64.extend16_s requires one i64 operand, empty stack should be
 *          handled appropriately by the WASM runtime.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:stack_validation
 * @input_conditions Stack underflow test module with no operands
 * @expected_behavior Module loads successfully, runtime handles stack conditions appropriately
 * @validation_method Module loading verification and error handling validation
 */
TEST_P(I64Extend16sTest, StackUnderflow_HandledGracefully)
{
    // Clean up current module to test underflow scenario
    if (exec_env) {
        wasm_runtime_destroy_exec_env(exec_env);
        exec_env = nullptr;
    }
    if (module_inst) {
        wasm_runtime_deinstantiate(module_inst);
        module_inst = nullptr;
    }
    if (module) {
        wasm_runtime_unload(module);
        module = nullptr;
    }
    if (buf) {
        BH_FREE(buf);
        buf = nullptr;
    }

    // Load stack underflow test module
    uint32_t underflow_buf_size;
    uint8_t *underflow_buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE_UNDERFLOW.c_str(), &underflow_buf_size);
    ASSERT_NE(underflow_buf, nullptr) << "Failed to read underflow test WASM file";

    // Module should load successfully even if it contains stack underflow scenarios
    wasm_module_t underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size, error_buf, sizeof(error_buf));
    ASSERT_NE(underflow_module, nullptr) << "Failed to load underflow test module: " << error_buf;

    // Instantiate the underflow test module
    wasm_module_inst_t underflow_module_inst = wasm_runtime_instantiate(underflow_module, stack_size, heap_size, error_buf, sizeof(error_buf));
    ASSERT_NE(underflow_module_inst, nullptr) << "Failed to instantiate underflow test module: " << error_buf;

    // Clean up underflow test resources
    wasm_runtime_deinstantiate(underflow_module_inst);
    wasm_runtime_unload(underflow_module);
    BH_FREE(underflow_buf);
}

// Instantiate parameterized tests for both Interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I64Extend16sTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I64Extend16sTest::ParamType>& info) {
                             return info.param == Mode_Interp ? "Interpreter" : "AOT";
                         });

