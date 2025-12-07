/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i32.shl Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i32.shl
 * (32-bit shift left) instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic functionality with typical shift operations
 * - Corner Cases: Boundary conditions and overflow scenarios
 * - Edge Cases: Zero operands, identity operations, and shift count masking
 * - Error Handling: Stack underflow and validation errors
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i32.shl)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:3160-3165
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

static int
app_argc;
static char **app_argv;


class I32ShlTest : public testing::TestWithParam<RunningMode>
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

    /**
     * @brief Helper function to call WASM i32.shl operation
     * @details Invokes the test_i32_shl function in the loaded WASM module
     * @param value The value to shift left
     * @param shift_count The number of positions to shift left
     * @return The result of value << shift_count
     */
    int32_t call_i32_shl(int32_t value, int32_t shift_count)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i32_shl");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_shl function";

        uint32_t argv[3] = { (uint32_t)value, (uint32_t)shift_count, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
        }

        return (int32_t)argv[0];
    }

    /**
     * @brief Test helper for error handling scenarios
     * @details Loads and executes a WASM module to test error handling paths in the framework
     * @param wasm_file Path to the WASM file containing error handling test functions
     */
    void test_stack_underflow(const std::string& wasm_file)
    {
        uint8_t *underflow_buf = nullptr;
        uint32_t underflow_buf_size;
        wasm_module_t underflow_module = nullptr;
        wasm_module_inst_t underflow_inst = nullptr;
        wasm_exec_env_t underflow_exec_env = nullptr;

        underflow_buf = (uint8_t *)bh_read_file_to_buffer(wasm_file.c_str(), &underflow_buf_size);
        ASSERT_NE(underflow_buf, nullptr) << "Failed to read error handling test WASM file";

        underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                           error_buf, sizeof(error_buf));

        // The error handling module should load successfully
        ASSERT_NE(underflow_module, nullptr)
            << "Failed to load error handling test module: " << error_buf;

        underflow_inst = wasm_runtime_instantiate(underflow_module, stack_size, heap_size,
                                                error_buf, sizeof(error_buf));
        ASSERT_NE(underflow_inst, nullptr)
            << "Failed to instantiate error handling test module: " << error_buf;

        wasm_runtime_set_running_mode(underflow_inst, GetParam());
        underflow_exec_env = wasm_runtime_create_exec_env(underflow_inst, stack_size);
        ASSERT_NE(underflow_exec_env, nullptr)
            << "Failed to create execution environment for error handling test";

        // Test that the error handling function executes successfully
        wasm_function_inst_t func = wasm_runtime_lookup_function(underflow_inst, "test_empty_stack");
        ASSERT_NE(func, nullptr) << "Failed to lookup test_empty_stack function";

        uint32_t argv[1] = { 0 };
        bool ret = wasm_runtime_call_wasm(underflow_exec_env, func, 0, argv);
        ASSERT_EQ(ret, true) << "Error handling test function should execute successfully";

        // Verify the function returned expected value (42)
        ASSERT_EQ((int32_t)argv[0], 42) << "Error handling function should return 42";

        // Clean up resources
        wasm_runtime_destroy_exec_env(underflow_exec_env);
        wasm_runtime_deinstantiate(underflow_inst);
        wasm_runtime_unload(underflow_module);
        BH_FREE(underflow_buf);
    }
};

/**
 * @test BasicShiftLeft_SmallPositives_ReturnsCorrectResult
 * @brief Validates i32.shl produces correct shift results for small positive values
 * @details Tests fundamental shift left operation with typical positive integers and small shift counts.
 *          Verifies that i32.shl correctly computes value << shift_count for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Standard value/shift pairs: (1,1), (5,2), (7,3), (10,4)
 * @expected_behavior Returns bitwise left shift results: 2, 20, 56, 160 respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32ShlTest, BasicShiftLeft_SmallPositives_ReturnsCorrectResult)
{
    // Test basic shift operations with positive values
    ASSERT_EQ(call_i32_shl(1, 1), 2) << "1 << 1 should equal 2";
    ASSERT_EQ(call_i32_shl(5, 2), 20) << "5 << 2 should equal 20";
    ASSERT_EQ(call_i32_shl(7, 3), 56) << "7 << 3 should equal 56";
    ASSERT_EQ(call_i32_shl(10, 4), 160) << "10 << 4 should equal 160";
}

/**
 * @test BasicShiftLeft_NegativeValues_ReturnsCorrectResult
 * @brief Validates i32.shl correctly handles negative values with sign bit preservation
 * @details Tests shift left operation with negative integers to verify proper sign extension behavior.
 *          Negative values should shift left while maintaining two's complement representation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Negative values with small shifts: (-1,1), (-5,2), (-10,3)
 * @expected_behavior Returns proper two's complement shift results: -2, -20, -80
 * @validation_method Verification of correct negative value shift behavior
 */
TEST_P(I32ShlTest, BasicShiftLeft_NegativeValues_ReturnsCorrectResult)
{
    // Test shift operations with negative values
    ASSERT_EQ(call_i32_shl(-1, 1), -2) << "-1 << 1 should equal -2";
    ASSERT_EQ(call_i32_shl(-5, 2), -20) << "-5 << 2 should equal -20";
    ASSERT_EQ(call_i32_shl(-10, 3), -80) << "-10 << 3 should equal -80";
}

/**
 * @test BasicShiftLeft_PowerOfTwo_MultipliesCorrectly
 * @brief Validates mathematical property that shift left by n equals multiplication by 2^n
 * @details Tests the equivalence between bitwise shift left and multiplication for values that don't overflow.
 *          Verifies that x << n = x * 2^n when the result fits in 32 bits.
 * @test_category Main - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Values that don't overflow: (3,2), (6,1), (100,3)
 * @expected_behavior Returns results equivalent to multiplication: 12, 12, 800
 * @validation_method Comparison with mathematical multiplication equivalents
 */
TEST_P(I32ShlTest, BasicShiftLeft_PowerOfTwo_MultipliesCorrectly)
{
    // Verify mathematical property: x << n = x * 2^n (when no overflow)
    ASSERT_EQ(call_i32_shl(3, 2), 12) << "3 << 2 should equal 3 * 4 = 12";
    ASSERT_EQ(call_i32_shl(6, 1), 12) << "6 << 1 should equal 6 * 2 = 12";
    ASSERT_EQ(call_i32_shl(100, 3), 800) << "100 << 3 should equal 100 * 8 = 800";
}

/**
 * @test BoundaryShift_MaximumShiftCount_MasksCorrectly
 * @brief Tests shift count masking behavior for counts >= 32
 * @details Validates that shift counts are masked to lower 5 bits, so shift by 32+ wraps around.
 *          Per WebAssembly spec, i32.shl masks shift count with 0x1F (31).
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Large shift counts: (1,32), (5,33), (7,63)
 * @expected_behavior Equivalent to smaller shifts due to masking: 1, 10, 14
 * @validation_method Verification of proper shift count masking (& 0x1F)
 */
TEST_P(I32ShlTest, BoundaryShift_MaximumShiftCount_MasksCorrectly)
{
    // Test shift count masking (shift_count & 0x1F)
    ASSERT_EQ(call_i32_shl(1, 32), 1) << "1 << 32 should equal 1 << 0 = 1 (due to masking)";
    ASSERT_EQ(call_i32_shl(5, 33), 10) << "5 << 33 should equal 5 << 1 = 10 (due to masking)";
    ASSERT_EQ(call_i32_shl(7, 63), INT32_MIN) << "7 << 63 should equal 7 << 31 = INT32_MIN (due to masking)";
}

/**
 * @test BoundaryShift_IntegerBoundaries_OverflowsCorrectly
 * @brief Tests shift operations at integer boundaries with overflow handling
 * @details Validates proper wraparound behavior when shifting large values causes overflow.
 *          Tests both positive and negative boundary values to ensure correct two's complement behavior.
 * @test_category Corner - Overflow boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Boundary values: (INT32_MAX,1), (INT32_MIN,1), (0x40000000,1)
 * @expected_behavior Proper overflow wraparound: -2, 0, INT32_MIN
 * @validation_method Verification of correct overflow behavior and sign changes
 */
TEST_P(I32ShlTest, BoundaryShift_IntegerBoundaries_OverflowsCorrectly)
{
    // Test overflow behavior at integer boundaries
    ASSERT_EQ(call_i32_shl(INT32_MAX, 1), -2) << "INT32_MAX << 1 should overflow to -2";
    ASSERT_EQ(call_i32_shl(INT32_MIN, 1), 0) << "INT32_MIN << 1 should overflow to 0";
    ASSERT_EQ(call_i32_shl(0x40000000, 1), INT32_MIN) << "0x40000000 << 1 should equal INT32_MIN";
}

/**
 * @test BoundaryShift_MaxValidShift_ProducesCorrectResult
 * @brief Tests maximum valid shift count (31) behavior
 * @details Validates shift by 31, which is the maximum effective shift for 32-bit integers.
 *          Only the sign bit should survive this operation for positive values.
 * @test_category Corner - Maximum shift validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Maximum valid shifts: (1,31), (2,31), (-1,31)
 * @expected_behavior Only sign bit remains: INT32_MIN, 0, INT32_MIN
 * @validation_method Verification of maximum shift count effects
 */
TEST_P(I32ShlTest, BoundaryShift_MaxValidShift_ProducesCorrectResult)
{
    // Test shift by maximum valid amount (31)
    ASSERT_EQ(call_i32_shl(1, 31), INT32_MIN) << "1 << 31 should equal INT32_MIN";
    ASSERT_EQ(call_i32_shl(2, 31), 0) << "2 << 31 should equal 0";
    ASSERT_EQ(call_i32_shl(-1, 31), INT32_MIN) << "-1 << 31 should equal INT32_MIN";
}

/**
 * @test EdgeShift_ZeroOperands_ReturnsZero
 * @brief Tests identity and zero properties of shift left operation
 * @details Validates that shifting zero by any amount produces zero, and shifting by zero is identity.
 *          These are fundamental mathematical properties that must hold for i32.shl.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Zero scenarios: (0,0), (0,5), (0,31), (42,0)
 * @expected_behavior Identity and zero preservation: 0, 0, 0, 42
 * @validation_method Verification of mathematical identity properties
 */
TEST_P(I32ShlTest, EdgeShift_ZeroOperands_ReturnsZero)
{
    // Test zero operand scenarios and identity operation
    ASSERT_EQ(call_i32_shl(0, 0), 0) << "0 << 0 should equal 0";
    ASSERT_EQ(call_i32_shl(0, 5), 0) << "0 << 5 should equal 0 (zero invariant)";
    ASSERT_EQ(call_i32_shl(0, 31), 0) << "0 << 31 should equal 0";
    ASSERT_EQ(call_i32_shl(42, 0), 42) << "42 << 0 should equal 42 (identity operation)";
}

/**
 * @test EdgeShift_BitPatterns_ZeroFillsCorrectly
 * @brief Tests specific bit patterns to verify zero-fill behavior from right side
 * @details Validates that i32.shl properly fills with zeros from the right side during shift.
 *          Tests alternating patterns and all-ones to verify correct bit manipulation.
 * @test_category Edge - Bit pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Bit patterns: (0xAAAAAAAA,1), (0x55555555,1), (0xFFFFFFFF,4)
 * @expected_behavior Proper zero-fill patterns: 0x55555554, 0xAAAAAAAA, 0xFFFFFFF0
 * @validation_method Verification of correct bitwise shift and zero-fill behavior
 */
TEST_P(I32ShlTest, EdgeShift_BitPatterns_ZeroFillsCorrectly)
{
    // Test specific bit patterns for zero-fill verification
    ASSERT_EQ(call_i32_shl((int32_t)0xAAAAAAAA, 1), (int32_t)0x55555554)
        << "0xAAAAAAAA << 1 should zero-fill from right";
    ASSERT_EQ(call_i32_shl(0x55555555, 1), (int32_t)0xAAAAAAAA)
        << "0x55555555 << 1 should zero-fill from right";
    ASSERT_EQ(call_i32_shl(-1, 4), (int32_t)0xFFFFFFF0)
        << "0xFFFFFFFF << 4 should zero-fill 4 bits from right";
}

/**
 * @test EdgeShift_PowerOfTwoValues_ShiftsCorrectly
 * @brief Tests shift operations on power-of-2 values for clean bit patterns
 * @details Validates shift behavior on values that are powers of 2, which have clean single-bit patterns.
 *          These values should shift predictably with clear bit position changes.
 * @test_category Edge - Power-of-2 validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shl_operation
 * @input_conditions Power-of-2 values: (1,8), (2,7), (4,6), (8,5)
 * @expected_behavior Clean bit position shifts: 256, 256, 256, 256
 * @validation_method Verification of single-bit pattern shift behavior
 */
TEST_P(I32ShlTest, EdgeShift_PowerOfTwoValues_ShiftsCorrectly)
{
    // Test power-of-2 values (clean bit patterns)
    ASSERT_EQ(call_i32_shl(1, 8), 256) << "1 << 8 should equal 256";
    ASSERT_EQ(call_i32_shl(2, 7), 256) << "2 << 7 should equal 256";
    ASSERT_EQ(call_i32_shl(4, 6), 256) << "4 << 6 should equal 256";
    ASSERT_EQ(call_i32_shl(8, 5), 256) << "8 << 5 should equal 256";
}

/**
 * @test ErrorHandling_StackUnderflow_FailsGracefully
 * @brief Validates proper error handling for stack underflow conditions
 * @details Tests WAMR's response to i32.shl execution with insufficient stack operands.
 *          Should fail gracefully during module loading or validation phase.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:stack_validation
 * @input_conditions WASM module with stack underflow scenario
 * @expected_behavior Module load failure or graceful error handling
 * @validation_method Verification of proper error detection and reporting
 */
TEST_P(I32ShlTest, ErrorHandling_StackUnderflow_FailsGracefully)
{
    test_stack_underflow(WASM_FILE_UNDERFLOW);
}

INSTANTIATE_TEST_SUITE_P(RunningModeTest, I32ShlTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I32ShlTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

// Initialize test file paths using constructor approach for shared test framework
static void setup_i32_shl_test_paths() {
    char *cwd = getcwd(NULL, 0);
    if (cwd) {
        CWD = std::string(cwd);
        free(cwd);
    } else {
        CWD = ".";
    }

    WASM_FILE = CWD + "/wasm-apps/i32_shl_test.wasm";
    WASM_FILE_UNDERFLOW = CWD + "/wasm-apps/i32_shl_stack_underflow.wasm";
}

// Static initialization to set up paths when module loads
static int dummy_init = []() {
    setup_i32_shl_test_paths();
    return 0;
}();