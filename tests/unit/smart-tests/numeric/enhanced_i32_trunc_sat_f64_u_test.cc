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
 * @brief Test fixture for i32.trunc_sat_f64_u opcode validation
 * @details Provides comprehensive testing infrastructure for saturating truncation
 *          from f64 to unsigned i32, including module loading, execution context
 *          management, and cross-execution mode validation.
 */
class I32TruncSatF64UTest : public testing::TestWithParam<RunningMode> {
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

        // Get execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and release resources
     * @details Destroys execution environment, deinstantiates module,
     *          unloads module, and destroys WAMR runtime
     */
    void TearDown() override {
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

        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM module for testing based on execution mode
     * @param mode Execution mode (interpreter or AOT)
     * @details Loads appropriate test module containing i32.trunc_sat_f64_u operations
     */
    void load_module(RunningMode mode) {
        const char* wasm_file = "wasm-apps/i32_trunc_sat_f64_u_test.wasm";

        module_buf = (unsigned char*)bh_read_file_to_buffer(
            wasm_file, &module_buf_size);
        ASSERT_NE(module_buf, nullptr)
            << "Failed to read module file: " << wasm_file;

        module = wasm_runtime_load(
            module_buf, module_buf_size, error_buf, sizeof(error_buf));
    }

    /**
     * @brief Execute i32.trunc_sat_f64_u function with given input
     * @param input f64 input value for truncation
     * @return uint32_t result of saturating truncation to unsigned i32
     * @details Calls WASM function and handles execution errors
     */
    uint32_t call_trunc_sat_f64_u(double input) {
        wasm_val_t args[1], results[1];
        args[0].kind = WASM_F64;
        args[0].of.f64 = input;

        results[0].kind = WASM_I32;

        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "trunc_sat_f64_u");
        EXPECT_NE(func, nullptr) << "Failed to lookup trunc_sat_f64_u function";

        bool success = wasm_runtime_call_wasm_a(
            exec_env, func, 1, results, 1, args);
        EXPECT_TRUE(success) << "Function call failed: "
                           << wasm_runtime_get_exception(module_inst);

        return results[0].of.i32;
    }

    // Test infrastructure members
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    unsigned char* module_buf = nullptr;
    unsigned int module_buf_size = 0;
    char error_buf[128] = {0};
    static char global_heap_buf[512 * 1024];
};

// Static member definition
char I32TruncSatF64UTest::global_heap_buf[512 * 1024];

/**
 * @test BasicConversion_ReturnsCorrectResults
 * @brief Validates i32.trunc_sat_f64_u produces correct results for normal f64 inputs
 * @details Tests fundamental saturating truncation operation with typical f64 values
 *          within valid unsigned 32-bit range, including zero handling and decimal truncation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:DEF_OP_TRUNC_SAT_F64
 * @input_conditions Normal f64 values: positive numbers, zero variants, decimals
 * @expected_behavior Returns truncated unsigned integer values, zero for negative inputs
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32TruncSatF64UTest, BasicConversion_ReturnsCorrectResults) {
    // Normal positive conversions
    ASSERT_EQ(100u, call_trunc_sat_f64_u(100.5))
        << "Failed to truncate 100.5 to 100";
    ASSERT_EQ(1000u, call_trunc_sat_f64_u(1000.7))
        << "Failed to truncate 1000.7 to 1000";
    ASSERT_EQ(2147483647u, call_trunc_sat_f64_u(2147483647.0))
        << "Failed to convert 2147483647.0 to unsigned";

    // Zero variants
    ASSERT_EQ(0u, call_trunc_sat_f64_u(0.0))
        << "Failed to convert positive zero";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(-0.0))
        << "Failed to convert negative zero";

    // Small decimal truncation
    ASSERT_EQ(0u, call_trunc_sat_f64_u(0.1))
        << "Failed to truncate 0.1 to 0";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(0.9))
        << "Failed to truncate 0.9 to 0";
    ASSERT_EQ(1u, call_trunc_sat_f64_u(1.9))
        << "Failed to truncate 1.9 to 1";

    // Large valid values
    ASSERT_EQ(4294967294u, call_trunc_sat_f64_u(4294967294.0))
        << "Failed to convert large valid f64 value";
    ASSERT_EQ(4294967295u, call_trunc_sat_f64_u(4294967295.0))
        << "Failed to convert maximum valid f64 value";
}

/**
 * @test SaturationBehavior_ClampsToValidRange
 * @brief Validates saturation behavior for f64 values outside unsigned 32-bit range
 * @details Tests overflow and underflow scenarios to ensure values are properly
 *          clamped to [0, UINT32_MAX] without causing traps or exceptions.
 * @test_category Corner - Boundary conditions and saturation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f64_to_int
 * @input_conditions Values outside u32 range: negative numbers, overflow values
 * @expected_behavior Saturates to 0 for negatives, UINT32_MAX for overflow
 * @validation_method Verification of saturation limits and no exception throwing
 */
TEST_P(I32TruncSatF64UTest, SaturationBehavior_ClampsToValidRange) {
    // Negative value saturation to 0
    ASSERT_EQ(0u, call_trunc_sat_f64_u(-1.0))
        << "Failed to saturate -1.0 to 0";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(-100.5))
        << "Failed to saturate -100.5 to 0";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(-1000000.0))
        << "Failed to saturate large negative to 0";

    // Positive overflow saturation to UINT32_MAX
    ASSERT_EQ(UINT32_MAX, call_trunc_sat_f64_u(4294967296.0))
        << "Failed to saturate 4294967296.0 to UINT32_MAX";
    ASSERT_EQ(UINT32_MAX, call_trunc_sat_f64_u(5000000000.0))
        << "Failed to saturate 5e9 to UINT32_MAX";
    ASSERT_EQ(UINT32_MAX, call_trunc_sat_f64_u(1e20))
        << "Failed to saturate 1e20 to UINT32_MAX";

    // Boundary edge cases
    ASSERT_EQ(0u, call_trunc_sat_f64_u(-0.999999))
        << "Failed to saturate near-zero negative to 0";
    ASSERT_EQ(UINT32_MAX, call_trunc_sat_f64_u(4294967295.999999))
        << "Failed to saturate just above max to UINT32_MAX";

    // Just below/above saturation points
    ASSERT_EQ(UINT32_MAX, call_trunc_sat_f64_u(4294967295.1))
        << "Failed to saturate slightly above max";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(-0.1))
        << "Failed to saturate slightly below zero";
}

/**
 * @test SpecialFloatValues_HandleCorrectly
 * @brief Validates handling of special IEEE 754 f64 values
 * @details Tests NaN, infinity, and subnormal number handling according to
 *          WebAssembly specification for saturating truncation operations.
 * @test_category Edge - Special numeric values and IEEE 754 compliance
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_sat_float_to_int
 * @input_conditions NaN, +/-Infinity, subnormal numbers, very small values
 * @expected_behavior NaN→0, +Inf→UINT32_MAX, -Inf→0, subnormals→0
 * @validation_method IEEE 754 compliance verification and edge case handling
 */
TEST_P(I32TruncSatF64UTest, SpecialFloatValues_HandleCorrectly) {
    // NaN handling - should convert to 0
    ASSERT_EQ(0u, call_trunc_sat_f64_u(std::nan("")))
        << "Failed to convert NaN to 0";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(-std::nan("")))
        << "Failed to convert negative NaN to 0";

    // Infinity handling
    ASSERT_EQ(UINT32_MAX, call_trunc_sat_f64_u(std::numeric_limits<double>::infinity()))
        << "Failed to convert +Infinity to UINT32_MAX";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(-std::numeric_limits<double>::infinity()))
        << "Failed to convert -Infinity to 0";

    // Subnormal numbers (very small values)
    ASSERT_EQ(0u, call_trunc_sat_f64_u(1e-308))
        << "Failed to convert small subnormal to 0";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(std::numeric_limits<double>::denorm_min()))
        << "Failed to convert minimum denormal to 0";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(2.225e-308))
        << "Failed to convert subnormal 2.225e-308 to 0";

    // Very small positive numbers
    ASSERT_EQ(0u, call_trunc_sat_f64_u(1e-100))
        << "Failed to convert 1e-100 to 0";
    ASSERT_EQ(0u, call_trunc_sat_f64_u(std::numeric_limits<double>::min()))
        << "Failed to convert DBL_MIN to 0";
}

/**
 * @test HighPrecisionValues_TruncateCorrectly
 * @brief Validates truncation behavior for high-precision f64 values
 * @details Tests f64 values with precision beyond f32 representation and
 *          verifies correct truncation (not rounding) behavior for decimal parts.
 * @test_category Edge - High precision and truncation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:truncation_logic
 * @input_conditions High-precision f64, values with many decimal places
 * @expected_behavior Proper truncation of fractional parts, no rounding
 * @validation_method Precision preservation and truncation behavior verification
 */
TEST_P(I32TruncSatF64UTest, HighPrecisionValues_TruncateCorrectly) {
    // High precision f64 values beyond f32 range
    ASSERT_EQ(123u, call_trunc_sat_f64_u(123.5))
        << "Failed to truncate high-precision decimal";
    ASSERT_EQ(1000000u, call_trunc_sat_f64_u(1000000.123456789012345))
        << "Failed to truncate f64 with many decimal places";

    // Values near precision limits (but within u32 range)
    ASSERT_EQ(16777216u, call_trunc_sat_f64_u(16777216.0))
        << "Failed to handle 2^24 exactly";

    // Truncation verification (not rounding)
    ASSERT_EQ(42u, call_trunc_sat_f64_u(42.99999999999999))
        << "Incorrectly rounded instead of truncated";
    ASSERT_EQ(1234567u, call_trunc_sat_f64_u(1234567.89012345678))
        << "Failed to truncate complex decimal";

    // Large integers within f64 precision
    ASSERT_EQ(16777216u, call_trunc_sat_f64_u(16777216.0))
        << "Failed to convert 2^24 exactly";
    ASSERT_EQ(33554432u, call_trunc_sat_f64_u(33554432.0))
        << "Failed to convert 2^25 exactly";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionModeValidation,
    I32TruncSatF64UTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return (info.param == Mode_Interp) ? "Interpreter" : "AOT";
    }
);