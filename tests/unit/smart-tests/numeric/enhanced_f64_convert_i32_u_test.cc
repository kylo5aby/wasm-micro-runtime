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
 * @file enhanced_f64_convert_i32_u_test.cc
 * @brief Enhanced unit tests for f64.convert_i32_u opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the f64.convert_i32_u (unsigned integer to double conversion)
 * WebAssembly instruction, focusing on:
 * - Basic unsigned integer to IEEE 754 double precision conversion functionality
 * - Corner cases including boundary values (0, UINT32_MAX) and unsigned interpretation
 * - Edge cases with zero operands, power-of-two values, and extreme u32 values
 * - IEEE 754 compliance verification (exact representation and precision limits)
 * - Cross-execution mode validation between interpreter and AOT compilation
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64.convert_i32_u operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:DEF_OP_CONVERT macro
 * @coverage_target core/iwasm/aot/aot_runtime.c:floating-point conversion instructions
 * @coverage_target IEEE 754 double precision conversion algorithms and unsigned interpretation
 * @coverage_target Stack management for numeric type conversion operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class F64ConvertI32UTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for f64.convert_i32_u testing";

        // Load WASM test module containing f64.convert_i32_u test functions
        std::string wasm_file = "./wasm-apps/f64_convert_i32_u_test.wasm";
        module_buffer = reinterpret_cast<uint8_t*>(
            bh_read_file_to_buffer(wasm_file.c_str(), &buffer_size));
        ASSERT_NE(nullptr, module_buffer)
            << "Failed to load WASM file: " << wasm_file;

        // Compile WASM module with proper error handling and mode configuration
        char error_buf[128];
        module = wasm_runtime_load(module_buffer, buffer_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module)
            << "Failed to load WASM module: " << error_buf;

        // Instantiate module with specified execution mode and resource limits
        module_inst = wasm_runtime_instantiate(module, 65536, 65536, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode for cross-validation testing
        wasm_runtime_set_running_mode(module_inst, GetParam());
    }

    void TearDown() override {
        // Clean up WASM runtime resources in proper destruction order
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (module_buffer) {
            wasm_runtime_free(module_buffer);
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Execute f64.convert_i32_u function with specified unsigned integer input
     * @param input Unsigned 32-bit integer to convert to f64
     * @return Converted f64 value
     * @coverage_target WAMR function call interface and parameter passing
     */
    double call_f64_convert_i32_u(uint32_t input) {
        // Lookup test function in the loaded module
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(
            module_inst, "test_f64_convert_i32_u");
        if (!func_inst) {
            ADD_FAILURE() << "Failed to lookup test_f64_convert_i32_u function";
            return 0.0;
        }

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        if (!exec_env) {
            ADD_FAILURE() << "Failed to create execution environment";
            return 0.0;
        }

        // Prepare function arguments: u32 input value (as i32 for WASM)
        uint32_t argv[2] = { input, 0 };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func_inst, 1, argv);
        if (!call_result) {
            ADD_FAILURE() << "test_f64_convert_i32_u function call failed: " << wasm_runtime_get_exception(module_inst);
            wasm_runtime_destroy_exec_env(exec_env);
            return 0.0;
        }

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return result (f64 value is in argv[0] and argv[1] after call)
        union { uint64_t u; double f; } result;
        result.u = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        return result.f;
    }

    // Test infrastructure members
    uint8_t *module_buffer = nullptr;
    uint32_t buffer_size = 0;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
};

/**
 * @test BasicConversion_TypicalValues_ReturnsCorrectF64
 * @brief Validates f64.convert_i32_u produces correct arithmetic results for typical unsigned integers
 * @details Tests fundamental conversion operation with small positive and larger unsigned integers.
 *          Verifies that f64.convert_i32_u correctly converts u32 values to exact f64 representations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64.convert_i32_u operation
 * @input_conditions Standard unsigned integer values: 1, 100, 1000, 65536
 * @expected_behavior Returns exact f64 equivalent: 1.0, 100.0, 1000.0, 65536.0
 * @validation_method Direct comparison of WASM function result with expected double values
 */
TEST_P(F64ConvertI32UTestSuite, BasicConversion_TypicalValues_ReturnsCorrectF64) {
    // Execute basic conversion tests with typical unsigned values
    ASSERT_EQ(1.0, call_f64_convert_i32_u(1U))         << "Conversion of 1 to f64 failed";
    ASSERT_EQ(100.0, call_f64_convert_i32_u(100U))     << "Conversion of 100 to f64 failed";
    ASSERT_EQ(1000.0, call_f64_convert_i32_u(1000U))   << "Conversion of 1000 to f64 failed";
    ASSERT_EQ(65536.0, call_f64_convert_i32_u(65536U)) << "Conversion of 65536 to f64 failed";
}

/**
 * @test BoundaryValues_MinMax_ConvertsCorrectly
 * @brief Validates f64.convert_i32_u handles boundary values (0, UINT32_MAX) correctly
 * @details Tests conversion at the extremes of unsigned 32-bit integer range.
 *          Verifies exact conversion for minimum (0) and maximum (4294967295) values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:boundary value handling
 * @input_conditions Boundary values: 0, UINT32_MAX (4294967295)
 * @expected_behavior Returns exact f64 equivalent: 0.0, 4294967295.0
 * @validation_method Direct comparison ensuring exact IEEE 754 representation
 */
TEST_P(F64ConvertI32UTestSuite, BoundaryValues_MinMax_ConvertsCorrectly) {
    // Test minimum unsigned value (0)
    ASSERT_EQ(0.0, call_f64_convert_i32_u(0U))
        << "Conversion of minimum unsigned value (0) to f64 failed";

    // Test maximum unsigned value (UINT32_MAX)
    ASSERT_EQ(4294967295.0, call_f64_convert_i32_u(UINT32_MAX))
        << "Conversion of maximum unsigned value (UINT32_MAX) to f64 failed";
}

/**
 * @test SignedUnsignedBoundary_LargeValues_HandlesCorrectly
 * @brief Validates f64.convert_i32_u correctly interprets large unsigned values that would be negative if signed
 * @details Tests conversion of values around INT32_MAX boundary, ensuring unsigned interpretation.
 *          Verifies values > INT32_MAX are treated as positive unsigned integers.
 * @test_category Corner - Signed/unsigned boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:unsigned interpretation logic
 * @input_conditions Values around signed boundary: INT32_MAX, INT32_MAX+1, 0x80000000U
 * @expected_behavior Returns positive f64 values: 2147483647.0, 2147483648.0, 2147483648.0
 * @validation_method Verify all conversions result in positive f64 values
 */
TEST_P(F64ConvertI32UTestSuite, SignedUnsignedBoundary_LargeValues_HandlesCorrectly) {
    // Test INT32_MAX (largest positive signed value)
    ASSERT_EQ(2147483647.0, call_f64_convert_i32_u(static_cast<uint32_t>(INT32_MAX)))
        << "Conversion of INT32_MAX as unsigned to f64 failed";

    // Test INT32_MAX + 1 (first value that would be negative if signed)
    ASSERT_EQ(2147483648.0, call_f64_convert_i32_u(static_cast<uint32_t>(INT32_MAX) + 1))
        << "Conversion of INT32_MAX+1 as unsigned to f64 failed";

    // Test 0x80000000U (MSB set, would be INT32_MIN if signed)
    ASSERT_EQ(2147483648.0, call_f64_convert_i32_u(0x80000000U))
        << "Conversion of 0x80000000U (MSB set) to f64 failed";
}

/**
 * @test ZeroConversion_Identity_ReturnsZero
 * @brief Validates f64.convert_i32_u identity property for zero conversion
 * @details Tests that zero converts to exactly 0.0 in f64 format.
 *          Verifies identity operation and IEEE 754 zero representation.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:zero value handling
 * @input_conditions Zero value: 0
 * @expected_behavior Returns exact zero: 0.0 (positive zero)
 * @validation_method Verify exact zero conversion and sign bit
 */
TEST_P(F64ConvertI32UTestSuite, ZeroConversion_Identity_ReturnsZero) {
    // Test zero conversion identity
    double result = call_f64_convert_i32_u(0U);
    ASSERT_EQ(0.0, result) << "Conversion of 0 to f64 failed identity check";
    ASSERT_FALSE(std::signbit(result)) << "Zero conversion should result in positive zero";
}

/**
 * @test BitPatternValidation_SpecificPatterns_MaintainsAccuracy
 * @brief Validates f64.convert_i32_u handles specific bit patterns correctly
 * @details Tests conversion of values with specific bit patterns that could cause precision issues.
 *          Verifies power-of-two values and high-bit patterns convert accurately.
 * @test_category Edge - Bit pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:bit pattern conversion
 * @input_conditions Specific patterns: 0x80000000U, 0xFFFF0000U, power-of-two values
 * @expected_behavior Returns exact f64 equivalent for all patterns
 * @validation_method Direct comparison ensuring bit-level accuracy
 */
TEST_P(F64ConvertI32UTestSuite, BitPatternValidation_SpecificPatterns_MaintainsAccuracy) {
    // Test highest bit set (0x80000000U)
    ASSERT_EQ(2147483648.0, call_f64_convert_i32_u(0x80000000U))
        << "Conversion of 0x80000000U (highest bit) to f64 failed";

    // Test high 16 bits set (0xFFFF0000U)
    ASSERT_EQ(4294901760.0, call_f64_convert_i32_u(0xFFFF0000U))
        << "Conversion of 0xFFFF0000U (high bits) to f64 failed";

    // Test power-of-two values for exact representation
    ASSERT_EQ(1073741824.0, call_f64_convert_i32_u(1U << 30))  // 2^30
        << "Conversion of 2^30 to f64 failed";

    ASSERT_EQ(16777216.0, call_f64_convert_i32_u(1U << 24))    // 2^24
        << "Conversion of 2^24 to f64 failed";
}

/**
 * @test PowerOfTwoValues_ExactRepresentation_ConvertsCorrectly
 * @brief Validates f64.convert_i32_u exactly represents power-of-two values
 * @details Tests that power-of-two values within u32 range convert to exact f64 representations.
 *          All power-of-two values up to 2^32 should be exactly representable in f64.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:exact representation logic
 * @input_conditions Power-of-two values: 2^1, 2^16, 2^31
 * @expected_behavior Returns exact power-of-two f64 values
 * @validation_method Mathematical verification of power-of-two representation
 */
TEST_P(F64ConvertI32UTestSuite, PowerOfTwoValues_ExactRepresentation_ConvertsCorrectly) {
    // Test various power-of-two values for exact representation
    ASSERT_EQ(2.0, call_f64_convert_i32_u(1U << 1))          // 2^1 = 2
        << "Conversion of 2^1 to f64 failed";

    ASSERT_EQ(65536.0, call_f64_convert_i32_u(1U << 16))     // 2^16 = 65536
        << "Conversion of 2^16 to f64 failed";

    ASSERT_EQ(2147483648.0, call_f64_convert_i32_u(1U << 31)) // 2^31 = 2147483648
        << "Conversion of 2^31 to f64 failed";
}

// Parameterized test execution for cross-mode validation
INSTANTIATE_TEST_SUITE_P(CrossModeValidation, F64ConvertI32UTestSuite,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<RunningMode>& info) {
                             return info.param == Mode_Interp ? "Interpreter" : "AOT";
                         });