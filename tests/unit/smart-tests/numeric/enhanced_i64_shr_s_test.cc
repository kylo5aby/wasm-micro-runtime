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
 * @file enhanced_i64_shr_s_test.cc
 * @brief Enhanced unit tests for i64.shr_s opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the i64.shr_s (64-bit signed right shift)
 * WebAssembly instruction, focusing on:
 * - Basic arithmetic right shift functionality with positive and negative 64-bit values
 * - Corner cases including boundary values (INT64_MIN, INT64_MAX) and maximum shifts
 * - Edge cases with zero operands, identity operations, and modulo shift behavior
 * - Sign extension verification and mathematical properties of 64-bit arithmetic shifts
 * - Shift count modulo 64 behavior for large shift values
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i64.shr_s operations
 * @coverage_target core/iwasm/aot/aot_runtime.c:64-bit signed shift instructions
 * @coverage_target Sign extension behavior and arithmetic shift algorithms
 * @coverage_target Stack management for 64-bit binary numeric operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class I64ShrSTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i64.shr_s testing";

        // Load WASM test module containing i64.shr_s test functions
        std::string wasm_file = "wasm-apps/i64_shr_s_test.wasm";
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

        // Instantiate WASM module for test execution
        module_inst = wasm_runtime_instantiate(module, 65536, 65536,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode based on test parameter (interpreter or AOT)
        wasm_runtime_set_running_mode(module_inst, GetParam());
    }

    void TearDown() override {
        // Clean up WASM resources in reverse order of creation
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (module_buffer) {
            BH_FREE(module_buffer);
        }

        // Shutdown WAMR runtime to prevent resource leaks
        wasm_runtime_destroy();
    }

    /**
     * @brief Executes i64.shr_s operation with two operands
     * @details Helper function to call WASM i64.shr_s function and retrieve result
     * @param value The 64-bit integer value to be shifted right
     * @param shift_count The number of positions to shift right (masked to 6 bits)
     * @return Result of arithmetic right shift operation
     * @coverage_target Function call mechanism and 64-bit parameter passing
     */
    int64_t call_i64_shr_s(int64_t value, uint64_t shift_count) {
        // Locate the exported i64_shr_s test function
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "i64_shr_s_test");
        EXPECT_NE(nullptr, func) << "Failed to find i64_shr_s_test function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: [value_low, value_high, shift_count_low, shift_count_high]
        // WASM uses 32-bit stack slots, so 64-bit values need two slots
        uint32_t argv[4] = {
            static_cast<uint32_t>(value & 0xFFFFFFFF),           // value low 32 bits
            static_cast<uint32_t>((value >> 32) & 0xFFFFFFFF),   // value high 32 bits
            static_cast<uint32_t>(shift_count & 0xFFFFFFFF),     // shift_count low 32 bits
            static_cast<uint32_t>((shift_count >> 32) & 0xFFFFFFFF) // shift_count high 32 bits
        };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(call_result)
            << "i64_shr_s function call failed: "
            << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Reconstruct 64-bit result from two 32-bit values
        uint64_t result = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        return static_cast<int64_t>(result);
    }

private:
    wasm_module_t module = nullptr;           // Loaded WASM module
    wasm_module_inst_t module_inst = nullptr; // Instantiated WASM module
    uint8_t* module_buffer = nullptr;         // Raw WASM bytecode buffer
    uint32_t buffer_size = 0;                 // Size of WASM bytecode buffer
};

/**
 * @test BasicArithmeticShift_ProducesCorrectResults
 * @brief Validates i64.shr_s produces correct results for typical 64-bit shift scenarios
 * @details Tests fundamental 64-bit arithmetic right shift operation with positive and negative values.
 *          Verifies sign extension behavior and proper bit shifting with 64-bit precision.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i64_shr_s_operation
 * @input_conditions Standard 64-bit integer values with positive and negative operands
 * @expected_behavior Returns mathematically correct 64-bit arithmetic right shift results
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64ShrSTestSuite, BasicArithmeticShift_ProducesCorrectResults) {
    // Test positive value shifts - basic 64-bit arithmetic right shift
    ASSERT_EQ(4LL, call_i64_shr_s(8LL, 1))
        << "64-bit arithmetic right shift of 8 by 1 position failed";

    ASSERT_EQ(125LL, call_i64_shr_s(1000LL, 3))
        << "64-bit arithmetic right shift of 1000 by 3 positions failed";

    // Test negative value shifts - verify 64-bit sign extension
    ASSERT_EQ(-4LL, call_i64_shr_s(-8LL, 1))
        << "64-bit arithmetic right shift of -8 by 1 position failed (sign extension)";

    ASSERT_EQ(-125LL, call_i64_shr_s(-1000LL, 3))
        << "64-bit arithmetic right shift of -1000 by 3 positions failed (sign extension)";

    // Test mixed scenarios with larger 64-bit values
    ASSERT_EQ(1073741824LL, call_i64_shr_s(4294967296LL, 2))  // 2^32 >> 2 = 2^30
        << "Large positive 64-bit shift by 2 positions failed";

    ASSERT_EQ(-1073741824LL, call_i64_shr_s(-4294967296LL, 2))  // -2^32 >> 2 = -2^30
        << "Large negative 64-bit shift by 2 positions failed (sign preservation)";
}

/**
 * @test BoundaryValues_HandleMinMaxCorrectly
 * @brief Validates i64.shr_s handles extreme 64-bit values correctly
 * @details Tests boundary conditions with INT64_MIN and INT64_MAX values,
 *          verifying proper sign extension and maximum shift behavior for 64-bit operations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target Extreme 64-bit value handling and sign preservation
 * @input_conditions Boundary values with various shift counts up to 63
 * @expected_behavior Correct 64-bit arithmetic shifts maintaining mathematical properties
 * @validation_method Boundary value testing with expected 64-bit results
 */
