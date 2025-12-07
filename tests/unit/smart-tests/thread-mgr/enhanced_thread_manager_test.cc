/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include "bh_platform.h"
#include "wasm_export.h"
#include "thread_manager.h"
#include "../common/test_helper.h"
#include <memory>

/**
 * Enhanced Thread Manager Test Suite for wasm_cluster_set_context coverage
 *
 * Target Function Coverage:
 * - wasm_cluster_set_context() lines 1499-1520 - 0 hits
 * - set_context_visitor() lines 1488-1496 - 0 hits (static function)
 *
 * Expected Coverage: Focus on MODULE_INST_CONTEXT functionality
 */

class EnhancedThreadManagerTest : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime first with proper configuration
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.max_thread_num = 4;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args));

        // Initialize thread manager after WAMR is ready
        ASSERT_TRUE(thread_manager_init());

        // Now create DummyExecEnv (it won't reinit WAMR since it's already initialized)
        dummy_env = std::make_unique<DummyExecEnv>();
        ASSERT_NE(dummy_env.get(), nullptr);

        exec_env = dummy_env->get();
        ASSERT_NE(exec_env, nullptr);

        module_inst = wasm_runtime_get_module_inst(exec_env);
        ASSERT_NE(module_inst, nullptr);

        // Create context key for testing
        context_key = wasm_runtime_create_context_key(context_destructor);
        ASSERT_NE(context_key, nullptr);
    }

    void TearDown() override {
        if (context_key) {
            wasm_runtime_destroy_context_key(context_key);
            context_key = nullptr;
        }

        dummy_env.reset();
        thread_manager_destroy();
        wasm_runtime_destroy();
    }

    static void context_destructor(WASMModuleInstanceCommon *inst, void *ctx) {
        // Simple destructor for test context
        if (ctx) {
            free(ctx);
        }
    }


public:
    std::unique_ptr<DummyExecEnv> dummy_env;
    WASMExecEnv *exec_env;
    WASMModuleInstanceCommon *module_inst;
    void *context_key;
};

/******
 * Test Case: wasm_cluster_set_context_NoCluster_DirectSet
 * Source: core/iwasm/libraries/thread-mgr/thread_manager.c:1499-1520
 * Target Lines: 1502 (search call), 1504-1507 (NULL exec_env path)
 * Functional Purpose: Validates that wasm_cluster_set_context() correctly handles
 *                     the case when no cluster exists (exec_env is NULL) and
 *                     falls back to direct wasm_runtime_set_context() call.
 * Call Path: wasm_cluster_set_context() <- wasm_native_set_context_spread() <- wasm_runtime_set_context_spread()
 * Coverage Goal: Exercise NULL exec_env fallback path (lines 1504-1507)
 ******/
TEST_F(EnhancedThreadManagerTest, wasm_cluster_set_context_NoCluster_DirectSet) {
    // Create a standalone module instance that we know is not part of any cluster
    char error_buf[128];
    memset(error_buf, 0, sizeof(error_buf));

    // Create a simple standalone WASM module
    wasm_module_t module = wasm_runtime_load(dummy_wasm_buffer, sizeof(dummy_wasm_buffer),
                                           error_buf, sizeof(error_buf));
    ASSERT_NE(module, nullptr);

    wasm_module_inst_t standalone_inst = wasm_runtime_instantiate(module, 32768, 32768,
                                                                error_buf, sizeof(error_buf));
    ASSERT_NE(standalone_inst, nullptr);

    // Verify this instance has no cluster (should return NULL)
    WASMExecEnv *found_env = wasm_clusters_search_exec_env((WASMModuleInstanceCommon*)standalone_inst);
    ASSERT_EQ(found_env, nullptr);  // Should be NULL since no cluster created

    // Create test context data
    void *test_context = malloc(64);
    ASSERT_NE(test_context, nullptr);
    strcpy((char*)test_context, "test_context_data");

    // Call wasm_cluster_set_context with non-clustered instance
    // This should trigger the NULL exec_env path (lines 1504-1507)
    wasm_cluster_set_context((WASMModuleInstanceCommon*)standalone_inst, context_key, test_context);

    // Verify context was set directly via wasm_runtime_set_context
    void *retrieved_context = wasm_runtime_get_context((WASMModuleInstanceCommon*)standalone_inst, context_key);
    ASSERT_EQ(retrieved_context, test_context);
    ASSERT_STREQ((char*)retrieved_context, "test_context_data");

    // Cleanup
    wasm_runtime_deinstantiate(standalone_inst);
    wasm_runtime_unload(module);
}
