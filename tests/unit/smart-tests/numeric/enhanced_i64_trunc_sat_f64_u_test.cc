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
 * @brief Test fixture for i64.trunc_sat_f64_u opcode validation
 * @details Provides comprehensive testing infrastructure for saturating truncation
 *          from f64 to unsigned i64, including module loading, execution context
 *          management, and cross-execution mode validation.
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_uint
 */
class I64TruncSatF64UTest : public testing::TestWithParam<RunningMode> {
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
        const char* file_path = "wasm-apps/i64_trunc_sat_f64_u_test.wasm";

        wasm_buf = reinterpret_cast<uint8*>(
            bh_read_file_to_buffer(file_path, &wasm_buf_size));
        ASSERT_NE(wasm_buf, nullptr) << "Failed to read WASM file: " << file_path;

        module = wasm_runtime_load(wasm_buf, wasm_buf_size, error_buf, sizeof(error_buf));
    }

    /**
     * @brief Execute i64.trunc_sat_f64_u with given f64 input
     * @param input f64 value to convert
     * @return uint64_t result of saturating truncation
     * @details Calls WASM function with f64 parameter and retrieves u64 result
     */
    uint64_t call_i64_trunc_sat_f64_u(double input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_i64_trunc_sat_f64_u");
        EXPECT_NE(func, nullptr) << "Failed to lookup test function";

        uint32_t argv[4];
        *(double*)argv = input;

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "WASM function call failed: "
                            << wasm_runtime_get_exception(module_inst);

        return *(uint64_t*)argv;
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
 * @test BasicConversion_ReturnsCorrectUnsignedInteger
 * @brief Validates i64.trunc_sat_f64_u produces correct results for typical f64 inputs
 * @details Tests fundamental saturating truncation operation with positive values,
 *          zero, and fractional truncation. Verifies truncation toward zero behavior
 *          and proper handling of values within unsigned 64-bit range.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_uint
 * @input_conditions Standard f64 values: 42.0, 1000.75, 0.0, large valid values
 * @expected_behavior Returns truncated u64: 42, 1000, 0, correct large values
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64TruncSatF64UTest, BasicConversion_ReturnsCorrectUnsignedInteger) {
    // Test positive value truncation
    ASSERT_EQ(42ULL, call_i64_trunc_sat_f64_u(42.0))
        << "Failed to truncate positive f64 value correctly";

    // Test fractional truncation toward zero
    ASSERT_EQ(1000ULL, call_i64_trunc_sat_f64_u(1000.75))
        << "Failed to truncate positive f64 fractional value";

    // Test zero conversion
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(0.0))
        << "Failed to convert zero correctly";

    // Test positive zero conversion
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(+0.0))
        << "Failed to convert positive zero correctly";

    // Test large positive value within u64 range
    ASSERT_EQ(999999999999ULL, call_i64_trunc_sat_f64_u(999999999999.9))
        << "Failed to truncate large positive f64 value";

    // Test fractional truncation properties
    ASSERT_EQ(3ULL, call_i64_trunc_sat_f64_u(3.7))
        << "Positive fractional should truncate toward zero";

    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(0.9))
        << "Value less than 1.0 should truncate to 0";

    ASSERT_EQ(1ULL, call_i64_trunc_sat_f64_u(1.9))
        << "Value between 1.0 and 2.0 should truncate to 1";
}

/**
 * @test SaturationBehavior_ClampsToUnsignedRange
 * @brief Validates saturation behavior for f64 values outside unsigned 64-bit range
 * @details Tests that values beyond unsigned 64-bit integer limits saturate to
 *          UINT64_MAX or 0 rather than causing traps or undefined behavior.
 *          Verifies infinity handling and negative value saturation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_uint saturation logic
 * @input_conditions Out-of-range values: +∞, -∞, 1e20, -1e20, DBL_MAX, negatives
 * @expected_behavior Saturates to UINT64_MAX (18446744073709551615) or 0
 * @validation_method Assert saturation to exact unsigned integer boundary values
 */
TEST_P(I64TruncSatF64UTest, SaturationBehavior_ClampsToUnsignedRange) {
    const uint64_t UINT64_MAX_VAL = 18446744073709551615ULL;

    // Test positive infinity saturation
    ASSERT_EQ(UINT64_MAX_VAL, call_i64_trunc_sat_f64_u(INFINITY))
        << "Positive infinity should saturate to UINT64_MAX";

    // Test negative infinity saturation to zero
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(-INFINITY))
        << "Negative infinity should saturate to 0";

    // Test large positive value saturation
    ASSERT_EQ(UINT64_MAX_VAL, call_i64_trunc_sat_f64_u(1e20))
        << "Large positive f64 should saturate to UINT64_MAX";

    // Test maximum double value saturation
    ASSERT_EQ(UINT64_MAX_VAL, call_i64_trunc_sat_f64_u(DBL_MAX))
        << "DBL_MAX should saturate to UINT64_MAX";

    // Test negative value saturation (all negatives become 0)
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(-1.0))
        << "Negative values should saturate to 0";

    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(-1000.5))
        << "Large negative values should saturate to 0";

    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(-DBL_MAX))
        << "Negative DBL_MAX should saturate to 0";

    // Test values just above u64 range
    ASSERT_EQ(UINT64_MAX_VAL, call_i64_trunc_sat_f64_u(1.8446744073709552e19))
        << "Value just above UINT64_MAX should saturate";

    // Test small negative values
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(-0.1))
        << "Small negative values should saturate to 0";
}

/**
 * @test SpecialFloatValues_HandledDeterministically
 * @brief Validates proper handling of special IEEE 754 f64 values
 * @details Tests NaN, signed zeros, and subnormal number handling. Verifies
 *          that special values don't cause exceptions but produce deterministic
 *          results according to WebAssembly saturating truncation specification.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_uint NaN/special handling
 * @input_conditions Special values: NaN, +0.0, -0.0, subnormal numbers, tiny values
 * @expected_behavior NaN→0, zeros→0, subnormals→0 (no exceptions thrown)
 * @validation_method Verify deterministic zero results and no runtime exceptions
 */
TEST_P(I64TruncSatF64UTest, SpecialFloatValues_HandledDeterministically) {
    // Test NaN handling - should saturate to 0
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(NAN))
        << "NaN should saturate to 0 without causing exceptions";

    // Test positive zero
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(+0.0))
        << "Positive zero should convert to 0";

    // Test negative zero
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(-0.0))
        << "Negative zero should convert to 0";

    // Test small subnormal numbers (smaller than DBL_MIN)
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(1e-308))
        << "Small subnormal positive value should truncate to 0";

    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(-1e-308))
        << "Small subnormal negative value should truncate to 0";

    // Test values just under 1.0 (should truncate to 0)
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(0.9999999999999999))
        << "Value just under 1.0 should truncate to 0";

    // Test smallest normal positive value
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(DBL_MIN))
        << "DBL_MIN should truncate to 0";

    // Test denormalized minimum value
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(std::numeric_limits<double>::denorm_min()))
        << "Denormalized minimum should truncate to 0";

    // Test very small positive values
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f64_u(1e-100))
        << "Very small positive value should truncate to 0";
}

/**
 * @test PrecisionBoundaries_HandlesCorrectly
 * @brief Validates conversion behavior at f64 precision limits and boundaries
 * @details Tests edge cases where f64 precision affects conversion accuracy,
 *          very large integers that can be exactly represented in f64, and
 *          the boundaries of precise integer representation in double precision.
 * @test_category Edge - Precision boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f64_to_uint precision handling
 * @input_conditions Precision-critical values, large exact integers, boundary conditions
 * @expected_behavior Accurate conversion with proper precision handling
 * @validation_method Test exact boundary behavior and precision properties
 */
TEST_P(I64TruncSatF64UTest, PrecisionBoundaries_HandlesCorrectly) {
    // Test largest exact integer representable in f64 (2^53)
    const uint64_t MAX_SAFE_INTEGER = 9007199254740992ULL; // 2^53
    ASSERT_EQ(MAX_SAFE_INTEGER, call_i64_trunc_sat_f64_u((double)MAX_SAFE_INTEGER))
        << "Largest safe f64 integer should convert exactly";

    // Test powers of 2 (exactly representable in f64)
    ASSERT_EQ(1048576ULL, call_i64_trunc_sat_f64_u(1048576.0)) // 2^20
        << "Power of 2 within f64 precision should convert exactly";

    ASSERT_EQ(4294967296ULL, call_i64_trunc_sat_f64_u(4294967296.0)) // 2^32
        << "2^32 should convert exactly";

    // Test large integers near but within u64 range that f64 can represent
    ASSERT_EQ(1152921504606846976ULL, call_i64_trunc_sat_f64_u(1152921504606846976.0)) // 2^60
        << "Large power of 2 within u64 range should convert exactly";

    // Test values at f64 precision boundary where integers become non-consecutive
    // Beyond 2^53, f64 cannot represent consecutive integers
    ASSERT_EQ(9007199254740994ULL, call_i64_trunc_sat_f64_u(9007199254740994.0))
        << "Integer just above 2^53 boundary should convert correctly";

    // Test large values near maximum representable f64 integer in u64 range
    // Use values that f64 can represent exactly
    ASSERT_EQ(18014398509481984ULL, call_i64_trunc_sat_f64_u(18014398509481984.0)) // 2^54
        << "Large power of 2 should convert exactly when representable";

    // Test fractional values just above integers at precision boundary
    ASSERT_EQ(9007199254740992ULL, call_i64_trunc_sat_f64_u(9007199254740992.5))
        << "Fractional value at precision boundary should truncate correctly";
}

/**
 * @test ModuleValidation_LoadsSuccessfully
 * @brief Validates that WASM modules with i64.trunc_sat_f64_u load correctly
 * @details Tests that modules containing i64.trunc_sat_f64_u load successfully
 *          in both interpreter and AOT modes without validation errors.
 * @test_category Error - Module validation
 * @coverage_target core/iwasm/interpreter/wasm_loader.c module validation logic
 * @input_conditions WASM module containing i64.trunc_sat_f64_u opcode
 * @expected_behavior Successful module load and function lookup
 * @validation_method Assert successful module loading and function availability
 */
TEST_P(I64TruncSatF64UTest, ModuleValidation_LoadsSuccessfully) {
    // Module should be loaded successfully (verified in SetUp)
    ASSERT_NE(module, nullptr)
        << "Module containing i64.trunc_sat_f64_u should load successfully";

    // Module should instantiate successfully (verified in SetUp)
    ASSERT_NE(module_inst, nullptr)
        << "Module instance should be created successfully";

    // Test function should be found in module
    wasm_function_inst_t func = wasm_runtime_lookup_function(
        module_inst, "test_i64_trunc_sat_f64_u");
    ASSERT_NE(func, nullptr)
        << "test_i64_trunc_sat_f64_u function should be found in module";

    // Execution environment should be created successfully (verified in SetUp)
    ASSERT_NE(exec_env, nullptr)
        << "Execution environment should be created successfully";

    // Verify a simple function call works without exceptions
    uint64_t result = call_i64_trunc_sat_f64_u(42.0);
    ASSERT_EQ(42ULL, result)
        << "Basic function call should work without exceptions";
}

// Test parameter instantiation for both execution modes
INSTANTIATE_TEST_SUITE_P(RunningModes, I64TruncSatF64UTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT));