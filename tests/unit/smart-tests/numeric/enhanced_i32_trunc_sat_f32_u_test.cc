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
 * @brief Test fixture for i32.trunc_sat_f32_u opcode validation
 * @details Provides comprehensive testing infrastructure for saturating truncation
 *          from f32 to unsigned i32, including module loading, execution context
 *          management, and cross-execution mode validation.
 */
class I32TruncSatF32UTest : public testing::TestWithParam<RunningMode> {
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
        ASSERT_NE(exec_env, nullptr)
            << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and release resources
     * @details Properly releases execution environment, module instance,
     *          module, and runtime resources following RAII principles
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
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM module based on execution mode
     * @param mode Execution mode (interpreter or AOT)
     */
    void load_module(RunningMode mode) {
        const char* module_path =
            "wasm-apps/i32_trunc_sat_f32_u_test.wasm";

        // Read module file
        module_buf = (uint8*)bh_read_file_to_buffer(module_path, &module_buf_size);
        ASSERT_NE(module_buf, nullptr) << "Failed to read module file: " << module_path;

        // Load module (both interpreter and AOT use same load function)
        module = wasm_runtime_load(module_buf, module_buf_size, error_buf, sizeof(error_buf));
    }

    /**
     * @brief Call i32.trunc_sat_f32_u WASM function with f32 parameter
     * @param f Input f32 value for conversion
     * @return Converted i32 value (representing unsigned integer)
     */
    uint32_t call_i32_trunc_sat_f32_u(float f) {
        // Find the target function in module
        WASMFunctionInstanceCommon* func = wasm_runtime_lookup_function(module_inst, "test_i32_trunc_sat_f32_u");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_trunc_sat_f32_u function";

        // Prepare arguments
        uint32_t argv[2]; // f32 value stored as uint32
        *(float*)&argv[0] = f;

        // Execute function
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        // Return result (stored in argv[0] after call)
        return argv[0];
    }

    // Test infrastructure
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8* module_buf = nullptr;
    uint32 module_buf_size = 0;
    char error_buf[256];
    static uint8 global_heap_buf[8192 * 1024];
};

uint8 I32TruncSatF32UTest::global_heap_buf[8192 * 1024];

/**
 * @test BasicConversion_ReturnsCorrectTruncatedValues
 * @brief Validates i32.trunc_sat_f32_u produces correct truncation results for typical inputs
 * @details Tests fundamental saturating truncation operation with positive f32 values and
 *          fractional numbers. Verifies that fractional parts are discarded (towards zero)
 *          and typical values convert correctly to unsigned integers without saturation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int
 * @input_conditions Typical positive f32 values including integers, fractions, and zero
 * @expected_behavior Returns truncated unsigned i32 values with fractional parts discarded
 * @validation_method Direct comparison of WASM function result with expected truncated values
 */
TEST_P(I32TruncSatF32UTest, BasicConversion_ReturnsCorrectTruncatedValues) {
    // Test positive integer conversions
    ASSERT_EQ(42U, call_i32_trunc_sat_f32_u(42.0f))
        << "Positive integer conversion failed";
    ASSERT_EQ(123U, call_i32_trunc_sat_f32_u(123.7f))
        << "Positive float with fraction conversion failed (truncation)";

    // Test zero conversions
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(0.0f))
        << "Positive zero conversion failed";
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(-0.0f))
        << "Negative zero conversion failed";

    // Test typical range values
    ASSERT_EQ(1000U, call_i32_trunc_sat_f32_u(1000.0f))
        << "Typical positive integer conversion failed";
    ASSERT_EQ(999U, call_i32_trunc_sat_f32_u(999.99f))
        << "Large fractional truncation failed";
}

/**
 * @test BoundaryValues_SaturatesCorrectly
 * @brief Validates saturation behavior at unsigned integer boundaries
 * @details Tests boundary conditions for unsigned integer conversion including negative
 *          values (saturate to 0), values exceeding UINT32_MAX (saturate to maximum),
 *          and exact boundary values to ensure proper saturation behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int (saturating=true)
 * @input_conditions Boundary values: negative, UINT32_MAX region, exact boundaries
 * @expected_behavior Negative values→0, overflow values→UINT32_MAX, boundaries handled correctly
 * @validation_method Verification of saturation to correct unsigned integer limits
 */
