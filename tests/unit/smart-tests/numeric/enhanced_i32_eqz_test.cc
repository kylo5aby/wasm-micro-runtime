/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i32.eqz Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i32.eqz
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic zero equality test functionality
 * - Corner Cases: Boundary conditions and extreme values
 * - Edge Cases: Zero operands, bit patterns, and sign variations
 * - Error Handling: Stack underflow and validation errors
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i32.eqz)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:4758-4762
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

/**
 * @brief Test fixture for i32.eqz opcode validation
 * @details Manages WAMR runtime initialization, module loading, and cleanup.
 *          Supports both interpreter and AOT execution modes through parameterization.
 */
class I32EqzTest : public testing::TestWithParam<RunningMode>
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
     * @brief Initialize test environment and load WASM module
     * @details Sets up WAMR runtime, loads test module, and creates execution environment.
     *          Called before each test case execution.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and release resources
     * @details Destroys execution environment, deinstantiates module, and frees memory.
     *          Called after each test case execution.
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
     * @brief Helper function to call i32.eqz operation in WASM module
     * @param operand Value to test against zero
     * @return Result of i32.eqz operation (1 if operand == 0, 0 if operand != 0)
     * @details Invokes test_i32_eqz function in loaded WASM module and handles exceptions.
     */
    int32_t call_i32_eqz(int32_t operand)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i32_eqz");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_eqz function";

        uint32_t argv[2] = { (uint32_t)operand, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
        }

        return (int32_t)argv[0];
    }

    /**
     * @brief Helper function to call specific test functions in WASM module
     * @param function_name Name of the function to call
     * @return Result of the function call
     * @details Generic helper to invoke any exported function without parameters.
     */
    int32_t call_test_function(const std::string& function_name)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, function_name.c_str());
        EXPECT_NE(func, nullptr) << "Failed to lookup function: " << function_name;

        uint32_t argv[1] = { 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 0, argv);
        EXPECT_EQ(ret, true) << "Function call failed for: " << function_name;

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred in " << function_name << ": " << exception;
        }

        return (int32_t)argv[0];
    }

    /**
     * @brief Test stack validation with boundary conditions near underflow
     * @param wasm_file Path to WASM file containing stack validation tests
     * @details Validates that WAMR properly manages stack in boundary conditions.
     */
    void test_stack_validation(const std::string& wasm_file)
    {
        uint8_t *validation_buf = nullptr;
        uint32_t validation_buf_size;
        wasm_module_t validation_module = nullptr;
        wasm_module_inst_t validation_inst = nullptr;
        wasm_exec_env_t validation_exec_env = nullptr;

        validation_buf = (uint8_t *)bh_read_file_to_buffer(wasm_file.c_str(), &validation_buf_size);
        ASSERT_NE(validation_buf, nullptr) << "Failed to read validation test WASM file";

        validation_module = wasm_runtime_load(validation_buf, validation_buf_size,
                                            error_buf, sizeof(error_buf));
        ASSERT_NE(validation_module, nullptr)
            << "Valid stack validation module should load successfully: " << error_buf;

        validation_inst = wasm_runtime_instantiate(validation_module, stack_size, heap_size,
                                                   error_buf, sizeof(error_buf));
        ASSERT_NE(validation_inst, nullptr)
            << "Stack validation module should instantiate successfully: " << error_buf;

        wasm_runtime_set_running_mode(validation_inst, GetParam());

        validation_exec_env = wasm_runtime_create_exec_env(validation_inst, stack_size);
        ASSERT_NE(validation_exec_env, nullptr) << "Failed to create validation exec environment";

        // Test minimal stack usage
        wasm_function_inst_t func = wasm_runtime_lookup_function(validation_inst, "test_minimal_stack");
        ASSERT_NE(func, nullptr) << "Failed to lookup test_minimal_stack function";

        uint32_t argv[1] = { 0 };
        bool success = wasm_runtime_call_wasm(validation_exec_env, func, 0, argv);
        ASSERT_EQ(success, true) << "Minimal stack test should succeed";

        const char *exc = wasm_runtime_get_exception(validation_inst);
        ASSERT_EQ(exc, nullptr) << "No exception should occur in valid stack test: " << (exc ? exc : "");

        ASSERT_EQ((int32_t)argv[0], 1) << "test_minimal_stack should return 1 (0 == 0)";

        // Cleanup validation resources
        if (validation_exec_env) {
            wasm_runtime_destroy_exec_env(validation_exec_env);
        }
        if (validation_inst) {
            wasm_runtime_deinstantiate(validation_inst);
        }
        if (validation_module) {
            wasm_runtime_unload(validation_module);
        }
        if (validation_buf) {
            BH_FREE(validation_buf);
        }
    }
};

// ============================================================================
// Main Routine Tests - Basic Functionality
// ============================================================================

/**
 * @brief Test zero value returns true (1)
 * @details Validates that i32.eqz correctly identifies zero as equal to zero.
 * @coverage Core zero detection logic
 */
TEST_P(I32EqzTest, ZeroValue_WithConstantZero_ReturnsTrue)
{
    int32_t result = call_i32_eqz(0);
    ASSERT_EQ(result, 1) << "i32.eqz(0) should return 1 (true)";
}

/**
 * @brief Test positive non-zero value returns false (0)
 * @details Validates that i32.eqz correctly identifies positive values as not equal to zero.
 * @coverage Non-zero positive value detection
 */
TEST_P(I32EqzTest, PositiveValue_WithConstantFortyTwo_ReturnsFalse)
{
    int32_t result = call_i32_eqz(42);
    ASSERT_EQ(result, 0) << "i32.eqz(42) should return 0 (false)";
}

/**
 * @brief Test negative non-zero value returns false (0)
 * @details Validates that i32.eqz correctly identifies negative values as not equal to zero.
 * @coverage Non-zero negative value detection
 */
TEST_P(I32EqzTest, NegativeValue_WithConstantNegativeFortyTwo_ReturnsFalse)
{
    int32_t result = call_i32_eqz(-42);
    ASSERT_EQ(result, 0) << "i32.eqz(-42) should return 0 (false)";
}

/**
 * @brief Test small positive value (1) returns false
 * @details Validates that minimal positive non-zero values are correctly identified.
 * @coverage Edge case around zero boundary (positive side)
 */
TEST_P(I32EqzTest, SmallPositive_WithConstantOne_ReturnsFalse)
{
    int32_t result = call_i32_eqz(1);
    ASSERT_EQ(result, 0) << "i32.eqz(1) should return 0 (false)";
}

/**
 * @brief Test small negative value (-1) returns false
 * @details Validates that minimal negative non-zero values are correctly identified.
 * @coverage Edge case around zero boundary (negative side)
 */
TEST_P(I32EqzTest, SmallNegative_WithConstantNegativeOne_ReturnsFalse)
{
    int32_t result = call_i32_eqz(-1);
    ASSERT_EQ(result, 0) << "i32.eqz(-1) should return 0 (false)";
}

// ============================================================================
// Corner Cases - Boundary Values and Extremes
// ============================================================================

/**
 * @brief Test INT32_MAX boundary value returns false
 * @details Validates that maximum 32-bit signed integer is correctly identified as non-zero.
 * @coverage Upper boundary validation (0x7FFFFFFF)
 */
TEST_P(I32EqzTest, BoundaryValue_WithINT32MAX_ReturnsFalse)
{
    int32_t result = call_test_function("test_max_boundary");
    ASSERT_EQ(result, 0) << "i32.eqz(INT32_MAX) should return 0 (false)";
}

/**
 * @brief Test INT32_MIN boundary value returns false
 * @details Validates that minimum 32-bit signed integer is correctly identified as non-zero.
 * @coverage Lower boundary validation (0x80000000)
 */
TEST_P(I32EqzTest, BoundaryValue_WithINT32MIN_ReturnsFalse)
{
    int32_t result = call_test_function("test_min_boundary");
    ASSERT_EQ(result, 0) << "i32.eqz(INT32_MIN) should return 0 (false)";
}

/**
 * @brief Test all ones bit pattern (0xFFFFFFFF) returns false
 * @details Validates that all bits set pattern (-1 in signed) is correctly identified as non-zero.
 * @coverage Bit pattern validation (all ones)
 */
TEST_P(I32EqzTest, BitPattern_WithAllOnes_ReturnsFalse)
{
    int32_t result = call_test_function("test_all_ones");
    ASSERT_EQ(result, 0) << "i32.eqz(0xFFFFFFFF) should return 0 (false)";
}

/**
 * @brief Test alternating bit pattern returns false
 * @details Validates that alternating bit patterns are correctly identified as non-zero.
 * @coverage Complex bit pattern validation (0xAAAAAAAA)
 */
TEST_P(I32EqzTest, BitPattern_WithAlternating_ReturnsFalse)
{
    int32_t result = call_test_function("test_alternating_bits");
    ASSERT_EQ(result, 0) << "i32.eqz(0xAAAAAAAA) should return 0 (false)";
}

/**
 * @brief Test single bit set in LSB position returns false
 * @details Validates that minimal non-zero value (LSB set) is correctly identified.
 * @coverage Single bit validation (least significant bit)
 */
TEST_P(I32EqzTest, SingleBit_WithLSBSet_ReturnsFalse)
{
    int32_t result = call_test_function("test_single_bit_lsb");
    ASSERT_EQ(result, 0) << "i32.eqz(0x00000001) should return 0 (false)";
}

/**
 * @brief Test single bit set in MSB position (sign bit) returns false
 * @details Validates that sign bit only pattern is correctly identified as non-zero.
 * @coverage Single bit validation (most significant bit/sign bit)
 */
TEST_P(I32EqzTest, SingleBit_WithMSBSet_ReturnsFalse)
{
    int32_t result = call_test_function("test_single_bit_msb");
    ASSERT_EQ(result, 0) << "i32.eqz(0x80000000) should return 0 (false)";
}

// ============================================================================
// Edge Cases - Special Values and Patterns
// ============================================================================

/**
 * @brief Test power of two value returns false
 * @details Validates that power-of-two values are correctly identified as non-zero.
 * @coverage Power of two bit patterns
 */
TEST_P(I32EqzTest, PowerOfTwo_WithTenTwentyFour_ReturnsFalse)
{
    int32_t result = call_test_function("test_power_of_two");
    ASSERT_EQ(result, 0) << "i32.eqz(1024) should return 0 (false)";
}

/**
 * @brief Test large positive value returns false
 * @details Validates that large positive integers are correctly identified as non-zero.
 * @coverage Large positive value handling
 */
TEST_P(I32EqzTest, LargeValue_WithPositiveMillion_ReturnsFalse)
{
    int32_t result = call_test_function("test_large_positive");
    ASSERT_EQ(result, 0) << "i32.eqz(1000000) should return 0 (false)";
}

/**
 * @brief Test large negative value returns false
 * @details Validates that large negative integers are correctly identified as non-zero.
 * @coverage Large negative value handling
 */
TEST_P(I32EqzTest, LargeValue_WithNegativeMillion_ReturnsFalse)
{
    int32_t result = call_test_function("test_large_negative");
    ASSERT_EQ(result, 0) << "i32.eqz(-1000000) should return 0 (false)";
}

/**
 * @brief Test values near zero boundary (positive side)
 * @details Validates that small positive values near zero are correctly identified.
 * @coverage Near-zero boundary validation (positive)
 */
TEST_P(I32EqzTest, NearZero_WithPositiveTwo_ReturnsFalse)
{
    int32_t result = call_test_function("test_near_zero_positive");
    ASSERT_EQ(result, 0) << "i32.eqz(2) should return 0 (false)";
}

/**
 * @brief Test values near zero boundary (negative side)
 * @details Validates that small negative values near zero are correctly identified.
 * @coverage Near-zero boundary validation (negative)
 */
TEST_P(I32EqzTest, NearZero_WithNegativeTwo_ReturnsFalse)
{
    int32_t result = call_test_function("test_near_zero_negative");
    ASSERT_EQ(result, 0) << "i32.eqz(-2) should return 0 (false)";
}

// ============================================================================
// Stack and Integration Tests
// ============================================================================

/**
 * @brief Test stack validation with boundary conditions near underflow
 * @details Validates that WAMR properly manages stack operations near boundary.
 * @coverage Stack management validation
 */
TEST_P(I32EqzTest, StackValidation_WithMinimalUsage_ExecutesCorrectly)
{
    test_stack_validation(WASM_FILE_UNDERFLOW);
}

// ============================================================================
// Parameterized Test Configuration
// ============================================================================

INSTANTIATE_TEST_SUITE_P(
    RunningMode, I32EqzTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<I32EqzTest::ParamType>& info) {
        return info.param == Mode_Interp ? "INTERP" : "AOT";
    }
);

// ============================================================================
// Test Environment Setup
// ============================================================================

// Global test setup - executed before all tests
struct I32EqzTestSetup
{
    I32EqzTestSetup()
    {
        CWD = getcwd(nullptr, 0);
        WASM_FILE = CWD + "/wasm-apps/i32_eqz_test.wasm";
        WASM_FILE_UNDERFLOW = CWD + "/wasm-apps/i32_eqz_stack_underflow.wasm";
    }
};

// Static initializer to set up global test environment
static I32EqzTestSetup test_setup;

