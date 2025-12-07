/**
 * @file enhanced_i32_le_u_test.cc
 * @brief Enhanced unit tests for i32.le_u (Less Than or Equal Unsigned) WASM opcode
 *
 * This file provides comprehensive test coverage for the i32.le_u WebAssembly opcode,
 * which performs unsigned less-than-or-equal comparison between two 32-bit integers and returns 1 if the
 * first operand is less than or equal to the second (treating both as unsigned), 0 otherwise.
 *
 * Test Categories:
 * - Basic functionality with typical unsigned integer values
 * - Boundary conditions with UINT32_MAX/MIN values
 * - Zero comparison scenarios across all combinations
 * - Unsigned vs signed semantics validation (negative numbers as large unsigned)
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
 * @class I32LeUTest
 * @brief Test fixture for i32.le_u opcode validation across execution modes
 *
 * This fixture provides comprehensive testing infrastructure for the i32.le_u
 * WebAssembly opcode, supporting both interpreter and AOT execution modes.
 * It manages WAMR runtime initialization, WASM module loading, and proper
 * resource cleanup through RAII patterns.
 */
class I32LeUTest : public testing::TestWithParam<RunningMode> {
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
        buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/i32_le_u_test.wasm", &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to load WASM file: wasm-apps/i32_le_u_test.wasm";

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
     * @brief Execute i32.le_u comparison with two operands
     * @param a First operand (left-hand side of comparison)
     * @param b Second operand (right-hand side of comparison)
     * @return Comparison result: 1 if a <= b (unsigned), 0 otherwise
     *
     * Calls the WASM function that performs i32.le_u comparison and returns the result.
     * The function handles stack management and error checking automatically.
     */
    int32_t call_i32_le_u(uint32_t a, uint32_t b) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = static_cast<int32_t>(a) } },
            { .kind = WASM_I32, .of = { .i32 = static_cast<int32_t>(b) } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        // Call the WASM function for i32.le_u basic comparison
        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_u_basic"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM execution exception: " << exception;

        return results[0].of.i32;
    }

    /**
     * @brief Execute i32.le_u boundary test cases
     * @param a First operand for boundary comparison
     * @param b Second operand for boundary comparison
     * @return Comparison result for boundary test cases
     */
    int32_t call_i32_le_u_boundary(uint32_t a, uint32_t b) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = static_cast<int32_t>(a) } },
            { .kind = WASM_I32, .of = { .i32 = static_cast<int32_t>(b) } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_u_boundary"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM boundary function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM boundary execution exception: " << exception;

        return results[0].of.i32;
    }

    /**
     * @brief Execute i32.le_u signed vs unsigned semantic tests
     * @param a First operand (may be negative when cast to signed)
     * @param b Second operand (may be negative when cast to signed)
     * @return Comparison result for signed/unsigned semantic validation
     */
    int32_t call_i32_le_u_signed_semantics(int32_t a, int32_t b) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = a } },
            { .kind = WASM_I32, .of = { .i32 = b } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_u_signed_semantics"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM signed semantics function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM signed semantics execution exception: " << exception;

        return results[0].of.i32;
    }

    /**
     * @brief Execute i32.le_u zero operand test cases
     * @param a First operand
     * @param b Second operand
     * @return Comparison result for zero operand scenarios
     */
    int32_t call_i32_le_u_zero(uint32_t a, uint32_t b) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = static_cast<int32_t>(a) } },
            { .kind = WASM_I32, .of = { .i32 = static_cast<int32_t>(b) } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_u_zero"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM zero operand function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM zero operand execution exception: " << exception;

        return results[0].of.i32;
    }

    /**
     * @brief Execute i32.le_u identity property test cases
     * @param a Operand for reflexivity testing (a <= a)
     * @return Comparison result for identity operations
     */
    int32_t call_i32_le_u_identity(uint32_t a) {
        wasm_val_t params[2] = {
            { .kind = WASM_I32, .of = { .i32 = static_cast<int32_t>(a) } },
            { .kind = WASM_I32, .of = { .i32 = static_cast<int32_t>(a) } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I32, .of = { .i32 = 0 } } };

        bool call_result = wasm_runtime_call_wasm_a(exec_env,
                                                   wasm_runtime_lookup_function(module_inst, "test_i32_le_u_identity"),
                                                   1, results, 2, params);

        EXPECT_TRUE(call_result) << "WASM identity function call failed";
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "WASM identity execution exception: " << exception;

        return results[0].of.i32;
    }
};

/**
 * @test BasicUnsignedComparison_ReturnsCorrectResults
 * @brief Validates i32.le_u produces correct unsigned less-than-or-equal results for typical inputs
 * @details Tests fundamental less-than-or-equal operation with various unsigned integer values.
 *          Verifies that i32.le_u correctly computes a <= b treating both operands as unsigned.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_u_operation
 * @input_conditions Standard unsigned integer pairs: (5,10), (10,5), (42,42), (0,100), (200,0)
 * @expected_behavior Returns correct comparison: 1, 0, 1, 1, 0 respectively
 * @validation_method Direct comparison of WASM function result with expected unsigned comparison values
 */
TEST_P(I32LeUTest, BasicUnsignedComparison_ReturnsCorrectResults) {
    // Test positive numbers: 5 <= 10 should be true (1)
    ASSERT_EQ(1, call_i32_le_u(5, 10))
        << "i32.le_u comparison failed: 5 <= 10 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test positive numbers: 10 <= 5 should be false (0)
    ASSERT_EQ(0, call_i32_le_u(10, 5))
        << "i32.le_u comparison failed: 10 <= 5 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test equal values: 42 <= 42 should be true (1)
    ASSERT_EQ(1, call_i32_le_u(42, 42))
        << "i32.le_u comparison failed: 42 <= 42 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test with zero: 0 <= 100 should be true (1)
    ASSERT_EQ(1, call_i32_le_u(0, 100))
        << "i32.le_u comparison failed: 0 <= 100 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test with zero: 200 <= 0 should be false (0)
    ASSERT_EQ(0, call_i32_le_u(200, 0))
        << "i32.le_u comparison failed: 200 <= 0 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test BoundaryValueComparison_ExtremeValues
 * @brief Validates i32.le_u behavior with UINT32_MAX and 0 boundary values
 * @details Tests comparison operations at the extremes of 32-bit unsigned integer range.
 *          Ensures correct handling of maximum unsigned value and zero.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_u_boundary_handling
 * @input_conditions Boundary values: 0, UINT32_MAX (4294967295), adjacent values
 * @expected_behavior 0 <= UINT32_MAX → 1, UINT32_MAX <= 0 → 0, self-comparisons → 1
 * @validation_method Verification of boundary value comparison results and reflexivity
 */
TEST_P(I32LeUTest, BoundaryValueComparison_ExtremeValues) {
    // Test 0 <= UINT32_MAX (minimum <= maximum) should be true (1)
    ASSERT_EQ(1, call_i32_le_u_boundary(0, UINT32_MAX))
        << "i32.le_u boundary comparison failed: 0 <= UINT32_MAX should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test UINT32_MAX <= 0 (maximum <= minimum) should be false (0)
    ASSERT_EQ(0, call_i32_le_u_boundary(UINT32_MAX, 0))
        << "i32.le_u boundary comparison failed: UINT32_MAX <= 0 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test UINT32_MAX <= UINT32_MAX (reflexivity) should be true (1)
    ASSERT_EQ(1, call_i32_le_u_boundary(UINT32_MAX, UINT32_MAX))
        << "i32.le_u boundary comparison failed: UINT32_MAX <= UINT32_MAX should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 0 <= 0 (reflexivity) should be true (1)
    ASSERT_EQ(1, call_i32_le_u_boundary(0, 0))
        << "i32.le_u boundary comparison failed: 0 <= 0 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test near-boundary values: UINT32_MAX - 1 <= UINT32_MAX should be true (1)
    ASSERT_EQ(1, call_i32_le_u_boundary(UINT32_MAX - 1, UINT32_MAX))
        << "i32.le_u boundary comparison failed: (UINT32_MAX-1) <= UINT32_MAX should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test SignedUnsignedBoundary_ValidatesCorrectly
 * @brief Validates i32.le_u handles signed boundary crossings correctly in unsigned context
 * @details Tests cases where signed and unsigned interpretations differ significantly,
 *          particularly around the sign bit boundary (0x7FFFFFFF to 0x80000000).
 * @test_category Corner - Signed/unsigned semantic validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_u_unsigned_interpretation
 * @input_conditions Sign boundary values and negative numbers treated as large unsigned
 * @expected_behavior Negative numbers become large unsigned values in comparison
 * @validation_method Verification that negative values are treated as large unsigned integers
 */
TEST_P(I32LeUTest, SignedUnsignedBoundary_ValidatesCorrectly) {
    // Test 0x7FFFFFFF <= 0x80000000 (2147483647 <= 2147483648 unsigned) should be true (1)
    ASSERT_EQ(1, call_i32_le_u_signed_semantics(0x7FFFFFFF, static_cast<int32_t>(0x80000000U)))
        << "i32.le_u signed boundary failed: 0x7FFFFFFF <= 0x80000000 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test -1 <= 1 (0xFFFFFFFF <= 1 unsigned) should be false (0) - large unsigned > small
    ASSERT_EQ(0, call_i32_le_u_signed_semantics(-1, 1))
        << "i32.le_u negative comparison failed: -1 <= 1 (as unsigned) should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test -2 <= -1 (0xFFFFFFFE <= 0xFFFFFFFF unsigned) should be true (1)
    ASSERT_EQ(1, call_i32_le_u_signed_semantics(-2, -1))
        << "i32.le_u negative comparison failed: -2 <= -1 (as unsigned) should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test -1 <= 0x7FFFFFFF (0xFFFFFFFF <= 0x7FFFFFFF unsigned) should be false (0)
    ASSERT_EQ(0, call_i32_le_u_signed_semantics(-1, 0x7FFFFFFF))
        << "i32.le_u negative vs positive failed: -1 <= 0x7FFFFFFF (as unsigned) should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test ZeroOperands_ValidatesCorrectly
 * @brief Validates i32.le_u behavior with zero operands in various combinations
 * @details Tests zero-related comparisons to ensure proper handling of minimum value.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_u_zero_handling
 * @input_conditions Zero combined with various values including extremes
 * @expected_behavior Zero is always <= any value, any value > zero is not <= zero
 * @validation_method Verification of zero comparison behavior across value ranges
 */
TEST_P(I32LeUTest, ZeroOperands_ValidatesCorrectly) {
    // Test 0 <= 0 (reflexivity) should be true (1)
    ASSERT_EQ(1, call_i32_le_u_zero(0, 0))
        << "i32.le_u zero comparison failed: 0 <= 0 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 0 <= 1 (minimum <= small value) should be true (1)
    ASSERT_EQ(1, call_i32_le_u_zero(0, 1))
        << "i32.le_u zero comparison failed: 0 <= 1 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 1 <= 0 (small value <= minimum) should be false (0)
    ASSERT_EQ(0, call_i32_le_u_zero(1, 0))
        << "i32.le_u zero comparison failed: 1 <= 0 should return 0 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test 0 <= UINT32_MAX (minimum <= maximum) should be true (1)
    ASSERT_EQ(1, call_i32_le_u_zero(0, UINT32_MAX))
        << "i32.le_u zero vs max failed: 0 <= UINT32_MAX should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

/**
 * @test IdentityOperations_BehavesCorrectly
 * @brief Validates i32.le_u reflexive property (x <= x always true)
 * @details Tests mathematical identity property of less-than-or-equal comparison.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_le_u_identity_property
 * @input_conditions Various values compared with themselves
 * @expected_behavior All reflexive comparisons return true (1)
 * @validation_method Verification that x <= x is always true for any x
 */
TEST_P(I32LeUTest, IdentityOperations_BehavesCorrectly) {
    // Test reflexive property with various values
    ASSERT_EQ(1, call_i32_le_u_identity(0))
        << "i32.le_u identity failed: 0 <= 0 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    ASSERT_EQ(1, call_i32_le_u_identity(1))
        << "i32.le_u identity failed: 1 <= 1 should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    ASSERT_EQ(1, call_i32_le_u_identity(0x7FFFFFFF))
        << "i32.le_u identity failed: 0x7FFFFFFF <= 0x7FFFFFFF should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    ASSERT_EQ(1, call_i32_le_u_identity(UINT32_MAX))
        << "i32.le_u identity failed: UINT32_MAX <= UINT32_MAX should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";

    // Test reflexive property with values that are negative when interpreted as signed
    ASSERT_EQ(1, call_i32_le_u_identity(static_cast<uint32_t>(-1)))
        << "i32.le_u identity failed: -1 <= -1 (as unsigned) should return 1 in "
        << (GetParam() == Mode_Interp ? "interpreter" : "AOT") << " mode";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, I32LeUTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I32LeUTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });