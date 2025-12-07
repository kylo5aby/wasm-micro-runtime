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
 * @brief Test fixture for i64.trunc_sat_f32_u opcode validation
 * @details Provides comprehensive testing infrastructure for saturating truncation
 *          from f32 to unsigned i64, including module loading, execution context
 *          management, and cross-execution mode validation.
 */
class I64TruncSatF32UTest : public testing::TestWithParam<RunningMode> {
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

        // Find the test function
        func_i64_trunc_sat_f32_u = wasm_runtime_lookup_function(
            module_inst, "test_i64_trunc_sat_f32_u");
        ASSERT_NE(func_i64_trunc_sat_f32_u, nullptr)
            << "Failed to find test_i64_trunc_sat_f32_u function";

        exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and free resources
     * @details Destroys execution environment, deinstantiates module,
     *          unloads module, and cleans up WAMR runtime
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
     * @details Loads either bytecode for interpreter or compiled module for AOT
     * @param mode Execution mode (interpreter or AOT)
     */
    void load_module(RunningMode mode) {
        if (mode == Mode_Interp) {
            load_module_from_file();
        }
#if WASM_ENABLE_AOT != 0
        else if (mode == Mode_LLVM_JIT) {
            load_aot_module_from_file();
        }
#endif
    }

    /**
     * @brief Load WASM module from bytecode file for interpreter mode
     * @details Reads .wasm file and loads it into WAMR interpreter
     */
    void load_module_from_file() {
        const char *file_path = "wasm-apps/i64_trunc_sat_f32_u_test.wasm";

        wasm_buf = (uint8_t *)bh_read_file_to_buffer(file_path, &wasm_buf_size);
        ASSERT_NE(wasm_buf, nullptr) << "Failed to read WASM file: " << file_path;

        module = wasm_runtime_load(wasm_buf, wasm_buf_size,
                                  error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;
    }

#if WASM_ENABLE_AOT != 0
    /**
     * @brief Load AOT compiled module for AOT mode
     * @details Loads pre-compiled AOT module for ahead-of-time execution
     */
    void load_aot_module_from_file() {
        const char *file_path = "wasm-apps/i64_trunc_sat_f32_u_test.aot";

        aot_buf = (uint8_t *)bh_read_file_to_buffer(file_path, &aot_buf_size);
        ASSERT_NE(aot_buf, nullptr) << "Failed to read AOT file: " << file_path;

        module = wasm_runtime_load(aot_buf, aot_buf_size,
                                  error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load AOT module: " << error_buf;
    }
#endif

    /**
     * @brief Execute i64.trunc_sat_f32_u operation with given input
     * @details Calls WASM function with f32 input and returns u64 result
     * @param input f32 value to convert
     * @return Converted u64 value or 0 on execution failure
     */
    uint64_t call_i64_trunc_sat_f32_u(float input) {
        uint32_t argv[2];
        argv[0] = *(uint32_t*)&input;  // f32 as uint32 bit pattern

        bool success = wasm_runtime_call_wasm(
            exec_env, func_i64_trunc_sat_f32_u, 1, argv);
        EXPECT_TRUE(success) << "WASM function call failed: "
                           << wasm_runtime_get_exception(module_inst);

        // Return value is stored in argv[0:1] as 64-bit value
        return ((uint64_t)argv[1] << 32) | argv[0];
    }

    // Test infrastructure members
    WASMModuleInstanceCommon *module_inst{nullptr};
    WASMModuleCommon *module{nullptr};
    WASMFunctionInstanceCommon *func_i64_trunc_sat_f32_u{nullptr};
    WASMExecEnv *exec_env{nullptr};
    char error_buf[256];
    uint8_t *wasm_buf{nullptr};
    uint32_t wasm_buf_size{0};
#if WASM_ENABLE_AOT != 0
    uint8_t *aot_buf{nullptr};
    uint32_t aot_buf_size{0};
#endif
    RuntimeInitArgs init_args;
    uint8_t global_heap_buf[512 * 1024];
};

/**
 * @test BasicConversion_ReturnsCorrectResults
 * @brief Validates i64.trunc_sat_f32_u produces correct results for typical f32 inputs
 * @details Tests fundamental conversion operation with standard positive values,
 *          large values that exceed u32 range, and zero value conversion.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:I64_TRUNC_SAT_U_F32
 * @input_conditions Standard f32 values: 123.45f, 5000000000.0f, 0.0f
 * @expected_behavior Returns truncated integer: 123ULL, 5000000000ULL, 0ULL
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64TruncSatF32UTest, BasicConversion_ReturnsCorrectResults) {
    // Test standard positive conversion
    ASSERT_EQ(123ULL, call_i64_trunc_sat_f32_u(123.45f))
        << "Failed to convert 123.45f to 123ULL";

    // Test large value that exceeds u32 but fits in u64
    ASSERT_EQ(5000000000ULL, call_i64_trunc_sat_f32_u(5000000000.0f))
        << "Failed to convert 5000000000.0f to 5000000000ULL";

    // Test zero conversion
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(0.0f))
        << "Failed to convert 0.0f to 0ULL";
}

/**
 * @test BoundaryValues_SaturateCorrectly
 * @brief Validates i64.trunc_sat_f32_u saturates correctly at u64 boundaries
 * @details Tests overflow saturation to UINT64_MAX and underflow saturation to 0
 *          for values outside the representable u64 range.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:trunc_f32_to_i64
 * @input_conditions Overflow: 2e19f, Underflow: -100.0f, Near boundary: 1.8446744e19f
 * @expected_behavior Saturates to UINT64_MAX for overflow, 0ULL for underflow
 * @validation_method Verification of saturation behavior at u64 limits
 */
TEST_P(I64TruncSatF32UTest, BoundaryValues_SaturateCorrectly) {
    // Test positive overflow - should saturate to UINT64_MAX
    ASSERT_EQ(UINT64_MAX, call_i64_trunc_sat_f32_u(2e19f))
        << "Failed to saturate positive overflow to UINT64_MAX";

    // Test negative values - should saturate to 0
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(-100.0f))
        << "Failed to saturate negative value to 0ULL";

    // Test near boundary value just below overflow threshold
    float near_max = 1.8446744e19f;  // Close to UINT64_MAX in f32 precision
    uint64_t result = call_i64_trunc_sat_f32_u(near_max);
    ASSERT_TRUE(result == UINT64_MAX || result > (UINT64_MAX - 1000000ULL))
        << "Near-boundary value conversion failed: " << result;
}

/**
 * @test SpecialFloatValues_HandleGracefully
 * @brief Validates i64.trunc_sat_f32_u handles special IEEE-754 values correctly
 * @details Tests NaN, positive/negative infinity, positive/negative zero,
 *          and subnormal number conversion behavior according to WASM spec.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:TRUNC_FUNCTION
 * @input_conditions NaN, +∞, -∞, +0.0f, -0.0f, subnormal numbers
 * @expected_behavior NaN→0, +∞→UINT64_MAX, -∞→0, ±0→0, subnormal→0
 * @validation_method Special value handling verification per WASM specification
 */
TEST_P(I64TruncSatF32UTest, SpecialFloatValues_HandleGracefully) {
    // Test NaN - should convert to 0
    float nan_val = std::nanf("");
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(nan_val))
        << "Failed to convert NaN to 0ULL";

    // Test positive infinity - should saturate to UINT64_MAX
    ASSERT_EQ(UINT64_MAX, call_i64_trunc_sat_f32_u(INFINITY))
        << "Failed to saturate +∞ to UINT64_MAX";

    // Test negative infinity - should saturate to 0
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(-INFINITY))
        << "Failed to saturate -∞ to 0ULL";

    // Test positive and negative zero
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(+0.0f))
        << "Failed to convert +0.0f to 0ULL";
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(-0.0f))
        << "Failed to convert -0.0f to 0ULL";
}

/**
 * @test SaturationBehavior_ClampsToRange
 * @brief Validates i64.trunc_sat_f32_u clamps values outside valid u64 range
 * @details Tests systematic saturation behavior for various out-of-range values,
 *          ensuring consistent clamping to [0, UINT64_MAX] boundaries.
 * @test_category Edge - Saturation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:DEF_OP_TRUNC_SAT_F32
 * @input_conditions Various out-of-range values: very large, very negative, extreme
 * @expected_behavior Consistent clamping to valid u64 range [0, UINT64_MAX]
 * @validation_method Range validation and saturation consistency verification
 */
TEST_P(I64TruncSatF32UTest, SaturationBehavior_ClampsToRange) {
    // Test extreme positive values
    ASSERT_EQ(UINT64_MAX, call_i64_trunc_sat_f32_u(FLT_MAX))
        << "Failed to saturate FLT_MAX to UINT64_MAX";

    // Test small negative values
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(-1.0f))
        << "Failed to saturate -1.0f to 0ULL";

    // Test fractional values near zero
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(0.9f))
        << "Failed to truncate 0.9f to 0ULL";

    // Test subnormal number (very small positive value)
    float subnormal = FLT_MIN / 2.0f;  // Create subnormal number
    ASSERT_EQ(0ULL, call_i64_trunc_sat_f32_u(subnormal))
        << "Failed to convert subnormal number to 0ULL";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionModeValidation,
    I64TruncSatF32UTest,
    ::testing::Values(
        Mode_Interp
        // Note: AOT mode requires pre-compiled .aot files
        // Add Mode_LLVM_JIT when AOT compilation pipeline is available
    )
);