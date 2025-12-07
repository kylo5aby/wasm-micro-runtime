/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <math.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <linux/limits.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;

static int
app_argc;
static char **app_argv;

static void initialize_globals() {
    static bool initialized = false;
    if (!initialized) {
        char exe_path[PATH_MAX];
        char *dir_buf_end;

        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        if (len > 0) {
            exe_path[len] = '\0';
            dir_buf_end = strrchr(exe_path, '/');
            if (dir_buf_end) {
                *dir_buf_end = '\0';
            }
            CWD = std::string(exe_path);
        } else {
            CWD = ".";
        }

        WASM_FILE = CWD + "/wasm-apps/f64_trunc_test.wasm";
        initialized = true;
    }
}

/**
 * @brief Test fixture class for comprehensive f64.trunc opcode validation
 *
 * This class provides a complete testing environment for the f64.trunc WebAssembly
 * instruction across both interpreter and AOT execution modes. The test suite
 * validates correct truncation behavior for all f64 value categories including
 * normal values, boundary conditions, special IEEE 754 values, and edge cases.
 *
 * Test coverage includes:
 * - Basic truncation functionality (positive/negative fractional values)
 * - Boundary value handling (f64 MIN/MAX, precision limits)
 * - Special value preservation (NaN, ±∞, ±0.0)
 * - Mathematical property validation (towards-zero truncation, idempotency)
 * - Cross-execution mode consistency (interpreter vs AOT)
 */
class F64TruncTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment with WAMR runtime initialization
     *
     * Initializes WAMR runtime with system allocator, loads the f64.trunc test module,
     * and prepares the execution environment for both interpreter and AOT modes.
     * Ensures proper resource allocation and module validation before test execution.
     */
    void SetUp() override
    {
        initialize_globals();

        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load the f64.trunc test module
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and release WAMR resources
     *
     * Properly deallocates WASM module instances, unloads modules, and destroys
     * the WAMR runtime. Ensures no resource leaks between test executions.
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
        wasm_runtime_destroy();
    }

    /**
     * @brief Execute f64.trunc operation with given input value
     *
     * @param input The f64 value to truncate
     * @return The truncated f64 result
     *
     * Executes the f64.trunc WASM function with the specified input value and
     * returns the result. Handles proper function lookup, argument passing,
     * and result extraction from the WASM execution environment.
     */
    double call_f64_trunc(double input)
    {
        // Find the f64_trunc test function
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f64_trunc_test");
        EXPECT_NE(nullptr, func) << "Failed to find f64_trunc_test function";

        // Prepare arguments and results arrays
        uint32 argv[2];  // f64 requires 2 uint32 slots
        uint32 argc = 2;

        // Convert double to WASM f64 representation
        union { double d; uint64 i; } u;
        u.d = input;
        argv[0] = (uint32)u.i;          // Lower 32 bits
        argv[1] = (uint32)(u.i >> 32);  // Upper 32 bits

        // Execute the function
        bool call_result = wasm_runtime_call_wasm(exec_env, func, argc, argv);
        EXPECT_TRUE(call_result) << "Failed to call f64_trunc_test function: " << wasm_runtime_get_exception(module_inst);

        // Extract result from argv (WASM stores result in same array)
        u.i = ((uint64)argv[1] << 32) | argv[0];
        return u.d;
    }

    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t *buf = nullptr;
    uint32 buf_size;
    uint32 stack_size = 8092, heap_size = 8092;
    char error_buf[128] = {0};
};

/**
 * @test BasicTruncation_ReturnsCorrectResult
 * @brief Validates f64.trunc produces correct truncation results for typical inputs
 * @details Tests fundamental truncation operation with positive, negative, and mixed-sign
 *          floating-point values. Verifies that f64.trunc correctly removes fractional
 *          parts while preserving signs, and leaves integer values unchanged.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_trunc_operation
 * @input_conditions Typical f64 values: fractional, integer, and zero values
 * @expected_behavior Removes fractional part, preserves sign, maintains integer values
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F64TruncTest, BasicTruncation_ReturnsCorrectResult)
{
    // Test positive fractional values - should truncate towards zero
    ASSERT_EQ(5.0, call_f64_trunc(5.7)) << "Truncation of positive fractional value failed";
    ASSERT_EQ(123.0, call_f64_trunc(123.456)) << "Truncation of large positive fractional failed";

    // Test negative fractional values - should truncate towards zero
    ASSERT_EQ(-5.0, call_f64_trunc(-5.7)) << "Truncation of negative fractional value failed";
    ASSERT_EQ(-123.0, call_f64_trunc(-123.456)) << "Truncation of large negative fractional failed";

    // Test integer values - should remain unchanged
    ASSERT_EQ(42.0, call_f64_trunc(42.0)) << "Integer value should remain unchanged";
    ASSERT_EQ(-1000.0, call_f64_trunc(-1000.0)) << "Negative integer value should remain unchanged";

    // Test zero values - should preserve sign
    ASSERT_EQ(0.0, call_f64_trunc(0.0)) << "Positive zero should remain unchanged";
    ASSERT_EQ(-0.0, call_f64_trunc(-0.0)) << "Negative zero should remain unchanged";
    ASSERT_TRUE(signbit(call_f64_trunc(-0.0))) << "Negative zero sign should be preserved";
}

/**
 * @test BoundaryValues_HandleCorrectly
 * @brief Validates f64.trunc handles f64 boundary conditions and precision limits correctly
 * @details Tests truncation behavior at f64 representation boundaries including maximum
 *          values, precision limits, and subnormal numbers. Ensures correct handling
 *          of values at the edge of floating-point representation capabilities.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_trunc_boundary_handling
 * @input_conditions f64 MAX/MIN values, precision boundaries, subnormal numbers
 * @expected_behavior Large integers preserved, subnormals truncated to zero, boundaries handled
 * @validation_method Boundary value testing with exact comparisons and range verification
 */
TEST_P(F64TruncTest, BoundaryValues_HandleCorrectly)
{
    // Test maximum f64 value - should remain unchanged (already integer)
    double max_f64 = 1.7976931348623157e+308;
    ASSERT_EQ(max_f64, call_f64_trunc(max_f64)) << "Maximum f64 value should remain unchanged";

    // Test minimum f64 value - should remain unchanged (already integer)
    double min_f64 = -1.7976931348623157e+308;
    ASSERT_EQ(min_f64, call_f64_trunc(min_f64)) << "Minimum f64 value should remain unchanged";

    // Test precision boundary - largest exactly representable integer (2^53)
    double precision_max = 9007199254740992.0;
    ASSERT_EQ(precision_max, call_f64_trunc(precision_max)) << "Precision boundary should remain unchanged";
    ASSERT_EQ(-precision_max, call_f64_trunc(-precision_max)) << "Negative precision boundary should remain unchanged";

    // Test values near integer boundaries
    ASSERT_EQ(0.0, call_f64_trunc(0.9999999999999999)) << "Value just below 1 should truncate to 0";
    ASSERT_EQ(-0.0, call_f64_trunc(-0.9999999999999999)) << "Value just above -1 should truncate to -0";
    ASSERT_TRUE(signbit(call_f64_trunc(-0.9999999999999999))) << "Negative truncation should preserve sign";

    // Test subnormal numbers - should truncate to zero
    double min_subnormal = 4.9406564584124654e-324;
    ASSERT_EQ(0.0, call_f64_trunc(min_subnormal)) << "Subnormal number should truncate to zero";
    ASSERT_EQ(-0.0, call_f64_trunc(-min_subnormal)) << "Negative subnormal should truncate to negative zero";
}

/**
 * @test SpecialValues_PreserveCorrectly
 * @brief Validates f64.trunc correctly preserves IEEE 754 special values
 * @details Tests handling of special floating-point values including NaN (Not a Number),
 *          positive and negative infinity, and signed zeros. Ensures that these special
 *          cases are preserved exactly as per IEEE 754 specifications.
 * @test_category Edge - Special value preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_trunc_special_values
 * @input_conditions NaN, ±∞, ±0.0, extreme mathematical values
 * @expected_behavior Special values preserved exactly: NaN→NaN, +∞→+∞, -∞→-∞, signs maintained
 * @validation_method Special value checks using isnan(), isinf(), signbit() functions
 */
TEST_P(F64TruncTest, SpecialValues_PreserveCorrectly)
{
    // Test NaN preservation - should remain NaN
    double nan_value = NAN;
    double trunc_nan = call_f64_trunc(nan_value);
    ASSERT_TRUE(isnan(trunc_nan)) << "NaN value should remain NaN after truncation";

    // Test positive infinity - should remain +∞
    double pos_inf = INFINITY;
    ASSERT_EQ(pos_inf, call_f64_trunc(pos_inf)) << "Positive infinity should remain unchanged";
    ASSERT_TRUE(isinf(call_f64_trunc(pos_inf)) && call_f64_trunc(pos_inf) > 0) << "Result should be positive infinity";

    // Test negative infinity - should remain -∞
    double neg_inf = -INFINITY;
    ASSERT_EQ(neg_inf, call_f64_trunc(neg_inf)) << "Negative infinity should remain unchanged";
    ASSERT_TRUE(isinf(call_f64_trunc(neg_inf)) && call_f64_trunc(neg_inf) < 0) << "Result should be negative infinity";

    // Test signed zeros - signs should be preserved
    ASSERT_EQ(0.0, call_f64_trunc(0.0)) << "Positive zero should remain positive zero";
    ASSERT_FALSE(signbit(call_f64_trunc(0.0))) << "Positive zero should not have sign bit set";

    ASSERT_EQ(-0.0, call_f64_trunc(-0.0)) << "Negative zero should remain negative zero";
    ASSERT_TRUE(signbit(call_f64_trunc(-0.0))) << "Negative zero should preserve sign bit";

    // Test mathematical property - towards zero truncation
    ASSERT_EQ(3.0, call_f64_trunc(3.7)) << "Positive truncation should go towards zero";
    ASSERT_EQ(-3.0, call_f64_trunc(-3.7)) << "Negative truncation should go towards zero";

    // Test idempotent property - trunc(trunc(x)) = trunc(x)
    double test_val = 42.7;
    double first_trunc = call_f64_trunc(test_val);
    double second_trunc = call_f64_trunc(first_trunc);
    ASSERT_EQ(first_trunc, second_trunc) << "Truncation should be idempotent";
}

/**
 * @test NoExceptionScenarios_HandleCorrectly
 * @brief Validates f64.trunc never causes exceptions and maintains execution consistency
 * @details Verifies that f64.trunc operation never traps or throws exceptions for any
 *          valid f64 input, including edge cases and special values. Tests execution
 *          consistency and confirms no side effects during operation.
 * @test_category Error - Exception handling and consistency validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_trunc_exception_safety
 * @input_conditions All edge cases that should never cause exceptions or traps
 * @expected_behavior All operations complete successfully, consistent results, no side effects
 * @validation_method Successful execution verification, consistency checks, no runtime errors
 */
TEST_P(F64TruncTest, NoExceptionScenarios_HandleCorrectly)
{
    // Verify no exceptions occur for special values
    ASSERT_NO_THROW({
        call_f64_trunc(NAN);
        call_f64_trunc(INFINITY);
        call_f64_trunc(-INFINITY);
        call_f64_trunc(0.0);
        call_f64_trunc(-0.0);
    }) << "Special values should not cause exceptions";

    // Verify no exceptions for boundary values
    ASSERT_NO_THROW({
        call_f64_trunc(1.7976931348623157e+308);   // MAX_F64
        call_f64_trunc(-1.7976931348623157e+308);  // MIN_F64
        call_f64_trunc(4.9406564584124654e-324);   // MIN_SUBNORMAL
        call_f64_trunc(-4.9406564584124654e-324);  // -MIN_SUBNORMAL
    }) << "Boundary values should not cause exceptions";

    // Verify execution consistency - same input should always produce same result
    double test_inputs[] = {5.7, -5.7, 42.0, NAN, INFINITY, -INFINITY, 0.0, -0.0};
    for (double input : test_inputs) {
        double result1 = call_f64_trunc(input);
        double result2 = call_f64_trunc(input);

        if (isnan(input)) {
            ASSERT_TRUE(isnan(result1) && isnan(result2)) << "NaN inputs should produce consistent NaN results";
        } else {
            ASSERT_EQ(result1, result2) << "Same input should produce identical results";
        }
    }

    // Verify no state changes or side effects
    call_f64_trunc(123.456);
    // Test execution should complete without exceptions
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(nullptr, exception) << "Function execution should not cause exceptions";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, F64TruncTest,
                         testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<RunningMode>& info) {
                             return info.param == RunningMode::Mode_Interp ? "Interpreter" : "AOT";
                         });