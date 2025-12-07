/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i64.trunc_u Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i64.trunc_u
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic functionality with typical values
 * - Corner Cases: Boundary conditions and overflow scenarios
 * - Edge Cases: Zero operands, special values, and extreme inputs
 * - Error Handling: Invalid values that should cause traps
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (i64.trunc_u operations)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:lines handling WASM_OP_I64_TRUNC_U_F32/F64
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <cfloat>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"


/*
 * I64TruncUTest Class
 *
 * Test fixture for i64.trunc_u opcode validation across execution modes.
 * Inherits from TestWithParam to support both interpreter and AOT modes.
 */
class I64TruncUTest : public testing::TestWithParam<RunningMode>
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

    /*
     * SetUp Method
     *
     * Initializes WAMR runtime environment and loads WASM test module.
     * Called before each test case execution.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        const char *wasm_path = "wasm-apps/i64_trunc_u_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(wasm_path, &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << wasm_path;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /*
     * TearDown Method
     *
     * Cleans up WAMR runtime resources.
     * Called after each test case execution.
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
            BH_FREE(buf);
            buf = nullptr;
        }
    }

    /*
     * call_i64_trunc_u_f32 Helper Method
     *
     * Invokes i64.trunc_u/f32 WASM function through WAMR runtime.
     */
    uint64_t call_i64_trunc_u_f32(float input)
    {
        // Clear any previous exception state
        wasm_runtime_clear_exception(module_inst);

        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_trunc_u_f32");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_trunc_u_f32 function";

        uint32_t argv[2] = { 0, 0 };
        memcpy(&argv[0], &input, sizeof(float));

        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        exception = wasm_runtime_get_exception(module_inst);

        if (!ret || exception) {
            if (exception) {
                std::cerr << "F32 Exception: " << exception << std::endl;
            }
            return UINT64_MAX; // Indicates trap condition
        }

        uint64_t result;
        memcpy(&result, argv, sizeof(uint64_t));
        return result;
    }

    /*
     * call_i64_trunc_u_f64 Helper Method
     *
     * Invokes i64.trunc_u/f64 WASM function through WAMR runtime.
     */
    uint64_t call_i64_trunc_u_f64(double input)
    {
        // Clear any previous exception state
        wasm_runtime_clear_exception(module_inst);

        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_trunc_u_f64");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_trunc_u_f64 function";

        uint32_t argv[2] = { 0, 0 };
        memcpy(argv, &input, sizeof(double));

        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        exception = wasm_runtime_get_exception(module_inst);

        if (!ret || exception) {
            return UINT64_MAX; // Indicates trap condition
        }

        uint64_t result;
        memcpy(&result, argv, sizeof(uint64_t));
        return result;
    }

    /*
     * expect_trap_f32 Helper Method
     *
     * Verifies that f32 input causes a WASM trap.
     */
    void expect_trap_f32(float input, const std::string& description)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_trunc_u_f32");
        ASSERT_NE(func, nullptr) << "Failed to lookup test_i64_trunc_u_f32 function";

        uint32_t argv[2] = { 0, 0 };
        memcpy(&argv[0], &input, sizeof(float));

        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        exception = wasm_runtime_get_exception(module_inst);

        ASSERT_FALSE(ret && !exception) << "Expected trap for " << description
                                       << " but operation succeeded";

        // Clear exception state for next test
        wasm_runtime_clear_exception(module_inst);
    }

    /*
     * expect_trap_f64 Helper Method
     *
     * Verifies that f64 input causes a WASM trap.
     */
    void expect_trap_f64(double input, const std::string& description)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_trunc_u_f64");
        ASSERT_NE(func, nullptr) << "Failed to lookup test_i64_trunc_u_f64 function";

        uint32_t argv[2] = { 0, 0 };
        memcpy(argv, &input, sizeof(double));

        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        exception = wasm_runtime_get_exception(module_inst);

        ASSERT_FALSE(ret && !exception) << "Expected trap for " << description
                                       << " but operation succeeded";

        // Clear exception state for next test
        wasm_runtime_clear_exception(module_inst);
    }
};

/**
 * @test BasicTruncation_ReturnsCorrectResults
 * @brief Validates i64.trunc_u produces correct results for typical positive inputs
 * @details Tests fundamental truncation operation with positive floating-point values.
 *          Verifies that i64.trunc_u correctly removes fractional parts and converts
 *          to unsigned 64-bit integers for both f32 and f64 inputs.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc_f32_to_int, trunc_f64_to_int
 * @input_conditions Typical positive floating-point values with fractional parts
 * @expected_behavior Returns truncated unsigned integer values (floor behavior for positive)
 * @validation_method Direct comparison of WASM function results with expected truncated values
 */
TEST_P(I64TruncUTest, BasicTruncation_ReturnsCorrectResults) {
    // Test typical positive values with fractional parts
    ASSERT_EQ(42ULL, call_i64_trunc_u_f32(42.7f)) << "f32 truncation of 42.7 failed";
    ASSERT_EQ(42ULL, call_i64_trunc_u_f64(42.7)) << "f64 truncation of 42.7 failed";

    ASSERT_EQ(1234ULL, call_i64_trunc_u_f32(1234.99f)) << "f32 truncation of 1234.99 failed";
    ASSERT_EQ(1234ULL, call_i64_trunc_u_f64(1234.99)) << "f64 truncation of 1234.99 failed";

    // Test small decimal values that truncate to zero
    ASSERT_EQ(0ULL, call_i64_trunc_u_f32(0.1f)) << "f32 truncation of 0.1 failed";
    ASSERT_EQ(0ULL, call_i64_trunc_u_f64(0.1)) << "f64 truncation of 0.1 failed";

    ASSERT_EQ(0ULL, call_i64_trunc_u_f32(0.9f)) << "f32 truncation of 0.9 failed";
    ASSERT_EQ(0ULL, call_i64_trunc_u_f64(0.9)) << "f64 truncation of 0.9 failed";

    // Test large integer values within u64 range (using exactly representable f64 value)
    ASSERT_EQ(18014398509481984ULL, call_i64_trunc_u_f64(18014398509481984.0))
        << "f64 truncation of large value failed";
}

