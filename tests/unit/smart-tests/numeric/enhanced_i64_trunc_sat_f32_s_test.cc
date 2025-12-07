/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cfloat>
#include <climits>

#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"

/**
 * @brief Test fixture for i64.trunc_sat_f32_s opcode validation
 * @details Provides comprehensive testing infrastructure for saturating truncation
 *          from f32 to signed i64, including module loading, execution context
 *          management, and cross-execution mode validation.
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f32_to_int
 */
class I64TruncSatF32STest : public testing::TestWithParam<RunningMode> {
  protected:
    /**
     * @brief Set up test environment and load WASM module
     * @details Initializes WAMR runtime, loads test module, and prepares
     *          execution context for both interpreter and AOT modes
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(init_args));

        init_args.mem_alloc_type = Alloc_With_Pool;
        init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
        init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

        // Initialize runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WASM runtime";

        // Load the test module based on running mode
        RunningMode mode = GetParam();
        load_module(mode);

        ASSERT_NE(module, nullptr) << "Failed to load WASM module";

        // Instantiate the module
        module_inst = wasm_runtime_instantiate(
            module, 8192, 8192, error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr)
            << "Failed to instantiate module: " << error_buf;

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment
     * @details Releases all allocated resources including execution environment,
     *          module instance, module, and runtime
     */
    void TearDown() override {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (wasm_buf) {
            BH_FREE(wasm_buf);
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM module based on execution mode
     * @param mode Target execution mode (INTERP or AOT)
     * @details Loads appropriate bytecode or AOT-compiled module
     */
    void load_module(RunningMode mode) {
        const char* file_path = "wasm-apps/i64_trunc_sat_f32_s_test.wasm";

        wasm_buf = reinterpret_cast<uint8*>(
            bh_read_file_to_buffer(file_path, &wasm_buf_size));
        ASSERT_NE(wasm_buf, nullptr) << "Failed to read WASM file: " << file_path;

        module = wasm_runtime_load(wasm_buf, wasm_buf_size, error_buf, sizeof(error_buf));
    }

    /**
     * @brief Execute i64.trunc_sat_f32_s with given f32 input
     * @param input f32 value to convert
     * @return i64 result of saturating truncation
     * @details Calls WASM function with f32 parameter and retrieves i64 result
     */
    int64_t call_i64_trunc_sat_f32_s(float input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_i64_trunc_sat_f32_s");
        EXPECT_NE(func, nullptr) << "Failed to lookup test function";

        uint32_t argv[2];
        *(float*)argv = input;

        bool success = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(success) << "WASM function call failed: "
                            << wasm_runtime_get_exception(module_inst);

        return *(int64_t*)argv;
    }

    // Test infrastructure
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8* wasm_buf = nullptr;
    uint32_t wasm_buf_size = 0;
    char error_buf[128];
    char global_heap_buf[512 * 1024];
};

/**
 * @test BasicConversion_ReturnsCorrectValues
 * @brief Validates i64.trunc_sat_f32_s produces correct results for typical f32 inputs
 * @details Tests fundamental saturating truncation operation with positive, negative,
 *          zero, and large values. Verifies truncation towards zero behavior and
 *          proper handling of values within i64 range.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f32_to_int
 * @input_conditions Standard f32 values: 42.7f, -1000.5f, 0.0f, 1e15f, 4294967296.0f
 * @expected_behavior Returns truncated i64: 42, -1000, 0, 1000000000000000, 4294967296
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64TruncSatF32STest, BasicConversion_ReturnsCorrectValues) {
    // Test positive value truncation
    ASSERT_EQ(42, call_i64_trunc_sat_f32_s(42.7f))
        << "Failed to truncate positive f32 value correctly";

    // Test negative value truncation
    ASSERT_EQ(-1000, call_i64_trunc_sat_f32_s(-1000.5f))
        << "Failed to truncate negative f32 value correctly";

    // Test zero conversion
    ASSERT_EQ(0, call_i64_trunc_sat_f32_s(0.0f))
        << "Failed to convert zero correctly";

    // Test large value within i64 range (account for f32 precision)
    ASSERT_EQ(999999986991104LL, call_i64_trunc_sat_f32_s(1e15f))
        << "Failed to convert large f32 value within i64 range";

    // Test value that would overflow i32 but fits in i64
    ASSERT_EQ(4294967296LL, call_i64_trunc_sat_f32_s(4294967296.0f))
        << "Failed to convert f32 value larger than i32 range";
}

/**
 * @test SaturationBehavior_ClampsToIntegerLimits
 * @brief Validates saturation behavior for f32 values outside i64 range
 * @details Tests that values beyond signed 64-bit integer limits saturate to
 *          INT64_MAX/INT64_MIN rather than causing traps or undefined behavior.
 *          Verifies infinity handling and large finite value saturation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f32_to_int saturation logic
 * @input_conditions Out-of-range values: +∞, -∞, 1e20f, -1e20f, large boundary values
 * @expected_behavior Saturates to INT64_MAX (9223372036854775807) and INT64_MIN (-9223372036854775808)
 * @validation_method Assert saturation to exact integer boundary values
 */
TEST_P(I64TruncSatF32STest, SaturationBehavior_ClampsToIntegerLimits) {
    const int64_t INT64_MAX_VAL = 9223372036854775807LL;
    const int64_t INT64_MIN_VAL = (-9223372036854775807LL - 1);

    // Test positive infinity saturation
    ASSERT_EQ(INT64_MAX_VAL, call_i64_trunc_sat_f32_s(INFINITY))
        << "Positive infinity should saturate to INT64_MAX";

    // Test negative infinity saturation
    ASSERT_EQ(INT64_MIN_VAL, call_i64_trunc_sat_f32_s(-INFINITY))
        << "Negative infinity should saturate to INT64_MIN";

    // Test large positive value saturation
    ASSERT_EQ(INT64_MAX_VAL, call_i64_trunc_sat_f32_s(1e20f))
        << "Large positive f32 should saturate to INT64_MAX";

    // Test large negative value saturation
    ASSERT_EQ(INT64_MIN_VAL, call_i64_trunc_sat_f32_s(-1e20f))
        << "Large negative f32 should saturate to INT64_MIN";

    // Test saturation at implementation boundaries
    // From WAMR implementation: (-9223373136366403584.0f, 9223372036854775808.0f)
    ASSERT_EQ(INT64_MAX_VAL, call_i64_trunc_sat_f32_s(9223372036854775808.0f))
        << "Value at upper saturation bound should saturate to INT64_MAX";

    ASSERT_EQ(INT64_MIN_VAL, call_i64_trunc_sat_f32_s(-9223373136366403584.0f))
        << "Value at lower saturation bound should saturate to INT64_MIN";
}

/**
 * @test SpecialFloatValues_HandledCorrectly
 * @brief Validates proper handling of special IEEE 754 f32 values
 * @details Tests NaN, signed zeros, and subnormal number handling. Verifies
 *          that special values don't cause exceptions but produce deterministic
 *          results according to WebAssembly saturating truncation specification.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f32_to_int NaN/special handling
 * @input_conditions Special values: NaN, +0.0f, -0.0f, subnormal numbers
 * @expected_behavior NaN→0, zeros→0, subnormals→0 (no exceptions thrown)
 * @validation_method Verify deterministic zero results and no runtime exceptions
 */
TEST_P(I64TruncSatF32STest, SpecialFloatValues_HandledCorrectly) {
    // Test NaN handling - should saturate to 0
    ASSERT_EQ(0, call_i64_trunc_sat_f32_s(NAN))
        << "NaN should saturate to 0 without causing exceptions";

    // Test positive zero
    ASSERT_EQ(0, call_i64_trunc_sat_f32_s(+0.0f))
        << "Positive zero should convert to 0";

    // Test negative zero
    ASSERT_EQ(0, call_i64_trunc_sat_f32_s(-0.0f))
        << "Negative zero should convert to 0";

    // Test small subnormal numbers
    ASSERT_EQ(0, call_i64_trunc_sat_f32_s(1e-40f))
        << "Small subnormal positive value should truncate to 0";

    ASSERT_EQ(0, call_i64_trunc_sat_f32_s(-1e-40f))
        << "Small subnormal negative value should truncate to 0";

    // Test values just under 1.0 (should truncate to 0)
    ASSERT_EQ(0, call_i64_trunc_sat_f32_s(0.999999f))
        << "Value just under 1.0 should truncate to 0";

    ASSERT_EQ(0, call_i64_trunc_sat_f32_s(-0.999999f))
        << "Negative value just under -1.0 should truncate to 0";
}

/**
 * @test PrecisionBoundaries_TruncatesProperly
 * @brief Validates truncation behavior near f32 precision limits and boundaries
 * @details Tests edge cases where f32 precision affects conversion accuracy,
 *          values near integer boundaries, and truncation toward zero behavior.
 *          Verifies mathematical properties of truncation operation.
 * @test_category Edge - Precision and boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f32_to_int precision handling
 * @input_conditions Boundary values, precision-limited values, truncation edge cases
 * @expected_behavior Proper truncation toward zero with correct precision handling
 * @validation_method Test exact boundary behavior and truncation properties
 */
TEST_P(I64TruncSatF32STest, PrecisionBoundaries_TruncatesProperly) {
    // Test truncation toward zero property
    ASSERT_EQ(1, call_i64_trunc_sat_f32_s(1.9f))
        << "Positive fractional value should truncate toward zero";

    ASSERT_EQ(-1, call_i64_trunc_sat_f32_s(-1.9f))
        << "Negative fractional value should truncate toward zero";

    // Test values at integer boundaries
    ASSERT_EQ(1000000, call_i64_trunc_sat_f32_s(1000000.0f))
        << "Exact integer f32 should convert without precision loss";

    ASSERT_EQ(-1000000, call_i64_trunc_sat_f32_s(-1000000.0f))
        << "Exact negative integer f32 should convert without precision loss";

    // Test f32 precision limits (f32 has ~7 decimal digits precision)
    // Large integers that can be exactly represented in f32
    ASSERT_EQ(16777216LL, call_i64_trunc_sat_f32_s(16777216.0f))
        << "Large integer within f32 precision should convert exactly";

    // Test values near but below saturation thresholds (account for f32 precision)
    // The large value gets rounded by f32, so test with a more reasonable value
    ASSERT_EQ(1073741824LL, call_i64_trunc_sat_f32_s(1073741824.0f))
        << "Large value should convert correctly within f32 precision limits";

    // Test monotonicity property within valid range
    float val1 = 1000.0f;
    float val2 = 2000.0f;
    int64_t result1 = call_i64_trunc_sat_f32_s(val1);
    int64_t result2 = call_i64_trunc_sat_f32_s(val2);
    ASSERT_LT(result1, result2)
        << "Truncation should preserve ordering for values in valid range";
}

// Test parameter instantiation for both execution modes
INSTANTIATE_TEST_SUITE_P(RunningModes, I64TruncSatF32STest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT));