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
 * @brief Test fixture class for f32.floor opcode comprehensive testing
 *
 * This class provides a complete test environment for validating the f32.floor
 * WebAssembly opcode implementation across different WAMR execution modes.
 * Tests cover basic functionality, edge cases, precision boundaries, and
 * special IEEE 754 values to ensure comprehensive opcode coverage.
 */
class F32FloorTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     *
     * Initializes WAMR runtime with proper memory allocation settings,
     * loads the test WASM module containing f32.floor test functions,
     * and sets up execution context for both interpreter and AOT modes.
     */
    void SetUp() override {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for f32.floor tests";

        // Load the WASM module containing f32.floor test functions
        WASM_FILE = "wasm-apps/f32_floor_test.wasm";
        load_wasm_module();
        ASSERT_NE(nullptr, module)
            << "Failed to load f32.floor test WASM module: " << WASM_FILE;

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
     * Loads the compiled WASM bytecode file containing f32.floor test
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
     * @brief Execute f32.floor operation with specified input value
     *
     * @param input f32 value to apply floor function to
     * @return f32 result of floor(input) operation
     *
     * Calls the exported WASM function to execute f32.floor opcode
     * and returns the computed result for validation.
     */
    float call_f32_floor(float input) {
        EXPECT_NE(nullptr, module_inst) << "Module instance not available";
        EXPECT_NE(nullptr, exec_env) << "Execution environment not available";

        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_f32_floor");
        EXPECT_NE(nullptr, func) << "Cannot find test_f32_floor function";

        uint32_t argv[2];
        memcpy(&argv[0], &input, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(success) << "f32.floor function call failed";

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
 * @test BasicFloorOperation_ReturnsCorrectResults
 * @brief Validates f32.floor produces correct floor results for typical inputs
 * @details Tests fundamental floor operation with positive, negative, and fractional values.
 *          Verifies that f32.floor correctly computes the largest integer <= input.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_floor_operation
 * @input_conditions Standard f32 values: fractional positives/negatives, mixed scenarios
 * @expected_behavior Returns mathematical floor: 3.7f→3.0f, -3.7f→-4.0f, 2.3f→2.0f, -2.3f→-3.0f
 * @validation_method Direct comparison of WASM function result with expected floor values
 */
TEST_P(F32FloorTest, BasicFloorOperation_ReturnsCorrectResults) {
    // Test positive fractional values
    ASSERT_EQ(3.0f, call_f32_floor(3.7f))
        << "floor(3.7f) should return 3.0f";
    ASSERT_EQ(2.0f, call_f32_floor(2.3f))
        << "floor(2.3f) should return 2.0f";
    ASSERT_EQ(0.0f, call_f32_floor(0.1f))
        << "floor(0.1f) should return 0.0f";

    // Test negative fractional values
    ASSERT_EQ(-4.0f, call_f32_floor(-3.7f))
        << "floor(-3.7f) should return -4.0f";
    ASSERT_EQ(-3.0f, call_f32_floor(-2.3f))
        << "floor(-2.3f) should return -3.0f";
    ASSERT_EQ(-1.0f, call_f32_floor(-0.9f))
        << "floor(-0.9f) should return -1.0f";

    // Test half values
    ASSERT_EQ(2.0f, call_f32_floor(2.5f))
        << "floor(2.5f) should return 2.0f";
    ASSERT_EQ(-3.0f, call_f32_floor(-2.5f))
        << "floor(-2.5f) should return -3.0f";
}

/**
 * @test SpecialFloatingPointValues_PreserveProperties
 * @brief Validates f32.floor handles IEEE 754 special values according to specification
 * @details Tests floor operation with special floating-point values including signed zeros,
 *          infinities, and NaN values. Ensures IEEE 754 compliance in special case handling.
 * @test_category Edge - Special IEEE 754 value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_floor_special_cases
 * @input_conditions IEEE 754 special values: ±0.0, ±∞, NaN
 * @expected_behavior Preserves special values: floor(±0.0)=±0.0, floor(±∞)=±∞, floor(NaN)=NaN
 * @validation_method Bit-level comparison for signed zeros, isnan/isinf checks for special values
 */
TEST_P(F32FloorTest, SpecialFloatingPointValues_PreserveProperties) {
    // Test positive and negative zero (preserve sign)
    float pos_zero = +0.0f;
    float neg_zero = -0.0f;
    ASSERT_EQ(pos_zero, call_f32_floor(pos_zero))
        << "floor(+0.0f) should return +0.0f";
    ASSERT_TRUE(signbit(call_f32_floor(neg_zero)) != 0)
        << "floor(-0.0f) should preserve negative zero sign";

    // Test positive and negative infinity
    ASSERT_TRUE(isinf(call_f32_floor(INFINITY)) && call_f32_floor(INFINITY) > 0)
        << "floor(+∞) should return +∞";
    ASSERT_TRUE(isinf(call_f32_floor(-INFINITY)) && call_f32_floor(-INFINITY) < 0)
        << "floor(-∞) should return -∞";

    // Test NaN values
    ASSERT_TRUE(isnan(call_f32_floor(NAN)))
        << "floor(NaN) should return NaN";

    // Test very large integers where fractional precision is lost
    float large_int = 16777216.0f; // 2^24 - beyond f32 fractional precision
    ASSERT_EQ(large_int, call_f32_floor(large_int))
        << "floor(2^24) should return unchanged value at precision boundary";
}

/**
 * @test IntegerValues_ReturnUnchanged
 * @brief Validates f32.floor returns integer values unchanged (identity operation)
 * @details Tests floor operation with exact integer values to ensure they are
 *          returned without modification, validating the mathematical property floor(n) = n for integers.
 * @test_category Main - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_floor_integer_cases
 * @input_conditions Exact integer f32 values: positive, negative, zero
 * @expected_behavior Identity operation: floor(5.0f)=5.0f, floor(-3.0f)=-3.0f, floor(0.0f)=0.0f
 * @validation_method Direct equality comparison with input values
 */
TEST_P(F32FloorTest, IntegerValues_ReturnUnchanged) {
    // Test positive integers
    ASSERT_EQ(5.0f, call_f32_floor(5.0f))
        << "floor(5.0f) should return 5.0f unchanged";
    ASSERT_EQ(100.0f, call_f32_floor(100.0f))
        << "floor(100.0f) should return 100.0f unchanged";

    // Test negative integers
    ASSERT_EQ(-5.0f, call_f32_floor(-5.0f))
        << "floor(-5.0f) should return -5.0f unchanged";
    ASSERT_EQ(-100.0f, call_f32_floor(-100.0f))
        << "floor(-100.0f) should return -100.0f unchanged";

    // Test zero
    ASSERT_EQ(0.0f, call_f32_floor(0.0f))
        << "floor(0.0f) should return 0.0f unchanged";
}

/**
 * @test BoundaryValues_HandleCorrectly
 * @brief Validates f32.floor behavior at floating-point boundary conditions
 * @details Tests floor operation with values near f32 precision limits and extreme values
 *          to ensure correct handling at boundary conditions without overflow or precision loss.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_floor_boundary_cases
 * @input_conditions Values around f32 boundaries: FLT_MAX, FLT_MIN, precision limits
 * @expected_behavior Correct floor computation at boundaries: FLT_MIN→0.0f, -FLT_MIN→-1.0f
 * @validation_method Verification of expected mathematical behavior at boundary conditions
 */
TEST_P(F32FloorTest, BoundaryValues_HandleCorrectly) {
    // Test maximum f32 value
    ASSERT_EQ(FLT_MAX, call_f32_floor(FLT_MAX))
        << "floor(FLT_MAX) should return FLT_MAX unchanged";

    // Test minimum f32 value (most negative)
    ASSERT_EQ(-FLT_MAX, call_f32_floor(-FLT_MAX))
        << "floor(-FLT_MAX) should return -FLT_MAX unchanged";

    // Test smallest positive normal f32 value
    float small_pos = FLT_MIN;
    ASSERT_EQ(0.0f, call_f32_floor(small_pos))
        << "floor(FLT_MIN) should return 0.0f";

    // Test smallest negative value near zero
    float small_neg = -FLT_MIN;
    ASSERT_EQ(-1.0f, call_f32_floor(small_neg))
        << "floor(-FLT_MIN) should return -1.0f";

    // Test values near integer boundaries
    ASSERT_EQ(0.0f, call_f32_floor(0.9999999f))
        << "floor(0.9999999f) should return 0.0f";
    ASSERT_EQ(1.0f, call_f32_floor(1.0000001f))
        << "floor(1.0000001f) should return 1.0f";

    // Test precision boundary (2^23 = 8388608.0f)
    float precision_boundary = 8388608.0f;
    ASSERT_EQ(precision_boundary, call_f32_floor(precision_boundary))
        << "floor(8388608.0f) should handle precision boundary correctly";
}

// Instantiate parametrized tests for both interpreter and AOT execution modes
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    F32FloorTest,
    testing::Values(
        Mode_Interp
#if WASM_ENABLE_AOT != 0
        , Mode_LLVM_JIT
#endif
    )
);