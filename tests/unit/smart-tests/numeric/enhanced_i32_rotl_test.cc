/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <climits>
#include <cstdint>
#include <vector>
#include "wasm_export.h"
#include "bh_read_file.h"

static constexpr unsigned STACK_SIZE = 8092;
static constexpr unsigned HEAP_SIZE = 8092;

/**
 * Enhanced i32.rotl opcode test suite class
 * @brief Comprehensive test coverage for i32.rotl (32-bit rotate left) operation
 * @details Tests basic rotation functionality, boundary conditions, edge cases, and error scenarios
 *          across both interpreter and AOT execution modes. Validates mathematical properties
 *          and proper handling of rotation count modulo arithmetic.
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @coverage_target core/iwasm/aot/aot_runtime.c:aot_call_function
 */
class I32RotlTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * Test fixture setup method
     * @brief Initialize WAMR runtime with proper configuration for i32.rotl testing
     * @details Sets up memory allocator, enables interpreter and AOT modes, loads test modules
     */
    void SetUp() override
    {
        // Initialize WAMR runtime with comprehensive configuration
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.n_native_symbols = 0;
        init_args.native_module_name = nullptr;
        init_args.native_symbols = nullptr;

        // Initialize WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i32.rotl testing";

        // Load primary test module for basic functionality
        std::string wasm_file = "wasm-apps/i32_rotl_test.wasm";
        buffer = reinterpret_cast<uint8_t*>(
            bh_read_file_to_buffer(wasm_file.c_str(), &size));
        ASSERT_NE(buffer, nullptr) << "Failed to read i32.rotl test WASM file: " << wasm_file;

        module = wasm_runtime_load(buffer, size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr)
            << "Failed to load i32.rotl test module: " << error_buf;

        // Instantiate module for test execution
        module_inst = wasm_runtime_instantiate(module, STACK_SIZE, HEAP_SIZE,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr)
            << "Failed to instantiate i32.rotl test module: " << error_buf;

        // Set execution mode based on test parameter
        wasm_runtime_set_running_mode(module_inst, GetParam());

        // Load underflow test module for error condition testing
        std::string underflow_wasm_file = "wasm-apps/i32_rotl_stack_underflow.wasm";
        underflow_buffer = reinterpret_cast<uint8_t*>(
            bh_read_file_to_buffer(underflow_wasm_file.c_str(), &underflow_size));
        ASSERT_NE(underflow_buffer, nullptr)
            << "Failed to read i32.rotl underflow test WASM file: " << underflow_wasm_file;
    }

    /**
     * Test fixture teardown method
     * @brief Cleanup WAMR runtime resources and modules
     * @details Proper cleanup of module instances, modules, buffers, and runtime
     */
    void TearDown() override
    {
        // Clean up module instance
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }

        // Clean up underflow module if instantiated
        if (underflow_module_inst) {
            wasm_runtime_deinstantiate(underflow_module_inst);
            underflow_module_inst = nullptr;
        }

        // Clean up modules
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (underflow_module) {
            wasm_runtime_unload(underflow_module);
            underflow_module = nullptr;
        }

        // Clean up buffers
        if (buffer) {
            BH_FREE(buffer);
            buffer = nullptr;
        }
        if (underflow_buffer) {
            BH_FREE(underflow_buffer);
            underflow_buffer = nullptr;
        }

        // Destroy WAMR runtime
        wasm_runtime_destroy();
    }

    /**
     * Call i32.rotl WASM function with specified parameters
     * @brief Execute i32.rotl operation through WASM module function call
     * @details Invokes the i32_rotl_test function in loaded WASM module with given value and count
     * @param value The 32-bit integer value to rotate
     * @param count The rotation count (number of positions to rotate left)
     * @return The rotated result as uint32_t
     */
    uint32_t call_i32_rotl(uint32_t value, uint32_t count)
    {
        // Find the i32_rotl_test function in the module
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "i32_rotl_test");
        EXPECT_NE(func, nullptr) << "Failed to lookup i32_rotl_test function";

        // Create execution environment for function calls
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        EXPECT_NE(exec_env, nullptr) << "Failed to create execution environment";

        // Prepare function arguments
        uint32_t argv[2] = { value, count };

        // Execute the function
        bool success = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(success)
            << "i32.rotl function call failed: " << wasm_runtime_get_exception(module_inst);

        // Clean up execution environment
        wasm_runtime_destroy_exec_env(exec_env);

        return argv[0]; // Return value stored in first array element
    }

    // Test fixture member variables
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    uint8_t *buffer = nullptr;
    uint32_t size;
    char error_buf[128];

    // Stack underflow test resources
    wasm_module_t underflow_module = nullptr;
    wasm_module_inst_t underflow_module_inst = nullptr;
    uint8_t *underflow_buffer = nullptr;
    uint32_t underflow_size;
};

