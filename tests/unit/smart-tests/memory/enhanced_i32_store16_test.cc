/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i32.store16 Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i32.store16
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Basic Store Operations: 16-bit truncated store operations with typical i32 values
 * - Truncation Behavior: Value truncation from i32 to 16-bit storage validation
 * - Boundary Value Operations: Min/max 16-bit value handling and edge cases
 * - Memory Boundary Operations: Store operations at memory boundaries and limits
 * - Out-of-bounds Access: Invalid memory access attempts and proper trap validation
 *
 * Target Coverage:
 * - Interpreter: core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_I32_STORE16
 * - Fast Interpreter: core/iwasm/interpreter/wasm_interp_fast.c:WASM_OP_I32_STORE16
 * - AOT: core/iwasm/compilation/aot_emit_memory.c:aot_compile_op_i32_store16()
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


/**
 * @class I32Store16Test
 * @brief Test fixture for i32.store16 opcode validation across execution modes
 * @details Provides comprehensive test infrastructure for i32.store16 operations
 *          including module loading, execution environment setup, and validation helpers
 */
class I32Store16Test : public testing::TestWithParam<RunningMode>
{
  protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_function_inst_t func = nullptr;

    /**
     * @brief Test fixture setup - initializes WASM module and execution environment
     * @details Loads i32.store16 test module and configures execution environment
     *          based on the parameterized running mode (interpreter/AOT)
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Get current working directory
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";
        std::string cwd = std::string(cwd_ptr);
        free(cwd_ptr);

        // Use i32.store16 specific WASM file
        std::string store16_wasm_file = cwd + "/wasm-apps/i32_store16_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(store16_wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << store16_wasm_file;

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
     * @brief Test fixture teardown - cleans up WASM execution resources
     * @details Properly destroys execution environment, module instance, and module
     *          to prevent memory leaks and ensure clean test isolation
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
     * @brief Execute i32.store16 WASM function with specified parameters
     * @details Calls the store16_value function in the loaded WASM module
     * @param address Memory address where value should be stored
     * @param value i32 value to store (will be truncated to 16 bits)
     * @return true if execution successful, false if exception occurred
     */
    bool call_store16_value(uint32_t address, uint32_t value)
    {
        func = wasm_runtime_lookup_function(module_inst, "store16_value");
        EXPECT_NE(nullptr, func) << "Failed to lookup store16_value function";
        if (!func) return false;

        uint32_t argv[2] = { address, value };
        return wasm_runtime_call_wasm(exec_env, func, 2, argv);
    }

    /**
     * @brief Execute i32.store16 WASM function with offset and parameters
     * @details Calls the store16_with_offset function in the loaded WASM module
     * @param address Base memory address
     * @param value i32 value to store (will be truncated to 16 bits)
     * @return true if execution successful, false if exception occurred
     */
    bool call_store16_with_offset(uint32_t address, uint32_t value)
    {
        func = wasm_runtime_lookup_function(module_inst, "store16_with_offset");
        EXPECT_NE(nullptr, func) << "Failed to lookup store16_with_offset function";
        if (!func) return false;

        uint32_t argv[2] = { address, value };
        return wasm_runtime_call_wasm(exec_env, func, 2, argv);
    }

    /**
     * @brief Read 16-bit value from WASM module memory at specified address
     * @details Accesses linear memory to verify stored values
     * @param address Memory address to read from
     * @return 16-bit value stored at the specified address
     */
    uint16_t read_memory_u16(uint32_t address)
    {
        uint8_t *memory_data = (uint8_t *)wasm_runtime_addr_app_to_native(module_inst, address);
        EXPECT_NE(nullptr, memory_data) << "Failed to get memory address: " << address;
        if (!memory_data) return 0;

        return *(uint16_t *)memory_data;
    }

    /**
     * @brief Get the current size of linear memory in bytes
     * @details Queries the WASM module's linear memory configuration
     * @return Size of linear memory in bytes
     */
    uint32_t get_memory_size()
    {
        uint64 start_offset = 0, end_offset = 0;
        if (wasm_runtime_get_app_addr_range(module_inst, 0, &start_offset, &end_offset)) {
            return (uint32_t)end_offset;
        }
        return 0;
    }

    /**
     * @brief Check if the last WASM execution resulted in an exception
     * @details Retrieves exception information from the execution environment
     * @return true if an exception occurred, false otherwise
     */
    bool has_exception()
    {
        const char* exception = wasm_runtime_get_exception(module_inst);
        return exception != nullptr && strlen(exception) > 0;
    }

    /**
     * @brief Get the exception message from the last WASM execution
     * @details Retrieves detailed exception information for validation
     * @return Exception message string, or empty string if no exception
     */
    std::string get_exception_message()
    {
        const char* exception = wasm_runtime_get_exception(module_inst);
        return exception ? std::string(exception) : std::string();
    }
};

/**
 * @test BasicStore_StoresCorrectValues
 * @brief Validates i32.store16 stores correct 16-bit values for typical inputs
 * @details Tests fundamental 16-bit store operation with various i32 values.
 *          Verifies that i32.store16 correctly truncates and stores values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_store16_operation
 * @input_conditions Typical i32 values: 0x1234, 0x5678, 0xABCD at addresses 0, 8, 16
 * @expected_behavior Stores lower 16 bits: 0x1234, 0x5678, 0xABCD respectively
 * @validation_method Direct memory reading and comparison with expected values
 */
TEST_P(I32Store16Test, BasicStore_StoresCorrectValues)
{
    // Test storing typical 16-bit values at various addresses
    ASSERT_TRUE(call_store16_value(0, 0x1234))
        << "Failed to store 0x1234 at address 0: " << get_exception_message();
    ASSERT_EQ(0x1234, read_memory_u16(0))
        << "Memory at address 0 should contain 0x1234";

    ASSERT_TRUE(call_store16_value(8, 0x5678))
        << "Failed to store 0x5678 at address 8: " << get_exception_message();
    ASSERT_EQ(0x5678, read_memory_u16(8))
        << "Memory at address 8 should contain 0x5678";

    ASSERT_TRUE(call_store16_value(16, 0xABCD))
        << "Failed to store 0xABCD at address 16: " << get_exception_message();
    ASSERT_EQ(0xABCD, read_memory_u16(16))
        << "Memory at address 16 should contain 0xABCD";
}

/**
 * @test TruncationBehavior_StoresOnlyLowerBits
 * @brief Validates i32.store16 properly truncates large i32 values to 16 bits
 * @details Tests truncation behavior with i32 values larger than 16 bits.
 *          Verifies that only the lower 16 bits are stored in memory.
 * @test_category Main - Value truncation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_store16_truncation
 * @input_conditions Large i32 values: 0x12345678, 0xFFFF1234, 0x80007FFF
 * @expected_behavior Only lower 16 bits stored: 0x5678, 0x1234, 0x7FFF
 * @validation_method Memory inspection to verify truncation correctness
 */
TEST_P(I32Store16Test, TruncationBehavior_StoresOnlyLowerBits)
{
    // Test truncation of large i32 values to 16-bit storage
    ASSERT_TRUE(call_store16_value(0, 0x12345678))
        << "Failed to store 0x12345678 (truncated) at address 0: " << get_exception_message();
    ASSERT_EQ(0x5678, read_memory_u16(0))
        << "Memory should contain lower 16 bits 0x5678, not full i32 value";

    ASSERT_TRUE(call_store16_value(4, 0xFFFF1234))
        << "Failed to store 0xFFFF1234 (truncated) at address 4: " << get_exception_message();
    ASSERT_EQ(0x1234, read_memory_u16(4))
        << "Memory should contain lower 16 bits 0x1234, ignoring upper 0xFFFF";

    ASSERT_TRUE(call_store16_value(8, 0x80007FFF))
        << "Failed to store 0x80007FFF (truncated) at address 8: " << get_exception_message();
    ASSERT_EQ(0x7FFF, read_memory_u16(8))
        << "Memory should contain lower 16 bits 0x7FFF, ignoring upper 0x8000";
}

/**
 * @test BoundaryValues_HandlesMinMaxCorrectly
 * @brief Validates i32.store16 handles 16-bit boundary values correctly
 * @details Tests storage of minimum and maximum 16-bit values including
 *          signed/unsigned boundaries and special values.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_store16_boundaries
 * @input_conditions Boundary values: 0x0000, 0xFFFF, 0x7FFF, 0x8000
 * @expected_behavior Exact value storage: 0x0000, 0xFFFF, 0x7FFF, 0x8000
 * @validation_method Memory comparison with expected 16-bit boundary values
 */
TEST_P(I32Store16Test, BoundaryValues_HandlesMinMaxCorrectly)
{
    // Test 16-bit boundary values
    ASSERT_TRUE(call_store16_value(0, 0x0000))
        << "Failed to store minimum 16-bit value 0x0000: " << get_exception_message();
    ASSERT_EQ(0x0000, read_memory_u16(0))
        << "Memory should contain minimum 16-bit value 0x0000";

    ASSERT_TRUE(call_store16_value(4, 0xFFFF))
        << "Failed to store maximum 16-bit value 0xFFFF: " << get_exception_message();
    ASSERT_EQ(0xFFFF, read_memory_u16(4))
        << "Memory should contain maximum 16-bit value 0xFFFF";

    ASSERT_TRUE(call_store16_value(8, 0x7FFF))
        << "Failed to store maximum signed 16-bit value 0x7FFF: " << get_exception_message();
    ASSERT_EQ(0x7FFF, read_memory_u16(8))
        << "Memory should contain maximum signed 16-bit value 0x7FFF";

    ASSERT_TRUE(call_store16_value(12, 0x8000))
        << "Failed to store minimum signed 16-bit value 0x8000: " << get_exception_message();
    ASSERT_EQ(0x8000, read_memory_u16(12))
        << "Memory should contain minimum signed 16-bit value 0x8000";
}

/**
 * @test OffsetStore_CalculatesAddressCorrectly
 * @brief Validates i32.store16 with immediate offsets calculates addresses correctly
 * @details Tests store operations with static offsets to verify proper
 *          address calculation (base_address + immediate_offset).
 * @test_category Main - Address calculation with offsets
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_store16_offset_calc
 * @input_conditions Base address with static offset, test values
 * @expected_behavior Correct storage at calculated address (base + offset)
 * @validation_method Memory verification at calculated addresses
 */
TEST_P(I32Store16Test, OffsetStore_CalculatesAddressCorrectly)
{
    // Test store operations with immediate offsets
    // store16_with_offset uses immediate offset of 4
    ASSERT_TRUE(call_store16_with_offset(0, 0x9ABC))
        << "Failed to store with offset calculation: " << get_exception_message();
    ASSERT_EQ(0x9ABC, read_memory_u16(4))  // offset 4 from base address 0
        << "Memory at offset address should contain stored value";

    ASSERT_TRUE(call_store16_with_offset(8, 0xDEF0))
        << "Failed to store with offset calculation: " << get_exception_message();
    ASSERT_EQ(0xDEF0, read_memory_u16(12))  // offset 4 from base address 8
        << "Memory at calculated offset address should contain stored value";
}

/**
 * @test MemoryBoundaries_ValidatesAccessLimits
 * @brief Validates i32.store16 respects memory boundaries and access limits
 * @details Tests store operations at memory boundaries to verify proper
 *          bounds checking and valid access patterns.
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_bounds_check
 * @input_conditions Addresses near memory limits, valid boundary addresses
 * @expected_behavior Successful storage at valid addresses within memory bounds
 * @validation_method Memory state verification at boundary addresses
 */
TEST_P(I32Store16Test, MemoryBoundaries_ValidatesAccessLimits)
{
    uint32_t memory_size = get_memory_size();
    ASSERT_GT(memory_size, 16U) << "Memory size should be sufficient for boundary tests";

    // Test store at valid boundary addresses
    uint32_t last_valid_addr = memory_size - 2;  // Last valid address for 16-bit store
    ASSERT_TRUE(call_store16_value(last_valid_addr, 0x1122))
        << "Failed to store at last valid memory address " << last_valid_addr
        << ": " << get_exception_message();
    ASSERT_EQ(0x1122, read_memory_u16(last_valid_addr))
        << "Memory at last valid address should contain stored value";

    // Test store near beginning of memory
    ASSERT_TRUE(call_store16_value(2, 0x3344))
        << "Failed to store near memory beginning: " << get_exception_message();
    ASSERT_EQ(0x3344, read_memory_u16(2))
        << "Memory near beginning should contain stored value";
}

/**
 * @test OutOfBounds_ThrowsProperException
 * @brief Validates i32.store16 throws proper exceptions for out-of-bounds access
 * @details Tests invalid memory access scenarios to verify proper exception
 *          handling and "out of bounds memory access" error reporting.
 * @test_category Error - Out-of-bounds access validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:out_of_bounds_handling
 * @input_conditions Addresses beyond memory limits, invalid access patterns
 * @expected_behavior "out of bounds memory access" exception thrown
 * @validation_method Exception type and message verification
 */
TEST_P(I32Store16Test, OutOfBounds_ThrowsProperException)
{
    uint32_t memory_size = get_memory_size();

    // Test store beyond memory limits
    uint32_t invalid_addr = memory_size;  // First invalid address
    ASSERT_FALSE(call_store16_value(invalid_addr, 0x5566))
        << "Store beyond memory limit should fail";
    ASSERT_TRUE(has_exception())
        << "Out-of-bounds store should generate exception";

    std::string exception_msg = get_exception_message();
    ASSERT_NE(exception_msg.find("out of bounds"), std::string::npos)
        << "Exception should mention 'out of bounds', got: " << exception_msg;

    // Clear exception for next test
    wasm_runtime_clear_exception(module_inst);

    // Test store that would overflow beyond memory (address + 2 > memory_size)
    if (memory_size > 0) {
        uint32_t overflow_addr = memory_size - 1;  // Would need 2 bytes but only 1 available
        ASSERT_FALSE(call_store16_value(overflow_addr, 0x7788))
            << "Store causing overflow should fail";
        ASSERT_TRUE(has_exception())
            << "Overflow store should generate exception";

        exception_msg = get_exception_message();
        ASSERT_NE(exception_msg.find("out of bounds"), std::string::npos)
            << "Overflow exception should mention 'out of bounds', got: " << exception_msg;
    }
}

/**
 * @test ZeroOperands_HandlesZeroAddressAndValue
 * @brief Validates i32.store16 handles zero address and zero value correctly
 * @details Tests edge case of storing zero value at zero address to verify
 *          proper handling of zero operands without errors.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:zero_operand_handling
 * @input_conditions Address = 0, Value = 0x00000000
 * @expected_behavior Successful storage of 0x0000 at memory address 0
 * @validation_method Memory verification at address 0 for zero value
 */
TEST_P(I32Store16Test, ZeroOperands_HandlesZeroAddressAndValue)
{
    // Test zero address and zero value storage
    ASSERT_TRUE(call_store16_value(0, 0x00000000))
        << "Failed to store zero value at zero address: " << get_exception_message();
    ASSERT_EQ(0x0000, read_memory_u16(0))
        << "Memory at address 0 should contain zero value 0x0000";

    // Verify no exception occurred with zero operands
    ASSERT_FALSE(has_exception())
        << "Zero operand storage should not generate exception: " << get_exception_message();
}

// Test parameterization for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    I32Store16Test,
    testing::Values(
#if WASM_ENABLE_INTERP != 0
        Mode_Interp
#endif
#if WASM_ENABLE_AOT != 0 && WASM_ENABLE_INTERP != 0
        ,
#endif
#if WASM_ENABLE_AOT != 0
        Mode_LLVM_JIT
#endif
    ),
    [](const testing::TestParamInfo<I32Store16Test::ParamType>& info) {
        switch (info.param) {
        case Mode_Interp:
            return "Interpreter";
        case Mode_LLVM_JIT:
            return "AOT";
        default:
            return "Unknown";
        }
    }
);

