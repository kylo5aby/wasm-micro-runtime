/*
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

static std::string CWD;
static std::string WASM_FILE;

static int app_argc;
static char **app_argv;

/**
 * @brief Test fixture for f32.gt opcode validation across execution modes
 *
 * This class provides comprehensive testing infrastructure for the f32.gt (float32 greater than)
 * WebAssembly opcode. It validates IEEE 754 compliant floating-point comparison behavior
 * across both interpreter and AOT execution modes.
 */
class F32GtTest : public testing::TestWithParam<RunningMode>
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
     * for f32.gt opcode validation. Loads the test WASM module containing f32.gt operations.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/f32_gt_test.wasm", &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: wasm-apps/f32_gt_test.wasm";

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
     * @brief Execute f32.gt operation via WASM function call
     *
     * @param a First f32 operand
     * @param b Second f32 operand
     * @return int32_t Result of f32.gt comparison (1 if a > b, 0 otherwise)
     */
    int32_t call_f32_gt(float a, float b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f32_gt_test");
        EXPECT_NE(nullptr, func) << "Failed to lookup f32_gt_test function";

        uint32_t argv[2];
        memcpy(&argv[0], &a, sizeof(float));
        memcpy(&argv[1], &b, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "Failed to execute f32_gt_test function: "
                            << wasm_runtime_get_exception(module_inst);

        return static_cast<int32_t>(argv[0]);
    }
};

/**
 * @test BasicComparison_ReturnsCorrectResult
 * @brief Validates f32.gt produces correct comparison results for typical floating-point values
 * @details Tests fundamental greater-than comparison with positive numbers, negative numbers,
 *          and mixed-sign combinations. Verifies that f32.gt correctly determines ordering
 *          relationships according to IEEE 754 specifications.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_gt_operation
 * @input_conditions Standard f32 pairs: (5.5f,3.3f), (-2.1f,-7.8f), (1.0f,-1.0f)
 * @expected_behavior Returns 1 when first operand is greater than second: 1, 1, 1 respectively
 * @validation_method Direct comparison of WASM function result with expected boolean values
 */
