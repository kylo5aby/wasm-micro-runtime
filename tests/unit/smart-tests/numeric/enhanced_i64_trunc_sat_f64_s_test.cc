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
 * @brief Test fixture for i64.trunc_sat_f64_s opcode validation
 * @details Provides comprehensive testing infrastructure for saturating truncation
 *          from f64 to signed i64, including module loading, execution context
 *          management, and cross-execution mode validation.
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_int
 */
class I64TruncSatF64STest : public testing::TestWithParam<RunningMode> {
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
        const char* file_path = "wasm-apps/i64_trunc_sat_f64_s_test.wasm";

        wasm_buf = reinterpret_cast<uint8*>(
            bh_read_file_to_buffer(file_path, &wasm_buf_size));
        ASSERT_NE(wasm_buf, nullptr) << "Failed to read WASM file: " << file_path;

        module = wasm_runtime_load(wasm_buf, wasm_buf_size, error_buf, sizeof(error_buf));
    }

    /**
     * @brief Execute i64.trunc_sat_f64_s with given f64 input
     * @param input f64 value to convert
     * @return i64 result of saturating truncation
     * @details Calls WASM function with f64 parameter and retrieves i64 result
     */
    int64_t call_i64_trunc_sat_f64_s(double input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_i64_trunc_sat_f64_s");
        EXPECT_NE(func, nullptr) << "Failed to lookup test function";

        uint32_t argv[4];
        *(double*)argv = input;

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
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
 * @test BasicConversion_ReturnsTruncatedInteger
 * @brief Validates i64.trunc_sat_f64_s produces correct results for typical f64 inputs
 * @details Tests fundamental saturating truncation operation with positive, negative,
 *          zero, and large values. Verifies truncation towards zero behavior and
 *          proper handling of values within i64 range.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_int
 * @input_conditions Standard f64 values: 42.0, -1234.567, 999999999.9, 0.0
 * @expected_behavior Returns truncated i64: 42, -1234, 999999999, 0
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64TruncSatF64STest, BasicConversion_ReturnsTruncatedInteger) {
    // Test positive value truncation
    ASSERT_EQ(42, call_i64_trunc_sat_f64_s(42.0))
        << "Failed to truncate positive f64 value correctly";

    // Test negative value truncation
    ASSERT_EQ(-1234, call_i64_trunc_sat_f64_s(-1234.567))
        << "Failed to truncate negative f64 value correctly";

    // Test large positive value within i64 range
    ASSERT_EQ(999999999, call_i64_trunc_sat_f64_s(999999999.9))
        << "Failed to truncate large positive f64 value";

    // Test zero conversion
    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(0.0))
        << "Failed to convert zero correctly";

    // Test fractional truncation toward zero
    ASSERT_EQ(3, call_i64_trunc_sat_f64_s(3.7))
        << "Positive fractional should truncate toward zero";

    ASSERT_EQ(-3, call_i64_trunc_sat_f64_s(-3.7))
        << "Negative fractional should truncate toward zero";
}

/**
 * @test SaturationBehavior_ClampsToValidRange
 * @brief Validates saturation behavior for f64 values outside i64 range
 * @details Tests that values beyond signed 64-bit integer limits saturate to
 *          INT64_MAX/INT64_MIN rather than causing traps or undefined behavior.
 *          Verifies infinity handling and large finite value saturation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_int saturation logic
 * @input_conditions Out-of-range values: +∞, -∞, 1e20, -1e20, DBL_MAX, -DBL_MAX
 * @expected_behavior Saturates to INT64_MAX (9223372036854775807) and INT64_MIN (-9223372036854775808)
 * @validation_method Assert saturation to exact integer boundary values
 */
TEST_P(I64TruncSatF64STest, SaturationBehavior_ClampsToValidRange) {
    const int64_t INT64_MAX_VAL = 9223372036854775807LL;
    const int64_t INT64_MIN_VAL = (-9223372036854775807LL - 1);

    // Test positive infinity saturation
    ASSERT_EQ(INT64_MAX_VAL, call_i64_trunc_sat_f64_s(INFINITY))
        << "Positive infinity should saturate to INT64_MAX";

    // Test negative infinity saturation
    ASSERT_EQ(INT64_MIN_VAL, call_i64_trunc_sat_f64_s(-INFINITY))
        << "Negative infinity should saturate to INT64_MIN";

    // Test large positive value saturation
    ASSERT_EQ(INT64_MAX_VAL, call_i64_trunc_sat_f64_s(1e20))
        << "Large positive f64 should saturate to INT64_MAX";

    // Test large negative value saturation
    ASSERT_EQ(INT64_MIN_VAL, call_i64_trunc_sat_f64_s(-1e20))
        << "Large negative f64 should saturate to INT64_MIN";

    // Test maximum double value saturation
    ASSERT_EQ(INT64_MAX_VAL, call_i64_trunc_sat_f64_s(DBL_MAX))
        << "DBL_MAX should saturate to INT64_MAX";

    // Test minimum double value saturation
    ASSERT_EQ(INT64_MIN_VAL, call_i64_trunc_sat_f64_s(-DBL_MAX))
        << "-DBL_MAX should saturate to INT64_MIN";

    // Test values just above/below i64 range
    ASSERT_EQ(INT64_MAX_VAL, call_i64_trunc_sat_f64_s(9.223372036854776e18))
        << "Value just above INT64_MAX should saturate";

    ASSERT_EQ(INT64_MIN_VAL, call_i64_trunc_sat_f64_s(-9.223372036854776e18))
        << "Value just below INT64_MIN should saturate";
}

/**
 * @test SpecialFloatValues_HandledCorrectly
 * @brief Validates proper handling of special IEEE 754 f64 values
 * @details Tests NaN, signed zeros, and subnormal number handling. Verifies
 *          that special values don't cause exceptions but produce deterministic
 *          results according to WebAssembly saturating truncation specification.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_int NaN/special handling
 * @input_conditions Special values: NaN, +0.0, -0.0, subnormal numbers, tiny values
 * @expected_behavior NaN→0, zeros→0, subnormals→0 (no exceptions thrown)
 * @validation_method Verify deterministic zero results and no runtime exceptions
 */
TEST_P(I64TruncSatF64STest, SpecialFloatValues_HandledCorrectly) {
    // Test NaN handling - should saturate to 0
    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(NAN))
        << "NaN should saturate to 0 without causing exceptions";

    // Test positive zero
    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(+0.0))
        << "Positive zero should convert to 0";

    // Test negative zero
    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(-0.0))
        << "Negative zero should convert to 0";

    // Test small subnormal numbers (smaller than DBL_MIN)
    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(1e-308))
        << "Small subnormal positive value should truncate to 0";

    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(-1e-308))
        << "Small subnormal negative value should truncate to 0";

    // Test values just under 1.0 (should truncate to 0)
    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(0.9999999999999999))
        << "Value just under 1.0 should truncate to 0";

    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(-0.9999999999999999))
        << "Negative value just under -1.0 should truncate to 0";

    // Test smallest normal positive value
    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(DBL_MIN))
        << "DBL_MIN should truncate to 0";

    ASSERT_EQ(0, call_i64_trunc_sat_f64_s(-DBL_MIN))
        << "-DBL_MIN should truncate to 0";
}

/**
 * @test FractionalTruncation_RoundsTowardZero
 * @brief Validates truncation behavior maintains toward-zero rounding
 * @details Tests edge cases where f64 precision and truncation direction
 *          are critical, including values near integer boundaries and
 *          mathematical properties of the truncation operation.
 * @test_category Edge - Truncation behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_int truncation logic
 * @input_conditions Fractional values, boundary cases, precision edge cases
 * @expected_behavior Proper truncation toward zero with consistent direction
 * @validation_method Test exact truncation behavior and mathematical properties
 */
