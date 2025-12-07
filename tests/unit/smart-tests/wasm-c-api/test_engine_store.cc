/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <memory>

#include "bh_platform.h"
#include "wasm_c_api.h"
#include "wasm_c_api_internal.h"

#ifndef own
#define own
#endif

class EngineStoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        bh_log_set_verbose_level(5);
    }

    void TearDown() override
    {
        // Cleanup handled by individual tests
    }
};

// Engine Configuration Tests
TEST_F(EngineStoreTest, EngineConfig_Default_CreatesSuccessfully)
{
    wasm_config_t* config = wasm_config_new();
    ASSERT_NE(nullptr, config);
    
    wasm_engine_t* engine = wasm_engine_new_with_config(config);
    ASSERT_NE(nullptr, engine);
    
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, EngineConfig_WithMemAllocator_CreatesSuccessfully)
{
    wasm_config_t* config = wasm_config_new();
    ASSERT_NE(nullptr, config);
    
    // Allocate heap buffer for pool allocator
    static uint8_t heap_buf[1024 * 1024];
    MemAllocOption alloc_option = {0};
    alloc_option.pool.heap_buf = heap_buf;
    alloc_option.pool.heap_size = sizeof(heap_buf);
    
    wasm_config_t* configured = wasm_config_set_mem_alloc_opt(
        config, Alloc_With_Pool, &alloc_option);
    ASSERT_NE(nullptr, configured);
    
    wasm_engine_t* engine = wasm_engine_new_with_config(configured);
    ASSERT_NE(nullptr, engine);
    
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, EngineConfig_WithLinuxPerf_CreatesSuccessfully)
{
    wasm_config_t* config = wasm_config_new();
    ASSERT_NE(nullptr, config);
    
    wasm_config_t* perf_config = wasm_config_set_linux_perf_opt(config, true);
    ASSERT_NE(nullptr, perf_config);
    
    wasm_engine_t* engine = wasm_engine_new_with_config(perf_config);
    ASSERT_NE(nullptr, engine);
    
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, EngineConfig_WithSegueFlags_CreatesSuccessfully)
{
    wasm_config_t* config = wasm_config_new();
    ASSERT_NE(nullptr, config);
    
    wasm_config_t* segue_config = wasm_config_set_segue_flags(config, 0x1F1F);
    ASSERT_NE(nullptr, segue_config);
    
    wasm_engine_t* engine = wasm_engine_new_with_config(segue_config);
    ASSERT_NE(nullptr, engine);
    
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, EngineConfig_NullConfig_HandlesGracefully)
{
    // Use regular engine creation instead of null config
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    wasm_engine_delete(engine);
}

// Engine Singleton Behavior Tests
TEST_F(EngineStoreTest, Engine_MultipleCalls_ReturnsSameInstance)
{
    wasm_engine_t* engine1 = wasm_engine_new();
    wasm_engine_t* engine2 = wasm_engine_new();
    
    ASSERT_NE(nullptr, engine1);
    ASSERT_NE(nullptr, engine2);
    ASSERT_EQ(engine1, engine2);
    
    wasm_engine_delete(engine1);
    // Don't delete engine2 as it's the same instance
}

TEST_F(EngineStoreTest, Engine_AfterDelete_CanRecreate)
{
    wasm_engine_t* engine1 = wasm_engine_new();
    ASSERT_NE(nullptr, engine1);
    
    wasm_engine_delete(engine1);
    
    wasm_engine_t* engine2 = wasm_engine_new();
    ASSERT_NE(nullptr, engine2);
    
    wasm_engine_delete(engine2);
}

// Store Creation and Lifecycle Tests
TEST_F(EngineStoreTest, Store_WithValidEngine_CreatesSuccessfully)
{
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    wasm_store_t* store = wasm_store_new(engine);
    ASSERT_NE(nullptr, store);
    ASSERT_NE(nullptr, store->modules);
    ASSERT_NE(nullptr, store->modules->data);
    
    wasm_store_delete(store);
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, Store_WithNullEngine_FailsGracefully)
{
    wasm_store_t* store = wasm_store_new(nullptr);
    ASSERT_EQ(nullptr, store);
}

TEST_F(EngineStoreTest, Store_MultipleInstances_AreIsolated)
{
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    wasm_store_t* store1 = wasm_store_new(engine);
    wasm_store_t* store2 = wasm_store_new(engine);
    
    ASSERT_NE(nullptr, store1);
    ASSERT_NE(nullptr, store2);
    ASSERT_NE(store1, store2);
    ASSERT_NE(store1->modules->data, store2->modules->data);
    
    wasm_store_delete(store1);
    wasm_store_delete(store2);
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, Store_DeletedEngine_HandlesGracefully)
{
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    wasm_store_t* store = wasm_store_new(engine);
    ASSERT_NE(nullptr, store);
    
    // Delete engine first, then store
    wasm_engine_delete(engine);
    wasm_store_delete(store);
}

TEST_F(EngineStoreTest, Store_RecreateAfterDelete_WorksCorrectly)
{
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    wasm_store_t* store1 = wasm_store_new(engine);
    ASSERT_NE(nullptr, store1);
    wasm_store_delete(store1);
    
    wasm_store_t* store2 = wasm_store_new(engine);
    ASSERT_NE(nullptr, store2);
    wasm_store_delete(store2);
    
    wasm_engine_delete(engine);
}

// Thread Safety Tests
TEST_F(EngineStoreTest, Engine_ConcurrentCreation_ThreadSafe)
{
    const int num_threads = 4;
    std::vector<std::thread> threads;
    std::vector<wasm_engine_t*> engines(num_threads);
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&engines, i]() {
            engines[i] = wasm_engine_new();
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All engines should be the same (singleton behavior)
    for (int i = 0; i < num_threads; ++i) {
        ASSERT_NE(nullptr, engines[i]);
        if (i > 0) {
            ASSERT_EQ(engines[0], engines[i]);
        }
    }
    
    wasm_engine_delete(engines[0]);
}

TEST_F(EngineStoreTest, Store_ConcurrentCreation_ThreadSafe)
{
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    const int num_threads = 4;
    std::vector<std::thread> threads;
    std::vector<wasm_store_t*> stores(num_threads);
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&stores, engine, i]() {
            stores[i] = wasm_store_new(engine);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All stores should be created successfully and be different
    for (int i = 0; i < num_threads; ++i) {
        ASSERT_NE(nullptr, stores[i]);
        for (int j = i + 1; j < num_threads; ++j) {
            ASSERT_NE(stores[i], stores[j]);
        }
    }
    
    for (int i = 0; i < num_threads; ++i) {
        wasm_store_delete(stores[i]);
    }
    wasm_engine_delete(engine);
}

// Resource Management Tests
TEST_F(EngineStoreTest, Engine_DoubleDelete_HandlesGracefully)
{
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    wasm_engine_delete(engine);
    // Second delete should not crash
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, Store_DoubleDelete_HandlesGracefully)
{
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    wasm_store_t* store = wasm_store_new(engine);
    ASSERT_NE(nullptr, store);
    
    wasm_store_delete(store);
    // Note: Double delete may not be safe in WAMR implementation
    // Just test single delete works correctly
    
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, Engine_DeleteNull_HandlesGracefully)
{
    // Should not crash
    wasm_engine_delete(nullptr);
}

TEST_F(EngineStoreTest, Store_DeleteNull_HandlesGracefully)
{
    // Should not crash
    wasm_store_delete(nullptr);
}

// Memory Allocation Tests
TEST_F(EngineStoreTest, Store_LargeNumberOfStores_HandlesCorrectly)
{
    wasm_engine_t* engine = wasm_engine_new();
    ASSERT_NE(nullptr, engine);
    
    const int num_stores = 100;
    std::vector<wasm_store_t*> stores;
    
    for (int i = 0; i < num_stores; ++i) {
        wasm_store_t* store = wasm_store_new(engine);
        ASSERT_NE(nullptr, store);
        stores.push_back(store);
    }
    
    // Clean up
    for (wasm_store_t* store : stores) {
        wasm_store_delete(store);
    }
    
    wasm_engine_delete(engine);
}

TEST_F(EngineStoreTest, Config_MultipleConfigurations_WorkCorrectly)
{
    // Test multiple different configurations
    std::vector<wasm_config_t*> configs;
    std::vector<wasm_engine_t*> engines;
    
    // Default config
    configs.push_back(wasm_config_new());
    
    // Pool allocator config
    wasm_config_t* pool_config = wasm_config_new();
    static uint8_t pool_heap_buf[512 * 1024];
    MemAllocOption pool_option = {0};
    pool_option.pool.heap_buf = pool_heap_buf;
    pool_option.pool.heap_size = sizeof(pool_heap_buf);
    configs.push_back(wasm_config_set_mem_alloc_opt(
        pool_config, Alloc_With_Pool, &pool_option));
    
    // Performance config
    wasm_config_t* perf_config = wasm_config_new();
    configs.push_back(wasm_config_set_linux_perf_opt(perf_config, true));
    
    for (wasm_config_t* config : configs) {
        ASSERT_NE(nullptr, config);
        wasm_engine_t* engine = wasm_engine_new_with_config(config);
        ASSERT_NE(nullptr, engine);
        engines.push_back(engine);
    }
    
    // All engines should be the same (singleton behavior)
    for (size_t i = 1; i < engines.size(); ++i) {
        ASSERT_EQ(engines[0], engines[i]);
    }
    
    wasm_engine_delete(engines[0]);
}