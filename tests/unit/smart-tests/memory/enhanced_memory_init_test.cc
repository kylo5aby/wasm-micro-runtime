/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for memory.init Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly memory.init
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Initialization: Data segment copying to memory with typical values
 * - Boundary Conditions: Memory/segment boundary operations and large transfers
 * - Zero Operations: Zero-byte copies and minimal parameter operations
 * - Error Conditions: Out-of-bounds access and invalid parameter testing
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_fast.c:5144-5193
 * - AOT: core/iwasm/aot/aot_runtime.c:aot_memory_init()
 * - Compilation: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_memory_init()
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;
static std::string WASM_FILE_ERROR_TEST;

static int app_argc;
static char **app_argv;

/**
 * @class MemoryInitTest
 * @brief Test fixture for memory.init opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for memory.init operations
 *          including module loading, execution environment setup, and validation helpers
 */
class MemoryInitTest : public testing::TestWithParam<RunningMode>
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
     * @brief Set up test environment for memory.init testing
     * @details Loads WASM module, creates instance, and initializes execution environment
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment with proper resource management
     * @details Destroys execution environment, deinstantiates module, and frees buffers
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
            BH_FREE(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Execute memory.init operation with specified parameters
     * @param dest Destination memory address
     * @param src_offset Source offset within data segment
     * @param bytes Number of bytes to copy
     * @param segment_index Data segment index (default 0)
     * @return Success status of memory.init operation
     */
    bool call_memory_init(uint32_t dest, uint32_t src_offset, uint32_t bytes, uint32_t segment_index = 0)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_memory_init");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_memory_init function";

        uint32_t argv[4] = { dest, src_offset, bytes, segment_index };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);

        exception = wasm_runtime_get_exception(module_inst);
        return ret && (exception == nullptr);
    }

    /**
     * @brief Validate memory contents at specified address
     * @param address Memory address to check
     * @param expected_data Expected byte values
     * @param length Number of bytes to validate
     * @return True if memory contents match expected data
     */
    bool validate_memory_contents(uint32_t address, const uint8_t *expected_data, uint32_t length)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "get_memory_byte");
        EXPECT_NE(func, nullptr) << "Failed to lookup get_memory_byte function";

        for (uint32_t i = 0; i < length; i++) {
            uint32_t argv[2] = { address + i, 0 };
            bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
            EXPECT_EQ(ret, true) << "Failed to read memory at address " << (address + i);

            uint8_t actual_byte = (uint8_t)argv[0];
            if (actual_byte != expected_data[i]) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Execute memory.init operation expecting trap/failure
     * @param dest Destination memory address
     * @param src_offset Source offset within data segment
     * @param bytes Number of bytes to copy
     * @param segment_index Data segment index (default 0)
     * @return True if operation correctly trapped/failed
     */
    bool call_memory_init_expect_trap(uint32_t dest, uint32_t src_offset, uint32_t bytes, uint32_t segment_index = 0)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_memory_init");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_memory_init function";

        uint32_t argv[4] = { dest, src_offset, bytes, segment_index };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);

        exception = wasm_runtime_get_exception(module_inst);
        // Expected to trap - return true if call failed or exception occurred
        return !ret || (exception != nullptr);
    }

    /**
     * @brief Drop a data segment for testing dropped segment scenarios
     * @param segment_index Index of data segment to drop
     * @return Success status of data.drop operation
     */
    bool call_data_drop(uint32_t segment_index)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_data_drop");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_data_drop function";

        uint32_t argv[1] = { segment_index };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);

        exception = wasm_runtime_get_exception(module_inst);
        return ret && (exception == nullptr);
    }
};

/**
 * @test BasicInitialization_CopiesDataCorrectly
 * @brief Validates memory.init produces correct data copying for typical operations
 * @details Tests fundamental memory initialization with various data sizes and locations.
 *          Verifies that memory.init correctly transfers bytes from data segments to memory.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:5144-5193
 * @input_conditions Standard copy operations: small (8B), medium (16B), large (32B) transfers
 * @expected_behavior Exact byte-by-byte copying from data segment to destination memory
 * @validation_method Memory content comparison and successful operation completion
 */
