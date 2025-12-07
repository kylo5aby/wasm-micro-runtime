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
 * @brief Enhanced unit tests for WASM memory.size opcode
 *
 * Tests comprehensive functionality of the memory.size instruction which returns
 * the current size of linear memory in pages (64KB units). Validates behavior
 * across interpreter and AOT execution modes with various memory configurations.
 */

/**
 * @brief Base class for memory.size tests with common functionality
 */
class MemorySizeTestBase : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment with WASM module loading
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
        heap_size = 0;  // No additional heap to avoid affecting memory.size

        // Get current working directory
        char *cwd_ptr = getcwd(nullptr, 0);
        ASSERT_NE(cwd_ptr, nullptr) << "Failed to get current working directory";
        cwd = std::string(cwd_ptr);
        free(cwd_ptr);

        loadWasmModule();
    }

    /**
     * @brief Clean up test environment
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
     * @brief Load WASM module - to be implemented by derived classes
     */
    virtual void loadWasmModule() = 0;

    /**
     * @brief Call WASM function that executes memory.size instruction
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

    /**
     * @brief Call WASM function that executes memory.grow then memory.size
     */
    int32_t call_memory_grow_and_size_function(const char* func_name, uint32_t pages_to_grow) {
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
 * @brief Test class for 1-page initial memory
 */
class MemorySize1PageTest : public MemorySizeTestBase {
protected:
    void loadWasmModule() override {
        std::string wasm_file = cwd + "/wasm-apps/memory_size_1page_test.wasm";
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
 * @brief Test class for 4-page initial memory
 */
class MemorySize4PageTest : public MemorySizeTestBase {
protected:
    void loadWasmModule() override {
        std::string wasm_file = cwd + "/wasm-apps/memory_size_4page_test.wasm";
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
 * @brief Test class for 0-page initial memory
 */
class MemorySize0PageTest : public MemorySizeTestBase {
protected:
    void loadWasmModule() override {
        std::string wasm_file = cwd + "/wasm-apps/memory_size_0page_test.wasm";
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
 * @brief Test class for limited memory (for testing failed growth)
 */
class MemorySizeLimitedTest : public MemorySizeTestBase {
protected:
    void loadWasmModule() override {
        std::string wasm_file = cwd + "/wasm-apps/memory_size_limited_test.wasm";
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

// Test cases

/**
 * @test InitialMemorySize_1Page_ReturnsCorrectCount
 * @brief Validates memory.size returns 1 for module with 1 page initial memory
 */
TEST_P(MemorySize1PageTest, InitialMemorySize_1Page_ReturnsCorrectCount) {
    int32_t size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(1, size) << "Expected 1 page initial memory, got " << size;
}

/**
 * @test PostGrowthMemorySize_1To5Pages_ReflectsNewSize
 * @brief Validates memory.size after growing from 1 to 5 pages
 */
TEST_P(MemorySize1PageTest, PostGrowthMemorySize_1To5Pages_ReflectsNewSize) {
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(1, initial_size) << "Expected 1 page initial memory";

    int32_t new_size = call_memory_grow_and_size_function("grow_and_get_size", 4);
    ASSERT_EQ(5, new_size) << "Expected 5 pages after growing by 4, got " << new_size;
}

/**
 * @test InitialMemorySize_4Pages_ReturnsCorrectCount
 * @brief Validates memory.size returns 4 for module with 4 pages initial memory
 */
TEST_P(MemorySize4PageTest, InitialMemorySize_4Pages_ReturnsCorrectCount) {
    int32_t size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(4, size) << "Expected 4 pages initial memory, got " << size;
}

/**
 * @test PostGrowthMemorySize_4To10Pages_ReflectsNewSize
 * @brief Validates memory.size after growing from 4 to 10 pages
 */
TEST_P(MemorySize4PageTest, PostGrowthMemorySize_4To10Pages_ReflectsNewSize) {
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(4, initial_size) << "Expected 4 pages initial memory";

    int32_t new_size = call_memory_grow_and_size_function("grow_and_get_size", 6);
    ASSERT_EQ(10, new_size) << "Expected 10 pages after growing by 6, got " << new_size;
}

/**
 * @test ConsecutiveMemorySize_ReturnsConsistent
 * @brief Validates multiple consecutive memory.size calls return identical results
 */
TEST_P(MemorySize4PageTest, ConsecutiveMemorySize_ReturnsConsistent) {
    int32_t size1 = call_memory_size_function("get_memory_size");
    int32_t size2 = call_memory_size_function("get_memory_size");
    int32_t size3 = call_memory_size_function("get_memory_size");

    ASSERT_EQ(4, size1) << "First memory.size call failed";
    ASSERT_EQ(size1, size2) << "Second memory.size call inconsistent with first";
    ASSERT_EQ(size2, size3) << "Third memory.size call inconsistent with second";
}

/**
 * @test BoundaryMemorySize_0Pages_HandlesZeroMemory
 * @brief Validates memory.size correctly handles zero initial memory
 */
TEST_P(MemorySize0PageTest, BoundaryMemorySize_0Pages_HandlesZeroMemory) {
    int32_t size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(0, size) << "Expected 0 pages initial memory, got " << size;
}

/**
 * @test MemorySizeWithFailedGrowth_RemainsUnchanged
 * @brief Validates memory.size remains unchanged after failed memory.grow operations
 */
TEST_P(MemorySizeLimitedTest, MemorySizeWithFailedGrowth_RemainsUnchanged) {
    // Get initial memory size (should be 5 for limited test module)
    int32_t initial_size = call_memory_size_function("get_memory_size");
    ASSERT_EQ(5, initial_size) << "Expected 5 pages initial memory for limited test";

    // Attempt to grow memory beyond limits (should fail)
    int32_t growth_result = call_memory_grow_and_size_function("try_grow_beyond_limit", 10000);
    ASSERT_EQ(-1, growth_result) << "Expected growth failure (-1), got " << growth_result;

    // Verify memory size unchanged after failed growth
    int32_t size_after_failed_growth = call_memory_size_function("get_memory_size");
    ASSERT_EQ(initial_size, size_after_failed_growth)
        << "Memory size changed after failed growth: expected " << initial_size
        << ", got " << size_after_failed_growth;
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_CASE_P(RunningModeTest, MemorySize1PageTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));

INSTANTIATE_TEST_CASE_P(RunningModeTest, MemorySize4PageTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));

INSTANTIATE_TEST_CASE_P(RunningModeTest, MemorySize0PageTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));

INSTANTIATE_TEST_CASE_P(RunningModeTest, MemorySizeLimitedTest,
                        testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT));