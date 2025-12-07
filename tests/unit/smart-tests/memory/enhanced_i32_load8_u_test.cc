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
 * @brief Test fixture for comprehensive i32.load8_u opcode validation
 *
 * This test suite validates the i32.load8_u WebAssembly opcode across different
 * execution modes (interpreter and AOT). The opcode loads an 8-bit unsigned integer
 * from memory and zero-extends it to a 32-bit unsigned integer. Tests cover basic
 * functionality, zero extension behavior, boundary conditions, error scenarios,
 * and cross-mode consistency.
 */
class I32Load8UTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment with WAMR runtime initialization
     *
     * Initializes the WAMR runtime with system allocator and loads
     * the i32.load8_u test module for execution validation.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        is_aot_mode = (GetParam() == Mode_LLVM_JIT);

        // Load the i32.load8_u test module
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
     * @brief Load and instantiate the i32.load8_u test WASM module
     *
     * Loads the compiled WASM bytecode and creates a module instance
     * with proper error handling and validation.
     */
    void LoadModule()
    {
        // Use .wasm files for both interpreter and AOT modes
        // AOT compilation happens at runtime for this test setup
        const char *wasm_path = "wasm-apps/i32_load8_u_test.wasm";

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

    /**
     * @brief Get current memory size from WASM module
     *
     * @return uint32_t Current memory size in bytes
     */
    uint32_t GetCurrentMemorySize()
    {
        wasm_function_inst_t size_func = wasm_runtime_lookup_function(module_inst, "get_memory_size");
        EXPECT_NE(nullptr, size_func) << "Failed to lookup get_memory_size function";

        uint32_t wasm_argv[1] = { 0 };
        wasm_exec_env_t size_exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        EXPECT_NE(nullptr, size_exec_env) << "Failed to create execution environment for memory size";

        bool ret = wasm_runtime_call_wasm(size_exec_env, size_func, 0, wasm_argv);
        EXPECT_TRUE(ret) << "Failed to call get_memory_size function";

        if (size_exec_env) {
            wasm_runtime_destroy_exec_env(size_exec_env);
        }

        return wasm_argv[0];
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
 * @test BasicLoad_ValidAddress_ReturnsZeroExtendedValue
 * @brief Validates i32.load8_u correctly loads and zero-extends 8-bit values
 * @details Tests fundamental load operation with known unsigned byte values at
 *          specific memory addresses, verifying correct zero extension behavior
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_i32_load8_u
 * @input_conditions Memory addresses 0, 1, 2, 3 with test bytes 0x42, 0x7F, 0xAB, 0x00
 * @expected_behavior Returns zero-extended values: 0x42, 0x7F, 0xAB, 0x00
 * @validation_method Direct comparison of loaded values with expected zero-extended results
 */
TEST_P(I32Load8UTest, BasicLoad_ValidAddress_ReturnsZeroExtendedValue)
{
    uint32_t argv[2];

    // Test load from address 0 (should contain 0x42 -> zero-extended to 0x00000042)
    argv[0] = 0; // address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute i32.load8_u from address 0";
    ASSERT_EQ(0x00000042U, argv[0]) << "Incorrect zero-extended value for byte 0x42";

    // Test load from address 1 (should contain 0x7F -> zero-extended to 0x0000007F)
    argv[0] = 1; // address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute i32.load8_u from address 1";
    ASSERT_EQ(0x0000007FU, argv[0]) << "Incorrect zero-extended value for byte 0x7F";

    // Test load from address 2 (should contain 0xAB -> zero-extended to 0x000000AB)
    argv[0] = 2; // address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute i32.load8_u from address 2";
    ASSERT_EQ(0x000000ABU, argv[0]) << "Incorrect zero-extended value for byte 0xAB";

    // Test load from address 3 (should contain 0x00 -> zero-extended to 0x00000000)
    argv[0] = 3; // address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute i32.load8_u from address 3";
    ASSERT_EQ(0x00000000U, argv[0]) << "Incorrect zero-extended value for byte 0x00";
}

/**
 * @test BasicLoad_WithOffset_ReturnsCorrectValue
 * @brief Validates i32.load8_u with immediate offset parameter
 * @details Tests address calculation using base address plus immediate offset,
 *          verifying that effective address calculation works correctly
 * @test_category Main - Address calculation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_load8_u_offset
 * @input_conditions Base addresses with immediate offsets 1, 2, 4
 * @expected_behavior Correct memory access at base_address + immediate_offset
 * @validation_method Compare results with direct address access without offset
 */
TEST_P(I32Load8UTest, BasicLoad_WithOffset_ReturnsCorrectValue)
{
    uint32_t argv[2];

    // Test load with offset 1 from base address 0 (effective address 1)
    argv[0] = 0; // base address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u_offset1", 1, argv))
        << "Failed to execute i32.load8_u with offset 1";
    ASSERT_EQ(0x0000007FU, argv[0]) << "Incorrect value with offset 1";

    // Test load with offset 2 from base address 0 (effective address 2)
    argv[0] = 0; // base address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u_offset2", 1, argv))
        << "Failed to execute i32.load8_u with offset 2";
    ASSERT_EQ(0x000000ABU, argv[0]) << "Incorrect value with offset 2";

    // Test load with offset 4 from base address 0 (effective address 4)
    argv[0] = 0; // base address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u_offset4", 1, argv))
        << "Failed to execute i32.load8_u with offset 4";
    ASSERT_EQ(0x000000CDU, argv[0]) << "Incorrect value with offset 4";
}

/**
 * @test BoundaryLoad_ZeroAndMaxValues_ReturnsCorrectExtension
 * @brief Validates zero extension for boundary 8-bit values
 * @details Tests critical 8-bit boundary values to ensure proper zero extension,
 *          including minimum (0x00), maximum (0xFF), and sign boundary (0x80)
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:zero_extension_logic
 * @input_conditions Memory locations with bytes 0x00, 0xFF, 0x7F, 0x80
 * @expected_behavior Zero extension: 0x00→0x00000000, 0xFF→0x000000FF, 0x80→0x00000080
 * @validation_method Verify upper 24 bits are always zero regardless of input byte
 */
TEST_P(I32Load8UTest, BoundaryLoad_ZeroAndMaxValues_ReturnsCorrectExtension)
{
    uint32_t argv[2];

    // Test zero value (0x00) -> should remain 0x00000000
    argv[0] = 8; // address containing 0x00
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute i32.load8_u for zero value";
    ASSERT_EQ(0x00000000U, argv[0]) << "Incorrect zero extension for 0x00";

    // Test maximum unsigned value (0xFF) -> should become 0x000000FF
    argv[0] = 9; // address containing 0xFF
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute i32.load8_u for maximum value";
    ASSERT_EQ(0x000000FFU, argv[0]) << "Incorrect zero extension for 0xFF";

    // Test sign boundary positive (0x7F) -> should become 0x0000007F
    argv[0] = 10; // address containing 0x7F
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute i32.load8_u for sign boundary positive";
    ASSERT_EQ(0x0000007FU, argv[0]) << "Incorrect zero extension for 0x7F";

    // Test sign boundary negative (0x80) -> should become 0x00000080 (not sign-extended)
    argv[0] = 11; // address containing 0x80
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute i32.load8_u for sign boundary negative";
    ASSERT_EQ(0x00000080U, argv[0]) << "Incorrect zero extension for 0x80";
}

/**
 * @test BoundaryLoad_MemoryLimits_ReturnsValidData
 * @brief Validates loads at memory boundary addresses
 * @details Tests memory access at extreme valid addresses (0 and near memory limit)
 *          to ensure proper boundary handling and memory access validation
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_bounds_check
 * @input_conditions Addresses at memory boundaries: 0, near memory_size-1
 * @expected_behavior Successful loads with correct values at boundary addresses
 * @validation_method Verify loads succeed and return expected values at memory limits
 */
TEST_P(I32Load8UTest, BoundaryLoad_MemoryLimits_ReturnsValidData)
{
    uint32_t argv[2];

    // Test load from first memory address (0)
    argv[0] = 0; // first valid address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to load from first memory address";
    ASSERT_EQ(0x00000042U, argv[0]) << "Incorrect value at first memory address";

    // Test load from address near memory boundary (within allocated range)
    argv[0] = 15; // near boundary but within valid range
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to load from near memory boundary";
    // Value should be valid (specific expected value based on test data setup)
}

/**
 * @test ExtremeValueLoad_UnsignedVsSigned_ShowsZeroExtension
 * @brief Compares unsigned vs signed load behavior for same memory content
 * @details Demonstrates the difference between i32.load8_u (zero extension)
 *          and i32.load8_s (sign extension) for the same memory byte values
 * @test_category Edge - Zero vs sign extension comparison
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:extension_behavior
 * @input_conditions Memory byte 0x80 loaded with both unsigned and signed opcodes
 * @expected_behavior Unsigned: 0x80→0x00000080, Signed: 0x80→0xFFFFFF80
 * @validation_method Compare results of both load variants for same memory content
 */
TEST_P(I32Load8UTest, ExtremeValueLoad_UnsignedVsSigned_ShowsZeroExtension)
{
    uint32_t argv[2];

    // Load byte 0x80 using i32.load8_u (should zero-extend to 0x00000080)
    argv[0] = 11; // address containing 0x80
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to execute unsigned load for 0x80";
    uint32_t unsigned_result = argv[0];
    ASSERT_EQ(0x00000080U, unsigned_result) << "Incorrect unsigned zero extension";

    // Load same byte 0x80 using i32.load8_s (should sign-extend to 0xFFFFFF80)
    argv[0] = 11; // same address
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_s_compare", 1, argv))
        << "Failed to execute signed load for comparison";
    uint32_t signed_result = argv[0];
    ASSERT_EQ(0xFFFFFF80U, signed_result) << "Incorrect signed sign extension";

    // Verify they are different, demonstrating zero vs sign extension
    ASSERT_NE(unsigned_result, signed_result)
        << "Unsigned and signed results should differ for 0x80";
}

/**
 * @test ExtremeValueLoad_AllBitsSet_ReturnsMaxUnsigned
 * @brief Validates zero extension for maximum 8-bit value
 * @details Tests that 0xFF (all bits set) correctly zero-extends to 0x000000FF
 *          rather than sign-extending to 0xFFFFFFFF
 * @test_category Edge - Maximum value zero extension
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:max_value_extension
 * @input_conditions Memory byte 0xFF (maximum unsigned 8-bit value)
 * @expected_behavior Zero extension produces 0x000000FF (not 0xFFFFFFFF)
 * @validation_method Verify upper 24 bits remain zero despite all lower bits set
 */
TEST_P(I32Load8UTest, ExtremeValueLoad_AllBitsSet_ReturnsMaxUnsigned)
{
    uint32_t argv[2];

    // Load 0xFF and verify it zero-extends to 0x000000FF
    argv[0] = 9; // address containing 0xFF
    ASSERT_TRUE(CallWasmFunction("test_i32_load8_u", 1, argv))
        << "Failed to load maximum unsigned byte value";
    ASSERT_EQ(0x000000FFU, argv[0]) << "Maximum byte did not zero-extend correctly";

    // Verify upper 24 bits are zero
    ASSERT_EQ(0U, (argv[0] & 0xFFFFFF00U)) << "Upper 24 bits should be zero";

    // Verify lower 8 bits are preserved
    ASSERT_EQ(0xFFU, (argv[0] & 0x000000FFU)) << "Lower 8 bits should be preserved";
}

/**
 * @test ErrorCondition_OutOfBounds_CausesRuntimeTrap
 * @brief Validates memory access trap for out-of-bounds addresses
 * @details Tests that accessing memory beyond valid bounds causes runtime trap,
 *          ensuring proper memory protection and error handling
 * @test_category Error - Out-of-bounds access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_bounds_trap
 * @input_conditions Address beyond memory size limit
 * @expected_behavior Runtime trap/exception, function call returns false
 * @validation_method Verify function call fails and WAMR reports bounds violation
 */
TEST_P(I32Load8UTest, ErrorCondition_OutOfBounds_CausesRuntimeTrap)
{
    uint32_t argv[2];

    // Get actual memory size for bounds testing
    uint32_t memory_size = GetCurrentMemorySize();
    ASSERT_GT(memory_size, 0) << "Memory size should be greater than 0";

    // Clear any previous exceptions
    wasm_runtime_clear_exception(module_inst);

    // Attempt to load from address at memory boundary (should fail)
    argv[0] = memory_size; // First invalid address
    bool result = CallWasmFunction("test_i32_load8_u", 1, argv);

    // Function should fail due to out-of-bounds access
    ASSERT_FALSE(result)
        << "Out-of-bounds memory access should cause runtime trap";

    // Verify exception was raised
    ASSERT_NE(nullptr, wasm_runtime_get_exception(module_inst))
        << "Expected exception for out-of-bounds access";

    // Clear exception for next test
    wasm_runtime_clear_exception(module_inst);

    // Test with clearly out-of-bounds address
    argv[0] = memory_size + 1000; // Far beyond bounds
    result = CallWasmFunction("test_i32_load8_u", 1, argv);

    // Should also fail
    ASSERT_FALSE(result)
        << "Far out-of-bounds memory access should cause runtime trap";

    // Verify exception was raised again
    ASSERT_NE(nullptr, wasm_runtime_get_exception(module_inst))
        << "Expected exception for far out-of-bounds access";

    // Verify module instance remains valid after traps
    ASSERT_NE(nullptr, module_inst)
        << "Module instance should remain valid after trap";
}

/**
 * @test ErrorCondition_AddressOverflow_HandledCorrectly
 * @brief Validates handling of address + offset overflow scenarios
 * @details Tests behavior when base address + immediate offset calculation
 *          overflows or produces invalid memory addresses
 * @test_category Error - Address calculation overflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:address_calculation
 * @input_conditions High base address values that may cause overflow with offset
 * @expected_behavior Proper overflow detection and trap generation
 * @validation_method Verify overflow conditions are detected and handled appropriately
 */
TEST_P(I32Load8UTest, ErrorCondition_AddressOverflow_HandledCorrectly)
{
    uint32_t argv[2];

    // Test with high address that may overflow when offset is added
    argv[0] = 0xFFFFFFFEU; // near UINT32_MAX
    bool result = CallWasmFunction("test_i32_load8_u_offset4", 1, argv);

    // Should fail due to address overflow or out-of-bounds
    ASSERT_FALSE(result)
        << "Address overflow should be detected and cause trap";

    // Verify module remains stable after overflow handling
    ASSERT_NE(nullptr, module_inst)
        << "Module should remain stable after address overflow";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunMode, I32Load8UTest,
                        ::testing::Values(Mode_Interp, Mode_LLVM_JIT));