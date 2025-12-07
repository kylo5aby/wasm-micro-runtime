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
 * @file enhanced_i32_shr_u_test.cc
 * @brief Enhanced unit tests for i32.shr_u opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the i32.shr_u (unsigned right shift)
 * WebAssembly instruction, focusing on:
 * - Basic logical right shift functionality with zero-fill behavior
 * - Corner cases including boundary values (UINT32_MAX, INT32_MIN) and maximum shifts
 * - Edge cases with zero operands, identity operations, and modulo shift behavior
 * - Zero-fill verification and mathematical properties of logical shifts
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32.shr_u operations
 * @coverage_target core/iwasm/aot/aot_runtime.c:unsigned shift instructions
 * @coverage_target Zero-fill behavior and logical shift algorithms
 * @coverage_target Stack management for binary numeric operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class I32ShrUTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i32.shr_u testing";

        // Load WASM test module containing i32.shr_u test functions
        std::string wasm_file = "./wasm-apps/i32_shr_u_test.wasm";
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

        // Instantiate module for execution
        module_inst = wasm_runtime_instantiate(module, 65536, 65536,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode (interpreter or AOT) based on test parameter
        wasm_runtime_set_running_mode(module_inst, GetParam());
    }

    void TearDown() override {
        // Clean up WAMR resources in reverse order of creation
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (module_buffer) {
            BH_FREE(module_buffer);
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Execute i32.shr_u operation with two i32 parameters
     * @param value The value to be shifted
     * @param shift_count Number of positions to shift right
     * @return Result of unsigned right shift operation
     */
    uint32_t CallI32ShrU(uint32_t value, uint32_t shift_count) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "i32_shr_u_test");
        EXPECT_NE(nullptr, func) << "Failed to lookup i32_shr_u_test function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        uint32_t argv[2] = {value, shift_count};

        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success) << "WASM function execution failed: "
                           << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        return argv[0]; // Return value stored in first element
    }

    /**
     * @brief Execute single-parameter test function with fixed shift count
     * @param input The value to be shifted by 4 positions
     * @return Result of input >> 4 operation
     */
    uint32_t CallBasicShrU(uint32_t input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_basic_shr_u");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_basic_shr_u function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        uint32_t argv[1] = {input};
        bool success = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(success) << "Basic shift test execution failed";

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        return argv[0];
    }

    /**
     * @brief Execute test function by name and return result
     * @param func_name Name of exported WASM function to call
     * @return Function execution result
     */
    uint32_t CallTestFunction(const char* func_name) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Failed to lookup function: " << func_name;

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        uint32_t argv[1] = {0}; // Result container
        bool success = wasm_runtime_call_wasm(exec_env, func, 0, argv);
        EXPECT_TRUE(success) << "Function " << func_name << " execution failed";

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        return argv[0];
    }

private:
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    uint8_t* module_buffer = nullptr;
    uint32_t buffer_size = 0;
};

/**
 * @test BasicUnsignedRightShift_ReturnsCorrectResults
 * @brief Validates i32.shr_u produces correct logical right shift results
 * @details Tests fundamental unsigned shift operation with various input combinations.
 *          Verifies zero-fill behavior and correct mathematical results.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_shr_u_operation
 * @input_conditions Standard integer pairs with different bit patterns
 * @expected_behavior Returns mathematical result with zero-fill from left
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32ShrUTestSuite, BasicUnsignedRightShift_ReturnsCorrectResults) {
    // Test basic positive shift operations
    ASSERT_EQ(4U, CallI32ShrU(8, 1))
        << "8 >> 1 should produce 4";
    ASSERT_EQ(4U, CallI32ShrU(16, 2))
        << "16 >> 2 should produce 4";
    ASSERT_EQ(0x00123456U, CallI32ShrU(0x12345678, 8))
        << "0x12345678 >> 8 should produce 0x00123456";

    // Test shift operations with different bit patterns
    ASSERT_EQ(16U, CallBasicShrU(256))
        << "256 >> 4 should produce 16";
    ASSERT_EQ(0x01234000U, CallBasicShrU(0x12340000))
        << "0x12340000 >> 4 should produce 0x01234000";
}

/**
 * @test BoundaryValues_HandleZeroFillCorrectly
 * @brief Validates boundary value handling with proper zero-fill behavior
 * @details Tests extreme values including high bit set and maximum values,
 *          ensuring zero-fill behavior distinguishes from signed shifts.
 * @test_category Corner - Boundary condition validation
 * @coverage_target Boundary value processing and zero-fill algorithms
 * @input_conditions Extreme values: 0x80000000, 0xFFFFFFFF, maximum shifts
 * @expected_behavior Zero-fill from left, no sign extension behavior
 * @validation_method Compare against expected unsigned shift results
 */
