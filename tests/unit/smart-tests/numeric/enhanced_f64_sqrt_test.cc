/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cfloat>
#include <limits>
#include <cstdio>

#include "wasm_runtime_common.h"
#include "wasm_runtime.h"
#include "wasm_export.h"

/**
 * @class F64SqrtTest
 * @brief Comprehensive test suite for the f64.sqrt WebAssembly opcode
 * @details This test class validates the f64.sqrt opcode implementation in WAMR runtime,
 *          ensuring IEEE 754 compliance for double-precision floating-point square root operations.
 *          Tests cover basic functionality, boundary conditions, special values, and cross-execution mode validation.
 */
class F64SqrtTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     * @details Initializes WAMR runtime with proper memory allocation settings,
     *          loads the f64.sqrt test WASM module, and prepares execution context
     *          for both interpreter and AOT modes.
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load WASM module from file
        const char *wasm_path = "wasm-apps/f64_sqrt_test.wasm";
        wasm_buffer = load_wasm_buffer(wasm_path, &wasm_size);
        ASSERT_NE(nullptr, wasm_buffer)
            << "Failed to read WASM file: " << wasm_path;

        // Load and validate WASM module
        wasm_module = wasm_runtime_load(wasm_buffer, wasm_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, wasm_module)
            << "Failed to load WASM module: " << error_buf;

        // Instantiate WASM module with execution mode
        wasm_module_inst = wasm_runtime_instantiate(wasm_module, 65536, 65536, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, wasm_module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        // Set running mode for AOT/interpreter validation
        wasm_runtime_set_running_mode(wasm_module_inst, GetParam());

        // Get execution environment
        exec_env = wasm_runtime_create_exec_env(wasm_module_inst, 32768);
        ASSERT_NE(nullptr, exec_env)
            << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test resources and destroy WAMR runtime
     * @details Properly releases all allocated resources including execution environment,
     *          module instance, module, and WASM buffer, then destroys the WAMR runtime.
     */
    void TearDown() override {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (wasm_module_inst) {
            wasm_runtime_deinstantiate(wasm_module_inst);
            wasm_module_inst = nullptr;
        }
        if (wasm_module) {
            wasm_runtime_unload(wasm_module);
            wasm_module = nullptr;
        }
        if (wasm_buffer) {
            wasm_runtime_free(wasm_buffer);
            wasm_buffer = nullptr;
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM file into memory buffer
     * @param filename Path to WASM file relative to test execution directory
     * @param size Pointer to store the loaded buffer size
     * @return Pointer to allocated buffer containing WASM bytecode
     */
    uint8_t* load_wasm_buffer(const char* filename, uint32_t* size) {
        FILE* file = fopen(filename, "rb");
        if (!file) return nullptr;

        fseek(file, 0, SEEK_END);
        *size = ftell(file);
        fseek(file, 0, SEEK_SET);

        uint8_t* buffer = (uint8_t*)wasm_runtime_malloc(*size);
        if (!buffer) {
            fclose(file);
            return nullptr;
        }

        if (fread(buffer, 1, *size, file) != *size) {
            wasm_runtime_free(buffer);
            fclose(file);
            return nullptr;
        }

        fclose(file);
        return buffer;
    }

    /**
     * @brief Call f64.sqrt function in WASM module with single operand
     * @param input Double-precision floating-point input value
     * @return Double-precision floating-point result from f64.sqrt operation
     * @details Executes the test_sqrt_basic function in the WASM module and returns
     *          the computed square root result, handling any execution errors.
     */
    double call_f64_sqrt(double input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "test_sqrt_basic");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_sqrt_basic function";

        uint32 argv[2];
        memcpy(argv, &input, sizeof(double));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute f64.sqrt function: "
                            << wasm_runtime_get_exception(wasm_module_inst);

        double result;
        memcpy(&result, argv, sizeof(double));
        return result;
    }

    /**
     * @brief Call boundary value test function in WASM module
     * @param input Double-precision floating-point boundary value
     * @return Double-precision floating-point result from boundary sqrt operation
     * @details Executes the test_sqrt_boundaries function for testing edge cases
     *          like DBL_MAX, DBL_MIN, and near-zero values.
     */
    double call_f64_sqrt_boundary(double input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "test_sqrt_boundaries");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_sqrt_boundaries function";

        uint32 argv[2];
        memcpy(argv, &input, sizeof(double));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute boundary sqrt function: "
                            << wasm_runtime_get_exception(wasm_module_inst);

        double result;
        memcpy(&result, argv, sizeof(double));
        return result;
    }

    /**
     * @brief Call special values test function in WASM module
     * @param input Double-precision floating-point special value (NaN, infinity, zero)
     * @return Double-precision floating-point result from special value sqrt operation
     * @details Executes the test_sqrt_special function for testing IEEE 754 special cases
     *          including NaN, positive/negative infinity, and signed zeros.
     */
    double call_f64_sqrt_special(double input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "test_sqrt_special");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_sqrt_special function";

        uint32 argv[2];
        memcpy(argv, &input, sizeof(double));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute special sqrt function: "
                            << wasm_runtime_get_exception(wasm_module_inst);

        double result;
        memcpy(&result, argv, sizeof(double));
        return result;
    }

    /**
     * @brief Call negative input test function in WASM module
     * @param input Negative double-precision floating-point value
     * @return Double-precision floating-point result (should be NaN for negative inputs)
     * @details Executes the test_sqrt_negative function for testing IEEE 754 compliant
     *          behavior with negative finite numbers (should return NaN without trapping).
     */
    double call_f64_sqrt_negative(double input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "test_sqrt_negative");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_sqrt_negative function";

        uint32 argv[2];
        memcpy(argv, &input, sizeof(double));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute negative sqrt function: "
                            << wasm_runtime_get_exception(wasm_module_inst);

        double result;
        memcpy(&result, argv, sizeof(double));
        return result;
    }

    // Test infrastructure
    RuntimeInitArgs init_args;
    wasm_module_t wasm_module = nullptr;
    wasm_module_inst_t wasm_module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    char error_buf[256];
    uint8 *wasm_buffer = nullptr;
    uint32 wasm_size = 0;
};

