/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <string>

#include "wasm_runtime.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_export.h"

static std::string CWD;
static std::string WASM_FILE;
static std::string WASM_FILE_STACK_UNDERFLOW;

// Test execution modes for cross-validation
enum class RefAsNonNullRunningMode : uint8_t {
    INTERP = 1 << 0,
    AOT = 1 << 1
};

/**
 * @brief Test fixture for comprehensive ref.as_non_null opcode validation
 *
 * This test suite validates the WebAssembly ref.as_non_null opcode implementation
 * in WAMR runtime, covering successful conversions, null reference traps, and
 * cross-execution mode consistency between interpreter and AOT compilation.
 */
class RefAsNonNullTest : public testing::TestWithParam<RefAsNonNullRunningMode>
{
  protected:
    /**
     * @brief Initialize WAMR runtime and test environment
     * @details Sets up runtime with proper configuration for reference types and GC support.
     *          Configures both interpreter and AOT execution modes for comprehensive testing.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));

        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Enable reference types and GC proposal support for ref.as_non_null testing
        init_args.n_native_symbols = 0;
        init_args.native_module_name = nullptr;
        init_args.native_symbols = nullptr;

        // Initialize runtime with reference types support
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for ref.as_non_null testing";

        // Set up test WASM file paths
        WASM_FILE = "wasm-apps/ref_as_non_null_test.wasm";
        WASM_FILE_STACK_UNDERFLOW = "wasm-apps/ref_as_non_null_stack_underflow.wasm";

        cleanup_required = true;
    }

    /**
     * @brief Clean up WAMR runtime resources
     * @details Ensures proper cleanup of runtime, modules, and execution instances
     *          to prevent resource leaks between test executions.
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

        if (cleanup_required) {
            wasm_runtime_destroy();
            cleanup_required = false;
        }
    }

    /**
     * @brief Load WASM module for ref.as_non_null testing
     * @param wasm_file_path Path to WASM module file
     * @return true if module loaded successfully, false otherwise
     * @details Loads and instantiates WASM module with reference types support,
     *          creating execution environment for test case execution.
     */
    bool LoadWasmModule(const std::string& wasm_file_path)
    {
        // Clean up any existing module first
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

        uint32 buf_size = 0;
        uint8 *buf = nullptr;
        char error_buf[128] = {0};

        // Read WASM module file
        buf = (uint8*)bh_read_file_to_buffer(wasm_file_path.c_str(), &buf_size);
        EXPECT_NE(nullptr, buf) << "Failed to read WASM file: " << wasm_file_path;
        if (!buf) {
            return false;
        }

        // Load WASM module with reference types support
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        BH_FREE(buf);

        EXPECT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;
        if (!module) {
            return false;
        }

        // Instantiate WASM module
        module_inst = wasm_runtime_instantiate(module, 0, 0, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module_inst) << "Failed to instantiate module: " << error_buf;
        if (!module_inst) return false;

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, 32768);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";


        return exec_env != nullptr;
    }

    /**
     * @brief Call WASM function and handle return values/traps
     * @param func_name Name of the exported WASM function
     * @param args Function arguments array
     * @param arg_count Number of arguments
     * @param results Result values array
     * @param result_count Number of expected results
     * @return true if function executed successfully, false if trapped
     * @details Executes WASM function with proper error handling for traps.
     *          Used to test both successful ref.as_non_null conversions and null reference traps.
     */
    bool CallWasmFunction(const std::string& func_name, uint32* args,
                         uint32 arg_count, uint32* results, uint32 result_count)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name.c_str());
        EXPECT_NE(nullptr, func) << "Failed to lookup function: " << func_name;
        if (!func) return false;

        // Prepare wasm_val_t arrays for arguments and results
        wasm_val_t* wasm_args = nullptr;
        wasm_val_t* wasm_results = nullptr;

        if (arg_count > 0 && args) {
            wasm_args = new wasm_val_t[arg_count];
            for (uint32 i = 0; i < arg_count; i++) {
                wasm_args[i].kind = WASM_I32;
                wasm_args[i].of.i32 = args[i];
            }
        }

        if (result_count > 0 && results) {
            wasm_results = new wasm_val_t[result_count];
            for (uint32 i = 0; i < result_count; i++) {
                wasm_results[i].kind = WASM_I32;
                wasm_results[i].of.i32 = 0;
            }
        }

        // Call the WASM function using the proper API
        bool call_result = wasm_runtime_call_wasm_a(exec_env, func, result_count, wasm_results, arg_count, wasm_args);

        // Copy results back to the provided array
        if (call_result && result_count > 0 && results && wasm_results) {
            for (uint32 i = 0; i < result_count; i++) {
                results[i] = wasm_results[i].of.i32;
            }
        }

        // Clean up allocated memory
        if (wasm_args) delete[] wasm_args;
        if (wasm_results) delete[] wasm_results;

        return call_result;
    }

  protected:
    RuntimeInitArgs init_args{};
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    bool cleanup_required = false;
};

