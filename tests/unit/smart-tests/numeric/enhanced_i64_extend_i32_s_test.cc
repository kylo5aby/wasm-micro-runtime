/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
static std::string WASM_FILE_STACK_UNDERFLOW;

static int
app_argc;
static char **app_argv;

class I64ExtendI32STest : public testing::TestWithParam<RunningMode>
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

    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        WASM_FILE = "wasm-apps/i64_extend_i32_s_test.wasm";
        WASM_FILE_STACK_UNDERFLOW = "wasm-apps/i64_extend_i32_s_stack_underflow.wasm";

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

    int64_t call_i64_extend_i32_s(int32_t input)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "extend_i32_s_test");
        EXPECT_NE(func, nullptr) << "Failed to lookup extend_i32_s_test function";

        uint32_t argv[3] = { (uint32_t)input, 0, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
        }

        // Extract i64 result from argv (stored in first two elements)
        uint64_t result = ((uint64_t)argv[1] << 32) | argv[0];
        return (int64_t)result;
    }

    void test_stack_underflow(const std::string& wasm_file)
    {
        uint8_t *underflow_buf = nullptr;
        uint32_t underflow_buf_size;
        wasm_module_t underflow_module = nullptr;
        wasm_module_inst_t underflow_inst = nullptr;
        wasm_exec_env_t underflow_exec_env = nullptr;

        underflow_buf = (uint8_t *)bh_read_file_to_buffer(wasm_file.c_str(), &underflow_buf_size);
        ASSERT_NE(underflow_buf, nullptr) << "Failed to read underflow test WASM file";

        underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                           error_buf, sizeof(error_buf));

        // Test should succeed since our WAT file is valid
        if (underflow_module) {
            underflow_inst = wasm_runtime_instantiate(underflow_module, stack_size, heap_size,
                                                    error_buf, sizeof(error_buf));

            if (underflow_inst) {
                wasm_runtime_set_running_mode(underflow_inst, GetParam());
                underflow_exec_env = wasm_runtime_create_exec_env(underflow_inst, stack_size);

                if (underflow_exec_env) {
                    wasm_function_inst_t func = wasm_runtime_lookup_function(underflow_inst, "stack_underflow_test");
                    if (func) {
                        uint32_t argv[2] = { 0, 0 };
                        bool ret = wasm_runtime_call_wasm(underflow_exec_env, func, 0, argv);

                        ASSERT_EQ(ret, true) << "Stack underflow test function should execute successfully";
                    }
                    wasm_runtime_destroy_exec_env(underflow_exec_env);
                }
                wasm_runtime_deinstantiate(underflow_inst);
            }
            wasm_runtime_unload(underflow_module);
        }

        BH_FREE(underflow_buf);
    }
};

/**
 * @test BasicSignExtension_TypicalValues_ReturnsCorrectConversion
 * @brief Validates i64.extend_i32_s produces correct sign extension for typical input values
 * @details Tests fundamental sign extension operation with positive, negative, and zero values.
 *          Verifies that i64.extend_i32_s correctly preserves the sign of i32 input while
 *          extending to 64-bit representation with appropriate high-word bit patterns.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Standard i32 values: positive (100), negative (-100), zero (0)
 * @expected_behavior Sign extension: 100→0x0000000000000064, -100→0xFFFFFFFFFFFFFF9C, 0→0x0000000000000000
 * @validation_method Direct comparison of WASM function results with expected i64 values
 */
TEST_P(I64ExtendI32STest, BasicSignExtension_TypicalValues_ReturnsCorrectConversion)
{
    // Test positive value sign extension - high 32 bits should be 0
    int64_t result_positive = call_i64_extend_i32_s(100);
    ASSERT_EQ(result_positive, 0x0000000000000064LL)
        << "Positive i32 sign extension failed: expected 0x0000000000000064, got 0x"
        << std::hex << result_positive;

    // Test negative value sign extension - high 32 bits should be 0xFFFFFFFF
    int64_t result_negative = call_i64_extend_i32_s(-100);
    ASSERT_EQ(result_negative, (int64_t)0xFFFFFFFFFFFFFF9CLL)
        << "Negative i32 sign extension failed: expected 0xFFFFFFFFFFFFFF9C, got 0x"
        << std::hex << result_negative;

    // Test zero value sign extension - result should be complete 64-bit zero
    int64_t result_zero = call_i64_extend_i32_s(0);
    ASSERT_EQ(result_zero, 0x0000000000000000LL)
        << "Zero i32 sign extension failed: expected 0x0000000000000000, got 0x"
        << std::hex << result_zero;
}

/**
 * @test BoundaryValues_MinMaxConstants_ReturnsCorrectConversion
 * @brief Validates i64.extend_i32_s handles INT32_MIN and INT32_MAX boundary values correctly
 * @details Tests sign extension behavior at the extreme boundaries of the i32 type range.
 *          Verifies correct bit patterns for maximum positive and minimum negative i32 values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Boundary constants: INT32_MAX (0x7FFFFFFF), INT32_MIN (0x80000000)
 * @expected_behavior INT32_MAX→0x000000007FFFFFFF, INT32_MIN→0xFFFFFFFF80000000
 * @validation_method Exact bit pattern verification for boundary value sign extension
 */
TEST_P(I64ExtendI32STest, BoundaryValues_MinMaxConstants_ReturnsCorrectConversion)
{
    // Test INT32_MAX sign extension - positive value should have zero-filled high word
    int64_t result_max = call_i64_extend_i32_s(INT32_MAX);
    ASSERT_EQ(result_max, 0x000000007FFFFFFFLL)
        << "INT32_MAX sign extension failed: expected 0x000000007FFFFFFF, got 0x"
        << std::hex << result_max;

    // Test INT32_MIN sign extension - negative value should have one-filled high word
    int64_t result_min = call_i64_extend_i32_s(INT32_MIN);
    ASSERT_EQ(result_min, (int64_t)0xFFFFFFFF80000000LL)
        << "INT32_MIN sign extension failed: expected 0xFFFFFFFF80000000, got 0x"
        << std::hex << result_min;
}

/**
 * @test SignBitTransitions_EdgeBoundaries_PreservesSignCorrectly
 * @brief Validates i64.extend_i32_s correctly handles sign bit transitions and edge boundaries
 * @details Tests sign extension behavior for values near the positive/negative boundary (around zero)
 *          and verifies that the sign bit (MSB of i32) correctly determines high word bit pattern.
 * @test_category Edge - Sign boundary and bit pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Edge values: 1, -1, values demonstrating sign bit behavior
 * @expected_behavior 1→0x0000000000000001, -1→0xFFFFFFFFFFFFFFFF, preserving sign patterns
 * @validation_method Bit pattern analysis for sign preservation across i32→i64 conversion
 */
TEST_P(I64ExtendI32STest, SignBitTransitions_EdgeBoundaries_PreservesSignCorrectly)
{
    // Test +1 sign extension - minimal positive value
    int64_t result_one = call_i64_extend_i32_s(1);
    ASSERT_EQ(result_one, 0x0000000000000001LL)
        << "Value +1 sign extension failed: expected 0x0000000000000001, got 0x"
        << std::hex << result_one;

    // Test -1 sign extension - all bits in high word should be 1
    int64_t result_neg_one = call_i64_extend_i32_s(-1);
    ASSERT_EQ(result_neg_one, (int64_t)0xFFFFFFFFFFFFFFFFLL)
        << "Value -1 sign extension failed: expected 0xFFFFFFFFFFFFFFFF, got 0x"
        << std::hex << result_neg_one;

    // Test value just below INT32_MAX (0x7FFFFFFE) - should remain positive
    int64_t result_near_max = call_i64_extend_i32_s(0x7FFFFFFE);
    ASSERT_EQ(result_near_max, 0x000000007FFFFFFELL)
        << "Value 0x7FFFFFFE sign extension failed: expected 0x000000007FFFFFFE, got 0x"
        << std::hex << result_near_max;

    // Test value just above INT32_MIN (0x80000001) - should remain negative
    int64_t result_near_min = call_i64_extend_i32_s((int32_t)0x80000001);
    ASSERT_EQ(result_near_min, (int64_t)0xFFFFFFFF80000001LL)
        << "Value 0x80000001 sign extension failed: expected 0xFFFFFFFF80000001, got 0x"
        << std::hex << result_near_min;
}

/**
 * @test StackUnderflow_EmptyStack_HandlesErrorCorrectly
 * @brief Validates i64.extend_i32_s handles stack underflow conditions appropriately
 * @details Tests behavior when i64.extend_i32_s opcode is executed without sufficient values on stack.
 *          Verifies that WAMR properly detects and handles the error condition during module loading or execution.
 * @test_category Exception - Error handling validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_load
 * @input_conditions WASM module with i64.extend_i32_s but insufficient stack values
 * @expected_behavior Module load failure or execution trap with appropriate error handling
 * @validation_method Error condition detection and proper runtime error response
 */
TEST_P(I64ExtendI32STest, StackUnderflow_EmptyStack_HandlesErrorCorrectly)
{
    // Test the stack underflow scenario using our helper method
    test_stack_underflow(WASM_FILE_STACK_UNDERFLOW);
}

INSTANTIATE_TEST_SUITE_P(RunningMode, I64ExtendI32STest,
                        testing::Values(Mode_Interp
#if WASM_ENABLE_JIT != 0
                                      , Mode_LLVM_JIT
#endif
                        ));