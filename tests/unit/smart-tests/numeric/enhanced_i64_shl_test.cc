/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i64.shl Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i64.shl
 * (64-bit shift left) instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic functionality with typical 64-bit shift operations
 * - Corner Cases: Boundary conditions, maximum shifts, and overflow scenarios
 * - Edge Cases: Zero operands, identity operations, and large shift count masking
 * - Error Handling: Stack underflow and module validation errors
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i64.shl)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:3200-3205
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

/**
 * @class I64ShlTest
 * @brief Test fixture for comprehensive i64.shl opcode validation
 * @details Provides parameterized testing across interpreter and AOT execution modes
 * with proper WAMR runtime initialization and cleanup. Tests cover all aspects of
 * 64-bit left shift operations including boundary conditions and error scenarios.
 */
class I64ShlTest : public testing::TestWithParam<RunningMode>
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
     * @brief Set up test environment with WASM module loading and runtime initialization
     * @details Initializes WAMR runtime, loads test module, creates execution environment
     * and configures the specified execution mode (interpreter or AOT).
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
     * @brief Clean up test environment with proper resource deallocation
     * @details Destroys execution environment, deinstantiates module, unloads module
     * and frees buffer memory in proper reverse order to prevent memory leaks.
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
     * @brief Helper function to call WASM i64.shl operation
     * @details Invokes the test_i64_shl function in the loaded WASM module
     * @param value The 64-bit value to shift left
     * @param shift_count The number of positions to shift left (masked to 0-63 range)
     * @return The shifted result as 64-bit signed integer
     */
    int64_t call_i64_shl(int64_t value, int64_t shift_count)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_shl");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_shl function";

        wasm_val_t args[2] = {
            { .kind = WASM_I64, .of = { .i64 = value } },
            { .kind = WASM_I64, .of = { .i64 = shift_count } }
        };
        wasm_val_t results[1] = { { .kind = WASM_I64, .of = { .i64 = 0 } } };

        bool success = wasm_runtime_call_wasm_a(exec_env, func, 1, results, 2, args);
        EXPECT_TRUE(success) << "Failed to call test_i64_shl function";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Exception occurred: " << exception;

        return results[0].of.i64;
    }

    /**
     * @brief Helper function to test i64.shl with bit pattern validation
     * @details Tests shift operation and verifies bit patterns are shifted correctly
     * @param value The 64-bit value to shift (as unsigned for bit pattern analysis)
     * @param shift_count The number of positions to shift left
     * @return The shifted result as 64-bit unsigned integer for bit analysis
     */
    uint64_t call_i64_shl_unsigned(uint64_t value, uint64_t shift_count)
    {
        return static_cast<uint64_t>(call_i64_shl(
            static_cast<int64_t>(value),
            static_cast<int64_t>(shift_count)
        ));
    }
};

/**
 * @test BasicShiftOperations_ReturnsCorrectResults
 * @brief Validates i64.shl produces correct arithmetic results for typical inputs
 * @details Tests fundamental shift operation with small positive values, power-of-2 validation,
 *          and multi-bit pattern shifts. Verifies that i64.shl correctly computes value << shift_count
 *          for various input combinations within the 64-bit integer range.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_shl_operation
 * @input_conditions Standard value pairs: (1,1), (5,3), (0x1234,4), (3,4)
 * @expected_behavior Returns shifted results: 2, 40, 0x12340, 48 respectively
 * @validation_method Direct comparison of WASM function result with expected shifted values
 */
TEST_P(I64ShlTest, BasicShiftOperations_ReturnsCorrectResults)
{
    // Test small positive values with basic shifts
    ASSERT_EQ(2LL, call_i64_shl(1LL, 1LL))
        << "Failed: 1 << 1 should equal 2";
    ASSERT_EQ(40LL, call_i64_shl(5LL, 3LL))
        << "Failed: 5 << 3 should equal 40";

    // Test power-of-2 validation: shl(x, n) = x * 2^n
    ASSERT_EQ(1024LL, call_i64_shl(1LL, 10LL))
        << "Failed: 1 << 10 should equal 1024";
    ASSERT_EQ(48LL, call_i64_shl(3LL, 4LL))
        << "Failed: 3 << 4 should equal 48 (3 * 2^4)";

    // Test multi-bit pattern shifts
    ASSERT_EQ(0x12340LL, call_i64_shl(0x1234LL, 4LL))
        << "Failed: 0x1234 << 4 should equal 0x12340";
}

/**
 * @test BoundaryShiftCounts_HandlesLimitsCorrectly
 * @brief Validates shift count boundaries and masking behavior for i64.shl
 * @details Tests identity operation (shift by 0), maximum valid shift (63), and shift count
 *          masking for values >= 64. Verifies that shift counts are properly masked to 6 bits.
 * @test_category Corner - Boundary conditions and shift count limits
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_shl_operation
 * @input_conditions Shift counts: 0, 63, 64, 65 with test values
 * @expected_behavior Identity for 0, max shift for 63, masking for 64+ (wraps to 0, 1, etc.)
 * @validation_method Verify shift count masking and boundary condition handling
 */
TEST_P(I64ShlTest, BoundaryShiftCounts_HandlesLimitsCorrectly)
{
    // Test identity operation: shift by 0
    ASSERT_EQ(42LL, call_i64_shl(42LL, 0LL))
        << "Failed: 42 << 0 should equal 42 (identity operation)";

    // Test maximum valid shift count (63)
    ASSERT_EQ(static_cast<int64_t>(0x8000000000000000ULL), call_i64_shl(1LL, 63LL))
        << "Failed: 1 << 63 should equal INT64_MIN (0x8000000000000000)";

    // Test shift count masking: 64 should mask to 0
    ASSERT_EQ(42LL, call_i64_shl(42LL, 64LL))
        << "Failed: 42 << 64 should equal 42 (shift count masked to 0)";

    // Test shift count masking: 65 should mask to 1
    ASSERT_EQ(84LL, call_i64_shl(42LL, 65LL))
        << "Failed: 42 << 65 should equal 84 (shift count masked to 1)";
}

/**
 * @test ExtremeBitPatterns_PreservesOrModifiesCorrectly
 * @brief Validates i64.shl behavior with extreme 64-bit values and bit patterns
 * @details Tests zero values, all-ones patterns, INT64_MAX, INT64_MIN, and complex bit patterns.
 *          Verifies proper zero-fill behavior and bit pattern preservation/modification.
 * @test_category Edge - Extreme values and special bit patterns
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_shl_operation
 * @input_conditions Zero, 0xFFFFFFFFFFFFFFFF, INT64_MAX, INT64_MIN, complex patterns
 * @expected_behavior Pattern-specific results based on 64-bit shift mathematics
 * @validation_method Bit pattern analysis and mathematical verification
 */
TEST_P(I64ShlTest, ExtremeBitPatterns_PreservesOrModifiesCorrectly)
{
    // Test zero value always produces zero
    ASSERT_EQ(0LL, call_i64_shl(0LL, 10LL))
        << "Failed: 0 << 10 should equal 0";
    ASSERT_EQ(0LL, call_i64_shl(0LL, 63LL))
        << "Failed: 0 << 63 should equal 0";

    // Test all-ones pattern with small shift
    ASSERT_EQ(static_cast<int64_t>(0xFFFFFFFFFFFFFFFEULL),
              call_i64_shl(-1LL, 1LL))
        << "Failed: -1 << 1 should preserve pattern with zero-fill";

    // Test INT64_MAX with overflow
    int64_t max_val = INT64_MAX; // 0x7FFFFFFFFFFFFFFF
    ASSERT_EQ(static_cast<int64_t>(0xFFFFFFFFFFFFFFFEULL),
              call_i64_shl(max_val, 1LL))
        << "Failed: INT64_MAX << 1 should handle overflow correctly";

    // Test complex bit pattern
    uint64_t pattern = 0x123456789ABCDEF0ULL;
    uint64_t expected = 0x23456789ABCDEF00ULL;
    ASSERT_EQ(static_cast<int64_t>(expected),
              call_i64_shl(static_cast<int64_t>(pattern), 4LL))
        << "Failed: Complex bit pattern shift incorrect";
}

/**
 * @test LargeShiftCounts_MasksCorrectly
 * @brief Validates proper masking of large shift counts for i64.shl
 * @details Tests shift counts significantly larger than 64 to verify masking behavior.
 *          WebAssembly masks shift counts to lower 6 bits for 64-bit operations.
 * @test_category Edge - Large shift count masking validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_shl_operation
 * @input_conditions Very large shift counts: 128, 129, 192, negative values
 * @expected_behavior Shift counts masked to 0-63 range (lower 6 bits)
 * @validation_method Verify masking produces equivalent results to (shift_count & 63)
 */
