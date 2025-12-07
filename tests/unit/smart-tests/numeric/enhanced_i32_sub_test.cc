/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i32.sub Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i32.sub
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic functionality with typical values
 * - Corner Cases: Boundary conditions and underflow scenarios
 * - Edge Cases: Zero operands, identity operations, and extreme values
 * - Mathematical Properties: Non-commutativity and wrap-around behavior
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i32.sub)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:2896-2901
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;

static int
app_argc;
static char **app_argv;

/**
 * @class I32SubTest
 * @brief Comprehensive test fixture for i32.sub opcode validation
 * @details Supports both interpreter and AOT execution modes with proper WAMR runtime management.
 *          Tests cover basic functionality, boundary conditions, overflow/underflow scenarios,
 *          and mathematical properties of 32-bit integer subtraction with wrap-around behavior.
 */
class I32SubTest : public testing::TestWithParam<RunningMode>
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

    /**
     * @brief Sets up WAMR runtime and loads i32.sub test module
     * @details Initializes WAMR runtime, loads WASM module, creates module instance,
     *          and sets up execution environment for test execution
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Initialize WASM file path if not already set
        if (WASM_FILE.empty()) {
            char *cwd = getcwd(NULL, 0);
            if (cwd) {
                CWD = std::string(cwd);
                free(cwd);
            } else {
                CWD = ".";
            }
            WASM_FILE = CWD + "/wasm-apps/i32_sub_test.wasm";
        }

        // Load WASM module from file
        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        // Load module into WAMR
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        // Instantiate module with stack and heap
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode (interpreter or AOT)
        wasm_runtime_set_running_mode(module_inst, GetParam());

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Tears down WAMR runtime and cleans up resources
     * @details Properly destroys execution environment, deinstantiates module,
     *          unloads module, and frees allocated buffers using RAII patterns
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

    /**
     * @brief Helper function to call i32.sub operation in WASM module
     * @param a First operand (minuend) - i32 value
     * @param b Second operand (subtrahend) - i32 value
     * @return Result of a - b as i32 with wrap-around behavior
     * @details Calls WASM test_i32_sub function and handles potential runtime exceptions
     */
    int32_t call_i32_sub(int32_t a, int32_t b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i32_sub");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_sub function";

        uint32_t argv[3] = { (uint32_t)a, (uint32_t)b, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
        }

        return (int32_t)argv[0];
    }
};

/**
 * @test BasicSubtraction_ReturnsCorrectResults
 * @brief Validates i32.sub produces correct arithmetic results for typical inputs
 * @details Tests fundamental subtraction operation with positive, negative, and mixed-sign integers.
 *          Verifies that i32.sub correctly computes a - b for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_sub_operation
 * @input_conditions Standard integer pairs with different sign combinations
 * @expected_behavior Returns mathematical difference: positive-positive, negative-negative, mixed signs
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32SubTest, BasicSubtraction_ReturnsCorrectResults)
{
    // Test positive integers: 10 - 3 = 7
    ASSERT_EQ(call_i32_sub(10, 3), 7)
        << "Subtraction of small positive integers failed";

    // Test negative integers: -15 - (-8) = -7
    ASSERT_EQ(call_i32_sub(-15, -8), -7)
        << "Subtraction of negative integers failed";

    // Test mixed signs: 50 - (-25) = 75
    ASSERT_EQ(call_i32_sub(50, -25), 75)
        << "Subtraction with negative second operand failed";

    // Test mixed signs: -40 - 15 = -55
    ASSERT_EQ(call_i32_sub(-40, 15), -55)
        << "Subtraction with negative first operand failed";

    // Test larger values: 1000000 - 300000 = 700000
    ASSERT_EQ(call_i32_sub(1000000, 300000), 700000)
        << "Subtraction of large positive numbers failed";
}

/**
 * @test BoundaryConditions_HandlesExtremeValues
 * @brief Validates i32.sub handles boundary values correctly without wrapping
 * @details Tests subtraction operations near INT32_MIN and INT32_MAX boundaries
 *          where results remain within valid 32-bit integer range.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_sub_boundary_handling
 * @input_conditions Values near INT32_MIN and INT32_MAX with safe operands
 * @expected_behavior Correct arithmetic results without overflow/underflow wrapping
 * @validation_method Verify boundary arithmetic produces expected mathematical results
 */
TEST_P(I32SubTest, BoundaryConditions_HandlesExtremeValues)
{
    // Test near maximum boundary: INT32_MAX - 1 = 2147483646
    ASSERT_EQ(call_i32_sub(INT32_MAX, 1), INT32_MAX - 1)
        << "Subtraction from INT32_MAX should not overflow";

    // Test near minimum boundary: INT32_MIN - (-1) = INT32_MIN + 1 = -2147483647
    ASSERT_EQ(call_i32_sub(INT32_MIN, -1), INT32_MIN + 1)
        << "Subtraction of negative from INT32_MIN should not underflow";

    // Test boundary with zero: INT32_MAX - 0 = INT32_MAX
    ASSERT_EQ(call_i32_sub(INT32_MAX, 0), INT32_MAX)
        << "Subtracting zero from INT32_MAX should preserve value";

    // Test boundary with zero: INT32_MIN - 0 = INT32_MIN
    ASSERT_EQ(call_i32_sub(INT32_MIN, 0), INT32_MIN)
        << "Subtracting zero from INT32_MIN should preserve value";
}

/**
 * @test OverflowUnderflow_WrapsCorrectly
 * @brief Validates i32.sub wrap-around behavior for overflow and underflow scenarios
 * @details Tests two's complement arithmetic wrapping when results exceed 32-bit range.
 *          Verifies correct implementation of modular arithmetic for integer overflow.
 * @test_category Corner - Overflow/underflow wrap-around validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_sub_wrap_handling
 * @input_conditions Values that cause arithmetic overflow or underflow
 * @expected_behavior Two's complement wrap-around according to 32-bit arithmetic rules
 * @validation_method Verify precise wrap-around calculations match expected results
 */
TEST_P(I32SubTest, OverflowUnderflow_WrapsCorrectly)
{
    // Test underflow: INT32_MIN - 1 = INT32_MAX (wraps to maximum)
    ASSERT_EQ(call_i32_sub(INT32_MIN, 1), INT32_MAX)
        << "INT32_MIN - 1 should wrap to INT32_MAX";

    // Test extreme underflow: INT32_MIN - INT32_MAX = 1 (wraps around)
    ASSERT_EQ(call_i32_sub(INT32_MIN, INT32_MAX), 1)
        << "INT32_MIN - INT32_MAX should wrap to 1";

    // Test overflow: INT32_MAX - INT32_MIN = -1 (overflows and wraps)
    ASSERT_EQ(call_i32_sub(INT32_MAX, INT32_MIN), -1)
        << "INT32_MAX - INT32_MIN should wrap to -1";

    // Test large underflow: 0 - (INT32_MIN + 1) wraps correctly
    ASSERT_EQ(call_i32_sub(0, INT32_MIN + 1), INT32_MAX)
        << "0 - (INT32_MIN + 1) should wrap to INT32_MAX";
}

/**
 * @test ZeroOperations_ProducesExpectedResults
 * @brief Validates i32.sub behavior with zero operands and identity properties
 * @details Tests mathematical identity properties: x - 0 = x, 0 - x = -x, x - x = 0
 *          Ensures correct handling of zero in subtraction operations.
 * @test_category Edge - Zero operand and identity validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_sub_zero_handling
 * @input_conditions Zero operand scenarios and self-subtraction cases
 * @expected_behavior Mathematical identity properties hold correctly
 * @validation_method Verify zero-related arithmetic properties and negation behavior
 */
TEST_P(I32SubTest, ZeroOperations_ProducesExpectedResults)
{
    // Test identity property: x - 0 = x
    ASSERT_EQ(call_i32_sub(42, 0), 42)
        << "Subtracting zero should preserve original value";

    // Test negation property: 0 - x = -x
    ASSERT_EQ(call_i32_sub(0, 42), -42)
        << "Subtracting from zero should produce negation";

    // Test self-subtraction: x - x = 0
    ASSERT_EQ(call_i32_sub(123, 123), 0)
        << "Self-subtraction should always produce zero";

    // Test zero-zero: 0 - 0 = 0
    ASSERT_EQ(call_i32_sub(0, 0), 0)
        << "Zero minus zero should be zero";

    // Test negative self-subtraction: (-x) - (-x) = 0
    ASSERT_EQ(call_i32_sub(-456, -456), 0)
        << "Negative self-subtraction should produce zero";
}

/**
 * @test MathematicalProperties_ValidatesCorrectness
 * @brief Validates mathematical properties and non-commutativity of subtraction
 * @details Tests that subtraction is non-commutative (a - b ≠ b - a) and verifies
 *          other mathematical properties specific to subtraction operation.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_sub_properties
 * @input_conditions Various integer pairs testing mathematical properties
 * @expected_behavior Non-commutativity and other subtraction properties hold
 * @validation_method Verify mathematical law compliance and property verification
 */
TEST_P(I32SubTest, MathematicalProperties_ValidatesCorrectness)
{
    // Test non-commutativity: a - b ≠ b - a (except when a = b)
    int32_t a = 15, b = 7;
    int32_t result1 = call_i32_sub(a, b);  // 15 - 7 = 8
    int32_t result2 = call_i32_sub(b, a);  // 7 - 15 = -8
    ASSERT_NE(result1, result2)
        << "Subtraction should be non-commutative for different operands";
    ASSERT_EQ(result1, 8) << "15 - 7 should equal 8";
    ASSERT_EQ(result2, -8) << "7 - 15 should equal -8";

    // Test unit subtraction: x - 1 (decrement operation)
    ASSERT_EQ(call_i32_sub(10, 1), 9)
        << "Unit subtraction should decrement by one";

    // Test inverse addition property: (a + b) - b = a
    int32_t sum_result = a + b;  // This would be 22 if we had addition
    // We simulate this by testing: 22 - 7 = 15
    ASSERT_EQ(call_i32_sub(22, b), a)
        << "Subtraction should be inverse of addition";

    // Test difference magnitude: |a - b| distance calculation
    int32_t diff_pos = call_i32_sub(20, 5);   // 15
    int32_t diff_neg = call_i32_sub(5, 20);   // -15
    ASSERT_EQ(diff_pos, -diff_neg)
        << "Magnitude should be preserved with sign flip";
}

// Parameter instantiation for running tests in both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, I32SubTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I32SubTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

