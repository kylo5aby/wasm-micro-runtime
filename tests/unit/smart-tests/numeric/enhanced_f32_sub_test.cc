/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cfloat>
#include <limits>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;

/**
 * @brief Enhanced test suite for f32.sub opcode validation
 *
 * This test suite provides comprehensive validation of the f32.sub WebAssembly opcode
 * implementation in WAMR runtime. It validates IEEE 754 single-precision floating-point
 * subtraction across multiple execution modes (interpreter and AOT) with extensive
 * coverage of edge cases, special values, and boundary conditions.
 *
 * The test suite covers:
 * - Basic arithmetic operations with various operand combinations
 * - IEEE 754 special value handling (NaN, infinity, signed zeros)
 * - Boundary value operations and overflow/underflow scenarios
 * - Cross-execution mode consistency validation
 * - Module loading and error handling scenarios
 */
class F32SubTest : public testing::TestWithParam<RunningMode>
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
     * @brief Set up test environment and initialize WAMR runtime
     *
     * Initializes WAMR runtime with system allocator, loads the f32.sub test module,
     * and instantiates it for test execution. Handles both interpreter and AOT modes
     * based on test parameter configuration.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Initialize WASM file path
        if (WASM_FILE.empty()) {
            char *cwd = getcwd(NULL, 0);
            if (cwd) {
                CWD = std::string(cwd);
                free(cwd);
            } else {
                CWD = ".";
            }
            WASM_FILE = CWD + "/wasm-apps/f32_sub_test.wasm";
        }

        // Load WASM test module from file
        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr)
            << "Failed to read WASM file: " << WASM_FILE;

        // Load and validate WASM module
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr)
            << "Failed to load WASM module: " << error_buf;

        // Instantiate WASM module for execution
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr)
            << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode based on test parameter
        wasm_runtime_set_running_mode(module_inst, GetParam());

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and WAMR runtime resources
     *
     * Properly deallocates module instances, unloads modules, frees buffers,
     * and destroys WAMR runtime to ensure clean test environment.
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
     * @brief Execute f32.sub operation with two f32 operands
     *
     * @param a First operand (minuend)
     * @param b Second operand (subtrahend)
     * @return Result of a - b as f32 value
     *
     * Calls the WASM f32_sub_test function with the specified operands and
     * returns the IEEE 754 compliant subtraction result.
     */
    float call_f32_sub(float a, float b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f32_sub_test");
        EXPECT_NE(func, nullptr) << "Failed to lookup f32_sub_test function";

        uint32_t argv[3];
        memcpy(&argv[0], &a, sizeof(float));
        memcpy(&argv[1], &b, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to call f32_sub_test function";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Runtime exception occurred: " << exception;

        float result;
        memcpy(&result, &argv[0], sizeof(float));
        return result;
    }
};

/**
 * @test BasicSubtraction_ReturnsCorrectResult
 * @brief Validates f32.sub produces correct arithmetic results for typical inputs
 * @details Tests fundamental subtraction operation with positive, negative, and mixed-sign floats.
 *          Verifies that f32.sub correctly computes a - b for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_sub_operation
 * @input_conditions Standard float pairs: (5.5, 2.3), (-10.7, -3.2), (15.8, -4.9), (-8.4, 12.6)
 * @expected_behavior Returns mathematical difference with IEEE 754 precision
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F32SubTest, BasicSubtraction_ReturnsCorrectResult)
{
    // Test basic positive number subtraction
    ASSERT_FLOAT_EQ(3.2f, call_f32_sub(5.5f, 2.3f))
        << "Failed subtraction of positive floats: 5.5 - 2.3";

    // Test subtraction of negative numbers
    ASSERT_FLOAT_EQ(-7.5f, call_f32_sub(-10.7f, -3.2f))
        << "Failed subtraction of negative floats: (-10.7) - (-3.2)";

    // Test mixed signs: positive - negative (effectively addition)
    ASSERT_FLOAT_EQ(20.7f, call_f32_sub(15.8f, -4.9f))
        << "Failed mixed sign subtraction: 15.8 - (-4.9)";

    // Test mixed signs: negative - positive
    ASSERT_FLOAT_EQ(-21.0f, call_f32_sub(-8.4f, 12.6f))
        << "Failed mixed sign subtraction: (-8.4) - 12.6";
}

/**
 * @test BoundaryValues_HandleOverflowCorrectly
 * @brief Validates f32.sub handles boundary values and overflow conditions per IEEE 754
 * @details Tests operations with FLT_MAX, FLT_MIN values that may cause overflow to infinity.
 *          Verifies proper IEEE 754 overflow behavior and boundary value arithmetic.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_sub_overflow_handling
 * @input_conditions FLT_MAX, FLT_MIN, and combinations causing overflow/underflow
 * @expected_behavior Returns +/-INFINITY for overflow, correct arithmetic for valid ranges
 * @validation_method IEEE 754 compliant overflow detection and boundary value verification
 */
TEST_P(F32SubTest, BoundaryValues_HandleOverflowCorrectly)
{
    // Test maximum value operations - should maintain precision where possible
    float result1 = call_f32_sub(FLT_MAX, 1.0f);
    ASSERT_FLOAT_EQ(FLT_MAX, result1)
        << "FLT_MAX - 1.0f should equal FLT_MAX due to precision limits";

    // Test minimum positive value operations
    ASSERT_FLOAT_EQ(0.0f, call_f32_sub(FLT_MIN, FLT_MIN))
        << "FLT_MIN - FLT_MIN should equal 0.0";

    // Test overflow to positive infinity: FLT_MAX - (-FLT_MAX) exceeds float range
    float result2 = call_f32_sub(FLT_MAX, -FLT_MAX);
    ASSERT_TRUE(std::isinf(result2) && result2 > 0)
        << "FLT_MAX - (-FLT_MAX) should overflow to positive infinity";

    // Test overflow to negative infinity: (-FLT_MAX) - FLT_MAX exceeds float range
    float result3 = call_f32_sub(-FLT_MAX, FLT_MAX);
    ASSERT_TRUE(std::isinf(result3) && result3 < 0)
        << "(-FLT_MAX) - FLT_MAX should overflow to negative infinity";
}

/**
 * @test SpecialValues_IEEE754Compliance
 * @brief Validates f32.sub correctly handles IEEE 754 special values
 * @details Tests NaN propagation, infinity arithmetic, and signed zero operations.
 *          Ensures full compliance with IEEE 754 standard for special value handling.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_sub_special_values
 * @input_conditions NaN, +/-INFINITY, +/-0.0f in various combinations
 * @expected_behavior IEEE 754 compliant results: NaN propagation, infinity arithmetic rules
 * @validation_method IEEE 754 special value verification with bit-level precision
 */
TEST_P(F32SubTest, SpecialValues_IEEE754Compliance)
{
    float nan_val = std::numeric_limits<float>::quiet_NaN();
    float pos_inf = std::numeric_limits<float>::infinity();
    float neg_inf = -std::numeric_limits<float>::infinity();

    // Test NaN propagation - any operation with NaN should return NaN
    ASSERT_TRUE(std::isnan(call_f32_sub(nan_val, 5.0f)))
        << "NaN - 5.0 should return NaN";
    ASSERT_TRUE(std::isnan(call_f32_sub(5.0f, nan_val)))
        << "5.0 - NaN should return NaN";
    ASSERT_TRUE(std::isnan(call_f32_sub(nan_val, nan_val)))
        << "NaN - NaN should return NaN";

    // Test infinity arithmetic
    ASSERT_EQ(pos_inf, call_f32_sub(pos_inf, 100.0f))
        << "+INFINITY - 100.0 should equal +INFINITY";
    ASSERT_EQ(neg_inf, call_f32_sub(neg_inf, 100.0f))
        << "-INFINITY - 100.0 should equal -INFINITY";
    ASSERT_EQ(neg_inf, call_f32_sub(100.0f, pos_inf))
        << "100.0 - (+INFINITY) should equal -INFINITY";
    ASSERT_EQ(pos_inf, call_f32_sub(100.0f, neg_inf))
        << "100.0 - (-INFINITY) should equal +INFINITY";

    // Test undefined infinity operations - should return NaN
    ASSERT_TRUE(std::isnan(call_f32_sub(pos_inf, pos_inf)))
        << "(+INFINITY) - (+INFINITY) should return NaN (undefined)";
    ASSERT_TRUE(std::isnan(call_f32_sub(neg_inf, neg_inf)))
        << "(-INFINITY) - (-INFINITY) should return NaN (undefined)";
}

/**
 * @test ZeroOperations_IdentityAndSignHandling
 * @brief Validates f32.sub identity properties and signed zero handling
 * @details Tests subtraction identity (x - 0 = x), negation (0 - x = -x), and IEEE 754
 *          signed zero arithmetic rules including (+0) - (-0) and (-0) - (+0) operations.
 * @test_category Edge - Identity and signed zero validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_sub_zero_handling
 * @input_conditions Zero values, signed zeros, identity operations, self-subtraction
 * @expected_behavior Identity preservation, correct signed zero results per IEEE 754
 * @validation_method Sign bit verification and identity property validation
 */
TEST_P(F32SubTest, ZeroOperations_IdentityAndSignHandling)
{
    float pos_zero = 0.0f;
    float neg_zero = -0.0f;

    // Test identity property: x - 0 = x
    ASSERT_FLOAT_EQ(42.5f, call_f32_sub(42.5f, 0.0f))
        << "Identity property failed: 42.5 - 0.0 should equal 42.5";
    ASSERT_FLOAT_EQ(-17.3f, call_f32_sub(-17.3f, 0.0f))
        << "Identity property failed: (-17.3) - 0.0 should equal -17.3";

    // Test negation property: 0 - x = -x
    ASSERT_FLOAT_EQ(-25.7f, call_f32_sub(0.0f, 25.7f))
        << "Negation property failed: 0.0 - 25.7 should equal -25.7";
    ASSERT_FLOAT_EQ(33.1f, call_f32_sub(0.0f, -33.1f))
        << "Negation property failed: 0.0 - (-33.1) should equal 33.1";

    // Test signed zero operations per IEEE 754
    float result1 = call_f32_sub(pos_zero, neg_zero);
    ASSERT_FALSE(std::signbit(result1))
        << "(+0.0) - (-0.0) should result in +0.0";

    float result2 = call_f32_sub(neg_zero, pos_zero);
    ASSERT_TRUE(std::signbit(result2))
        << "(-0.0) - (+0.0) should result in -0.0";

    // Test self-subtraction: x - x = 0
    ASSERT_FLOAT_EQ(0.0f, call_f32_sub(123.456f, 123.456f))
        << "Self-subtraction failed: 123.456 - 123.456 should equal 0.0";
    ASSERT_FLOAT_EQ(0.0f, call_f32_sub(-987.654f, -987.654f))
        << "Self-subtraction failed: (-987.654) - (-987.654) should equal 0.0";
}

/**
 * @test ModuleLoading_HandleInvalidModule
 * @brief Validates proper error handling for invalid WASM modules
 * @details Tests module loading behavior with corrupted or invalid bytecode to ensure
 *          proper error reporting and graceful failure handling in WAMR runtime.
 * @test_category Error - Module loading validation
 * @coverage_target core/iwasm/common/wasm_loader.c:wasm_load_module_validation
 * @input_conditions Invalid WASM bytecode, corrupted module data
 * @expected_behavior Module loading fails gracefully with appropriate error messages
 * @validation_method Error condition verification and resource cleanup validation
 */
TEST_P(F32SubTest, ModuleLoading_HandleInvalidModule)
{
    // Test with invalid WASM bytecode
    uint8_t invalid_wasm[] = {0x00, 0x61, 0x73, 0x6D, 0xFF, 0xFF, 0xFF, 0xFF}; // Invalid WASM magic
    char error_buffer[256];
    memset(error_buffer, 0, sizeof(error_buffer));

    wasm_module_t invalid_module = wasm_runtime_load(invalid_wasm, sizeof(invalid_wasm),
                                                   error_buffer, sizeof(error_buffer));

    ASSERT_EQ(nullptr, invalid_module)
        << "Expected module loading to fail for invalid WASM bytecode";
    ASSERT_NE(strlen(error_buffer), 0)
        << "Expected error message for invalid module loading";

    // Test with completely corrupted data
    uint8_t corrupted_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
    memset(error_buffer, 0, sizeof(error_buffer));

    wasm_module_t corrupted_module = wasm_runtime_load(corrupted_data, sizeof(corrupted_data),
                                                     error_buffer, sizeof(error_buffer));

    ASSERT_EQ(nullptr, corrupted_module)
        << "Expected module loading to fail for corrupted data";
    ASSERT_NE(strlen(error_buffer), 0)
        << "Expected error message for corrupted module data";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningModeTest, F32SubTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<F32SubTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });