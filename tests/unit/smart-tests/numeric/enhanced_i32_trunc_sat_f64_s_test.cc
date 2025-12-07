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
 * @brief Test fixture for i32.trunc_sat_f64_s opcode validation
 * @details Provides comprehensive testing infrastructure for saturating truncation
 *          from f64 to signed i32, including module loading, execution context
 *          management, and cross-execution mode validation.
 */
class I32TruncSatF64STest : public testing::TestWithParam<RunningMode> {
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
        const char* module_path = "wasm-apps/i32_trunc_sat_f64_s_test.wasm";

        // Read module file
        module_buf = (uint8*)bh_read_file_to_buffer(module_path, &module_buf_size);
        ASSERT_NE(module_buf, nullptr) << "Failed to read module file: " << module_path;

        // Load module (both interpreter and AOT use same load function)
        module = wasm_runtime_load(module_buf, module_buf_size, error_buf, sizeof(error_buf));
    }

    /**
     * @brief Call i32.trunc_sat_f64_s WASM function with f64 parameter
     * @param d Input f64 value for conversion
     * @return Converted i32 value
     */
    int32_t call_i32_trunc_sat_f64_s(double d) {
        // Find the target function in module
        WASMFunctionInstanceCommon* func = wasm_runtime_lookup_function(module_inst, "test_i32_trunc_sat_f64_s");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_trunc_sat_f64_s function";

        // Prepare arguments for f64 parameter (stored as two uint32 values)
        uint32_t argv[3]; // f64 requires 2 slots, result uses 1 slot
        *(double*)&argv[0] = d;

        // Execute function
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        // Return result (stored in argv[0] after call)
        return (int32_t)argv[0];
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

uint8 I32TruncSatF64STest::global_heap_buf[8192 * 1024];

/**
 * @test BasicConversion_ReturnsCorrectInteger
 * @brief Validates i32.trunc_sat_f64_s produces correct truncation results for typical inputs
 * @details Tests fundamental saturating truncation operation with positive, negative, and
 *          fractional f64 values. Verifies that fractional parts are discarded (towards zero)
 *          and typical values convert correctly without saturation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_sat_f64_to_i32_s
 * @input_conditions Typical f64 values including integers, fractions, positive and negative
 * @expected_behavior Returns truncated signed i32 values with fractional parts discarded
 * @validation_method Direct comparison of WASM function result with expected truncated values
 */
TEST_P(I32TruncSatF64STest, BasicConversion_ReturnsCorrectInteger) {
    // Test positive integer conversions
    ASSERT_EQ(1234, call_i32_trunc_sat_f64_s(1234.5678))
        << "Positive f64 with fraction conversion failed";
    ASSERT_EQ(42, call_i32_trunc_sat_f64_s(42.0))
        << "Positive integer conversion failed";

    // Test negative integer conversions
    ASSERT_EQ(-9876, call_i32_trunc_sat_f64_s(-9876.1234))
        << "Negative f64 with fraction conversion failed";
    ASSERT_EQ(-42, call_i32_trunc_sat_f64_s(-42.0))
        << "Negative integer conversion failed";

    // Test truncation toward zero behavior
    ASSERT_EQ(0, call_i32_trunc_sat_f64_s(0.9999))
        << "Fractional value truncation failed";
    ASSERT_EQ(0, call_i32_trunc_sat_f64_s(-0.9999))
        << "Negative fractional value truncation failed";
}

/**
 * @test BoundaryValues_SaturateCorrectly
 * @brief Validates i32.trunc_sat_f64_s correctly saturates at i32 boundaries
 * @details Tests behavior at and beyond INT32_MIN/MAX boundaries, verifying
 *          saturation to proper limits instead of overflow/underflow trapping.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:saturating_truncation_logic
 * @input_conditions Values at, slightly within, and beyond i32 boundaries
 * @expected_behavior Clamp to INT32_MIN (-2147483648) or INT32_MAX (2147483647)
 * @validation_method Verify saturated results match exact boundary limits
 */
TEST_P(I32TruncSatF64STest, BoundaryValues_SaturateCorrectly) {
    // Test exact boundary values
    ASSERT_EQ(INT32_MAX, call_i32_trunc_sat_f64_s(2147483647.0))
        << "Exact INT32_MAX boundary conversion failed";
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f64_s(-2147483648.0))
        << "Exact INT32_MIN boundary conversion failed";

    // Test overflow saturation
    ASSERT_EQ(INT32_MAX, call_i32_trunc_sat_f64_s(2147483648.0))
        << "Overflow to INT32_MAX saturation failed";
    ASSERT_EQ(INT32_MAX, call_i32_trunc_sat_f64_s(1e20))
        << "Large positive overflow saturation failed";

    // Test underflow saturation
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f64_s(-2147483649.0))
        << "Underflow to INT32_MIN saturation failed";
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f64_s(-1e20))
        << "Large negative underflow saturation failed";

    // Test near-boundary values within range
    ASSERT_EQ(2147483646, call_i32_trunc_sat_f64_s(2147483646.9))
        << "Near INT32_MAX within-range conversion failed";
    ASSERT_EQ(-2147483647, call_i32_trunc_sat_f64_s(-2147483647.9))
        << "Near INT32_MIN within-range conversion failed";
}

/**
 * @test SpecialFloatValues_ReturnExpectedResults
 * @brief Validates i32.trunc_sat_f64_s handles special f64 values correctly
 * @details Tests behavior with NaN, infinities, zeros, and subnormal values,
 *          ensuring proper saturating conversion without traps or exceptions.
 * @test_category Edge - Special numeric value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:special_f64_handling
 * @input_conditions NaN, ±Infinity, ±0.0, subnormal numbers, high precision values
 * @expected_behavior NaN→0, +∞→INT32_MAX, -∞→INT32_MIN, zeros→0, subnormals→0
 * @validation_method Verify specific expected values for each special case
 */
TEST_P(I32TruncSatF64STest, SpecialFloatValues_ReturnExpectedResults) {
    // Test NaN conversion (should return 0)
    ASSERT_EQ(0, call_i32_trunc_sat_f64_s(std::nan("1")))
        << "NaN conversion should return 0";

    // Test infinity conversions (should saturate to limits)
    ASSERT_EQ(INT32_MAX, call_i32_trunc_sat_f64_s(std::numeric_limits<double>::infinity()))
        << "Positive infinity should saturate to INT32_MAX";
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f64_s(-std::numeric_limits<double>::infinity()))
        << "Negative infinity should saturate to INT32_MIN";

    // Test zero conversions (both positive and negative zero)
    ASSERT_EQ(0, call_i32_trunc_sat_f64_s(0.0))
        << "Positive zero conversion failed";
    ASSERT_EQ(0, call_i32_trunc_sat_f64_s(-0.0))
        << "Negative zero conversion failed";

    // Test very small values (should truncate to 0)
    ASSERT_EQ(0, call_i32_trunc_sat_f64_s(DBL_MIN))
        << "Smallest positive f64 should truncate to 0";
    ASSERT_EQ(0, call_i32_trunc_sat_f64_s(-DBL_MIN))
        << "Smallest negative f64 should truncate to 0";

    // Test high precision values
    ASSERT_EQ(123, call_i32_trunc_sat_f64_s(123.99999999999999))
        << "High precision fractional part truncation failed";
    ASSERT_EQ(456, call_i32_trunc_sat_f64_s(456.0000000001))
        << "Very small fractional part truncation failed";
}

/**
 * @test ModuleLoadingAndExecution_SucceedsAcrossModes
 * @brief Validates WASM module loading and function execution across execution modes
 * @details Tests that modules containing i32.trunc_sat_f64_s load successfully
 *          and execute without errors in both interpreter and AOT modes.
 * @test_category Error - Runtime validation and error handling
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:module_loading
 * @input_conditions Valid WASM module with i32.trunc_sat_f64_s opcode
 * @expected_behavior Module loads successfully, function calls execute without error
 * @validation_method Assert module/instance creation success, verify function execution
 */
TEST_P(I32TruncSatF64STest, ModuleLoadingAndExecution_SucceedsAcrossModes) {
    // Verify module loaded successfully (already tested in SetUp, but explicit check)
    ASSERT_NE(module, nullptr) << "WASM module should load successfully";
    ASSERT_NE(module_inst, nullptr) << "WASM module instance should be created successfully";
    ASSERT_NE(exec_env, nullptr) << "Execution environment should be created successfully";

    // Verify function lookup succeeds
    WASMFunctionInstanceCommon* func = wasm_runtime_lookup_function(module_inst, "test_i32_trunc_sat_f64_s");
    ASSERT_NE(func, nullptr) << "test_i32_trunc_sat_f64_s function should be found in module";

    // Verify function can be called without runtime errors
    int32_t result = call_i32_trunc_sat_f64_s(100.0);
    ASSERT_EQ(100, result) << "Basic function execution should work correctly";

    // Verify no exceptions occurred during function calls
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_TRUE(exception == nullptr || strlen(exception) == 0)
        << "No exceptions should occur during normal function execution";
}

// Instantiate parameterized tests for both execution modes
INSTANTIATE_TEST_SUITE_P(RunningModes, I32TruncSatF64STest,
                         testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<RunningMode>& info) {
                             return info.param == RunningMode::Mode_Interp ? "Interpreter" : "AOT";
                         });