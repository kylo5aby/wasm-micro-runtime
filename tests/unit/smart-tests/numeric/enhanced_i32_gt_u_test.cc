/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i32.gt_u Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i32.gt_u
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic unsigned greater-than comparison functionality
 * - Boundary Cases: Zero, maximum values, and signed/unsigned boundary conditions
 * - Edge Cases: Identity operations, mathematical properties, and extreme values
 * - Module Validation: Proper WASM module loading and execution
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i32.gt_u)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:4780-4784
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

/**
 * @brief Test fixture for i32.gt_u opcode validation
 * @details Manages WAMR runtime initialization, module loading, and cleanup.
 *          Supports both interpreter and AOT execution modes through parameterization.
 */
class I32GtUTest : public testing::TestWithParam<RunningMode>
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
     * @brief Helper function to call i32.gt_u operation in WASM module
     * @param a Left operand for i32.gt_u comparison (treated as unsigned)
     * @param b Right operand for i32.gt_u comparison (treated as unsigned)
     * @return Result of i32.gt_u operation (1 if a > b unsigned, 0 otherwise)
     * @details Invokes test_i32_gt_u function in loaded WASM module and handles exceptions.
     */
    int32_t call_i32_gt_u(int32_t a, int32_t b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i32_gt_u");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_gt_u function";

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
 * @test BasicUnsignedComparison_ReturnsCorrectResults
 * @brief Validates i32.gt_u produces correct comparison results for typical unsigned values
 * @details Tests fundamental unsigned greater-than operation with various input combinations.
 *          Verifies that i32.gt_u correctly compares unsigned values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_gt_u_operation
 * @input_conditions Standard unsigned integer pairs: (10,5), (200,100), (2000,1000)
 * @expected_behavior Returns 1 for true comparisons, 0 for false comparisons
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32GtUTest, BasicUnsignedComparison_ReturnsCorrectResults)
{
    // Test basic unsigned greater-than comparisons
    ASSERT_EQ(call_i32_gt_u(10, 5), 1) << "10 > 5 (unsigned) should return true";
    ASSERT_EQ(call_i32_gt_u(5, 10), 0) << "5 > 10 (unsigned) should return false";
    ASSERT_EQ(call_i32_gt_u(200, 100), 1) << "200 > 100 (unsigned) should return true";
    ASSERT_EQ(call_i32_gt_u(100, 200), 0) << "100 > 200 (unsigned) should return false";
    ASSERT_EQ(call_i32_gt_u(2000, 1000), 1) << "2000 > 1000 (unsigned) should return true";
    ASSERT_EQ(call_i32_gt_u(1000, 2000), 0) << "1000 > 2000 (unsigned) should return false";
}

/**
 * @test BoundaryValues_ValidateUnsignedLimits
 * @brief Validates i32.gt_u behavior at unsigned integer boundaries and critical values
 * @details Tests comparison behavior with zero, maximum unsigned values, and critical boundaries.
 *          Demonstrates unsigned interpretation of values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_gt_u_operation
 * @input_conditions Boundary values: 0, 1, MAX_UINT32, and critical unsigned boundaries
 * @expected_behavior Correct unsigned comparison results at all boundaries
 * @validation_method Verification of boundary behavior and unsigned semantics
 */
TEST_P(I32GtUTest, BoundaryValues_ValidateUnsignedLimits)
{
    // Test with zero boundary
    ASSERT_EQ(call_i32_gt_u(1, 0), 1) << "1 > 0 (unsigned) should return true";
    ASSERT_EQ(call_i32_gt_u(0, 1), 0) << "0 > 1 (unsigned) should return false";

    // Test maximum unsigned value boundary
    const uint32_t MAX_UINT32 = 0xFFFFFFFF;
    ASSERT_EQ(call_i32_gt_u(MAX_UINT32, MAX_UINT32 - 1), 1)
        << "MAX_UINT32 > MAX_UINT32-1 should return true";
    ASSERT_EQ(call_i32_gt_u(MAX_UINT32 - 1, MAX_UINT32), 0)
        << "MAX_UINT32-1 > MAX_UINT32 should return false";

    // Test signed/unsigned boundary (INT32_MIN as unsigned is middle of range)
    const uint32_t INT32_MIN_AS_UINT = 0x80000000;
    const uint32_t INT32_MAX_AS_UINT = 0x7FFFFFFF;
    ASSERT_EQ(call_i32_gt_u(INT32_MIN_AS_UINT, INT32_MAX_AS_UINT), 1)
        << "INT32_MIN > INT32_MAX in unsigned comparison should return true";
}

/**
 * @test SignedVsUnsigned_DemonstrateUnsignedBehavior
 * @brief Demonstrates where unsigned comparison differs from signed comparison
 * @details Tests critical cases where signed and unsigned comparisons produce opposite results.
 *          Validates that negative signed values are treated as large unsigned values.
 * @test_category Corner - Signed vs unsigned behavior differences
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_gt_u_operation
 * @input_conditions Negative signed values treated as large unsigned values
 * @expected_behavior Unsigned interpretation produces different results than signed
 * @validation_method Direct verification of unsigned behavior with negative values
 */
TEST_P(I32GtUTest, SignedVsUnsigned_DemonstrateUnsignedBehavior)
{
    // -1 as unsigned (0xFFFFFFFF) is largest possible unsigned value
    ASSERT_EQ(call_i32_gt_u(-1, 100), 1) << "-1 as uint32 (0xFFFFFFFF) > 100 should return true";
    ASSERT_EQ(call_i32_gt_u(100, -1), 0) << "100 > -1 as uint32 (0xFFFFFFFF) should return false";

    // -1 as unsigned (0xFFFFFFFF) vs -2 as unsigned (0xFFFFFFFE)
    ASSERT_EQ(call_i32_gt_u(-1, -2), 1) << "-1 as uint32 > -2 as uint32 should return true";

    // Negative values as unsigned are larger than positive values
    ASSERT_EQ(call_i32_gt_u(-100, 1000), 1)
        << "Negative value as unsigned should be larger than positive value";
}

/**
 * @test IdentityComparisons_ReturnFalse
 * @brief Validates that identical values always return false for greater-than comparison
 * @details Tests mathematical property that x > x is always false.
 *          Covers various identical value patterns including boundary cases.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_gt_u_operation
 * @input_conditions Identical value pairs across the unsigned range
 * @expected_behavior All identity comparisons return false (0)
 * @validation_method Verification of anti-reflexivity property
 */
TEST_P(I32GtUTest, IdentityComparisons_ReturnFalse)
{
    // Test identity property: x > x should always be false
    ASSERT_EQ(call_i32_gt_u(0, 0), 0) << "0 > 0 should return false";
    ASSERT_EQ(call_i32_gt_u(100, 100), 0) << "100 > 100 should return false";
    ASSERT_EQ(call_i32_gt_u(-1, -1), 0) << "-1 > -1 (as unsigned) should return false";

    const uint32_t MAX_UINT32 = 0xFFFFFFFF;
    ASSERT_EQ(call_i32_gt_u(MAX_UINT32, MAX_UINT32), 0)
        << "MAX_UINT32 > MAX_UINT32 should return false";

    const uint32_t INT32_MIN_AS_UINT = 0x80000000;
    ASSERT_EQ(call_i32_gt_u(INT32_MIN_AS_UINT, INT32_MIN_AS_UINT), 0)
        << "INT32_MIN as unsigned > itself should return false";
}

/**
 * @test TransitivityProperty_ValidateLogicalConsistency
 * @brief Validates transitivity property of greater-than comparison: if a > b and b > c, then a > c
 * @details Tests that unsigned greater-than comparison maintains logical consistency
 *          across chained comparisons.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_gt_u_operation
 * @input_conditions Value triplets that demonstrate transitivity
 * @expected_behavior Transitivity property holds for all valid chains
 * @validation_method Verification of transitive property through chained comparisons
 */
TEST_P(I32GtUTest, TransitivityProperty_ValidateLogicalConsistency)
{
    // Test transitivity: if a > b and b > c, then a > c
    uint32_t a = 30, b = 20, c = 10;

    ASSERT_EQ(call_i32_gt_u(a, b), 1) << "First comparison a > b should be true";
    ASSERT_EQ(call_i32_gt_u(b, c), 1) << "Second comparison b > c should be true";
    ASSERT_EQ(call_i32_gt_u(a, c), 1) << "Transitive comparison a > c should be true";

    // Test with unsigned boundary values
    uint32_t x = 0x90000000, y = 0x80000000, z = 0x70000000;

    ASSERT_EQ(call_i32_gt_u(x, y), 1) << "First comparison x > y should be true";
    ASSERT_EQ(call_i32_gt_u(y, z), 1) << "Second comparison y > z should be true";
    ASSERT_EQ(call_i32_gt_u(x, z), 1) << "Transitive comparison x > z should be true";
}

/**
 * @test ZeroComparisons_ValidateZeroBehavior
 * @brief Validates i32.gt_u behavior when zero is involved as operand
 * @details Tests specific zero-value scenarios important for unsigned comparison.
 *          Zero is the minimum value in unsigned arithmetic.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_gt_u_operation
 * @input_conditions Zero as first/second operand with various other values
 * @expected_behavior Correct comparison results with zero as minimum unsigned value
 * @validation_method Verification of zero comparison behavior across value ranges
 */
TEST_P(I32GtUTest, ZeroComparisons_ValidateZeroBehavior)
{
    // Zero vs positive values
    ASSERT_EQ(call_i32_gt_u(0, 1), 0) << "0 > 1 should return false";
    ASSERT_EQ(call_i32_gt_u(1, 0), 1) << "1 > 0 should return true";
    ASSERT_EQ(call_i32_gt_u(0, 100), 0) << "0 > 100 should return false";
    ASSERT_EQ(call_i32_gt_u(100, 0), 1) << "100 > 0 should return true";

    // Zero vs negative values (large unsigned)
    ASSERT_EQ(call_i32_gt_u(0, -1), 0) << "0 > -1 (as large unsigned) should return false";
    ASSERT_EQ(call_i32_gt_u(-1, 0), 1) << "-1 (as large unsigned) > 0 should return true";
}

// Test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningMode, I32GtUTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == Mode_Interp ? "InterpreterMode" : "AOTMode";
    }
);

// Global test setup - executed before all tests
struct I32GtUTestSetup
{
    I32GtUTestSetup()
    {
        CWD = getcwd(nullptr, 0);
        WASM_FILE = CWD + "/wasm-apps/i32_gt_u_test.wasm";
    }
};

// Static initializer to set up global test environment
static I32GtUTestSetup test_setup;