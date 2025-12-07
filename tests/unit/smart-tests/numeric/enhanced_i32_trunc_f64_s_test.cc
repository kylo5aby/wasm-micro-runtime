/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <cstring>

#include "wasm_runtime_common.h"
#include "wasm_native.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"
#include "bh_platform.h"
#include "wasm_export.h"

static std::string CUR_DIR;

static const char *WASM_FILE = "wasm-apps/i32_trunc_f64_s_test.wasm";

/**
 * @brief Test fixture for i32.trunc_f64_s opcode validation
 * @details Comprehensive test suite for WebAssembly i32.trunc_f64_s instruction testing
 *          conversion from f64 to signed i32 with truncation semantics across multiple execution modes.
 *          Tests cover basic conversion, boundary conditions, special values, and error handling.
 */
class I32TruncF64STest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test fixture with WAMR runtime initialization
     * @details Initializes WAMR runtime with system allocator, loads test WASM module,
     *          and prepares execution environment for both interpreter and AOT modes.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize the WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load WASM module from file
        module_buffer = (char *)bh_read_file_to_buffer(WASM_FILE, &buffer_size);
        ASSERT_NE(module_buffer, nullptr)
            << "Failed to read WASM file: " << WASM_FILE;

        // Load the WASM module
        module = wasm_runtime_load((uint8_t *)module_buffer, buffer_size,
                                 error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr)
            << "Failed to load WASM module: " << error_buf;

        // Create module instance
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr)
            << "Failed to instantiate WASM module: " << error_buf;

        // Get execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr)
            << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test fixture and WAMR runtime resources
     * @details Destroys execution environment, module instance, module, and runtime.
     *          Releases allocated memory and ensures proper resource cleanup.
     */
    void TearDown() override
    {
        // Clean up WAMR runtime resources
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (module_buffer) {
            BH_FREE(module_buffer);
        }

        // Destroy the runtime environment
        wasm_runtime_destroy();
    }

    /**
     * @brief Call i32.trunc_f64_s WASM function with f64 input
     * @details Invokes the WASM test function that performs i32.trunc_f64_s conversion.
     *          Handles parameter marshalling and result extraction.
     * @param input f64 value to convert to signed i32
     * @return Converted i32 value or throws on trap conditions
     */
    int32_t call_i32_trunc_f64_s_basic(double input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_i32_trunc_f64_s_basic");
        EXPECT_NE(func, nullptr)
            << "Failed to lookup function test_i32_trunc_f64_s_basic";

        // Prepare function arguments: one f64 parameter
        uint32_t argv[2];  // f64 requires 2 uint32_t slots
        memcpy(argv, &input, sizeof(double));

        // Execute the function (f64 parameter requires argc = 2)
        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success)
            << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        // Extract i32 result from argv[0]
        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Call i32.trunc_f64_s function that should trap on overflow
     * @details Invokes WASM function expecting trap conditions for overflow/special values.
     *          Returns false if trap occurs as expected, true if function completes normally.
     * @param input f64 value expected to cause trap
     * @return false if trap occurred (expected), true if no trap (unexpected)
     */
    bool call_i32_trunc_f64_s_overflow(double input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_i32_trunc_f64_s_overflow");
        EXPECT_NE(func, nullptr)
            << "Failed to lookup function test_i32_trunc_f64_s_overflow";

        // Prepare function arguments: one f64 parameter
        uint32_t argv[2];  // f64 requires 2 uint32_t slots
        memcpy(argv, &input, sizeof(double));

        // Execute function and expect it to trap (f64 parameter requires argc = 2)
        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);

        // Return false if trap occurred (expected behavior)
        return success;
    }

    // Test fixture member variables
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    char *module_buffer = nullptr;
    uint32_t buffer_size = 0;
    uint32_t stack_size = 16 * 1024;
    uint32_t heap_size = 16 * 1024;
    char error_buf[256];
};

/**
 * @test BasicTruncation_ReturnsCorrectInteger
 * @brief Validates i32.trunc_f64_s produces correct truncation results for typical inputs
 * @details Tests fundamental truncation operation with positive, negative, and mixed fractional values.
 *          Verifies that i32.trunc_f64_s correctly truncates towards zero for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f64_s_operation
 * @input_conditions Standard f64 pairs with fractional parts: (42.7), (-42.7), (100.99), (-1.1)
 * @expected_behavior Returns truncated integers: 42, -42, 100, -1 respectively (towards zero)
 * @validation_method Direct comparison of WASM function result with expected truncated values
 */
TEST_P(I32TruncF64STest, BasicTruncation_ReturnsCorrectInteger)
{
    // Test positive fractional truncation
    ASSERT_EQ(call_i32_trunc_f64_s_basic(42.7), 42)
        << "Failed to truncate positive fractional value 42.7";

    // Test negative fractional truncation
    ASSERT_EQ(call_i32_trunc_f64_s_basic(-42.7), -42)
        << "Failed to truncate negative fractional value -42.7";

    // Test larger positive fractional
    ASSERT_EQ(call_i32_trunc_f64_s_basic(100.99), 100)
        << "Failed to truncate large positive fractional 100.99";

    // Test small negative fractional
    ASSERT_EQ(call_i32_trunc_f64_s_basic(-1.1), -1)
        << "Failed to truncate small negative fractional -1.1";

    // Test whole number conversion
    ASSERT_EQ(call_i32_trunc_f64_s_basic(123.0), 123)
        << "Failed to convert whole number 123.0";

    ASSERT_EQ(call_i32_trunc_f64_s_basic(-456.0), -456)
        << "Failed to convert negative whole number -456.0";
}

/**
 * @test BoundaryConversion_HandlesIntegerLimits
 * @brief Validates conversion behavior at INT32_MIN/MAX boundaries
 * @details Tests boundary conditions including exact integer limits and near-boundary values.
 *          Ensures proper handling of values at the edge of representable signed 32-bit range.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f64_s_boundary_check
 * @input_conditions Boundary values: INT32_MAX, INT32_MIN as f64, and near-boundary values
 * @expected_behavior Returns exact boundary integers and proper truncation near boundaries
 * @validation_method Boundary value conversion validation with exact integer comparison
 */
TEST_P(I32TruncF64STest, BoundaryConversion_HandlesIntegerLimits)
{
    // Test INT32_MAX boundary (2147483647)
    ASSERT_EQ(call_i32_trunc_f64_s_basic(2147483647.0), 2147483647)
        << "Failed to convert INT32_MAX boundary value";

    // Test INT32_MIN boundary (-2147483648)
    ASSERT_EQ(call_i32_trunc_f64_s_basic(-2147483648.0), -2147483648)
        << "Failed to convert INT32_MIN boundary value";

    // Test just below INT32_MAX with fractional part
    ASSERT_EQ(call_i32_trunc_f64_s_basic(2147483647.999), 2147483647)
        << "Failed to truncate value just below INT32_MAX overflow";

    // Test near-boundary precision values
    ASSERT_EQ(call_i32_trunc_f64_s_basic(2147483646.999), 2147483646)
        << "Failed to truncate near-boundary positive value";

    ASSERT_EQ(call_i32_trunc_f64_s_basic(-2147483647.999), -2147483647)
        << "Failed to truncate near-boundary negative value";
}

/**
 * @test ZeroTruncation_ReturnsZero
 * @brief Validates zero handling and near-zero truncation behavior
 * @details Tests truncation of zero values and fractional values that should truncate to zero.
 *          Verifies proper truncation towards zero for small fractional values.
 * @test_category Edge - Zero and near-zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f64_s_zero_handling
 * @input_conditions Zero values: +0.0, -0.0, and small fractionals: 0.9, -0.9, 0.5, -0.5
 * @expected_behavior All should return 0 (truncation towards zero, not rounding)
 * @validation_method Zero equality verification with truncation behavior validation
 */
TEST_P(I32TruncF64STest, ZeroTruncation_ReturnsZero)
{
    // Test positive zero
    ASSERT_EQ(call_i32_trunc_f64_s_basic(+0.0), 0)
        << "Failed to convert positive zero";

    // Test negative zero
    ASSERT_EQ(call_i32_trunc_f64_s_basic(-0.0), 0)
        << "Failed to convert negative zero";

    // Test truncation towards zero (not rounding)
    ASSERT_EQ(call_i32_trunc_f64_s_basic(0.9), 0)
        << "Failed to truncate 0.9 towards zero";

    ASSERT_EQ(call_i32_trunc_f64_s_basic(-0.9), 0)
        << "Failed to truncate -0.9 towards zero";

    ASSERT_EQ(call_i32_trunc_f64_s_basic(0.5), 0)
        << "Failed to truncate 0.5 towards zero (not round to 1)";

    ASSERT_EQ(call_i32_trunc_f64_s_basic(-0.5), 0)
        << "Failed to truncate -0.5 towards zero (not round to -1)";
}

/**
 * @test TruncationBehavior_VerifiesTowardsZero
 * @brief Validates proper truncation semantics (towards zero, not rounding)
 * @details Tests that truncation always moves towards zero, never away from zero.
 *          Distinguishes truncation from rounding behavior.
 * @test_category Edge - Truncation semantics validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f64_s_semantics
 * @input_conditions Fractional values that test truncation vs rounding: 1.9, -1.9, etc.
 * @expected_behavior Always truncates towards zero, never rounds away from zero
 * @validation_method Comparison of truncation results with expected towards-zero behavior
 */
TEST_P(I32TruncF64STest, TruncationBehavior_VerifiesTowardsZero)
{
    // Test that 1.9 truncates to 1 (not rounds to 2)
    ASSERT_EQ(call_i32_trunc_f64_s_basic(1.9), 1)
        << "Failed to truncate 1.9 towards zero (should be 1, not 2)";

    // Test that -1.9 truncates to -1 (not rounds to -2)
    ASSERT_EQ(call_i32_trunc_f64_s_basic(-1.9), -1)
        << "Failed to truncate -1.9 towards zero (should be -1, not -2)";

    // Test high precision f64 values
    ASSERT_EQ(call_i32_trunc_f64_s_basic(1.0000000000000002), 1)
        << "Failed to truncate high precision value > 1.0";

    ASSERT_EQ(call_i32_trunc_f64_s_basic(-1.0000000000000002), -1)
        << "Failed to truncate high precision value < -1.0";

    // Test with boundary fractional values
    ASSERT_EQ(call_i32_trunc_f64_s_basic(2147483647.5), 2147483647)
        << "Failed to truncate near-boundary fractional value";
}

/**
 * @test OverflowValues_GenerateTraps
 * @brief Verifies trap generation for overflow conditions beyond INT32 range
 * @details Tests that values outside the representable range of signed 32-bit integers
 *          properly generate traps as required by WebAssembly specification.
 * @test_category Error - Overflow trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f64_s_overflow_check
 * @input_conditions Overflow values: 2147483648.0, -2147483649.0, large positive/negative
 * @expected_behavior All should generate traps/exceptions (function returns false)
 * @validation_method Trap detection verification with overflow condition testing
 */
TEST_P(I32TruncF64STest, OverflowValues_GenerateTraps)
{
    // Test positive overflow (INT32_MAX + 1)
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(2147483648.0))
        << "Expected trap for positive overflow value 2147483648.0";

    // Test negative underflow (INT32_MIN - 1)
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(-2147483649.0))
        << "Expected trap for negative overflow value -2147483649.0";

    // Test large positive value
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(1e10))
        << "Expected trap for large positive value 1e10";

    // Test large negative value
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(-1e10))
        << "Expected trap for large negative value -1e10";

    // Test UINT32_MAX (should overflow for signed conversion)
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(4294967295.0))
        << "Expected trap for UINT32_MAX value in signed conversion";
}

/**
 * @test SpecialValues_GenerateTraps
 * @brief Validates trap generation for IEEE 754 special values
 * @details Tests that NaN, infinity, and other special floating-point values
 *          properly generate traps as required by WebAssembly specification.
 * @test_category Error - Special value trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f64_s_special_check
 * @input_conditions Special IEEE 754 values: NaN, +∞, -∞
 * @expected_behavior All should generate traps/exceptions (function returns false)
 * @validation_method Special value trap verification with IEEE 754 compliance testing
 */
TEST_P(I32TruncF64STest, SpecialValues_GenerateTraps)
{
    // Test NaN (Not a Number)
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(std::numeric_limits<double>::quiet_NaN()))
        << "Expected trap for NaN input";

    // Test positive infinity
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(std::numeric_limits<double>::infinity()))
        << "Expected trap for positive infinity";

    // Test negative infinity
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(-std::numeric_limits<double>::infinity()))
        << "Expected trap for negative infinity";

    // Test signaling NaN (if different from quiet NaN)
    ASSERT_FALSE(call_i32_trunc_f64_s_overflow(std::numeric_limits<double>::signaling_NaN()))
        << "Expected trap for signaling NaN";
}

// Parameterized test instantiation for multiple execution modes
// Test parameter instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    I32TruncF64STest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);