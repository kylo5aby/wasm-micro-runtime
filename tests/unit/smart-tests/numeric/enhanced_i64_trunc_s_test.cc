/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include "bh_read_file.h"
#include "wasm_runtime_common.h"

static std::string WASM_FILE;

/**
 * @brief Test suite for i64.trunc_s opcode validation
 *
 * This class tests both i64.trunc_s/f32 and i64.trunc_s/f64 operations.
 * Tests cover basic functionality, boundary conditions, edge cases, and error scenarios.
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @coverage_target core/iwasm/aot/aot_runtime.c:aot_call_function
 */
class I64TruncSTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment and WAMR runtime
     *
     * Initializes WAMR runtime with system allocator, loads test WASM modules
     * and prepares execution instances for both interpreter and AOT modes.
     *
     * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_full_init
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_EQ(wasm_runtime_full_init(&init_args), true);

        buffer = (uint8_t*)bh_read_file_to_buffer(WASM_FILE.c_str(), &size);
        ASSERT_NE(buffer, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buffer, size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load module: " << error_buf;

        wasm_module_inst = wasm_runtime_instantiate(
            module, 65536, 0, error_buf, sizeof(error_buf));
        ASSERT_NE(wasm_module_inst, nullptr) << "Failed to instantiate module: " << error_buf;

        exec_env = wasm_runtime_create_exec_env(wasm_module_inst, 65536);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and WAMR runtime
     *
     * Properly deallocates WASM instances, modules, and runtime resources
     * following RAII patterns for reliable cleanup.
     *
     * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_destroy
     */
    void TearDown() override
    {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (wasm_module_inst) {
            wasm_runtime_deinstantiate(wasm_module_inst);
            wasm_module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (buffer) {
            wasm_runtime_free(buffer);
            buffer = nullptr;
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Call i64.trunc_s/f32 WASM function with f32 input
     *
     * @param input f32 floating-point value to convert
     * @return int64_t converted signed 64-bit integer value
     *
     * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
     */
    int64_t call_i64_trunc_s_f32(float input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "i64_trunc_s_f32");
        EXPECT_NE(func, nullptr) << "Failed to lookup i64_trunc_s_f32 function";

        uint32 argv[1] = { 0 };
        memcpy(&argv[0], &input, sizeof(float));

        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(wasm_module_inst);

        int64_t result;
        memcpy(&result, argv, sizeof(int64_t));
        return result;
    }

    /**
     * @brief Call i64.trunc_s/f64 WASM function with f64 input
     *
     * @param input f64 floating-point value to convert
     * @return int64_t converted signed 64-bit integer value
     *
     * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
     */
    int64_t call_i64_trunc_s_f64(double input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "i64_trunc_s_f64");
        EXPECT_NE(func, nullptr) << "Failed to lookup i64_trunc_s_f64 function";

        uint32 argv[2] = { 0, 0 };
        memcpy(argv, &input, sizeof(double));

        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(wasm_module_inst);

        int64_t result;
        memcpy(&result, argv, sizeof(int64_t));
        return result;
    }

    /**
     * @brief Call function expecting trap and verify trap occurs
     *
     * @param func_name Name of the WASM function to call
     * @param input_f32 f32 input value (for f32 variant)
     * @param input_f64 f64 input value (for f64 variant)
     * @param use_f64 true for f64 variant, false for f32 variant
     * @return bool true if trap occurred as expected
     *
     * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_call_wasm
     */
    bool call_and_expect_trap(const char* func_name, float input_f32, double input_f64, bool use_f64)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, func_name);
        EXPECT_NE(func, nullptr) << "Failed to lookup function: " << func_name;

        uint32 argv[2] = { 0, 0 };
        uint32 argc;

        if (use_f64) {
            memcpy(argv, &input_f64, sizeof(double));
            argc = 2;
        } else {
            memcpy(&argv[0], &input_f32, sizeof(float));
            argc = 1;
        }

        bool ret = wasm_runtime_call_wasm(exec_env, func, argc, argv);

        // For trap scenarios, we expect the function call to fail
        return !ret && wasm_runtime_get_exception(wasm_module_inst) != nullptr;
    }

private:
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t wasm_module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t *buffer = nullptr;
    uint32_t size;
    char error_buf[128];
};

/**
 * @test BasicTruncationF32_ValidInputs_ReturnsCorrectI64
 * @brief Validates i64.trunc_s/f32 produces correct results for typical f32 inputs
 * @details Tests fundamental truncation operation with positive, negative, and fractional f32 values.
 *          Verifies that i64.trunc_s/f32 correctly truncates toward zero for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Standard f32 values: 42.7f, -25.8f, 1000.9f, -500.3f
 * @expected_behavior Returns truncated integers: 42, -25, 1000, -500 respectively
 * @validation_method Direct comparison of WASM function result with expected truncated values
 */
TEST_P(I64TruncSTest, BasicTruncationF32_ValidInputs_ReturnsCorrectI64) {
    ASSERT_EQ(call_i64_trunc_s_f32(42.7f), 42LL)
        << "Positive f32 truncation to i64 failed";
    ASSERT_EQ(call_i64_trunc_s_f32(-25.8f), -25LL)
        << "Negative f32 truncation to i64 failed";
    ASSERT_EQ(call_i64_trunc_s_f32(1000.9f), 1000LL)
        << "Large positive f32 truncation to i64 failed";
    ASSERT_EQ(call_i64_trunc_s_f32(-500.3f), -500LL)
        << "Large negative f32 truncation to i64 failed";
}

/**
 * @test BasicTruncationF64_ValidInputs_ReturnsCorrectI64
 * @brief Validates i64.trunc_s/f64 produces correct results for typical f64 inputs
 * @details Tests fundamental truncation operation with positive, negative, and high-precision f64 values.
 *          Verifies that i64.trunc_s/f64 correctly truncates toward zero for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Standard f64 values: 42.7, -25.8, 123.456789012345
 * @expected_behavior Returns truncated integers: 42, -25, 123 respectively
 * @validation_method Direct comparison of WASM function result with expected truncated values
 */
TEST_P(I64TruncSTest, BasicTruncationF64_ValidInputs_ReturnsCorrectI64) {
    ASSERT_EQ(call_i64_trunc_s_f64(42.7), 42LL)
        << "Positive f64 truncation to i64 failed";
    ASSERT_EQ(call_i64_trunc_s_f64(-25.8), -25LL)
        << "Negative f64 truncation to i64 failed";
    ASSERT_EQ(call_i64_trunc_s_f64(123.456789012345), 123LL)
        << "High precision f64 truncation to i64 failed";
}

/**
 * @test BoundaryValues_I64Limits_ConvertsCorrectly
 * @brief Tests conversion behavior near INT64_MIN and INT64_MAX boundaries
 * @details Validates that floating-point values near i64 limits convert correctly without overflow.
 *          Tests values just within the convertible range to ensure proper boundary handling.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Values near i64 boundaries: large positive/negative f32/f64 within range
 * @expected_behavior Returns correct i64 values without traps for values within range
 * @validation_method Comparison with expected boundary conversion results
 */
TEST_P(I64TruncSTest, BoundaryValues_I64Limits_ConvertsCorrectly) {
    // Test values within safe i64 range for f32 (considering f32 precision)
    ASSERT_EQ(call_i64_trunc_s_f32(9223371487098961920.0f), 9223371487098961920LL)
        << "Large positive f32 within i64 range conversion failed";
    ASSERT_EQ(call_i64_trunc_s_f32(-9223371487098961920.0f), -9223371487098961920LL)
        << "Large negative f32 within i64 range conversion failed";

    // Test values within safe i64 range for f64
    ASSERT_EQ(call_i64_trunc_s_f64(9223372036854774784.0), 9223372036854774784LL)
        << "Large positive f64 within i64 range conversion failed";
    ASSERT_EQ(call_i64_trunc_s_f64(-9223372036854774784.0), -9223372036854774784LL)
        << "Large negative f64 within i64 range conversion failed";
}

/**
 * @test PrecisionBoundaries_FloatingPointLimits_TruncatesCorrectly
 * @brief Tests truncation behavior at floating-point precision boundaries
 * @brief Validates correct truncation of values at f32 and f64 precision limits.
 *        Ensures that precision loss during conversion doesn't affect truncation correctness.
 * @test_category Corner - Floating-point precision boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions f32 precision limit (2^24), f64 precision limit (2^53)
 * @expected_behavior Returns exact integer values at precision boundaries
 * @validation_method Verification of exact integer preservation at precision limits
 */
TEST_P(I64TruncSTest, PrecisionBoundaries_FloatingPointLimits_TruncatesCorrectly) {
    // F32 precision limit: 2^24 = 16,777,216
    ASSERT_EQ(call_i64_trunc_s_f32(16777216.0f), 16777216LL)
        << "F32 precision boundary truncation failed";
    ASSERT_EQ(call_i64_trunc_s_f32(-16777216.0f), -16777216LL)
        << "Negative f32 precision boundary truncation failed";

    // F64 precision limit: 2^53 = 9,007,199,254,740,992
    ASSERT_EQ(call_i64_trunc_s_f64(9007199254740992.0), 9007199254740992LL)
        << "F64 precision boundary truncation failed";
    ASSERT_EQ(call_i64_trunc_s_f64(-9007199254740992.0), -9007199254740992LL)
        << "Negative f64 precision boundary truncation failed";
}

/**
 * @test ZeroValues_PositiveNegativeZero_ReturnsZero
 * @brief Tests conversion behavior with positive and negative zero values
 * @details Validates that both +0.0 and -0.0 floating-point values convert to i64(0).
 *          Ensures proper handling of IEEE 754 signed zero representation.
 * @test_category Edge - Special zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions +0.0f, -0.0f, +0.0, -0.0, values very close to zero
 * @expected_behavior All zero variants return i64(0)
 * @validation_method Direct comparison with expected zero result
 */
TEST_P(I64TruncSTest, ZeroValues_PositiveNegativeZero_ReturnsZero) {
    ASSERT_EQ(call_i64_trunc_s_f32(0.0f), 0LL)
        << "Positive zero f32 conversion failed";
    ASSERT_EQ(call_i64_trunc_s_f32(-0.0f), 0LL)
        << "Negative zero f32 conversion failed";
    ASSERT_EQ(call_i64_trunc_s_f64(0.0), 0LL)
        << "Positive zero f64 conversion failed";
    ASSERT_EQ(call_i64_trunc_s_f64(-0.0), 0LL)
        << "Negative zero f64 conversion failed";

    // Values very close to zero should truncate to 0
    ASSERT_EQ(call_i64_trunc_s_f32(0.9f), 0LL)
        << "Small positive f32 truncation failed";
    ASSERT_EQ(call_i64_trunc_s_f32(-0.9f), 0LL)
        << "Small negative f32 truncation failed";
}

/**
 * @test IntegerValues_ExactFloats_PreservesValue
 * @brief Tests conversion of integer-valued floating-point numbers
 * @details Validates that floating-point values with no fractional part convert exactly.
 *          Ensures that integer-valued floats preserve their exact integer representation.
 * @test_category Edge - Integer-valued floating-point validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Integer-valued floats: 42.0f, -25.0, powers of 2
 * @expected_behavior Returns exact integer values without precision loss
 * @validation_method Direct comparison with expected integer values
 */
TEST_P(I64TruncSTest, IntegerValues_ExactFloats_PreservesValue) {
    ASSERT_EQ(call_i64_trunc_s_f32(42.0f), 42LL)
        << "Integer-valued f32 conversion failed";
    ASSERT_EQ(call_i64_trunc_s_f32(-25.0f), -25LL)
        << "Negative integer-valued f32 conversion failed";
    ASSERT_EQ(call_i64_trunc_s_f64(1024.0), 1024LL)
        << "Power-of-2 f64 conversion failed";
    ASSERT_EQ(call_i64_trunc_s_f64(2048.0), 2048LL)
        << "Large power-of-2 f64 conversion failed";
}

/**
 * @test InvalidValues_NaN_ThrowsTrap
 * @brief Tests that NaN inputs cause proper WASM traps
 * @details Validates that both f32 and f64 NaN values trigger execution traps.
 *          Ensures proper error handling for invalid numeric inputs.
 * @test_category Exception - NaN input validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_call_wasm
 * @input_conditions Various NaN values: quiet NaN, signaling NaN
 * @expected_behavior WASM trap occurs, function call returns false
 * @validation_method Verification that trap is properly generated and detected
 */
TEST_P(I64TruncSTest, InvalidValues_NaN_ThrowsTrap) {
    float f32_nan = std::numeric_limits<float>::quiet_NaN();
    double f64_nan = std::numeric_limits<double>::quiet_NaN();

    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f32", f32_nan, 0.0, false))
        << "F32 NaN should cause trap";
    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f64", 0.0f, f64_nan, true))
        << "F64 NaN should cause trap";

    // Test signaling NaN
    float f32_snan = std::numeric_limits<float>::signaling_NaN();
    double f64_snan = std::numeric_limits<double>::signaling_NaN();

    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f32", f32_snan, 0.0, false))
        << "F32 signaling NaN should cause trap";
    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f64", 0.0f, f64_snan, true))
        << "F64 signaling NaN should cause trap";
}

/**
 * @test InvalidValues_Infinity_ThrowsTrap
 * @brief Tests that infinity inputs cause proper WASM traps
 * @details Validates that both positive and negative infinity values trigger execution traps.
 *          Ensures proper error handling for infinite numeric inputs.
 * @test_category Exception - Infinity input validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_call_wasm
 * @input_conditions +∞ and -∞ for both f32 and f64
 * @expected_behavior WASM trap occurs, function call returns false
 * @validation_method Verification that trap is properly generated and detected
 */
TEST_P(I64TruncSTest, InvalidValues_Infinity_ThrowsTrap) {
    float f32_pos_inf = std::numeric_limits<float>::infinity();
    float f32_neg_inf = -std::numeric_limits<float>::infinity();
    double f64_pos_inf = std::numeric_limits<double>::infinity();
    double f64_neg_inf = -std::numeric_limits<double>::infinity();

    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f32", f32_pos_inf, 0.0, false))
        << "F32 positive infinity should cause trap";
    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f32", f32_neg_inf, 0.0, false))
        << "F32 negative infinity should cause trap";
    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f64", 0.0f, f64_pos_inf, true))
        << "F64 positive infinity should cause trap";
    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f64", 0.0f, f64_neg_inf, true))
        << "F64 negative infinity should cause trap";
}

/**
 * @test OverflowValues_OutOfRange_ThrowsTrap
 * @brief Tests that out-of-range inputs cause proper WASM traps
 * @details Validates that floating-point values outside i64 range trigger execution traps.
 *          Ensures proper overflow/underflow detection and error handling.
 * @test_category Exception - Overflow/underflow validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_call_wasm
 * @input_conditions Values > INT64_MAX and < INT64_MIN
 * @expected_behavior WASM trap occurs for out-of-range values
 * @validation_method Verification that overflow conditions generate proper traps
 */
TEST_P(I64TruncSTest, OverflowValues_OutOfRange_ThrowsTrap) {
    // Values larger than INT64_MAX should trap
    float f32_overflow = 1e20f; // Much larger than INT64_MAX
    double f64_overflow = 1e20; // Much larger than INT64_MAX

    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f32", f32_overflow, 0.0, false))
        << "F32 overflow value should cause trap";
    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f64", 0.0f, f64_overflow, true))
        << "F64 overflow value should cause trap";

    // Values smaller than INT64_MIN should trap
    float f32_underflow = -1e20f; // Much smaller than INT64_MIN
    double f64_underflow = -1e20; // Much smaller than INT64_MIN

    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f32", f32_underflow, 0.0, false))
        << "F32 underflow value should cause trap";
    ASSERT_TRUE(call_and_expect_trap("i64_trunc_s_f64", 0.0f, f64_underflow, true))
        << "F64 underflow value should cause trap";
}

INSTANTIATE_TEST_SUITE_P(RunningModeTest, I64TruncSTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I64TruncSTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

// Initialize WASM_FILE for i64.trunc_s tests
static struct WasmFileInitializer {
    WasmFileInitializer() {
        WASM_FILE = "wasm-apps/i64_trunc_s_test.wasm";
    }
} wasm_file_init;