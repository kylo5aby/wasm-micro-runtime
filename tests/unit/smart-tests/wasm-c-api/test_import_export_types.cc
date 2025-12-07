/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include "wasm_c_api.h"

class ImportExportTypesTest : public testing::Test
{
protected:
    void SetUp() override
    {
        engine = wasm_engine_new();
        ASSERT_NE(nullptr, engine);
        store = wasm_store_new(engine);
        ASSERT_NE(nullptr, store);
    }

    void TearDown() override
    {
        if (store) {
            wasm_store_delete(store);
        }
        if (engine) {
            wasm_engine_delete(engine);
        }
    }

    wasm_engine_t* engine = nullptr;
    wasm_store_t* store = nullptr;

    // Helper to create a simple function type
    wasm_functype_t* create_simple_functype()
    {
        wasm_valtype_vec_t params, results;
        wasm_valtype_vec_new_empty(&params);
        wasm_valtype_vec_new_uninitialized(&results, 1);
        results.data[0] = wasm_valtype_new(WASM_I32);
        return wasm_functype_new(&params, &results);
    }

    // Helper to create a simple global type
    wasm_globaltype_t* create_simple_globaltype()
    {
        wasm_valtype_t* valtype = wasm_valtype_new(WASM_I32);
        return wasm_globaltype_new(valtype, WASM_CONST);
    }

    // Helper to create a name
    wasm_name_t* create_name(const char* str)
    {
        wasm_name_t* name = new wasm_name_t;
        wasm_name_new_from_string(name, str);
        return name;
    }
};

// Import Type Creation Tests
TEST_F(ImportExportTypesTest, ImportType_CreateWithFunctionType_SucceedsCorrectly)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name("test_function");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    ASSERT_NE(nullptr, importtype);
    ASSERT_EQ(WASM_EXTERN_FUNC, wasm_externtype_kind(wasm_importtype_type(importtype)));
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ImportType_CreateWithGlobalType_SucceedsCorrectly)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name("test_global");
    wasm_globaltype_t* globaltype = create_simple_globaltype();
    wasm_externtype_t* externtype = wasm_globaltype_as_externtype(globaltype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    ASSERT_NE(nullptr, importtype);
    ASSERT_EQ(WASM_EXTERN_GLOBAL, wasm_externtype_kind(wasm_importtype_type(importtype)));
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ImportType_CreateWithTableType_SucceedsCorrectly)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name("test_table");
    
    wasm_valtype_t* elemtype = wasm_valtype_new(WASM_FUNCREF);
    wasm_limits_t limits = { 10, 100 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(elemtype, &limits);
    wasm_externtype_t* externtype = wasm_tabletype_as_externtype(tabletype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    ASSERT_NE(nullptr, importtype);
    ASSERT_EQ(WASM_EXTERN_TABLE, wasm_externtype_kind(wasm_importtype_type(importtype)));
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ImportType_CreateWithMemoryType_SucceedsCorrectly)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name("test_memory");
    
    wasm_limits_t limits = { 1, 10 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    wasm_externtype_t* externtype = wasm_memorytype_as_externtype(memorytype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    ASSERT_NE(nullptr, importtype);
    ASSERT_EQ(WASM_EXTERN_MEMORY, wasm_externtype_kind(wasm_importtype_type(importtype)));
    
    wasm_importtype_delete(importtype);
}

// Import Type Inspection Tests
TEST_F(ImportExportTypesTest, ImportType_ModuleName_ReturnsCorrectName)
{
    const char* expected_module = "test_module";
    wasm_name_t* module_name = create_name(expected_module);
    wasm_name_t* import_name = create_name("test_function");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    const wasm_name_t* retrieved_module = wasm_importtype_module(importtype);
    ASSERT_NE(nullptr, retrieved_module);
    ASSERT_EQ(strlen(expected_module), retrieved_module->size);
    ASSERT_EQ(0, strncmp(expected_module, retrieved_module->data, retrieved_module->size));
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ImportType_ImportName_ReturnsCorrectName)
{
    const char* expected_name = "test_function";
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name(expected_name);
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    const wasm_name_t* retrieved_name = wasm_importtype_name(importtype);
    ASSERT_NE(nullptr, retrieved_name);
    ASSERT_EQ(strlen(expected_name), retrieved_name->size);
    ASSERT_EQ(0, strncmp(expected_name, retrieved_name->data, retrieved_name->size));
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ImportType_Type_ReturnsCorrectExternType)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name("test_function");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    const wasm_externtype_t* retrieved_type = wasm_importtype_type(importtype);
    ASSERT_NE(nullptr, retrieved_type);
    ASSERT_EQ(WASM_EXTERN_FUNC, wasm_externtype_kind(retrieved_type));
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ImportType_IsLinked_ReturnsCorrectStatus)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name("test_function");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    // Initially should not be linked
    bool is_linked = wasm_importtype_is_linked(importtype);
    ASSERT_FALSE(is_linked);
    
    wasm_importtype_delete(importtype);
}