TEST_P(I64ShrSTestSuite, BoundaryValues_HandleMinMaxCorrectly) {
    // Test INT64_MIN boundary (-9223372036854775808)
    ASSERT_EQ(-4611686018427387904LL, call_i64_shr_s(INT64_MIN, 1))
        << "INT64_MIN >> 1 should produce -4611686018427387904";

    ASSERT_EQ(-1LL, call_i64_shr_s(INT64_MIN, 63))
        << "INT64_MIN >> 63 should produce -1 (maximum 64-bit shift)";

    // Test INT64_MAX boundary (9223372036854775807)
    ASSERT_EQ(4611686018427387903LL, call_i64_shr_s(INT64_MAX, 1))
        << "INT64_MAX >> 1 should produce 4611686018427387903";

    ASSERT_EQ(0LL, call_i64_shr_s(INT64_MAX, 63))
        << "INT64_MAX >> 63 should produce 0 (maximum 64-bit shift)";

    // Test near-boundary values with 64-bit precision
    ASSERT_EQ(-1LL, call_i64_shr_s(-1LL, 1))
        << "Shifting -1 right should maintain -1 (all 64 bits set)";

    ASSERT_EQ(0LL, call_i64_shr_s(1LL, 63))
        << "Shifting 1 right by 63 should produce 0";

    // Test high-order bit patterns
    ASSERT_EQ(-1LL, call_i64_shr_s(0x8000000000000000LL, 63))  // MSB set >> 63 = -1 (sign extended)
        << "High bit shift extraction failed";
}

/**
 * @test ZeroAndIdentity_ProduceExpectedResults
 * @brief Validates i64.shr_s zero operand and identity operation behavior
 * @details Tests edge cases with zero values and zero shift counts,
 *          verifying identity operation behavior for 64-bit operations.
 * @test_category Edge - Zero operand and identity validation
 * @coverage_target Edge case handling and 64-bit identity operations
 * @input_conditions Zero values and zero shift counts
 * @expected_behavior Predictable results for zero and identity operations
 * @validation_method Zero value and identity operation testing with 64-bit precision
 */
TEST_P(I64ShrSTestSuite, ZeroAndIdentity_ProduceExpectedResults) {
    // Test zero value shifts
    ASSERT_EQ(0LL, call_i64_shr_s(0LL, 1))
        << "Shifting zero should always produce zero";

    ASSERT_EQ(0LL, call_i64_shr_s(0LL, 63))
        << "Maximum shift of zero should produce zero";

    // Test identity operations (shift by zero)
    ASSERT_EQ(42LL, call_i64_shr_s(42LL, 0))
        << "Shifting by zero should return original value";

    ASSERT_EQ(-42LL, call_i64_shr_s(-42LL, 0))
        << "Shifting negative value by zero should return original";

    ASSERT_EQ(INT64_MAX, call_i64_shr_s(INT64_MAX, 0))
        << "Identity shift of INT64_MAX should return original";

    ASSERT_EQ(INT64_MIN, call_i64_shr_s(INT64_MIN, 0))
        << "Identity shift of INT64_MIN should return original";

    // Test large values identity
    int64_t large_value = 0x123456789ABCDEFLL;
    ASSERT_EQ(large_value, call_i64_shr_s(large_value, 0))
        << "Identity shift of large 64-bit value should return original";
}

/**
 * @test ModuloWrapAround_HandlesLargeShifts
 * @brief Validates i64.shr_s handles shift counts with modulo 64 behavior
 * @details Tests that shift counts larger than 63 wrap around correctly
 *          according to WebAssembly specification (shift_count % 64).
 * @test_category Edge - Modulo arithmetic validation for 64-bit operations
 * @coverage_target Large shift count handling and modulo 64 behavior
 * @input_conditions Shift counts >= 64 testing modulo wrapping
 * @expected_behavior Shift counts wrap according to modulo 64
 * @validation_method Large shift count testing with modulo 64 verification
 */