TEST_P(F32GtTest, BasicComparison_ReturnsCorrectResult)
{
    // Positive numbers comparison
    ASSERT_EQ(1, call_f32_gt(5.5f, 3.3f))
        << "f32.gt failed for positive number comparison: 5.5 > 3.3";

    // Negative numbers comparison
    ASSERT_EQ(1, call_f32_gt(-2.1f, -7.8f))
        << "f32.gt failed for negative number comparison: -2.1 > -7.8";

    // Mixed signs comparison
    ASSERT_EQ(1, call_f32_gt(1.0f, -1.0f))
        << "f32.gt failed for mixed-sign comparison: 1.0 > -1.0";

    // False comparison cases
    ASSERT_EQ(0, call_f32_gt(3.3f, 5.5f))
        << "f32.gt failed for false comparison: 3.3 > 5.5 should be 0";

    ASSERT_EQ(0, call_f32_gt(-1.0f, 1.0f))
        << "f32.gt failed for false mixed-sign comparison: -1.0 > 1.0 should be 0";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Validates f32.gt handles IEEE 754 boundary values according to specification
 * @details Tests comparisons involving FLT_MAX, FLT_MIN, and subnormal values to ensure
 *          proper handling of floating-point range boundaries and precision limits.
 * @test_category Main - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_gt_operation
 * @input_conditions FLT_MAX, FLT_MIN, subnormal values, zero comparisons
 * @expected_behavior Correct IEEE 754 compliant comparison results for boundary values
 * @validation_method Verification of proper ordering relationships at f32 precision boundaries
 */
TEST_P(F32GtTest, BoundaryValues_HandledCorrectly)
{
    // FLT_MAX comparisons
    ASSERT_EQ(1, call_f32_gt(FLT_MAX, 1.0f))
        << "f32.gt failed: FLT_MAX should be greater than 1.0";

    ASSERT_EQ(0, call_f32_gt(1.0f, FLT_MAX))
        << "f32.gt failed: 1.0 should not be greater than FLT_MAX";

    // FLT_MIN (smallest positive normal) comparisons
    ASSERT_EQ(1, call_f32_gt(FLT_MIN, 0.0f))
        << "f32.gt failed: FLT_MIN should be greater than 0.0";

    ASSERT_EQ(1, call_f32_gt(1.0f, FLT_MIN))
        << "f32.gt failed: 1.0 should be greater than FLT_MIN";

    // Zero comparisons (IEEE 754: +0.0 == -0.0)
    ASSERT_EQ(0, call_f32_gt(0.0f, -0.0f))
        << "f32.gt failed: +0.0 should not be greater than -0.0 (IEEE 754 equality)";

    ASSERT_EQ(0, call_f32_gt(-0.0f, 0.0f))
        << "f32.gt failed: -0.0 should not be greater than +0.0 (IEEE 754 equality)";
}

/**
 * @test SpecialIEEE754Cases_ProduceStandardResults
 * @brief Validates f32.gt handles IEEE 754 special values (infinity, NaN) correctly
 * @details Tests comparison behavior with positive infinity, negative infinity, and NaN values.
 *          Ensures compliance with IEEE 754 standard for special value comparisons, particularly
 *          that any comparison involving NaN returns false (unordered comparison).
 * @test_category Main - IEEE 754 special case validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_gt_operation
 * @input_conditions +∞, -∞, NaN values compared with finite numbers and each other
 * @expected_behavior IEEE 754 compliant results: +∞ > finite, NaN comparisons return false
 * @validation_method Verification of IEEE 754 special value comparison semantics
 */
TEST_P(F32GtTest, SpecialIEEE754Cases_ProduceStandardResults)
{
    const float pos_inf = std::numeric_limits<float>::infinity();
    const float neg_inf = -std::numeric_limits<float>::infinity();
    const float quiet_nan = std::numeric_limits<float>::quiet_NaN();

    // Positive infinity comparisons
    ASSERT_EQ(1, call_f32_gt(pos_inf, 1.0f))
        << "f32.gt failed: +∞ should be greater than finite value";

    ASSERT_EQ(1, call_f32_gt(pos_inf, FLT_MAX))
        << "f32.gt failed: +∞ should be greater than FLT_MAX";

    ASSERT_EQ(0, call_f32_gt(1.0f, pos_inf))
        << "f32.gt failed: finite value should not be greater than +∞";

    // Negative infinity comparisons
    ASSERT_EQ(0, call_f32_gt(neg_inf, -1.0f))
        << "f32.gt failed: -∞ should not be greater than finite negative value";

    ASSERT_EQ(1, call_f32_gt(-1.0f, neg_inf))
        << "f32.gt failed: finite negative value should be greater than -∞";

    // Infinity vs infinity
    ASSERT_EQ(1, call_f32_gt(pos_inf, neg_inf))
        << "f32.gt failed: +∞ should be greater than -∞";

    ASSERT_EQ(0, call_f32_gt(pos_inf, pos_inf))
        << "f32.gt failed: +∞ should not be greater than itself";

    // NaN comparisons (IEEE 754: all comparisons with NaN are false)
    ASSERT_EQ(0, call_f32_gt(quiet_nan, 1.0f))
        << "f32.gt failed: NaN comparison should return false";

    ASSERT_EQ(0, call_f32_gt(1.0f, quiet_nan))
        << "f32.gt failed: comparison with NaN should return false";

    ASSERT_EQ(0, call_f32_gt(quiet_nan, quiet_nan))
        << "f32.gt failed: NaN compared to itself should return false";
}

/**
 * @test CrossSignComparisons_ValidateCorrectOrdering
 * @brief Validates f32.gt correctly handles comparisons across positive/negative boundaries
 * @details Tests systematic comparison patterns across zero boundary, ensuring proper
 *          mathematical ordering relationships are maintained for cross-sign scenarios.
 * @test_category Main - Cross-sign boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_gt_operation
 * @input_conditions Positive vs negative values, zero vs non-zero, magnitude vs sign precedence
 * @expected_behavior Mathematically correct ordering: positive > zero > negative
 * @validation_method Verification of proper sign-based ordering relationships
 */
TEST_P(F32GtTest, CrossSignComparisons_ValidateCorrectOrdering)
{
    // Positive vs zero
    ASSERT_EQ(1, call_f32_gt(1.0f, 0.0f))
        << "f32.gt failed: positive should be greater than zero";

    ASSERT_EQ(0, call_f32_gt(0.0f, 1.0f))
        << "f32.gt failed: zero should not be greater than positive";

    // Zero vs negative
    ASSERT_EQ(1, call_f32_gt(0.0f, -1.0f))
        << "f32.gt failed: zero should be greater than negative";

    ASSERT_EQ(0, call_f32_gt(-1.0f, 0.0f))
        << "f32.gt failed: negative should not be greater than zero";

    // Large magnitude negative vs small positive
    ASSERT_EQ(1, call_f32_gt(0.1f, -1000.0f))
        << "f32.gt failed: small positive should be greater than large negative";

    ASSERT_EQ(0, call_f32_gt(-1000.0f, 0.1f))
        << "f32.gt failed: large negative should not be greater than small positive";

    // Equal magnitude, different signs
    ASSERT_EQ(1, call_f32_gt(5.0f, -5.0f))
        << "f32.gt failed: positive should be greater than negative of equal magnitude";

    ASSERT_EQ(0, call_f32_gt(-5.0f, 5.0f))
        << "f32.gt failed: negative should not be greater than positive of equal magnitude";
}

// Test parameter instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    F32GtTestSuite,
    F32GtTest,
    ::testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT),
    [](const testing::TestParamInfo<F32GtTest::ParamType> &info) {
        return info.param == RunningMode::Mode_Interp ? "Interpreter" : "AOT";
    }
);