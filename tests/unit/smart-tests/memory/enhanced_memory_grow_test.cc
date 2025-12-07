/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "test_helper.h"

/**
 * @brief Enhanced unit tests for WASM memory.grow opcode
 *
 * Tests comprehensive functionality of the memory.grow instruction which attempts
 * to grow linear memory by specified pages and returns previous size or -1 on failure.
 * Validates behavior across interpreter and AOT execution modes with various memory
 * configurations including boundary conditions, failure scenarios, and edge cases.
 */

/**
 * @brief Base class for memory.grow tests with common functionality
 */
class MemoryGrowTestBase : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment with WASM module loading and runtime initialization
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Initialize members
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;
        stack_size = 65536;
        heap_size = 0;  // No additional heap to avoid affecting memory operations

        // Get current working directory for WASM file loading
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";
        cwd = std::string(cwd_ptr);
        free(cwd_ptr);

        loadWasmModule();
    }

    /**
     * @brief Clean up test environment and release all resources
     */
    void TearDown() override {
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
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM module - to be implemented by derived classes for specific memory configurations
     */
    virtual void loadWasmModule() = 0;

    /**
     * @brief Call WASM function that executes memory.grow instruction
     * @param func_name Name of the exported WASM function to call
     * @param pages_to_grow Number of pages to grow memory by
     * @return Previous memory size in pages, or -1 if growth failed
     */
    int32_t call_memory_grow_function(const char* func_name, uint32_t pages_to_grow) {
        EXPECT_NE(nullptr, module_inst) << "Module instance not initialized";
        EXPECT_NE(nullptr, exec_env) << "Execution environment not initialized";

        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Function not found: " << func_name;
        if (!func) return -1;

        uint32_t argv[1] = {pages_to_grow};
        bool call_result = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_TRUE(call_result) << "Function call failed: " << wasm_runtime_get_exception(module_inst);
        if (!call_result) return -1;

        return (int32_t)argv[0];
    }

    /**
     * @brief Call WASM function that executes memory.size instruction
     * @param func_name Name of the exported WASM function to call
     * @return Current memory size in pages
     */
    int32_t call_memory_size_function(const char* func_name) {
        EXPECT_NE(nullptr, module_inst) << "Module instance not initialized";
        EXPECT_NE(nullptr, exec_env) << "Execution environment not initialized";

        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        EXPECT_NE(nullptr, func) << "Function not found: " << func_name;
        if (!func) return -1;

        uint32_t argv[1] = {0};  // memory.size takes no arguments
        bool call_result = wasm_runtime_call_wasm(exec_env, func, 0, argv);
        EXPECT_TRUE(call_result) << "Function call failed: " << wasm_runtime_get_exception(module_inst);
        if (!call_result) return -1;

        return (int32_t)argv[0];
    }

    // Common test members
    RuntimeInitArgs init_args;
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    wasm_exec_env_t exec_env;
    uint32_t buf_size, stack_size, heap_size;
    uint8_t *buf;
    char error_buf[128];
    std::string cwd;
};

/**
 * @brief Test class for basic memory growth scenarios with reasonable limits
 */
class MemoryGrowBasicTest : public MemoryGrowTestBase {
protected:
    /**
     * @brief Load WASM module with basic memory configuration (initial 2 pages, max 10 pages)
     */
    void loadWasmModule() override {
        std::string wasm_file = cwd + "/wasm-apps/memory_grow_test.wasm";
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
};

/**
 * @brief Test class for boundary testing with limited maximum memory (initial 1 page, max 3 pages)
 */
class MemoryGrowLimitedTest : public MemoryGrowTestBase {
protected:
    /**
     * @brief Load WASM module with limited memory configuration for boundary testing
     */
    void loadWasmModule() override {
        std::string wasm_file = cwd + "/wasm-apps/memory_grow_limited_test.wasm";
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
};

// Test cases implementing the test strategy from Phase 2

/**
 * @test BasicMemoryGrowth_ReturnsCorrectPreviousSize
 * @brief Validates memory.grow returns correct previous size and successfully grows memory
 * @details Tests fundamental memory growth operation with typical growth increments.
 *          Verifies that memory.grow correctly returns the previous memory size and
 *          updates memory size appropriately for successful growth operations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_grow_operation
 * @input_conditions Initial memory 2 pages, grow by 3 pages
 * @expected_behavior Returns 2 (previous size), new memory size becomes 5 pages
 * @validation_method Direct comparison of return value with expected previous size and size validation
 */
TEST_P(MemoryGrowBasicTest, BasicMemoryGrowth_ReturnsCorrectPreviousSize) {
    // Verify initial memory state
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(2, initial_size) << "Expected 2 pages initial memory";

    // Perform memory growth operation
    int32_t grow_result = call_memory_grow_function("grow_memory", 3);
    ASSERT_EQ(2, grow_result) << "Expected previous size 2, got " << grow_result;

    // Verify new memory size after growth
    int32_t new_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(5, new_size) << "Expected 5 pages after growth, got " << new_size;
}

/**
 * @test ConsecutiveGrowth_MaintainsConsistentSizes
 * @brief Validates multiple consecutive memory.grow operations return correct previous sizes
 * @details Tests sequence of memory growth operations to ensure each operation correctly
 *          reports the previous memory size and updates memory state consistently.
 * @test_category Main - Sequential operations validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_grow_operation
 * @input_conditions Series of growth operations: grow 1, then grow 2, then grow 1
 * @expected_behavior Each operation returns correct previous size: 2, 3, 5
 * @validation_method Sequence validation of return values and memory size consistency
 */
TEST_P(MemoryGrowBasicTest, ConsecutiveGrowth_MaintainsConsistentSizes) {
    // Verify initial state
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(2, initial_size) << "Expected 2 pages initial memory";

    // First growth: 2 -> 3 pages
    int32_t grow1_result = call_memory_grow_function("grow_memory", 1);
    ASSERT_EQ(2, grow1_result) << "First growth should return previous size 2";

    // Second growth: 3 -> 5 pages
    int32_t grow2_result = call_memory_grow_function("grow_memory", 2);
    ASSERT_EQ(3, grow2_result) << "Second growth should return previous size 3";

    // Third growth: 5 -> 6 pages
    int32_t grow3_result = call_memory_grow_function("grow_memory", 1);
    ASSERT_EQ(5, grow3_result) << "Third growth should return previous size 5";

    // Verify final memory size
    int32_t final_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(6, final_size) << "Expected 6 pages final memory";
}

/**
 * @test IntegrationWithMemorySize_ConsistentResults
 * @brief Validates memory.grow integrates correctly with memory.size operations
 * @details Tests combined usage of memory.grow and memory.size to ensure consistent
 *          memory state reporting and proper integration between memory operations.
 * @test_category Main - Integration testing
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory operations
 * @input_conditions Growth operations interspersed with memory.size queries
 * @expected_behavior memory.size always reflects correct current memory size after growth
 * @validation_method Cross-validation between memory.grow results and memory.size queries
 */
TEST_P(MemoryGrowBasicTest, IntegrationWithMemorySize_ConsistentResults) {
    // Initial state validation
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(2, initial_size) << "Expected 2 pages initial memory";

    // Grow memory and validate consistency
    int32_t grow_result = call_memory_grow_function("grow_memory", 2);
    ASSERT_EQ(2, grow_result) << "Growth should return previous size 2";

    int32_t size_after_growth = call_memory_size_function("get_memory_size");
    ASSERT_EQ(4, size_after_growth) << "Memory size should be 4 after growing by 2";

    // Additional growth and validation
    int32_t second_grow_result = call_memory_grow_function("grow_memory", 1);
    ASSERT_EQ(4, second_grow_result) << "Second growth should return previous size 4";

    int32_t final_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(5, final_size) << "Final memory size should be 5 after all growth operations";
}

/**
 * @test MemoryLimitBoundary_FailsCorrectly
 * @brief Validates memory.grow fails correctly when exceeding maximum memory limits
 * @details Tests boundary condition where growth request would exceed the module's
 *          declared maximum memory limit, ensuring proper failure handling.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_grow_limits
 * @input_conditions Module max 3 pages, current 1 page, attempt grow by 3 pages (would need 4 total)
 * @expected_behavior Returns -1 (failure), memory remains unchanged at 1 page
 * @validation_method Failure return value validation and memory state preservation verification
 */
TEST_P(MemoryGrowLimitedTest, MemoryLimitBoundary_FailsCorrectly) {
    // Verify initial state of limited memory module
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(1, initial_size) << "Expected 1 page initial memory for limited module";

    // Attempt to grow beyond maximum (1 + 3 = 4 pages, but max is 3)
    int32_t grow_result = call_memory_grow_function("grow_memory", 3);
    ASSERT_EQ(-1, grow_result) << "Expected growth failure (-1) when exceeding limit";

    // Verify memory size unchanged after failed growth
    int32_t size_after_failed_growth = call_memory_size_function("get_memory_size");
    ASSERT_EQ(initial_size, size_after_failed_growth)
        << "Memory size should remain unchanged after failed growth";
}

/**
 * @test MaximumMemoryGrowth_SucceedsToLimit
 * @brief Validates memory.grow succeeds when growing exactly to maximum memory limit
 * @details Tests successful growth to the exact maximum memory limit, ensuring
 *          proper handling of boundary conditions for successful operations.
 * @test_category Corner - Maximum boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_grow_exact_limit
 * @input_conditions Module max 3 pages, current 1 page, grow by 2 pages (reaches max)
 * @expected_behavior Returns 1 (previous size), new size becomes 3 pages (maximum)
 * @validation_method Success return value validation and exact limit verification
 */
TEST_P(MemoryGrowLimitedTest, MaximumMemoryGrowth_SucceedsToLimit) {
    // Verify initial state
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(1, initial_size) << "Expected 1 page initial memory";

    // Grow to exact maximum (1 + 2 = 3 pages, which is the maximum)
    int32_t grow_result = call_memory_grow_function("grow_memory", 2);
    ASSERT_EQ(1, grow_result) << "Expected previous size 1 when growing to limit";

    // Verify we reached exactly the maximum
    int32_t size_at_limit = call_memory_size_function("get_memory_size");
    ASSERT_EQ(3, size_at_limit) << "Expected 3 pages (maximum) after growth";

    // Verify further growth fails
    int32_t further_growth = call_memory_grow_function("grow_memory", 1);
    ASSERT_EQ(-1, further_growth) << "Expected failure when already at maximum";
}

/**
 * @test ZeroGrowth_ReturnsCurrentSize
 * @brief Validates memory.grow with 0 pages returns current size without changing memory
 * @details Tests edge case where growth request is 0 pages, which should act as
 *          a memory size query operation without modifying memory state.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_grow_zero
 * @input_conditions Current memory 2 pages, grow by 0 pages
 * @expected_behavior Returns 2 (current size), memory remains 2 pages unchanged
 * @validation_method Identity operation verification and state preservation validation
 */
TEST_P(MemoryGrowBasicTest, ZeroGrowth_ReturnsCurrentSize) {
    // Verify initial state
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(2, initial_size) << "Expected 2 pages initial memory";

    // Perform zero growth (identity operation)
    int32_t zero_grow_result = call_memory_grow_function("grow_memory", 0);
    ASSERT_EQ(2, zero_grow_result) << "Zero growth should return current size 2";

    // Verify memory size unchanged
    int32_t size_after_zero_growth = call_memory_size_function("get_memory_size");
    ASSERT_EQ(initial_size, size_after_zero_growth)
        << "Memory size should remain unchanged after zero growth";

    // Perform another zero growth to ensure consistency
    int32_t second_zero_grow = call_memory_grow_function("grow_memory", 0);
    ASSERT_EQ(2, second_zero_grow) << "Second zero growth should also return current size 2";
}

/**
 * @test LargeGrowthRequest_FailsGracefully
 * @brief Validates memory.grow fails gracefully with extremely large growth requests
 * @details Tests edge case with maximum i32 value growth request to ensure proper
 *          handling of impossible growth scenarios without runtime crashes.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_grow_overflow
 * @input_conditions Grow by 0x7FFFFFFF pages (i32 maximum positive value)
 * @expected_behavior Returns -1 (failure), memory state remains unchanged
 * @validation_method Extreme value failure handling and state preservation verification
 */
TEST_P(MemoryGrowBasicTest, LargeGrowthRequest_FailsGracefully) {
    // Record initial state
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(2, initial_size) << "Expected 2 pages initial memory";

    // Attempt extremely large growth (i32 maximum)
    int32_t large_grow_result = call_memory_grow_function("grow_memory", 0x7FFFFFFF);
    ASSERT_EQ(-1, large_grow_result) << "Expected failure (-1) for extreme growth request";

    // Verify memory state preserved after failure
    int32_t size_after_large_request = call_memory_size_function("get_memory_size");
    ASSERT_EQ(initial_size, size_after_large_request)
        << "Memory size should remain unchanged after failed extreme growth";
}

/**
 * @test ExcessiveGrowth_ReturnsFailure
 * @brief Validates consistent failure handling for various impossible growth scenarios
 * @details Tests multiple scenarios where memory growth should fail, ensuring
 *          consistent -1 return value and proper memory state preservation.
 * @test_category Error - Failure scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_grow_failures
 * @input_conditions Various impossible growth scenarios beyond system/module limits
 * @expected_behavior Consistent -1 return for all failure cases, no memory state changes
 * @validation_method Comprehensive failure pattern verification and state consistency validation
 */
TEST_P(MemoryGrowLimitedTest, ExcessiveGrowth_ReturnsFailure) {
    // Record initial state
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(1, initial_size) << "Expected 1 page initial memory for limited module";

    // Test various excessive growth scenarios
    std::vector<uint32_t> excessive_values = {100, 1000, 10000, 0xFFFFFFFF};

    for (uint32_t excessive_growth : excessive_values) {
        int32_t grow_result = call_memory_grow_function("grow_memory", excessive_growth);
        ASSERT_EQ(-1, grow_result) << "Expected failure (-1) for excessive growth: " << excessive_growth;

        // Verify memory state unchanged after each failure
        int32_t current_size = call_memory_size_function("get_memory_size");
        ASSERT_EQ(initial_size, current_size)
            << "Memory size changed after failed growth by " << excessive_growth;
    }
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_CASE_P(RunningModeTest, MemoryGrowBasicTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));

INSTANTIATE_TEST_CASE_P(RunningModeTest, MemoryGrowLimitedTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));