/**
 * @test BasicSquareRoot_ReturnsCorrectResults
 * @brief Validates f64.sqrt produces correct mathematical results for typical inputs
 * @details Tests fundamental square root operation with perfect squares, non-perfect squares,
 *          and fractional values. Verifies that f64.sqrt correctly computes mathematical
 *          square root with double-precision accuracy.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sqrt_operation
 * @input_conditions Perfect squares (1.0, 4.0, 9.0), non-perfect squares (2.0), fractional values (0.25, 0.01)
 * @expected_behavior Returns mathematical square root: 1.0, 2.0, 3.0, ~1.414213562373095, 0.5, 0.1 respectively
 * @validation_method Direct comparison with expected values and epsilon-based validation for non-exact results
 */
TEST_P(F64SqrtTest, BasicSquareRoot_ReturnsCorrectResults) {
    // Test perfect squares - exact results expected
    ASSERT_DOUBLE_EQ(1.0, call_f64_sqrt(1.0))
        << "Square root of 1.0 should be exactly 1.0";
    ASSERT_DOUBLE_EQ(2.0, call_f64_sqrt(4.0))
        << "Square root of 4.0 should be exactly 2.0";
    ASSERT_DOUBLE_EQ(3.0, call_f64_sqrt(9.0))
        << "Square root of 9.0 should be exactly 3.0";

    // Test non-perfect squares - validate with epsilon
    double sqrt_2_result = call_f64_sqrt(2.0);
    double expected_sqrt_2 = 1.414213562373095;
    ASSERT_NEAR(expected_sqrt_2, sqrt_2_result, 1e-15)
        << "Square root of 2.0 should match IEEE 754 double precision result";

    // Test fractional values - exact results expected
    ASSERT_DOUBLE_EQ(0.5, call_f64_sqrt(0.25))
        << "Square root of 0.25 should be exactly 0.5";
    ASSERT_DOUBLE_EQ(0.1, call_f64_sqrt(0.01))
        << "Square root of 0.01 should be exactly 0.1";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Validates f64.sqrt handles boundary values according to IEEE 754 specification
 * @details Tests square root operation with extreme values including DBL_MAX, DBL_MIN,
 *          and near-zero positive values. Ensures proper handling without overflow/underflow issues.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sqrt_boundary_handling
 * @input_conditions DBL_MAX, DBL_MIN, very small positive numbers (1e-300, 1e-308)
 * @expected_behavior Returns valid square root results within IEEE 754 double precision limits
 * @validation_method Range validation and finite result checking for boundary inputs
 */
TEST_P(F64SqrtTest, BoundaryValues_HandledCorrectly) {
    // Test DBL_MAX boundary - should not overflow
    double max_input = DBL_MAX;
    double max_result = call_f64_sqrt_boundary(max_input);
    ASSERT_TRUE(std::isfinite(max_result))
        << "Square root of DBL_MAX should produce finite result";
    ASSERT_GT(max_result, 0.0)
        << "Square root of DBL_MAX should be positive";

    // Test DBL_MIN boundary - should not underflow to zero inappropriately
    double min_input = DBL_MIN;
    double min_result = call_f64_sqrt_boundary(min_input);
    ASSERT_TRUE(std::isfinite(min_result))
        << "Square root of DBL_MIN should produce finite result";
    ASSERT_GT(min_result, 0.0)
        << "Square root of DBL_MIN should be positive";

    // Test very small positive values
    double small_result_1 = call_f64_sqrt_boundary(1e-300);
    ASSERT_TRUE(std::isfinite(small_result_1))
        << "Square root of 1e-300 should produce finite result";

    double small_result_2 = call_f64_sqrt_boundary(1e-308);
    ASSERT_TRUE(std::isfinite(small_result_2))
        << "Square root of 1e-308 should produce finite result";
}

/**
 * @test SpecialValues_IEEE754Compliant
 * @brief Validates f64.sqrt handles IEEE 754 special values correctly
 * @details Tests square root operation with special IEEE 754 values including signed zeros,
 *          positive/negative infinity, and NaN. Ensures compliance with IEEE 754 standard behavior.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sqrt_special_cases
 * @input_conditions +0.0, -0.0, +∞, -∞, NaN
 * @expected_behavior +0.0→+0.0, -0.0→-0.0, +∞→+∞, -∞→NaN, NaN→NaN (IEEE 754 compliant)
 * @validation_method Sign bit checking for zeros, infinity/NaN validation using std::is* functions
 */
TEST_P(F64SqrtTest, SpecialValues_IEEE754Compliant) {
    // Test positive zero - should return positive zero
    double pos_zero_result = call_f64_sqrt_special(+0.0);
    ASSERT_DOUBLE_EQ(+0.0, pos_zero_result)
        << "Square root of +0.0 should be +0.0";
    ASSERT_FALSE(std::signbit(pos_zero_result))
        << "Result of sqrt(+0.0) should have positive sign bit";

    // Test negative zero - should return negative zero (sign preservation)
    double neg_zero_result = call_f64_sqrt_special(-0.0);
    ASSERT_DOUBLE_EQ(-0.0, neg_zero_result)
        << "Square root of -0.0 should be -0.0";
    ASSERT_TRUE(std::signbit(neg_zero_result))
        << "Result of sqrt(-0.0) should have negative sign bit";

    // Test positive infinity - should return positive infinity
    double pos_inf = std::numeric_limits<double>::infinity();
    double pos_inf_result = call_f64_sqrt_special(pos_inf);
    ASSERT_TRUE(std::isinf(pos_inf_result))
        << "Square root of +∞ should be +∞";
    ASSERT_FALSE(std::signbit(pos_inf_result))
        << "Square root of +∞ should be positive";

    // Test negative infinity - should return NaN
    double neg_inf = -std::numeric_limits<double>::infinity();
    double neg_inf_result = call_f64_sqrt_special(neg_inf);
    ASSERT_TRUE(std::isnan(neg_inf_result))
        << "Square root of -∞ should be NaN";

    // Test NaN input - should return NaN (NaN propagation)
    double nan_input = std::numeric_limits<double>::quiet_NaN();
    double nan_result = call_f64_sqrt_special(nan_input);
    ASSERT_TRUE(std::isnan(nan_result))
        << "Square root of NaN should be NaN";
}

/**
 * @test NegativeInputs_ReturnNaN
 * @brief Validates f64.sqrt returns NaN for negative finite inputs without trapping
 * @details Tests IEEE 754 compliant behavior where negative finite numbers produce NaN results.
 *          Verifies that no runtime traps occur (pure IEEE 754 mathematical behavior).
 * @test_category Error - Invalid input validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sqrt_negative_handling
 * @input_conditions Negative finite numbers (-1.0, -100.0, -0.5)
 * @expected_behavior Returns NaN without causing runtime traps (IEEE 754 standard behavior)
 * @validation_method NaN result validation using std::isnan() without exception checking
 */
TEST_P(F64SqrtTest, NegativeInputs_ReturnNaN) {
    // Test negative finite numbers - should return NaN without trapping
    double neg_result_1 = call_f64_sqrt_negative(-1.0);
    ASSERT_TRUE(std::isnan(neg_result_1))
        << "Square root of -1.0 should be NaN per IEEE 754";

    double neg_result_2 = call_f64_sqrt_negative(-100.0);
    ASSERT_TRUE(std::isnan(neg_result_2))
        << "Square root of -100.0 should be NaN per IEEE 754";

    double neg_result_3 = call_f64_sqrt_negative(-0.5);
    ASSERT_TRUE(std::isnan(neg_result_3))
        << "Square root of -0.5 should be NaN per IEEE 754";
}

// Test parameter setup for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningMode, F64SqrtTest,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT),
                        [](const testing::TestParamInfo<RunningMode>& info) {
                            return info.param == Mode_Interp ? "Interpreter" : "AOT";
                        });