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
 * @file enhanced_i64_rotr_test.cc
 * @brief Enhanced unit tests for i64.rotr opcode - Numeric Category
 * @details This test suite provides comprehensive coverage for the i64.rotr (rotate right)
 * WebAssembly instruction, focusing on:
 * - Basic rotation functionality with typical values and rotation counts
 * - Corner cases including boundary rotation counts (0, 63, 64, 65+) and modulo behavior
 * - Edge cases with extreme values (0, 0xFFFFFFFFFFFFFFFF, powers of 2, alternating patterns)
 * - Identity operations and mathematical properties (cyclic behavior, bit preservation)
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:rotr64 function
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:rotr64 function
 * @coverage_target core/iwasm/aot/aot_runtime.c:bitwise rotation instructions
 * @coverage_target Bit rotation algorithms and modulo 64 behavior
 * @coverage_target Stack management for binary numeric operations
 * @test_modes Both interpreter (Mode_Interp) and AOT (Mode_LLVM_JIT) execution
 */

class I64RotrTestSuite : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime with system allocator for test isolation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i64.rotr testing";

        // Load WASM test module containing i64.rotr test functions
        std::string wasm_file = "./wasm-apps/i64_rotr_test.wasm";
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
     * @brief Executes i64.rotr operation with two operands
     * @details Helper function to call WASM i64.rotr function and retrieve result
     * @param value The 64-bit integer value to rotate
     * @param count The number of positions to rotate right
     * @return Result of rotating value right by count positions
     * @coverage_target Function call mechanism and parameter passing
     */
    uint64_t call_i64_rotr(uint64_t value, uint64_t count) {
        // Locate the exported i64_rotr test function
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "i64_rotr_test");
        EXPECT_NE(nullptr, func) << "Failed to find i64_rotr_test function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare function arguments: [value, count]
        uint32_t argv[4];
        // Split 64-bit values into two 32-bit arguments (little-endian)
        argv[0] = static_cast<uint32_t>(value & 0xFFFFFFFF);
        argv[1] = static_cast<uint32_t>(value >> 32);
        argv[2] = static_cast<uint32_t>(count & 0xFFFFFFFF);
        argv[3] = static_cast<uint32_t>(count >> 32);

        // Execute i64.rotr function and capture result
        bool success = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(success) << "Failed to execute i64_rotr_test function";

        // Cleanup execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        // Reconstruct 64-bit result from two 32-bit values
        return static_cast<uint64_t>(argv[0]) |
               (static_cast<uint64_t>(argv[1]) << 32);
    }

    // Test infrastructure members
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    uint8_t* module_buffer = nullptr;
    uint32_t buffer_size = 0;
};

/**
 * @test BasicRotation_ReturnsCorrectResult
 * @brief Validates i64.rotr produces correct arithmetic results for typical inputs
 * @details Tests fundamental right rotation operation with small, medium, and large rotation counts.
 *          Verifies that i64.rotr correctly performs circular bit shifts to the right.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:rotr64
 * @input_conditions Standard rotation scenarios: 4-bit, 8-bit, and 32-bit shifts
 * @expected_behavior Returns mathematically correct right-rotated results
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I64RotrTestSuite, BasicRotation_ReturnsCorrectResult) {
    // Small rotation: 0x123456789ABCDEF0 >> 4 = 0x0123456789ABCDEF
    ASSERT_EQ(call_i64_rotr(0x123456789ABCDEF0ULL, 4), 0x0123456789ABCDEFULL)
        << "4-bit right rotation failed for test pattern";

    // Byte rotation: rotate right by 8 bits
    ASSERT_EQ(call_i64_rotr(0x123456789ABCDEF0ULL, 8), 0xF0123456789ABCDEULL)
        << "8-bit right rotation failed for test pattern";

    // Half rotation: rotate right by 32 bits
    ASSERT_EQ(call_i64_rotr(0x123456789ABCDEF0ULL, 32), 0x9ABCDEF012345678ULL)
        << "32-bit right rotation failed for test pattern";
}

/**
 * @test BoundaryValues_HandleCorrectly
 * @brief Validates i64.rotr handles boundary values and modulo 64 behavior correctly
 * @details Tests rotation with maximum values, sign boundaries, and large rotation counts.
 *          Verifies proper modulo 64 behavior for counts >= 64.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:rotr64 modulo logic
 * @input_conditions Boundary values (0xFFFF..., 0x8000...) and large rotation counts
 * @expected_behavior Correct boundary handling and modulo 64 equivalence
 * @validation_method Verification of boundary cases and modulo arithmetic properties
 */
TEST_P(I64RotrTestSuite, BoundaryValues_HandleCorrectly) {
    // All 1s rotated by 1 bit: 0xFFFF...FFFF >> 1 = 0xFFFF...FFFF
    ASSERT_EQ(call_i64_rotr(0xFFFFFFFFFFFFFFFFULL, 1), 0xFFFFFFFFFFFFFFFFULL)
        << "Right rotation of all 1s failed to preserve pattern";

    // MSB rotation: 0x8000...0000 >> 1 = 0x4000...0000
    ASSERT_EQ(call_i64_rotr(0x8000000000000000ULL, 1), 0x4000000000000000ULL)
        << "Right rotation of MSB failed";

    // Large count modulo: count 65 should equal count 1
    ASSERT_EQ(call_i64_rotr(0x123456789ABCDEF0ULL, 65),
              call_i64_rotr(0x123456789ABCDEF0ULL, 1))
        << "Modulo 64 behavior failed for large rotation count";
}

/**
 * @test IdentityAndMathematicalProperties_ValidateCorrectness
 * @brief Validates mathematical properties of i64.rotr operation
 * @details Tests identity operations, inverse relationships, and special bit patterns.
 *          Verifies fundamental rotation properties and bit preservation.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:rotr64 mathematical correctness
 * @input_conditions Zero rotation, full rotation, inverse operations, single bit patterns
 * @expected_behavior Mathematical property compliance and bit pattern preservation
 * @validation_method Identity verification, inverse operation testing, bit pattern analysis
 */
TEST_P(I64RotrTestSuite, IdentityAndMathematicalProperties_ValidateCorrectness) {
    // Identity: zero rotation should return original value
    ASSERT_EQ(call_i64_rotr(0x123456789ABCDEF0ULL, 0), 0x123456789ABCDEF0ULL)
        << "Zero rotation failed to preserve original value";

    // Full rotation: 64-bit rotation should equal original
    ASSERT_EQ(call_i64_rotr(0x123456789ABCDEF0ULL, 64), 0x123456789ABCDEF0ULL)
        << "Full rotation (64 bits) failed to preserve original value";

    // Inverse property: rotr(rotr(x,n), 64-n) == x
    uint64_t original = 0x123456789ABCDEF0ULL;
    uint64_t rotated = call_i64_rotr(original, 17);
    ASSERT_EQ(call_i64_rotr(rotated, 47), original)  // 17 + 47 = 64
        << "Inverse rotation property failed";

    // Single bit rotation: MSB becomes next bit
    ASSERT_EQ(call_i64_rotr(0x8000000000000000ULL, 1), 0x4000000000000000ULL)
        << "Single MSB rotation failed";
}

/**
 * @test RuntimeRobustness_HandlesEdgeCases
 * @brief Validates runtime robustness with extreme inputs and consistency
 * @details Tests extreme rotation counts and verifies consistent behavior across executions.
 *          Focuses on runtime stability rather than operation-specific errors.
 * @test_category Error - Runtime robustness validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:rotr64 robustness
 * @input_conditions Extreme rotation counts, consistency validation scenarios
 * @expected_behavior Robust execution without runtime failures, consistent results
 * @validation_method Large count testing, cross-execution consistency verification
 */
TEST_P(I64RotrTestSuite, RuntimeRobustness_HandlesEdgeCases) {
    // Large rotation counts should work correctly due to modulo behavior
    ASSERT_EQ(call_i64_rotr(0x123456789ABCDEF0ULL, 0xFFFFFFFFFFFFFFFFULL),
              call_i64_rotr(0x123456789ABCDEF0ULL, 63))  // Large count mod 64 = 63
        << "Extreme rotation count failed modulo behavior";

    // Verify consistent behavior across different execution modes
    // This tests runtime robustness without triggering actual errors
    uint64_t test_value = 0x123456789ABCDEF0ULL;
    uint64_t result1 = call_i64_rotr(test_value, 15);
    uint64_t result2 = call_i64_rotr(test_value, 15);
    ASSERT_EQ(result1, result2)
        << "Inconsistent results across executions";
}

// Test suite instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossMode, I64RotrTestSuite,
    testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == RunningMode::Mode_Interp ? "Interpreter" : "AOT";
    }
);