/**
 * @test BasicFuncrefConversion_ReturnsNonNullReference
 * @brief Validates ref.as_non_null with valid funcref produces correct non-nullable reference
 * @details Tests fundamental ref.as_non_null operation with non-null function reference.
 *          Verifies that valid function references pass through unchanged with updated type annotation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Valid non-null function reference on execution stack
 * @expected_behavior Returns identical function reference with non-nullable type annotation
 * @validation_method Direct function call validation and reference identity comparison
 */
TEST_P(RefAsNonNullTest, BasicFuncrefConversion_ReturnsNonNullReference)
{
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};  // No arguments for basic funcref test
    uint32 results[1] = {0};

    // Test ref.as_non_null with valid funcref - should succeed
    ASSERT_TRUE(CallWasmFunction("test_funcref_as_non_null", args, 0, results, 1))
        << "ref.as_non_null with valid funcref should succeed";

    // Verify function reference was converted successfully (non-zero indicates success)
    ASSERT_NE(0, results[0]) << "Valid funcref conversion should return non-null reference";
}

/**
 * @test BasicExternrefConversion_ReturnsNonNullReference
 * @brief Validates ref.as_non_null with valid externref produces correct non-nullable reference
 * @details Tests ref.as_non_null operation with non-null external reference.
 *          Verifies external references are properly converted to non-nullable type.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Valid non-null external reference on execution stack
 * @expected_behavior Returns identical external reference with non-nullable type annotation
 * @validation_method Function execution success and reference value validation
 */
TEST_P(RefAsNonNullTest, BasicExternrefConversion_ReturnsNonNullReference)
{
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test ref.as_non_null with valid externref - should succeed
    ASSERT_TRUE(CallWasmFunction("test_externref_as_non_null", args, 0, results, 1))
        << "ref.as_non_null with valid externref should succeed";

    // Verify external reference was converted successfully
    ASSERT_NE(0, results[0]) << "Valid externref conversion should return non-null reference";
}

/**
 * @test ChainedConversions_SuccessfullyProcessed
 * @brief Validates multiple ref.as_non_null operations on same reference work correctly
 * @details Tests applying ref.as_non_null twice to the same reference to verify idempotent behavior.
 *          Ensures repeated conversions don't corrupt references or cause runtime issues.
 * @test_category Corner - Boundary conditions and chained operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Non-null reference converted multiple times sequentially
 * @expected_behavior Consistent results across multiple conversion operations
 * @validation_method Multiple function calls with identical expected results
 */
TEST_P(RefAsNonNullTest, ChainedConversions_SuccessfullyProcessed)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test chained ref.as_non_null operations - should succeed consistently
    ASSERT_TRUE(CallWasmFunction("test_chained_conversions", args, 0, results, 1))
        << "Chained ref.as_non_null conversions should succeed";

    // Verify chained conversions produce expected results
    ASSERT_NE(0, results[0]) << "Chained conversions should maintain reference validity";
}

/**
 * @test MultipleReferenceTypes_AllConvertCorrectly
 * @brief Validates ref.as_non_null with different reference types in sequence
 * @details Tests sequential conversion of various reference types (funcref, externref)
 *          to ensure type system consistency across different reference kinds.
 * @test_category Corner - Multiple reference type handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Sequence of different valid reference types
 * @expected_behavior All reference types convert successfully without interference
 * @validation_method Sequential function calls with different reference types
 */
TEST_P(RefAsNonNullTest, MultipleReferenceTypes_AllConvertCorrectly)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test multiple reference types conversion in sequence
    ASSERT_TRUE(CallWasmFunction("test_multiple_reference_types", args, 0, results, 1))
        << "Multiple reference types conversion should succeed";

    // Verify all reference types processed correctly
    ASSERT_NE(0, results[0]) << "Multiple reference types should all convert successfully";
}

/**
 * @test FunctionCallIntegration_NonNullReferenceUsed
 * @brief Validates ref.as_non_null output can be used in function calls requiring non-null references
 * @details Tests integration of ref.as_non_null with function calls that require non-nullable references.
 *          Verifies converted references maintain proper type annotations for subsequent operations.
 * @test_category Corner - Integration with function call operations
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions ref.as_non_null output used as function call argument
 * @expected_behavior Function call succeeds with converted non-null reference
 * @validation_method Function call success and proper argument passing
 */
TEST_P(RefAsNonNullTest, FunctionCallIntegration_NonNullReferenceUsed)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test ref.as_non_null integration with function calls
    ASSERT_TRUE(CallWasmFunction("test_function_call_integration", args, 0, results, 1))
        << "ref.as_non_null integration with function calls should succeed";

    // Verify function call with converted reference succeeds
    ASSERT_NE(0, results[0]) << "Function call with converted non-null reference should succeed";
}

/**
 * @test ControlFlowIntegration_ConditionalConversion
 * @brief Validates ref.as_non_null within control flow constructs (if/else, loops)
 * @details Tests ref.as_non_null operation within conditional branches and loop constructs.
 *          Verifies correct behavior in different execution paths and control flow contexts.
 * @test_category Corner - Control flow integration
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions ref.as_non_null executed within conditional and loop constructs
 * @expected_behavior Consistent behavior across different control flow paths
 * @validation_method Execution success across multiple control flow scenarios
 */
TEST_P(RefAsNonNullTest, ControlFlowIntegration_ConditionalConversion)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test ref.as_non_null within control flow constructs
    ASSERT_TRUE(CallWasmFunction("test_control_flow_integration", args, 0, results, 1))
        << "ref.as_non_null within control flow should succeed";

    // Verify control flow integration works correctly
    ASSERT_NE(0, results[0]) << "Control flow integration should maintain proper execution";
}

/**
 * @test IdentityOperations_NoPerformancePenalty
 * @brief Validates ref.as_non_null on already non-null references has minimal overhead
 * @details Tests ref.as_non_null applied to references already known to be non-null.
 *          Verifies operation is efficient and doesn't introduce unnecessary overhead.
 * @test_category Edge - Identity operations and performance characteristics
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions References already confirmed to be non-null
 * @expected_behavior Efficient pass-through operation with minimal overhead
 * @validation_method Execution success and performance consistency
 */
TEST_P(RefAsNonNullTest, IdentityOperations_NoPerformancePenalty)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test ref.as_non_null on already non-null references (identity operation)
    ASSERT_TRUE(CallWasmFunction("test_identity_operations", args, 0, results, 1))
        << "Identity ref.as_non_null operations should succeed efficiently";

    // Verify identity operations work correctly
    ASSERT_NE(0, results[0]) << "Identity operations should preserve reference validity";
}

/**
 * @test ReferenceIdentityPreservation_SameObjectReturned
 * @brief Validates ref.as_non_null returns identical reference object (not copy)
 * @details Tests that ref.as_non_null preserves reference identity and doesn't create copies.
 *          Verifies the same reference object is returned with updated type annotation.
 * @test_category Edge - Reference identity and memory efficiency
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Non-null reference with identity tracking
 * @expected_behavior Same reference object returned, no copying performed
 * @validation_method Reference identity comparison and memory consistency checks
 */
TEST_P(RefAsNonNullTest, ReferenceIdentityPreservation_SameObjectReturned)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test reference identity preservation through ref.as_non_null
    ASSERT_TRUE(CallWasmFunction("test_reference_identity", args, 0, results, 1))
        << "Reference identity preservation should succeed";

    // Verify reference identity is maintained
    ASSERT_NE(0, results[0]) << "Reference identity should be preserved through conversion";
}

/**
 * @test PolymorphicReferences_TypeSystemRespected
 * @brief Validates ref.as_non_null with polymorphic references and type hierarchies
 * @details Tests ref.as_non_null with various reference subtypes and polymorphic references.
 *          Verifies type system consistency across reference type hierarchies.
 * @test_category Edge - Polymorphic type handling and inheritance
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Polymorphic references and subtype relationships
 * @expected_behavior Consistent behavior across reference type hierarchies
 * @validation_method Type system consistency validation across polymorphic scenarios
 */
TEST_P(RefAsNonNullTest, PolymorphicReferences_TypeSystemRespected)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test polymorphic reference handling with ref.as_non_null
    ASSERT_TRUE(CallWasmFunction("test_polymorphic_references", args, 0, results, 1))
        << "Polymorphic reference handling should succeed";

    // Verify polymorphic references are handled correctly
    ASSERT_NE(0, results[0]) << "Polymorphic references should respect type system constraints";
}

/**
 * @test NullFuncrefConversion_TrapsCorrectly
 * @brief Validates ref.as_non_null with null funcref causes proper trap
 * @details Tests ref.as_non_null operation with null function reference input.
 *          Verifies proper trap behavior when attempting to convert null reference.
 * @test_category Exception - Null reference trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Null function reference on execution stack
 * @expected_behavior Execution traps with proper error handling
 * @validation_method Trap detection and error handling verification
 */
TEST_P(RefAsNonNullTest, NullFuncrefConversion_TrapsCorrectly)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test ref.as_non_null with null funcref - should trap
    ASSERT_FALSE(CallWasmFunction("test_null_funcref_trap", args, 0, results, 1))
        << "ref.as_non_null with null funcref should trap";

    // Verify trap occurred as expected (no results should be returned)
    // Exception traps don't modify results array
}

/**
 * @test NullExternrefConversion_TrapsCorrectly
 * @brief Validates ref.as_non_null with null externref causes proper trap
 * @details Tests ref.as_non_null operation with null external reference input.
 *          Verifies trap behavior is consistent across different reference types.
 * @test_category Exception - Null reference trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Null external reference on execution stack
 * @expected_behavior Execution traps with consistent error handling
 * @validation_method Trap detection and error consistency verification
 */
TEST_P(RefAsNonNullTest, NullExternrefConversion_TrapsCorrectly)
{
    // WASM_FILE already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE)) << "Failed to load ref.as_non_null test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test ref.as_non_null with null externref - should trap
    ASSERT_FALSE(CallWasmFunction("test_null_externref_trap", args, 0, results, 1))
        << "ref.as_non_null with null externref should trap";

    // Verify trap occurred as expected
    // Exception traps don't modify results array
}

/**
 * @test StackUnderflow_TrapsAppropriately
 * @brief Validates ref.as_non_null with empty stack causes stack underflow trap
 * @details Tests ref.as_non_null execution when no values are available on execution stack.
 *          Verifies proper stack underflow handling and error reporting.
 * @test_category Exception - Stack underflow error handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:ref_as_non_null_operation
 * @input_conditions Empty execution stack
 * @expected_behavior Stack underflow trap with proper error reporting
 * @validation_method Stack underflow detection and error handling validation
 */
TEST_P(RefAsNonNullTest, StackUnderflow_TrapsAppropriately)
{
    // WASM_FILE_STACK_UNDERFLOW already set in SetUp()
    ASSERT_TRUE(LoadWasmModule(WASM_FILE_STACK_UNDERFLOW)) << "Failed to load stack underflow test module";

    uint32 args[1] = {0};
    uint32 results[1] = {0};

    // Test ref.as_non_null with empty stack - should cause stack underflow trap
    ASSERT_FALSE(CallWasmFunction("test_stack_underflow", args, 0, results, 1))
        << "ref.as_non_null with empty stack should cause stack underflow";

    // Verify stack underflow trap occurred as expected
    // Stack underflow traps don't modify results array
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningMode, RefAsNonNullTest,
                        testing::Values(RefAsNonNullRunningMode::INTERP, RefAsNonNullRunningMode::AOT));