// Export Type Creation Tests
TEST_F(ImportExportTypesTest, ExportType_CreateWithFunctionType_SucceedsCorrectly)
{
    wasm_name_t* export_name = create_name("exported_function");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
    
    ASSERT_NE(nullptr, exporttype);
    ASSERT_EQ(WASM_EXTERN_FUNC, wasm_externtype_kind(wasm_exporttype_type(exporttype)));
    
    wasm_exporttype_delete(exporttype);
}

TEST_F(ImportExportTypesTest, ExportType_CreateWithGlobalType_SucceedsCorrectly)
{
    wasm_name_t* export_name = create_name("exported_global");
    wasm_globaltype_t* globaltype = create_simple_globaltype();
    wasm_externtype_t* externtype = wasm_globaltype_as_externtype(globaltype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
    
    ASSERT_NE(nullptr, exporttype);
    ASSERT_EQ(WASM_EXTERN_GLOBAL, wasm_externtype_kind(wasm_exporttype_type(exporttype)));
    
    wasm_exporttype_delete(exporttype);
}

TEST_F(ImportExportTypesTest, ExportType_CreateWithTableType_SucceedsCorrectly)
{
    wasm_name_t* export_name = create_name("exported_table");
    
    wasm_valtype_t* elemtype = wasm_valtype_new(WASM_FUNCREF);
    wasm_limits_t limits = { 5, 50 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(elemtype, &limits);
    wasm_externtype_t* externtype = wasm_tabletype_as_externtype(tabletype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
    
    ASSERT_NE(nullptr, exporttype);
    ASSERT_EQ(WASM_EXTERN_TABLE, wasm_externtype_kind(wasm_exporttype_type(exporttype)));
    
    wasm_exporttype_delete(exporttype);
}

TEST_F(ImportExportTypesTest, ExportType_CreateWithMemoryType_SucceedsCorrectly)
{
    wasm_name_t* export_name = create_name("exported_memory");
    
    wasm_limits_t limits = { 2, 20 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    wasm_externtype_t* externtype = wasm_memorytype_as_externtype(memorytype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
    
    ASSERT_NE(nullptr, exporttype);
    ASSERT_EQ(WASM_EXTERN_MEMORY, wasm_externtype_kind(wasm_exporttype_type(exporttype)));
    
    wasm_exporttype_delete(exporttype);
}

// Export Type Inspection Tests
TEST_F(ImportExportTypesTest, ExportType_Name_ReturnsCorrectName)
{
    const char* expected_name = "exported_function";
    wasm_name_t* export_name = create_name(expected_name);
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
    
    const wasm_name_t* retrieved_name = wasm_exporttype_name(exporttype);
    ASSERT_NE(nullptr, retrieved_name);
    ASSERT_EQ(strlen(expected_name), retrieved_name->size);
    ASSERT_EQ(0, strncmp(expected_name, retrieved_name->data, retrieved_name->size));
    
    wasm_exporttype_delete(exporttype);
}

TEST_F(ImportExportTypesTest, ExportType_Type_ReturnsCorrectExternType)
{
    wasm_name_t* export_name = create_name("exported_function");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
    
    const wasm_externtype_t* retrieved_type = wasm_exporttype_type(exporttype);
    ASSERT_NE(nullptr, retrieved_type);
    ASSERT_EQ(WASM_EXTERN_FUNC, wasm_externtype_kind(retrieved_type));
    
    wasm_exporttype_delete(exporttype);
}

// Edge Cases and Error Handling Tests
TEST_F(ImportExportTypesTest, ImportType_CreateWithNullModuleName_HandlesGracefully)
{
    wasm_name_t* import_name = create_name("test_function");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(nullptr, import_name, externtype);
    
    // Implementation may handle this differently - test actual behavior
    if (importtype) {
        wasm_importtype_delete(importtype);
    }
    // Test passes if no crash occurs
    SUCCEED();
}

TEST_F(ImportExportTypesTest, ImportType_CreateWithNullImportName_HandlesGracefully)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, nullptr, externtype);
    
    // Implementation may handle this differently - test actual behavior
    if (importtype) {
        wasm_importtype_delete(importtype);
    }
    // Test passes if no crash occurs
    SUCCEED();
}

TEST_F(ImportExportTypesTest, ImportType_CreateWithNullExternType_HandlesGracefully)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name("test_function");
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, nullptr);
    
    // Implementation may handle this differently - test actual behavior
    if (importtype) {
        wasm_importtype_delete(importtype);
    }
    // Test passes if no crash occurs
    SUCCEED();
}

TEST_F(ImportExportTypesTest, ExportType_CreateWithNullName_HandlesGracefully)
{
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(nullptr, externtype);
    
    // Implementation may handle this differently - test actual behavior
    if (exporttype) {
        wasm_exporttype_delete(exporttype);
    }
    // Test passes if no crash occurs
    SUCCEED();
}

TEST_F(ImportExportTypesTest, ExportType_CreateWithNullExternType_HandlesGracefully)
{
    wasm_name_t* export_name = create_name("exported_function");
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, nullptr);
    
    // Implementation may handle this differently - test actual behavior
    if (exporttype) {
        wasm_exporttype_delete(exporttype);
    }
    // Test passes if no crash occurs
    SUCCEED();
}

// Name Validation Tests
TEST_F(ImportExportTypesTest, ImportType_EmptyModuleName_HandlesCorrectly)
{
    wasm_name_t* module_name = create_name("");
    wasm_name_t* import_name = create_name("test_function");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    ASSERT_NE(nullptr, importtype);
    
    const wasm_name_t* retrieved_module = wasm_importtype_module(importtype);
    ASSERT_NE(nullptr, retrieved_module);
    ASSERT_EQ(0, retrieved_module->size);
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ImportType_EmptyImportName_HandlesCorrectly)
{
    wasm_name_t* module_name = create_name("test_module");
    wasm_name_t* import_name = create_name("");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    ASSERT_NE(nullptr, importtype);
    
    const wasm_name_t* retrieved_name = wasm_importtype_name(importtype);
    ASSERT_NE(nullptr, retrieved_name);
    ASSERT_EQ(0, retrieved_name->size);
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ExportType_EmptyName_HandlesCorrectly)
{
    wasm_name_t* export_name = create_name("");
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
    
    ASSERT_NE(nullptr, exporttype);
    
    const wasm_name_t* retrieved_name = wasm_exporttype_name(exporttype);
    ASSERT_NE(nullptr, retrieved_name);
    ASSERT_EQ(0, retrieved_name->size);
    
    wasm_exporttype_delete(exporttype);
}

// Complex Name Tests
TEST_F(ImportExportTypesTest, ImportType_LongNames_HandlesCorrectly)
{
    std::string long_module(1000, 'M');
    std::string long_import(1000, 'I');
    
    wasm_name_t* module_name = create_name(long_module.c_str());
    wasm_name_t* import_name = create_name(long_import.c_str());
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
    
    ASSERT_NE(nullptr, importtype);
    
    const wasm_name_t* retrieved_module = wasm_importtype_module(importtype);
    const wasm_name_t* retrieved_name = wasm_importtype_name(importtype);
    
    ASSERT_NE(nullptr, retrieved_module);
    ASSERT_NE(nullptr, retrieved_name);
    ASSERT_EQ(1000, retrieved_module->size);
    ASSERT_EQ(1000, retrieved_name->size);
    
    wasm_importtype_delete(importtype);
}

TEST_F(ImportExportTypesTest, ExportType_LongName_HandlesCorrectly)
{
    std::string long_name(1000, 'E');
    
    wasm_name_t* export_name = create_name(long_name.c_str());
    wasm_functype_t* functype = create_simple_functype();
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    
    wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
    
    ASSERT_NE(nullptr, exporttype);
    
    const wasm_name_t* retrieved_name = wasm_exporttype_name(exporttype);
    ASSERT_NE(nullptr, retrieved_name);
    ASSERT_EQ(1000, retrieved_name->size);
    
    wasm_exporttype_delete(exporttype);
}

// Resource Management Tests
TEST_F(ImportExportTypesTest, ImportType_MultipleCreationDeletion_NoMemoryLeaks)
{
    for (int i = 0; i < 100; ++i) {
        wasm_name_t* module_name = create_name("test_module");
        wasm_name_t* import_name = create_name("test_function");
        wasm_functype_t* functype = create_simple_functype();
        wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
        
        wasm_importtype_t* importtype = wasm_importtype_new(module_name, import_name, externtype);
        ASSERT_NE(nullptr, importtype);
        
        wasm_importtype_delete(importtype);
    }
    // Test passes if no memory issues occur
    SUCCEED();
}

TEST_F(ImportExportTypesTest, ExportType_MultipleCreationDeletion_NoMemoryLeaks)
{
    for (int i = 0; i < 100; ++i) {
        wasm_name_t* export_name = create_name("exported_function");
        wasm_functype_t* functype = create_simple_functype();
        wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
        
        wasm_exporttype_t* exporttype = wasm_exporttype_new(export_name, externtype);
        ASSERT_NE(nullptr, exporttype);
        
        wasm_exporttype_delete(exporttype);
    }
    // Test passes if no memory issues occur
    SUCCEED();
}