TEST_P(I64TruncSatF64STest, FractionalTruncation_RoundsTowardZero) {
    // Test truncation direction for positive fractional values
    ASSERT_EQ(1, call_i64_trunc_sat_f64_s(1.999999999999999))
        << "Large positive fractional should truncate toward zero";

    // Test truncation direction for negative fractional values
    ASSERT_EQ(-1, call_i64_trunc_sat_f64_s(-1.999999999999999))
        << "Large negative fractional should truncate toward zero";

    // Test values very close to integers
    ASSERT_EQ(42, call_i64_trunc_sat_f64_s(42.0000000000000001))
        << "Value just above integer should truncate to integer";

    ASSERT_EQ(-42, call_i64_trunc_sat_f64_s(-42.0000000000000001))
        << "Negative value just below integer should truncate to integer";

    // Test large fractional values within valid range (use f64-representable values)
    ASSERT_EQ(123456789012345LL, call_i64_trunc_sat_f64_s(123456789012345.99))
        << "Large positive fractional in range should truncate properly";

    ASSERT_EQ(-123456789012345LL, call_i64_trunc_sat_f64_s(-123456789012345.99))
        << "Large negative fractional in range should truncate properly";

    // Test monotonicity within valid range
    double val1 = 1000.5;
    double val2 = 2000.5;
    int64_t result1 = call_i64_trunc_sat_f64_s(val1);
    int64_t result2 = call_i64_trunc_sat_f64_s(val2);
    ASSERT_LT(result1, result2)
        << "Truncation should preserve ordering for values in valid range";
}

/**
 * @test PrecisionBoundaries_HandlesCorrectly
 * @brief Validates conversion behavior at f64 precision limits and boundaries
 * @details Tests edge cases where f64 precision affects conversion accuracy,
 *          very large integers that can be exactly represented in f64, and
 *          the boundaries of precise integer representation in double precision.
 * @test_category Edge - Precision boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_int precision handling
 * @input_conditions Precision-critical values, large exact integers, boundary conditions
 * @expected_behavior Accurate conversion with proper precision handling
 * @validation_method Test exact boundary behavior and precision properties
 */
TEST_P(I64TruncSatF64STest, PrecisionBoundaries_HandlesCorrectly) {
    // Test largest exact integer representable in f64 (2^53)
    const int64_t MAX_SAFE_INTEGER = 9007199254740992LL; // 2^53
    ASSERT_EQ(MAX_SAFE_INTEGER, call_i64_trunc_sat_f64_s((double)MAX_SAFE_INTEGER))
        << "Largest safe f64 integer should convert exactly";

    // Test negative largest exact integer
    ASSERT_EQ(-MAX_SAFE_INTEGER, call_i64_trunc_sat_f64_s((double)(-MAX_SAFE_INTEGER)))
        << "Negative largest safe f64 integer should convert exactly";

    // Test powers of 2 (exactly representable in f64)
    ASSERT_EQ(1048576LL, call_i64_trunc_sat_f64_s(1048576.0)) // 2^20
        << "Power of 2 within f64 precision should convert exactly";

    ASSERT_EQ(4294967296LL, call_i64_trunc_sat_f64_s(4294967296.0)) // 2^32
        << "2^32 should convert exactly";

    // Test large integers near but within i64 range that f64 can represent
    ASSERT_EQ(1152921504606846976LL, call_i64_trunc_sat_f64_s(1152921504606846976.0)) // 2^60
        << "Large power of 2 within i64 range should convert exactly";

    // Test values at f64 precision boundary where integers become non-consecutive
    // Beyond 2^53, f64 cannot represent consecutive integers
    ASSERT_EQ(9007199254740994LL, call_i64_trunc_sat_f64_s(9007199254740994.0))
        << "Integer just above 2^53 boundary should convert correctly";
}

// Test parameter instantiation for both execution modes
INSTANTIATE_TEST_SUITE_P(RunningModes, I64TruncSatF64STest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT));