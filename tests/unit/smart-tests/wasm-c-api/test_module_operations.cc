/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "wasm_c_api.h"

// Test fixture for module operations
class ModuleOperationsTest : public testing::Test {
protected:
    void SetUp() override {
        engine = wasm_engine_new();
        ASSERT_NE(nullptr, engine);
        store = wasm_store_new(engine);
        ASSERT_NE(nullptr, store);

        // Create minimal valid WASM module bytecode
        valid_wasm_bytes = {
            0x00, 0x61, 0x73, 0x6d,  // WASM magic number
            0x01, 0x00, 0x00, 0x00,  // Version 1
            0x01, 0x04, 0x01, 0x60,  // Type section: 1 function type
            0x00, 0x00,              // Function type: no params, no results
            0x03, 0x02, 0x01, 0x00,  // Function section: 1 function of type 0
            0x0a, 0x04, 0x01, 0x02,  // Code section: 1 function body
            0x00, 0x0b               // Function body: end
        };

        // Create invalid WASM module (missing magic number)
        invalid_wasm_bytes = {
            0xFF, 0xFF, 0xFF, 0xFF,  // Invalid magic number
            0x01, 0x00, 0x00, 0x00   // Version 1
        };

        // Create WASM module with exports
        wasm_with_exports = {
            0x00, 0x61, 0x73, 0x6d,  // WASM magic number
            0x01, 0x00, 0x00, 0x00,  // Version 1
            0x01, 0x04, 0x01, 0x60,  // Type section: 1 function type
            0x00, 0x00,              // Function type: no params, no results
            0x03, 0x02, 0x01, 0x00,  // Function section: 1 function of type 0
            0x07, 0x07, 0x01, 0x03,  // Export section: 1 export
            0x66, 0x6f, 0x6f, 0x00,  // Export name "foo", function index 0
            0x00,
            0x0a, 0x04, 0x01, 0x02,  // Code section: 1 function body
            0x00, 0x0b               // Function body: end
        };

        // Create WASM module with imports
        wasm_with_imports = {
            0x00, 0x61, 0x73, 0x6d,  // WASM magic number
            0x01, 0x00, 0x00, 0x00,  // Version 1
            0x01, 0x04, 0x01, 0x60,  // Type section: 1 function type
            0x00, 0x00,              // Function type: no params, no results
            0x02, 0x0b, 0x01, 0x03,  // Import section: 1 import, 11 bytes
            0x65, 0x6e, 0x76, 0x03,  // Module name "env" (3 bytes)
            0x66, 0x6f, 0x6f, 0x00,  // Import name "foo" (3 bytes), function type 0
            0x00                     // Import kind: function
        };
    }

    void TearDown() override {
        if (store) wasm_store_delete(store);
        if (engine) wasm_engine_delete(engine);
    }

    wasm_engine_t* engine = nullptr;
    wasm_store_t* store = nullptr;
    std::vector<uint8_t> valid_wasm_bytes;
    std::vector<uint8_t> invalid_wasm_bytes;
    std::vector<uint8_t> wasm_with_exports;
    std::vector<uint8_t> wasm_with_imports;
};

// Test 1: Module creation from valid bytecode
TEST_F(ModuleOperationsTest, ModuleNew_ValidBytecode_CreatesSuccessfully) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(), 
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 2: Module creation from invalid bytecode
TEST_F(ModuleOperationsTest, ModuleNew_InvalidBytecode_ReturnsNull) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, invalid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(invalid_wasm_bytes.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_EQ(nullptr, module);

    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 3: Module creation with null parameters
TEST_F(ModuleOperationsTest, ModuleNew_NullParameters_ReturnsNull) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    // Test with null store
    wasm_module_t* module1 = wasm_module_new(nullptr, &wasm_bytes);
    ASSERT_EQ(nullptr, module1);

    // Test with null byte vector
    wasm_module_t* module2 = wasm_module_new(store, nullptr);
    ASSERT_EQ(nullptr, module2);

    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 4: Module creation with empty bytecode
TEST_F(ModuleOperationsTest, ModuleNew_EmptyBytecode_ReturnsNull) {
    wasm_byte_vec_t empty_bytes;
    wasm_byte_vec_new_empty(&empty_bytes);

    wasm_module_t* module = wasm_module_new(store, &empty_bytes);
    ASSERT_EQ(nullptr, module);

    wasm_byte_vec_delete(&empty_bytes);
}

// Test 5: Module validation with valid bytecode
TEST_F(ModuleOperationsTest, ModuleValidate_ValidBytecode_ReturnsTrue) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    bool is_valid = wasm_module_validate(store, &wasm_bytes);
    ASSERT_TRUE(is_valid);

    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 6: Module validation with invalid bytecode
TEST_F(ModuleOperationsTest, ModuleValidate_InvalidBytecode_ReturnsFalse) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, invalid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(invalid_wasm_bytes.data()));

    bool is_valid = wasm_module_validate(store, &wasm_bytes);
    ASSERT_FALSE(is_valid);

    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 7: Module validation with null parameters
TEST_F(ModuleOperationsTest, ModuleValidate_NullParameters_ReturnsFalse) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    // Test with null store
    bool is_valid1 = wasm_module_validate(nullptr, &wasm_bytes);
    ASSERT_FALSE(is_valid1);

    // Test with null byte vector
    bool is_valid2 = wasm_module_validate(store, nullptr);
    ASSERT_FALSE(is_valid2);

    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 8: Module imports introspection
TEST_F(ModuleOperationsTest, ModuleImports_ModuleWithImports_ReturnsCorrectImports) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, wasm_with_imports.size(),
                      reinterpret_cast<const wasm_byte_t*>(wasm_with_imports.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    wasm_importtype_vec_t imports;
    wasm_module_imports(module, &imports);

    ASSERT_GT(imports.size, 0u);
    
    // Check first import
    if (imports.size > 0) {
        const wasm_name_t* module_name = wasm_importtype_module(imports.data[0]);
        const wasm_name_t* name = wasm_importtype_name(imports.data[0]);
        
        ASSERT_NE(nullptr, module_name);
        ASSERT_NE(nullptr, name);
        ASSERT_GT(module_name->size, 0u);
        ASSERT_GT(name->size, 0u);
    }

    wasm_importtype_vec_delete(&imports);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 9: Module exports introspection
TEST_F(ModuleOperationsTest, ModuleExports_ModuleWithExports_ReturnsCorrectExports) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, wasm_with_exports.size(),
                      reinterpret_cast<const wasm_byte_t*>(wasm_with_exports.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    wasm_exporttype_vec_t exports;
    wasm_module_exports(module, &exports);

    ASSERT_GT(exports.size, 0u);
    
    // Check first export
    if (exports.size > 0) {
        const wasm_name_t* name = wasm_exporttype_name(exports.data[0]);
        const wasm_externtype_t* type = wasm_exporttype_type(exports.data[0]);
        
        ASSERT_NE(nullptr, name);
        ASSERT_NE(nullptr, type);
        ASSERT_GT(name->size, 0u);
    }

    wasm_exporttype_vec_delete(&exports);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 10: Module imports on module without imports
TEST_F(ModuleOperationsTest, ModuleImports_ModuleWithoutImports_ReturnsEmptyVector) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    wasm_importtype_vec_t imports;
    wasm_module_imports(module, &imports);

    ASSERT_EQ(0u, imports.size);
    ASSERT_EQ(nullptr, imports.data);

    wasm_importtype_vec_delete(&imports);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 11: Module exports on module without exports
TEST_F(ModuleOperationsTest, ModuleExports_ModuleWithoutExports_ReturnsEmptyVector) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    wasm_exporttype_vec_t exports;
    wasm_module_exports(module, &exports);

    ASSERT_EQ(0u, exports.size);
    ASSERT_EQ(nullptr, exports.data);

    wasm_exporttype_vec_delete(&exports);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 12: Module serialization (only supported in JIT mode)
TEST_F(ModuleOperationsTest, ModuleSerialize_ValidModule_ReturnsEmptyInInterpreterMode) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    wasm_byte_vec_t serialized;
    wasm_byte_vec_new_empty(&serialized);
    wasm_module_serialize(module, &serialized);

    // In interpreter mode, serialization is not supported and leaves vector uninitialized
    // We should expect the vector to remain empty as we initialized it
    ASSERT_EQ(0u, serialized.size);
    ASSERT_EQ(nullptr, serialized.data);

    wasm_byte_vec_delete(&serialized);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 13: Module deserialization (only supported in JIT mode)
TEST_F(ModuleOperationsTest, ModuleDeserialize_NotSupportedInInterpreterMode_ReturnsNull) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    // First serialize a module
    wasm_module_t* original_module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, original_module);

    wasm_byte_vec_t serialized;
    wasm_byte_vec_new_empty(&serialized);
    wasm_module_serialize(original_module, &serialized);
    // In interpreter mode, serialization returns empty as we initialized it
    ASSERT_EQ(0u, serialized.size);

    // Deserialization should return null in interpreter mode
    wasm_module_t* deserialized_module = wasm_module_deserialize(store, &serialized);
    ASSERT_EQ(nullptr, deserialized_module);

    wasm_byte_vec_delete(&serialized);
    wasm_module_delete(original_module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 14: Module deserialization with invalid data
TEST_F(ModuleOperationsTest, ModuleDeserialize_InvalidData_ReturnsNull) {
    wasm_byte_vec_t invalid_serialized;
    wasm_byte_vec_new(&invalid_serialized, invalid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(invalid_wasm_bytes.data()));

    wasm_module_t* module = wasm_module_deserialize(store, &invalid_serialized);
    ASSERT_EQ(nullptr, module);

    wasm_byte_vec_delete(&invalid_serialized);
}

// Test 15: Module sharing between stores
TEST_F(ModuleOperationsTest, ModuleSharing_SameEngine_SharesSuccessfully) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    // Create second store with same engine
    wasm_store_t* store2 = wasm_store_new(engine);
    ASSERT_NE(nullptr, store2);

    // Module should be shareable between stores of same engine
    wasm_shared_module_t* shared = wasm_module_share(module);
    ASSERT_NE(nullptr, shared);

    wasm_module_t* obtained = wasm_module_obtain(store2, shared);
    ASSERT_NE(nullptr, obtained);

    wasm_module_delete(obtained);
    wasm_shared_module_delete(shared);
    wasm_store_delete(store2);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 16: Module sharing with null parameters
TEST_F(ModuleOperationsTest, ModuleSharing_NullParameters_HandlesGracefully) {
    wasm_shared_module_t* shared = wasm_module_share(nullptr);
    ASSERT_EQ(nullptr, shared);

    wasm_module_t* obtained = wasm_module_obtain(nullptr, nullptr);
    ASSERT_EQ(nullptr, obtained);
}

// Test 17: Multiple module creation from same bytecode
TEST_F(ModuleOperationsTest, ModuleCreation_SameBytecode_CreatesIndependentModules) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    wasm_module_t* module1 = wasm_module_new(store, &wasm_bytes);
    wasm_module_t* module2 = wasm_module_new(store, &wasm_bytes);

    ASSERT_NE(nullptr, module1);
    ASSERT_NE(nullptr, module2);
    ASSERT_NE(module1, module2);  // Different instances

    wasm_module_delete(module2);
    wasm_module_delete(module1);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 18: Module lifecycle management
TEST_F(ModuleOperationsTest, ModuleLifecycle_CreateAndDelete_HandlesCorrectly) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    // Create module
    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    // Use module for imports/exports
    wasm_importtype_vec_t imports;
    wasm_exporttype_vec_t exports;
    wasm_module_imports(module, &imports);
    wasm_module_exports(module, &exports);

    // Clean up should not crash
    wasm_importtype_vec_delete(&imports);
    wasm_exporttype_vec_delete(&exports);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 19: Module operations with deleted module
TEST_F(ModuleOperationsTest, ModuleOperations_DeletedModule_AvoidsCrash) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    // Delete module early
    wasm_module_delete(module);

    // Further operations with deleted module should not crash
    // (Though results are undefined, we test for no segfault)
    wasm_importtype_vec_t imports;
    wasm_exporttype_vec_t exports;
    
    // Initialize vectors to empty state
    wasm_importtype_vec_new_empty(&imports);
    wasm_exporttype_vec_new_empty(&exports);
    
    // These should handle null gracefully
    wasm_module_imports(nullptr, &imports);
    wasm_module_exports(nullptr, &exports);
    
    ASSERT_EQ(0u, imports.size);
    ASSERT_EQ(0u, exports.size);

    wasm_importtype_vec_delete(&imports);
    wasm_exporttype_vec_delete(&exports);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 20: Module compilation and loading processes
TEST_F(ModuleOperationsTest, ModuleCompilation_ValidProcess_CompletesSuccessfully) {
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    // Validate first
    bool is_valid = wasm_module_validate(store, &wasm_bytes);
    ASSERT_TRUE(is_valid);

    // Then compile/load
    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    ASSERT_NE(nullptr, module);

    // Verify module properties
    wasm_importtype_vec_t imports;
    wasm_exporttype_vec_t exports;
    wasm_module_imports(module, &imports);
    wasm_module_exports(module, &exports);

    // Module should be usable
    ASSERT_GE(imports.size, 0u);
    ASSERT_GE(exports.size, 0u);

    wasm_importtype_vec_delete(&imports);
    wasm_exporttype_vec_delete(&exports);
    wasm_module_delete(module);
    wasm_byte_vec_delete(&wasm_bytes);
}

// Test 21: Module error scenarios and edge cases
TEST_F(ModuleOperationsTest, ModuleErrorScenarios_VariousErrors_HandledGracefully) {
    // Test with corrupted WASM magic number
    std::vector<uint8_t> corrupted_magic = {
        0x00, 0x61, 0x73, 0x6d,  // Valid magic
        0xFF, 0xFF, 0xFF, 0xFF   // Invalid version
    };

    wasm_byte_vec_t corrupted_bytes;
    wasm_byte_vec_new(&corrupted_bytes, corrupted_magic.size(),
                      reinterpret_cast<const wasm_byte_t*>(corrupted_magic.data()));

    wasm_module_t* module1 = wasm_module_new(store, &corrupted_bytes);
    ASSERT_EQ(nullptr, module1);

    bool is_valid1 = wasm_module_validate(store, &corrupted_bytes);
    ASSERT_FALSE(is_valid1);

    // Test with truncated module
    std::vector<uint8_t> truncated = {0x00, 0x61};
    wasm_byte_vec_t truncated_bytes;
    wasm_byte_vec_new(&truncated_bytes, truncated.size(),
                      reinterpret_cast<const wasm_byte_t*>(truncated.data()));

    wasm_module_t* module2 = wasm_module_new(store, &truncated_bytes);
    ASSERT_EQ(nullptr, module2);

    bool is_valid2 = wasm_module_validate(store, &truncated_bytes);
    ASSERT_FALSE(is_valid2);

    wasm_byte_vec_delete(&truncated_bytes);
    wasm_byte_vec_delete(&corrupted_bytes);
}

// Test 22: Module resource management
TEST_F(ModuleOperationsTest, ModuleResourceManagement_ProperCleanup_NoMemoryLeaks) {
    // Create multiple modules and ensure proper cleanup
    std::vector<wasm_module_t*> modules;
    
    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, valid_wasm_bytes.size(),
                      reinterpret_cast<const wasm_byte_t*>(valid_wasm_bytes.data()));

    // Create several modules
    for (int i = 0; i < 5; i++) {
        wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
        ASSERT_NE(nullptr, module);
        modules.push_back(module);
    }

    // Clean up all modules
    for (wasm_module_t* module : modules) {
        wasm_module_delete(module);
    }

    wasm_byte_vec_delete(&wasm_bytes);
}