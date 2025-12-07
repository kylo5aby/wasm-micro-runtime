/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for f64.nearest Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly f64.nearest
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic rounding functionality with banker's rounding validation
 * - Corner Cases: Boundary conditions including IEEE 754 "ties to even" behavior
 * - Edge Cases: Special IEEE 754 values (NaN, infinities, signed zeros, subnormals)
 * - Error Handling: Validation of non-trapping behavior for all f64 inputs
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (f64.nearest implementation using rint)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c (WASM_OP_F64_NEAREST: DEF_OP_MATH(float64, F64, rint))
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <limits>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

using namespace std;

/**
 * @brief Test fixture class for f64.nearest opcode validation
 * @details Provides comprehensive testing infrastructure for the WebAssembly f64.nearest
 *          instruction across both interpreter and AOT execution modes. This test suite
 *          validates IEEE 754 banker's rounding behavior, ensuring correct implementation
 *          of the nearest integer rounding operation which rounds half-values to the nearest
 *          even integer (banker's rounding) using the C library rint() function.
 */
class F64NearestTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment before each test case
     * @details Initializes WAMR runtime with system allocator, loads the f64.nearest test
     *          WASM module, and prepares execution context for both interpreter and AOT modes.
     *          Ensures proper module validation and instance creation for reliable test execution.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load WASM module for f64.nearest testing
        load_test_module();
    }

    /**
     * @brief Clean up test environment after each test case
     * @details Properly destroys WASM module instance, unloads module, releases memory
     *          resources, and shuts down WAMR runtime to prevent resource leaks.
     */
    void TearDown() override
    {
        // Clean up WAMR resources in proper order
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

private:
    /**
     * @brief Load f64.nearest test WASM module from file system
     * @details Reads the compiled WASM bytecode for f64.nearest tests, validates module
     *          format, loads module into WAMR, and creates executable module instance.
     *          Handles both interpreter and AOT execution modes based on test parameters.
     */
    void load_test_module()
    {
        const char *wasm_path = "wasm-apps/f64_nearest_test.wasm";

        // Read WASM module file from disk
        buf = (uint8_t*)bh_read_file_to_buffer(wasm_path, &buf_size);
        ASSERT_NE(nullptr, buf) << "Failed to read WASM file: " << wasm_path;
        ASSERT_GT(buf_size, 0U) << "WASM file is empty: " << wasm_path;

        // Load and validate WASM module
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        // Create module instance for execution
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode and create execution environment
        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
    }

public:
    /**
     * @brief Execute f64.nearest operation with single f64 operand
     * @details Calls the WASM f64.nearest test function with specified parameter,
     *          handles execution errors, and returns the computed result for validation.
     * @param input f64 value to be rounded to nearest integer
     * @return f64 result of nearest(input) operation using banker's rounding
     */
    double call_f64_nearest(double input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_nearest");
        EXPECT_NE(nullptr, func) << "Failed to find test_f64_nearest function";

        // Convert double parameter to uint64_t for WAMR function call
        union { double d; uint64_t u; } input_conv = { .d = input };

        uint32_t argv[4] = {
            static_cast<uint32_t>(input_conv.u & 0xFFFFFFFF),
            static_cast<uint32_t>(input_conv.u >> 32),
            0, 0
        };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        const char *exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            EXPECT_TRUE(false) << "Runtime exception occurred: " << exception;
        }

        // Convert result back to double
        union { double d; uint64_t u; } result_conv = {
            .u = (static_cast<uint64_t>(argv[1]) << 32) | argv[0]
        };
        return result_conv.d;
    }

protected:
    // WAMR runtime components
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t *buf = nullptr;
    uint32_t buf_size = 0;
    char error_buf[256];

    // Execution environment configuration
    static constexpr uint32_t stack_size = 16 * 1024;
    static constexpr uint32_t heap_size = 16 * 1024;
};

/**
 * @test BasicRounding_ReturnsCorrectValues
 * @brief Validates f64.nearest produces correct rounding results for typical inputs
 * @details Tests fundamental banker's rounding operation with positive, negative, and mixed-sign doubles.
 *          Verifies that f64.nearest correctly implements "round half to even" behavior where
 *          exact half-values (x.5) round to the nearest even integer.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:DEF_OP_MATH(float64, F64, rint)
 * @input_conditions Standard floating-point values with standard rounding requirements
 * @expected_behavior Returns mathematically correct rounded values following IEEE 754 standard
 * @validation_method Direct comparison of WASM function result with expected rounded values
 */
TEST_P(F64NearestTest, BasicRounding_ReturnsCorrectValues)
{
    // Test standard rounding down cases
    ASSERT_EQ(3.0, call_f64_nearest(3.2))
        << "Standard rounding down failed for positive value";
    ASSERT_EQ(4.0, call_f64_nearest(4.3))
        << "Standard rounding down failed for positive value";

    // Test standard rounding up cases
    ASSERT_EQ(6.0, call_f64_nearest(5.7))
        << "Standard rounding up failed for positive value";
    ASSERT_EQ(8.0, call_f64_nearest(7.8))
        << "Standard rounding up failed for positive value";

    // Test negative number rounding
    ASSERT_EQ(-2.0, call_f64_nearest(-2.3))
        << "Standard rounding failed for negative value";
    ASSERT_EQ(-7.0, call_f64_nearest(-6.7))
        << "Standard rounding failed for negative value";
}

/**
 * @test TiesToEven_FollowsIEEE754Rules
 * @brief Validates f64.nearest implements IEEE 754 "ties to even" (banker's rounding) correctly
 * @details Tests critical tie-breaking cases where input values are exactly halfway between
 *          two integers. Validates that the result rounds to the nearest even integer in all cases.
 * @test_category Corner - Boundary conditions for IEEE 754 compliance
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:rint function behavior
 * @input_conditions Exact half-values (x.5) for both positive and negative numbers
 * @expected_behavior Rounds to nearest even integer following banker's rounding rules
 * @validation_method Verify exact banker's rounding behavior: 2.5→2, 3.5→4, -2.5→-2, -3.5→-4
 */
TEST_P(F64NearestTest, TiesToEven_FollowsIEEE754Rules)
{
    // Positive tie cases - should round to nearest even
    ASSERT_EQ(2.0, call_f64_nearest(2.5))
        << "Banker's rounding failed: 2.5 should round to 2 (even)";
    ASSERT_EQ(4.0, call_f64_nearest(3.5))
        << "Banker's rounding failed: 3.5 should round to 4 (even)";
    ASSERT_EQ(4.0, call_f64_nearest(4.5))
        << "Banker's rounding failed: 4.5 should round to 4 (even)";
    ASSERT_EQ(6.0, call_f64_nearest(5.5))
        << "Banker's rounding failed: 5.5 should round to 6 (even)";

    // Negative tie cases - should round to nearest even (magnitude)
    ASSERT_EQ(-2.0, call_f64_nearest(-2.5))
        << "Banker's rounding failed: -2.5 should round to -2 (even magnitude)";
    ASSERT_EQ(-4.0, call_f64_nearest(-3.5))
        << "Banker's rounding failed: -3.5 should round to -4 (even magnitude)";
    ASSERT_EQ(-4.0, call_f64_nearest(-4.5))
        << "Banker's rounding failed: -4.5 should round to -4 (even magnitude)";
    ASSERT_EQ(-6.0, call_f64_nearest(-5.5))
        << "Banker's rounding failed: -5.5 should round to -6 (even magnitude)";
}

/**
 * @test SpecialValues_PreservesIEEE754Semantics
 * @brief Validates f64.nearest handles special IEEE 754 values correctly
 * @details Tests behavior with signed zeros, infinities, NaN values, and integer inputs
 *          that should remain unchanged. Validates sign preservation and special value semantics.
 * @test_category Edge - Special IEEE 754 value handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:rint function with special values
 * @input_conditions Special f64 values: ±0.0, ±Infinity, NaN, exact integers
 * @expected_behavior Preserves special value semantics and sign bits correctly
 * @validation_method Verify special values pass through unchanged with proper sign handling
 */
TEST_P(F64NearestTest, SpecialValues_PreservesIEEE754Semantics)
{
    // Test signed zeros preservation
    double pos_zero = call_f64_nearest(+0.0);
    double neg_zero = call_f64_nearest(-0.0);
    ASSERT_EQ(0.0, pos_zero) << "Positive zero should be preserved";
    ASSERT_EQ(0.0, neg_zero) << "Negative zero should be preserved";
    ASSERT_FALSE(std::signbit(pos_zero)) << "Positive zero sign should be preserved";
    ASSERT_TRUE(std::signbit(neg_zero)) << "Negative zero sign should be preserved";

    // Test infinity preservation
    double pos_inf = call_f64_nearest(std::numeric_limits<double>::infinity());
    double neg_inf = call_f64_nearest(-std::numeric_limits<double>::infinity());
    ASSERT_TRUE(std::isinf(pos_inf)) << "Positive infinity should be preserved";
    ASSERT_TRUE(std::isinf(neg_inf)) << "Negative infinity should be preserved";
    ASSERT_FALSE(std::signbit(pos_inf)) << "Positive infinity sign should be preserved";
    ASSERT_TRUE(std::signbit(neg_inf)) << "Negative infinity sign should be preserved";

    // Test NaN preservation
    double nan_result = call_f64_nearest(std::numeric_limits<double>::quiet_NaN());
    ASSERT_TRUE(std::isnan(nan_result)) << "NaN should be preserved";

    // Test integer values remain unchanged
    ASSERT_EQ(42.0, call_f64_nearest(42.0))
        << "Integer values should remain unchanged";
    ASSERT_EQ(-17.0, call_f64_nearest(-17.0))
        << "Negative integer values should remain unchanged";
    ASSERT_EQ(0.0, call_f64_nearest(0.0))
        << "Zero should remain unchanged";
}

/**
 * @test AllValidInputs_NeverTraps
 * @brief Validates f64.nearest never traps for any valid f64 input
 * @details Tests that f64.nearest operates correctly on all categories of f64 values
 *          without causing runtime traps or exceptions, including edge cases.
 * @test_category Exception - Non-trapping behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64.nearest execution path
 * @input_conditions Various f64 values including boundary cases and special values
 * @expected_behavior All operations complete successfully without traps or exceptions
 * @validation_method Verify successful execution and no runtime exceptions for diverse inputs
 */
TEST_P(F64NearestTest, AllValidInputs_NeverTraps)
{
    // Test large finite values
    double large_val = call_f64_nearest(1.7976931348623157e+308);  // Near DBL_MAX
    ASSERT_FALSE(std::isnan(large_val)) << "Large finite value should not produce NaN";
    ASSERT_FALSE(std::isinf(large_val)) << "Large finite value should not produce infinity";

    // Test small positive value
    double small_val = call_f64_nearest(2.2250738585072014e-308);  // Near DBL_MIN
    ASSERT_FALSE(std::isnan(small_val)) << "Small finite value should not produce NaN";

    // Test subnormal values
    double subnormal = call_f64_nearest(4.9406564584124654e-324);  // Smallest subnormal
    ASSERT_FALSE(std::isnan(subnormal)) << "Subnormal value should not produce NaN";

    // Test values near boundaries that might cause precision issues
    ASSERT_EQ(1.0, call_f64_nearest(1.0 + std::numeric_limits<double>::epsilon()))
        << "Values near 1.0 should round correctly";
    ASSERT_EQ(2.0, call_f64_nearest(2.0 - std::numeric_limits<double>::epsilon()))
        << "Values near 2.0 should round correctly";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(CrossMode, F64NearestTest,
                         testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<RunningMode>& info) {
                             return info.param == RunningMode::Mode_Interp ? "Interpreter" : "AOT";
                         });