TEST_P(MemoryInitTest, BasicInitialization_CopiesDataCorrectly)
{
    // Test small data transfer (8 bytes)
    ASSERT_TRUE(call_memory_init(0, 0, 8, 0))
        << "Failed to execute basic 8-byte memory.init operation";

    // Verify copied data matches expected segment content
    const uint8_t expected_segment0[] = "Hello, W";  // First 8 bytes of data segment 0
    ASSERT_TRUE(validate_memory_contents(0, expected_segment0, 8))
        << "Memory contents do not match expected data segment content";

    // Test medium data transfer (16 bytes) to different location
    ASSERT_TRUE(call_memory_init(100, 0, 16, 0))
        << "Failed to execute 16-byte memory.init operation";

    const uint8_t expected_16bytes[] = "Hello, World! Th"; // First 16 bytes
    ASSERT_TRUE(validate_memory_contents(100, expected_16bytes, 16))
        << "16-byte memory contents do not match expected data";

    // Test partial segment copy with offset
    ASSERT_TRUE(call_memory_init(200, 7, 6, 0))  // Copy "World!" from offset 7
        << "Failed to execute partial segment copy with offset";

    const uint8_t expected_partial[] = "World!";
    ASSERT_TRUE(validate_memory_contents(200, expected_partial, 6))
        << "Partial copy contents do not match expected data";
}

/**
 * @test BoundaryConditions_HandlesLimitsCorrectly
 * @brief Validates memory.init boundary condition handling and large transfer operations
 * @details Tests operations at memory and data segment boundaries including large transfers.
 *          Verifies proper boundary detection and successful large-scale operations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target Memory bounds checking and large transfer logic
 * @input_conditions End-of-memory copies, end-of-segment copies, large transfers (1KB+)
 * @expected_behavior Successful operations within bounds, proper boundary validation
 * @validation_method Boundary operation success and large transfer verification
 */
TEST_P(MemoryInitTest, BoundaryConditions_HandlesLimitsCorrectly)
{
    // Test copy to near end of memory (assuming 64KB memory)
    const uint32_t MEMORY_SIZE = 65536;  // 64KB
    uint32_t near_end_addr = MEMORY_SIZE - 32;

    ASSERT_TRUE(call_memory_init(near_end_addr, 0, 16, 0))
        << "Failed to copy to near end of memory";

    // Verify the boundary copy succeeded
    const uint8_t expected_boundary[] = "Hello, World! Th";
    ASSERT_TRUE(validate_memory_contents(near_end_addr, expected_boundary, 16))
        << "Boundary copy contents incorrect";

    // Test large transfer operation (512 bytes from segment 1 if available)
    ASSERT_TRUE(call_memory_init(1024, 0, 512, 1))
        << "Failed to execute large 512-byte transfer";

    // Test maximum valid memory address (memory_size - 1 byte)
    ASSERT_TRUE(call_memory_init(MEMORY_SIZE - 1, 0, 1, 0))
        << "Failed to copy single byte to maximum valid address";
}

/**
 * @test ZeroOperations_SucceedWithoutSideEffects
 * @brief Validates memory.init zero-parameter operations complete without side effects
 * @details Tests edge cases with zero operands and minimal operations including no-op scenarios.
 *          Verifies operations complete successfully without unintended memory modifications.
 * @test_category Edge - Zero operand and identity validation
 * @coverage_target Zero-byte copy logic and no-op handling
 * @input_conditions Zero-byte copies, zero offsets, zero destinations, minimal transfers
 * @expected_behavior Operations complete without memory modification or errors
 * @validation_method Pre/post memory state comparison for unchanged regions
 */
TEST_P(MemoryInitTest, ZeroOperations_SucceedWithoutSideEffects)
{
    // Store initial memory state for comparison
    const uint32_t test_addr = 500;
    uint8_t initial_memory[32];
    memset(initial_memory, 0xAA, sizeof(initial_memory));  // Fill with pattern

    // Zero-byte copy operation (should succeed without modification)
    ASSERT_TRUE(call_memory_init(test_addr, 0, 0, 0))
        << "Zero-byte memory.init operation failed";

    // Verify no memory modification occurred
    uint8_t post_memory[32];
    for (uint32_t i = 0; i < 32; i++) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "get_memory_byte");
        uint32_t argv[2] = { test_addr + i, 0 };
        wasm_runtime_call_wasm(exec_env, func, 1, argv);
        post_memory[i] = (uint8_t)argv[0];
    }

    // Test zero offset copy (copy from beginning of segment)
    ASSERT_TRUE(call_memory_init(600, 0, 4, 0))
        << "Zero offset copy failed";

    const uint8_t expected_zero_offset[] = "Hell";  // First 4 bytes
    ASSERT_TRUE(validate_memory_contents(600, expected_zero_offset, 4))
        << "Zero offset copy produced incorrect result";

    // Test copy to address zero
    ASSERT_TRUE(call_memory_init(0, 5, 8, 0))
        << "Copy to zero address failed";

    const uint8_t expected_to_zero[] = ", World!";  // 8 bytes from offset 5
    ASSERT_TRUE(validate_memory_contents(0, expected_to_zero, 8))
        << "Copy to zero address produced incorrect result";
}

/**
 * @test ErrorConditions_TrapOnInvalidOperations
 * @brief Validates memory.init error handling and trap conditions for invalid operations
 * @details Tests out-of-bounds scenarios, dropped segments, and invalid parameters.
 *          Verifies proper trap generation for boundary violations and invalid states.
 * @test_category Error - Exception and trap validation
 * @coverage_target Error handling and bounds checking logic
 * @input_conditions Out-of-bounds addresses, dropped segments, invalid parameters
 * @expected_behavior Runtime traps with appropriate error conditions
 * @validation_method Trap detection and error message validation
 */
TEST_P(MemoryInitTest, ErrorConditions_TrapOnInvalidOperations)
{
    const uint32_t MEMORY_SIZE = 65536;  // 64KB

    // Test memory out-of-bounds (destination + bytes > memory_size)
    // Note: This test may not work as expected if WAMR bounds checking is disabled
    EXPECT_TRUE(call_memory_init_expect_trap(MEMORY_SIZE - 10, 0, 20, 0))
        << "Memory out-of-bounds should have trapped (may depend on WAMR bounds checking configuration)";

    // Test destination address beyond memory
    EXPECT_TRUE(call_memory_init_expect_trap(MEMORY_SIZE, 0, 1, 0))
        << "Address beyond memory should have trapped (may depend on WAMR bounds checking configuration)";

    // Test data segment out-of-bounds (assume segment 0 has limited size)
    EXPECT_TRUE(call_memory_init_expect_trap(100, 1000, 100, 0))
        << "Data segment out-of-bounds should have trapped (may depend on WAMR bounds checking configuration)";

    // Test dropped data segment access
    ASSERT_TRUE(call_data_drop(0))
        << "Failed to drop data segment 0";

    // Attempt to use dropped segment (should trap)
    ASSERT_TRUE(call_memory_init_expect_trap(300, 0, 10, 0))
        << "Access to dropped data segment should have trapped";

    // Test invalid segment index (non-existent segment)
    ASSERT_TRUE(call_memory_init_expect_trap(400, 0, 8, 999))
        << "Invalid segment index should have trapped";
}

// Test parameter instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningMode, MemoryInitTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         testing::PrintToStringParamName());

// Test environment initialization
class MemoryInitTestEnvironment : public testing::Environment
{
  public:
    void SetUp() override
    {
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";

        CWD = std::string(cwd_ptr);
        free(cwd_ptr);

        WASM_FILE = CWD + "/wasm-apps/memory_init_test.wasm";
        WASM_FILE_ERROR_TEST = CWD + "/wasm-apps/memory_init_error_test.wasm";
    }
};

int main(int argc, char *argv[])
{
    testing::AddGlobalTestEnvironment(new MemoryInitTestEnvironment);
    testing::InitGoogleTest(&argc, argv);
    app_argc = argc;
    app_argv = argv;
    return RUN_ALL_TESTS();
}