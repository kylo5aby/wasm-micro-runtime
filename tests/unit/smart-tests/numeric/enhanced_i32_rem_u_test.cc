/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;

/**
 * @brief Test fixture for i32.rem_u opcode validation
 * @details Provides comprehensive test framework for unsigned 32-bit integer remainder operation.
 *          Tests cover basic functionality, boundary conditions, edge cases, and error scenarios
 *          across both interpreter and AOT execution modes.
 */
class I32RemUTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment and initialize WAMR runtime
     * @details Initializes WAMR runtime and prepares test module loading.
     *          Validates that runtime initialization succeeds before proceeding with tests.
     */
    void SetUp() override {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t*)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
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
     * @brief Clean up test environment
     * @details Cleanup WAMR runtime resources and free memory.
     *          Ensures proper resource deallocation after test completion.
     */
    void TearDown() override {
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
     * @brief Execute WASM function with two i32 parameters and return result
     * @param func_name Name of the exported function to call
     * @param param1 First i32 parameter (dividend)
     * @param param2 Second i32 parameter (divisor)
     * @return Function execution result as uint32_t
     * @details Invokes exported WASM function with specified parameters and returns
     *          the computed result. Handles execution context and error reporting.
     */
    uint32_t call_i32_rem_u(uint32_t a, uint32_t b) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_rem_u");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_rem_u function";

        uint32_t argv[3] = { a, b, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        const char* exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
        }

        return argv[0];
    }

    /**
     * @brief Execute WASM function expecting trap/exception
     * @param a First i32 parameter (dividend)
     * @param b Second i32 parameter (divisor)
     * @return True if function trapped as expected, false otherwise
     * @details Invokes WASM function that should cause a runtime trap and validates
     *          that the trap occurs with appropriate error reporting.
     */
    bool call_i32_rem_u_expect_trap(uint32_t a, uint32_t b) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_rem_u");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_rem_u function";

        uint32_t argv[3] = { a, b, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);

        // Function should fail (trap) and have exception message
        if (!ret) {
            const char* exception = wasm_runtime_get_exception(module_inst);
            return (exception != nullptr && strlen(exception) > 0);
        }

        return false;  // Function should have trapped but didn't
    }

    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = {0};
    const char *exception = nullptr;
    static constexpr uint32_t UINT32_MAX_VAL = 4294967295U;
};

/**
 * @test BasicRemainderOperation_ReturnsCorrectResult
 * @brief Validates i32.rem_u produces correct unsigned remainder results for typical inputs
 * @details Tests fundamental unsigned remainder operation with various input combinations.
 *          Verifies that i32.rem_u correctly computes dividend % divisor for standard cases.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_op_i32_rem_u
 * @input_conditions Standard integer pairs: (10,3), (17,5), (100,7)
 * @expected_behavior Returns mathematical remainder: 1, 2, 2 respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32RemUTest, BasicRemainderOperation_ReturnsCorrectResult) {
    // Test basic remainder operations with typical values
    ASSERT_EQ(1U, call_i32_rem_u(10, 3))
        << "10 % 3 should equal 1";
    ASSERT_EQ(2U, call_i32_rem_u(17, 5))
        << "17 % 5 should equal 2";
    ASSERT_EQ(2U, call_i32_rem_u(100, 7))
        << "100 % 7 should equal 2";
}

/**
 * @test BoundaryValues_HandleMaximumValues
 * @brief Validates i32.rem_u handles boundary conditions with maximum unsigned values
 * @details Tests remainder operation with UINT32_MAX and other boundary values.
 *          Ensures correct behavior at the limits of unsigned 32-bit integer range.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_op_i32_rem_u
 * @input_conditions Maximum value scenarios: (UINT32_MAX,1), (UINT32_MAX,UINT32_MAX), (UINT32_MAX,2)
 * @expected_behavior Returns correct remainders: 0, 0, 1 respectively
 * @validation_method Mathematical verification of boundary value remainder calculations
 */
TEST_P(I32RemUTest, BoundaryValues_HandleMaximumValues) {
    // Test boundary values with maximum unsigned integers
    ASSERT_EQ(0U, call_i32_rem_u(UINT32_MAX_VAL, 1))
        << "UINT32_MAX % 1 should equal 0";
    ASSERT_EQ(0U, call_i32_rem_u(UINT32_MAX_VAL, UINT32_MAX_VAL))
        << "UINT32_MAX % UINT32_MAX should equal 0";
    ASSERT_EQ(1U, call_i32_rem_u(UINT32_MAX_VAL, 2))
        << "UINT32_MAX % 2 should equal 1";
}

/**
 * @test IdentityOperations_ProduceExpectedResults
 * @brief Validates i32.rem_u mathematical identity properties and edge cases
 * @details Tests remainder operation identity behaviors including modulo 1 and self-modulo.
 *          Verifies mathematical properties of remainder operation hold correctly.
 * @test_category Edge - Mathematical identity validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_op_i32_rem_u
 * @input_conditions Identity scenarios: (42,1), (15,15), (7,10)
 * @expected_behavior Returns identity results: 0, 0, 7 respectively
 * @validation_method Verification of mathematical remainder identity properties
 */
TEST_P(I32RemUTest, IdentityOperations_ProduceExpectedResults) {
    // Test mathematical identity operations
    ASSERT_EQ(0U, call_i32_rem_u(42, 1))
        << "Any value % 1 should equal 0";
    ASSERT_EQ(0U, call_i32_rem_u(15, 15))
        << "Any value % itself should equal 0";
    ASSERT_EQ(7U, call_i32_rem_u(7, 10))
        << "Dividend < divisor should return dividend";
}

/**
 * @test ZeroDividend_ReturnsZero
 * @brief Validates i32.rem_u behavior with zero dividend
 * @details Tests remainder operation when dividend is zero with various divisors.
 *          Verifies that 0 % n = 0 for any positive divisor n.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_op_i32_rem_u
 * @input_conditions Zero dividend scenarios: (0,5), (0,100), (0,UINT32_MAX)
 * @expected_behavior Returns zero for all cases: 0, 0, 0
 * @validation_method Mathematical verification of zero dividend remainder property
 */
TEST_P(I32RemUTest, ZeroDividend_ReturnsZero) {
    // Test zero dividend with various divisors
    ASSERT_EQ(0U, call_i32_rem_u(0, 5))
        << "0 % 5 should equal 0";
    ASSERT_EQ(0U, call_i32_rem_u(0, 100))
        << "0 % 100 should equal 0";
    ASSERT_EQ(0U, call_i32_rem_u(0, UINT32_MAX_VAL))
        << "0 % UINT32_MAX should equal 0";
}

/**
 * @test DivisionByZero_CausesTrap
 * @brief Validates i32.rem_u properly traps on division by zero
 * @details Tests remainder operation error handling when divisor is zero.
 *          Verifies that division by zero causes appropriate runtime trap.
 * @test_category Error - Exception handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_op_i32_rem_u
 * @input_conditions Division by zero scenarios: (42,0), (UINT32_MAX,0)
 * @expected_behavior Runtime trap with proper error handling
 * @validation_method Exception mechanism verification and error message validation
 */
TEST_P(I32RemUTest, DivisionByZero_CausesTrap) {
    // Test division by zero trap scenarios
    ASSERT_TRUE(call_i32_rem_u_expect_trap(42, 0))
        << "42 % 0 should cause runtime trap";
    ASSERT_TRUE(call_i32_rem_u_expect_trap(UINT32_MAX_VAL, 0))
        << "UINT32_MAX % 0 should cause runtime trap";
}

// Parameterized test execution for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, I32RemUTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I32RemUTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

static int app_argc;
static char **app_argv;

/**
 * @brief Initialize test file paths and runtime environment
 * @details Sets up WASM file paths relative to current working directory.
 */
static void InitializeTestEnvironment()
{
    char *cwd = getcwd(NULL, 0);
    if (cwd) {
        CWD = std::string(cwd);
        free(cwd);
    } else {
        CWD = ".";
    }

    WASM_FILE = CWD + "/wasm-apps/i32_rem_u_test.wasm";
}

// Initialize environment before tests run
class GlobalTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        InitializeTestEnvironment();
    }
};

// Register the global environment
static ::testing::Environment* const global_env =
    ::testing::AddGlobalTestEnvironment(new GlobalTestEnvironment());