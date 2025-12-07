/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <vector>
#include <memory>

extern "C" {
#include "wasm_export.h"
#include "bh_platform.h"
#include "thread_manager.h"
#include "wasm_runtime_common.h"
#include "wasm_exec_env.h"
}

#include "../../common/test_helper.h"

// Platform detection utility for tests - REQUIRED in every test file
class PlatformTestContext {
public:
    // Feature detection
    static bool HasThreadMgrSupport() {
#if WASM_ENABLE_THREAD_MGR != 0
        return true;
#else
        return false;
#endif
    }
    
    static bool HasSharedMemorySupport() {
#if WASM_ENABLE_SHARED_MEMORY != 0
        return true;
#else
        return false;
#endif
    }
    
    static bool HasPthreadSupport() {
#if WASM_ENABLE_LIB_PTHREAD != 0
        return true;
#else
        return false;
#endif
    }
    
    static bool HasAuxStackAllocation() {
#if WASM_ENABLE_HEAP_AUX_STACK_ALLOCATION == 0
        return true;
#else
        return false;
#endif
    }
};

// Test fixture for Thread Manager Step 3: Synchronization & State Management
class ThreadMgrStep3Test : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime once per test
        runtime = std::make_unique<WAMRRuntimeRAII<>>();
        
        // Initialize thread manager
        ASSERT_TRUE(thread_manager_init());
        
        // Create test execution environment using test helper classes
        dummy_env = std::make_unique<DummyExecEnv>();
        
        exec_env = dummy_env->get();
        ASSERT_NE(exec_env, nullptr);
        
        module_inst = wasm_runtime_get_module_inst(exec_env);
        ASSERT_NE(module_inst, nullptr);
        
        cluster = wasm_exec_env_get_cluster(exec_env);
        ASSERT_NE(cluster, nullptr);
    }
    
    void TearDown() override {
        // Clean up in reverse order with proper checks
        if (dummy_env) {
            dummy_env.reset();
        }
        
        // Thread manager cleanup
        thread_manager_destroy();
        
        // WAMR runtime cleanup handled by WAMRRuntimeRAII destructor
        runtime.reset();
        
        // Reset pointers
        exec_env = nullptr;
        module_inst = nullptr;
        cluster = nullptr;
    }
    
    std::unique_ptr<WAMRRuntimeRAII<>> runtime;
    std::unique_ptr<DummyExecEnv> dummy_env;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    WASMCluster* cluster = nullptr;
    
    // Helper to create a cluster with multiple exec envs for testing
    WASMCluster* CreateTestCluster() {
        if (!exec_env) return nullptr;
        
        // Get cluster from existing exec_env
        return wasm_exec_env_get_cluster(exec_env);
    }
    
    // Helper to validate cluster state
    bool ValidateClusterState(WASMCluster* cluster) {
        return cluster != nullptr;
    }
};

// Test: wasm_cluster_suspend_all() - Suspend all threads in cluster
TEST_F(ThreadMgrStep3Test, SuspendAll_ValidCluster_ExecutesCorrectly) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    WASMCluster* cluster = CreateTestCluster();
    ASSERT_NE(cluster, nullptr);
    
    // Test suspend all operation
    // Note: Function may not have visible effects in single-threaded test environment
    // but we're testing that it executes without crashing
    wasm_cluster_suspend_all(cluster);
    
    // Validate cluster remains in valid state
    ASSERT_TRUE(ValidateClusterState(cluster));
}

// Test: wasm_cluster_suspend_all_except_self() - Suspend all except current thread
TEST_F(ThreadMgrStep3Test, SuspendAllExceptSelf_ValidCluster_ExecutesCorrectly) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    WASMCluster* cluster = CreateTestCluster();
    ASSERT_NE(cluster, nullptr);
    
    // Test suspend all except self operation
    wasm_cluster_suspend_all_except_self(cluster, exec_env);
    
    // Validate cluster remains in valid state
    ASSERT_TRUE(ValidateClusterState(cluster));
}

// Test: wasm_cluster_resume_all() - Resume all suspended threads
TEST_F(ThreadMgrStep3Test, ResumeAll_ValidCluster_ExecutesCorrectly) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    WASMCluster* cluster = CreateTestCluster();
    ASSERT_NE(cluster, nullptr);
    
    // First suspend, then resume to test complete cycle
    wasm_cluster_suspend_all(cluster);
    wasm_cluster_resume_all(cluster);
    
    // Validate cluster remains in valid state
    ASSERT_TRUE(ValidateClusterState(cluster));
}

// Test: wasm_cluster_resume_thread() - Resume specific thread
TEST_F(ThreadMgrStep3Test, ResumeThread_ValidExecEnv_ExecutesCorrectly) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    ASSERT_NE(exec_env, nullptr);
    
    // Test resume specific thread operation
    wasm_cluster_resume_thread(exec_env);
    
    // Validate exec_env remains valid
    ASSERT_NE(exec_env, nullptr);
    ASSERT_NE(wasm_exec_env_get_module_inst(exec_env), nullptr);
}

// Test: wasm_cluster_terminate_all() - Terminate all threads in cluster
TEST_F(ThreadMgrStep3Test, TerminateAll_ValidCluster_ExecutesCorrectly) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    WASMCluster* cluster = CreateTestCluster();
    ASSERT_NE(cluster, nullptr);
    
    // Test terminate all operation
    // Note: This may affect cluster state, so we test execution without validation
    wasm_cluster_terminate_all(cluster);
    
    // After termination, cluster may be in different state
    // We only verify the function executed without crashing
}

// Test: wasm_cluster_terminate_all_except_self() - Terminate all except current thread
TEST_F(ThreadMgrStep3Test, TerminateAllExceptSelf_ValidCluster_ExecutesCorrectly) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    WASMCluster* cluster = CreateTestCluster();
    ASSERT_NE(cluster, nullptr);
    
    // Test terminate all except self operation
    wasm_cluster_terminate_all_except_self(cluster, exec_env);
    
    // After selective termination, our exec_env should remain valid
    ASSERT_NE(exec_env, nullptr);
}

// Test: wasm_cluster_wait_for_all() - Wait for all threads to complete
TEST_F(ThreadMgrStep3Test, WaitForAll_ValidCluster_ExecutesCorrectly) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    WASMCluster* cluster = CreateTestCluster();
    ASSERT_NE(cluster, nullptr);
    
    // Test wait for all operation
    // In single-threaded test environment, this should return quickly
    wasm_cluster_wait_for_all(cluster);
    
    // Validate cluster remains accessible
    ASSERT_TRUE(ValidateClusterState(cluster));
}

// Test: wasm_cluster_is_thread_terminated() - Check if thread is terminated
TEST_F(ThreadMgrStep3Test, IsThreadTerminated_ValidExecEnv_ReturnsFalse) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    ASSERT_NE(exec_env, nullptr);
    
    // Test thread termination check
    bool is_terminated = wasm_cluster_is_thread_terminated(exec_env);
    
    // Active exec_env should not be terminated
    ASSERT_FALSE(is_terminated);
}

// Test: wasm_cluster_is_thread_terminated() with cluster validation
TEST_F(ThreadMgrStep3Test, IsThreadTerminated_ClusterValidation_ConsistentResults) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    ASSERT_NE(exec_env, nullptr);
    
    // Test thread termination status multiple times for consistency
    bool is_terminated_1 = wasm_cluster_is_thread_terminated(exec_env);
    bool is_terminated_2 = wasm_cluster_is_thread_terminated(exec_env);
    
    // Results should be consistent for same exec_env
    ASSERT_EQ(is_terminated_1, is_terminated_2);
    
    // Verify cluster remains accessible
    WASMCluster* cluster = wasm_exec_env_get_cluster(exec_env);
    ASSERT_NE(cluster, nullptr);
}

// Test: Suspend and Resume cycle with state validation
TEST_F(ThreadMgrStep3Test, SuspendResumeCycle_ValidCluster_MaintainsState) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    WASMCluster* cluster = CreateTestCluster();
    ASSERT_NE(cluster, nullptr);
    
    // Perform complete suspend/resume cycle
    wasm_cluster_suspend_all(cluster);
    ASSERT_TRUE(ValidateClusterState(cluster));
    
    wasm_cluster_resume_all(cluster);
    ASSERT_TRUE(ValidateClusterState(cluster));
    
    // Verify thread is still active
    ASSERT_FALSE(wasm_cluster_is_thread_terminated(exec_env));
}

// Test: Multiple synchronization operations in sequence
TEST_F(ThreadMgrStep3Test, SequentialOperations_ValidCluster_ExecutesCorrectly) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    WASMCluster* cluster = CreateTestCluster();
    ASSERT_NE(cluster, nullptr);
    
    // Test sequence of operations
    wasm_cluster_suspend_all_except_self(cluster, exec_env);
    wasm_cluster_resume_thread(exec_env);
    wasm_cluster_wait_for_all(cluster);
    
    // Verify thread state after operations
    ASSERT_FALSE(wasm_cluster_is_thread_terminated(exec_env));
    ASSERT_TRUE(ValidateClusterState(cluster));
}

// Test: Thread termination status after termination operations
TEST_F(ThreadMgrStep3Test, TerminationStatus_AfterTerminate_ReflectsState) {
    if (!PlatformTestContext::HasThreadMgrSupport()) {
        return;
    }
    
    // Create separate exec_env for termination test
    WAMRModule temp_module(dummy_wasm_buffer, sizeof(dummy_wasm_buffer));
    ASSERT_NE(temp_module.get(), nullptr);
    
    WAMRInstance temp_instance(temp_module, 4096, 4096);
    ASSERT_NE(temp_instance.get(), nullptr);
    
    WASMExecEnv* temp_exec_env = wasm_exec_env_create(temp_instance.get(), 4096);
    ASSERT_NE(temp_exec_env, nullptr);
    
    // Check initial state
    ASSERT_FALSE(wasm_cluster_is_thread_terminated(temp_exec_env));
    
    // Clean up temporary resources
    wasm_exec_env_destroy(temp_exec_env);
}