TEST_P(I32ShrUTestSuite, BoundaryValues_HandleZeroFillCorrectly) {
    // High bit set (INT32_MIN as unsigned) - should zero-fill, not sign extend
    ASSERT_EQ(0x40000000U, CallI32ShrU(0x80000000, 1))
        << "0x80000000 >> 1 should produce 0x40000000 (zero-fill)";

    // All bits set (UINT32_MAX) - should zero-fill from left
    ASSERT_EQ(0x0FFFFFFFU, CallI32ShrU(0xFFFFFFFF, 4))
        << "0xFFFFFFFF >> 4 should produce 0x0FFFFFFF";

    // Maximum shift (31 positions) - should produce 0 or 1
    ASSERT_EQ(0U, CallI32ShrU(0x7FFFFFFF, 31))
        << "0x7FFFFFFF >> 31 should produce 0";
    ASSERT_EQ(1U, CallI32ShrU(0x80000000, 31))
        << "0x80000000 >> 31 should produce 1";
}

/**
 * @test ShiftByZero_PreservesOriginalValue
 * @brief Validates that shifting by 0 preserves original value (identity operation)
 * @details Tests identity property of shift operations where shift count is 0.
 *          Verifies no side effects or unintended modifications occur.
 * @test_category Edge - Identity operation validation
 * @coverage_target Identity operation handling in shift instructions
 * @input_conditions Various values with shift count of 0
 * @expected_behavior Original value returned unchanged
 * @validation_method Direct equality comparison with input values
 */
TEST_P(I32ShrUTestSuite, ShiftByZero_PreservesOriginalValue) {
    // Identity operation validation for various bit patterns
    ASSERT_EQ(0x12345678U, CallI32ShrU(0x12345678, 0))
        << "Value should remain unchanged when shifted by 0";
    ASSERT_EQ(0x80000000U, CallI32ShrU(0x80000000, 0))
        << "High bit value should remain unchanged when shifted by 0";
    ASSERT_EQ(0xFFFFFFFFU, CallI32ShrU(0xFFFFFFFF, 0))
        << "Maximum value should remain unchanged when shifted by 0";
    ASSERT_EQ(0U, CallI32ShrU(0, 0))
        << "Zero should remain zero when shifted by 0";
}

/**
 * @test LargeShiftCounts_MaskProperly
 * @brief Validates proper masking behavior for shift counts >= 32
 * @details Tests WebAssembly specification requirement that shift counts
 *          are masked to lower 5 bits (count & 0x1F) before shifting.
 * @test_category Corner - Modulo shift count validation
 * @coverage_target Shift count masking and modulo arithmetic
 * @input_conditions Shift counts: 32, 33, 100, 0xFFFFFFFF
 * @expected_behavior Shift count masked to lower 5 bits before operation
 * @validation_method Compare results with equivalent masked shift operations
 */
