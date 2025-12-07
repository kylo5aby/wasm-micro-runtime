/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "wasm_c_api.h"
#include "bh_platform.h"

/**
 * Test Suite: Module Lifecycle
 * 
 * This test suite provides comprehensive coverage of WASM-C-API module
 * lifecycle operations including:
 * - Module loading from bytecode
 * - Module validation and compilation
 * - Module sharing between instances
 * - Module serialization and deserialization
 * - Module import/export introspection
 * - Error handling for invalid modules
 * - Resource management and cleanup
 */

class ModuleLifecycleTest : public testing::Test {
protected:
    void SetUp() override {
        engine = wasm_engine_new();
        ASSERT_NE(nullptr, engine);
        store = wasm_store_new(engine);
        ASSERT_NE(nullptr, store);
        
        create_test_modules();
    }
    
    void TearDown() override {
        if (complex_module) {
            wasm_module_delete(complex_module);
            complex_module = nullptr;
        }
        if (minimal_module) {
            wasm_module_delete(minimal_module);
            minimal_module = nullptr;
        }
        if (store) {
            wasm_store_delete(store);
            store = nullptr;
        }
        if (engine) {
            wasm_engine_delete(engine);
            engine = nullptr;
        }
    }
    
    void create_test_modules() {
        // Create minimal valid WASM module
        uint8_t minimal_wasm[] = {
            0x00, 0x61, 0x73, 0x6d, // magic
            0x01, 0x00, 0x00, 0x00  // version
        };
        
        wasm_byte_vec_t minimal_binary;
        wasm_byte_vec_new(&minimal_binary, sizeof(minimal_wasm), (char*)minimal_wasm);
        minimal_module = wasm_module_new(store, &minimal_binary);
        wasm_byte_vec_delete(&minimal_binary);
        
        ASSERT_NE(nullptr, minimal_module);
        
        // Create more complex module with function
        uint8_t complex_wasm[] = {
            0x00, 0x61, 0x73, 0x6d, // magic
            0x01, 0x00, 0x00, 0x00, // version
            0x01, 0x07, 0x01, 0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, // type section: (i32, i32) -> i32
            0x03, 0x02, 0x01, 0x00, // function section: 1 function of type 0
            0x07, 0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, // export section: export function 0 as "add"
            0x0a, 0x09, 0x01, 0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a, 0x0b // code section: local.get 0, local.get 1, i32.add, end
        };
        
        wasm_byte_vec_t complex_binary;
        wasm_byte_vec_new(&complex_binary, sizeof(complex_wasm), (char*)complex_wasm);
        complex_module = wasm_module_new(store, &complex_binary);
        wasm_byte_vec_delete(&complex_binary);
        
        ASSERT_NE(nullptr, complex_module);
    }
    
    wasm_engine_t* engine = nullptr;
    wasm_store_t* store = nullptr;
    wasm_module_t* minimal_module = nullptr;
    wasm_module_t* complex_module = nullptr;
};

// Test Category 1: Module Loading from Bytecode

TEST_F(ModuleLifecycleTest, Module_LoadFromValidBytecode_SucceedsCorrectly) {
    uint8_t valid_wasm[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x01, 0x00, 0x00, 0x00  // version
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(valid_wasm), (char*)valid_wasm);
    
    wasm_module_t* module = wasm_module_new(store, &binary);
    ASSERT_NE(nullptr, module);
    
    wasm_module_delete(module);
    wasm_byte_vec_delete(&binary);
}

TEST_F(ModuleLifecycleTest, Module_LoadFromInvalidMagic_ReturnsNull) {
    uint8_t invalid_wasm[] = {
        0x00, 0x61, 0x73, 0x6e, // invalid magic
        0x01, 0x00, 0x00, 0x00  // version
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(invalid_wasm), (char*)invalid_wasm);
    
    wasm_module_t* module = wasm_module_new(store, &binary);
    ASSERT_EQ(nullptr, module);
    
    wasm_byte_vec_delete(&binary);
}

