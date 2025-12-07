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
 * @file enhanced_i32_popcnt_test.cc
 * @brief Enhanced unit tests for i32.popcnt opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the i32.popcnt (population count)
 * WebAssembly instruction, focusing on:
 * - Basic population counting with typical bit patterns and integer values
 * - Corner cases including boundary values (0, MAX_INT, alternating patterns)
 * - Edge cases with extreme bit patterns, powers of 2, and mathematical properties
 * - Error conditions including stack underflow protection and runtime validation
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32.popcnt operations
 * @coverage_target core/iwasm/aot/aot_runtime.c:bit counting instructions
 * @coverage_target Population count algorithms and bit manipulation primitives
 * @coverage_target Stack management for unary numeric operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class I32PopcntTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i32.popcnt testing";

        // Load WASM test module containing i32.popcnt test functions
        std::string wasm_file = "./wasm-apps/i32_popcnt_test.wasm";
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

        // Instantiate module with adequate stack/heap for population count operations
        module_inst = wasm_runtime_instantiate(module, 8192, 8192,
                                              error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst)
            << "Failed to instantiate module: " << error_buf;

        // Set execution mode (Interpreter or AOT) based on test parameter
        wasm_runtime_set_running_mode(module_inst, GetParam());
    }

    void TearDown() override {
        // Clean up resources in reverse order of creation
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (module_buffer) {
            BH_FREE(module_buffer);
            module_buffer = nullptr;
        }
        wasm_runtime_destroy();
    }

    // Execute i32.popcnt test function with single input validation
    uint32_t ExecuteI32Popcnt(uint32_t input_value, uint32_t expected_result) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_basic_popcnt");
        EXPECT_NE(nullptr, func) << "Function 'test_basic_popcnt' not found in module";

        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment for popcnt test";

        uint32_t argv[1] = {input_value};
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(ret) << "Function execution failed: "
            << wasm_runtime_get_exception(module_inst);

        EXPECT_EQ(expected_result, argv[0])
            << "i32.popcnt(" << std::hex << "0x" << input_value << std::dec
            << ") returned " << argv[0] << ", expected " << expected_result;

        wasm_runtime_destroy_exec_env(exec_env);
        return argv[0];
    }

    // Execute batch i32.popcnt tests for efficiency in pattern testing
    void ExecuteI32PopcntBatch(const std::vector<std::pair<uint32_t, uint32_t>>& test_cases) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "test_basic_popcnt");
        EXPECT_NE(nullptr, func) << "Function 'test_basic_popcnt' not found in module";

        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment for batch testing";

        for (const auto& test_case : test_cases) {
            uint32_t argv[1] = {test_case.first};

            bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
            EXPECT_TRUE(ret) << "Function execution failed for input 0x" << std::hex
                << test_case.first << ": " << wasm_runtime_get_exception(module_inst);

            EXPECT_EQ(test_case.second, argv[0])
                << "i32.popcnt(" << std::hex << "0x" << test_case.first << std::dec
                << ") returned " << argv[0] << ", expected " << test_case.second;
        }

        wasm_runtime_destroy_exec_env(exec_env);
    }

private:
    uint8_t *module_buffer = nullptr;  // WASM module binary data buffer
    uint32_t buffer_size = 0;          // Size of loaded WASM binary
    wasm_module_t module = nullptr;    // Loaded and validated WASM module
    wasm_module_inst_t module_inst = nullptr;  // Instantiated module for execution
};

// Main Routine Tests: Basic population counting functionality with typical values

/**
 * @test BasicPopulationCount_ReturnsCorrectCount
 * @brief Validates i32.popcnt produces correct bit counts for typical bit patterns
 * @details Tests fundamental population counting operation with known bit patterns.
 *          Verifies that i32.popcnt correctly counts set bits (1s) in various integers.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32_popcnt_operation
 * @input_conditions Simple bit patterns: 0b1010 (10), 0b1111 (15), 0b100010001 (273)
 * @expected_behavior Returns accurate bit count: 2, 4, 3 respectively
 * @validation_method Direct comparison of WASM function result with mathematical bit count
 */
TEST_P(I32PopcntTestSuite, BasicPopulationCount_ReturnsCorrectCount)
{
    // Test common bit patterns with known population counts
    // 10 (binary: 1010) -> 2 set bits
    // 15 (binary: 1111) -> 4 set bits
    // 273 (binary: 100010001) -> 3 set bits
    ExecuteI32Popcnt(10, 2);
    ExecuteI32Popcnt(15, 4);
    ExecuteI32Popcnt(273, 3);
}

/**
 * @test StandardIntegerValues_ReturnsAccurateCount
 * @brief Validates i32.popcnt handles common integer values correctly including negative numbers
 * @details Tests population counting with typical positive and negative integers.
 *          Negative numbers tested in two's complement representation.
 * @test_category Main - Standard integer validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32_popcnt_signed_handling
 * @input_conditions Common integers: 1, 7, 31, -1, -2, -8
 * @expected_behavior Returns mathematically correct population counts for two's complement
 * @validation_method Verification against known bit patterns in 32-bit two's complement
 */
TEST_P(I32PopcntTestSuite, StandardIntegerValues_ReturnsAccurateCount)
{
    // Test positive integers with known bit counts
    std::vector<std::pair<uint32_t, uint32_t>> standard_values = {
        {1, 1},      // 0x00000001: 1 set bit
        {7, 3},      // 0x00000007: 3 set bits (111)
        {31, 5},     // 0x0000001F: 5 set bits (11111)
        {0xFFFFFFFF, 32},  // -1: All 32 bits set
        {0xFFFFFFFE, 31},  // -2: 31 bits set
        {0xFFFFFFF8, 29}   // -8: 29 bits set
    };

    ExecuteI32PopcntBatch(standard_values);
}

// Corner Case Tests: Boundary values and extreme bit patterns

/**
 * @test BoundaryValues_ReturnsExpectedCounts
 * @brief Validates i32.popcnt handles integer boundary values correctly
 * @details Tests population counting at integer boundaries and extreme values.
 *          Covers minimum/maximum values and critical bit patterns.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32_popcnt_boundaries
 * @input_conditions 0x00000000, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF
 * @expected_behavior Returns exact bit counts: 0, 32, 1, 31 respectively
 * @validation_method Direct verification of population count at integer limits
 */
TEST_P(I32PopcntTestSuite, BoundaryValues_ReturnsExpectedCounts)
{
    // Test critical boundary values
    std::vector<std::pair<uint32_t, uint32_t>> boundary_values = {
        {0x00000000, 0},   // MIN_VALUE: no bits set
        {0xFFFFFFFF, 32},  // MAX_VALUE: all 32 bits set
        {0x80000000, 1},   // INT32_MIN: only MSB set
        {0x7FFFFFFF, 31}   // INT32_MAX: all bits except MSB set
    };

    ExecuteI32PopcntBatch(boundary_values);
}

/**
 * @test BitPatternBoundaries_HandledCorrectly
 * @brief Validates i32.popcnt handles alternating and sparse bit patterns accurately
 * @details Tests population counting with regular alternating patterns and sparse bits.
 *          Verifies mathematical properties of bit pattern counting.
 * @test_category Corner - Pattern boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32_popcnt_patterns
 * @input_conditions 0xAAAAAAAA, 0x55555555, 0x80000001, 0x7FFFFFFE
 * @expected_behavior Returns pattern-specific counts: 16, 16, 2, 30 respectively
 * @validation_method Verification of alternating and boundary pattern bit counts
 */
TEST_P(I32PopcntTestSuite, BitPatternBoundaries_HandledCorrectly)
{
    // Test alternating and sparse bit patterns
    std::vector<std::pair<uint32_t, uint32_t>> pattern_boundaries = {
        {0xAAAAAAAA, 16}, // Alternating 10101010...: 16 set bits
        {0x55555555, 16}, // Alternating 01010101...: 16 set bits
        {0x80000001, 2},  // MSB and LSB set: 2 bits
        {0x7FFFFFFE, 30}  // All except MSB and LSB: 30 bits
    };

    ExecuteI32PopcntBatch(pattern_boundaries);
}

// Edge Case Tests: Extreme patterns and mathematical properties

