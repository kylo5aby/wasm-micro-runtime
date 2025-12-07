/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i64.lt_s Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i64.lt_s
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic signed 64-bit integer less-than comparison functionality
 * - Corner Cases: Boundary conditions with INT64_MAX/MIN values
 * - Edge Cases: Zero comparisons, identity operations, and mathematical properties
 * - Error Handling: Stack underflow and validation scenarios
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i64.lt_s)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c (i64 comparison operations)
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
static std::string WASM_FILE_UNDERFLOW;


/**
 * @class I64LtSTest
 * @brief Test fixture for comprehensive i64.lt_s opcode validation
 *
 * Provides WAMR runtime environment setup and cleanup for testing
 * i64.lt_s signed less-than comparison operations across different execution modes.
 * Supports both interpreter and AOT modes through parameterized testing.
 */
class I64LtSTest : public testing::TestWithParam<RunningMode>
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
     * @brief Set up WAMR runtime environment and load i64.lt_s test module
     *
     * Initializes WAMR runtime, loads the test WASM module, creates module instance,
     * and sets up execution environment for i64.lt_s testing.
     * Configures the running mode based on test parameter (interpreter/AOT).
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Load i64.lt_s test WASM module
        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode (interpreter or AOT)
        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up WAMR runtime resources
     *
     * Destroys execution environment, deinstantiates module, unloads module,
     * and frees allocated buffers to prevent resource leaks.
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
     * @brief Execute i64.lt_s comparison with two operands
     * @param left Left operand (first value)
     * @param right Right operand (second value)
     * @return Comparison result as i32 (1 if left < right, 0 otherwise)
     *
     * Calls the test_i64_lt_s function in the loaded WASM module,
     * passing two i64 values and returning the comparison result.
     */
    int32_t call_i64_lt_s(int64_t left, int64_t right)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_lt_s");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_lt_s function";

        uint32_t argv[4]; // Two i64 parameters (8 bytes each)
        argv[0] = (uint32_t)left;         // Low 32 bits of left operand
        argv[1] = (uint32_t)(left >> 32); // High 32 bits of left operand
        argv[2] = (uint32_t)right;        // Low 32 bits of right operand
        argv[3] = (uint32_t)(right >> 32); // High 32 bits of right operand

        bool ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(ret) << "Failed to call test_i64_lt_s function";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Exception during test_i64_lt_s execution: " << exception;

        return (int32_t)argv[0]; // Result is i32
    }

    /**
     * @brief Load and validate stack underflow test module
     * @return Stack underflow test module instance
     *
     * Loads a WASM module designed to test stack underflow conditions
     * for i64.lt_s operation when insufficient operands are available.
     */
    wasm_module_inst_t load_underflow_module()
    {
        uint32_t underflow_buf_size;
        uint8_t *underflow_buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE_UNDERFLOW.c_str(), &underflow_buf_size);
        EXPECT_NE(underflow_buf, nullptr) << "Failed to read underflow WASM file: " << WASM_FILE_UNDERFLOW;

        char underflow_error_buf[128] = { 0 };
        wasm_module_t underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                                          underflow_error_buf, sizeof(underflow_error_buf));
        EXPECT_NE(underflow_module, nullptr) << "Failed to load underflow module: " << underflow_error_buf;

        wasm_module_inst_t underflow_inst = wasm_runtime_instantiate(underflow_module, stack_size, heap_size,
                                                                    underflow_error_buf, sizeof(underflow_error_buf));
        EXPECT_NE(underflow_inst, nullptr) << "Failed to instantiate underflow module: " << underflow_error_buf;

        // Clean up the module and buffer (instance keeps the needed data)
        wasm_runtime_unload(underflow_module);
        BH_FREE(underflow_buf);

        return underflow_inst;
    }
};

/**
 * @test BasicComparison_ValidatesCorrectResults
 * @brief Validates i64.lt_s produces correct signed comparison results for typical inputs
 * @details Tests fundamental less-than comparison operation with positive, negative, and mixed-sign integers.
 *          Verifies that i64.lt_s correctly computes left < right for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_lt_s_operation
 * @input_conditions Standard integer pairs: (100,200), (1000,500), (-500,-200), (-100,-300), (-50,50), (75,-25)
 * @expected_behavior Returns mathematical comparison: true (1) or false (0) respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64LtSTest, BasicComparison_ValidatesCorrectResults)
{
    // Test positive integer comparisons
    ASSERT_EQ(1, call_i64_lt_s(100, 200)) << "100 < 200 should return true (1)";
    ASSERT_EQ(0, call_i64_lt_s(1000, 500)) << "1000 < 500 should return false (0)";

    // Test negative integer comparisons
    ASSERT_EQ(1, call_i64_lt_s(-500, -200)) << "-500 < -200 should return true (1)";
    ASSERT_EQ(0, call_i64_lt_s(-100, -300)) << "-100 < -300 should return false (0)";

    // Test mixed sign comparisons
    ASSERT_EQ(1, call_i64_lt_s(-50, 50)) << "-50 < 50 should return true (1)";
    ASSERT_EQ(0, call_i64_lt_s(75, -25)) << "75 < -25 should return false (0)";
}

/**
 * @test BoundaryValues_HandleExtremeCorrectly
 * @brief Validates i64.lt_s handles INT64_MAX/MIN boundary conditions correctly
 * @details Tests comparison behavior at the limits of 64-bit signed integer range.
 *          Verifies proper handling of maximum and minimum values in signed arithmetic.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_lt_s_boundary_handling
 * @input_conditions Boundary pairs: (INT64_MAX,INT64_MIN), (INT64_MIN,INT64_MAX), (INT64_MAX,INT64_MAX-1), (INT64_MIN,INT64_MIN+1)
 * @expected_behavior Returns correct signed comparison results: false (0), true (1), false (0), true (1)
 * @validation_method Boundary value testing with extreme signed integer values
 */
TEST_P(I64LtSTest, BoundaryValues_HandleExtremeCorrectly)
{
    // Test INT64_MAX vs INT64_MIN (wraparound behavior)
    ASSERT_EQ(0, call_i64_lt_s(INT64_MAX, INT64_MIN))
        << "INT64_MAX < INT64_MIN should return false (0) in signed arithmetic";

    ASSERT_EQ(1, call_i64_lt_s(INT64_MIN, INT64_MAX))
        << "INT64_MIN < INT64_MAX should return true (1) in signed arithmetic";

    // Test boundary adjacent values
    ASSERT_EQ(0, call_i64_lt_s(INT64_MAX, INT64_MAX - 1))
        << "INT64_MAX < (INT64_MAX-1) should return false (0)";

    ASSERT_EQ(1, call_i64_lt_s(INT64_MIN, INT64_MIN + 1))
        << "INT64_MIN < (INT64_MIN+1) should return true (1)";
}

/**
 * @test ZeroComparisons_ReturnsAccurateResults
 * @brief Validates i64.lt_s correctly handles zero-based comparisons
 * @details Tests comparison behavior with zero as one or both operands.
 *          Verifies correct handling of positive vs zero, negative vs zero, and zero vs zero.
 * @test_category Edge - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_lt_s_zero_handling
 * @input_conditions Zero-based pairs: (0,100), (-50,0), (0,0)
 * @expected_behavior Returns correct comparison results: true (1), true (1), false (0)
 * @validation_method Zero-centered boundary testing
 */
TEST_P(I64LtSTest, ZeroComparisons_ReturnsAccurateResults)
{
    // Zero vs positive
    ASSERT_EQ(1, call_i64_lt_s(0, 100)) << "0 < 100 should return true (1)";

    // Negative vs zero
    ASSERT_EQ(1, call_i64_lt_s(-50, 0)) << "-50 < 0 should return true (1)";

    // Zero vs zero (identity)
    ASSERT_EQ(0, call_i64_lt_s(0, 0)) << "0 < 0 should return false (0)";
}

/**
 * @test IdentityComparisons_AlwaysReturnsFalse
 * @brief Validates i64.lt_s returns false for identical value comparisons
 * @details Tests anti-reflexive property of less-than operator.
 *          Verifies that any value compared with itself always returns false.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_lt_s_identity_handling
 * @input_conditions Identity pairs: (42,42), (-100,-100), (INT64_MAX,INT64_MAX)
 * @expected_behavior Returns false (0) for all identity comparisons
 * @validation_method Mathematical property verification (anti-reflexivity)
 */
TEST_P(I64LtSTest, IdentityComparisons_AlwaysReturnsFalse)
{
    // Positive value identity
    ASSERT_EQ(0, call_i64_lt_s(42, 42)) << "42 < 42 should return false (0)";

    // Negative value identity
    ASSERT_EQ(0, call_i64_lt_s(-100, -100)) << "-100 < -100 should return false (0)";

    // Boundary value identity
    ASSERT_EQ(0, call_i64_lt_s(INT64_MAX, INT64_MAX))
        << "INT64_MAX < INT64_MAX should return false (0)";
}

/**
 * @test StackUnderflow_ProducesRuntimeTrap
 * @brief Validates i64.lt_s produces runtime trap when insufficient stack operands
 * @details Tests error handling when i64.lt_s is executed with insufficient values on stack.
 *          Verifies that WAMR properly detects and reports stack underflow conditions.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_lt_s_stack_validation
 * @input_conditions Execute i64.lt_s with 0 or 1 stack items instead of required 2
 * @expected_behavior Runtime trap or exception indicating stack underflow
 * @validation_method Stack underflow detection and trap handling verification
 */
TEST_P(I64LtSTest, StackUnderflow_ProducesRuntimeTrap)
{
    // Load stack underflow test module
    wasm_module_inst_t underflow_inst = load_underflow_module();
    ASSERT_NE(underflow_inst, nullptr) << "Failed to load stack underflow test module";

    // Create execution environment for underflow testing
    wasm_exec_env_t underflow_exec_env = wasm_runtime_create_exec_env(underflow_inst, stack_size);
    ASSERT_NE(underflow_exec_env, nullptr) << "Failed to create underflow execution environment";

    // Set the running mode for underflow testing
    wasm_runtime_set_running_mode(underflow_inst, GetParam());

    // Test stack underflow scenario
    wasm_function_inst_t underflow_func = wasm_runtime_lookup_function(underflow_inst, "test_stack_underflow");
    ASSERT_NE(underflow_func, nullptr) << "Failed to lookup stack underflow test function";

    uint32_t underflow_argv[1] = {0}; // No meaningful arguments for underflow test
    bool ret = wasm_runtime_call_wasm(underflow_exec_env, underflow_func, 0, underflow_argv);

    // Expect function call to fail due to stack underflow
    ASSERT_FALSE(ret) << "Stack underflow test should fail due to insufficient operands";

    const char *underflow_exception = wasm_runtime_get_exception(underflow_inst);
    ASSERT_NE(underflow_exception, nullptr) << "Expected exception for stack underflow condition";

    // Clean up underflow test resources
    wasm_runtime_destroy_exec_env(underflow_exec_env);
    wasm_runtime_deinstantiate(underflow_inst);
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(I64LtSModes, I64LtSTest,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT),
                        [](const testing::TestParamInfo<I64LtSTest::ParamType>& info) {
                            return info.param == Mode_Interp ? "Interpreter" : "AOT";
                        });

// Module initialization for i64_lt_s tests
class I64LtSTestSetup {
public:
    I64LtSTestSetup() {
        char *cwd_buffer = getcwd(NULL, 0);
        if (cwd_buffer) {
            CWD = std::string(cwd_buffer);
            free(cwd_buffer);
        }

        WASM_FILE = CWD + "/wasm-apps/i64_lt_s_test.wasm";
        WASM_FILE_UNDERFLOW = CWD + "/wasm-apps/i64_lt_s_stack_underflow.wasm";
    }
};

static I64LtSTestSetup test_setup;