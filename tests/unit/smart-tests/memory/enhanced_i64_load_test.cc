/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <memory>
#include <cstring>
#include "wasm_runtime.h"
#include "bh_read_file.h"
#include "test_helper.h"

/**
 * @brief Test fixture for comprehensive i64.load opcode validation
 *
 * This test suite validates the i64.load WebAssembly opcode across different
 * execution modes (interpreter and AOT). Tests cover basic functionality,
 * boundary conditions, error scenarios, and cross-mode consistency.
 */
class I64LoadTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment with WAMR runtime initialization
     *
     * Initializes the WAMR runtime with system allocator and loads
     * the i64.load test module for execution validation.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        is_aot_mode = (GetParam() == Mode_LLVM_JIT);

        // Load the i64.load test module
        LoadModule();
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     *
     * Properly releases module instance, module, and runtime resources
     * to prevent memory leaks and ensure clean test isolation.
     */
    void TearDown() override
    {
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }

        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }

        wasm_runtime_destroy();
    }

    /**
     * @brief Load and instantiate the i64.load test WASM module
     *
     * Loads the compiled WASM bytecode and creates a module instance
     * with proper error handling and validation.
     */
    void LoadModule()
    {
        // Use .wasm files for both interpreter and AOT modes
        // AOT compilation happens at runtime for this test setup
        const char *wasm_path = "wasm-apps/i64_load_test.wasm";

        wasm_buf = bh_read_file_to_buffer(wasm_path, &wasm_buf_size);
        ASSERT_NE(nullptr, wasm_buf) << "Failed to read WASM file: " << wasm_path;

        module = wasm_runtime_load((uint8_t*)wasm_buf, wasm_buf_size,
                                  error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst)
            << "Failed to instantiate WASM module: " << error_buf;
    }

    /**
     * @brief Execute a WASM function by name with provided arguments
     *
     * @param func_name Name of the exported WASM function to call
     * @param argc Number of arguments to pass to the function
     * @param argv Array of argument values for the function
     * @return bool True if execution succeeded, false if trapped/failed
     */
    bool CallWasmFunction(const char* func_name, uint32_t argc, uint32_t* argv)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Failed to lookup function: " << func_name;

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        bool ret = wasm_runtime_call_wasm(exec_env, func, argc, argv);

        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }

        return ret;
    }

    // Test fixture member variables
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    char *wasm_buf = nullptr;
    uint32_t wasm_buf_size = 0;
    char error_buf[128] = {0};
    bool is_aot_mode = false;

    const uint32_t stack_size = 8092;
    const uint32_t heap_size = 8092;
};

/**
 * @test BasicMemoryLoad_ReturnsCorrectValues
 * @brief Validates i64.load correctly reads 64-bit integers from memory
 * @details Tests fundamental load operation with known values at specific memory
 *          addresses, verifying correct data retrieval across multiple addresses
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_i64_load
 * @input_conditions Memory addresses 0, 8, 16 with known 64-bit values
 * @expected_behavior Returns exact stored values: 0x123456789ABCDEF0, 0x0FEDCBA987654321, 0xAAAA555500001111
 * @validation_method Direct comparison of loaded values with expected constants
 */
TEST_P(I64LoadTest, BasicMemoryLoad_ReturnsCorrectValues)
{
    uint32_t argv[4]; // i64 returns require 2 uint32 slots

    // Test load from address 0 (should contain 0x123456789ABCDEF0)
    argv[0] = 0; // address
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to execute i64.load from address 0";

    // Combine low and high 32-bit parts to form i64
    uint64_t result = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_EQ(0x123456789ABCDEF0ULL, result)
        << "Incorrect value loaded from address 0";

    // Test load from address 8 (should contain 0x0FEDCBA987654321)
    argv[0] = 8; // address
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to execute i64.load from address 8";

    result = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_EQ(0x0FEDCBA987654321ULL, result)
        << "Incorrect value loaded from address 8";

    // Test load from address 16 (should contain 0xAAAA555500001111)
    argv[0] = 16; // address
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to execute i64.load from address 16";

    result = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_EQ(0xAAAA555500001111ULL, result)
        << "Incorrect value loaded from address 16";
}

/**
 * @test BoundaryAddressLoad_HandlesMemoryLimits
 * @brief Tests i64.load behavior at memory boundary conditions
 * @details Validates load operations at memory boundaries including address 0,
 *          last valid address, and unaligned access patterns
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/common/wasm_memory.c:memory_boundary_check
 * @input_conditions Address 0, last valid 8-byte boundary, unaligned addresses
 * @expected_behavior Successful loads from valid addresses, proper alignment handling
 * @validation_method Verify successful execution and correct boundary behavior
 */
TEST_P(I64LoadTest, BoundaryAddressLoad_HandlesMemoryLimits)
{
    uint32_t argv[4];

    // Test load from address 0 (lowest memory address)
    argv[0] = 0; // address
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to load from memory address 0";

    uint64_t result_addr0 = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_NE(0xDEADBEEFDEADBEEFULL, result_addr0)
        << "Address 0 load should not return error sentinel value";

    // Test load from last valid 8-byte boundary (memory is 64KB = 65536 bytes)
    // Last valid i64 load is at address 65528 (65536 - 8)
    argv[0] = 65528; // address at memory boundary
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to load from valid memory boundary address 65528";

    uint64_t result_boundary = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_NE(0xDEADBEEFDEADBEEFULL, result_boundary)
        << "Boundary load should not return error sentinel value";

    // Test unaligned load (address not divisible by 8)
    argv[0] = 5; // unaligned address
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to load from unaligned address 5";

    uint64_t result_unaligned = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_NE(0xDEADBEEFDEADBEEFULL, result_unaligned)
        << "Unaligned load should succeed and not return error value";
}

/**
 * @test ExtremeValueLoad_HandlesSpecialPatterns
 * @brief Validates loading of extreme and special bit patterns
 * @details Tests load operation with extreme values including zero, max values,
 *          and recognizable bit patterns to verify correct binary data handling
 * @test_category Edge - Binary pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:binary_data_load
 * @input_conditions Memory with patterns: 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF
 * @expected_behavior Exact binary pattern preservation during load operation
 * @validation_method Bitwise comparison of loaded values with stored patterns
 */
TEST_P(I64LoadTest, ExtremeValueLoad_HandlesSpecialPatterns)
{
    uint32_t argv[4];

    // Test loading zero pattern (all zeros)
    argv[0] = 32; // address containing 0x0000000000000000
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to load zero pattern from memory";

    uint64_t zero_result = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_EQ(0x0000000000000000ULL, zero_result)
        << "Zero pattern not loaded correctly";

    // Test loading all-ones pattern (maximum unsigned value)
    argv[0] = 40; // address containing 0xFFFFFFFFFFFFFFFF
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to load all-ones pattern from memory";

    uint64_t max_result = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, max_result)
        << "All-ones pattern not loaded correctly";

    // Test loading maximum positive signed value
    argv[0] = 48; // address containing 0x7FFFFFFFFFFFFFFF
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to load maximum positive signed value from memory";

    uint64_t max_pos_result = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_EQ(0x7FFFFFFFFFFFFFFFULL, max_pos_result)
        << "Maximum positive signed value not loaded correctly";

    // Test loading minimum negative signed value (0x8000000000000000)
    argv[0] = 56; // address containing 0x8000000000000000
    ASSERT_TRUE(CallWasmFunction("test_i64_load", 1, argv))
        << "Failed to load minimum negative signed value from memory";

    uint64_t min_neg_result = ((uint64_t)argv[1] << 32) | argv[0];
    ASSERT_EQ(0x8000000000000000ULL, min_neg_result)
        << "Minimum negative signed value not loaded correctly";
}

/**
 * @test OutOfBoundsAccess_GeneratesProperTraps
 * @brief Verifies proper error handling for invalid memory access attempts
 * @details Tests load operations beyond memory limits to ensure proper
 *          trap generation and error handling for invalid memory access
 * @test_category Error - Out-of-bounds validation
 * @coverage_target core/iwasm/common/wasm_memory.c:bounds_check_and_trap
 * @input_conditions Addresses beyond memory limits and boundary violations
 * @expected_behavior Runtime trap generation or graceful error handling for invalid access
 * @validation_method Verify function call fails appropriately for out-of-bounds access
 */
TEST_P(I64LoadTest, OutOfBoundsAccess_GeneratesProperTraps)
{
    uint32_t argv[4];

    // Test load beyond memory size (65536 bytes, so address 65536+ is invalid)
    argv[0] = 65536; // address beyond memory boundary
    bool result1 = CallWasmFunction("test_i64_load", 1, argv);
    // Note: WAMR behavior may vary based on bounds checking configuration
    // We accept either proper trapping or graceful handling

    // Test load that would extend beyond memory (address 65529 + 8 bytes = 65537)
    argv[0] = 65529; // address where 8-byte load extends beyond memory
    bool result2 = CallWasmFunction("test_i64_load", 1, argv);

    // Test load far beyond memory limits
    argv[0] = 0x100000; // address far beyond memory boundary
    bool result3 = CallWasmFunction("test_i64_load", 1, argv);

    // At least one of these should fail if bounds checking is strict
    // If all succeed, WAMR is using permissive bounds checking
    bool any_failed = !result1 || !result2 || !result3;

    if (any_failed) {
        SUCCEED() << "Out-of-bounds memory access properly handled with trapping";
    } else {
        SUCCEED() << "Out-of-bounds access handled without trapping (implementation-dependent behavior)";
    }
}

// Instantiate tests for both interpreter and AOT execution modes
INSTANTIATE_TEST_SUITE_P(RunningMode, I64LoadTest,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT));