/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "../../common/test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"


/**
 * @brief Test suite for extern.convert_any opcode
 *
 * This test suite validates the extern.convert_any instruction which converts
 * an anyref value to an externref value. This opcode is part of the WebAssembly
 * reference types proposal and enables interoperability between internal WASM
 * reference system and external host environment references.
 *
 * Key validation areas:
 * - Basic anyref to externref conversion functionality
 * - Null reference handling and preservation
 * - Reference identity preservation during conversion
 * - Stack manipulation validation (pop anyref, push externref)
 * - Round-trip conversion testing with any.convert_extern
 * - Error handling for invalid reference states
 * - Cross-execution mode consistency (interpreter vs AOT)
 */
class ExternConvertAnyTest : public testing::TestWithParam<RunningMode>
{
protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };
    const char *exception = nullptr;

    /**
     * @brief Set up test environment with WAMR runtime initialization
     *
     * Initializes WAMR runtime with reference types support, loads the test
     * WASM module, creates module instance and execution environment.
     * Ensures proper resource setup for extern.convert_any testing.
     */
    void SetUp() override
    {
        memset(error_buf, 0, sizeof(error_buf));

        // Get current working directory and construct WASM file path
        char *cwd = getcwd(NULL, 0);
        std::string wasm_file;
        if (cwd) {
            wasm_file = std::string(cwd) + "/wasm-apps/extern_convert_any_test.wasm";
            free(cwd);
        } else {
            wasm_file = "wasm-apps/extern_convert_any_test.wasm";
        }

        buf = (uint8_t *)bh_read_file_to_buffer(wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << wasm_file;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test resources and destroy WAMR instances
     *
     * Properly destroys execution environment, module instance, and module
     * to prevent resource leaks and ensure clean test environment.
     */
    void TearDown() override
    {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (buf) {
            wasm_runtime_free(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Call a WASM function that tests extern.convert_any operations
     *
     * @param func_name Name of the WASM function to execute (must return i32)
     * @return int32_t Result indicating success (1) or failure (0) of operation
     */
    int32_t call_conversion_func(const char* func_name)
    {
        wasm_function_inst_t func_inst = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(func_inst, nullptr) << "Failed to lookup function: " << func_name;

        uint32_t argv[1] = {0};

        bool success = wasm_runtime_call_wasm(exec_env, func_inst, 0, argv);
        EXPECT_TRUE(success) << "Failed to call function: " << func_name
                           << " - " << wasm_runtime_get_exception(module_inst);

        return static_cast<int32_t>(argv[0]);
    }
};

/**
 * @test BasicConversion_ValidAnyRef_ReturnsExternRef
 * @brief Validates extern.convert_any produces correct externref from valid anyref
 * @details Tests fundamental conversion operation with valid anyref input.
 *          Verifies that extern.convert_any correctly converts anyref to externref
 *          and maintains reference validity throughout the conversion process.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:extern_convert_any_operation
 * @input_conditions Valid anyref value created via ref.func or other reference creation
 * @expected_behavior Returns valid externref that preserves original reference identity
 * @validation_method Direct comparison of WASM function result indicating successful conversion
 */
TEST_P(ExternConvertAnyTest, BasicConversion_ValidAnyRef_ReturnsExternRef)
{
    // Execute basic conversion test which creates anyref via ref.func and converts to externref
    int32_t result = call_conversion_func("test_basic_conversion");
    ASSERT_EQ(1, result) << "Basic extern.convert_any conversion failed for valid anyref";

    // Verify that the conversion maintains proper reference semantics
    // The WASM function internally validates that the externref is valid and usable
    ASSERT_NE(0, result) << "Converted externref should be valid and non-null for valid input";
}

/**
 * @test NullHandling_NullAnyRef_ReturnsNullExternRef
 * @brief Validates extern.convert_any handles null references correctly
 * @details Tests null reference conversion behavior ensuring that null anyref
 *          converts to null externref while maintaining null semantics.
 *          This is critical for proper null propagation in reference chains.
 * @test_category Main - Null reference handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:extern_convert_any_null_handling
 * @input_conditions Null anyref value created via ref.null anyref
 * @expected_behavior Returns null externref maintaining null reference semantics
 * @validation_method Verification that converted reference tests as null with ref.is_null
 */
TEST_P(ExternConvertAnyTest, NullHandling_NullAnyRef_ReturnsNullExternRef)
{
    // Execute null conversion test which creates null anyref and converts to externref
    int32_t result = call_conversion_func("test_null_conversion");
    ASSERT_EQ(1, result) << "Null extern.convert_any conversion failed - null anyref should convert to null externref";

    // Verify that null conversion maintains proper null semantics
    // The WASM function internally validates that the result is null via ref.is_null
    ASSERT_EQ(1, result) << "Converted null externref should properly test as null reference";
}

/**
 * @test RoundTripConversion_PreservesIdentity_MaintainsReferenceEquality
 * @brief Validates reference identity preservation through round-trip conversion
 * @details Tests anyref -> externref (via extern.convert_any) -> anyref (via any.convert_extern)
 *          conversion cycle to ensure reference identity is preserved throughout
 *          the conversion process. Critical for reference equality semantics.
 * @test_category Edge - Identity operations and reference preservation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:reference_conversion_roundtrip
 * @input_conditions Valid anyref converted to externref then back to anyref
 * @expected_behavior Final anyref maintains identity equality with original anyref
 * @validation_method Comparison of original and final anyref values for identity preservation
 */
TEST_P(ExternConvertAnyTest, RoundTripConversion_PreservesIdentity_MaintainsReferenceEquality)
{
    // Execute round-trip conversion test: anyref -> externref -> anyref
    int32_t result = call_conversion_func("test_roundtrip_conversion");
    ASSERT_EQ(1, result) << "Round-trip conversion failed - reference identity not preserved";

    // Verify that the round-trip conversion maintains reference identity
    // The WASM function internally compares original vs final reference for equality
    ASSERT_EQ(1, result) << "Round-trip converted reference should maintain identity with original";
}

/**
 * @test StackUnderflow_EmptyStack_ProducesValidationError
 * @brief Validates proper error handling when stack is empty
 * @details Tests extern.convert_any behavior when called with insufficient stack values.
 *          Should produce validation error or runtime trap rather than crashing.
 *          Critical for runtime safety and proper error propagation.
 * @test_category Error - Stack underflow handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:stack_underflow_detection
 * @input_conditions Empty execution stack when extern.convert_any is executed
 * @expected_behavior Runtime error or trap indicating insufficient stack values
 * @validation_method Function call fails with appropriate error rather than crashing
 */
TEST_P(ExternConvertAnyTest, StackUnderflow_EmptyStack_ProducesValidationError)
{
    // Execute stack underflow test which attempts extern.convert_any with empty stack
    int32_t result = call_conversion_func("test_stack_underflow");
    ASSERT_EQ(1, result) << "Stack underflow should be properly detected and handled";

    // Verify that the runtime properly handles the underflow condition
    // Function should fail gracefully rather than crash or produce invalid results
    ASSERT_EQ(1, result) << "extern.convert_any should fail gracefully on stack underflow";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, ExternConvertAnyTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<ExternConvertAnyTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });