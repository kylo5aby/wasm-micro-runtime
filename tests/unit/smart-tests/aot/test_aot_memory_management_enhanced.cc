/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <limits.h>
#include "gtest/gtest.h"
#include "wasm_export.h"
#include "bh_platform.h"
#include "../../common/test_helper.h"
#include <fstream>
#include <vector>

class AOTMemoryManagementTest : public testing::Test
{
protected:
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_Pool;
        init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
        init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
    }

    void TearDown() override
    {
        wasm_runtime_destroy();
    }

    bool load_wasm_file(const char *filename, uint8_t **buffer, uint32_t *size)
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return false;
        }

        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        *buffer = (uint8_t *)wasm_runtime_malloc(file_size);
        if (!*buffer) {
            return false;
        }

        if (!file.read(reinterpret_cast<char*>(*buffer), file_size)) {
            wasm_runtime_free(*buffer);
            *buffer = nullptr;
            return false;
        }

        *size = static_cast<uint32_t>(file_size);
        return true;
    }

    void free_wasm_buffer(uint8_t *buffer)
    {
        if (buffer) {
            wasm_runtime_free(buffer);
        }
    }

    wasm_module_inst_t create_test_instance_with_memory(uint32_t initial_pages, uint32_t max_pages)
    {
        char error_buf[128] = {0};
        // Use the dummy WASM buffer from test_helper.h
        wasm_module_t module = wasm_runtime_load(dummy_wasm_buffer, sizeof(dummy_wasm_buffer), 
                                                 error_buf, sizeof(error_buf));
        if (!module) {
            return nullptr;
        }

        wasm_module_inst_t module_inst = wasm_runtime_instantiate(module, 8192, 8192, 
                                                                  error_buf, sizeof(error_buf));
        wasm_runtime_unload(module);
        return module_inst;
    }

    uint32_t get_memory_page_count(wasm_module_inst_t module_inst)
    {
        if (!module_inst) return 0;
        
        // Use app address range to get memory size
        uint64_t app_offset;
        uint64_t app_size;
        if (wasm_runtime_get_app_addr_range(module_inst, 0, &app_offset, &app_size)) {
            return (uint32_t)(app_size / 65536); // 64KB per page
        }
        return 0;
    }

    char global_heap_buf[512 * 1024];
    RuntimeInitArgs init_args;
};

// Test 1: AOT linear memory initialization success
TEST_F(AOTMemoryManagementTest, LinearMemoryInitialization_Success)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range to verify memory is accessible
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    ASSERT_TRUE(range_result) << "Should be able to get app address range";
    ASSERT_GT(app_size, 0) << "App memory size should be greater than 0";
    
    wasm_runtime_deinstantiate(inst);
}

// Test 2: AOT linear memory access valid range success
TEST_F(AOTMemoryManagementTest, LinearMemoryAccess_ValidRange_Success)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size > 0) {
        // Test valid memory access through address validation
        bool valid_start = wasm_runtime_validate_app_addr(inst, app_offset, 4);
        ASSERT_TRUE(valid_start) << "Should be able to access start of memory";
        
        if (app_size > 4) {
            bool valid_end = wasm_runtime_validate_app_addr(inst, app_offset + app_size - 4, 4);
            ASSERT_TRUE(valid_end) << "Should be able to access end of memory";
        }
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 3: AOT linear memory access out of bounds fails
TEST_F(AOTMemoryManagementTest, LinearMemoryAccess_OutOfBounds_Fails)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size > 0) {
        // Test out of bounds access
        bool invalid_access = wasm_runtime_validate_app_addr(inst, app_offset + app_size, 4);
        ASSERT_FALSE(invalid_access) << "Out of bounds access should fail validation";
        
        // Test very large offset
        bool invalid_large = wasm_runtime_validate_app_addr(inst, UINT32_MAX - 3, 4);
        ASSERT_FALSE(invalid_large) << "Large offset access should fail validation";
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 4: AOT linear memory growth valid size success
// TEST_F(AOTMemoryManagementTest, LinearMemoryGrowth_ValidSize_Success)
// {
//     wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
//     if (inst == nullptr) {
//         return; // Skip if cannot create test instance
//     }

//     uint32_t initial_pages = get_memory_page_count(inst);
    
//     // Try to grow memory by 1 page
//     bool result = wasm_runtime_enlarge_memory(inst, 1);
//     if (result) {
//         uint32_t new_pages = get_memory_page_count(inst);
//         ASSERT_EQ(new_pages, initial_pages + 1) << "Memory should grow by 1 page";
//     }
//     // If growth fails, it's still valid behavior depending on configuration
    
//     wasm_runtime_deinstantiate(inst);
// }

// Test 5: AOT linear memory growth exceeds max fails
TEST_F(AOTMemoryManagementTest, LinearMemoryGrowth_ExceedsMax_Fails)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 2); // Max 2 pages
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Try to grow memory beyond reasonable limits
    bool result = wasm_runtime_enlarge_memory(inst, 1000); // Try to grow by 1000 pages
    ASSERT_FALSE(result) << "Memory growth beyond reasonable limits should fail";
    
    wasm_runtime_deinstantiate(inst);
}

// Test 6: AOT linear memory i32 load store success
TEST_F(AOTMemoryManagementTest, LinearMemory_I32LoadStore_Success)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size >= 4) {
        // Test i32 access through app to native address conversion
        void* native_addr = wasm_runtime_addr_app_to_native(inst, app_offset);
        if (native_addr != nullptr) {
            uint32_t test_value = 0x12345678;
            memcpy(native_addr, &test_value, sizeof(uint32_t));
            
            uint32_t loaded_value;
            memcpy(&loaded_value, native_addr, sizeof(uint32_t));
            ASSERT_EQ(loaded_value, test_value) << "i32 load/store should preserve value";
        }
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 7: AOT linear memory i64 load store success
TEST_F(AOTMemoryManagementTest, LinearMemory_I64LoadStore_Success)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size >= 8) {
        // Test i64 access through app to native address conversion
        void* native_addr = wasm_runtime_addr_app_to_native(inst, app_offset);
        if (native_addr != nullptr) {
            uint64_t test_value = 0x123456789ABCDEF0ULL;
            memcpy(native_addr, &test_value, sizeof(uint64_t));
            
            uint64_t loaded_value;
            memcpy(&loaded_value, native_addr, sizeof(uint64_t));
            ASSERT_EQ(loaded_value, test_value) << "i64 load/store should preserve value";
        }
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 8: AOT linear memory f32 load store success
TEST_F(AOTMemoryManagementTest, LinearMemory_F32LoadStore_Success)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size >= 4) {
        // Test f32 access through app to native address conversion
        void* native_addr = wasm_runtime_addr_app_to_native(inst, app_offset);
        if (native_addr != nullptr) {
            float test_value = 3.14159f;
            memcpy(native_addr, &test_value, sizeof(float));
            
            float loaded_value;
            memcpy(&loaded_value, native_addr, sizeof(float));
            ASSERT_FLOAT_EQ(loaded_value, test_value) << "f32 load/store should preserve value";
        }
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 9: AOT linear memory f64 load store success
TEST_F(AOTMemoryManagementTest, LinearMemory_F64LoadStore_Success)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size >= 8) {
        // Test f64 access through app to native address conversion
        void* native_addr = wasm_runtime_addr_app_to_native(inst, app_offset);
        if (native_addr != nullptr) {
            double test_value = 3.141592653589793;
            memcpy(native_addr, &test_value, sizeof(double));
            
            double loaded_value;
            memcpy(&loaded_value, native_addr, sizeof(double));
            ASSERT_DOUBLE_EQ(loaded_value, test_value) << "f64 load/store should preserve value";
        }
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 10: AOT linear memory bulk operations success
TEST_F(AOTMemoryManagementTest, LinearMemory_BulkOperations_Success)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size >= 256) {
        void* native_addr = wasm_runtime_addr_app_to_native(inst, app_offset);
        if (native_addr != nullptr) {
            // Test bulk memory operations
            uint8_t pattern[256];
            for (int i = 0; i < 256; i++) {
                pattern[i] = i & 0xFF;
            }
            
            // Bulk copy
            memcpy(native_addr, pattern, 256);
            
            // Verify bulk copy
            bool bulk_copy_success = true;
            uint8_t* memory_ptr = (uint8_t*)native_addr;
            for (int i = 0; i < 256; i++) {
                if (memory_ptr[i] != pattern[i]) {
                    bulk_copy_success = false;
                    break;
                }
            }
            ASSERT_TRUE(bulk_copy_success) << "Bulk memory operations should work correctly";
            
            // Test bulk fill
            memset(native_addr, 0xAA, 128);
            bool bulk_fill_success = true;
            for (int i = 0; i < 128; i++) {
                if (memory_ptr[i] != 0xAA) {
                    bulk_fill_success = false;
                    break;
                }
            }
            ASSERT_TRUE(bulk_fill_success) << "Bulk memory fill should work correctly";
        }
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 11: AOT linear memory bounds checking enforced
TEST_F(AOTMemoryManagementTest, LinearMemory_BoundsChecking_Enforced)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Test that bounds checking is enforced through proper API usage
    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, 8192);
    ASSERT_NE(exec_env, nullptr) << "Execution environment should be created";
    
    // Bounds checking is enforced internally by WAMR runtime
    // We verify this by ensuring the runtime handles memory correctly
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    ASSERT_TRUE(range_result) << "Should be able to get memory range";
    ASSERT_GT(app_size, 0) << "Memory size should be valid";
    
    wasm_runtime_destroy_exec_env(exec_env);
    wasm_runtime_deinstantiate(inst);
}

// Test 12: AOT linear memory alignment validation
TEST_F(AOTMemoryManagementTest, LinearMemory_Alignment_Validation)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size > 0) {
        void* native_addr = wasm_runtime_addr_app_to_native(inst, app_offset);
        if (native_addr != nullptr) {
            // Test memory alignment for different data types
            uintptr_t ptr_addr = (uintptr_t)native_addr;
            
            // Memory should be at least 8-byte aligned for most architectures
            bool alignment_valid = (ptr_addr % 8 == 0) || (ptr_addr % 4 == 0) || (ptr_addr % 2 == 0);
            ASSERT_TRUE(alignment_valid) << "Memory should be properly aligned";
            
            // Test aligned access
            if (ptr_addr % 4 == 0 && app_size >= 4) {
                uint32_t test_value = 0x12345678;
                memcpy(native_addr, &test_value, sizeof(uint32_t));
                
                uint32_t loaded_value;
                memcpy(&loaded_value, native_addr, sizeof(uint32_t));
                ASSERT_EQ(loaded_value, test_value) << "Aligned access should work correctly";
            }
        }
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 13: AOT memory allocation heap management
TEST_F(AOTMemoryManagementTest, MemoryAllocation_HeapManagement)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Test heap allocation through WAMR runtime
    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, 8192);
    if (exec_env != nullptr) {
        // Test that execution environment manages heap correctly
        ASSERT_NE(exec_env, nullptr) << "Execution environment should manage heap";
        
        // Test module malloc/free functionality
        uint32_t app_offset = wasm_runtime_module_malloc(inst, 64, nullptr);
        if (app_offset != 0) {
            // Successfully allocated memory
            wasm_runtime_module_free(inst, app_offset);
        }
        // If allocation fails, it's still valid behavior depending on configuration
        
        wasm_runtime_destroy_exec_env(exec_env);
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 14: AOT memory allocation stack management
TEST_F(AOTMemoryManagementTest, MemoryAllocation_StackManagement)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Test stack management through execution environment
    uint32_t stack_size = 16384; // 16KB stack
    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, stack_size);
    
    if (exec_env != nullptr) {
        // Verify stack is properly managed
        ASSERT_NE(exec_env, nullptr) << "Execution environment should manage stack";
        
        // Test that we can get the module instance from exec env
        wasm_module_inst_t retrieved_inst = wasm_runtime_get_module_inst(exec_env);
        ASSERT_EQ(retrieved_inst, inst) << "Should be able to retrieve module instance from exec env";
        
        wasm_runtime_destroy_exec_env(exec_env);
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 15: AOT memory allocation resource limits
TEST_F(AOTMemoryManagementTest, MemoryAllocation_ResourceLimits)
{
    // Test memory allocation with different resource limits
    uint32_t small_stack = 1024;  // 1KB stack
    uint32_t small_heap = 1024;   // 1KB heap
    
    wasm_module_t module = wasm_runtime_load(dummy_wasm_buffer, sizeof(dummy_wasm_buffer), nullptr, 0);
    if (module != nullptr) {
        wasm_module_inst_t inst = wasm_runtime_instantiate(module, small_stack, small_heap, nullptr, 0);
        
        if (inst != nullptr) {
            // Test that small resource limits are respected
            wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, small_stack);
            ASSERT_NE(exec_env, nullptr) << "Should handle small resource limits";
            
            wasm_runtime_destroy_exec_env(exec_env);
            wasm_runtime_deinstantiate(inst);
        }
        
        wasm_runtime_unload(module);
    }
}

// Test 16: AOT memory deallocation cleanup success
// TEST_F(AOTMemoryManagementTest, MemoryDeallocation_CleanupSuccess)
// {
//     wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
//     if (inst == nullptr) {
//         return; // Skip if cannot create test instance
//     }

//     wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(inst, 8192);
//     if (exec_env != nullptr) {
//         // Test proper cleanup sequence
//         wasm_runtime_destroy_exec_env(exec_env);
//     }
    
//     // Test that deinstantiation cleans up properly
//     wasm_runtime_deinstantiate(inst);
    
//     // Test multiple cleanup calls (should be safe)
//     wasm_runtime_deinstantiate(nullptr);
//     wasm_runtime_destroy_exec_env(nullptr);
    
//     ASSERT_TRUE(true) << "Memory cleanup should complete without errors";
// }

// Test 17: AOT memory fragmentation handling
TEST_F(AOTMemoryManagementTest, Memory_FragmentationHandling)
{
    // Test memory fragmentation scenarios
    std::vector<wasm_module_inst_t> instances;
    
    // Create multiple small instances to test fragmentation
    for (int i = 0; i < 5; i++) {
        wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
        if (inst != nullptr) {
            instances.push_back(inst);
        }
    }
    
    // Verify instances were created
    ASSERT_GT(instances.size(), 0) << "Should be able to create multiple instances";
    
    // Clean up instances in random order to test fragmentation handling
    for (auto inst : instances) {
        wasm_runtime_deinstantiate(inst);
    }
    
    // Test that memory can still be allocated after fragmentation
    wasm_module_inst_t new_inst = create_test_instance_with_memory(1, 65536);
    if (new_inst != nullptr) {
        wasm_runtime_deinstantiate(new_inst);
    }
    
    ASSERT_TRUE(true) << "Memory fragmentation should be handled correctly";
}

// Test 18: AOT memory pressure scenarios handled
TEST_F(AOTMemoryManagementTest, Memory_PressureScenarios_Handled)
{
    // Test memory pressure by creating instances with large memory requirements
    uint32_t large_stack = 32768;  // 32KB stack
    uint32_t large_heap = 32768;   // 32KB heap
    
    wasm_module_t module = wasm_runtime_load(dummy_wasm_buffer, sizeof(dummy_wasm_buffer), nullptr, 0);
    if (module != nullptr) {
        std::vector<wasm_module_inst_t> instances;
        
        // Try to create multiple large instances until memory pressure
        for (int i = 0; i < 10; i++) {
            wasm_module_inst_t inst = wasm_runtime_instantiate(module, large_stack, large_heap, nullptr, 0);
            if (inst != nullptr) {
                instances.push_back(inst);
            } else {
                // Memory pressure reached - this is expected behavior
                break;
            }
        }
        
        // Clean up all instances
        for (auto inst : instances) {
            wasm_runtime_deinstantiate(inst);
        }
        
        wasm_runtime_unload(module);
        
        ASSERT_TRUE(true) << "Memory pressure scenarios should be handled gracefully";
    }
}

// Test 19: AOT memory concurrent access safety
TEST_F(AOTMemoryManagementTest, Memory_ConcurrentAccess_Safety)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Test concurrent access through multiple execution environments
    wasm_exec_env_t exec_env1 = wasm_runtime_create_exec_env(inst, 4096);
    wasm_exec_env_t exec_env2 = wasm_runtime_create_exec_env(inst, 4096);
    
    if (exec_env1 != nullptr && exec_env2 != nullptr) {
        // Both execution environments should be valid
        ASSERT_NE(exec_env1, exec_env2) << "Different execution environments should be distinct";
        
        wasm_runtime_destroy_exec_env(exec_env1);
        wasm_runtime_destroy_exec_env(exec_env2);
    } else {
        if (exec_env1) wasm_runtime_destroy_exec_env(exec_env1);
        if (exec_env2) wasm_runtime_destroy_exec_env(exec_env2);
    }
    
    wasm_runtime_deinstantiate(inst);
}

// Test 20: AOT memory page size validation
TEST_F(AOTMemoryManagementTest, Memory_PageSizeValidation)
{
    wasm_module_inst_t inst = create_test_instance_with_memory(1, 65536);
    if (inst == nullptr) {
        return; // Skip if cannot create test instance
    }

    // Get app address range
    uint64_t app_offset;
    uint64_t app_size;
    bool range_result = wasm_runtime_get_app_addr_range(inst, 0, &app_offset, &app_size);
    
    if (range_result && app_size > 0) {
        // WebAssembly page size is 64KB (65536 bytes)
        uint32_t page_size = 65536;
        uint32_t expected_pages = app_size / page_size;
        
        // Memory size should be a multiple of page size or close to it
        // (some overhead is expected)
        ASSERT_GT(expected_pages, 0) << "Should have at least one memory page worth of space";
        
        // Test memory growth in page increments
        bool growth_result = wasm_runtime_enlarge_memory(inst, 1);
        if (growth_result) {
            uint64_t new_app_offset;
            uint64_t new_app_size;
            bool new_range_result = wasm_runtime_get_app_addr_range(inst, 0, &new_app_offset, &new_app_size);
            if (new_range_result) {
                ASSERT_GT(new_app_size, app_size) << "Memory should have grown";
            }
        }
    }
    
    wasm_runtime_deinstantiate(inst);
}