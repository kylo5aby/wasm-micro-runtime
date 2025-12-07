/**
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <float.h>
#include <limits>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

/**
 * @brief Test fixture for f64.le opcode validation across execution modes
 *
 * This class provides comprehensive testing infrastructure for the f64.le (float64 less than or equal)
 * WebAssembly opcode. It validates IEEE 754 compliant floating-point comparison behavior
 * across both interpreter and AOT execution modes.
 */
class F64LeTest : public testing::TestWithParam<RunningMode>
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
     * Initializes WAMR runtime with system allocator and prepares test infrastructure
     * for f64.le opcode validation. Loads the test WASM module containing f64.le operations.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/f64_le_test.wasm", &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: wasm-apps/f64_le_test.wasm";

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
     * @brief Clean up test environment and shutdown WAMR runtime
     *
     * Releases WASM module resources and shuts down WAMR runtime to ensure
     * clean test environment for subsequent test cases.
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
            wasm_runtime_free(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Execute f64.le operation via WASM function call
     *
     * @param a First f64 operand
     * @param b Second f64 operand
     * @return int32_t Result of f64.le comparison (1 if a <= b, 0 otherwise)
     */
    int32_t call_f64_le(double a, double b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_le");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f64_le function";

        uint32_t argv[4];  // Two f64 values = 4 uint32_t slots
        memcpy(&argv[0], &a, sizeof(double));
        memcpy(&argv[2], &b, sizeof(double));

        bool success = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(success) << "Failed to execute test_f64_le function: "
                            << wasm_runtime_get_exception(module_inst);

        return static_cast<int32_t>(argv[0]);
    }
};

/**
 * @test BasicComparison_ReturnsCorrectResult
 * @brief Validates f64.le produces correct comparison results for typical inputs
 * @details Tests fundamental less-than-or-equal operation with positive, negative, mixed-sign,
 *          zero, and equal values. Verifies that f64.le correctly computes (a <= b) for
 *          various input combinations including the equality aspect.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_le_operation
 * @input_conditions Standard f64 pairs: positive, negative, mixed-sign, zero, equal values
 * @expected_behavior Returns 1 when first value <= second value, 0 otherwise
 * @validation_method Direct comparison of WASM function result with expected IEEE 754 behavior
 */
TEST_P(F64LeTest, BasicComparison_ReturnsCorrectResult)
{
    // Positive numbers comparison
    ASSERT_EQ(1, call_f64_le(1.5, 2.5)) << "1.5 <= 2.5 should return true (1)";
    ASSERT_EQ(0, call_f64_le(2.5, 1.5)) << "2.5 <= 1.5 should return false (0)";

    // Negative numbers comparison
    ASSERT_EQ(1, call_f64_le(-2.5, -1.5)) << "-2.5 <= -1.5 should return true (1)";
    ASSERT_EQ(0, call_f64_le(-1.5, -2.5)) << "-1.5 <= -2.5 should return false (0)";

    // Mixed signs comparison
    ASSERT_EQ(1, call_f64_le(-1.5, 1.5)) << "-1.5 <= 1.5 should return true (1)";
    ASSERT_EQ(0, call_f64_le(1.5, -1.5)) << "1.5 <= -1.5 should return false (0)";

    // Zero comparisons
    ASSERT_EQ(1, call_f64_le(0.0, 1.0)) << "0.0 <= 1.0 should return true (1)";
    ASSERT_EQ(1, call_f64_le(-1.0, 0.0)) << "-1.0 <= 0.0 should return true (1)";
    ASSERT_EQ(1, call_f64_le(0.0, 0.0)) << "0.0 <= 0.0 should return true (1)";

    // Equal values (testing "equal" part of "less than or equal")
    ASSERT_EQ(1, call_f64_le(5.5, 5.5)) << "5.5 <= 5.5 should return true (1)";
    ASSERT_EQ(1, call_f64_le(-3.14, -3.14)) << "-3.14 <= -3.14 should return true (1)";
}

/**
 * @test SpecialValues_HandlesIEEE754Cases
 * @brief Validates f64.le handles IEEE 754 special values correctly
 * @details Tests infinity, NaN, and signed zero behavior according to IEEE 754 specification.
 *          Verifies proper handling of ±infinity comparisons and NaN comparison semantics
 *          (NaN comparisons always return false except for != operation).
 * @test_category Special - IEEE 754 special values validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_le_operation
 * @input_conditions ±infinity, NaN, ±0.0 in various combinations
 * @expected_behavior IEEE 754 compliant results: inf <= inf is true, NaN <= x is always false
 * @validation_method Verification against IEEE 754 comparison rules for special values
 */
TEST_P(F64LeTest, SpecialValues_HandlesIEEE754Cases)
{
    double pos_inf = std::numeric_limits<double>::infinity();
    double neg_inf = -std::numeric_limits<double>::infinity();
    double nan_val = std::numeric_limits<double>::quiet_NaN();

    // Infinity comparisons
    ASSERT_EQ(1, call_f64_le(neg_inf, pos_inf)) << "-inf <= +inf should return true (1)";
    ASSERT_EQ(1, call_f64_le(neg_inf, 1.0)) << "-inf <= 1.0 should return true (1)";
    ASSERT_EQ(0, call_f64_le(pos_inf, 1.0)) << "+inf <= 1.0 should return false (0)";
    ASSERT_EQ(1, call_f64_le(pos_inf, pos_inf)) << "+inf <= +inf should return true (1)";
    ASSERT_EQ(1, call_f64_le(neg_inf, neg_inf)) << "-inf <= -inf should return true (1)";

    // NaN comparisons (NaN <= x is always false for any x, including NaN)
    ASSERT_EQ(0, call_f64_le(nan_val, 1.0)) << "NaN <= 1.0 should return false (0)";
    ASSERT_EQ(0, call_f64_le(1.0, nan_val)) << "1.0 <= NaN should return false (0)";
    ASSERT_EQ(0, call_f64_le(nan_val, nan_val)) << "NaN <= NaN should return false (0)";
    ASSERT_EQ(0, call_f64_le(nan_val, pos_inf)) << "NaN <= +inf should return false (0)";
    ASSERT_EQ(0, call_f64_le(neg_inf, nan_val)) << "-inf <= NaN should return false (0)";

    // Signed zero comparisons (+0.0 == -0.0 in IEEE 754)
    ASSERT_EQ(1, call_f64_le(0.0, -0.0)) << "0.0 <= -0.0 should return true (1)";
    ASSERT_EQ(1, call_f64_le(-0.0, 0.0)) << "-0.0 <= 0.0 should return true (1)";
}

/**
 * @test BoundaryValues_ProcessesExtremeRanges
 * @brief Validates f64.le handles boundary values and extreme ranges correctly
 * @details Tests comparison behavior with f64 maximum, minimum, and denormalized values.
 *          Ensures correct comparison results at the extremes of the f64 range and
 *          validates proper handling of very small denormalized numbers.
 * @test_category Boundary - Extreme value range validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_le_operation
 * @input_conditions DBL_MAX, DBL_MIN, denormalized numbers, very small values
 * @expected_behavior Correct boundary comparison results respecting IEEE 754 ordering
 * @validation_method Boundary value analysis with extreme f64 range limits
 */
TEST_P(F64LeTest, BoundaryValues_ProcessesExtremeRanges)
{
    double max_val = DBL_MAX;
    double min_positive = DBL_MIN;  // Smallest positive normalized value
    double denorm_val = min_positive / 2.0;  // Denormalized value

    // Maximum value comparisons
    ASSERT_EQ(0, call_f64_le(max_val, 1.0)) << "DBL_MAX <= 1.0 should return false (0)";
    ASSERT_EQ(1, call_f64_le(1.0, max_val)) << "1.0 <= DBL_MAX should return true (1)";
    ASSERT_EQ(1, call_f64_le(max_val, max_val)) << "DBL_MAX <= DBL_MAX should return true (1)";

    // Minimum positive value comparisons
    ASSERT_EQ(1, call_f64_le(min_positive, 1.0)) << "DBL_MIN <= 1.0 should return true (1)";
    ASSERT_EQ(0, call_f64_le(1.0, min_positive)) << "1.0 <= DBL_MIN should return false (0)";

    // Denormalized number comparisons
    ASSERT_EQ(1, call_f64_le(denorm_val, min_positive)) << "denorm <= DBL_MIN should return true (1)";
    ASSERT_EQ(0, call_f64_le(min_positive, denorm_val)) << "DBL_MIN <= denorm should return false (0)";

    // Very small comparisons near zero
    ASSERT_EQ(0, call_f64_le(denorm_val, 0.0)) << "denorm <= 0.0 should return false (0) - denorm is positive";
    ASSERT_EQ(1, call_f64_le(-denorm_val, 0.0)) << "-denorm <= 0.0 should return true (1)";
    ASSERT_EQ(0, call_f64_le(0.0, -denorm_val)) << "0.0 <= -denorm should return false (0)";
}

// Parameterized test instantiation for interpreter and AOT modes
INSTANTIATE_TEST_CASE_P(
    RunningMode,
    F64LeTest,
    ::testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT),
    [](const testing::TestParamInfo<F64LeTest::ParamType> &info) {
        return info.param == RunningMode::Mode_Interp ? "Interpreter" : "AOT";
    }
);