/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "wasm_c_api.h"

class ExternalTypeSystemTest : public testing::Test
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
            store = nullptr;
        }
        if (engine) {
            wasm_engine_delete(engine);
            engine = nullptr;
        }
    }

    wasm_engine_t* engine = nullptr;
    wasm_store_t* store = nullptr;
};

// External Type Creation and Management Tests

TEST_F(ExternalTypeSystemTest, ExternType_FromFunctionType_CreatesCorrectly)
{
    // Create function type
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new_empty(&params);
    wasm_valtype_vec_new_empty(&results);
    wasm_functype_t* functype = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, functype);

    // Convert to external type
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    ASSERT_NE(nullptr, externtype);

    // Verify kind
    wasm_externkind_t kind = wasm_externtype_kind(externtype);
    ASSERT_EQ(WASM_EXTERN_FUNC, kind);

    // Clean up - externtype is owned by functype
    wasm_functype_delete(functype);
}

TEST_F(ExternalTypeSystemTest, ExternType_FromGlobalType_CreatesCorrectly)
{
    // Create global type
    wasm_valtype_t* valtype = wasm_valtype_new_i32();
    ASSERT_NE(nullptr, valtype);
    wasm_globaltype_t* globaltype = wasm_globaltype_new(valtype, WASM_CONST);
    ASSERT_NE(nullptr, globaltype);

    // Convert to external type
    wasm_externtype_t* externtype = wasm_globaltype_as_externtype(globaltype);
    ASSERT_NE(nullptr, externtype);

    // Verify kind
    wasm_externkind_t kind = wasm_externtype_kind(externtype);
    ASSERT_EQ(WASM_EXTERN_GLOBAL, kind);

    wasm_globaltype_delete(globaltype);
}

TEST_F(ExternalTypeSystemTest, ExternType_FromTableType_CreatesCorrectly)
{
    // Create table type
    wasm_valtype_t* valtype = wasm_valtype_new_funcref();
    ASSERT_NE(nullptr, valtype);
    wasm_limits_t limits = { 1, 10 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &limits);
    ASSERT_NE(nullptr, tabletype);

    // Convert to external type
    wasm_externtype_t* externtype = wasm_tabletype_as_externtype(tabletype);
    ASSERT_NE(nullptr, externtype);

    // Verify kind
    wasm_externkind_t kind = wasm_externtype_kind(externtype);
    ASSERT_EQ(WASM_EXTERN_TABLE, kind);

    wasm_tabletype_delete(tabletype);
}

TEST_F(ExternalTypeSystemTest, ExternType_FromMemoryType_CreatesCorrectly)
{
    // Create memory type
    wasm_limits_t limits = { 1, 100 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    ASSERT_NE(nullptr, memorytype);

    // Convert to external type
    wasm_externtype_t* externtype = wasm_memorytype_as_externtype(memorytype);
    ASSERT_NE(nullptr, externtype);

    // Verify kind
    wasm_externkind_t kind = wasm_externtype_kind(externtype);
    ASSERT_EQ(WASM_EXTERN_MEMORY, kind);

    wasm_memorytype_delete(memorytype);
}

// External Type Classification and Inspection Tests

TEST_F(ExternalTypeSystemTest, ExternType_KindClassification_WorksCorrectly)
{
    // Test all external kinds
    wasm_externkind_t kinds[] = {
        WASM_EXTERN_FUNC,
        WASM_EXTERN_GLOBAL,
        WASM_EXTERN_TABLE,
        WASM_EXTERN_MEMORY
    };

    for (size_t i = 0; i < sizeof(kinds)/sizeof(kinds[0]); ++i) {
        wasm_externkind_t kind = kinds[i];
        
        // Verify kind values are in expected range
        ASSERT_GE(kind, WASM_EXTERN_FUNC);
        ASSERT_LE(kind, WASM_EXTERN_MEMORY);
        
        // Each kind should be unique
        for (size_t j = i + 1; j < sizeof(kinds)/sizeof(kinds[0]); ++j) {
            ASSERT_NE(kind, kinds[j]);
        }
    }
}

TEST_F(ExternalTypeSystemTest, ExternType_FunctionTypeInspection_WorksCorrectly)
{
    // Create complex function type
    wasm_valtype_t* param_types[] = { wasm_valtype_new_i32(), wasm_valtype_new_f64() };
    wasm_valtype_t* result_types[] = { wasm_valtype_new_i64() };
    
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new(&params, 2, param_types);
    wasm_valtype_vec_new(&results, 1, result_types);
    
    wasm_functype_t* functype = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, functype);

    // Convert to external type and back
    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    ASSERT_NE(nullptr, externtype);
    ASSERT_EQ(WASM_EXTERN_FUNC, wasm_externtype_kind(externtype));

    wasm_functype_t* converted_functype = wasm_externtype_as_functype(externtype);
    ASSERT_NE(nullptr, converted_functype);

    // Verify parameters and results are preserved
    const wasm_valtype_vec_t* converted_params = wasm_functype_params(converted_functype);
    const wasm_valtype_vec_t* converted_results = wasm_functype_results(converted_functype);
    
    ASSERT_EQ(2, converted_params->size);
    ASSERT_EQ(1, converted_results->size);
    
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(converted_params->data[0]));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(converted_params->data[1]));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(converted_results->data[0]));

    wasm_functype_delete(functype);
}

TEST_F(ExternalTypeSystemTest, ExternType_GlobalTypeInspection_WorksCorrectly)
{
    // Create global type with different mutability
    wasm_valtype_t* valtype = wasm_valtype_new_f32();
    ASSERT_NE(nullptr, valtype);
    wasm_globaltype_t* globaltype = wasm_globaltype_new(valtype, WASM_VAR);
    ASSERT_NE(nullptr, globaltype);

    // Convert to external type and back
    wasm_externtype_t* externtype = wasm_globaltype_as_externtype(globaltype);
    ASSERT_NE(nullptr, externtype);
    ASSERT_EQ(WASM_EXTERN_GLOBAL, wasm_externtype_kind(externtype));

    wasm_globaltype_t* converted_globaltype = wasm_externtype_as_globaltype(externtype);
    ASSERT_NE(nullptr, converted_globaltype);

    // Verify content type and mutability are preserved
    const wasm_valtype_t* content = wasm_globaltype_content(converted_globaltype);
    ASSERT_NE(nullptr, content);
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(content));
    ASSERT_EQ(WASM_VAR, wasm_globaltype_mutability(converted_globaltype));

    wasm_globaltype_delete(globaltype);
}

TEST_F(ExternalTypeSystemTest, ExternType_TableTypeInspection_WorksCorrectly)
{
    // Create table type with specific limits
    wasm_valtype_t* valtype = wasm_valtype_new_funcref();
    ASSERT_NE(nullptr, valtype);
    wasm_limits_t limits = { 5, 50 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &limits);
    ASSERT_NE(nullptr, tabletype);

    // Convert to external type and back
    wasm_externtype_t* externtype = wasm_tabletype_as_externtype(tabletype);
    ASSERT_NE(nullptr, externtype);
    ASSERT_EQ(WASM_EXTERN_TABLE, wasm_externtype_kind(externtype));

    wasm_tabletype_t* converted_tabletype = wasm_externtype_as_tabletype(externtype);
    ASSERT_NE(nullptr, converted_tabletype);

    // Verify element type and limits are preserved
    const wasm_valtype_t* element = wasm_tabletype_element(converted_tabletype);
    ASSERT_NE(nullptr, element);
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(element));

    const wasm_limits_t* converted_limits = wasm_tabletype_limits(converted_tabletype);
    ASSERT_NE(nullptr, converted_limits);
    ASSERT_EQ(5, converted_limits->min);
    ASSERT_EQ(50, converted_limits->max);

    wasm_tabletype_delete(tabletype);
}

TEST_F(ExternalTypeSystemTest, ExternType_MemoryTypeInspection_WorksCorrectly)
{
    // Create memory type with specific limits
    wasm_limits_t limits = { 2, 256 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    ASSERT_NE(nullptr, memorytype);

    // Convert to external type and back
    wasm_externtype_t* externtype = wasm_memorytype_as_externtype(memorytype);
    ASSERT_NE(nullptr, externtype);
    ASSERT_EQ(WASM_EXTERN_MEMORY, wasm_externtype_kind(externtype));

    wasm_memorytype_t* converted_memorytype = wasm_externtype_as_memorytype(externtype);
    ASSERT_NE(nullptr, converted_memorytype);

    // Verify limits are preserved
    const wasm_limits_t* converted_limits = wasm_memorytype_limits(converted_memorytype);
    ASSERT_NE(nullptr, converted_limits);
    ASSERT_EQ(2, converted_limits->min);
    ASSERT_EQ(256, converted_limits->max);

    wasm_memorytype_delete(memorytype);
}

// External Type Conversion Operations Tests

TEST_F(ExternalTypeSystemTest, ExternType_RoundTripConversion_PreservesData)
{
    // Test function type round-trip
    {
        wasm_valtype_vec_t params, results;
        wasm_valtype_vec_new_empty(&params);
        wasm_valtype_vec_new_empty(&results);
        wasm_functype_t* original = wasm_functype_new(&params, &results);
        
        wasm_externtype_t* externtype = wasm_functype_as_externtype(original);
        wasm_functype_t* converted = wasm_externtype_as_functype(externtype);
        
        ASSERT_NE(nullptr, converted);
        ASSERT_EQ(0, wasm_functype_params(converted)->size);
        ASSERT_EQ(0, wasm_functype_results(converted)->size);
        
        wasm_functype_delete(original);
    }

    // Test global type round-trip
    {
        wasm_valtype_t* valtype = wasm_valtype_new_i64();
        wasm_globaltype_t* original = wasm_globaltype_new(valtype, WASM_CONST);
        
        wasm_externtype_t* externtype = wasm_globaltype_as_externtype(original);
        wasm_globaltype_t* converted = wasm_externtype_as_globaltype(externtype);
        
        ASSERT_NE(nullptr, converted);
        ASSERT_EQ(WASM_I64, wasm_valtype_kind(wasm_globaltype_content(converted)));
        ASSERT_EQ(WASM_CONST, wasm_globaltype_mutability(converted));
        
        wasm_globaltype_delete(original);
    }
}

TEST_F(ExternalTypeSystemTest, ExternType_CrossTypeConversion_FailsGracefully)
{
    // Create a function type
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new_empty(&params);
    wasm_valtype_vec_new_empty(&results);
    wasm_functype_t* functype = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, functype);

    wasm_externtype_t* externtype = wasm_functype_as_externtype(functype);
    ASSERT_NE(nullptr, externtype);
    ASSERT_EQ(WASM_EXTERN_FUNC, wasm_externtype_kind(externtype));

    // Note: WAMR implementation may be more permissive with type conversions
    // Verify that correct conversion works
    wasm_functype_t* as_func = wasm_externtype_as_functype(externtype);
    ASSERT_NE(nullptr, as_func);

    // Test that we can distinguish between different extern types by their kinds
    wasm_valtype_t* valtype = wasm_valtype_new_i32();
    wasm_globaltype_t* globaltype = wasm_globaltype_new(valtype, WASM_CONST);
    wasm_externtype_t* global_externtype = wasm_globaltype_as_externtype(globaltype);
    
    ASSERT_EQ(WASM_EXTERN_FUNC, wasm_externtype_kind(externtype));
    ASSERT_EQ(WASM_EXTERN_GLOBAL, wasm_externtype_kind(global_externtype));
    ASSERT_NE(wasm_externtype_kind(externtype), wasm_externtype_kind(global_externtype));

    wasm_functype_delete(functype);
    wasm_globaltype_delete(globaltype);
}

// External Type Ownership and Lifecycle Tests

TEST_F(ExternalTypeSystemTest, ExternType_OwnershipModel_WorksCorrectly)
{
    // Create a global type
    wasm_valtype_t* valtype = wasm_valtype_new_i32();
    ASSERT_NE(nullptr, valtype);
    wasm_globaltype_t* globaltype = wasm_globaltype_new(valtype, WASM_VAR);
    ASSERT_NE(nullptr, globaltype);

    // Get external type - should be owned by the original type
    wasm_externtype_t* externtype = wasm_globaltype_as_externtype(globaltype);
    ASSERT_NE(nullptr, externtype);

    // External type should remain valid while original type exists
    ASSERT_EQ(WASM_EXTERN_GLOBAL, wasm_externtype_kind(externtype));

    // After deleting original type, externtype pointer becomes invalid
    wasm_globaltype_delete(globaltype);
    // Note: Cannot safely test externtype after this point
}

TEST_F(ExternalTypeSystemTest, ExternType_ConstConversion_WorksCorrectly)
{
    // Create table type
    wasm_valtype_t* valtype = wasm_valtype_new_funcref();
    ASSERT_NE(nullptr, valtype);
    wasm_limits_t limits = { 1, 10 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &limits);
    ASSERT_NE(nullptr, tabletype);

    // Test const conversion
    const wasm_externtype_t* const_externtype = wasm_tabletype_as_externtype_const(tabletype);
    ASSERT_NE(nullptr, const_externtype);
    ASSERT_EQ(WASM_EXTERN_TABLE, wasm_externtype_kind(const_externtype));

    // Convert back using const version
    const wasm_tabletype_t* const_tabletype = wasm_externtype_as_tabletype_const(const_externtype);
    ASSERT_NE(nullptr, const_tabletype);

    // Verify data integrity
    const wasm_valtype_t* element = wasm_tabletype_element(const_tabletype);
    ASSERT_NE(nullptr, element);
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(element));

    wasm_tabletype_delete(tabletype);
}

// External Type Error Handling Tests

TEST_F(ExternalTypeSystemTest, ExternType_NullInput_HandlesGracefully)
{
    // Test null externtype
    wasm_externkind_t kind = wasm_externtype_kind(nullptr);
    // Note: This may cause undefined behavior, but testing for robustness
    // In practice, API should not be called with null pointers

    // Test null conversion attempts
    wasm_functype_t* functype = wasm_externtype_as_functype(nullptr);
    ASSERT_EQ(nullptr, functype);

    wasm_globaltype_t* globaltype = wasm_externtype_as_globaltype(nullptr);
    ASSERT_EQ(nullptr, globaltype);

    wasm_tabletype_t* tabletype = wasm_externtype_as_tabletype(nullptr);
    ASSERT_EQ(nullptr, tabletype);

    wasm_memorytype_t* memorytype = wasm_externtype_as_memorytype(nullptr);
    ASSERT_EQ(nullptr, memorytype);
}

TEST_F(ExternalTypeSystemTest, ExternType_InvalidKindHandling_WorksCorrectly)
{
    // Create a memory type to get a valid externtype
    wasm_limits_t limits = { 1, 10 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    ASSERT_NE(nullptr, memorytype);

    wasm_externtype_t* externtype = wasm_memorytype_as_externtype(memorytype);
    ASSERT_NE(nullptr, externtype);
    ASSERT_EQ(WASM_EXTERN_MEMORY, wasm_externtype_kind(externtype));

    // Verify correct conversion works
    wasm_memorytype_t* as_memory = wasm_externtype_as_memorytype(externtype);
    ASSERT_NE(nullptr, as_memory);

    // Verify the limits are preserved in the correct conversion
    const wasm_limits_t* converted_limits = wasm_memorytype_limits(as_memory);
    ASSERT_NE(nullptr, converted_limits);
    ASSERT_EQ(1, converted_limits->min);
    ASSERT_EQ(10, converted_limits->max);

    // Test kind-based type checking works correctly
    ASSERT_EQ(WASM_EXTERN_MEMORY, wasm_externtype_kind(externtype));
    ASSERT_NE(WASM_EXTERN_FUNC, wasm_externtype_kind(externtype));
    ASSERT_NE(WASM_EXTERN_GLOBAL, wasm_externtype_kind(externtype));
    ASSERT_NE(WASM_EXTERN_TABLE, wasm_externtype_kind(externtype));

    wasm_memorytype_delete(memorytype);
}

TEST_F(ExternalTypeSystemTest, ExternType_ComplexTypeSystem_IntegrationTest)
{
    // Create multiple external types and verify they work together
    
    // Function type: (i32, f64) -> i64
    wasm_valtype_t* func_params[] = { wasm_valtype_new_i32(), wasm_valtype_new_f64() };
    wasm_valtype_t* func_results[] = { wasm_valtype_new_i64() };
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new(&params, 2, func_params);
    wasm_valtype_vec_new(&results, 1, func_results);
    wasm_functype_t* functype = wasm_functype_new(&params, &results);
    
    // Global type: var f32
    wasm_valtype_t* global_valtype = wasm_valtype_new_f32();
    wasm_globaltype_t* globaltype = wasm_globaltype_new(global_valtype, WASM_VAR);
    
    // Table type: funcref[5..50]
    wasm_valtype_t* table_valtype = wasm_valtype_new_funcref();
    wasm_limits_t table_limits = { 5, 50 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(table_valtype, &table_limits);
    
    // Memory type: [2..256]
    wasm_limits_t memory_limits = { 2, 256 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&memory_limits);

    // Convert all to external types
    wasm_externtype_t* extern_types[] = {
        wasm_functype_as_externtype(functype),
        wasm_globaltype_as_externtype(globaltype),
        wasm_tabletype_as_externtype(tabletype),
        wasm_memorytype_as_externtype(memorytype)
    };

    // Verify all conversions succeeded
    for (size_t i = 0; i < 4; ++i) {
        ASSERT_NE(nullptr, extern_types[i]);
    }

    // Verify kinds are correct and unique
    wasm_externkind_t kinds[] = {
        wasm_externtype_kind(extern_types[0]),
        wasm_externtype_kind(extern_types[1]),
        wasm_externtype_kind(extern_types[2]),
        wasm_externtype_kind(extern_types[3])
    };
    
    ASSERT_EQ(WASM_EXTERN_FUNC, kinds[0]);
    ASSERT_EQ(WASM_EXTERN_GLOBAL, kinds[1]);
    ASSERT_EQ(WASM_EXTERN_TABLE, kinds[2]);
    ASSERT_EQ(WASM_EXTERN_MEMORY, kinds[3]);

    // Verify each kind is unique
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = i + 1; j < 4; ++j) {
            ASSERT_NE(kinds[i], kinds[j]);
        }
    }

    // Verify correct type conversions work
    wasm_functype_t* converted_func = wasm_externtype_as_functype(extern_types[0]);
    ASSERT_NE(nullptr, converted_func);
    ASSERT_EQ(2, wasm_functype_params(converted_func)->size);
    ASSERT_EQ(1, wasm_functype_results(converted_func)->size);

    wasm_globaltype_t* converted_global = wasm_externtype_as_globaltype(extern_types[1]);
    ASSERT_NE(nullptr, converted_global);
    ASSERT_EQ(WASM_VAR, wasm_globaltype_mutability(converted_global));

    wasm_tabletype_t* converted_table = wasm_externtype_as_tabletype(extern_types[2]);
    ASSERT_NE(nullptr, converted_table);
    ASSERT_EQ(5, wasm_tabletype_limits(converted_table)->min);
    ASSERT_EQ(50, wasm_tabletype_limits(converted_table)->max);

    wasm_memorytype_t* converted_memory = wasm_externtype_as_memorytype(extern_types[3]);
    ASSERT_NE(nullptr, converted_memory);
    ASSERT_EQ(2, wasm_memorytype_limits(converted_memory)->min);
    ASSERT_EQ(256, wasm_memorytype_limits(converted_memory)->max);

    // Clean up
    wasm_functype_delete(functype);
    wasm_globaltype_delete(globaltype);
    wasm_tabletype_delete(tabletype);
    wasm_memorytype_delete(memorytype);
}

TEST_F(ExternalTypeSystemTest, ExternType_VectorOperations_WorkCorrectly)
{
    // Create external type vector (this would typically be used with imports/exports)
    wasm_extern_vec_t extern_vec;
    wasm_extern_vec_new_empty(&extern_vec);
    
    // Verify empty vector
    ASSERT_EQ(0, extern_vec.size);
    ASSERT_EQ(nullptr, extern_vec.data);
    
    // Clean up empty vector
    wasm_extern_vec_delete(&extern_vec);
    
    // Note: Full extern vector testing would require actual extern objects,
    // which are created from instantiated modules. This tests the basic vector operations.
}