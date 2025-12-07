/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <vector>
#include <memory>

#include "bh_platform.h"
#include "wasm_c_api.h"
#include "wasm_c_api_internal.h"

#ifndef own
#define own
#endif

class ValueTypeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        bh_log_set_verbose_level(5);
        engine = wasm_engine_new();
        ASSERT_NE(nullptr, engine);
        store = wasm_store_new(engine);
        ASSERT_NE(nullptr, store);
    }

    void TearDown() override
    {
        if (store) wasm_store_delete(store);
        if (engine) wasm_engine_delete(engine);
    }

    wasm_engine_t* engine = nullptr;
    wasm_store_t* store = nullptr;
};

// Primitive Type Creation Tests
TEST_F(ValueTypeTest, PrimitiveTypes_AllKinds_CreateCorrectly)
{
    wasm_valtype_t* i32_type = wasm_valtype_new_i32();
    wasm_valtype_t* i64_type = wasm_valtype_new_i64();
    wasm_valtype_t* f32_type = wasm_valtype_new_f32();
    wasm_valtype_t* f64_type = wasm_valtype_new_f64();
    
    ASSERT_NE(nullptr, i32_type);
    ASSERT_NE(nullptr, i64_type);
    ASSERT_NE(nullptr, f32_type);
    ASSERT_NE(nullptr, f64_type);
    
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(i32_type));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(i64_type));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(f32_type));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(f64_type));
    
    wasm_valtype_delete(i32_type);
    wasm_valtype_delete(i64_type);
    wasm_valtype_delete(f32_type);
    wasm_valtype_delete(f64_type);
}

TEST_F(ValueTypeTest, ReferenceTypes_Validation_WorksCorrectly)
{
    wasm_valtype_t* funcref_type = wasm_valtype_new_funcref();
    
    ASSERT_NE(nullptr, funcref_type);
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(funcref_type));
    
    wasm_valtype_delete(funcref_type);
    
    // Test anyref if available
    wasm_valtype_t* anyref_type = wasm_valtype_new_anyref();
    if (anyref_type != nullptr) {
        ASSERT_EQ(WASM_EXTERNREF, wasm_valtype_kind(anyref_type));
        wasm_valtype_delete(anyref_type);
    }
}

TEST_F(ValueTypeTest, TypeComparison_SameTypes_ReturnsTrue)
{
    wasm_valtype_t* i32_type1 = wasm_valtype_new_i32();
    wasm_valtype_t* i32_type2 = wasm_valtype_new_i32();
    wasm_valtype_t* f32_type = wasm_valtype_new_f32();
    
    // Same kind should have same kind value
    ASSERT_EQ(wasm_valtype_kind(i32_type1), wasm_valtype_kind(i32_type2));
    ASSERT_NE(wasm_valtype_kind(i32_type1), wasm_valtype_kind(f32_type));
    
    wasm_valtype_delete(i32_type1);
    wasm_valtype_delete(i32_type2);
    wasm_valtype_delete(f32_type);
}

TEST_F(ValueTypeTest, TypeCopy_CreatesIndependentCopy)
{
    wasm_valtype_t* original = wasm_valtype_new_i64();
    wasm_valtype_t* copy = wasm_valtype_copy(original);
    
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy); // Different objects
    ASSERT_EQ(wasm_valtype_kind(original), wasm_valtype_kind(copy));
    
    wasm_valtype_delete(original);
    wasm_valtype_delete(copy);
}

// Generic Type Creation Tests
TEST_F(ValueTypeTest, GenericTypeCreation_AllKinds_WorksCorrectly)
{
    wasm_valtype_t* i32_generic = wasm_valtype_new(WASM_I32);
    wasm_valtype_t* i64_generic = wasm_valtype_new(WASM_I64);
    wasm_valtype_t* f32_generic = wasm_valtype_new(WASM_F32);
    wasm_valtype_t* f64_generic = wasm_valtype_new(WASM_F64);
    wasm_valtype_t* funcref_generic = wasm_valtype_new(WASM_FUNCREF);
    
    ASSERT_NE(nullptr, i32_generic);
    ASSERT_NE(nullptr, i64_generic);
    ASSERT_NE(nullptr, f32_generic);
    ASSERT_NE(nullptr, f64_generic);
    ASSERT_NE(nullptr, funcref_generic);
    
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(i32_generic));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(i64_generic));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(f32_generic));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(f64_generic));
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(funcref_generic));
    
    wasm_valtype_delete(i32_generic);
    wasm_valtype_delete(i64_generic);
    wasm_valtype_delete(f32_generic);
    wasm_valtype_delete(f64_generic);
    wasm_valtype_delete(funcref_generic);
    
    // Test externref if available
    wasm_valtype_t* externref_generic = wasm_valtype_new(WASM_EXTERNREF);
    if (externref_generic != nullptr) {
        ASSERT_EQ(WASM_EXTERNREF, wasm_valtype_kind(externref_generic));
        wasm_valtype_delete(externref_generic);
    }
}

// Value Creation and Manipulation Tests
TEST_F(ValueTypeTest, Value_I32Operations_WorkCorrectly)
{
    wasm_val_t val = { 0 };
    val.kind = WASM_I32;
    val.of.i32 = 42;
    
    ASSERT_EQ(WASM_I32, val.kind);
    ASSERT_EQ(42, val.of.i32);
    
    wasm_val_t copy = { 0 };
    wasm_val_copy(&copy, &val);
    
    ASSERT_EQ(val.kind, copy.kind);
    ASSERT_EQ(val.of.i32, copy.of.i32);
    
    // No need to delete stack-allocated values
}

TEST_F(ValueTypeTest, Value_I64Operations_WorkCorrectly)
{
    wasm_val_t val = { 0 };
    val.kind = WASM_I64;
    val.of.i64 = 0x123456789ABCDEF0LL;
    
    ASSERT_EQ(WASM_I64, val.kind);
    ASSERT_EQ(0x123456789ABCDEF0LL, val.of.i64);
    
    wasm_val_t copy = { 0 };
    wasm_val_copy(&copy, &val);
    
    ASSERT_EQ(val.kind, copy.kind);
    ASSERT_EQ(val.of.i64, copy.of.i64);
}

TEST_F(ValueTypeTest, Value_F32Operations_WorkCorrectly)
{
    wasm_val_t val = { 0 };
    val.kind = WASM_F32;
    val.of.f32 = 3.14159f;
    
    ASSERT_EQ(WASM_F32, val.kind);
    ASSERT_FLOAT_EQ(3.14159f, val.of.f32);
    
    wasm_val_t copy = { 0 };
    wasm_val_copy(&copy, &val);
    
    ASSERT_EQ(val.kind, copy.kind);
    ASSERT_FLOAT_EQ(val.of.f32, copy.of.f32);
}

TEST_F(ValueTypeTest, Value_F64Operations_WorkCorrectly)
{
    wasm_val_t val = { 0 };
    val.kind = WASM_F64;
    val.of.f64 = 2.718281828459045;
    
    ASSERT_EQ(WASM_F64, val.kind);
    ASSERT_DOUBLE_EQ(2.718281828459045, val.of.f64);
    
    wasm_val_t copy = { 0 };
    wasm_val_copy(&copy, &val);
    
    ASSERT_EQ(val.kind, copy.kind);
    ASSERT_DOUBLE_EQ(val.of.f64, copy.of.f64);
}

TEST_F(ValueTypeTest, Value_ReferenceOperations_WorkCorrectly)
{
    wasm_val_t val = { 0 };
    val.kind = WASM_FUNCREF;
    val.of.ref = nullptr; // Null reference
    
    ASSERT_EQ(WASM_FUNCREF, val.kind);
    ASSERT_EQ(nullptr, val.of.ref);
    
    wasm_val_t copy = { 0 };
    wasm_val_copy(&copy, &val);
    
    ASSERT_EQ(val.kind, copy.kind);
    ASSERT_EQ(val.of.ref, copy.of.ref);
}

// Value Vector Operations
TEST_F(ValueTypeTest, ValueVector_MultipleValues_HandlesCorrectly)
{
    wasm_val_t values[4] = { 0 };
    
    // Set up different value types
    values[0].kind = WASM_I32;
    values[0].of.i32 = 100;
    
    values[1].kind = WASM_I64;
    values[1].of.i64 = 200LL;
    
    values[2].kind = WASM_F32;
    values[2].of.f32 = 3.5f;
    
    values[3].kind = WASM_F64;
    values[3].of.f64 = 4.5;
    
    wasm_val_vec_t vec = { 0 };
    wasm_val_vec_new(&vec, 4, values);
    
    ASSERT_EQ(4, vec.size);
    ASSERT_NE(nullptr, vec.data);
    
    // Verify values were copied correctly
    ASSERT_EQ(WASM_I32, vec.data[0].kind);
    ASSERT_EQ(100, vec.data[0].of.i32);
    
    ASSERT_EQ(WASM_I64, vec.data[1].kind);
    ASSERT_EQ(200LL, vec.data[1].of.i64);
    
    ASSERT_EQ(WASM_F32, vec.data[2].kind);
    ASSERT_FLOAT_EQ(3.5f, vec.data[2].of.f32);
    
    ASSERT_EQ(WASM_F64, vec.data[3].kind);
    ASSERT_DOUBLE_EQ(4.5, vec.data[3].of.f64);
    
    wasm_val_vec_delete(&vec);
}

TEST_F(ValueTypeTest, ValueVector_EmptyVector_HandlesCorrectly)
{
    wasm_val_vec_t vec = { 0 };
    wasm_val_vec_new_empty(&vec);
    
    ASSERT_EQ(0, vec.size);
    ASSERT_EQ(nullptr, vec.data);
    
    wasm_val_vec_delete(&vec);
}

TEST_F(ValueTypeTest, ValueVector_Copy_CreatesIndependentCopy)
{
    wasm_val_t values[2] = { 0 };
    values[0].kind = WASM_I32;
    values[0].of.i32 = 42;
    values[1].kind = WASM_F64;
    values[1].of.f64 = 3.14;
    
    wasm_val_vec_t original = { 0 };
    wasm_val_vec_new(&original, 2, values);
    
    wasm_val_vec_t copy = { 0 };
    wasm_val_vec_copy(&copy, &original);
    
    ASSERT_EQ(original.size, copy.size);
    ASSERT_NE(original.data, copy.data);
    
    // Verify values were copied correctly
    ASSERT_EQ(original.data[0].kind, copy.data[0].kind);
    ASSERT_EQ(original.data[0].of.i32, copy.data[0].of.i32);
    
    ASSERT_EQ(original.data[1].kind, copy.data[1].kind);
    ASSERT_DOUBLE_EQ(original.data[1].of.f64, copy.data[1].of.f64);
    
    wasm_val_vec_delete(&original);
    wasm_val_vec_delete(&copy);
}

// Error Handling Tests
TEST_F(ValueTypeTest, TypeDelete_NullPointer_HandlesGracefully)
{
    // Should not crash
    wasm_valtype_delete(nullptr);
}

TEST_F(ValueTypeTest, ValueDelete_NullPointer_HandlesGracefully)
{
    // Should not crash
    wasm_val_delete(nullptr);
}

TEST_F(ValueTypeTest, ValueCopy_NullSource_HandlesGracefully)
{
    wasm_val_t copy = { 0 };
    
    // Copying from null should not crash
    wasm_val_copy(&copy, nullptr);
    
    // Result should be initialized to zero
    ASSERT_EQ(0, copy.kind);
}

TEST_F(ValueTypeTest, ValueCopy_NullDestination_HandlesGracefully)
{
    wasm_val_t val = { 0 };
    val.kind = WASM_I32;
    val.of.i32 = 42;
    
    // Copying to null should not crash
    wasm_val_copy(nullptr, &val);
}

// Type Validation Tests
TEST_F(ValueTypeTest, TypeKind_AllValidTypes_ReturnsCorrectKind)
{
    wasm_valtype_t* types[] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_i64(),
        wasm_valtype_new_f32(),
        wasm_valtype_new_f64(),
        wasm_valtype_new_funcref()
    };
    
    wasm_valkind_t expected_kinds[] = {
        WASM_I32, WASM_I64, WASM_F32, WASM_F64, WASM_FUNCREF
    };
    
    for (size_t i = 0; i < 5; ++i) {
        ASSERT_NE(nullptr, types[i]);
        ASSERT_EQ(expected_kinds[i], wasm_valtype_kind(types[i]));
        wasm_valtype_delete(types[i]);
    }
    
    // Test anyref if available
    wasm_valtype_t* anyref = wasm_valtype_new_anyref();
    if (anyref != nullptr) {
        ASSERT_EQ(WASM_EXTERNREF, wasm_valtype_kind(anyref));
        wasm_valtype_delete(anyref);
    }
}

TEST_F(ValueTypeTest, TypeKind_NullType_HandlesGracefully)
{
    // Should not crash, may return undefined value
    wasm_valkind_t kind = wasm_valtype_kind(nullptr);
    (void)kind; // Suppress unused variable warning
}

// Boundary Value Tests
TEST_F(ValueTypeTest, Value_BoundaryValues_HandlesCorrectly)
{
    // Test boundary values for different types
    wasm_val_t vals[8] = { 0 };
    
    // I32 boundaries
    vals[0].kind = WASM_I32;
    vals[0].of.i32 = INT32_MAX;
    vals[1].kind = WASM_I32;
    vals[1].of.i32 = INT32_MIN;
    
    // I64 boundaries
    vals[2].kind = WASM_I64;
    vals[2].of.i64 = INT64_MAX;
    vals[3].kind = WASM_I64;
    vals[3].of.i64 = INT64_MIN;
    
    // F32 boundaries
    vals[4].kind = WASM_F32;
    vals[4].of.f32 = FLT_MAX;
    vals[5].kind = WASM_F32;
    vals[5].of.f32 = -FLT_MAX;
    
    // F64 boundaries
    vals[6].kind = WASM_F64;
    vals[6].of.f64 = DBL_MAX;
    vals[7].kind = WASM_F64;
    vals[7].of.f64 = -DBL_MAX;
    
    // Verify all values are handled correctly
    for (int i = 0; i < 8; ++i) {
        wasm_val_t copy = { 0 };
        wasm_val_copy(&copy, &vals[i]);
        
        ASSERT_EQ(vals[i].kind, copy.kind);
        
        switch (vals[i].kind) {
            case WASM_I32:
                ASSERT_EQ(vals[i].of.i32, copy.of.i32);
                break;
            case WASM_I64:
                ASSERT_EQ(vals[i].of.i64, copy.of.i64);
                break;
            case WASM_F32:
                ASSERT_FLOAT_EQ(vals[i].of.f32, copy.of.f32);
                break;
            case WASM_F64:
                ASSERT_DOUBLE_EQ(vals[i].of.f64, copy.of.f64);
                break;
            default:
                break;
        }
        

    }
}

TEST_F(ValueTypeTest, MultipleTypeOperations_Sequential_WorksCorrectly)
{
    // Test multiple type operations in sequence
    for (int i = 0; i < 10; ++i) {
        wasm_valtype_t* type = wasm_valtype_new_i32();
        ASSERT_NE(nullptr, type);
        ASSERT_EQ(WASM_I32, wasm_valtype_kind(type));
        
        wasm_valtype_t* copy = wasm_valtype_copy(type);
        ASSERT_NE(nullptr, copy);
        ASSERT_EQ(WASM_I32, wasm_valtype_kind(copy));
        
        wasm_valtype_delete(type);
        wasm_valtype_delete(copy);
    }
}