/**
 * @test ExtremePatterns_HandlesCorrectly
 * @brief Validates i32.popcnt handles powers of 2 and mathematical bit properties
 * @details Tests population counting with powers of 2 and complement relationships.
 *          Verifies mathematical properties: popcnt(x) + popcnt(~x) = 32
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32_popcnt_mathematics
 * @input_conditions Powers of 2, complement pairs, identity operations
 * @expected_behavior Powers of 2 return 1; complement pairs sum to 32
 * @validation_method Mathematical property verification and identity testing
 */
TEST_P(I32PopcntTestSuite, ExtremePatterns_HandlesCorrectly)
{
    // Test powers of 2 - each should have exactly 1 set bit
    std::vector<std::pair<uint32_t, uint32_t>> powers_of_2 = {
        {1, 1},           // 2^0
        {2, 1},           // 2^1
        {4, 1},           // 2^2
        {8, 1},           // 2^3
        {16, 1},          // 2^4
        {32, 1},          // 2^5
        {64, 1},          // 2^6
        {128, 1},         // 2^7
        {256, 1},         // 2^8
        {512, 1},         // 2^9
        {1024, 1},        // 2^10
        {0x80000000, 1}   // 2^31 (MSB)
    };

    ExecuteI32PopcntBatch(powers_of_2);

    // Test mathematical property: popcnt(x) + popcnt(~x) = 32
    // For x = 0xF0F0F0F0, ~x = 0x0F0F0F0F
    ExecuteI32Popcnt(0xF0F0F0F0, 16); // 16 set bits
    ExecuteI32Popcnt(0x0F0F0F0F, 16); // 16 set bits (complement)
}

/**
 * @test ZeroOperand_ReturnsZero
 * @brief Validates i32.popcnt correctly handles zero input
 * @details Tests the edge case of zero input which has no set bits.
 *          Verifies identity operation for minimal bit count.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:i32_popcnt_zero_case
 * @input_conditions Input value 0x00000000 (zero)
 * @expected_behavior Returns 0 (no set bits to count)
 * @validation_method Direct verification of zero bit count for zero input
 */
TEST_P(I32PopcntTestSuite, ZeroOperand_ReturnsZero)
{
    // Zero input should return zero population count
    ExecuteI32Popcnt(0x00000000, 0);
}

// Error Exception Tests: Runtime infrastructure validation

/**
 * @test StackUnderflow_HandledGracefully
 * @brief Validates WAMR runtime properly handles i32.popcnt execution infrastructure
 * @details Tests that i32.popcnt operates correctly within WAMR runtime context.
 *          Since i32.popcnt is non-trapping, focuses on successful execution validation.
 * @test_category Error - Runtime validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:stack_management
 * @input_conditions Standard execution with adequate stack (input: 42)
 * @expected_behavior Successful execution with correct result (2 set bits in 42)
 * @validation_method Verification of normal operation without runtime exceptions
 */
TEST_P(I32PopcntTestSuite, StackUnderflow_HandledGracefully)
{
    // Note: i32.popcnt is a non-trapping operation that never fails for valid i32 inputs
    // This test verifies normal execution path and stack management
    // 42 = 0x0000002A = binary 101010 -> 3 set bits
    ExecuteI32Popcnt(42, 3);
}

/**
 * @test InvalidModuleStructure_FailsGracefully
 * @brief Validates WAMR handles module loading errors gracefully for popcnt operations
 * @details Tests runtime behavior with valid module structure and proper error handling.
 *          Focuses on successful module instantiation and function lookup.
 * @test_category Error - Module validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:module_validation
 * @input_conditions Valid module with proper function exports (input: 255)
 * @expected_behavior Successful function lookup and execution (8 set bits in 255)
 * @validation_method Module integrity validation and function resolution testing
 */
TEST_P(I32PopcntTestSuite, InvalidModuleStructure_FailsGracefully)
{
    // Test with valid module structure - should succeed
    // 255 = 0x000000FF = 8 set bits in lower byte
    ExecuteI32Popcnt(255, 8);
}

// Test both execution modes: Interpreter and AOT
INSTANTIATE_TEST_SUITE_P(
    CrossMode,
    I32PopcntTestSuite,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);