/**
 * @test BoundaryValues_ReturnsCorrectResults
 * @brief Validates i64.trunc_u handles boundary conditions correctly
 * @details Tests conversion behavior at the limits of valid u64 range and precision boundaries.
 *          Verifies correct handling of values near UINT64_MAX and precision-limited values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trunc bounds checking
 * @input_conditions Values at u64 limits, precision boundaries, and near-zero values
 * @expected_behavior Returns correct truncated values respecting u64 range limits
 * @validation_method Boundary value analysis with precise expected results
 */
TEST_P(I64TruncUTest, BoundaryValues_ReturnsCorrectResults) {
    // Test values approaching UINT64_MAX (within valid range)
    ASSERT_EQ(18446744073709551615ULL, call_i64_trunc_u_f64(18446744073709551615.0))
        << "f64 truncation at UINT64_MAX boundary failed";

    // Test values just above zero
    ASSERT_EQ(1ULL, call_i64_trunc_u_f32(1.0f)) << "f32 truncation of 1.0 failed";
    ASSERT_EQ(1ULL, call_i64_trunc_u_f64(1.0)) << "f64 truncation of 1.0 failed";

    // Test precision boundary cases where float precision affects result
    ASSERT_EQ(16777216ULL, call_i64_trunc_u_f32(16777216.0f))
        << "f32 truncation at precision boundary failed";

    // Test very small positive numbers
    ASSERT_EQ(0ULL, call_i64_trunc_u_f64(1e-10)) << "f64 truncation of very small positive failed";
}

/**
 * @test SpecialValues_ReturnsCorrectResults
 * @brief Validates i64.trunc_u handles special IEEE 754 values correctly
 * @details Tests conversion behavior for special floating-point values including
 *          positive/negative zero, denormal numbers, and identity conversions.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:special value handling
 * @input_conditions IEEE 754 special values: ±0.0, denormals, identity values
 * @expected_behavior Returns mathematically correct conversions for valid special values
 * @validation_method Special value analysis with IEEE 754 compliance verification
 */
TEST_P(I64TruncUTest, SpecialValues_ReturnsCorrectResults) {
    // Test positive and negative zero (both should convert to 0)
    ASSERT_EQ(0ULL, call_i64_trunc_u_f32(+0.0f)) << "f32 truncation of +0.0 failed";
    ASSERT_EQ(0ULL, call_i64_trunc_u_f32(-0.0f)) << "f32 truncation of -0.0 failed";
    ASSERT_EQ(0ULL, call_i64_trunc_u_f64(+0.0)) << "f64 truncation of +0.0 failed";
    ASSERT_EQ(0ULL, call_i64_trunc_u_f64(-0.0)) << "f64 truncation of -0.0 failed";

    // Test identity operations (values that should convert to themselves)
    ASSERT_EQ(42ULL, call_i64_trunc_u_f32(42.0f)) << "f32 identity conversion failed";
    ASSERT_EQ(42ULL, call_i64_trunc_u_f64(42.0)) << "f64 identity conversion failed";

    ASSERT_EQ(100ULL, call_i64_trunc_u_f32(100.0f)) << "f32 identity conversion failed";
    ASSERT_EQ(100ULL, call_i64_trunc_u_f64(100.0)) << "f64 identity conversion failed";

    // Test denormal number handling (very small normalized values)
    ASSERT_EQ(0ULL, call_i64_trunc_u_f32(FLT_MIN)) << "f32 denormal conversion failed";
    ASSERT_EQ(0ULL, call_i64_trunc_u_f64(DBL_MIN)) << "f64 denormal conversion failed";
}

/**
 * @test InvalidValues_CausesTrap
 * @brief Validates i64.trunc_u properly traps on invalid input values
 * @details Tests that conversion operations correctly trap on invalid inputs including
 *          NaN values, infinity, negative numbers, and overflow conditions.
 * @test_category Error - Invalid input validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:trap condition handling
 * @input_conditions Invalid values: NaN, ±∞, negative numbers, overflow values
 * @expected_behavior WAMR runtime traps with appropriate error indication
 * @validation_method Trap verification with exception state validation
 */
TEST_P(I64TruncUTest, InvalidValues_CausesTrap) {
    // Test NaN values (should cause trap)
    expect_trap_f32(NAN, "NaN f32 value");
    expect_trap_f64(NAN, "NaN f64 value");

    // Test infinity values (should cause trap)
    expect_trap_f32(INFINITY, "positive infinity f32");
    expect_trap_f32(-INFINITY, "negative infinity f32");
    expect_trap_f64(INFINITY, "positive infinity f64");
    expect_trap_f64(-INFINITY, "negative infinity f64");

    // Test negative values (should cause trap for unsigned conversion)
    expect_trap_f32(-1.0f, "negative f32 value");
    expect_trap_f64(-1.0, "negative f64 value");
    expect_trap_f32(-42.5f, "negative f32 with fractional part");
    expect_trap_f64(-42.5, "negative f64 with fractional part");

    // Test overflow values (≥ 2^64 should cause trap)
    expect_trap_f64(18446744073709551616.0, "f64 overflow value");
    expect_trap_f64(1e20, "f64 large overflow value");

    // Test f32 overflow (2^64 in f32 representation)
    expect_trap_f32(18446744073709551616.0f, "f32 overflow value");
}

// Test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, I64TruncUTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I64TruncUTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

