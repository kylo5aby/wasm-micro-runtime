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
 * @file enhanced_i32_shr_s_test.cc
 * @brief Enhanced unit tests for i32.shr_s opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the i32.shr_s (signed right shift)
 * WebAssembly instruction, focusing on:
 * - Basic arithmetic right shift functionality with positive and negative values
 * - Corner cases including boundary values (INT32_MIN, INT32_MAX) and maximum shifts
 * - Edge cases with zero operands, identity operations, and modulo shift behavior
 * - Sign extension verification and mathematical properties of arithmetic shifts
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32.shr_s operations
 * @coverage_target core/iwasm/aot/aot_runtime.c:signed shift instructions
 * @coverage_target Sign extension behavior and arithmetic shift algorithms
 * @coverage_target Stack management for binary numeric operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class I32ShrSTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i32.shr_s testing";

        // Load WASM test module containing i32.shr_s test functions
        std::string wasm_file = "./wasm-apps/i32_shr_s_test.wasm";
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
     * @brief Executes i32.shr_s operation with two operands
     * @details Helper function to call WASM i32.shr_s function and retrieve result
     * @param value The 32-bit integer value to be shifted right
     * @param shift_count The number of positions to shift right
     * @return Result of arithmetic right shift operation
     * @coverage_target Function call mechanism and parameter passing
     */
    int32_t call_i32_shr_s(int32_t value, uint32_t shift_count) {
        // Locate the exported i32_shr_s test function
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "i32_shr_s_test");
        EXPECT_NE(nullptr, func) << "Failed to find i32_shr_s_test function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: [value, shift_count]
        uint32_t argv[2] = { static_cast<uint32_t>(value), shift_count };

        // Execute function with error handling
        bool call_result = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(call_result)
            << "i32_shr_s function call failed: "
            << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Return the computed result (cast back to signed)
        return static_cast<int32_t>(argv[0]);
    }

private:
    wasm_module_t module = nullptr;           // Loaded WASM module
    wasm_module_inst_t module_inst = nullptr; // Instantiated WASM module
    uint8_t* module_buffer = nullptr;         // Raw WASM bytecode buffer
    uint32_t buffer_size = 0;                 // Size of WASM bytecode buffer
};

/**
 * @test BasicArithmeticShift_ProducesCorrectResults
 * @brief Validates i32.shr_s produces correct results for typical shift scenarios
 * @details Tests fundamental arithmetic right shift operation with positive and negative values.
 *          Verifies sign extension behavior and proper bit shifting with wraparound.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32_shr_s_operation
 * @input_conditions Standard integer values with positive and negative operands
 * @expected_behavior Returns mathematically correct arithmetic right shift results
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32ShrSTestSuite, BasicArithmeticShift_ProducesCorrectResults) {
    // Test positive value shifts - basic arithmetic right shift
    ASSERT_EQ(4, call_i32_shr_s(8, 1))
        << "Arithmetic right shift of 8 by 1 position failed";

    ASSERT_EQ(12, call_i32_shr_s(100, 3))
        << "Arithmetic right shift of 100 by 3 positions failed";

    // Test negative value shifts - verify sign extension
    ASSERT_EQ(-4, call_i32_shr_s(-8, 1))
        << "Arithmetic right shift of -8 by 1 position failed (sign extension)";

    ASSERT_EQ(-13, call_i32_shr_s(-100, 3))
        << "Arithmetic right shift of -100 by 3 positions failed (sign extension)";

    // Test mixed scenarios demonstrating sign preservation
    ASSERT_EQ(16, call_i32_shr_s(64, 2))
        << "Positive shift by 2 positions failed";

    ASSERT_EQ(-16, call_i32_shr_s(-64, 2))
        << "Negative shift by 2 positions failed (sign preservation)";
}

/**
 * @test BoundaryValues_HandleMinMaxCorrectly
 * @brief Validates i32.shr_s handles extreme values correctly
 * @details Tests boundary conditions with INT32_MIN and INT32_MAX values,
 *          verifying proper sign extension and maximum shift behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target Extreme value handling and sign preservation
 * @input_conditions Boundary values with various shift counts
 * @expected_behavior Correct arithmetic shifts maintaining mathematical properties
 * @validation_method Boundary value testing with expected results
 */
TEST_P(I32ShrSTestSuite, BoundaryValues_HandleMinMaxCorrectly) {
    // Test INT32_MIN boundary (-2147483648)
    ASSERT_EQ(-1073741824, call_i32_shr_s(INT32_MIN, 1))
        << "INT32_MIN >> 1 should produce -1073741824";

    ASSERT_EQ(-1, call_i32_shr_s(INT32_MIN, 31))
        << "INT32_MIN >> 31 should produce -1 (maximum shift)";

    // Test INT32_MAX boundary (2147483647)
    ASSERT_EQ(1073741823, call_i32_shr_s(INT32_MAX, 1))
        << "INT32_MAX >> 1 should produce 1073741823";

    ASSERT_EQ(0, call_i32_shr_s(INT32_MAX, 31))
        << "INT32_MAX >> 31 should produce 0 (maximum shift)";

    // Test near-boundary values
    ASSERT_EQ(-1, call_i32_shr_s(-1, 1))
        << "Shifting -1 right should maintain -1 (all bits set)";

    ASSERT_EQ(0, call_i32_shr_s(1, 31))
        << "Shifting 1 right by 31 should produce 0";
}

/**
 * @test ZeroAndIdentity_ProduceExpectedResults
 * @brief Validates i32.shr_s zero operand and identity operation behavior
 * @details Tests edge cases with zero values and zero shift counts,
 *          verifying identity operation behavior.
 * @test_category Edge - Zero operand and identity validation
 * @coverage_target Edge case handling and identity operations
 * @input_conditions Zero values and zero shift counts
 * @expected_behavior Predictable results for zero and identity operations
 * @validation_method Zero value and identity operation testing
 */
TEST_P(I32ShrSTestSuite, ZeroAndIdentity_ProduceExpectedResults) {
    // Test zero value shifts
    ASSERT_EQ(0, call_i32_shr_s(0, 1))
        << "Shifting zero should always produce zero";

    ASSERT_EQ(0, call_i32_shr_s(0, 31))
        << "Maximum shift of zero should produce zero";

    // Test identity operations (shift by zero)
    ASSERT_EQ(42, call_i32_shr_s(42, 0))
        << "Shifting by zero should return original value";

    ASSERT_EQ(-42, call_i32_shr_s(-42, 0))
        << "Shifting negative value by zero should return original";

    ASSERT_EQ(INT32_MAX, call_i32_shr_s(INT32_MAX, 0))
        << "Identity shift of INT32_MAX should return original";

    ASSERT_EQ(INT32_MIN, call_i32_shr_s(INT32_MIN, 0))
        << "Identity shift of INT32_MIN should return original";
}

/**
 * @test ModuloWrapAround_HandlesLargeShifts
 * @brief Validates i32.shr_s handles shift counts with modulo 32 behavior
 * @details Tests that shift counts larger than 31 wrap around correctly
 *          according to WebAssembly specification (shift_count % 32).
 * @test_category Edge - Modulo arithmetic validation
 * @coverage_target Large shift count handling and modulo behavior
 * @input_conditions Shift counts >= 32 testing modulo wrapping
 * @expected_behavior Shift counts wrap according to modulo 32
 * @validation_method Large shift count testing with modulo verification
 */
TEST_P(I32ShrSTestSuite, ModuloWrapAround_HandlesLargeShifts) {
    // Test shift count exactly 32 (should wrap to 0)
    ASSERT_EQ(16, call_i32_shr_s(16, 32))
        << "Shift by 32 should wrap to shift by 0";

    // Test shift count 33 (should wrap to 1)
    ASSERT_EQ(8, call_i32_shr_s(16, 33))
        << "Shift by 33 should wrap to shift by 1";

    // Test large shift count with negative value
    ASSERT_EQ(-1, call_i32_shr_s(-16, 100))  // 100 % 32 = 4, so -16 >> 4 = -1
        << "Large shift count with negative value failed modulo wrapping";

    // Test very large shift count
    ASSERT_EQ(2, call_i32_shr_s(16, 67))  // 67 % 32 = 3, so 16 >> 3 = 2
        << "Very large shift count failed modulo wrapping";
}

/**
 * @test SignExtension_PreservesSignBit
 * @brief Validates i32.shr_s properly extends sign bit during arithmetic shifts
 * @details Tests that negative values maintain their sign through arithmetic
 *          right shift operations, distinguishing from logical right shift.
 * @test_category Edge - Sign extension behavior validation
 * @coverage_target Arithmetic vs logical shift differentiation
 * @input_conditions Various negative values with different shift counts
 * @expected_behavior Sign bit preserved through shift operations
 * @validation_method Sign preservation verification across shift ranges
 */
TEST_P(I32ShrSTestSuite, SignExtension_PreservesSignBit) {
    // Test that negative values remain negative (except for specific edge cases)
    int32_t test_negative = -1000;
    for (uint32_t shift = 1; shift <= 31; ++shift) {
        int32_t result = call_i32_shr_s(test_negative, shift);
        ASSERT_LT(result, 0)
            << "Negative value " << test_negative << " >> " << shift
            << " should remain negative, got: " << result;
    }

    // Test maximum shift of any negative number produces -1
    ASSERT_EQ(-1, call_i32_shr_s(-42, 31))
        << "Maximum shift of negative value should produce -1";

    ASSERT_EQ(-1, call_i32_shr_s(-1, 31))
        << "Maximum shift of -1 should produce -1";

    // Test positive values never become negative
    int32_t test_positive = 1000;
    for (uint32_t shift = 1; shift <= 31; ++shift) {
        int32_t result = call_i32_shr_s(test_positive, shift);
        ASSERT_GE(result, 0)
            << "Positive value " << test_positive << " >> " << shift
            << " should remain non-negative, got: " << result;
    }
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(I32ShrSTest, I32ShrSTestSuite,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT));