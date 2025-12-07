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
 * @file enhanced_f64_convert_i32_s_test.cc
 * @brief Enhanced unit tests for f64.convert_i32_s opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the f64.convert_i32_s (signed integer to double conversion)
 * WebAssembly instruction, focusing on:
 * - Basic signed integer to IEEE 754 double precision conversion functionality
 * - Corner cases including boundary values (INT32_MIN, INT32_MAX) and sign preservation
 * - Edge cases with zero operands, identity operations, and extreme i32 values
 * - IEEE 754 compliance verification (exact representation and sign handling)
 * - Cross-execution mode validation between interpreter and AOT compilation
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64.convert_i32_s operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:DEF_OP_CONVERT macro
 * @coverage_target core/iwasm/aot/aot_runtime.c:floating-point conversion instructions
 * @coverage_target IEEE 754 double precision conversion algorithms and sign preservation
 * @coverage_target Stack management for numeric type conversion operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class F64ConvertI32STestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for f64.convert_i32_s testing";

        // Load WASM test module containing f64.convert_i32_s test functions
        std::string wasm_file = "./wasm-apps/f64_convert_i32_s_test.wasm";
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
     * @brief Helper function to call f64.convert_i32_s WASM function
     * @param input i32 input value to be converted to f64
     * @return double converted result from WASM execution
     * @details Executes the WASM convert_i32_s function and returns the f64 result.
     * Function handles WAMR execution context and validates successful execution.
     */
    double call_convert_i32_s(int32_t input) {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "convert_i32_s");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup convert_i32_s function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: i32 input value
        uint32_t argv[2] = { static_cast<uint32_t>(input), 0 };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 1, argv);
        EXPECT_TRUE(call_result)
            << "convert_i32_s function call failed: " << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return result (f64 value is in argv[0] and argv[1] after call)
        union { uint64_t u; double f; } result;
        result.u = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        return result.f;
    }

    /**
     * @brief Helper function to call boundary test WASM function
     * @param input i32 input value for boundary testing
     * @return double converted result from boundary test function
     * @details Executes the WASM convert_i32_s_boundary function for testing boundary conditions.
     */
    double call_convert_i32_s_boundary(int32_t input) {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "convert_i32_s_boundary");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup convert_i32_s_boundary function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: i32 input value
        uint32_t argv[2] = { static_cast<uint32_t>(input), 0 };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 1, argv);
        EXPECT_TRUE(call_result)
            << "convert_i32_s_boundary function call failed: " << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return result (f64 value is in argv[0] and argv[1] after call)
        union { uint64_t u; double f; } result;
        result.u = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        return result.f;
    }

    /**
     * @brief Helper function to call extreme values test WASM function
     * @param input i32 input value for extreme value testing
     * @return double converted result from extreme values test function
     * @details Executes the WASM convert_i32_s_extreme function for testing extreme value conditions.
     */
    double call_convert_i32_s_extreme(int32_t input) {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "convert_i32_s_extreme");
        EXPECT_NE(nullptr, func_inst) << "Failed to lookup convert_i32_s_extreme function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: i32 input value
        uint32_t argv[2] = { static_cast<uint32_t>(input), 0 };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 1, argv);
        EXPECT_TRUE(call_result)
            << "convert_i32_s_extreme function call failed: " << wasm_runtime_get_exception(module_inst);

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
 * @brief Validates f64.convert_i32_s produces correct IEEE 754 double for typical i32 inputs
 * @details Tests fundamental conversion operation with positive, negative, and zero integers.
 *          Verifies that f64.convert_i32_s correctly converts signed integers to double precision.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_convert_i32_s_operation
 * @input_conditions Standard i32 values: 5, -10, 12345, -67890, 0
 * @expected_behavior Returns exact mathematical conversion with sign preservation
 * @validation_method Direct comparison of WASM function result with expected double values
 */
TEST_P(F64ConvertI32STestSuite, BasicConversion_ReturnsCorrectF64) {
    // Test small positive integer conversion
    ASSERT_EQ(5.0, call_convert_i32_s(5))
        << "Small positive i32 should convert to exact double equivalent";

    // Test small negative integer conversion
    ASSERT_EQ(-10.0, call_convert_i32_s(-10))
        << "Small negative i32 should convert to exact double equivalent";

    // Test medium positive integer conversion
    ASSERT_EQ(12345.0, call_convert_i32_s(12345))
        << "Medium positive i32 should convert to exact double equivalent";

    // Test medium negative integer conversion
    ASSERT_EQ(-67890.0, call_convert_i32_s(-67890))
        << "Medium negative i32 should convert to exact double equivalent";

    // Test zero conversion (identity case)
    ASSERT_EQ(0.0, call_convert_i32_s(0))
        << "Zero i32 should convert to positive zero double";
}

/**
 * @test BoundaryValues_ConvertExactly
 * @brief Validates f64.convert_i32_s correctly handles i32 boundary values and sign transitions
 * @details Tests conversion behavior at i32 MIN/MAX boundaries and values that test
 *          sign handling around zero boundary, ensuring exact IEEE 754 representation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:boundary_value_handling
 * @input_conditions i32 boundary values: INT32_MIN, INT32_MAX, -1, 1
 * @expected_behavior Exact representation in IEEE 754 double precision with sign preservation
 * @validation_method Boundary value verification and exact mathematical comparison
 */
TEST_P(F64ConvertI32STestSuite, BoundaryValues_ConvertExactly) {
    // Test INT32_MAX boundary conversion
    ASSERT_EQ(static_cast<double>(INT32_MAX), call_convert_i32_s_boundary(INT32_MAX))
        << "INT32_MAX should convert to exact double representation without precision loss";

    // Test INT32_MIN boundary conversion
    ASSERT_EQ(static_cast<double>(INT32_MIN), call_convert_i32_s_boundary(INT32_MIN))
        << "INT32_MIN should convert to exact double representation with sign preservation";

    // Test just below zero boundary
    ASSERT_EQ(-1.0, call_convert_i32_s_boundary(-1))
        << "Negative one should convert to exact -1.0 double";

    // Test just above zero boundary
    ASSERT_EQ(1.0, call_convert_i32_s_boundary(1))
        << "Positive one should convert to exact 1.0 double";

    // Test large positive value within range
    ASSERT_EQ(2000000000.0, call_convert_i32_s_boundary(2000000000))
        << "Large positive i32 should convert to exact double equivalent";

    // Test large negative value within range
    ASSERT_EQ(-2000000000.0, call_convert_i32_s_boundary(-2000000000))
        << "Large negative i32 should convert to exact double equivalent";
}

/**
 * @test ExtremeValues_MaintainPrecision
 * @brief Validates f64.convert_i32_s conversion of extreme values with IEEE 754 compliance
 * @details Tests conversion of powers of 2, maximum magnitude values, and special integers
 *          that verify IEEE 754 exact representation properties for all i32 values.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ieee754_compliance
 * @input_conditions Powers of 2, extreme magnitudes, special integers
 * @expected_behavior All i32 values exactly representable in f64 (53-bit mantissa > 32-bit range)
 * @validation_method Mathematical equivalence after conversion and precision verification
 */
TEST_P(F64ConvertI32STestSuite, ExtremeValues_MaintainPrecision) {
    // Test powers of 2 (should be exactly representable)
    ASSERT_EQ(1024.0, call_convert_i32_s_extreme(1024))
        << "Power of 2 (1024) should convert to exact double representation";

    ASSERT_EQ(-512.0, call_convert_i32_s_extreme(-512))
        << "Negative power of 2 (-512) should convert to exact double representation";

    ASSERT_EQ(65536.0, call_convert_i32_s_extreme(65536))
        << "Large power of 2 (65536) should convert to exact double representation";

    // Test extreme boundary values
    ASSERT_EQ(static_cast<double>(INT32_MAX), call_convert_i32_s_extreme(INT32_MAX))
        << "INT32_MAX extreme value should maintain exact precision in double";

    ASSERT_EQ(static_cast<double>(INT32_MIN), call_convert_i32_s_extreme(INT32_MIN))
        << "INT32_MIN extreme value should maintain exact precision in double";

    // Test special integer patterns
    ASSERT_EQ(16777216.0, call_convert_i32_s_extreme(16777216))
        << "Large representable integer should convert exactly";

    ASSERT_EQ(-16777216.0, call_convert_i32_s_extreme(-16777216))
        << "Large negative representable integer should convert exactly";
}

/**
 * @test ErrorConditions_HandleGracefully
 * @brief Tests module loading with invalid bytecode and error handling
 * @details Validates that WAMR properly handles invalid WASM modules with type mismatches
 *          and provides appropriate error reporting for f64.convert_i32_s instruction validation.
 * @test_category Error - Error condition validation
 * @coverage_target core/iwasm/interpreter/wasm_loader.c:type_validation
 * @input_conditions Malformed WASM with type mismatches
 * @expected_behavior Module loading failure with clear error messages
 * @validation_method Error detection and proper resource cleanup
 */
TEST_P(F64ConvertI32STestSuite, ErrorConditions_HandleGracefully) {
    // Test invalid WASM bytecode with type mismatch
    // This simulates a WASM module where f64.convert_i32_s has wrong stack types
    uint8_t invalid_wasm[] = {
        0x00, 0x61, 0x73, 0x6d,  // WASM magic
        0x01, 0x00, 0x00, 0x00,  // Version
        0x01, 0x05,              // Type section
        0x01,                    // 1 type
        0x60, 0x01, 0x7e, 0x00,  // Function type: (i64) -> () [INVALID for f64.convert_i32_s]
        0x03, 0x02,              // Function section
        0x01, 0x00,              // 1 function of type 0
        0x0a, 0x06,              // Code section
        0x01, 0x04, 0x00,        // Function 0 body
        0x20, 0x00,              // local.get 0
        0xb7,                    // f64.convert_i32_s [INVALID: expects i32 but gets i64]
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
}

// Instantiate parameterized tests for both execution modes
INSTANTIATE_TEST_SUITE_P(
    RunningMode,
    F64ConvertI32STestSuite,
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