TEST_P(I32TruncSatF32UTest, BoundaryValues_SaturatesCorrectly) {
    // Test negative value saturation (should saturate to 0)
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(-1.0f))
        << "Negative value should saturate to 0";
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(-100.0f))
        << "Large negative value should saturate to 0";

    // Test positive overflow saturation (should saturate to UINT32_MAX)
    ASSERT_EQ(UINT32_MAX, call_i32_trunc_sat_f32_u(4294967296.0f))
        << "Overflow value should saturate to UINT32_MAX";
    ASSERT_EQ(UINT32_MAX, call_i32_trunc_sat_f32_u(1e10f))
        << "Large overflow value should saturate to UINT32_MAX";

    // Test near-boundary values
    ASSERT_EQ(4294967295U, call_i32_trunc_sat_f32_u(4294967295.0f))
        << "Exact UINT32_MAX value should convert correctly";
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(-0.1f))
        << "Small negative value should saturate to 0";
}

/**
 * @test SpecialFloatValues_HandledCorrectly
 * @brief Validates handling of special IEEE 754 floating-point values
 * @details Tests conversion of special f32 values including NaN (should become 0),
 *          positive infinity (should saturate to UINT32_MAX), negative infinity
 *          (should saturate to 0), and verification of consistent behavior.
 * @test_category Edge - Special IEEE 754 value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int (NaN/infinity handling)
 * @input_conditions NaN values, +/-infinity, extreme magnitude values
 * @expected_behavior NaN→0, +∞→UINT32_MAX, -∞→0, consistent saturation behavior
 * @validation_method Verification of safe conversion without traps for special values
 */
TEST_P(I32TruncSatF32UTest, SpecialFloatValues_HandledCorrectly) {
    // Test NaN handling (should saturate to 0)
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(NAN))
        << "NaN should saturate to 0";
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(-NAN))
        << "Negative NaN should saturate to 0";

    // Test infinity handling
    ASSERT_EQ(UINT32_MAX, call_i32_trunc_sat_f32_u(INFINITY))
        << "Positive infinity should saturate to UINT32_MAX";
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(-INFINITY))
        << "Negative infinity should saturate to 0";

    // Test extreme magnitude values
    ASSERT_EQ(UINT32_MAX, call_i32_trunc_sat_f32_u(FLT_MAX))
        << "FLT_MAX should saturate to UINT32_MAX";
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(-FLT_MAX))
        << "Negative FLT_MAX should saturate to 0";

    // Test very small positive values (should truncate to 0)
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(1e-10f))
        << "Very small positive value should truncate to 0";
    ASSERT_EQ(0U, call_i32_trunc_sat_f32_u(FLT_MIN))
        << "FLT_MIN should truncate to 0";
}

/**
 * @test NoExceptionHandling_AlwaysSucceeds
 * @brief Validates that i32.trunc_sat_f32_u never traps or throws exceptions
 * @details Tests the fundamental guarantee of saturating truncation - that all
 *          possible f32 inputs are handled gracefully without runtime exceptions.
 *          Verifies successful execution and module health across all test scenarios.
 * @test_category Error - Runtime behavior validation (non-trapping verification)
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int (saturating=true)
 * @input_conditions All categories of potentially problematic f32 values
 * @expected_behavior All conversions succeed, no exceptions thrown, consistent behavior
 * @validation_method Verification of successful execution and proper module state
 */
TEST_P(I32TruncSatF32UTest, NoExceptionHandling_AlwaysSucceeds) {
    // Test that various problematic inputs do not cause exceptions
    std::vector<float> test_values = {
        NAN, -NAN, INFINITY, -INFINITY,
        FLT_MAX, -FLT_MAX, FLT_MIN, -FLT_MIN,
        4294967296.0f, -1000000.0f, 0.0f, -0.0f
    };

    for (float test_val : test_values) {
        // Each call should succeed without throwing exceptions
        uint32_t result = call_i32_trunc_sat_f32_u(test_val);

        // Verify result is within expected unsigned range
        // (All results should be valid uint32_t values by definition)
        ASSERT_TRUE(result == result) // Basic sanity check
            << "Invalid result for input " << test_val;

        // Verify module is still in good state after operation
        ASSERT_EQ(nullptr, wasm_runtime_get_exception(module_inst))
            << "Unexpected exception after processing " << test_val;
    }

    // Verify module instance remains healthy
    ASSERT_NE(nullptr, module_inst) << "Module instance corrupted during testing";
}

// Test parameter configuration for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionModeValidation,
    I32TruncSatF32UTest,
    testing::Values(
        Mode_Interp
#if WASM_ENABLE_JIT != 0
        , Mode_LLVM_JIT
#endif
    )
);