TEST_P(I64ShrSTestSuite, ModuloWrapAround_HandlesLargeShifts) {
    // Test shift count exactly 64 (should wrap to 0)
    ASSERT_EQ(256LL, call_i64_shr_s(256LL, 64))
        << "Shift by 64 should wrap to shift by 0";

    // Test shift count 65 (should wrap to 1)
    ASSERT_EQ(128LL, call_i64_shr_s(256LL, 65))
        << "Shift by 65 should wrap to shift by 1";

    // Test large shift count with negative value
    ASSERT_EQ(-1LL, call_i64_shr_s(-256LL, 100))  // 100 % 64 = 36, so -256 >> 36 = -1
        << "Large shift count with negative value failed modulo 64 wrapping";

    // Test very large shift count
    ASSERT_EQ(32LL, call_i64_shr_s(256LL, 67))  // 67 % 64 = 3, so 256 >> 3 = 32
        << "Very large shift count failed modulo 64 wrapping";

    // Test shift count with maximum values
    ASSERT_EQ(-1LL, call_i64_shr_s(INT64_MIN, 127))  // 127 % 64 = 63, INT64_MIN >> 63 = -1
        << "Maximum shift count wrapping with INT64_MIN failed";
}

/**
 * @test SignExtension_PreservesSignBit
 * @brief Validates i64.shr_s properly extends sign bit during 64-bit arithmetic shifts
 * @details Tests that negative values maintain their sign through 64-bit arithmetic
 *          right shift operations, distinguishing from logical right shift.
 * @test_category Edge - Sign extension behavior validation for 64-bit operations
 * @coverage_target Arithmetic vs logical shift differentiation
 * @input_conditions Various negative 64-bit values with different shift counts
 * @expected_behavior Sign bit preserved through 64-bit shift operations
 * @validation_method Sign preservation verification across 64-bit shift ranges
 */
TEST_P(I64ShrSTestSuite, SignExtension_PreservesSignBit) {
    // Test that negative values remain negative across all valid shifts
    int64_t test_negative = -1000000000LL;  // Large negative 64-bit value
    for (uint32_t shift = 1; shift <= 63; ++shift) {
        int64_t result = call_i64_shr_s(test_negative, shift);
        ASSERT_LT(result, 0LL)
            << "Negative value " << test_negative << " >> " << shift
            << " should remain negative, got: " << result;
    }

    // Test maximum shift of any negative number produces -1
    ASSERT_EQ(-1LL, call_i64_shr_s(-42000000000LL, 63))
        << "Maximum shift of negative 64-bit value should produce -1";

    ASSERT_EQ(-1LL, call_i64_shr_s(-1LL, 63))
        << "Maximum shift of -1 should produce -1";

    // Test positive values never become negative
    int64_t test_positive = 1000000000000LL;  // Large positive 64-bit value
    for (uint32_t shift = 1; shift <= 63; ++shift) {
        int64_t result = call_i64_shr_s(test_positive, shift);
        ASSERT_GE(result, 0LL)
            << "Positive value " << test_positive << " >> " << shift
            << " should remain non-negative, got: " << result;
    }

    // Test special bit patterns for sign extension
    ASSERT_EQ(-4611686018427387904LL, call_i64_shr_s(0x8000000000000000LL, 1))  // MSB set, shift by 1
        << "Sign extension from MSB set failed";

    ASSERT_EQ(-1125899906842624LL, call_i64_shr_s(0xF000000000000000LL, 10))  // High bits set
        << "Sign extension with multiple high bits set failed";
}

/**
 * @test LargeValuePatterns_HandleCorrectly
 * @brief Validates i64.shr_s handles large 64-bit value patterns correctly
 * @details Tests arithmetic right shift with various 64-bit bit patterns,
 *          verifying correct behavior with large numbers beyond 32-bit range.
 * @test_category Edge - Large value pattern validation
 * @coverage_target Large 64-bit value handling and bit pattern processing
 * @input_conditions Large 64-bit values with specific bit patterns
 * @expected_behavior Correct arithmetic shift results for large values
 * @validation_method Large value testing with pattern verification
 */
TEST_P(I64ShrSTestSuite, LargeValuePatterns_HandleCorrectly) {
    // Test values in upper 32-bit range
    int64_t large_positive = 0x123456789ABCDEFLL;
    ASSERT_EQ(0x91A2B3C4D5E6F7LL, call_i64_shr_s(large_positive, 1))
        << "Large positive 64-bit value right shift failed";

    // Test alternating bit patterns
    int64_t alternating = 0xAAAAAAAAAAAAAAAALL;  // Negative alternating pattern
    int64_t result = call_i64_shr_s(alternating, 4);
    ASSERT_LT(result, 0LL)
        << "Alternating negative pattern should remain negative after shift";

    // Test high-order bits with shift
    int64_t high_bits = 0xFF00000000000000LL;  // High byte set (negative)
    ASSERT_EQ(-1LL, call_i64_shr_s(high_bits, 63))
        << "High-order negative bits should produce -1 when maximally shifted";

    // Test powers of 2 in 64-bit range
    ASSERT_EQ(0x200000000LL, call_i64_shr_s(0x800000000LL, 2))  // 2^35 >> 2 = 2^33
        << "Power of 2 shift in 64-bit range failed";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(I64ShrSTest, I64ShrSTestSuite,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT));