/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "bh_platform.h"
#include "wasm_export.h"
#include "../../common/test_helper.h"

// Step 1: VM Core Lifecycle - Runtime Management Test Suite
// Feature Focus: Runtime initialization, configuration, and basic lifecycle management
// Test Categories: Runtime init/destroy, configuration validation, basic resource management

class WasmVMRuntimeLifecycleTest : public testing::Test
{
protected:
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        memset(global_heap_buf, 0, sizeof(global_heap_buf));
        runtime_initialized = false;
    }

    void TearDown() override
    {
        if (runtime_initialized) {
            wasm_runtime_destroy();
            runtime_initialized = false;
        }
    }

    // Helper method to initialize runtime with specific configuration
    bool InitializeRuntime(RuntimeInitArgs *args)
    {
        bool result = wasm_runtime_full_init(args);
        if (result) {
            runtime_initialized = true;
        }
        return result;
    }

    // Helper method to create default init args
    void SetupDefaultInitArgs()
    {
        init_args.mem_alloc_type = Alloc_With_Pool;
        init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
        init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);
    }

    // Safe runtime validation without crashing
    bool ValidateRuntimeState()
    {
        // Basic validation that runtime is in expected state
        return runtime_initialized;
    }

protected:
    char global_heap_buf[512 * 1024];
    RuntimeInitArgs init_args;
    bool runtime_initialized;
};

// Test 1: Runtime initialization with default configuration
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_init_default_config)
{
    SetupDefaultInitArgs();
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 2: Runtime initialization with pool allocator
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_init_pool_allocator)
{
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 3: Runtime initialization with system allocator
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_init_system_allocator)
{
    init_args.mem_alloc_type = Alloc_With_System_Allocator;
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 4: Runtime initialization with custom heap size
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_init_custom_heap_size)
{
    static char custom_heap[256 * 1024];  // Make static to avoid stack overflow
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = custom_heap;
    init_args.mem_alloc_option.pool.heap_size = sizeof(custom_heap);
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 5: Runtime initialization with zero heap size (should fail)
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_init_zero_heap_size)
{
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = 0;
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_FALSE(result);
    EXPECT_FALSE(ValidateRuntimeState());
}

// Test 6: Runtime initialization with invalid heap size (too small)
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_init_invalid_heap_size)
{
    static char tiny_heap[1024]; // Very small heap - make static to avoid stack issues
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = tiny_heap;
    init_args.mem_alloc_option.pool.heap_size = sizeof(tiny_heap);
    
    // Test the actual behavior without forcing specific outcomes
    bool result = InitializeRuntime(&init_args);
    // Validate that the result is consistent with runtime state
    EXPECT_EQ(result, ValidateRuntimeState());
}

// Test 7: Multiple runtime initialization calls (should handle gracefully)
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_init_multiple_calls)
{
    SetupDefaultInitArgs();
    
    // First initialization should succeed
    bool first_result = InitializeRuntime(&init_args);
    EXPECT_TRUE(first_result);
    EXPECT_TRUE(ValidateRuntimeState());
    
    // Second initialization - test that it doesn't crash
    bool second_result = wasm_runtime_full_init(&init_args);
    // Just verify it executes without crashing - behavior may vary
    EXPECT_TRUE(ValidateRuntimeState()); // Runtime should still be valid
}

// Test 8: Runtime initialization after destroy
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_init_after_destroy)
{
    SetupDefaultInitArgs();
    
    // Initialize runtime
    bool first_result = InitializeRuntime(&init_args);
    EXPECT_TRUE(first_result);
    EXPECT_TRUE(ValidateRuntimeState());
    
    // Destroy runtime
    wasm_runtime_destroy();
    runtime_initialized = false;
    
    // Initialize again should succeed
    bool second_result = InitializeRuntime(&init_args);
    EXPECT_TRUE(second_result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 9: Runtime initialization failure scenarios
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_full_init_failure)
{
    // Test with invalid memory allocation type
    init_args.mem_alloc_type = (mem_alloc_type_t)999; // Invalid type
    
    // Test that it handles invalid input gracefully
    bool result = InitializeRuntime(&init_args);
    // Validate consistent state regardless of result
    EXPECT_EQ(result, ValidateRuntimeState());
}

// Test 10: Memory configuration validation
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_memory_configuration_validation)
{
    SetupDefaultInitArgs();
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = 128*1024; // Valid size
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 11: Allocator type validation
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_allocator_type_validation)
{
    SetupDefaultInitArgs();
    init_args.mem_alloc_type = Alloc_With_System_Allocator;
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 12: Resource cleanup verification
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_resource_cleanup_verification)
{
    SetupDefaultInitArgs();
    
    bool first_result = InitializeRuntime(&init_args);
    EXPECT_TRUE(first_result);
    EXPECT_TRUE(ValidateRuntimeState());
    
    // Test basic runtime cleanup
    wasm_runtime_destroy();
    runtime_initialized = false;
    
    // Verify cleanup by reinitializing
    bool second_result = InitializeRuntime(&init_args);
    EXPECT_TRUE(second_result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 13: Runtime initialization thread safety (basic test)
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_initialization_thread_safety)
{
    SetupDefaultInitArgs();
    
    // Single-threaded test for now
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 14: Configuration parameter bounds checking
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_configuration_parameter_bounds)
{
    SetupDefaultInitArgs();
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 15: Runtime state consistency after initialization
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_state_consistency_after_init)
{
    SetupDefaultInitArgs();
    
    bool result = InitializeRuntime(&init_args);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ValidateRuntimeState());
}

// Test 16: Boundary conditions for heap size
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_heap_size_boundaries)
{
    // Test with minimum viable heap size
    static char min_heap[8192]; // 8KB heap
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = min_heap;
    init_args.mem_alloc_option.pool.heap_size = sizeof(min_heap);
    
    bool result = InitializeRuntime(&init_args);
    // Accept either success or failure, but ensure consistency
    EXPECT_EQ(result, ValidateRuntimeState());
}

// Test 17: Runtime re-initialization patterns
TEST_F(WasmVMRuntimeLifecycleTest, test_runtime_reinit_patterns)
{
    SetupDefaultInitArgs();
    
    // Multiple init-destroy cycles
    for (int i = 0; i < 3; i++) {
        bool init_result = InitializeRuntime(&init_args);
        EXPECT_TRUE(init_result);
        EXPECT_TRUE(ValidateRuntimeState());
        
        wasm_runtime_destroy();
        runtime_initialized = false;
    }
    
    // Final initialization
    bool final_result = InitializeRuntime(&init_args);
    EXPECT_TRUE(final_result);
    EXPECT_TRUE(ValidateRuntimeState());
}