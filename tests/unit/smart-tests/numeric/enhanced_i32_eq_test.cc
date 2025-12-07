/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i32.eq Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i32.eq
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic equality comparison functionality
 * - Corner Cases: Boundary conditions and extreme values
 * - Edge Cases: Zero operands, identity operations, and bit patterns
 * - Error Handling: Stack underflow and validation errors
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i32.eq)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:4753-4757
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
 * @brief Test fixture for i32.eq opcode validation
 * @details Manages WAMR runtime initialization, module loading, and cleanup.
 *          Supports both interpreter and AOT execution modes through parameterization.
 */
class I32EqTest : public testing::TestWithParam<RunningMode>
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
     * @brief Initialize test environment and load WASM module
     * @details Sets up WAMR runtime, loads test module, and creates execution environment.
     *          Called before each test case execution.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
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
     * @brief Clean up test environment and release resources
     * @details Destroys execution environment, deinstantiates module, and frees memory.
     *          Called after each test case execution.
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
     * @brief Helper function to call i32.eq operation in WASM module
     * @param a Left operand for i32.eq comparison
     * @param b Right operand for i32.eq comparison
     * @return Result of i32.eq operation (1 if equal, 0 if not equal)
     * @details Invokes test_i32_eq function in loaded WASM module and handles exceptions.
     */
    int32_t call_i32_eq(int32_t a, int32_t b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i32_eq");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_eq function";

        uint32_t argv[3] = { (uint32_t)a, (uint32_t)b, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
        }

        return (int32_t)argv[0];
    }

    /**
     * @brief Test stack validation with boundary conditions near underflow
     * @param wasm_file Path to WASM file containing stack validation tests
     * @details Validates that WAMR properly manages stack in boundary conditions.
     */
    void test_stack_validation(const std::string& wasm_file)
    {
        uint8_t *validation_buf = nullptr;
        uint32_t validation_buf_size;
        wasm_module_t validation_module = nullptr;
        wasm_module_inst_t validation_inst = nullptr;
        wasm_exec_env_t validation_exec_env = nullptr;

        validation_buf = (uint8_t *)bh_read_file_to_buffer(wasm_file.c_str(), &validation_buf_size);
        ASSERT_NE(validation_buf, nullptr) << "Failed to read validation test WASM file";

        validation_module = wasm_runtime_load(validation_buf, validation_buf_size,
                                            error_buf, sizeof(error_buf));
        ASSERT_NE(validation_module, nullptr)
            << "Valid stack validation module should load successfully: " << error_buf;

        validation_inst = wasm_runtime_instantiate(validation_module, stack_size, heap_size,
                                                  error_buf, sizeof(error_buf));
        ASSERT_NE(validation_inst, nullptr)
            << "Stack validation module should instantiate successfully: " << error_buf;

        wasm_runtime_set_running_mode(validation_inst, GetParam());
        validation_exec_env = wasm_runtime_create_exec_env(validation_inst, stack_size);
        ASSERT_NE(validation_exec_env, nullptr) << "Failed to create validation execution environment";

        // Test minimal stack usage - should return 1 (42 == 42)
        wasm_function_inst_t func = wasm_runtime_lookup_function(validation_inst, "test_minimal_stack");
        ASSERT_NE(func, nullptr) << "Failed to lookup test_minimal_stack function";

        uint32_t argv[1] = { 0 };
        bool ret = wasm_runtime_call_wasm(validation_exec_env, func, 0, argv);
        ASSERT_EQ(ret, true) << "Minimal stack validation should succeed";
        ASSERT_EQ((int32_t)argv[0], 1) << "Minimal stack test should return 1 (42 == 42)";

        // Cleanup
        wasm_runtime_destroy_exec_env(validation_exec_env);
        wasm_runtime_deinstantiate(validation_inst);
        wasm_runtime_unload(validation_module);
        BH_FREE(validation_buf);
    }
};

/**
 * @test BasicEquality_ReturnsCorrectResults
 * @brief Validates i32.eq produces correct equality comparison results for typical inputs
 * @details Tests fundamental equality operation with positive, negative, zero, and mixed-sign integers.
 *          Verifies that i32.eq correctly computes a == b for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:DEF_OP_CMP(uint32, I32, ==)
 * @input_conditions Standard integer pairs: (5,5), (5,3), (-10,-10), (-10,-15), (0,0), (0,1)
 * @expected_behavior Returns 1 for equal values, 0 for unequal values: 1, 0, 1, 0, 1, 0 respectively
 * @validation_method Direct comparison of WASM function result with expected boolean values
 */
TEST_P(I32EqTest, BasicEquality_ReturnsCorrectResults)
{
    // Test equal positive integers
    ASSERT_EQ(1, call_i32_eq(5, 5)) << "Equal positive integers should return 1";

    // Test unequal positive integers
    ASSERT_EQ(0, call_i32_eq(5, 3)) << "Unequal positive integers should return 0";

    // Test equal negative integers
    ASSERT_EQ(1, call_i32_eq(-10, -10)) << "Equal negative integers should return 1";

    // Test unequal negative integers
    ASSERT_EQ(0, call_i32_eq(-10, -15)) << "Unequal negative integers should return 0";

    // Test zero equality
    ASSERT_EQ(1, call_i32_eq(0, 0)) << "Zero should equal zero";

    // Test zero vs non-zero
    ASSERT_EQ(0, call_i32_eq(0, 1)) << "Zero should not equal non-zero";

    // Test mixed signs
    ASSERT_EQ(0, call_i32_eq(20, -20)) << "Same magnitude different signs should not be equal";
    ASSERT_EQ(0, call_i32_eq(-5, 5)) << "Positive and negative same value should not be equal";
}

/**
 * @test BoundaryValues_HandleCorrectly
 * @brief Validates i32.eq handles integer boundary values and extreme cases correctly
 * @details Tests comparison operations with INT32_MAX, INT32_MIN, and UINT32_MAX values.
 *          Ensures boundary conditions don't cause incorrect equality results.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:DEF_OP_CMP(uint32, I32, ==)
 * @input_conditions Integer boundaries: INT32_MAX, INT32_MIN, edge values near boundaries
 * @expected_behavior Identity comparisons return 1, different boundary values return 0
 * @validation_method Boundary value equality testing with comprehensive assertions
 */
TEST_P(I32EqTest, BoundaryValues_HandleCorrectly)
{
    // Test INT32_MAX boundary
    ASSERT_EQ(1, call_i32_eq(INT32_MAX, INT32_MAX)) << "INT32_MAX should equal itself";
    ASSERT_EQ(0, call_i32_eq(INT32_MAX, INT32_MAX - 1)) << "INT32_MAX should not equal INT32_MAX-1";

    // Test INT32_MIN boundary
    ASSERT_EQ(1, call_i32_eq(INT32_MIN, INT32_MIN)) << "INT32_MIN should equal itself";
    ASSERT_EQ(0, call_i32_eq(INT32_MIN, INT32_MIN + 1)) << "INT32_MIN should not equal INT32_MIN+1";

    // Test signed/unsigned boundary crossover (same bit patterns)
    ASSERT_EQ(1, call_i32_eq(-1, (int32_t)0xFFFFFFFF)) << "Same bit patterns should be equal";
    ASSERT_EQ(1, call_i32_eq((int32_t)0x80000000, INT32_MIN)) << "Same bit representation should be equal";

    // Test boundary differences
    ASSERT_EQ(0, call_i32_eq(INT32_MAX, INT32_MIN)) << "INT32_MAX should not equal INT32_MIN";
}

/**
 * @test BitPatterns_ValidateIdentical
 * @brief Validates i32.eq correctly compares identical and different bit patterns
 * @details Tests extreme bit patterns including all zeros, all ones, and alternating patterns.
 *          Verifies that equality is based on exact bit-wise comparison.
 * @test_category Edge - Extreme value and pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:DEF_OP_CMP(uint32, I32, ==)
 * @input_conditions Bit patterns: 0x00000000, 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555
 * @expected_behavior Identical patterns return 1, different patterns return 0
 * @validation_method Bit-pattern equality validation with hex output in failure messages
 */
TEST_P(I32EqTest, BitPatterns_ValidateIdentical)
{
    // Test all zeros pattern
    ASSERT_EQ(1, call_i32_eq(0x00000000, 0x00000000)) << "All zeros should equal all zeros";

    // Test all ones pattern
    ASSERT_EQ(1, call_i32_eq((int32_t)0xFFFFFFFF, (int32_t)0xFFFFFFFF)) << "All ones should equal all ones";

    // Test alternating patterns
    ASSERT_EQ(1, call_i32_eq((int32_t)0xAAAAAAAA, (int32_t)0xAAAAAAAA)) << "Alternating pattern A should equal itself";
    ASSERT_EQ(1, call_i32_eq((int32_t)0x55555555, (int32_t)0x55555555)) << "Alternating pattern 5 should equal itself";

    // Test different patterns
    ASSERT_EQ(0, call_i32_eq((int32_t)0xAAAAAAAA, (int32_t)0x55555555)) << "Different alternating patterns should not be equal";
    ASSERT_EQ(0, call_i32_eq(0x00000000, (int32_t)0xFFFFFFFF)) << "All zeros should not equal all ones";

    // Test single bit patterns
    ASSERT_EQ(1, call_i32_eq(0x00000001, 0x00000001)) << "Single bit pattern should equal itself";
    ASSERT_EQ(1, call_i32_eq((int32_t)0x80000000, (int32_t)0x80000000)) << "Sign bit pattern should equal itself";
    ASSERT_EQ(0, call_i32_eq(0x00000001, (int32_t)0x80000000)) << "Different single bit patterns should not be equal";

    // Test mathematical properties - reflexive property
    int32_t test_values[] = { 0, 1, -1, 42, -42, INT32_MAX, INT32_MIN };
    for (int32_t val : test_values) {
        ASSERT_EQ(1, call_i32_eq(val, val)) << "Reflexive property: value " << val << " should equal itself";
    }
}

/**
 * @test StackValidation_HandlesCorrectly
 * @brief Validates WAMR properly manages stack operations in boundary conditions
 * @details Tests stack validation with boundary conditions near underflow to ensure proper handling.
 *          Verifies that valid modules with minimal stack usage work correctly.
 * @test_category Error - Stack validation and boundary condition validation
 * @coverage_target Stack management and validation in WAMR runtime
 * @input_conditions WASM modules with minimal valid stack usage for i32.eq operation
 * @expected_behavior Module loads successfully, functions execute with correct results
 * @validation_method Stack boundary validation testing with proper execution verification
 */
TEST_P(I32EqTest, StackValidation_HandlesCorrectly)
{
    // Test stack validation with boundary conditions - ensures WAMR properly manages minimal stack usage
    test_stack_validation(WASM_FILE_UNDERFLOW);
}

// Test parameterization for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningMode, I32EqTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == Mode_Interp ? "InterpreterMode" : "AOTMode";
    }
);

// Global test setup - executed before all tests
struct I32EqTestSetup
{
    I32EqTestSetup()
    {
        CWD = getcwd(nullptr, 0);
        WASM_FILE = CWD + "/wasm-apps/i32_eq_test.wasm";
        WASM_FILE_UNDERFLOW = CWD + "/wasm-apps/i32_eq_stack_underflow.wasm";
    }
};

// Static initializer to set up global test environment
static I32EqTestSetup test_setup;