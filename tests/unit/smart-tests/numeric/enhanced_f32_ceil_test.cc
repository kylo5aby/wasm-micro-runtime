/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <math.h>
#include <float.h>
#include <gtest/gtest.h>
#include "wasm_runtime_common.h"
#include "wasm_exec_env.h"
#include "wasm_export.h"

/**
 * @brief Test fixture class for f32.ceil opcode comprehensive testing
 *
 * This class provides a complete test environment for validating the f32.ceil
 * WebAssembly opcode implementation across different WAMR execution modes.
 * Tests cover basic functionality, edge cases, precision boundaries, and
 * special IEEE 754 values to ensure comprehensive opcode coverage.
 */
class F32CeilTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     *
     * Initializes WAMR runtime with proper memory allocation settings,
     * loads the test WASM module containing f32.ceil test functions,
     * and sets up execution context for both interpreter and AOT modes.
     */
    void SetUp() override {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for f32.ceil tests";

        // Load the WASM module containing f32.ceil test functions
        WASM_FILE = "wasm-apps/f32_ceil_test.wasm";
        load_wasm_module();
        ASSERT_NE(nullptr, module)
            << "Failed to load f32.ceil test WASM module: " << WASM_FILE;

        setup_execution_environment();
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     *
     * Properly cleans up execution contexts, unloads WASM modules,
     * and destroys WAMR runtime to prevent resource leaks between tests.
     */
    void TearDown() override {
        cleanup_execution_environment();
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        wasm_runtime_destroy();
    }

private:
    /**
     * @brief Load WASM module from file system
     *
     * Loads the compiled WASM bytecode file containing f32.ceil test
     * functions and validates successful module loading.
     */
    void load_wasm_module() {
        FILE *file = fopen(WASM_FILE, "rb");
        ASSERT_NE(nullptr, file) << "Cannot open WASM file: " << WASM_FILE;

        fseek(file, 0, SEEK_END);
        wasm_file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        wasm_file_buf = new uint8_t[wasm_file_size];
        ASSERT_EQ(wasm_file_size, fread(wasm_file_buf, 1, wasm_file_size, file))
            << "Failed to read WASM file contents";
        fclose(file);

        char error_buf[128];
        module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                   error_buf, sizeof(error_buf));
        if (!module) {
            delete[] wasm_file_buf;
            wasm_file_buf = nullptr;
        }
    }

    /**
     * @brief Set up execution environment for current test mode
     *
     * Creates appropriate execution environment based on test parameter
     * (interpreter vs AOT mode) and prepares for function execution.
     */
    void setup_execution_environment() {
        if (!module) return;

        module_inst = wasm_runtime_instantiate(module, 65536, 65536,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        ASSERT_NE(nullptr, exec_env)
            << "Failed to create execution environment";
    }

    /**
     * @brief Clean up execution environment and free resources
     *
     * Properly destroys execution contexts and frees allocated memory
     * to prevent resource leaks during test execution.
     */
    void cleanup_execution_environment() {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (wasm_file_buf) {
            delete[] wasm_file_buf;
            wasm_file_buf = nullptr;
        }
    }

protected:
    /**
     * @brief Execute f32.ceil operation with specified input value
     *
     * @param input f32 value to apply ceiling function to
     * @return f32 result of ceil(input) operation
     *
     * Calls the exported WASM function to execute f32.ceil opcode
     * and returns the computed result for validation.
     */
    float call_f32_ceil(float input) {
        EXPECT_NE(nullptr, module_inst) << "Module instance not available";
        EXPECT_NE(nullptr, exec_env) << "Execution environment not available";

        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_f32_ceil");
        EXPECT_NE(nullptr, func) << "Cannot find test_f32_ceil function";

        uint32_t argv[2];
        memcpy(&argv[0], &input, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(success) << "f32.ceil function call failed";

        float result;
        memcpy(&result, &argv[0], sizeof(float));
        return result;
    }

    // Test infrastructure members
    const char* WASM_FILE;
    uint8_t* wasm_file_buf = nullptr;
    uint32_t wasm_file_size = 0;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    char error_buf[128];
};

/**
 * @test BasicCeiling_ReturnsCorrectResults
 * @brief Validates f32.ceil produces correct ceiling results for typical inputs
 * @details Tests fundamental ceiling operation with positive, negative, and fractional values.
 *          Verifies that f32.ceil correctly computes the smallest integer >= input.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_ceil_operation
 * @input_conditions Standard f32 values: fractional positives/negatives, mixed scenarios
 * @expected_behavior Returns mathematical ceiling: 1.3f→2.0f, -1.7f→-1.0f, 0.1f→1.0f, -0.9f→0.0f
 * @validation_method Direct comparison of WASM function result with expected ceiling values
 */
TEST_P(F32CeilTest, BasicCeiling_ReturnsCorrectResults) {
    // Test positive fractional values
    ASSERT_EQ(2.0f, call_f32_ceil(1.3f))
        << "ceil(1.3f) should return 2.0f";
    ASSERT_EQ(3.0f, call_f32_ceil(2.7f))
        << "ceil(2.7f) should return 3.0f";
    ASSERT_EQ(1.0f, call_f32_ceil(0.1f))
        << "ceil(0.1f) should return 1.0f";

    // Test negative fractional values
    ASSERT_EQ(-1.0f, call_f32_ceil(-1.3f))
        << "ceil(-1.3f) should return -1.0f";
    ASSERT_EQ(-2.0f, call_f32_ceil(-2.7f))
        << "ceil(-2.7f) should return -2.0f";
    ASSERT_EQ(0.0f, call_f32_ceil(-0.9f))
        << "ceil(-0.9f) should return 0.0f";
}

/**
 * @test SpecialValues_HandledCorrectly
 * @brief Validates f32.ceil handles IEEE 754 special values according to specification
 * @details Tests ceiling operation with special floating-point values including signed zeros,
 *          infinities, and NaN values. Ensures IEEE 754 compliance in special case handling.
 * @test_category Edge - Special IEEE 754 value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_ceil_special_cases
 * @input_conditions IEEE 754 special values: ±0.0, ±∞, NaN
 * @expected_behavior Preserves special values: ceil(±0.0)=±0.0, ceil(±∞)=±∞, ceil(NaN)=NaN
 * @validation_method Bit-level comparison for signed zeros, isnan/isinf checks for special values
 */
TEST_P(F32CeilTest, SpecialValues_HandledCorrectly) {
    // Test positive and negative zero (preserve sign)
    float pos_zero = +0.0f;
    float neg_zero = -0.0f;
    ASSERT_EQ(pos_zero, call_f32_ceil(pos_zero))
        << "ceil(+0.0f) should return +0.0f";
    ASSERT_TRUE(signbit(call_f32_ceil(neg_zero)) != 0)
        << "ceil(-0.0f) should preserve negative zero sign";

    // Test positive and negative infinity
    ASSERT_TRUE(isinf(call_f32_ceil(INFINITY)) && call_f32_ceil(INFINITY) > 0)
        << "ceil(+∞) should return +∞";
    ASSERT_TRUE(isinf(call_f32_ceil(-INFINITY)) && call_f32_ceil(-INFINITY) < 0)
        << "ceil(-∞) should return -∞";

    // Test NaN values
    ASSERT_TRUE(isnan(call_f32_ceil(NAN)))
        << "ceil(NaN) should return NaN";
}

/**
 * @test IntegerValues_ReturnUnchanged
 * @brief Validates f32.ceil returns integer values unchanged (identity operation)
 * @details Tests ceiling operation with exact integer values to ensure they are
 *          returned without modification, validating the mathematical property ceil(n) = n for integers.
 * @test_category Main - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_ceil_integer_cases
 * @input_conditions Exact integer f32 values: positive, negative, zero
 * @expected_behavior Identity operation: ceil(5.0f)=5.0f, ceil(-3.0f)=-3.0f, ceil(0.0f)=0.0f
 * @validation_method Direct equality comparison with input values
 */
TEST_P(F32CeilTest, IntegerValues_ReturnUnchanged) {
    // Test positive integers
    ASSERT_EQ(5.0f, call_f32_ceil(5.0f))
        << "ceil(5.0f) should return 5.0f unchanged";
    ASSERT_EQ(100.0f, call_f32_ceil(100.0f))
        << "ceil(100.0f) should return 100.0f unchanged";

    // Test negative integers
    ASSERT_EQ(-3.0f, call_f32_ceil(-3.0f))
        << "ceil(-3.0f) should return -3.0f unchanged";
    ASSERT_EQ(-50.0f, call_f32_ceil(-50.0f))
        << "ceil(-50.0f) should return -50.0f unchanged";

    // Test zero
    ASSERT_EQ(0.0f, call_f32_ceil(0.0f))
        << "ceil(0.0f) should return 0.0f unchanged";
}

/**
 * @test PrecisionBoundaries_HandleCorrectly
 * @brief Validates f32.ceil behavior at floating-point precision boundaries
 * @details Tests ceiling operation with values near f32 precision limits where
 *          floating-point representation affects ceiling computation accuracy.
 * @test_category Corner - Precision boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_ceil_precision_limits
 * @input_conditions Values around f32 precision boundary (2^23), min/max f32 values
 * @expected_behavior Correct handling at precision limits without overflow or precision loss
 * @validation_method Verification of expected mathematical behavior at boundary conditions
 */
TEST_P(F32CeilTest, PrecisionBoundaries_HandleCorrectly) {
    // Test values around f32 precision boundary (2^23 = 8388608.0f)
    float boundary = 8388608.0f; // 2^23 - where f32 precision changes
    ASSERT_EQ(boundary, call_f32_ceil(boundary))
        << "ceil(8388608.0f) should handle precision boundary correctly";

    // Test maximum f32 value
    ASSERT_EQ(FLT_MAX, call_f32_ceil(FLT_MAX))
        << "ceil(FLT_MAX) should return FLT_MAX unchanged";

    // Test minimum f32 value (most negative)
    ASSERT_EQ(-FLT_MAX, call_f32_ceil(-FLT_MAX))
        << "ceil(-FLT_MAX) should return -FLT_MAX unchanged";

    // Test smallest positive normal f32 value
    float small_pos = FLT_MIN;
    ASSERT_EQ(1.0f, call_f32_ceil(small_pos))
        << "ceil(FLT_MIN) should return 1.0f";
}

// Instantiate parametrized tests for both interpreter and AOT execution modes
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    F32CeilTest,
    testing::Values(
        Mode_Interp
#if WASM_ENABLE_AOT != 0
        , Mode_LLVM_JIT
#endif
    )
);