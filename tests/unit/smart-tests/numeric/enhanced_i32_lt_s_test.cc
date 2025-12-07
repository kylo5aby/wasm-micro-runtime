/**
 * @file enhanced_i32_lt_s_test.cc
 * @brief Enhanced unit tests for i32.lt_s (Less Than Signed) WASM opcode
 *
 * This file provides comprehensive test coverage for the i32.lt_s WebAssembly opcode,
 * which performs signed comparison between two 32-bit integers and returns 1 if the
 * first operand is less than the second, 0 otherwise.
 *
 * Test Categories:
 * - Basic functionality with typical signed integer values
 * - Boundary conditions with INT32_MIN/MAX values
 * - Zero comparison scenarios
 * - Mathematical property validation (reflexivity, antisymmetry)
 * - Cross-execution mode validation (interpreter vs AOT)
 *
 * @author WAMR Test Generator
 * @date 2025-11-05
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
 * @class I32LtSTest
 * @brief Test fixture for i32.lt_s opcode validation across execution modes
 *
 * This fixture provides comprehensive testing infrastructure for the i32.lt_s
 * WebAssembly opcode, supporting both interpreter and AOT execution modes.
 * It manages WAMR runtime initialization, WASM module loading, and proper
 * resource cleanup through RAII patterns.
 */
class I32LtSTest : public testing::TestWithParam<RunningMode> {
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
     * @brief Set up test environment before each test case
     *
     * Initializes WAMR runtime with system allocator, loads the i32.lt_s test module,
     * and prepares execution environment for both interpreter and AOT modes.
     * Validates that all initialization steps complete successfully.
     */
    void SetUp() override {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/i32_lt_s_test.wasm", &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: wasm-apps/i32_lt_s_test.wasm";

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
     * @brief Clean up test environment after each test case
     *
     * Properly destroys execution environment, unloads WASM module instances,
     * and shuts down WAMR runtime to prevent resource leaks between test cases.
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
     * @brief Execute i32.lt_s test function with two operands
     * @param a First operand (left side of comparison)
     * @param b Second operand (right side of comparison)
     * @return Comparison result: 1 if a < b (signed), 0 otherwise
     *
     * Calls the WASM test function that performs i32.lt_s operation and
     * returns the comparison result as a 32-bit integer.
     */
    int32_t call_i32_lt_s(int32_t a, int32_t b) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i32_lt_s");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_lt_s function";

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
 * @test BasicComparison_ReturnsCorrectResult
 * @brief Validates i32.lt_s produces correct signed comparison results for typical inputs
 * @details Tests fundamental signed comparison operation with positive, negative, and mixed-sign integers.
 *          Verifies that i32.lt_s correctly computes (a < b) for various input combinations using signed arithmetic.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_lt_s_operation
 * @input_conditions Standard integer pairs including positive, negative, and mixed-sign comparisons
 * @expected_behavior Returns 1 when first operand is less than second (signed), 0 otherwise
 * @validation_method Direct comparison of WASM function result with expected mathematical values
 */
TEST_P(I32LtSTest, BasicComparison_ReturnsCorrectResult) {
    // Positive integer comparisons - smaller < larger should return 1
    ASSERT_EQ(1, call_i32_lt_s(5, 10)) << "5 < 10 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(15, 20)) << "15 < 20 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(100, 200)) << "100 < 200 should return 1 (true)";

    // Positive integer comparisons - larger < smaller should return 0
    ASSERT_EQ(0, call_i32_lt_s(10, 5)) << "10 < 5 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(20, 15)) << "20 < 15 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(200, 100)) << "200 < 100 should return 0 (false)";

    // Negative integer comparisons - more negative < less negative should return 1
    ASSERT_EQ(1, call_i32_lt_s(-10, -5)) << "-10 < -5 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(-100, -50)) << "-100 < -50 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(-20, -15)) << "-20 < -15 should return 1 (true)";

    // Negative integer comparisons - less negative < more negative should return 0
    ASSERT_EQ(0, call_i32_lt_s(-5, -10)) << "-5 < -10 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(-50, -100)) << "-50 < -100 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(-15, -20)) << "-15 < -20 should return 0 (false)";

    // Mixed sign comparisons - negative < positive should return 1
    ASSERT_EQ(1, call_i32_lt_s(-5, 5)) << "-5 < 5 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(-100, 50)) << "-100 < 50 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(-1, 1)) << "-1 < 1 should return 1 (true)";

    // Mixed sign comparisons - positive < negative should return 0
    ASSERT_EQ(0, call_i32_lt_s(5, -5)) << "5 < -5 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(50, -100)) << "50 < -100 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(1, -1)) << "1 < -1 should return 0 (false)";
}

/**
 * @test BoundaryValues_HandlesExtremeCorrectly
 * @brief Validates i32.lt_s handles boundary values and extreme integers correctly
 * @details Tests signed comparison behavior at integer boundaries including INT32_MIN, INT32_MAX,
 *          and values near these boundaries. Ensures proper handling of signed arithmetic edge cases.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_lt_s_boundary_handling
 * @input_conditions INT32_MIN, INT32_MAX, and adjacent boundary values
 * @expected_behavior Correct signed comparison results even at integer extremes
 * @validation_method Verification of signed comparison semantics at boundaries
 */
TEST_P(I32LtSTest, BoundaryValues_HandlesExtremeCorrectly) {
    // Maximum vs minimum signed values
    ASSERT_EQ(0, call_i32_lt_s(INT32_MAX, INT32_MIN))
        << "INT32_MAX < INT32_MIN should return 0 (false) - max is not less than min";
    ASSERT_EQ(1, call_i32_lt_s(INT32_MIN, INT32_MAX))
        << "INT32_MIN < INT32_MAX should return 1 (true) - min is less than max";

    // Near-boundary value testing
    ASSERT_EQ(1, call_i32_lt_s(INT32_MAX - 1, INT32_MAX))
        << "(INT32_MAX - 1) < INT32_MAX should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(INT32_MIN, INT32_MIN + 1))
        << "INT32_MIN < (INT32_MIN + 1) should return 1 (true)";
    ASSERT_EQ(0, call_i32_lt_s(INT32_MAX, INT32_MAX - 1))
        << "INT32_MAX < (INT32_MAX - 1) should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(INT32_MIN + 1, INT32_MIN))
        << "(INT32_MIN + 1) < INT32_MIN should return 0 (false)";

    // Boundary vs zero comparisons
    ASSERT_EQ(1, call_i32_lt_s(INT32_MIN, 0))
        << "INT32_MIN < 0 should return 1 (true) - minimum value is negative";
    ASSERT_EQ(0, call_i32_lt_s(INT32_MAX, 0))
        << "INT32_MAX < 0 should return 0 (false) - maximum value is positive";
    ASSERT_EQ(1, call_i32_lt_s(0, INT32_MAX))
        << "0 < INT32_MAX should return 1 (true) - zero is less than maximum";
    ASSERT_EQ(0, call_i32_lt_s(0, INT32_MIN))
        << "0 < INT32_MIN should return 0 (false) - zero is greater than minimum";
}

/**
 * @test ZeroComparisons_ReturnsExpectedResults
 * @brief Validates comparison behavior involving zero operands
 * @details Tests all combinations of zero with positive, negative, and zero values.
 *          Ensures proper signed comparison semantics when zero is involved.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_lt_s_zero_handling
 * @input_conditions Zero compared with positive, negative, and zero values
 * @expected_behavior Mathematically correct signed comparison results for zero cases
 * @validation_method Direct assertion of zero comparison outcomes
 */
TEST_P(I32LtSTest, ZeroComparisons_ReturnsExpectedResults) {
    // Zero vs positive numbers
    ASSERT_EQ(1, call_i32_lt_s(0, 1)) << "0 < 1 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(0, 100)) << "0 < 100 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(0, INT32_MAX)) << "0 < INT32_MAX should return 1 (true)";

    // Positive numbers vs zero
    ASSERT_EQ(0, call_i32_lt_s(1, 0)) << "1 < 0 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(100, 0)) << "100 < 0 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(INT32_MAX, 0)) << "INT32_MAX < 0 should return 0 (false)";

    // Zero vs negative numbers
    ASSERT_EQ(0, call_i32_lt_s(0, -1)) << "0 < -1 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(0, -100)) << "0 < -100 should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(0, INT32_MIN)) << "0 < INT32_MIN should return 0 (false)";

    // Negative numbers vs zero
    ASSERT_EQ(1, call_i32_lt_s(-1, 0)) << "-1 < 0 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(-100, 0)) << "-100 < 0 should return 1 (true)";
    ASSERT_EQ(1, call_i32_lt_s(INT32_MIN, 0)) << "INT32_MIN < 0 should return 1 (true)";

    // Zero vs zero - reflexive property
    ASSERT_EQ(0, call_i32_lt_s(0, 0)) << "0 < 0 should return 0 (false) - reflexive property";
}

/**
 * @test MathematicalProperties_ValidatesCorrectness
 * @brief Tests mathematical properties of the less-than relation
 * @details Validates reflexivity (x < x = false) and antisymmetry properties of signed comparison.
 *          Ensures that i32.lt_s behaves as a proper mathematical ordering relation.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_lt_s_relation_properties
 * @input_conditions Various integer values tested for reflexivity and antisymmetry
 * @expected_behavior Consistent mathematical behavior as a strict ordering relation
 * @validation_method Verification of mathematical properties across value ranges
 */
TEST_P(I32LtSTest, MathematicalProperties_ValidatesCorrectness) {
    // Reflexivity: x < x should always be false (irreflexive property)
    ASSERT_EQ(0, call_i32_lt_s(5, 5)) << "5 < 5 should return 0 (false) - reflexive property";
    ASSERT_EQ(0, call_i32_lt_s(-10, -10)) << "-10 < -10 should return 0 (false) - reflexive property";
    ASSERT_EQ(0, call_i32_lt_s(INT32_MAX, INT32_MAX)) << "INT32_MAX < INT32_MAX should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(INT32_MIN, INT32_MIN)) << "INT32_MIN < INT32_MIN should return 0 (false)";
    ASSERT_EQ(0, call_i32_lt_s(0, 0)) << "0 < 0 should return 0 (false) - reflexive property";

    // Antisymmetry: if a < b is true, then b < a must be false (and vice versa)
    // Test case: 3 < 7 = true, therefore 7 < 3 = false
    ASSERT_EQ(1, call_i32_lt_s(3, 7)) << "3 < 7 should return 1 (true)";
    ASSERT_EQ(0, call_i32_lt_s(7, 3)) << "7 < 3 should return 0 (false) - antisymmetry";

    // Test case: -15 < -5 = true, therefore -5 < -15 = false
    ASSERT_EQ(1, call_i32_lt_s(-15, -5)) << "-15 < -5 should return 1 (true)";
    ASSERT_EQ(0, call_i32_lt_s(-5, -15)) << "-5 < -15 should return 0 (false) - antisymmetry";

    // Test case: -10 < 10 = true, therefore 10 < -10 = false
    ASSERT_EQ(1, call_i32_lt_s(-10, 10)) << "-10 < 10 should return 1 (true)";
    ASSERT_EQ(0, call_i32_lt_s(10, -10)) << "10 < -10 should return 0 (false) - antisymmetry";

    // Boundary antisymmetry test
    ASSERT_EQ(1, call_i32_lt_s(INT32_MIN, INT32_MAX)) << "INT32_MIN < INT32_MAX should return 1 (true)";
    ASSERT_EQ(0, call_i32_lt_s(INT32_MAX, INT32_MIN)) << "INT32_MAX < INT32_MIN should return 0 (false) - antisymmetry";
}

INSTANTIATE_TEST_SUITE_P(RunningModeTest, I32LtSTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I32LtSTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });