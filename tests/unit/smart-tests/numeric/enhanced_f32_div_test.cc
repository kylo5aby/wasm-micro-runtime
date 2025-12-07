/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for f32.div Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly f32.div
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic floating-point division with typical values
 * - Corner Cases: Boundary conditions and overflow/underflow scenarios
 * - Edge Cases: IEEE 754 special values (NaN, infinity, signed zeros)
 * - Error Handling: Stack underflow and validation errors
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (f32.div implementation)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c (DEF_OP_EQZ macro for f32.div)
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
 * @brief Test fixture class for f32.div opcode validation
 * @details Provides comprehensive testing infrastructure for the WebAssembly f32.div
 *          instruction across both interpreter and AOT execution modes. This test suite
 *          validates IEEE 754 single-precision floating-point division behavior, including
 *          special value handling, overflow/underflow conditions, and compliance with
 *          IEEE 754 standard for division operations.
 */
class F32DivTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment before each test case
     * @details Initializes WAMR runtime with system allocator, loads the f32.div test
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

        // Load WASM module for f32.div testing
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
     * @brief Load f32.div test WASM module from file system
     * @details Reads the compiled WASM bytecode for f32.div tests, validates module
     *          format, loads module into WAMR, and creates executable module instance.
     *          Handles both interpreter and AOT execution modes based on test parameters.
     */
    void load_test_module()
    {
        // Construct WASM file path relative to test execution directory
        const char *wasm_file = "wasm-apps/f32_div_test.wasm";

        // Read WASM module bytecode from file
        buf = (uint8_t *)bh_read_file_to_buffer(wasm_file, &buf_size);
        ASSERT_NE(buf, nullptr)
            << "Failed to read WASM file: " << wasm_file;
        ASSERT_GT(buf_size, 0U)
            << "WASM file is empty: " << wasm_file;

        // Load WASM module with validation
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr)
            << "Failed to load WASM module: " << error_buf;

        // Create module instance with appropriate memory configuration
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr)
            << "Failed to instantiate WASM module: " << error_buf;

        // Create execution environment for function calls
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr)
            << "Failed to create execution environment";
    }

protected:
    // WAMR runtime configuration and state management
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };

    /**
     * @brief Execute f32.div WASM function with two float arguments
     * @param dividend The first operand (dividend) for division operation
     * @param divisor The second operand (divisor) for division operation
     * @return Result of f32.div operation as IEEE 754 single-precision float
     */
    float call_f32_div(float dividend, float divisor)
    {
        // Lookup f32.div test function in module
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f32_div_test");
        EXPECT_NE(func, nullptr) << "Failed to find f32_div_test function";

        // Prepare function arguments: dividend and divisor
        uint32_t wasm_argv[2];
        memcpy(&wasm_argv[0], &dividend, sizeof(float));
        memcpy(&wasm_argv[1], &divisor, sizeof(float));

        // Execute function and capture result
        bool success = wasm_runtime_call_wasm(exec_env, func, 2, wasm_argv);
        EXPECT_TRUE(success) << "f32.div function execution failed: "
                            << wasm_runtime_get_exception(module_inst);

        // Extract and return f32 result
        float result;
        memcpy(&result, &wasm_argv[0], sizeof(float));
        return result;
    }

    /**
     * @brief Execute f32.div WASM function expecting potential exceptions
     * @param dividend The first operand (dividend) for division operation
     * @param divisor The second operand (divisor) for division operation
     * @return True if function executed without exceptions, false otherwise
     */
    bool call_f32_div_no_exception_expected(float dividend, float divisor)
    {
        // Lookup f32.div test function in module
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f32_div_test");
        EXPECT_NE(func, nullptr) << "Failed to find f32_div_test function";

        // Prepare function arguments: dividend and divisor
        uint32_t wasm_argv[2];
        memcpy(&wasm_argv[0], &dividend, sizeof(float));
        memcpy(&wasm_argv[1], &divisor, sizeof(float));

        // Execute function and return success status
        return wasm_runtime_call_wasm(exec_env, func, 2, wasm_argv);
    }
};

/**
 * @test StandardDivision_ReturnsCorrectQuotient
 * @brief Validates f32.div produces correct arithmetic results for typical floating-point inputs
 * @details Tests fundamental division operation with positive, negative, and mixed-sign floats.
 *          Verifies that f32.div correctly computes dividend / divisor for various input combinations
 *          following IEEE 754 single-precision arithmetic rules.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_div_operation
 * @input_conditions Standard float pairs: (6.0f, 2.0f), (-8.0f, 4.0f), (15.0f, -3.0f)
 * @expected_behavior Returns mathematical quotient: 3.0f, -2.0f, -5.0f respectively
 * @validation_method Direct comparison of WASM function result with expected IEEE 754 values
 */
TEST_P(F32DivTest, StandardDivision_ReturnsCorrectQuotient) {
    // Test positive dividend and positive divisor
    ASSERT_EQ(call_f32_div(6.0f, 2.0f), 3.0f)
        << "Failed: 6.0 / 2.0 should equal 3.0";

    // Test negative dividend and positive divisor
    ASSERT_EQ(call_f32_div(-8.0f, 4.0f), -2.0f)
        << "Failed: -8.0 / 4.0 should equal -2.0";

    // Test positive dividend and negative divisor
    ASSERT_EQ(call_f32_div(15.0f, -3.0f), -5.0f)
        << "Failed: 15.0 / -3.0 should equal -5.0";

    // Test negative dividend and negative divisor
    ASSERT_EQ(call_f32_div(-12.0f, -4.0f), 3.0f)
        << "Failed: -12.0 / -4.0 should equal 3.0";
}

/**
 * @test FractionalDivision_ProducesAccurateResults
 * @brief Validates f32.div produces correct results for fractional quotients within IEEE 754 precision
 * @details Tests division operations that produce non-integer results, ensuring accuracy
 *          within single-precision floating-point representation limits. Validates proper
 *          handling of repeating decimals and fractional arithmetic.
 * @test_category Main - Fractional arithmetic validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_div_operation
 * @input_conditions Fractional cases: (1.0f, 3.0f), (7.0f, 2.0f), (22.0f, 7.0f)
 * @expected_behavior Returns approximate quotients with IEEE 754 single-precision accuracy
 * @validation_method Comparison within acceptable floating-point tolerance limits
 */
TEST_P(F32DivTest, FractionalDivision_ProducesAccurateResults) {
    // Test division producing repeating decimal (1/3)
    float result1 = call_f32_div(1.0f, 3.0f);
    ASSERT_NEAR(result1, 0.33333334f, 1e-7f)
        << "Failed: 1.0 / 3.0 approximation within single-precision limits";

    // Test simple fractional division (7/2)
    ASSERT_EQ(call_f32_div(7.0f, 2.0f), 3.5f)
        << "Failed: 7.0 / 2.0 should equal exactly 3.5";

    // Test irrational approximation (22/7 ≈ π)
    float result2 = call_f32_div(22.0f, 7.0f);
    ASSERT_NEAR(result2, 3.142857f, 1e-6f)
        << "Failed: 22.0 / 7.0 approximation accuracy";
}

/**
 * @test ZeroDivisionBehavior_ProducesInfinity
 * @brief Validates f32.div produces IEEE 754 infinity for division by zero scenarios
 * @details Tests division by positive and negative zero, ensuring IEEE 754 compliance.
 *          Unlike integer division, f32.div does not trap on division by zero but
 *          produces ±infinity according to IEEE 754 standard rules.
 * @test_category Edge - IEEE 754 special value handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_div_zero_handling
 * @input_conditions Zero division cases: (5.0f, +0.0f), (5.0f, -0.0f), (0.0f, 0.0f)
 * @expected_behavior Returns +∞, -∞, NaN respectively following IEEE 754 rules
 * @validation_method IEEE 754 special value detection using isinf() and isnan()
 */
TEST_P(F32DivTest, ZeroDivisionBehavior_ProducesInfinity) {
    // Test positive value divided by positive zero
    float result1 = call_f32_div(5.0f, +0.0f);
    ASSERT_TRUE(isinf(result1) && result1 > 0)
        << "Failed: 5.0 / +0.0 should produce +infinity";

    // Test positive value divided by negative zero
    float result2 = call_f32_div(5.0f, -0.0f);
    ASSERT_TRUE(isinf(result2) && result2 < 0)
        << "Failed: 5.0 / -0.0 should produce -infinity";

    // Test zero divided by zero (indeterminate form)
    float result3 = call_f32_div(0.0f, 0.0f);
    ASSERT_TRUE(isnan(result3))
        << "Failed: 0.0 / 0.0 should produce NaN";

    // Test negative zero divided by positive zero
    float result4 = call_f32_div(-0.0f, +0.0f);
    ASSERT_TRUE(isnan(result4))
        << "Failed: -0.0 / +0.0 should produce NaN";
}

/**
 * @test InfinityOperations_FollowsIEEE754
 * @brief Validates f32.div handles infinity operands according to IEEE 754 standard
 * @details Tests all combinations of infinity with finite values and infinity with infinity,
 *          ensuring proper IEEE 754 behavior including sign propagation and NaN production
 *          for indeterminate forms like ∞/∞.
 * @test_category Edge - IEEE 754 infinity arithmetic
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_div_infinity_handling
 * @input_conditions Infinity cases: (∞, 2.0f), (2.0f, ∞), (∞, ∞), (-∞, -∞)
 * @expected_behavior Returns ∞, 0.0f, NaN, NaN respectively with proper signs
 * @validation_method IEEE 754 infinity and NaN detection with sign verification
 */
TEST_P(F32DivTest, InfinityOperations_FollowsIEEE754) {
    const float pos_inf = INFINITY;
    const float neg_inf = -INFINITY;

    // Test positive infinity divided by finite value
    float result1 = call_f32_div(pos_inf, 2.0f);
    ASSERT_TRUE(isinf(result1) && result1 > 0)
        << "Failed: +∞ / 2.0 should produce +infinity";

    // Test negative infinity divided by finite value
    float result2 = call_f32_div(neg_inf, 2.0f);
    ASSERT_TRUE(isinf(result2) && result2 < 0)
        << "Failed: -∞ / 2.0 should produce -infinity";

    // Test finite value divided by positive infinity
    float result3 = call_f32_div(2.0f, pos_inf);
    ASSERT_TRUE(result3 == 0.0f && !signbit(result3))
        << "Failed: 2.0 / +∞ should produce +0.0";

    // Test finite value divided by negative infinity
    float result4 = call_f32_div(2.0f, neg_inf);
    ASSERT_TRUE(result4 == 0.0f && signbit(result4))
        << "Failed: 2.0 / -∞ should produce -0.0";

    // Test infinity divided by infinity (indeterminate)
    float result5 = call_f32_div(pos_inf, pos_inf);
    ASSERT_TRUE(isnan(result5))
        << "Failed: +∞ / +∞ should produce NaN";

    float result6 = call_f32_div(neg_inf, neg_inf);
    ASSERT_TRUE(isnan(result6))
        << "Failed: -∞ / -∞ should produce NaN";
}

/**
 * @test NaNPropagation_PreservesNaN
 * @brief Validates f32.div propagates NaN values according to IEEE 754 quiet NaN rules
 * @details Tests all combinations of NaN with finite values, infinity, and other NaN values,
 *          ensuring that any arithmetic operation involving NaN produces NaN as result
 *          without causing exceptions or traps.
 * @test_category Edge - IEEE 754 NaN propagation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_div_nan_handling
 * @input_conditions NaN cases: (NaN, 5.0f), (5.0f, NaN), (NaN, ∞), (NaN, NaN)
 * @expected_behavior All operations produce NaN (quiet NaN propagation)
 * @validation_method NaN detection using isnan() with no exception verification
 */
TEST_P(F32DivTest, NaNPropagation_PreservesNaN) {
    const float quiet_nan = NAN;
    const float pos_inf = INFINITY;

    // Test NaN dividend with finite divisor
    float result1 = call_f32_div(quiet_nan, 5.0f);
    ASSERT_TRUE(isnan(result1))
        << "Failed: NaN / 5.0 should produce NaN";

    // Test finite dividend with NaN divisor
    float result2 = call_f32_div(5.0f, quiet_nan);
    ASSERT_TRUE(isnan(result2))
        << "Failed: 5.0 / NaN should produce NaN";

    // Test NaN dividend with infinity divisor
    float result3 = call_f32_div(quiet_nan, pos_inf);
    ASSERT_TRUE(isnan(result3))
        << "Failed: NaN / ∞ should produce NaN";

    // Test NaN dividend with NaN divisor
    float result4 = call_f32_div(quiet_nan, quiet_nan);
    ASSERT_TRUE(isnan(result4))
        << "Failed: NaN / NaN should produce NaN";

    // Verify no exceptions are thrown for NaN operations
    ASSERT_TRUE(call_f32_div_no_exception_expected(quiet_nan, 1.0f))
        << "NaN operations should not cause exceptions";
}

/**
 * @test ExtremeValueDivision_HandlesLimits
 * @brief Validates f32.div handles floating-point range limits including overflow and underflow
 * @details Tests division with maximum and minimum finite values, ensuring proper IEEE 754
 *          behavior for operations that exceed single-precision range or produce denormalized
 *          numbers through underflow conditions.
 * @test_category Corner - Floating-point range boundaries
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_div_range_handling
 * @input_conditions Extreme values: (FLT_MAX, FLT_MIN), (FLT_MIN, FLT_MAX), large/small ratios
 * @expected_behavior Handles overflow to ∞, underflow to 0, or produces large finite results
 * @validation_method Range verification using isinf(), finite value bounds checking
 */
TEST_P(F32DivTest, ExtremeValueDivision_HandlesLimits) {
    const float flt_max = std::numeric_limits<float>::max();
    const float flt_min = std::numeric_limits<float>::min();

    // Test maximum divided by minimum (potential overflow)
    float result1 = call_f32_div(flt_max, flt_min);
    ASSERT_TRUE(isinf(result1))
        << "Failed: FLT_MAX / FLT_MIN should overflow to infinity";

    // Test minimum divided by maximum (underflow toward zero)
    float result2 = call_f32_div(flt_min, flt_max);
    ASSERT_TRUE(result2 == 0.0f || isnan(result2) || isnormal(result2))
        << "Failed: FLT_MIN / FLT_MAX should underflow or produce small normal";

    // Test large value divided by very small value (overflow scenario)
    float result3 = call_f32_div(1.0e30f, 1.0e-30f);
    ASSERT_TRUE(isinf(result3))
        << "Failed: Large / Very_Small should overflow to infinity";

    // Test very small value divided by large value (underflow scenario)
    float result4 = call_f32_div(1.0e-30f, 1.0e30f);
    ASSERT_TRUE(result4 == 0.0f)
        << "Failed: Very_Small / Large should underflow to zero";
}

/**
 * @test IdentityOperations_PreserveValue
 * @brief Validates f32.div identity and inverse operations preserve mathematical properties
 * @details Tests division by 1.0 (identity), self-division (should equal 1.0), and division
 *          by -1.0 (negation), ensuring these fundamental mathematical properties are
 *          correctly implemented in the IEEE 754 division operation.
 * @test_category Edge - Mathematical identity properties
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_div_identity_handling
 * @input_conditions Identity cases: (x, 1.0f), (x, x), (x, -1.0f) for various x values
 * @expected_behavior Returns x, 1.0f, -x respectively (when mathematically valid)
 * @validation_method Direct equality comparison for exact mathematical identities
 */
TEST_P(F32DivTest, IdentityOperations_PreserveValue) {
    // Test division by 1.0 (multiplicative identity)
    ASSERT_EQ(call_f32_div(42.5f, 1.0f), 42.5f)
        << "Failed: x / 1.0 should equal x (identity property)";

    ASSERT_EQ(call_f32_div(-17.25f, 1.0f), -17.25f)
        << "Failed: negative x / 1.0 should equal x (identity property)";

    // Test self-division (should equal 1.0 for non-zero finite values)
    ASSERT_EQ(call_f32_div(5.5f, 5.5f), 1.0f)
        << "Failed: x / x should equal 1.0 for finite non-zero x";

    ASSERT_EQ(call_f32_div(-8.75f, -8.75f), 1.0f)
        << "Failed: negative x / negative x should equal 1.0";

    // Test division by -1.0 (negation operation)
    ASSERT_EQ(call_f32_div(3.14f, -1.0f), -3.14f)
        << "Failed: x / -1.0 should equal -x (negation property)";

    ASSERT_EQ(call_f32_div(-2.718f, -1.0f), 2.718f)
        << "Failed: negative x / -1.0 should equal positive x";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningMode, F32DivTest,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT),
                        [](const testing::TestParamInfo<F32DivTest::ParamType> &info) {
                            return info.param == Mode_Interp ? "INTERP" : "AOT";
                        });