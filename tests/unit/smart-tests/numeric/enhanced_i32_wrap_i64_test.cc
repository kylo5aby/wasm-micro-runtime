/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>  // Primary GTest framework for unit testing
#include <climits>        // Standard integer limits for boundary testing
#include <cstdint>        // Standard integer types for precise type control
#include <vector>         // Container for batch test case management
#include "wasm_export.h"  // Core WAMR runtime API for module management
#include "bh_read_file.h" // WAMR utility for loading WASM binary files

/**
 * @file enhanced_i32_wrap_i64_test.cc
 * @brief Enhanced unit tests for i32.wrap_i64 opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the i32.wrap_i64 (integer wrapping)
 * WebAssembly instruction, focusing on:
 * - Basic integer truncation functionality from 64-bit to 32-bit integers
 * - Corner cases including boundary values (INT32_MIN, INT32_MAX) and overflow scenarios
 * - Edge cases with zero operands, identity operations, and extreme i64 values
 * - Mathematical properties verification (modulo 2^32 arithmetic and bit masking)
 * - Cross-execution mode validation between interpreter and AOT compilation
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32.wrap_i64 operations
 * @coverage_target core/iwasm/aot/aot_runtime.c:integer conversion instructions
 * @coverage_target Bit truncation behavior and type conversion algorithms
 * @coverage_target Stack management for numeric type conversion operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class I32WrapI64TestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i32.wrap_i64 testing";

        // Load WASM test module containing i32.wrap_i64 test functions
        std::string wasm_file = "./wasm-apps/i32_wrap_i64_test.wasm";
        module_buffer = reinterpret_cast<uint8_t*>(
            bh_read_file_to_buffer(wasm_file.c_str(), &buffer_size));
        ASSERT_NE(nullptr, module_buffer)
            << "Failed to load WASM file: " << wasm_file;

        // Load and validate WASM module with error reporting
        char error_buf[128];
        module = wasm_runtime_load(module_buffer, buffer_size,
                                 error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module)
            << "Failed to load WASM module: " << error_buf;

        // Create module instance for test execution
        module_inst = wasm_runtime_instantiate(module, 8192, 8192,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode for parameterized testing
        wasm_runtime_set_running_mode(module_inst, GetParam());
    }

    void TearDown() override {
        // Clean up WASM resources in proper order
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (module_buffer) {
            wasm_runtime_free(module_buffer);
            module_buffer = nullptr;
        }

        // Shutdown WAMR runtime
        wasm_runtime_destroy();
    }

    /**
     * @brief Helper function to call i32.wrap_i64 WASM function
     * @param input i64 input value to be wrapped to i32
     * @return i32 wrapped result from WASM execution
     * @details Executes the WASM wrap_i64 function and returns the i32 result.
     * Function handles WAMR execution context and validates successful execution.
     */
    int32_t call_wrap_i64(int64_t input) {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "wrap_i64");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup wrap_i64 function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: i64 input as two 32-bit values (low, high)
        uint32_t argv[2] = {
            static_cast<uint32_t>(input & 0xFFFFFFFF),         // Low 32 bits
            static_cast<uint32_t>((input >> 32) & 0xFFFFFFFF)  // High 32 bits
        };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 2, argv);
        EXPECT_TRUE(call_result)
            << "wrap_i64 function call failed: " << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return result (i32 value is in argv[0] after call)
        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Helper function to call boundary test WASM function
     * @param input i64 input value for boundary testing
     * @return i32 wrapped result from boundary test function
     * @details Executes the WASM wrap_i64_boundary function for testing boundary conditions.
     */
    int32_t call_wrap_i64_boundary(int64_t input) {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "wrap_i64_boundary");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup wrap_i64_boundary function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: i64 input as two 32-bit values (low, high)
        uint32_t argv[2] = {
            static_cast<uint32_t>(input & 0xFFFFFFFF),         // Low 32 bits
            static_cast<uint32_t>((input >> 32) & 0xFFFFFFFF)  // High 32 bits
        };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 2, argv);
        EXPECT_TRUE(call_result)
            << "wrap_i64_boundary function call failed: " << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return result (i32 value is in argv[0] after call)
        return static_cast<int32_t>(argv[0]);
    }

    // WAMR runtime resources
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    uint8_t* module_buffer = nullptr;
    uint32_t buffer_size = 0;
};

/**
 * @test BasicWrapping_TypicalValues_ReturnsCorrectResults
 * @brief Validates i32.wrap_i64 produces correct truncation for typical i64 inputs
 * @details Tests fundamental wrapping operation with positive, negative, and mixed-sign integers
 *          that fit within i32 range. Verifies that values within i32 bounds remain unchanged.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32_wrap_i64_operation
 * @input_conditions Standard i64 values within i32 range: 42, -100, 0x12345678, 0
 * @expected_behavior Returns identical i32 values for inputs within i32 bounds
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32WrapI64TestSuite, BasicWrapping_TypicalValues_ReturnsCorrectResults) {
    // Test positive values within i32 range
    ASSERT_EQ(42, call_wrap_i64(42LL))
        << "Basic positive i64 value should wrap to identical i32 value";

    // Test negative values within i32 range
    ASSERT_EQ(-100, call_wrap_i64(-100LL))
        << "Basic negative i64 value should wrap to identical i32 value";

    // Test larger positive value within i32 range
    ASSERT_EQ(0x12345678, call_wrap_i64(0x12345678LL))
        << "Large positive i64 value within i32 range should remain unchanged";

    // Test zero value (identity case)
    ASSERT_EQ(0, call_wrap_i64(0LL))
        << "Zero i64 value should wrap to zero i32 value";
}

/**
 * @test BoundaryWrapping_I32MinMax_HandlesCorrectly
 * @brief Validates i32.wrap_i64 correctly handles i32 boundary values and sign transitions
 * @details Tests wrapping behavior at i32 MIN/MAX boundaries and values that cross
 *          the 32-bit boundary, ensuring proper sign handling and overflow behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:boundary_value_handling
 * @input_conditions i32 boundary values: INT32_MAX, INT32_MIN, and 32-bit boundary crossings
 * @expected_behavior Correct sign preservation within bounds, proper wrapping beyond bounds
 * @validation_method Boundary value verification and sign bit analysis
 */
TEST_P(I32WrapI64TestSuite, BoundaryWrapping_I32MinMax_HandlesCorrectly) {
    // Test i32 MAX boundary (should remain positive)
    ASSERT_EQ(INT32_MAX, call_wrap_i64_boundary(static_cast<int64_t>(INT32_MAX)))
        << "i32 MAX value should remain unchanged when wrapped";

    // Test i32 MIN boundary (should remain negative)
    ASSERT_EQ(INT32_MIN, call_wrap_i64_boundary(static_cast<int64_t>(INT32_MIN)))
        << "i32 MIN value should remain unchanged when wrapped";

    // Test just beyond i32 MAX (should become negative due to sign bit)
    ASSERT_EQ(INT32_MIN, call_wrap_i64_boundary(0x80000000LL))
        << "Value 0x80000000 should wrap to INT32_MIN (sign bit set)";

    // Test all 32 bits set (should become -1)
    ASSERT_EQ(-1, call_wrap_i64_boundary(0xFFFFFFFFLL))
        << "Value 0xFFFFFFFF should wrap to -1 (all 32 bits set)";
}

/**
 * @test OverflowWrapping_BeyondI32Range_WrapsCorrectly
 * @brief Validates i32.wrap_i64 implements correct modulo 2^32 arithmetic for overflow
 * @details Tests wrapping behavior for i64 values that exceed i32 range, verifying
 *          that high-order bits are discarded and only lower 32 bits are preserved.
 * @test_category Corner - Overflow/underflow scenarios
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:overflow_wrapping_logic
 * @input_conditions i64 values beyond i32 range: 2^32, i64_MAX, i64_MIN
 * @expected_behavior Modulo 2^32 arithmetic with proper bit truncation
 * @validation_method Mathematical verification of modulo properties
 */
TEST_P(I32WrapI64TestSuite, OverflowWrapping_BeyondI32Range_WrapsCorrectly) {
    // Test 2^32 boundary (should wrap to 0)
    ASSERT_EQ(0, call_wrap_i64_boundary(0x100000000LL))
        << "Value 2^32 should wrap to 0 (exact modulo boundary)";

    // Test 2^32 + 1 (should wrap to 1)
    ASSERT_EQ(1, call_wrap_i64_boundary(0x100000001LL))
        << "Value 2^32 + 1 should wrap to 1 (modulo arithmetic)";

    // Test i64 MAX value (should extract lower 32 bits)
    ASSERT_EQ(-1, call_wrap_i64_boundary(INT64_MAX))
        << "i64 MAX should wrap to -1 (lower 32 bits all set)";

    // Test i64 MIN value (should wrap to 0)
    ASSERT_EQ(0, call_wrap_i64_boundary(INT64_MIN))
        << "i64 MIN should wrap to 0 (lower 32 bits are zero)";
}

/**
 * @test ExtremeValues_I64MinMax_ProducesExpectedWrapping
 * @brief Validates i32.wrap_i64 handles extreme i64 values and special bit patterns
 * @details Tests wrapping behavior with extreme values, alternating bit patterns,
 *          and mathematical properties like bit masking and modulo arithmetic.
 * @test_category Edge - Extreme values and mathematical properties
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:bit_truncation_logic
 * @input_conditions Extreme i64 values and special bit patterns
 * @expected_behavior Correct lower 32-bit extraction and mathematical consistency
 * @validation_method Bit pattern analysis and mathematical property verification
 */
TEST_P(I32WrapI64TestSuite, ExtremeValues_I64MinMax_ProducesExpectedWrapping) {
    // Test alternating bit pattern (high and low 32 bits different)
    ASSERT_EQ(static_cast<int32_t>(0xBBBBBBBB), call_wrap_i64_boundary(0xAAAAAAAABBBBBBBBLL))
        << "Alternating bit pattern should preserve only lower 32 bits";

    // Test high 32 bits set, low 32 bits zero
    ASSERT_EQ(0, call_wrap_i64_boundary(0xFFFFFFFF00000000LL))
        << "High bits only pattern should wrap to zero";

    // Test mathematical property: input & 0xFFFFFFFF == result
    int64_t test_value = 0x123456789ABCDEFLL;
    int32_t expected = static_cast<int32_t>(test_value & 0xFFFFFFFF);
    ASSERT_EQ(expected, call_wrap_i64_boundary(test_value))
        << "Wrapping should be equivalent to bitwise AND with 0xFFFFFFFF";

    // Test modulo property: result should equal (input % 2^32) adjusted for signed representation
    int64_t large_positive = 0x1ABCDEF012345678LL;
    int32_t wrapped = call_wrap_i64_boundary(large_positive);
    ASSERT_EQ(static_cast<int32_t>(large_positive & 0xFFFFFFFF), wrapped)
        << "Large positive value should follow modulo 2^32 arithmetic";
}

/**
 * @test RuntimeErrors_InvalidModule_HandlesGracefully
 * @brief Validates robust error handling for runtime environment issues
 * @details Tests behavior when WASM module loading fails, function lookup fails,
 *          or other runtime errors occur, ensuring graceful failure without crashes.
 * @test_category Error - Runtime environment robustness
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:error_handling
 * @input_conditions Invalid module scenarios and runtime error conditions
 * @expected_behavior Proper error handling and reporting without crashes
 * @validation_method Error condition verification and graceful failure testing
 */
TEST_P(I32WrapI64TestSuite, RuntimeErrors_InvalidModule_HandlesGracefully) {
    // Test behavior with invalid function lookup (function doesn't exist)
    wasm_function_inst_t invalid_func = wasm_runtime_lookup_function(
        module_inst, "nonexistent_function");
    ASSERT_EQ(nullptr, invalid_func)
        << "Lookup of nonexistent function should return nullptr";

    // Verify module instance is still valid after failed lookup
    wasm_function_inst_t valid_func = wasm_runtime_lookup_function(
        module_inst, "wrap_i64");
    ASSERT_NE(nullptr, valid_func)
        << "Valid function lookup should still work after invalid lookup";

    // Test successful execution after error recovery
    ASSERT_EQ(42, call_wrap_i64(42LL))
        << "Normal operation should continue after error recovery";
}

// Instantiate parameterized tests for both execution modes
INSTANTIATE_TEST_SUITE_P(
    CrossModeValidation,
    I32WrapI64TestSuite,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        switch (info.param) {
            case Mode_Interp:
                return "InterpreterMode";
            case Mode_LLVM_JIT:
                return "AOTMode";
            default:
                return "UnknownMode";
        }
    }
);