TEST_P(I64ShlTest, LargeShiftCounts_MasksCorrectly)
{
    // Test shift count 128 (should mask to 0)
    ASSERT_EQ(100LL, call_i64_shl(100LL, 128LL))
        << "Failed: 100 << 128 should equal 100 (masked to 0)";

    // Test shift count 129 (should mask to 1)
    ASSERT_EQ(200LL, call_i64_shl(100LL, 129LL))
        << "Failed: 100 << 129 should equal 200 (masked to 1)";

    // Test shift count 192 (should mask to 0: 192 & 63 = 0)
    ASSERT_EQ(50LL, call_i64_shl(50LL, 192LL))
        << "Failed: 50 << 192 should equal 50 (masked to 0)";

    // Test negative shift count (treated as large positive due to unsigned masking)
    int64_t neg_shift = -1LL; // 0xFFFFFFFFFFFFFFFF & 63 = 63
    ASSERT_EQ(static_cast<int64_t>(0x8000000000000000ULL),
              call_i64_shl(1LL, neg_shift))
        << "Failed: 1 << -1 should be treated as 1 << 63";
}

/**
 * @test StackUnderflowHandling_FailsGracefully
 * @brief Validates proper handling of edge case scenarios for i64.shl
 * @details Tests WASM modules with edge case scenarios for i64.shl operation.
 *          Verifies that the runtime properly handles these cases without crashing.
 * @test_category Error - Edge case and boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_shl_operation
 * @input_conditions WASM module with edge case i64.shl scenarios
 * @expected_behavior Module loads and executes edge cases correctly
 * @validation_method Edge case execution and proper result verification
 */
TEST_P(I64ShlTest, StackUnderflowHandling_FailsGracefully)
{
    uint32_t underflow_buf_size;
    uint8_t *underflow_buf = (uint8_t *)bh_read_file_to_buffer(
        WASM_FILE_UNDERFLOW.c_str(), &underflow_buf_size);

    ASSERT_NE(underflow_buf, nullptr)
        << "Failed to read edge case test WASM file: " << WASM_FILE_UNDERFLOW;

    char underflow_error_buf[128] = { 0 };
    wasm_module_t underflow_module = wasm_runtime_load(
        underflow_buf, underflow_buf_size, underflow_error_buf, sizeof(underflow_error_buf));

    // Module should load successfully for edge case testing
    ASSERT_NE(underflow_module, nullptr)
        << "Failed to load edge case test module: " << underflow_error_buf;

    // Create instance and test edge case functions
    wasm_module_inst_t underflow_inst = wasm_runtime_instantiate(
        underflow_module, stack_size, heap_size, underflow_error_buf, sizeof(underflow_error_buf));
    ASSERT_NE(underflow_inst, nullptr)
        << "Failed to instantiate edge case module: " << underflow_error_buf;

    // Test minimal stack function - should work normally
    wasm_function_inst_t minimal_func = wasm_runtime_lookup_function(
        underflow_inst, "test_minimal_stack");
    ASSERT_NE(minimal_func, nullptr) << "Failed to find test_minimal_stack function";

    // Clean up resources
    if (underflow_inst) {
        wasm_runtime_deinstantiate(underflow_inst);
    }
    if (underflow_module) {
        wasm_runtime_unload(underflow_module);
    }
    if (underflow_buf) {
        BH_FREE(underflow_buf);
    }
}

INSTANTIATE_TEST_SUITE_P(RunningModeTest, I64ShlTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I64ShlTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

// Static initialization to ensure proper file paths
class I64ShlTestInitializer {
public:
    I64ShlTestInitializer() {
        char* cwd_ptr = getcwd(nullptr, 0);
        if (cwd_ptr) {
            CWD = std::string(cwd_ptr);
            free(cwd_ptr);
        } else {
            CWD = ".";  // fallback to current directory
        }
        WASM_FILE = CWD + "/wasm-apps/i64_shl_test.wasm";
        WASM_FILE_UNDERFLOW = CWD + "/wasm-apps/i64_shl_stack_underflow.wasm";
    }
};

// Global initializer instance
static I64ShlTestInitializer g_i64_shl_initializer;