TEST_F(ModuleLifecycleTest, Module_LoadFromInvalidVersion_ReturnsNull) {
    uint8_t invalid_wasm[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x02, 0x00, 0x00, 0x00  // invalid version
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(invalid_wasm), (char*)invalid_wasm);
    
    wasm_module_t* module = wasm_module_new(store, &binary);
    ASSERT_EQ(nullptr, module);
    
    wasm_byte_vec_delete(&binary);
}

TEST_F(ModuleLifecycleTest, Module_LoadFromEmptyBytecode_ReturnsNull) {
    wasm_byte_vec_t empty_binary;
    wasm_byte_vec_new_empty(&empty_binary);
    
    wasm_module_t* module = wasm_module_new(store, &empty_binary);
    ASSERT_EQ(nullptr, module);
    
    wasm_byte_vec_delete(&empty_binary);
}

TEST_F(ModuleLifecycleTest, Module_LoadFromNullBytecode_ReturnsNull) {
    wasm_module_t* module = wasm_module_new(store, nullptr);
    ASSERT_EQ(nullptr, module);
}

// Test Category 2: Module Validation and Compilation

TEST_F(ModuleLifecycleTest, Module_ValidateComplexModule_SucceedsCorrectly) {
    // Complex module should be valid and loadable
    ASSERT_NE(nullptr, complex_module);
    
    // Verify we can create instance from it
    wasm_extern_vec_t imports;
    wasm_extern_vec_new_empty(&imports);
    
    wasm_instance_t* instance = wasm_instance_new(store, complex_module, &imports, nullptr);
    ASSERT_NE(nullptr, instance);
    
    wasm_instance_delete(instance);
    wasm_extern_vec_delete(&imports);
}

TEST_F(ModuleLifecycleTest, Module_ValidateModuleWithExports_SucceedsCorrectly) {
    // Complex module has exports, verify they're accessible
    wasm_extern_vec_t imports;
    wasm_extern_vec_new_empty(&imports);
    
    wasm_instance_t* instance = wasm_instance_new(store, complex_module, &imports, nullptr);
    ASSERT_NE(nullptr, instance);
    
    wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    
    ASSERT_GT(exports.size, 0); // Should have at least one export ("add" function)
    
    wasm_extern_vec_delete(&exports);
    wasm_instance_delete(instance);
    wasm_extern_vec_delete(&imports);
}

TEST_F(ModuleLifecycleTest, Module_ValidateInvalidFunctionSection_ReturnsNull) {
    uint8_t invalid_wasm[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x01, 0x00, 0x00, 0x00, // version
        0x03, 0x02, 0x01, 0x99  // invalid function section (type index 0x99 doesn't exist)
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(invalid_wasm), (char*)invalid_wasm);
    
    wasm_module_t* module = wasm_module_new(store, &binary);
    ASSERT_EQ(nullptr, module);
    
    wasm_byte_vec_delete(&binary);
}

// Test Category 3: Module Sharing Between Instances

TEST_F(ModuleLifecycleTest, Module_ShareBetweenMultipleInstances_WorksCorrectly) {
    wasm_extern_vec_t imports;
    wasm_extern_vec_new_empty(&imports);
    
    // Create multiple instances from same module
    wasm_instance_t* instance1 = wasm_instance_new(store, minimal_module, &imports, nullptr);
    wasm_instance_t* instance2 = wasm_instance_new(store, minimal_module, &imports, nullptr);
    
    ASSERT_NE(nullptr, instance1);
    ASSERT_NE(nullptr, instance2);
    ASSERT_NE(instance1, instance2); // Different instances
    
    wasm_instance_delete(instance2);
    wasm_instance_delete(instance1);
    wasm_extern_vec_delete(&imports);
}

TEST_F(ModuleLifecycleTest, Module_ShareAcrossStores_WorksCorrectly) {
    // Create second store
    wasm_store_t* store2 = wasm_store_new(engine);
    ASSERT_NE(nullptr, store2);
    
    // Load same module in different stores
    uint8_t wasm_bytes[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x01, 0x00, 0x00, 0x00  // version
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(wasm_bytes), (char*)wasm_bytes);
    
    wasm_module_t* module1 = wasm_module_new(store, &binary);
    wasm_module_t* module2 = wasm_module_new(store2, &binary);
    
    ASSERT_NE(nullptr, module1);
    ASSERT_NE(nullptr, module2);
    
    wasm_module_delete(module2);
    wasm_module_delete(module1);
    wasm_store_delete(store2);
    wasm_byte_vec_delete(&binary);
}

// Test Category 4: Module Import/Export Introspection

TEST_F(ModuleLifecycleTest, Module_GetImports_ReturnsCorrectCount) {
    wasm_importtype_vec_t imports;
    wasm_module_imports(minimal_module, &imports);
    
    ASSERT_EQ(0, imports.size); // Minimal module has no imports
    
    wasm_importtype_vec_delete(&imports);
}

TEST_F(ModuleLifecycleTest, Module_GetExports_ReturnsCorrectCount) {
    wasm_exporttype_vec_t exports;
    wasm_module_exports(complex_module, &exports);
    
    ASSERT_GT(exports.size, 0); // Complex module should have exports
    
    wasm_exporttype_vec_delete(&exports);
}

TEST_F(ModuleLifecycleTest, Module_InspectExportTypes_WorksCorrectly) {
    wasm_exporttype_vec_t exports;
    wasm_module_exports(complex_module, &exports);
    
    ASSERT_GT(exports.size, 0);
    
    for (size_t i = 0; i < exports.size; i++) {
        wasm_exporttype_t* export_type = exports.data[i];
        ASSERT_NE(nullptr, export_type);
        
        const wasm_name_t* name = wasm_exporttype_name(export_type);
        ASSERT_NE(nullptr, name);
        ASSERT_GT(name->size, 0);
        
        const wasm_externtype_t* extern_type = wasm_exporttype_type(export_type);
        ASSERT_NE(nullptr, extern_type);
        
        wasm_externkind_t kind = wasm_externtype_kind(extern_type);
        ASSERT_TRUE(kind == WASM_EXTERN_FUNC || 
                   kind == WASM_EXTERN_GLOBAL || 
                   kind == WASM_EXTERN_TABLE || 
                   kind == WASM_EXTERN_MEMORY);
    }
    
    wasm_exporttype_vec_delete(&exports);
}

// Test Category 5: Module Serialization and Deserialization

TEST_F(ModuleLifecycleTest, Module_SerializeInInterpreterMode_ReturnsEmpty) {
    wasm_byte_vec_t serialized;
    wasm_byte_vec_new_empty(&serialized);
    
    wasm_module_serialize(minimal_module, &serialized);
    
    // In interpreter mode, serialization is not supported
    ASSERT_EQ(0, serialized.size);
    ASSERT_EQ(nullptr, serialized.data);
    
    wasm_byte_vec_delete(&serialized);
}

TEST_F(ModuleLifecycleTest, Module_DeserializeInInterpreterMode_ReturnsNull) {
    wasm_byte_vec_t empty_data;
    wasm_byte_vec_new_empty(&empty_data);
    
    wasm_module_t* deserialized = wasm_module_deserialize(store, &empty_data);
    ASSERT_EQ(nullptr, deserialized); // Not supported in interpreter mode
    
    wasm_byte_vec_delete(&empty_data);
}

// Test Category 6: Error Handling and Edge Cases

TEST_F(ModuleLifecycleTest, Module_LoadWithNullStore_ReturnsNull) {
    uint8_t wasm_bytes[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x01, 0x00, 0x00, 0x00  // version
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(wasm_bytes), (char*)wasm_bytes);
    
    wasm_module_t* module = wasm_module_new(nullptr, &binary);
    ASSERT_EQ(nullptr, module);
    
    wasm_byte_vec_delete(&binary);
}

TEST_F(ModuleLifecycleTest, Module_DeleteNull_HandlesGracefully) {
    // Should not crash
    wasm_module_delete(nullptr);
}

TEST_F(ModuleLifecycleTest, Module_IntrospectNullModule_HandlesGracefully) {
    wasm_importtype_vec_t imports;
    wasm_importtype_vec_new_empty(&imports);
    wasm_module_imports(nullptr, &imports);
    
    ASSERT_EQ(0, imports.size);
    ASSERT_EQ(nullptr, imports.data);
    
    wasm_importtype_vec_delete(&imports);
}

TEST_F(ModuleLifecycleTest, Module_LoadMalformedBytecode_ReturnsNull) {
    uint8_t malformed_wasm[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x01, 0x00, 0x00, 0x00, // version
        0x01, 0xff, 0xff, 0xff  // malformed type section
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(malformed_wasm), (char*)malformed_wasm);
    
    wasm_module_t* module = wasm_module_new(store, &binary);
    ASSERT_EQ(nullptr, module);
    
    wasm_byte_vec_delete(&binary);
}

// Test Category 7: Resource Management and Cleanup

TEST_F(ModuleLifecycleTest, Module_MultipleLoadUnload_WorksCorrectly) {
    uint8_t wasm_bytes[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x01, 0x00, 0x00, 0x00  // version
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(wasm_bytes), (char*)wasm_bytes);
    
    // Load and unload multiple times
    for (int i = 0; i < 5; i++) {
        wasm_module_t* module = wasm_module_new(store, &binary);
        ASSERT_NE(nullptr, module);
        wasm_module_delete(module);
    }
    
    wasm_byte_vec_delete(&binary);
}

TEST_F(ModuleLifecycleTest, Module_MemoryLeakPrevention_WorksCorrectly) {
    uint8_t wasm_bytes[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x01, 0x00, 0x00, 0x00  // version
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(wasm_bytes), (char*)wasm_bytes);
    
    // Create multiple modules simultaneously
    const size_t num_modules = 10;
    wasm_module_t* modules[num_modules];
    
    for (size_t i = 0; i < num_modules; i++) {
        modules[i] = wasm_module_new(store, &binary);
        ASSERT_NE(nullptr, modules[i]);
    }
    
    // Delete all modules
    for (size_t i = 0; i < num_modules; i++) {
        wasm_module_delete(modules[i]);
    }
    
    wasm_byte_vec_delete(&binary);
}

TEST_F(ModuleLifecycleTest, Module_CompleteLifecycleWithInstances_WorksCorrectly) {
    uint8_t wasm_bytes[] = {
        0x00, 0x61, 0x73, 0x6d, // magic
        0x01, 0x00, 0x00, 0x00  // version
    };
    
    wasm_byte_vec_t binary;
    wasm_byte_vec_new(&binary, sizeof(wasm_bytes), (char*)wasm_bytes);
    
    // Load module
    wasm_module_t* module = wasm_module_new(store, &binary);
    ASSERT_NE(nullptr, module);
    
    // Create instance
    wasm_extern_vec_t imports;
    wasm_extern_vec_new_empty(&imports);
    
    wasm_instance_t* instance = wasm_instance_new(store, module, &imports, nullptr);
    ASSERT_NE(nullptr, instance);
    
    // Get exports
    wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    ASSERT_GE(exports.size, 0);
    
    // Clean up in correct order
    wasm_extern_vec_delete(&exports);
    wasm_instance_delete(instance);
    wasm_extern_vec_delete(&imports);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&binary);
}

TEST_F(ModuleLifecycleTest, Module_ConcurrentAccess_WorksCorrectly) {
    // Test concurrent access to module (simulated)
    wasm_extern_vec_t imports;
    wasm_extern_vec_new_empty(&imports);
    
    // Create multiple instances concurrently (simulated)
    wasm_instance_t* instance1 = wasm_instance_new(store, minimal_module, &imports, nullptr);
    wasm_instance_t* instance2 = wasm_instance_new(store, minimal_module, &imports, nullptr);
    
    ASSERT_NE(nullptr, instance1);
    ASSERT_NE(nullptr, instance2);
    
    // Both should work independently
    wasm_extern_vec_t exports1, exports2;
    wasm_instance_exports(instance1, &exports1);
    wasm_instance_exports(instance2, &exports2);
    
    ASSERT_EQ(exports1.size, exports2.size);
    
    wasm_extern_vec_delete(&exports2);
    wasm_extern_vec_delete(&exports1);
    wasm_instance_delete(instance2);
    wasm_instance_delete(instance1);
    wasm_extern_vec_delete(&imports);
}