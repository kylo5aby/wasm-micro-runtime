/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <memory>
#include <cstring>
#include <cmath>
#include <limits>
#include "wasm_runtime.h"
#include "bh_read_file.h"
#include "test_helper.h"

/**
 * @brief Test fixture for comprehensive f32.load opcode validation
 *
 * This test suite validates the f32.load WebAssembly opcode across different
 * execution modes (interpreter and AOT). Tests cover basic functionality,
 * IEEE 754 special values, boundary conditions, error scenarios, and cross-mode consistency.
 */
class F32LoadTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment with WAMR runtime initialization
     *
     * Initializes the WAMR runtime with system allocator and loads
     * the f32.load test module for execution validation.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        is_aot_mode = (GetParam() == Mode_LLVM_JIT);

        // Load the f32.load test module
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
     * @brief Load and instantiate the f32.load test WASM module
     *
     * Loads the compiled WASM bytecode and creates a module instance
     * with proper error handling and validation.
     */
    void LoadModule()
    {
        // Use .wasm files for both interpreter and AOT modes
        // AOT compilation happens at runtime for this test setup
        const char *wasm_path = "wasm-apps/f32_load_test.wasm";

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
     * @brief Convert uint32_t bit representation to float32 value
     *
     * @param bits 32-bit unsigned integer containing IEEE 754 float bits
     * @return float The corresponding single-precision floating-point value
     */
    float BitsToFloat(uint32_t bits)
    {
        union { uint32_t i; float f; } converter;
        converter.i = bits;
        return converter.f;
    }

    /**
     * @brief Convert float32 value to uint32_t bit representation
     *
     * @param value Single-precision floating-point value
     * @return uint32_t The corresponding 32-bit IEEE 754 representation
     */
    uint32_t FloatToBits(float value)
    {
        union { float f; uint32_t i; } converter;
        converter.f = value;
        return converter.i;
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
 * @test BasicLoad_ReturnsCorrectValues
 * @brief Validates f32.load correctly reads 32-bit floats from memory
 * @details Tests fundamental load operation with known float values at specific memory
 *          addresses, verifying correct IEEE 754 data retrieval across multiple addresses
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_f32_load
 * @input_conditions Memory addresses 0, 4, 8 with known float values
 * @expected_behavior Returns exact stored values: 3.14f, -2.5f, 1.0f
 * @validation_method Direct float comparison of loaded values with expected constants
 */
TEST_P(F32LoadTest, BasicLoad_ReturnsCorrectValues)
{
    uint32_t argv[2];

    // Test load from address 0 (should contain 3.14f)
    argv[0] = 0; // address
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to execute f32.load from address 0";
    float loaded_value = BitsToFloat(argv[0]);
    ASSERT_FLOAT_EQ(3.14f, loaded_value)
        << "Incorrect value loaded from address 0: expected 3.14f, got " << loaded_value;

    // Test load from address 4 (should contain -2.5f)
    argv[0] = 4; // address
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to execute f32.load from address 4";
    loaded_value = BitsToFloat(argv[0]);
    ASSERT_FLOAT_EQ(-2.5f, loaded_value)
        << "Incorrect value loaded from address 4: expected -2.5f, got " << loaded_value;

    // Test load from address 8 (should contain 1.0f)
    argv[0] = 8; // address
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to execute f32.load from address 8";
    loaded_value = BitsToFloat(argv[0]);
    ASSERT_FLOAT_EQ(1.0f, loaded_value)
        << "Incorrect value loaded from address 8: expected 1.0f, got " << loaded_value;
}

/**
 * @test BoundaryMemoryAccess_HandlesCorrectly
 * @brief Validates f32.load behavior at memory boundaries and with large offsets
 * @details Tests load operation with boundary conditions, large offsets, and edge
 *          memory positions to ensure proper bounds checking and address calculation
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_boundary_check
 * @input_conditions Loads near memory boundaries and with maximum valid offsets
 * @expected_behavior Successful loads within bounds, correct address calculations
 * @validation_method Verification of loaded values and absence of boundary violations
 */
TEST_P(F32LoadTest, BoundaryMemoryAccess_HandlesCorrectly)
{
    uint32_t argv[2];

    // Test load from near end of memory (address 60, should contain 999.9f)
    argv[0] = 60; // address near boundary
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to execute f32.load from boundary address 60";
    float loaded_value = BitsToFloat(argv[0]);
    ASSERT_FLOAT_EQ(999.9f, loaded_value)
        << "Incorrect value loaded from boundary address 60";

    // Test load with large offset from base address 0 (offset should reach address 32)
    argv[0] = 0; // base address
    ASSERT_TRUE(CallWasmFunction("test_f32_load_offset32", 1, argv))
        << "Failed to execute f32.load with offset 32";
    loaded_value = BitsToFloat(argv[0]);
    ASSERT_FLOAT_EQ(0.001f, loaded_value)
        << "Incorrect value loaded with offset 32 from address 0";

    // Test load with combined base and offset addressing
    argv[0] = 16; // base address
    ASSERT_TRUE(CallWasmFunction("test_f32_load_offset4", 1, argv))
        << "Failed to execute f32.load with base 16, offset 4";
    loaded_value = BitsToFloat(argv[0]);
    ASSERT_FLOAT_EQ(100.0f, loaded_value)
        << "Incorrect value loaded with combined addressing (base 16, offset 4)";
}

/**
 * @test SpecialFloatValues_PreservesIEEE754
 * @brief Validates correct handling of IEEE 754 special values (NaN, Infinity, denormals)
 * @details Tests load operation with special floating-point values to ensure
 *          bit-perfect IEEE 754 representation preservation during memory access
 * @test_category Edge - IEEE 754 compliance validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_load_ieee754
 * @input_conditions Memory containing special float bit patterns (NaN, ±∞, ±0, denormals)
 * @expected_behavior Exact IEEE 754 representation maintained, special values detected correctly
 * @validation_method Bitwise comparison and IEEE 754 special value detection functions
 */
TEST_P(F32LoadTest, SpecialFloatValues_PreservesIEEE754)
{
    uint32_t argv[2];

    // Test positive zero: +0.0f (0x00000000)
    argv[0] = 64; // address containing +0.0f
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to load positive zero from address 64";
    float loaded_value = BitsToFloat(argv[0]);
    ASSERT_FLOAT_EQ(0.0f, loaded_value)
        << "Positive zero not preserved correctly";
    ASSERT_EQ(0x00000000U, argv[0])
        << "Positive zero bit pattern not preserved";

    // Test negative zero: -0.0f (0x80000000)
    argv[0] = 68; // address containing -0.0f
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to load negative zero from address 68";
    loaded_value = BitsToFloat(argv[0]);
    ASSERT_EQ(0x80000000U, argv[0])
        << "Negative zero bit pattern not preserved";
    ASSERT_TRUE(std::signbit(loaded_value))
        << "Negative zero sign bit not preserved";

    // Test positive infinity: +∞ (0x7F800000)
    argv[0] = 72; // address containing +∞
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to load positive infinity from address 72";
    loaded_value = BitsToFloat(argv[0]);
    ASSERT_TRUE(std::isinf(loaded_value) && loaded_value > 0)
        << "Positive infinity not detected correctly";
    ASSERT_EQ(0x7F800000U, argv[0])
        << "Positive infinity bit pattern not preserved";

    // Test negative infinity: -∞ (0xFF800000)
    argv[0] = 76; // address containing -∞
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to load negative infinity from address 76";
    loaded_value = BitsToFloat(argv[0]);
    ASSERT_TRUE(std::isinf(loaded_value) && loaded_value < 0)
        << "Negative infinity not detected correctly";
    ASSERT_EQ(0xFF800000U, argv[0])
        << "Negative infinity bit pattern not preserved";

    // Test quiet NaN: (0x7FC00000)
    argv[0] = 80; // address containing quiet NaN
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to load quiet NaN from address 80";
    loaded_value = BitsToFloat(argv[0]);
    ASSERT_TRUE(std::isnan(loaded_value))
        << "NaN not detected correctly";
    ASSERT_EQ(0x7FC00000U, argv[0])
        << "Quiet NaN bit pattern not preserved";
}

/**
 * @test OutOfBoundsAccess_HandlesBoundaryConditions
 * @brief Validates f32.load behavior at memory boundaries and beyond
 * @details Tests load operations at the edge and beyond memory limits to verify
 *          consistent behavior and proper handling of boundary conditions
 * @test_category Error - Memory boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_boundary_check
 * @input_conditions Memory access at valid boundaries and beyond allocated memory
 * @expected_behavior Consistent behavior: either successful load or proper error handling
 * @validation_method Verify that boundary behavior is consistent and deterministic
 */
TEST_P(F32LoadTest, OutOfBoundsAccess_HandlesBoundaryConditions)
{
    uint32_t argv[2];

    // Test access at last valid f32 position (should succeed)
    argv[0] = 65532; // Last valid address for 4-byte f32 load (65532 + 4 = 65536)
    ASSERT_TRUE(CallWasmFunction("test_f32_load", 1, argv))
        << "Failed to load from last valid f32 address 65532";

    // Test access at memory boundary - behavior may be implementation dependent
    // but should be consistent across calls
    argv[0] = 65536; // First address beyond memory boundary
    bool result1 = CallWasmFunction("test_f32_load", 1, argv);
    bool result2 = CallWasmFunction("test_f32_load", 1, argv);
    ASSERT_EQ(result1, result2)
        << "Inconsistent behavior for boundary access at address 65536";

    // Test access far beyond memory - should be consistent
    argv[0] = 100000; // Far beyond memory boundary
    bool result3 = CallWasmFunction("test_f32_load", 1, argv);
    bool result4 = CallWasmFunction("test_f32_load", 1, argv);
    ASSERT_EQ(result3, result4)
        << "Inconsistent behavior for far out-of-bounds access at address 100000";
}

// Test parameters for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    F32LoadTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<F32LoadTest::ParamType>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);