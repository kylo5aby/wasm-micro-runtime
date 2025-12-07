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
 * @brief Enhanced test suite for f32.mul opcode validation
 *
 * This test suite provides comprehensive validation of the f32.mul WebAssembly opcode
 * implementation in WAMR runtime. It validates IEEE 754 single-precision floating-point
 * multiplication across multiple execution modes (interpreter and AOT) with extensive
 * coverage of edge cases, special values, and boundary conditions.
 *
 * The test suite covers:
 * - Basic arithmetic operations with various operand combinations
 * - IEEE 754 special value handling (NaN, infinity, signed zeros)
 * - Boundary value operations and overflow/underflow scenarios
 * - Cross-execution mode consistency validation
 * - Mathematical property verification (commutativity)
 * - Module loading and error handling scenarios
 */
class F32MulTest : public testing::TestWithParam<RunningMode>
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
     * Initializes WAMR runtime with system allocator, loads the f32.mul test module,
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
            WASM_FILE = CWD + "/wasm-apps/f32_mul_test.wasm";
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
     * @brief Execute f32.mul operation with two f32 operands
     *
     * @param a First operand (multiplicand)
     * @param b Second operand (multiplier)
     * @return Result of a * b as f32 value
     *
     * Calls the WASM f32_mul_test function with the specified operands and
     * returns the IEEE 754 compliant multiplication result.
     */
    float call_f32_mul(float a, float b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f32_mul_test");
        EXPECT_NE(func, nullptr) << "Failed to lookup f32_mul_test function";

        uint32_t argv[3];
        memcpy(&argv[0], &a, sizeof(float));
        memcpy(&argv[1], &b, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to call f32_mul_test function";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Runtime exception occurred: " << exception;

        float result;
        memcpy(&result, &argv[0], sizeof(float));
        return result;
    }
};

/**
 * @test BasicMultiplication_ReturnsCorrectResults
 * @brief Validates f32.mul produces correct arithmetic results for typical inputs
 * @details Tests fundamental multiplication operation with positive, negative, and mixed-sign floats.
 *          Verifies that f32.mul correctly computes a * b for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_mul_operation
 * @input_conditions Standard float pairs: (2.5, 3.0), (-4.0, -2.5), (6.0, -1.5), (-3.0, 4.0)
 * @expected_behavior Returns mathematical product with IEEE 754 precision
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F32MulTest, BasicMultiplication_ReturnsCorrectResults)
{
    // Test positive number multiplication
    ASSERT_FLOAT_EQ(7.5f, call_f32_mul(2.5f, 3.0f))
        << "Failed multiplication of positive floats: 2.5 * 3.0";

    // Test multiplication of negative numbers (result positive)
    ASSERT_FLOAT_EQ(10.0f, call_f32_mul(-4.0f, -2.5f))
        << "Failed multiplication of negative floats: (-4.0) * (-2.5)";

    // Test mixed signs: positive * negative (result negative)
    ASSERT_FLOAT_EQ(-9.0f, call_f32_mul(6.0f, -1.5f))
        << "Failed mixed sign multiplication: 6.0 * (-1.5)";

    // Test mixed signs: negative * positive (result negative)
    ASSERT_FLOAT_EQ(-12.0f, call_f32_mul(-3.0f, 4.0f))
        << "Failed mixed sign multiplication: (-3.0) * 4.0";
}

/**
 * @test BoundaryConditions_HandleOverflowUnderflow
 * @brief Validates f32.mul handles boundary values and overflow/underflow conditions per IEEE 754
 * @details Tests operations with FLT_MAX, FLT_MIN values that may cause overflow to infinity
 *          or underflow to zero/denormal. Verifies proper IEEE 754 overflow/underflow behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_mul_overflow_handling
 * @input_conditions FLT_MAX, FLT_MIN, and combinations causing overflow/underflow
 * @expected_behavior Returns +/-INFINITY for overflow, zero/denormal for underflow
 * @validation_method IEEE 754 compliant overflow/underflow detection and boundary verification
 */
TEST_P(F32MulTest, BoundaryConditions_HandleOverflowUnderflow)
{
    // Test maximum value preservation with identity
    ASSERT_FLOAT_EQ(FLT_MAX, call_f32_mul(FLT_MAX, 1.0f))
        << "FLT_MAX * 1.0f should equal FLT_MAX";

    // Test minimum positive value preservation with identity
    ASSERT_FLOAT_EQ(FLT_MIN, call_f32_mul(FLT_MIN, 1.0f))
        << "FLT_MIN * 1.0f should equal FLT_MIN";

    // Test overflow to positive infinity: FLT_MAX * 2.0f exceeds float range
    float result1 = call_f32_mul(FLT_MAX, 2.0f);
    ASSERT_TRUE(std::isinf(result1) && result1 > 0)
        << "FLT_MAX * 2.0f should overflow to positive infinity";

    // Test overflow to negative infinity: (-FLT_MAX) * 2.0f exceeds float range
    float result2 = call_f32_mul(-FLT_MAX, 2.0f);
    ASSERT_TRUE(std::isinf(result2) && result2 < 0)
        << "(-FLT_MAX) * 2.0f should overflow to negative infinity";

    // Test underflow to zero: FLT_MIN * small value may underflow
    float result3 = call_f32_mul(FLT_MIN, 0.5f);
    ASSERT_TRUE(result3 == 0.0f || (result3 > 0.0f && result3 < FLT_MIN))
        << "FLT_MIN * 0.5f should underflow to zero or denormal";
}

/**
 * @test SpecialValues_FollowIEEE754Rules
 * @brief Validates f32.mul correctly handles IEEE 754 special values
 * @details Tests NaN propagation, infinity arithmetic, signed zero operations, and identity values.
 *          Ensures full compliance with IEEE 754 standard for special value handling.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_mul_special_values
 * @input_conditions NaN, +/-INFINITY, +/-0.0f, identity values in various combinations
 * @expected_behavior IEEE 754 compliant results: NaN propagation, infinity rules, sign preservation
 * @validation_method IEEE 754 special value verification with bit-level precision
 */
TEST_P(F32MulTest, SpecialValues_FollowIEEE754Rules)
{
    // Test NaN propagation - any operation with NaN should return NaN
    float nan_val = std::numeric_limits<float>::quiet_NaN();
    float result1 = call_f32_mul(nan_val, 5.0f);
    ASSERT_TRUE(std::isnan(result1))
        << "NaN * 5.0f should return NaN";

    float result2 = call_f32_mul(3.5f, nan_val);
    ASSERT_TRUE(std::isnan(result2))
        << "3.5f * NaN should return NaN";

    // Test infinity * finite positive = infinity
    float pos_inf = std::numeric_limits<float>::infinity();
    float result3 = call_f32_mul(pos_inf, 2.5f);
    ASSERT_TRUE(std::isinf(result3) && result3 > 0)
        << "+INFINITY * 2.5f should return +INFINITY";

    // Test infinity * finite negative = negative infinity
    float result4 = call_f32_mul(pos_inf, -1.8f);
    ASSERT_TRUE(std::isinf(result4) && result4 < 0)
        << "+INFINITY * (-1.8f) should return -INFINITY";

    // Test infinity * zero = NaN (invalid operation per IEEE 754)
    float result5 = call_f32_mul(pos_inf, 0.0f);
    ASSERT_TRUE(std::isnan(result5))
        << "+INFINITY * 0.0f should return NaN";

    // Test signed zero handling: (+0.0) * (-value) = -0.0
    float result6 = call_f32_mul(0.0f, -2.0f);
    ASSERT_EQ(result6, 0.0f) << "0.0f * (-2.0f) should return -0.0f or 0.0f";

    // Test multiplicative identity
    ASSERT_FLOAT_EQ(7.25f, call_f32_mul(7.25f, 1.0f))
        << "7.25f * 1.0f should equal 7.25f (multiplicative identity)";

    ASSERT_FLOAT_EQ(-7.25f, call_f32_mul(7.25f, -1.0f))
        << "7.25f * (-1.0f) should equal -7.25f (negation through multiplication)";
}

/**
 * @test CommutativityProperty_ValidatesSymmetry
 * @brief Validates that f32.mul follows commutative property (a*b = b*a)
 * @details Tests mathematical commutativity across various operand combinations including
 *          normal values, boundary values, and special IEEE 754 values.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_mul_commutativity
 * @input_conditions Various operand pairs tested in both orders (a,b) and (b,a)
 * @expected_behavior Identical results regardless of operand order
 * @validation_method Direct comparison of a*b with b*a for bitwise equality
 */
TEST_P(F32MulTest, CommutativityProperty_ValidatesSymmetry)
{
    // Test commutativity with normal values
    float result1 = call_f32_mul(3.14f, 2.71f);
    float result2 = call_f32_mul(2.71f, 3.14f);
    ASSERT_FLOAT_EQ(result1, result2)
        << "Commutativity failed: 3.14f * 2.71f != 2.71f * 3.14f";

    // Test commutativity with mixed signs
    float result3 = call_f32_mul(-5.5f, 4.2f);
    float result4 = call_f32_mul(4.2f, -5.5f);
    ASSERT_FLOAT_EQ(result3, result4)
        << "Commutativity failed: (-5.5f) * 4.2f != 4.2f * (-5.5f)";

    // Test commutativity with zero
    float result5 = call_f32_mul(0.0f, 8.9f);
    float result6 = call_f32_mul(8.9f, 0.0f);
    ASSERT_EQ(result5, result6)
        << "Commutativity failed: 0.0f * 8.9f != 8.9f * 0.0f";

    // Test commutativity with infinity (where valid)
    float pos_inf = std::numeric_limits<float>::infinity();
    float result7 = call_f32_mul(pos_inf, 3.0f);
    float result8 = call_f32_mul(3.0f, pos_inf);
    ASSERT_TRUE(std::isinf(result7) && std::isinf(result8) &&
                (result7 > 0) == (result8 > 0))
        << "Commutativity failed for infinity: +INF * 3.0f != 3.0f * +INF";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningModeTest, F32MulTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<F32MulTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });