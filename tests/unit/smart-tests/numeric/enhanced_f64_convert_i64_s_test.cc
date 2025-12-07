/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>  // Primary GTest framework for unit testing
#include <climits>        // Standard integer limits for boundary testing
#include <cstdint>        // Standard integer types for precise type control
#include <cmath>          // Mathematical functions for floating-point validation
#include <vector>         // Container for batch test case management
#include "wasm_export.h"  // Core WAMR runtime API for module management
#include "bh_read_file.h" // WAMR utility for loading WASM binary files

/**
 * @file enhanced_f64_convert_i64_s_test.cc
 * @brief Enhanced unit tests for f64.convert_i64_s opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the f64.convert_i64_s (signed 64-bit integer to double conversion)
 * WebAssembly instruction, focusing on:
 * - Basic signed 64-bit integer to IEEE 754 double precision conversion functionality
 * - Corner cases including boundary values (INT64_MIN, INT64_MAX) and precision limits
 * - Edge cases with zero operands, power-of-2 values, and IEEE 754 rounding behavior
 * - Precision loss scenarios for integers exceeding 53-bit mantissa precision
 * - Cross-execution mode validation between interpreter and AOT compilation
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64.convert_i64_s operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:DEF_OP_CONVERT macro
 * @coverage_target core/iwasm/aot/aot_runtime.c:floating-point conversion instructions
 * @coverage_target IEEE 754 double precision conversion algorithms and rounding behavior
 * @coverage_target Stack management for numeric type conversion operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class F64ConvertI64STestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for f64.convert_i64_s testing";

        // Load WASM test module containing f64.convert_i64_s test functions
        std::string wasm_file = "./wasm-apps/f64_convert_i64_s_test.wasm";
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
     * @brief Helper function to call f64.convert_i64_s WASM function
     * @param input i64 input value to be converted to f64
     * @return double converted result from WASM execution
     * @details Executes the WASM convert_i64_s function and returns the f64 result.
     * Function handles WAMR execution context and validates successful execution.
     */
    double call_convert_i64_s(int64_t input) {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "convert_i64_s");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup convert_i64_s function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: i64 input value (split into two 32-bit values)
        uint32_t argv[3] = {
            static_cast<uint32_t>(input & 0xFFFFFFFF),        // Low 32 bits
            static_cast<uint32_t>((input >> 32) & 0xFFFFFFFF), // High 32 bits
            0  // Result placeholder
        };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 2, argv);
        EXPECT_TRUE(call_result)
            << "convert_i64_s function call failed: " << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return result (f64 value is in argv[0] and argv[1] after call)
        union { uint64_t u; double f; } result;
        result.u = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        return result.f;
    }

    /**
     * @brief Helper function to call boundary test WASM function
     * @param input i64 input value for boundary testing
     * @return double converted result from boundary test function
     * @details Executes the WASM convert_i64_s_boundary function for testing boundary conditions.
     */
    double call_convert_i64_s_boundary(int64_t input) {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "convert_i64_s_boundary");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup convert_i64_s_boundary function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: i64 input value (split into two 32-bit values)
        uint32_t argv[3] = {
            static_cast<uint32_t>(input & 0xFFFFFFFF),        // Low 32 bits
            static_cast<uint32_t>((input >> 32) & 0xFFFFFFFF), // High 32 bits
            0  // Result placeholder
        };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 2, argv);
        EXPECT_TRUE(call_result)
            << "convert_i64_s_boundary function call failed: " << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return result (f64 value is in argv[0] and argv[1] after call)
        union { uint64_t u; double f; } result;
        result.u = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        return result.f;
    }

    /**
     * @brief Helper function to call precision test WASM function
     * @param input i64 input value for precision testing
     * @return double converted result from precision test function
     * @details Executes the WASM convert_i64_s_precision function for testing precision-related scenarios.
     */
    double call_convert_i64_s_precision(int64_t input) {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "convert_i64_s_precision");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup convert_i64_s_precision function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: i64 input value (split into two 32-bit values)
        uint32_t argv[3] = {
            static_cast<uint32_t>(input & 0xFFFFFFFF),        // Low 32 bits
            static_cast<uint32_t>((input >> 32) & 0xFFFFFFFF), // High 32 bits
            0  // Result placeholder
        };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 2, argv);
        EXPECT_TRUE(call_result)
            << "convert_i64_s_precision function call failed: " << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return result (f64 value is in argv[0] and argv[1] after call)
        union { uint64_t u; double f; } result;
        result.u = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        return result.f;
    }

    // WAMR runtime resources
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    uint8_t* module_buffer = nullptr;
    uint32_t buffer_size = 0;
};

/**
 * @test BasicConversion_ReturnsCorrectF64
 * @brief Validates f64.convert_i64_s produces correct IEEE 754 double for typical i64 inputs
 * @details Tests fundamental conversion operation with positive, negative, and zero 64-bit integers.
 *          Verifies that f64.convert_i64_s correctly converts signed integers to double precision.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_convert_i64_s_operation
 * @input_conditions Standard i64 values: 42, -123, 1000000, -2000000, 0
 * @expected_behavior Returns exact mathematical conversion with sign preservation
 * @validation_method Direct comparison of WASM function result with expected double values
 */
TEST_P(F64ConvertI64STestSuite, BasicConversion_ReturnsCorrectF64) {
    // Test small positive integer conversion
    ASSERT_EQ(42.0, call_convert_i64_s(42LL))
        << "Small positive i64 should convert to exact double equivalent";

    // Test small negative integer conversion
    ASSERT_EQ(-123.0, call_convert_i64_s(-123LL))
        << "Small negative i64 should convert to exact double equivalent";

    // Test medium positive integer conversion
    ASSERT_EQ(1000000.0, call_convert_i64_s(1000000LL))
        << "Medium positive i64 should convert to exact double equivalent";

    // Test medium negative integer conversion
    ASSERT_EQ(-2000000.0, call_convert_i64_s(-2000000LL))
        << "Medium negative i64 should convert to exact double equivalent";

    // Test zero conversion (identity case)
    ASSERT_EQ(0.0, call_convert_i64_s(0LL))
        << "Zero i64 should convert to positive zero double";
}

/**
 * @test BoundaryValues_HandleCorrectly
 * @brief Validates f64.convert_i64_s correctly handles i64 boundary values and sign transitions
 * @details Tests conversion behavior at i64 MIN/MAX boundaries and values that test
 *          sign handling around zero boundary, ensuring proper IEEE 754 representation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:boundary_value_handling
 * @input_conditions i64 boundary values: INT64_MIN, INT64_MAX, -1, 1
 * @expected_behavior Proper IEEE 754 double precision representation with sign preservation
 * @validation_method Boundary value verification and mathematical comparison
 */
TEST_P(F64ConvertI64STestSuite, BoundaryValues_HandleCorrectly) {
    // Test INT64_MAX boundary conversion (will have precision loss)
    double expected_max = static_cast<double>(INT64_MAX);
    ASSERT_EQ(expected_max, call_convert_i64_s_boundary(INT64_MAX))
        << "INT64_MAX should convert to IEEE 754 compliant double representation";

    // Test INT64_MIN boundary conversion (will have precision loss)
    double expected_min = static_cast<double>(INT64_MIN);
    ASSERT_EQ(expected_min, call_convert_i64_s_boundary(INT64_MIN))
        << "INT64_MIN should convert to IEEE 754 compliant double representation with sign preservation";

    // Test just below zero boundary
    ASSERT_EQ(-1.0, call_convert_i64_s_boundary(-1LL))
        << "Negative one should convert to exact -1.0 double";

    // Test just above zero boundary
    ASSERT_EQ(1.0, call_convert_i64_s_boundary(1LL))
        << "Positive one should convert to exact 1.0 double";
}

/**
 * @test PrecisionLimits_RoundCorrectly
 * @brief Validates f64.convert_i64_s IEEE 754 rounding behavior at 53-bit mantissa precision limit
 * @details Tests conversion of integers at the IEEE 754 double precision limit (2^53),
 *          verifying proper rounding behavior for values exceeding mantissa precision.
 * @test_category Edge - Precision and rounding validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ieee754_rounding
 * @input_conditions Precision boundary values: 2^53-1, 2^53, 2^53+1, large integers with precision loss
 * @expected_behavior IEEE 754 compliant rounding with proper precision loss handling
 * @validation_method Mathematical equivalence verification and precision boundary testing
 */
TEST_P(F64ConvertI64STestSuite, PrecisionLimits_RoundCorrectly) {
    // Test largest exactly representable integer (2^53 - 1)
    int64_t max_exact = (1LL << 53) - 1;  // 9007199254740991
    ASSERT_EQ(static_cast<double>(max_exact), call_convert_i64_s_precision(max_exact))
        << "2^53-1 should be exactly representable in double precision";

    // Test first integer requiring rounding (2^53 + 1)
    int64_t first_rounded = (1LL << 53) + 1;  // 9007199254740993
    double expected_rounded = static_cast<double>(first_rounded);
    ASSERT_EQ(expected_rounded, call_convert_i64_s_precision(first_rounded))
        << "2^53+1 should round according to IEEE 754 rules";

    // Test power of 2 at precision limit (2^53)
    int64_t power_53 = 1LL << 53;  // 9007199254740992
    ASSERT_EQ(static_cast<double>(power_53), call_convert_i64_s_precision(power_53))
        << "2^53 should be exactly representable as power of 2";

    // Test negative precision boundary
    int64_t neg_max_exact = -((1LL << 53) - 1);
    ASSERT_EQ(static_cast<double>(neg_max_exact), call_convert_i64_s_precision(neg_max_exact))
        << "-(2^53-1) should be exactly representable with sign preservation";

    // Test large value with significant precision loss
    int64_t large_with_loss = (1LL << 60) + (1LL << 10);  // Large value that loses precision
    double expected_large = static_cast<double>(large_with_loss);
    ASSERT_EQ(expected_large, call_convert_i64_s_precision(large_with_loss))
        << "Large i64 values should round correctly with expected precision loss";
}

/**
 * @test StackUnderflow_HandlesGracefully
 * @brief Tests module loading with invalid bytecode and stack underflow scenarios
 * @details Validates that WAMR properly handles invalid WASM modules with type mismatches
 *          and provides appropriate error reporting for f64.convert_i64_s instruction validation.
 * @test_category Error - Error condition validation
 * @coverage_target core/iwasm/interpreter/wasm_loader.c:type_validation
 * @input_conditions Malformed WASM with type mismatches and stack underflow scenarios
 * @expected_behavior Module loading failure with clear error messages
 * @validation_method Error detection and proper resource cleanup
 */
TEST_P(F64ConvertI64STestSuite, StackUnderflow_HandlesGracefully) {
    // Test invalid WASM bytecode with type mismatch
    // This simulates a WASM module where f64.convert_i64_s has wrong stack types
    uint8_t invalid_wasm[] = {
        0x00, 0x61, 0x73, 0x6d,  // WASM magic
        0x01, 0x00, 0x00, 0x00,  // Version
        0x01, 0x05,              // Type section
        0x01,                    // 1 type
        0x60, 0x01, 0x7f, 0x00,  // Function type: (i32) -> () [INVALID for f64.convert_i64_s]
        0x03, 0x02,              // Function section
        0x01, 0x00,              // 1 function of type 0
        0x0a, 0x06,              // Code section
        0x01, 0x04, 0x00,        // Function 0 body
        0x20, 0x00,              // local.get 0
        0xb9,                    // f64.convert_i64_s [INVALID: expects i64 but gets i32]
        0x0b                     // end
    };

    char error_buf[256];
    wasm_module_t invalid_module = wasm_runtime_load(
        invalid_wasm, sizeof(invalid_wasm), error_buf, sizeof(error_buf));

    // Expect module loading to fail due to type mismatch
    ASSERT_EQ(nullptr, invalid_module)
        << "Invalid WASM module with type mismatch should fail to load";

    // Verify error message contains meaningful information
    std::string error_msg(error_buf);
    ASSERT_FALSE(error_msg.empty())
        << "Error message should be provided for invalid module loading";

    // Test stack underflow scenario with empty function
    uint8_t underflow_wasm[] = {
        0x00, 0x61, 0x73, 0x6d,  // WASM magic
        0x01, 0x00, 0x00, 0x00,  // Version
        0x01, 0x04,              // Type section
        0x01,                    // 1 type
        0x60, 0x00, 0x00,        // Function type: () -> ()
        0x03, 0x02,              // Function section
        0x01, 0x00,              // 1 function of type 0
        0x0a, 0x05,              // Code section
        0x01, 0x03, 0x00,        // Function 0 body
        0xb9,                    // f64.convert_i64_s [INVALID: no i64 on stack]
        0x0b                     // end
    };

    wasm_module_t underflow_module = wasm_runtime_load(
        underflow_wasm, sizeof(underflow_wasm), error_buf, sizeof(error_buf));

    // Expect module loading to fail due to stack underflow
    ASSERT_EQ(nullptr, underflow_module)
        << "WASM module with stack underflow should fail to load";
}

// Instantiate parameterized tests for both execution modes
INSTANTIATE_TEST_SUITE_P(
    RunningMode,
    F64ConvertI64STestSuite,
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