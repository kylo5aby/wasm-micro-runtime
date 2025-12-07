/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

extern "C" {
#include "wasm_export.h"
#include "bh_read_file.h"
#include "wasm_runtime_common.h"
}

/**
 * @brief Test fixture for i64.lt_u opcode validation
 * @details Provides comprehensive testing infrastructure for WebAssembly i64.lt_u
 *          instruction across different WAMR execution modes (interpreter and AOT).
 *          Tests unsigned 64-bit integer less-than comparison operations.
 */
class I64LtUTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Initialize WAMR runtime and load test module
     * @details Sets up WAMR runtime environment with proper memory allocation
     *          and loads the i64.lt_u test module for execution validation.
     */
    void SetUp() override {
        // Initialize WAMR runtime with system allocator
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load test WASM module
        module_buffer = LoadWASMModule();
        ASSERT_NE(nullptr, module_buffer) << "Failed to load WASM module buffer";

        // Load module into WAMR runtime
        char error_buf[128] = {0};
        module = wasm_runtime_load(module_buffer, module_buffer_size,
                                   error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module)
            << "Failed to load WASM module: " << error_buf;

        // Instantiate module with execution mode
        module_inst = wasm_runtime_instantiate(module, 65536, 65536,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode for parameterized testing
        wasm_runtime_set_running_mode(module_inst, GetParam());

        // Get function reference
        lt_u_func = wasm_runtime_lookup_function(module_inst, "i64_lt_u_test");
        ASSERT_NE(nullptr, lt_u_func)
            << "Failed to lookup i64_lt_u_test function";
    }

    /**
     * @brief Clean up WAMR runtime resources
     * @details Properly deallocates all WAMR runtime resources including
     *          module instances, modules, and runtime environment.
     */
    void TearDown() override {
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (module_buffer) {
            delete[] module_buffer;
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM test module from file system
     * @details Reads the compiled WASM module containing i64.lt_u test function
     * @return Pointer to module buffer or nullptr on failure
     */
    unsigned char* LoadWASMModule() {
        const char* wasm_file = "wasm-apps/i64_lt_u_test.wasm";
        return (unsigned char*)bh_read_file_to_buffer(wasm_file, &module_buffer_size);
    }

    /**
     * @brief Execute i64.lt_u test function with given inputs
     * @details Calls the WASM function that applies i64.lt_u to two 64-bit inputs
     * @param first The first 64-bit integer operand
     * @param second The second 64-bit integer operand
     * @return The comparison result as 32-bit integer (1 if first < second unsigned, 0 otherwise)
     */
    int32_t CallI64LtU(uint64_t first, uint64_t second) {
        wasm_val_t args[2];
        wasm_val_t results[1];

        // Set up function arguments
        args[0].kind = WASM_I64;
        args[0].of.i64 = first;
        args[1].kind = WASM_I64;
        args[1].of.i64 = second;

        // Execute function
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        bool success = wasm_runtime_call_wasm_a(exec_env, lt_u_func, 1, results, 2, args);
        wasm_runtime_destroy_exec_env(exec_env);

        EXPECT_TRUE(success) << "Function call failed";
        return results[0].of.i32;
    }

protected:
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_function_inst_t lt_u_func = nullptr;
    unsigned char* module_buffer = nullptr;
    uint32_t module_buffer_size = 0;
};

/**
 * @test BasicComparison_ValidInputs_ReturnsCorrectResults
 * @brief Validates fundamental unsigned comparison for typical input values
 * @details Tests core i64.lt_u functionality with representative unsigned integer values.
 *          Verifies that comparison treats values as unsigned integers where large values
 *          like 0x8000000000000000 are positive, not negative.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Standard unsigned comparisons: (5, 10), (0x7FFFFFFFFFFFFFFF, 0x8000000000000000)
 * @expected_behavior Returns 1 for true comparisons, 0 for false comparisons
 * @validation_method Direct comparison of WASM function result with expected boolean values
 */
TEST_P(I64LtUTest, BasicComparison_ValidInputs_ReturnsCorrectResults) {
    // Test basic positive unsigned comparison
    ASSERT_EQ(1, CallI64LtU(5, 10))
        << "Failed basic unsigned comparison: 5 < 10 should be true";

    // Test false comparison
    ASSERT_EQ(0, CallI64LtU(10, 5))
        << "Failed basic unsigned comparison: 10 < 5 should be false";

    // Test unsigned interpretation: largest signed positive < smallest "negative" (but unsigned positive)
    ASSERT_EQ(1, CallI64LtU(0x7FFFFFFFFFFFFFFFULL, 0x8000000000000000ULL))
        << "Failed unsigned comparison: 0x7FFFFFFFFFFFFFFF < 0x8000000000000000 should be true in unsigned";

    // Test medium-range values
    ASSERT_EQ(1, CallI64LtU(1000ULL, 2000ULL))
        << "Failed medium-range unsigned comparison: 1000 < 2000 should be true";

    // Test large unsigned values
    ASSERT_EQ(1, CallI64LtU(0x8000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL))
        << "Failed large unsigned comparison: 0x8000000000000000 < 0xFFFFFFFFFFFFFFFF should be true";
}

/**
 * @test BoundaryValues_ZeroAndMax_ReturnsExpectedResults
 * @brief Tests critical boundary conditions at unsigned 64-bit limits
 * @details Validates comparison behavior at exact boundaries: zero, maximum unsigned value,
 *          and transitions around signed/unsigned interpretation differences.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Boundary values: 0, UINT64_MAX-1, UINT64_MAX, signed/unsigned boundaries
 * @expected_behavior Mathematically correct unsigned comparisons at all boundaries
 * @validation_method Verify boundary cases produce correct unsigned comparison results
 */
TEST_P(I64LtUTest, BoundaryValues_ZeroAndMax_ReturnsExpectedResults) {
    // Test zero boundary conditions
    ASSERT_EQ(1, CallI64LtU(0ULL, 1ULL))
        << "Failed zero boundary: 0 < 1 should be true";
    ASSERT_EQ(0, CallI64LtU(1ULL, 0ULL))
        << "Failed zero boundary: 1 < 0 should be false";
    ASSERT_EQ(0, CallI64LtU(0ULL, 0ULL))
        << "Failed zero boundary: 0 < 0 should be false";

    // Test maximum value boundary conditions
    ASSERT_EQ(1, CallI64LtU(0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFFULL))
        << "Failed max boundary: (UINT64_MAX-1) < UINT64_MAX should be true";
    ASSERT_EQ(0, CallI64LtU(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL))
        << "Failed max boundary: UINT64_MAX < (UINT64_MAX-1) should be false";

    // Test zero compared to maximum
    ASSERT_EQ(1, CallI64LtU(0ULL, 0xFFFFFFFFFFFFFFFFULL))
        << "Failed zero to max: 0 < UINT64_MAX should be true";

    // Test signed/unsigned boundary (0x8000000000000000 is positive in unsigned)
    ASSERT_EQ(0, CallI64LtU(0x8000000000000000ULL, 0x7FFFFFFFFFFFFFFFULL))
        << "Failed signed/unsigned boundary: 0x8000000000000000 < 0x7FFFFFFFFFFFFFFF should be false in unsigned";
}

/**
 * @test SignedUnsignedBoundary_LargeValues_ReturnsUnsignedComparison
 * @brief Confirms unsigned semantics override signed interpretation for large values
 * @details Validates that i64.lt_u treats all values as unsigned, ensuring that values
 *          which would be negative in signed interpretation (0x8000000000000000-0xFFFFFFFFFFFFFFFF)
 *          are correctly treated as large positive values.
 * @test_category Corner - Signed vs unsigned interpretation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Large values that differ in signed vs unsigned interpretation
 * @expected_behavior Unsigned comparison behavior, not signed comparison behavior
 * @validation_method Compare results against unsigned arithmetic, not signed arithmetic
 */
TEST_P(I64LtUTest, SignedUnsignedBoundary_LargeValues_ReturnsUnsignedComparison) {
    // Values that are "negative" in signed but positive in unsigned
    // In signed: -1 < -2 is false, but in unsigned: 0xFFFFFFFFFFFFFFFF < 0xFFFFFFFFFFFFFFFE is false
    ASSERT_EQ(0, CallI64LtU(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL))
        << "Failed unsigned semantics: 0xFFFFFFFFFFFFFFFF < 0xFFFFFFFFFFFFFFFE should be false in unsigned";

    // In signed: -1 < 1 is true, but in unsigned: 0xFFFFFFFFFFFFFFFF < 1 is false
    ASSERT_EQ(0, CallI64LtU(0xFFFFFFFFFFFFFFFFULL, 1ULL))
        << "Failed unsigned semantics: 0xFFFFFFFFFFFFFFFF < 1 should be false in unsigned";

    // In signed: -2 < -1 is true, but in unsigned: 0xFFFFFFFFFFFFFFFE < 0xFFFFFFFFFFFFFFFF is true
    ASSERT_EQ(1, CallI64LtU(0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFFULL))
        << "Failed unsigned semantics: 0xFFFFFFFFFFFFFFFE < 0xFFFFFFFFFFFFFFFF should be true in unsigned";

    // Test boundary at sign bit
    ASSERT_EQ(1, CallI64LtU(0x8000000000000001ULL, 0xFFFFFFFFFFFFFFFFULL))
        << "Failed unsigned semantics: 0x8000000000000001 < 0xFFFFFFFFFFFFFFFF should be true in unsigned";
}

/**
 * @test MathematicalProperties_AntiReflexive_ReturnsFalse
 * @brief Validates anti-reflexive property of less-than relation
 * @details Tests that any value compared with itself always returns false (a < a = false).
 *          Validates mathematical correctness of the less-than relation across various
 *          representative values including boundaries.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Self-comparison of various values: 0, typical values, boundary values
 * @expected_behavior All self-comparisons return false
 * @validation_method Verify a < a returns 0 for all test values
 */
TEST_P(I64LtUTest, MathematicalProperties_AntiReflexive_ReturnsFalse) {
    // Test anti-reflexive property: a < a is always false
    ASSERT_EQ(0, CallI64LtU(0ULL, 0ULL))
        << "Failed anti-reflexive property: 0 < 0 should be false";
    ASSERT_EQ(0, CallI64LtU(42ULL, 42ULL))
        << "Failed anti-reflexive property: 42 < 42 should be false";
    ASSERT_EQ(0, CallI64LtU(0x7FFFFFFFFFFFFFFFULL, 0x7FFFFFFFFFFFFFFFULL))
        << "Failed anti-reflexive property: 0x7FFFFFFFFFFFFFFF < 0x7FFFFFFFFFFFFFFF should be false";
    ASSERT_EQ(0, CallI64LtU(0x8000000000000000ULL, 0x8000000000000000ULL))
        << "Failed anti-reflexive property: 0x8000000000000000 < 0x8000000000000000 should be false";
    ASSERT_EQ(0, CallI64LtU(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL))
        << "Failed anti-reflexive property: 0xFFFFFFFFFFFFFFFF < 0xFFFFFFFFFFFFFFFF should be false";
}

/**
 * @test MathematicalProperties_Transitive_ReturnsConsistent
 * @brief Validates transitive property of less-than relation
 * @details Tests that if a < b and b < c, then a < c. Ensures mathematical consistency
 *          of the unsigned less-than comparison across chains of values.
 * @test_category Edge - Mathematical transitivity validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Chains of values where a < b < c should hold
 * @expected_behavior Transitive property maintained across all test chains
 * @validation_method Verify transitivity with multiple value sequences
 */
TEST_P(I64LtUTest, MathematicalProperties_Transitive_ReturnsConsistent) {
    // Test transitive property: if a < b and b < c, then a < c
    uint64_t a = 100ULL;
    uint64_t b = 200ULL;
    uint64_t c = 300ULL;

    // Verify preconditions
    ASSERT_EQ(1, CallI64LtU(a, b)) << "Precondition failed: a < b";
    ASSERT_EQ(1, CallI64LtU(b, c)) << "Precondition failed: b < c";

    // Verify transitive conclusion
    ASSERT_EQ(1, CallI64LtU(a, c))
        << "Failed transitive property: if a < b and b < c, then a < c should be true";

    // Test transitivity with boundary values
    uint64_t x = 0ULL;
    uint64_t y = 0x8000000000000000ULL;
    uint64_t z = 0xFFFFFFFFFFFFFFFFULL;

    ASSERT_EQ(1, CallI64LtU(x, y)) << "Precondition failed: x < y";
    ASSERT_EQ(1, CallI64LtU(y, z)) << "Precondition failed: y < z";
    ASSERT_EQ(1, CallI64LtU(x, z))
        << "Failed transitive property with boundary values: x < z should be true";
}

/**
 * @test StackUnderflow_InsufficientValues_FailsGracefully
 * @brief Tests runtime behavior with insufficient stack values
 * @details Validates that WAMR handles modules with insufficient stack values gracefully,
 *          detecting stack underflow conditions before runtime execution.
 * @test_category Error - Stack underflow error handling
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_load
 * @input_conditions WASM module with i64.lt_u but insufficient operands on stack
 * @expected_behavior Graceful module load failure with descriptive error message
 * @validation_method Verify module load fails and error buffer contains relevant message
 */
TEST_P(I64LtUTest, StackUnderflow_InsufficientValues_FailsGracefully) {
    // Create a simple WASM module that attempts i64.lt_u without sufficient stack values
    // WASM binary with function that has i64.lt_u but only one i64 constant on stack
    unsigned char underflow_wasm[] = {
        0x00, 0x61, 0x73, 0x6D,  // WASM magic
        0x01, 0x00, 0x00, 0x00,  // Version
        0x01, 0x07,              // Type section
        0x01,                    // 1 type
        0x60, 0x00, 0x01, 0x7F,  // Function type: () -> i32
        0x03, 0x02,              // Function section
        0x01, 0x00,              // 1 function of type 0
        0x0A, 0x09,              // Code section
        0x01,                    // 1 function
        0x07, 0x00,              // Function body size and local count
        0x42, 0x01,              // i64.const 1 (only one operand)
        0x55,                    // i64.lt_u (needs two operands)
        0x0B                     // end
    };

    char error_buf[256] = {0};
    wasm_module_t underflow_module = wasm_runtime_load(underflow_wasm, sizeof(underflow_wasm),
                                                       error_buf, sizeof(error_buf));

    ASSERT_EQ(nullptr, underflow_module)
        << "Expected module load to fail for stack underflow condition";
    ASSERT_NE(0, strlen(error_buf))
        << "Expected error message for stack underflow validation failure";
}

/**
 * @test TypeMismatch_WrongTypes_FailsValidation
 * @brief Tests runtime behavior with incorrect operand types on stack
 * @details Validates that WAMR detects type mismatches when i64.lt_u encounters
 *          non-i64 operands on the stack, ensuring type safety.
 * @test_category Error - Type validation error handling
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_load
 * @input_conditions WASM module with i64.lt_u but i32 operands on stack
 * @expected_behavior Module load failure with type validation error
 * @validation_method Verify module load fails for type mismatch conditions
 */
TEST_P(I64LtUTest, TypeMismatch_WrongTypes_FailsValidation) {
    // Create WASM module that attempts i64.lt_u with wrong operand types (i32 instead of i64)
    unsigned char type_mismatch_wasm[] = {
        0x00, 0x61, 0x73, 0x6D,  // WASM magic
        0x01, 0x00, 0x00, 0x00,  // Version
        0x01, 0x07,              // Type section
        0x01,                    // 1 type
        0x60, 0x00, 0x01, 0x7F,  // Function type: () -> i32
        0x03, 0x02,              // Function section
        0x01, 0x00,              // 1 function of type 0
        0x0A, 0x0C,              // Code section
        0x01,                    // 1 function
        0x0A, 0x00,              // Function body size and local count
        0x41, 0x01,              // i32.const 1 (wrong type)
        0x41, 0x02,              // i32.const 2 (wrong type)
        0x55,                    // i64.lt_u (expects i64 operands)
        0x0B                     // end
    };

    char error_buf[256] = {0};
    wasm_module_t type_mismatch_module = wasm_runtime_load(type_mismatch_wasm, sizeof(type_mismatch_wasm),
                                                           error_buf, sizeof(error_buf));

    ASSERT_EQ(nullptr, type_mismatch_module)
        << "Expected module load to fail for type mismatch";
    ASSERT_NE(0, strlen(error_buf))
        << "Expected error message for type validation failure";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionModeValidation,
    I64LtUTest,
    testing::Values(
        RunningMode::Mode_Interp,
        RunningMode::Mode_LLVM_JIT
    ),
    [](const testing::TestParamInfo<I64LtUTest::ParamType>& info) {
        switch (info.param) {
            case RunningMode::Mode_Interp:
                return "InterpreterMode";
            case RunningMode::Mode_LLVM_JIT:
                return "AOTMode";
            default:
                return "UnknownMode";
        }
    }
);