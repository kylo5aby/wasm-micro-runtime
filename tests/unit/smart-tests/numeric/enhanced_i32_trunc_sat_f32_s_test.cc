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
 * @brief Test fixture for i32.trunc_sat_f32_s opcode validation
 * @details Provides comprehensive testing infrastructure for saturating truncation
 *          from f32 to signed i32, including module loading, execution context
 *          management, and cross-execution mode validation.
 */
class I32TruncSatF32STest : public testing::TestWithParam<RunningMode> {
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
            "wasm-apps/i32_trunc_sat_f32_s_test.wasm";

        // Read module file
        module_buf = (uint8*)bh_read_file_to_buffer(module_path, &module_buf_size);
        ASSERT_NE(module_buf, nullptr) << "Failed to read module file: " << module_path;

        // Load module (both interpreter and AOT use same load function)
        module = wasm_runtime_load(module_buf, module_buf_size, error_buf, sizeof(error_buf));
    }

    /**
     * @brief Call i32.trunc_sat_f32_s WASM function with f32 parameter
     * @param f Input f32 value for conversion
     * @return Converted i32 value
     */
    int32_t call_i32_trunc_sat_f32_s(float f) {
        // Find the target function in module
        WASMFunctionInstanceCommon* func = wasm_runtime_lookup_function(module_inst, "test_i32_trunc_sat_f32_s");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_trunc_sat_f32_s function";

        // Prepare arguments
        uint32_t argv[2]; // f32 value stored as uint32
        *(float*)&argv[0] = f;

        // Execute function
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
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

uint8 I32TruncSatF32STest::global_heap_buf[8192 * 1024];

/**
 * @test BasicConversion_ReturnsCorrectResults
 * @brief Validates i32.trunc_sat_f32_s produces correct truncation results for typical inputs
 * @details Tests fundamental saturating truncation operation with positive, negative, and
 *          fractional f32 values. Verifies that fractional parts are discarded (towards zero)
 *          and typical values convert correctly without saturation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int
 * @input_conditions Typical f32 values including integers, fractions, positive and negative
 * @expected_behavior Returns truncated signed i32 values with fractional parts discarded
 * @validation_method Direct comparison of WASM function result with expected truncated values
 */
TEST_P(I32TruncSatF32STest, BasicConversion_ReturnsCorrectResults) {
    // Test positive integer conversions
    ASSERT_EQ(42, call_i32_trunc_sat_f32_s(42.0f))
        << "Positive integer conversion failed";
    ASSERT_EQ(100, call_i32_trunc_sat_f32_s(100.7f))
        << "Positive float with fraction conversion failed";

    // Test negative integer conversions
    ASSERT_EQ(-42, call_i32_trunc_sat_f32_s(-42.0f))
        << "Negative integer conversion failed";
    ASSERT_EQ(-100, call_i32_trunc_sat_f32_s(-100.7f))
        << "Negative float with fraction conversion failed";

    // Test fractional truncation (toward zero)
    ASSERT_EQ(3, call_i32_trunc_sat_f32_s(3.14f))
        << "Positive fractional truncation failed";
    ASSERT_EQ(-3, call_i32_trunc_sat_f32_s(-3.14f))
        << "Negative fractional truncation failed";

    // Test zero cases
    ASSERT_EQ(0, call_i32_trunc_sat_f32_s(0.0f))
        << "Positive zero conversion failed";
    ASSERT_EQ(0, call_i32_trunc_sat_f32_s(-0.0f))
        << "Negative zero conversion failed";
}

/**
 * @test BoundaryConversion_HandlesBounds
 * @brief Validates i32.trunc_sat_f32_s handles values near INT32_MIN/MAX boundaries
 * @details Tests conversion behavior at and near the signed 32-bit integer limits.
 *          Verifies that values within the representable range convert correctly
 *          and values at the boundaries behave as expected.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int
 * @input_conditions f32 values near INT32_MIN and INT32_MAX boundaries
 * @expected_behavior Proper conversion for in-range values, correct boundary handling
 * @validation_method Comparison with expected boundary values and limits
 */
TEST_P(I32TruncSatF32STest, BoundaryConversion_HandlesBounds) {
    // Test values near but within INT32_MAX range
    // f32 precision limits mean we can't represent all i32 values exactly
    ASSERT_EQ(2147483520, call_i32_trunc_sat_f32_s(2147483520.0f))
        << "Large positive in-range conversion failed";

    // Test exact INT32_MIN (should be representable in f32)
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f32_s(-2147483648.0f))
        << "INT32_MIN boundary conversion failed";

    // Test values near the saturation boundaries
    ASSERT_EQ(-2147483648, call_i32_trunc_sat_f32_s(-2147483904.0f))
        << "Near minimum boundary conversion failed";
}

/**
 * @test SpecialValues_HandlesProperly
 * @brief Validates i32.trunc_sat_f32_s handles special IEEE 754 floating-point values
 * @details Tests NaN, positive/negative infinity, and denormal number conversion.
 *          Verifies saturating behavior: NaN→0, +∞→INT32_MAX, -∞→INT32_MIN,
 *          and that very small denormal numbers truncate to zero.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int
 * @input_conditions IEEE 754 special values: NaN, ±infinity, ±zero, denormals
 * @expected_behavior Proper saturation: NaN→0, ∞→limits, denormals→0
 * @validation_method Verification of saturated results for each special case
 */
TEST_P(I32TruncSatF32STest, SpecialValues_HandlesProperly) {
    // Test NaN conversion (should saturate to 0)
    ASSERT_EQ(0, call_i32_trunc_sat_f32_s(NAN))
        << "NaN should saturate to 0";
    ASSERT_EQ(0, call_i32_trunc_sat_f32_s(-NAN))
        << "Negative NaN should saturate to 0";

    // Test infinity conversion (should saturate to limits)
    ASSERT_EQ(INT32_MAX, call_i32_trunc_sat_f32_s(INFINITY))
        << "Positive infinity should saturate to INT32_MAX";
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f32_s(-INFINITY))
        << "Negative infinity should saturate to INT32_MIN";

    // Test very small denormal numbers (should truncate to 0)
    ASSERT_EQ(0, call_i32_trunc_sat_f32_s(1e-40f))
        << "Small denormal should truncate to 0";
    ASSERT_EQ(0, call_i32_trunc_sat_f32_s(-1e-40f))
        << "Negative small denormal should truncate to 0";

    // Test values very close to zero
    ASSERT_EQ(0, call_i32_trunc_sat_f32_s(0.9f))
        << "Value less than 1.0 should truncate to 0";
    ASSERT_EQ(0, call_i32_trunc_sat_f32_s(-0.9f))
        << "Negative value greater than -1.0 should truncate to 0";
}

/**
 * @test SaturationBehavior_ClampsCorrectly
 * @brief Validates i32.trunc_sat_f32_s properly saturates on overflow/underflow
 * @details Tests behavior when f32 input values exceed the range representable
 *          by signed 32-bit integers. Verifies that overflow cases saturate to
 *          INT32_MAX and underflow cases saturate to INT32_MIN without trapping.
 * @test_category Edge - Saturation behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int
 * @input_conditions f32 values that exceed signed 32-bit integer range
 * @expected_behavior Saturation to INT32_MIN/MAX without traps or exceptions
 * @validation_method Verification that extreme values clamp to appropriate limits
 */
TEST_P(I32TruncSatF32STest, SaturationBehavior_ClampsCorrectly) {
    // Test positive overflow (should saturate to INT32_MAX)
    ASSERT_EQ(INT32_MAX, call_i32_trunc_sat_f32_s(3e9f))
        << "Large positive value should saturate to INT32_MAX";
    ASSERT_EQ(INT32_MAX, call_i32_trunc_sat_f32_s(1e10f))
        << "Very large positive value should saturate to INT32_MAX";

    // Test negative underflow (should saturate to INT32_MIN)
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f32_s(-3e9f))
        << "Large negative value should saturate to INT32_MIN";
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f32_s(-1e10f))
        << "Very large negative value should saturate to INT32_MIN";

    // Test edge of saturation bounds (from WAMR implementation)
    ASSERT_EQ(INT32_MAX, call_i32_trunc_sat_f32_s(2147483648.0f))
        << "Value at upper saturation bound should saturate to INT32_MAX";
    ASSERT_EQ(INT32_MIN, call_i32_trunc_sat_f32_s(-2147483904.0f))
        << "Value at lower saturation bound should saturate to INT32_MIN";
}

// Test parameter configuration for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionModeValidation,
    I32TruncSatF32STest,
    testing::Values(
        Mode_Interp
#if WASM_ENABLE_JIT != 0
        , Mode_LLVM_JIT
#endif
    )
);