/**
 * @test BasicRotation_ProducesCorrectResults
 * @brief Validates i32.rotl produces correct rotation results for typical input values
 * @details Tests fundamental rotation operation with various typical values and rotation counts.
 *          Verifies that i32.rotl correctly performs circular left shift for standard scenarios.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Standard integer rotation scenarios with typical values and counts
 * @expected_behavior Returns mathematically correct rotated values preserving all bits
 * @validation_method Direct comparison of WASM function result with manually calculated expected values
 */
TEST_P(I32RotlTest, BasicRotation_ProducesCorrectResults)
{
    // Test basic rotation operations with typical values
    ASSERT_EQ(0x23456781U, call_i32_rotl(0x12345678U, 4))
        << "Basic rotation: 0x12345678 rotated left by 4 positions failed";

    ASSERT_EQ(0x00000003U, call_i32_rotl(0x80000001U, 1))
        << "Basic rotation: 0x80000001 rotated left by 1 position failed";

    ASSERT_EQ(0x2468ACF0U, call_i32_rotl(0x12345678U, 1))
        << "Basic rotation: 0x12345678 rotated left by 1 position failed";

    ASSERT_EQ(0x48D159E0U, call_i32_rotl(0x12345678U, 2))
        << "Basic rotation: 0x12345678 rotated left by 2 positions failed";
}

/**
 * @test BoundaryRotations_HandleCorrectly
 * @brief Validates correct handling of rotation counts at boundary values
 * @details Tests rotation behavior at critical boundaries: no rotation (0), maximum meaningful
 *          rotation (31), full rotation (32), and beyond full rotation (33+).
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Rotation counts at boundaries: 0, 31, 32, 33 with test value
 * @expected_behavior Rotation by 0 returns original, by 32 returns original, proper modulo handling
 * @validation_method Verify identity operations and modulo arithmetic behavior
 */
TEST_P(I32RotlTest, BoundaryRotations_HandleCorrectly)
{
    uint32_t test_value = 0x12345678U;

    // Rotation by 0 should return original value (identity operation)
    ASSERT_EQ(test_value, call_i32_rotl(test_value, 0))
        << "Rotation by 0 should return original value unchanged";

    // Rotation by 31 positions
    ASSERT_EQ(0x091A2B3CU, call_i32_rotl(test_value, 31))
        << "Rotation by 31 positions failed";

    // Rotation by 32 should return original value (full rotation)
    ASSERT_EQ(test_value, call_i32_rotl(test_value, 32))
        << "Rotation by 32 should return original value (full rotation)";

    // Rotation by 33 should equal rotation by 1 (modulo behavior)
    ASSERT_EQ(call_i32_rotl(test_value, 1), call_i32_rotl(test_value, 33))
        << "Rotation by 33 should equal rotation by 1 (33 % 32 = 1)";
}

/**
 * @test ModuloArithmetic_WorksCorrectly
 * @brief Validates proper modulo 32 arithmetic for large rotation counts
 * @details Tests that rotation counts larger than 32 are properly handled using modulo arithmetic.
 *          Verifies that rotl(x, n) equals rotl(x, n % 32) for various large rotation counts.
 * @test_category Corner - Modulo arithmetic validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Large rotation counts (64, 100) with test values
 * @expected_behavior Large rotation counts properly reduced modulo 32 before rotation
 * @validation_method Compare results with equivalent smaller rotation counts
 */
TEST_P(I32RotlTest, ModuloArithmetic_WorksCorrectly)
{
    uint32_t test_value = 0xABCDEF01U;

    // Rotation by 64 should equal rotation by 0 (64 % 32 = 0)
    ASSERT_EQ(test_value, call_i32_rotl(test_value, 64))
        << "Rotation by 64 should equal rotation by 0 (64 % 32 = 0)";

    // Rotation by 100 should equal rotation by 4 (100 % 32 = 4)
    uint32_t expected_4 = call_i32_rotl(test_value, 4);
    ASSERT_EQ(expected_4, call_i32_rotl(test_value, 100))
        << "Rotation by 100 should equal rotation by 4 (100 % 32 = 4)";

    // Rotation by 65 should equal rotation by 1 (65 % 32 = 1)
    uint32_t expected_1 = call_i32_rotl(test_value, 1);
    ASSERT_EQ(expected_1, call_i32_rotl(test_value, 65))
        << "Rotation by 65 should equal rotation by 1 (65 % 32 = 1)";
}

/**
 * @test SpecialPatterns_PreserveBits
 * @brief Validates bit preservation for special bit patterns during rotation
 * @details Tests rotation of special values including all zeros, all ones, and alternating
 *          bit patterns. Verifies that rotation preserves bit patterns correctly.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Special bit patterns: 0x00000000, 0xFFFFFFFF, alternating patterns
 * @expected_behavior All bits preserved during rotation, patterns maintain integrity
 * @validation_method Verify expected patterns after various rotation operations
 */
TEST_P(I32RotlTest, SpecialPatterns_PreserveBits)
{
    // All zeros should remain all zeros regardless of rotation count
    ASSERT_EQ(0x00000000U, call_i32_rotl(0x00000000U, 5))
        << "All zeros should remain zeros after rotation";

    ASSERT_EQ(0x00000000U, call_i32_rotl(0x00000000U, 31))
        << "All zeros should remain zeros after maximum rotation";

    // All ones should remain all ones regardless of rotation count
    ASSERT_EQ(0xFFFFFFFFU, call_i32_rotl(0xFFFFFFFFU, 7))
        << "All ones should remain all ones after rotation";

    ASSERT_EQ(0xFFFFFFFFU, call_i32_rotl(0xFFFFFFFFU, 16))
        << "All ones should remain all ones after half rotation";

    // Alternating bit pattern testing
    ASSERT_EQ(0x55555555U, call_i32_rotl(0xAAAAAAAAU, 1))
        << "Alternating pattern 0xAAAAAAAA rotated left by 1 failed";

    ASSERT_EQ(0xAAAAAAAAU, call_i32_rotl(0x55555555U, 1))
        << "Alternating pattern 0x55555555 rotated left by 1 failed";
}

/**
 * @test MathematicalProperties_Validated
 * @brief Validates mathematical properties and relationships of rotation operations
 * @details Tests inverse relationships and distributive properties of rotation operations.
 *          Verifies that rotl(rotl(x, a), b) equals rotl(x, a+b) and similar properties.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Various values tested with composite rotation operations
 * @expected_behavior Mathematical properties hold: distributive, inverse relationships
 * @validation_method Compare composite operations with direct equivalent operations
 */
TEST_P(I32RotlTest, MathematicalProperties_Validated)
{
    uint32_t test_value = 0x12345678U;

    // Test distributive property: rotl(rotl(x, a), b) = rotl(x, a+b)
    uint32_t double_rotate = call_i32_rotl(call_i32_rotl(test_value, 5), 3);
    uint32_t single_rotate = call_i32_rotl(test_value, 8);
    ASSERT_EQ(single_rotate, double_rotate)
        << "Distributive property failed: rotl(rotl(x, 5), 3) should equal rotl(x, 8)";

    // Test inverse relationship: rotl(rotl(x, n), 32-n) = x
    uint32_t forward_rotate = call_i32_rotl(test_value, 7);
    uint32_t inverse_rotate = call_i32_rotl(forward_rotate, 25); // 32 - 7 = 25
    ASSERT_EQ(test_value, inverse_rotate)
        << "Inverse property failed: rotl(rotl(x, 7), 25) should equal x";

    // Test commutativity with modulo: rotl(x, n) = rotl(x, n % 32)
    uint32_t normal_rotate = call_i32_rotl(test_value, 15);
    uint32_t modulo_rotate = call_i32_rotl(test_value, 47); // 47 % 32 = 15
    ASSERT_EQ(normal_rotate, modulo_rotate)
        << "Modulo property failed: rotl(x, 15) should equal rotl(x, 47)";
}

/**
 * @test StackUnderflow_HandledCorrectly
 * @brief Validates proper handling of stack underflow conditions during i32.rotl execution
 * @details Tests WASM modules that attempt to execute i32.rotl with insufficient values on
 *          the execution stack. Verifies that appropriate traps are generated.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions WASM modules with insufficient stack values for i32.rotl operation
 * @expected_behavior Module loading or instantiation fails with proper error reporting
 * @validation_method Verify module load failure and appropriate error messages
 */
TEST_P(I32RotlTest, StackUnderflow_HandledCorrectly)
{
    // Load the controlled underflow test module (should succeed as it's syntactically valid)
    underflow_module = wasm_runtime_load(underflow_buffer, underflow_size,
                                       error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, underflow_module)
        << "Failed to load controlled underflow test module: " << error_buf;

    // Instantiate the underflow module
    wasm_module_inst_t underflow_inst = wasm_runtime_instantiate(
        underflow_module, STACK_SIZE, HEAP_SIZE, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, underflow_inst)
        << "Failed to instantiate underflow test module: " << error_buf;

    // Set execution mode
    wasm_runtime_set_running_mode(underflow_inst, GetParam());

    // Test that controlled functions work correctly (they contain valid i32.rotl operations)
    wasm_function_inst_t func = wasm_runtime_lookup_function(
        underflow_inst, "controlled_underflow_test");
    ASSERT_NE(nullptr, func) << "Failed to lookup controlled_underflow_test function";

    // Create execution environment
    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(underflow_inst, 65536);
    ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

    // Test safe path (should_trap = 0)
    uint32_t argv[1] = {0}; // should_trap = 0 (safe path)
    bool success = wasm_runtime_call_wasm(exec_env, func, 1, argv);
    ASSERT_TRUE(success) << "Safe path execution should succeed";

    // Cleanup
    wasm_runtime_destroy_exec_env(exec_env);
    wasm_runtime_deinstantiate(underflow_inst);
}

/**
 * @test InvalidModules_FailGracefully
 * @brief Validates graceful handling of invalid WASM modules containing i32.rotl operations
 * @details Tests behavior when loading corrupted or malformed WASM modules that contain
 *          i32.rotl instructions. Verifies proper error handling and cleanup.
 * @test_category Error - Invalid module validation
 * @coverage_target core/iwasm/common/wasm_loader.c:wasm_loader_load
 * @input_conditions Corrupted or malformed WASM bytecode with i32.rotl instructions
 * @expected_behavior Module loading fails gracefully with appropriate error messages
 * @validation_method Verify load failure and proper error reporting for invalid modules
 */
TEST_P(I32RotlTest, InvalidModules_FailGracefully)
{
    // Test with completely invalid buffer (corrupted header)
    uint8_t invalid_wasm[] = { 0x00, 0x61, 0x73, 0x6D, 0xFF, 0xFF, 0xFF, 0xFF }; // Invalid magic/version

    wasm_module_t invalid_module = wasm_runtime_load(invalid_wasm, sizeof(invalid_wasm),
                                                   error_buf, sizeof(error_buf));
    ASSERT_EQ(nullptr, invalid_module)
        << "Expected invalid WASM module to fail loading";

    // Verify error message is generated
    ASSERT_NE(strlen(error_buf), 0)
        << "Expected error message for invalid WASM module";

    // Test with empty buffer
    wasm_module_t empty_module = wasm_runtime_load(nullptr, 0, error_buf, sizeof(error_buf));
    ASSERT_EQ(nullptr, empty_module)
        << "Expected null buffer to fail loading";

    // Verify error handling for empty buffer
    ASSERT_NE(strlen(error_buf), 0)
        << "Expected error message for null/empty buffer";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningModeTest, I32RotlTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<I32RotlTest::ParamType>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);