TEST_P(I32ShrUTestSuite, LargeShiftCounts_MaskProperly) {
    uint32_t test_value = 0x12345678;

    // Shift by 32 should be equivalent to shift by 0 (32 & 0x1F = 0)
    ASSERT_EQ(test_value, CallI32ShrU(test_value, 32))
        << "Shift by 32 should be equivalent to shift by 0";

    // Shift by 33 should be equivalent to shift by 1 (33 & 0x1F = 1)
    ASSERT_EQ(CallI32ShrU(test_value, 1), CallI32ShrU(test_value, 33))
        << "Shift by 33 should be equivalent to shift by 1";

    // Shift by 100 should be equivalent to shift by 4 (100 & 0x1F = 4)
    ASSERT_EQ(CallI32ShrU(test_value, 4), CallI32ShrU(test_value, 100))
        << "Shift by 100 should be equivalent to shift by 4";

    // Maximum shift count should mask to 31 (0xFFFFFFFF & 0x1F = 31)
    ASSERT_EQ(CallI32ShrU(0x80000000, 31), CallI32ShrU(0x80000000, 0xFFFFFFFF))
        << "Maximum shift count should mask to 31";
}

/**
 * @test ZeroOperand_ProducesZeroResult
 * @brief Validates that shifting zero produces zero regardless of shift count
 * @details Tests mathematical property that 0 >> n = 0 for any valid n.
 *          Ensures zero operand handling works correctly across all shift counts.
 * @test_category Edge - Zero operand validation
 * @coverage_target Zero operand special case handling
 * @input_conditions Zero value with various shift counts
 * @expected_behavior Always produces zero result
 * @validation_method Verify zero result for multiple shift count values
 */
TEST_P(I32ShrUTestSuite, ZeroOperand_ProducesZeroResult) {
    // Zero shifted by any amount should remain zero
    ASSERT_EQ(0U, CallI32ShrU(0, 1))
        << "0 >> 1 should produce 0";
    ASSERT_EQ(0U, CallI32ShrU(0, 15))
        << "0 >> 15 should produce 0";
    ASSERT_EQ(0U, CallI32ShrU(0, 31))
        << "0 >> 31 should produce 0";
    ASSERT_EQ(0U, CallI32ShrU(0, 100)) // Tests modulo behavior too
        << "0 >> 100 should produce 0";
}

/**
 * @test ZeroFillBehavior_DistinguishesFromSignedShift
 * @brief Validates zero-fill behavior differentiates from signed right shift
 * @details Verifies that i32.shr_u fills with zeros from the left, unlike
 *          i32.shr_s which performs sign extension for negative values.
 * @test_category Main - Zero-fill behavior validation
 * @coverage_target Zero-fill vs sign extension algorithm differences
 * @input_conditions Negative values (high bit set) with various shifts
 * @expected_behavior Zero-fill from left, never sign extension
 * @validation_method Compare against known zero-filled results
 */
TEST_P(I32ShrUTestSuite, ZeroFillBehavior_DistinguishesFromSignedShift) {
    // Verify zero-fill result using exported test function
    ASSERT_EQ(0x40000000U, CallTestFunction("test_zero_fill"))
        << "Zero-fill test should produce 0x40000000";

    // Test alternating bit pattern with high bit set
    uint32_t alternating_high = 0xAAAAAAAA; // 10101010... pattern
    ASSERT_EQ(0x55555555U, CallI32ShrU(alternating_high, 1))
        << "Alternating high bits should zero-fill to 0x55555555";
}

/**
 * @test PowersOfTwo_ShiftCorrectly
 * @brief Validates shift operations on powers of 2 values
 * @details Tests mathematical relationship between powers of 2 and right shifts.
 *          Verifies 2^n >> m = 2^(n-m) when n >= m.
 * @test_category Main - Mathematical property validation
 * @coverage_target Power-of-2 arithmetic and bit manipulation
 * @input_conditions Various powers of 2 with different shift counts
 * @expected_behavior Correct power-of-2 arithmetic results
 * @validation_method Mathematical verification of shift results
 */
TEST_P(I32ShrUTestSuite, PowersOfTwo_ShiftCorrectly) {
    // Test basic power of 2 shifts
    ASSERT_EQ(16U, CallTestFunction("test_power_of_2"))
        << "64 >> 2 should produce 16";

    // Test high bit power of 2
    ASSERT_EQ(0x20000000U, CallTestFunction("test_power_of_2_high_bit"))
        << "0x80000000 >> 2 should produce 0x20000000";

    // Additional power of 2 validation
    ASSERT_EQ(1U, CallI32ShrU(0x80000000, 31)) // 2^31 >> 31 = 1
        << "2^31 >> 31 should produce 1";
    ASSERT_EQ(512U, CallI32ShrU(0x40000000, 21)) // 2^30 >> 21 = 2^9 = 512
        << "2^30 >> 21 should produce 512";
}

/**
 * @test MathematicalProperties_ValidateConsistency
 * @brief Validates mathematical properties and consistency of shift operations
 * @details Tests various mathematical properties including consecutive shifts,
 *          division equivalence, and modulo behavior consistency.
 * @test_category Edge - Mathematical property validation
 * @coverage_target Mathematical consistency and property preservation
 * @input_conditions Complex property validation scenarios
 * @expected_behavior All mathematical properties hold true
 * @validation_method Execute comprehensive property validation function
 */
TEST_P(I32ShrUTestSuite, MathematicalProperties_ValidateConsistency) {
    // Execute comprehensive mathematical property validation
    ASSERT_EQ(1U, CallTestFunction("verify_mathematical_properties"))
        << "Mathematical properties validation should pass";

    // Test consecutive shift property: (x >> a) >> b == x >> (a+b) when a+b < 32
    uint32_t test_val = 0x12345678;
    uint32_t consecutive_result = CallI32ShrU(CallI32ShrU(test_val, 3), 2);
    uint32_t direct_result = CallI32ShrU(test_val, 5);
    ASSERT_EQ(direct_result, consecutive_result)
        << "Consecutive shifts should equal direct shift: (x>>3)>>2 == x>>5";

    // Test division equivalence for small values (x >> n == x / 2^n for unsigned)
    ASSERT_EQ(125U, CallI32ShrU(1000, 3)) // 1000 / 8 = 125
        << "1000 >> 3 should equal 1000 / 8 = 125";
}

/**
 * @test ComplexValidation_VerifiesAllProperties
 * @brief Executes comprehensive validation of all i32.shr_u properties
 * @details Runs the exported complex validation function that tests multiple
 *          properties including identity, modulo, zero-fill, and boundary behavior.
 * @test_category Main - Comprehensive validation
 * @coverage_target Complete opcode behavior validation
 * @input_conditions Multiple property validation in single test
 * @expected_behavior All properties validated successfully
 * @validation_method Execute exported validation function
 */
TEST_P(I32ShrUTestSuite, ComplexValidation_VerifiesAllProperties) {
    // Execute comprehensive property validation function
    ASSERT_EQ(1U, CallTestFunction("validate_shr_u_properties"))
        << "Complex validation should pass all property checks";

    // Execute boundary extremes validation
    ASSERT_EQ(1U, CallTestFunction("test_boundary_extremes"))
        << "Boundary extremes validation should pass";
}

/**
 * @test AlternatingBitPatterns_ShiftCorrectly
 * @brief Validates shift operations on alternating bit patterns
 * @details Tests behavior with alternating 0/1 bit patterns to ensure
 *          proper bit manipulation and zero-fill behavior.
 * @test_category Edge - Bit pattern validation
 * @coverage_target Bit pattern manipulation and preservation
 * @input_conditions Alternating bit patterns: 0x55555555, 0xAAAAAAAA
 * @expected_behavior Correct bit pattern shifts with zero-fill
 * @validation_method Verify expected bit pattern results
 */
TEST_P(I32ShrUTestSuite, AlternatingBitPatterns_ShiftCorrectly) {
    // Test alternating 01010101 pattern (0x55555555 >> 1 = 715827882)
    ASSERT_EQ(715827882U, CallTestFunction("test_alternating_positive"))
        << "Alternating positive pattern shift result mismatch";

    // Test alternating 10101010 pattern (high bits set)
    ASSERT_EQ(0x55555555U, CallTestFunction("test_alternating_high_bits"))
        << "Alternating high bits pattern should zero-fill correctly";
}

// Instantiate test suite for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(I32ShrUTest, I32ShrUTestSuite,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT));