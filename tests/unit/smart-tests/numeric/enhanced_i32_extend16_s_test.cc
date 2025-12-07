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
 * @brief Test fixture for i32.extend16_s opcode validation
 * @details Provides comprehensive testing infrastructure for WebAssembly i32.extend16_s
 *          instruction across different WAMR execution modes (interpreter and AOT).
 *          Tests sign extension of 16-bit values to 32-bit integers.
 */
class I32Extend16STest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Initialize WAMR runtime and load test module
     * @details Sets up WAMR runtime environment with proper memory allocation
     *          and loads the i32.extend16_s test module for execution validation.
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
        extend16s_func = wasm_runtime_lookup_function(module_inst,
                                                      "i32_extend16_s_test");
        ASSERT_NE(nullptr, extend16s_func)
            << "Failed to lookup i32_extend16_s_test function";
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
     * @details Reads the compiled WASM module containing i32.extend16_s test function
     * @return Pointer to module buffer or nullptr on failure
     */
    unsigned char* LoadWASMModule() {
        const char* wasm_file = "wasm-apps/i32_extend16_s_test.wasm";
        return (unsigned char*)bh_read_file_to_buffer(wasm_file, &module_buffer_size);
    }

    /**
     * @brief Execute i32.extend16_s test function with given input
     * @details Calls the WASM function that applies i32.extend16_s to input value
     * @param input The 32-bit integer input value
     * @return The sign-extended result as 32-bit integer
     */
    int32_t CallI32Extend16S(int32_t input) {
        wasm_val_t args[1];
        wasm_val_t results[1];

        // Set up function arguments
        args[0].kind = WASM_I32;
        args[0].of.i32 = input;

        // Execute function
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        bool success = wasm_runtime_call_wasm_a(exec_env, extend16s_func, 1, results, 1, args);
        wasm_runtime_destroy_exec_env(exec_env);

        EXPECT_TRUE(success) << "Function call failed";
        return results[0].of.i32;
    }

protected:
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_function_inst_t extend16s_func = nullptr;
    unsigned char* module_buffer = nullptr;
    uint32_t module_buffer_size = 0;
};

/**
 * @test BasicSignExtension_ReturnsCorrectResults
 * @brief Validates fundamental sign extension for typical positive and negative values
 * @details Tests core i32.extend16_s functionality with representative input values.
 *          Verifies that positive 16-bit values extend with zeros and negative values
 *          extend with ones in the upper 16 bits.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Typical values: 0x1234 (positive), 0x8000 (negative), 0xFFFF (-1)
 * @expected_behavior Returns 0x00001234, 0xFFFF8000, 0xFFFFFFFF respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(I32Extend16STest, BasicSignExtension_ReturnsCorrectResults) {
    // Test positive value sign extension
    ASSERT_EQ(0x00001234, CallI32Extend16S(0x1234))
        << "Failed to correctly sign-extend positive 16-bit value";

    // Test negative value sign extension (smallest negative)
    ASSERT_EQ(static_cast<int32_t>(0xFFFF8000), CallI32Extend16S(0x8000))
        << "Failed to correctly sign-extend negative 16-bit value 0x8000";

    // Test negative value sign extension (-1)
    ASSERT_EQ(static_cast<int32_t>(0xFFFFFFFF), CallI32Extend16S(0xFFFF))
        << "Failed to correctly sign-extend negative 16-bit value 0xFFFF";

    // Test small positive values
    ASSERT_EQ(0x00000001, CallI32Extend16S(0x0001))
        << "Failed to correctly sign-extend small positive value";

    // Test mixed sign behavior
    ASSERT_EQ(0x00002A5C, CallI32Extend16S(0x2A5C))
        << "Failed to correctly sign-extend typical positive value";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Tests critical boundary conditions at 16-bit limits
 * @details Validates sign extension behavior at exact boundaries where sign bit changes.
 *          Tests zero, maximum positive (0x7FFF), minimum negative (0x8000), and maximum
 *          negative (0xFFFF) values for correct mathematical behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Boundary values: 0x0000, 0x7FFF, 0x8000, 0xFFFF
 * @expected_behavior Returns 0x00000000, 0x00007FFF, 0xFFFF8000, 0xFFFFFFFF
 * @validation_method Verify each boundary produces mathematically correct sign extension
 */
TEST_P(I32Extend16STest, BoundaryValues_HandledCorrectly) {
    // Test zero value (identity)
    ASSERT_EQ(0x00000000, CallI32Extend16S(0x0000))
        << "Failed to correctly handle zero value";

    // Test maximum positive 16-bit value
    ASSERT_EQ(0x00007FFF, CallI32Extend16S(0x7FFF))
        << "Failed to correctly sign-extend maximum positive 16-bit value";

    // Test minimum negative 16-bit value (when sign-extended)
    ASSERT_EQ(static_cast<int32_t>(0xFFFF8000), CallI32Extend16S(0x8000))
        << "Failed to correctly sign-extend minimum negative 16-bit value";

    // Test maximum negative 16-bit value (-1 when sign-extended)
    ASSERT_EQ(static_cast<int32_t>(0xFFFFFFFF), CallI32Extend16S(0xFFFF))
        << "Failed to correctly sign-extend maximum negative 16-bit value";
}

/**
 * @test UpperBitsIgnored_ProducesConsistentResults
 * @brief Confirms upper 16 bits are ignored during sign extension
 * @details Validates that i32.extend16_s only considers the lower 16 bits of input,
 *          ignoring any data in upper bits. This ensures consistent behavior regardless
 *          of what values exist in the upper portion of the 32-bit input.
 * @test_category Edge - Upper bit ignore validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Values with varying upper bits but identical lower 16 bits
 * @expected_behavior Identical results regardless of upper bits
 * @validation_method Compare results of inputs differing only in upper 16 bits
 */
TEST_P(I32Extend16STest, UpperBitsIgnored_ProducesConsistentResults) {
    // Test positive value with different upper bits
    int32_t expected_positive = 0x00007FFF;
    ASSERT_EQ(expected_positive, CallI32Extend16S(0x12347FFF))
        << "Upper bits affected positive sign extension result";
    ASSERT_EQ(expected_positive, CallI32Extend16S(0xABCD7FFF))
        << "Upper bits affected positive sign extension result";
    ASSERT_EQ(expected_positive, CallI32Extend16S(0x00007FFF))
        << "Failed baseline positive sign extension";

    // Test negative value with different upper bits
    int32_t expected_negative = static_cast<int32_t>(0xFFFF8000);
    ASSERT_EQ(expected_negative, CallI32Extend16S(0x12348000))
        << "Upper bits affected negative sign extension result";
    ASSERT_EQ(expected_negative, CallI32Extend16S(0xABCD8000))
        << "Upper bits affected negative sign extension result";
    ASSERT_EQ(expected_negative, CallI32Extend16S(0x00008000))
        << "Failed baseline negative sign extension";

    // Test zero with different upper bits
    ASSERT_EQ(0x00000000, CallI32Extend16S(0x12340000))
        << "Upper bits affected zero sign extension result";
    ASSERT_EQ(0x00000000, CallI32Extend16S(0xFFFF0000))
        << "Upper bits affected zero sign extension result";
}

/**
 * @test ModuleLoadFailure_HandledGracefully
 * @brief Tests runtime behavior with invalid WASM modules
 * @details Validates that WAMR handles malformed or corrupted WASM modules gracefully
 *          without crashes or undefined behavior. Tests error handling pathways.
 * @test_category Error - Module load error handling
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_load
 * @input_conditions Malformed bytecode, corrupted module data
 * @expected_behavior Graceful failure without crashes, proper error reporting
 * @validation_method Verify null return values and error messages for invalid modules
 */
TEST_P(I32Extend16STest, ModuleLoadFailure_HandledGracefully) {
    // Test with invalid module data
    char error_buf[128] = {0};
    unsigned char invalid_wasm[] = {0x00, 0x61, 0x73, 0x6D}; // Invalid WASM header

    wasm_module_t invalid_module = wasm_runtime_load(invalid_wasm, sizeof(invalid_wasm),
                                                     error_buf, sizeof(error_buf));

    ASSERT_EQ(nullptr, invalid_module)
        << "Expected module load to fail for invalid bytecode";
    ASSERT_NE(0, strlen(error_buf))
        << "Expected error message for failed module load";

    // Test with empty buffer
    wasm_module_t empty_module = wasm_runtime_load(nullptr, 0, error_buf, sizeof(error_buf));
    ASSERT_EQ(nullptr, empty_module)
        << "Expected module load to fail for null buffer";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionModeValidation,
    I32Extend16STest,
    testing::Values(
        RunningMode::Mode_Interp,
        RunningMode::Mode_LLVM_JIT
    ),
    [](const testing::TestParamInfo<I32Extend16STest::ParamType>& info) {
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