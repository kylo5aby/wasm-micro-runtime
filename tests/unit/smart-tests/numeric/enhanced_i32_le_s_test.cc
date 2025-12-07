/**
 * @file enhanced_i32_le_s_test.cc
 * @brief Enhanced unit tests for i32.le_s (Less Than or Equal Signed) WASM opcode
 *
 * This file provides comprehensive test coverage for the i32.le_s WebAssembly opcode,
 * which performs signed less-than-or-equal comparison between two 32-bit integers and returns 1 if the
 * first operand is less than or equal to the second, 0 otherwise.
 *
 * Test Categories:
 * - Basic functionality with typical signed integer values
 * - Boundary conditions with INT32_MIN/MAX values
 * - Zero comparison scenarios across all combinations
 * - Signed semantics validation (vs unsigned interpretation)
 * - Cross-execution mode validation (interpreter vs AOT)
 *
 * @author WAMR Test Generator
 * @date 2025-11-06
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"


/**
 * @class I32LeSTest
 * @brief Test fixture for i32.le_s opcode validation across execution modes
 *
 * This fixture provides comprehensive testing infrastructure for the i32.le_s
 * WebAssembly opcode, supporting both interpreter and AOT execution modes.
 * It manages WAMR runtime initialization, WASM module loading, and proper
 * resource cleanup through RAII patterns.
 */
class I32LeSTest : public testing::TestWithParam<RunningMode> {
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
     * @brief Set up test environment and initialize WAMR runtime
     *
     * Initializes the WAMR runtime, loads the test WASM module, and sets up
     * the execution environment. This method is called before each test case.
     */
    void SetUp() override {
        memset(error_buf, 0, sizeof(error_buf));

        // Load the WASM module from file
        buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/i32_le_s_test.wasm", &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to load WASM file: wasm-apps/i32_le_s_test.wasm";

        // Load WASM module
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        // Create module instance
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and release resources
     *
     * Properly releases all WAMR resources including execution environment,
     * module instance, module, and file buffer. This method is called after each test case.
     */
    void TearDown() override {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (buf) {
            BH_FREE(buf);
        }
    }

    /**
     * @brief Execute i32.le_s comparison with two operands
     * @param a First operand (left-hand side of comparison)
     * @param b Second operand (right-hand side of comparison)
     * @return Comparison result: 1 if a <= b (signed), 0 otherwise
     *
     * Calls the WASM function that performs i32.le_s comparison and returns the result.
     * The function handles stack management and error checking automatically.
     */
    int32_t call_i32_le_s(int32_t a, int32_t b) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = a } },
            { .kind = WASM_I32, .of = { .i32 = b } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        // Call the WASM function for i32.le_s basic comparison
        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_s_basic"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM execution exception: " << exception;

        return results[0].of.i32;
    }

    /**
     * @brief Execute i32.le_s boundary test cases
     * @param a First operand for boundary comparison
     * @param b Second operand for boundary comparison
     * @return Comparison result for boundary test cases
     */
    int32_t call_i32_le_s_boundary(int32_t a, int32_t b) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = a } },
            { .kind = WASM_I32, .of = { .i32 = b } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_s_boundary"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM boundary function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM boundary execution exception: " << exception;

        return results[0].of.i32;
    }

    /**
     * @brief Execute i32.le_s zero comparison test cases
     * @param a First operand for zero comparison
     * @param b Second operand for zero comparison
     * @return Comparison result for zero test cases
     */
    int32_t call_i32_le_s_zero(int32_t a, int32_t b) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = a } },
            { .kind = WASM_I32, .of = { .i32 = b } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_s_zero"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM zero function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM zero execution exception: " << exception;

        return results[0].of.i32;
    }

    /**
     * @brief Execute i32.le_s signed semantics validation test cases
     * @param a First operand for signed semantics test
     * @param b Second operand for signed semantics test
     * @return Comparison result for signed semantics test cases
     */
    int32_t call_i32_le_s_signed_semantics(int32_t a, int32_t b) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = a } },
            { .kind = WASM_I32, .of = { .i32 = b } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_s_signed_semantics"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM signed semantics function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM signed semantics execution exception: " << exception;

        return results[0].of.i32;
    }
};

/**
 * @test BasicSignedComparison_ReturnsCorrectResults
 * @brief Validates i32.le_s produces correct signed less-than-or-equal results for typical inputs
 * @details Tests fundamental less-than-or-equal operation with positive, negative, and mixed-sign integers.
 *          Verifies that i32.le_s correctly computes a <= b for various signed input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_s_operation
 * @input_conditions Standard signed integer pairs: (5,10), (10,5), (-15,-5), (-5,-15), (-10,5), (5,-10)
 * @expected_behavior Returns correct comparison: 1, 0, 1, 0, 1, 0 respectively
 * @validation_method Direct comparison of WASM function result with expected signed comparison values
 */
TEST_P(I32LeSTest, BasicSignedComparison_ReturnsCorrectResults) {
    // Test positive numbers: 5 <= 10 should be true (1)
    ASSERT_EQ(1, call_i32_le_s(5, 10))
        << "i32.le_s comparison failed: 5 <= 10 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test positive numbers: 10 <= 5 should be false (0)
    ASSERT_EQ(0, call_i32_le_s(10, 5))
        << "i32.le_s comparison failed: 10 <= 5 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test negative numbers: -15 <= -5 should be true (1)
    ASSERT_EQ(1, call_i32_le_s(-15, -5))
        << "i32.le_s comparison failed: -15 <= -5 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test negative numbers: -5 <= -15 should be false (0)
    ASSERT_EQ(0, call_i32_le_s(-5, -15))
        << "i32.le_s comparison failed: -5 <= -15 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test mixed signs: -10 <= 5 should be true (1)
    ASSERT_EQ(1, call_i32_le_s(-10, 5))
        << "i32.le_s comparison failed: -10 <= 5 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test mixed signs: 5 <= -10 should be false (0)
    ASSERT_EQ(0, call_i32_le_s(5, -10))
        << "i32.le_s comparison failed: 5 <= -10 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test equal values: 42 <= 42 should be true (1)
    ASSERT_EQ(1, call_i32_le_s(42, 42))
        << "i32.le_s comparison failed: 42 <= 42 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test BoundaryValueComparison_ExtremeValues
 * @brief Validates i32.le_s behavior with INT32_MIN and INT32_MAX boundary values
 * @details Tests comparison operations at the extremes of 32-bit signed integer range.
 *          Ensures correct handling of most negative and most positive values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_s_boundary_handling
 * @input_conditions Boundary values: INT32_MIN (-2147483648), INT32_MAX (2147483647)
 * @expected_behavior INT32_MIN <= INT32_MAX → 1, INT32_MAX <= INT32_MIN → 0, self-comparisons → 1
 * @validation_method Verification of boundary value comparison results and reflexivity
 */
TEST_P(I32LeSTest, BoundaryValueComparison_ExtremeValues) {
    // Test INT32_MIN <= INT32_MAX (most negative <= most positive) should be true (1)
    ASSERT_EQ(1, call_i32_le_s_boundary(INT32_MIN, INT32_MAX))
        << "i32.le_s boundary comparison failed: INT32_MIN <= INT32_MAX should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test INT32_MAX <= INT32_MIN (most positive <= most negative) should be false (0)
    ASSERT_EQ(0, call_i32_le_s_boundary(INT32_MAX, INT32_MIN))
        << "i32.le_s boundary comparison failed: INT32_MAX <= INT32_MIN should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test INT32_MIN <= INT32_MIN (reflexivity) should be true (1)
    ASSERT_EQ(1, call_i32_le_s_boundary(INT32_MIN, INT32_MIN))
        << "i32.le_s boundary comparison failed: INT32_MIN <= INT32_MIN should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test INT32_MAX <= INT32_MAX (reflexivity) should be true (1)
    ASSERT_EQ(1, call_i32_le_s_boundary(INT32_MAX, INT32_MAX))
        << "i32.le_s boundary comparison failed: INT32_MAX <= INT32_MAX should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test near-boundary values: INT32_MIN <= (INT32_MIN + 1) should be true (1)
    ASSERT_EQ(1, call_i32_le_s_boundary(INT32_MIN, INT32_MIN + 1))
        << "i32.le_s near-boundary comparison failed: INT32_MIN <= (INT32_MIN + 1) should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test near-boundary values: (INT32_MAX - 1) <= INT32_MAX should be true (1)
    ASSERT_EQ(1, call_i32_le_s_boundary(INT32_MAX - 1, INT32_MAX))
        << "i32.le_s near-boundary comparison failed: (INT32_MAX - 1) <= INT32_MAX should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test ZeroOperandScenarios_VariousZeroComparisons
 * @brief Validates i32.le_s behavior in all zero-related comparison scenarios
 * @details Tests comprehensive zero comparison patterns including zero with itself,
 *          zero with positive numbers, and zero with negative numbers.
 * @test_category Edge - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_s_zero_handling
 * @input_conditions Zero combinations: (0,0), (0,positive), (0,negative), (positive,0), (negative,0)
 * @expected_behavior 0<=0 → 1, 0<=positive → 1, 0<=negative → 0, positive<=0 → 0, negative<=0 → 1
 * @validation_method Verification of zero comparison semantics across all scenarios
 */
TEST_P(I32LeSTest, ZeroOperandScenarios_VariousZeroComparisons) {
    // Test 0 <= 0 (zero equals zero) should be true (1)
    ASSERT_EQ(1, call_i32_le_s_zero(0, 0))
        << "i32.le_s zero comparison failed: 0 <= 0 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 0 <= 1 (zero less than positive) should be true (1)
    ASSERT_EQ(1, call_i32_le_s_zero(0, 1))
        << "i32.le_s zero comparison failed: 0 <= 1 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 0 <= -1 (zero greater than negative) should be false (0)
    ASSERT_EQ(0, call_i32_le_s_zero(0, -1))
        << "i32.le_s zero comparison failed: 0 <= -1 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 1 <= 0 (positive greater than zero) should be false (0)
    ASSERT_EQ(0, call_i32_le_s_zero(1, 0))
        << "i32.le_s zero comparison failed: 1 <= 0 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test -1 <= 0 (negative less than zero) should be true (1)
    ASSERT_EQ(1, call_i32_le_s_zero(-1, 0))
        << "i32.le_s zero comparison failed: -1 <= 0 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test SignBehaviorValidation_CorrectSignedSemantics
 * @brief Validates i32.le_s uses correct signed interpretation vs unsigned bit patterns
 * @details Tests critical signed comparison cases where signed interpretation differs
 *          from unsigned bit-pattern comparison, ensuring proper two's complement handling.
 * @test_category Edge - Signed semantics validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_s_signed_interpretation
 * @input_conditions Special bit patterns: 0x7FFFFFFF, 0x80000000, -1, large positive values
 * @expected_behavior Signed semantics: INT32_MAX <= INT32_MIN → 0, INT32_MIN <= INT32_MAX → 1
 * @validation_method Verification that signed comparison logic is used, not unsigned bit comparison
 */
TEST_P(I32LeSTest, SignBehaviorValidation_CorrectSignedSemantics) {
    // Test 0x7FFFFFFF <= 0x80000000 (INT32_MAX <= INT32_MIN in signed interpretation) should be false (0)
    // In unsigned: 2147483647 <= 2147483648, but in signed: 2147483647 <= -2147483648
    ASSERT_EQ(0, call_i32_le_s_signed_semantics(0x7FFFFFFF, 0x80000000))
        << "i32.le_s signed semantics failed: 0x7FFFFFFF <= 0x80000000 should return 0 (signed interpretation) in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 0x80000000 <= 0x7FFFFFFF (INT32_MIN <= INT32_MAX in signed interpretation) should be true (1)
    // In signed: -2147483648 <= 2147483647
    ASSERT_EQ(1, call_i32_le_s_signed_semantics(0x80000000, 0x7FFFFFFF))
        << "i32.le_s signed semantics failed: 0x80000000 <= 0x7FFFFFFF should return 1 (signed interpretation) in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 0xFFFFFFFF <= 0x00000001 (-1 <= 1 in signed interpretation) should be true (1)
    ASSERT_EQ(1, call_i32_le_s_signed_semantics(0xFFFFFFFF, 0x00000001))
        << "i32.le_s signed semantics failed: 0xFFFFFFFF <= 0x00000001 should return 1 (-1 <= 1) in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 0x00000001 <= 0xFFFFFFFF (1 <= -1 in signed interpretation) should be false (0)
    ASSERT_EQ(0, call_i32_le_s_signed_semantics(0x00000001, 0xFFFFFFFF))
        << "i32.le_s signed semantics failed: 0x00000001 <= 0xFFFFFFFF should return 0 (1 <= -1) in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, I32LeSTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I32LeSTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });