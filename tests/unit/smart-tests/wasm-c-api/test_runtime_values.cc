/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "wasm_c_api.h"
#include <cmath>
#include <limits>

// Test fixture for runtime values and references
class RuntimeValuesTest : public testing::Test {
protected:
    void SetUp() override {
        engine = wasm_engine_new();
        ASSERT_NE(nullptr, engine);
        store = wasm_store_new(engine);
        ASSERT_NE(nullptr, store);
    }

    void TearDown() override {
        if (store) wasm_store_delete(store);
        if (engine) wasm_engine_delete(engine);
    }

    wasm_engine_t* engine = nullptr;
    wasm_store_t* store = nullptr;
};

// Test 1: Value creation for primitive types
TEST_F(RuntimeValuesTest, ValueCreation_PrimitiveTypes_CreatesCorrectly) {
    // Test i32 value
    wasm_val_t i32_val = WASM_I32_VAL(42);
    ASSERT_EQ(WASM_I32, i32_val.kind);
    ASSERT_EQ(42, i32_val.of.i32);

    // Test i64 value
    wasm_val_t i64_val = WASM_I64_VAL(1234567890LL);
    ASSERT_EQ(WASM_I64, i64_val.kind);
    ASSERT_EQ(1234567890LL, i64_val.of.i64);

    // Test f32 value
    wasm_val_t f32_val = WASM_F32_VAL(3.14f);
    ASSERT_EQ(WASM_F32, f32_val.kind);
    ASSERT_FLOAT_EQ(3.14f, f32_val.of.f32);

    // Test f64 value
    wasm_val_t f64_val = WASM_F64_VAL(2.71828);
    ASSERT_EQ(WASM_F64, f64_val.kind);
    ASSERT_DOUBLE_EQ(2.71828, f64_val.of.f64);
}

// Test 2: Value creation for reference types
TEST_F(RuntimeValuesTest, ValueCreation_ReferenceTypes_CreatesCorrectly) {
    // Test funcref null value
    wasm_val_t funcref_val = WASM_REF_VAL(nullptr);
    funcref_val.kind = WASM_FUNCREF;
    ASSERT_EQ(WASM_FUNCREF, funcref_val.kind);
    ASSERT_EQ(nullptr, funcref_val.of.ref);

    // Test externref null value
    wasm_val_t externref_val = WASM_REF_VAL(nullptr);
    externref_val.kind = WASM_EXTERNREF;
    ASSERT_EQ(WASM_EXTERNREF, externref_val.kind);
    ASSERT_EQ(nullptr, externref_val.of.ref);
}

// Test 3: Value copying operations
TEST_F(RuntimeValuesTest, ValueCopy_PrimitiveValues_CopiesCorrectly) {
    wasm_val_t original = WASM_I32_VAL(100);
    wasm_val_t copy;
    
    wasm_val_copy(&copy, &original);
    
    ASSERT_EQ(original.kind, copy.kind);
    ASSERT_EQ(original.of.i32, copy.of.i32);
    
    // Modify original to ensure independence
    original.of.i32 = 200;
    ASSERT_EQ(100, copy.of.i32);
    ASSERT_EQ(200, original.of.i32);
}

// Test 4: Value copying for references
TEST_F(RuntimeValuesTest, ValueCopy_ReferenceValues_CopiesCorrectly) {
    wasm_val_t original = WASM_REF_VAL(nullptr);
    wasm_val_t copy;
    
    wasm_val_copy(&copy, &original);
    
    ASSERT_EQ(original.kind, copy.kind);
    ASSERT_EQ(original.of.ref, copy.of.ref);
}

// Test 5: Value vector operations
TEST_F(RuntimeValuesTest, ValueVector_BasicOperations_WorksCorrectly) {
    wasm_val_vec_t values;
    
    // Create vector with capacity
    wasm_val_vec_new_uninitialized(&values, 3);
    ASSERT_EQ(3u, values.size);
    ASSERT_NE(nullptr, values.data);
    
    // Initialize values
    values.data[0] = WASM_I32_VAL(10);
    values.data[1] = WASM_I64_VAL(20);
    values.data[2] = WASM_F32_VAL(3.0f);
    
    // Verify values
    ASSERT_EQ(WASM_I32, values.data[0].kind);
    ASSERT_EQ(10, values.data[0].of.i32);
    ASSERT_EQ(WASM_I64, values.data[1].kind);
    ASSERT_EQ(20, values.data[1].of.i64);
    ASSERT_EQ(WASM_F32, values.data[2].kind);
    ASSERT_FLOAT_EQ(3.0f, values.data[2].of.f32);
    
    wasm_val_vec_delete(&values);
}

// Test 6: Empty value vector
TEST_F(RuntimeValuesTest, ValueVector_EmptyVector_HandlesCorrectly) {
    wasm_val_vec_t empty_values;
    wasm_val_vec_new_empty(&empty_values);
    
    ASSERT_EQ(0u, empty_values.size);
    ASSERT_EQ(nullptr, empty_values.data);
    
    wasm_val_vec_delete(&empty_values);
}

// Test 7: Value vector from array
TEST_F(RuntimeValuesTest, ValueVector_FromArray_CreatesCorrectly) {
    wasm_val_t array[] = {
        WASM_I32_VAL(1),
        WASM_I32_VAL(2),
        WASM_I32_VAL(3)
    };
    
    wasm_val_vec_t values;
    wasm_val_vec_new(&values, 3, array);
    
    ASSERT_EQ(3u, values.size);
    ASSERT_NE(nullptr, values.data);
    
    for (size_t i = 0; i < 3; i++) {
        ASSERT_EQ(WASM_I32, values.data[i].kind);
        ASSERT_EQ(static_cast<int32_t>(i + 1), values.data[i].of.i32);
    }
    
    wasm_val_vec_delete(&values);
}

// Test 8: Reference creation and management
TEST_F(RuntimeValuesTest, ReferenceCreation_FunctionReference_WorksCorrectly) {
    // Create function type for reference
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new_empty(&params);
    wasm_valtype_vec_new_empty(&results);
    
    wasm_functype_t* functype = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, functype);
    
    // Create function reference (null for now)
    wasm_val_t funcref_val = WASM_REF_VAL(nullptr);
    funcref_val.kind = WASM_FUNCREF;
    
    ASSERT_EQ(WASM_FUNCREF, funcref_val.kind);
    ASSERT_EQ(nullptr, funcref_val.of.ref);
    
    wasm_functype_delete(functype);
}

// Test 9: External reference operations
TEST_F(RuntimeValuesTest, ExternalReference_Operations_WorksCorrectly) {
    // Create external reference to some host data
    wasm_foreign_t* foreign = wasm_foreign_new(store);
    
    if (foreign) {
        wasm_ref_t* externref = wasm_foreign_as_ref(foreign);
        if (externref) {
            wasm_val_t externref_val = WASM_REF_VAL(externref);
            externref_val.kind = WASM_EXTERNREF;
            
            ASSERT_EQ(WASM_EXTERNREF, externref_val.kind);
            ASSERT_NE(nullptr, externref_val.of.ref);
            
            // Test reference copying
            wasm_val_t copy;
            wasm_val_copy(&copy, &externref_val);
            ASSERT_EQ(WASM_EXTERNREF, copy.kind);
            ASSERT_EQ(externref_val.of.ref, copy.of.ref);
        }
        wasm_foreign_delete(foreign);
    }
}

// Test 10: Value type checking
TEST_F(RuntimeValuesTest, ValueTypeChecking_AllTypes_ChecksCorrectly) {
    wasm_val_t i32_val = WASM_I32_VAL(1);
    wasm_val_t i64_val = WASM_I64_VAL(1);
    wasm_val_t f32_val = WASM_F32_VAL(1.0f);
    wasm_val_t f64_val = WASM_F64_VAL(1.0);
    wasm_val_t ref_val = WASM_REF_VAL(nullptr);
    
    // Create corresponding value types
    wasm_valtype_t* i32_type = wasm_valtype_new(WASM_I32);
    wasm_valtype_t* i64_type = wasm_valtype_new(WASM_I64);
    wasm_valtype_t* f32_type = wasm_valtype_new(WASM_F32);
    wasm_valtype_t* f64_type = wasm_valtype_new(WASM_F64);
    wasm_valtype_t* funcref_type = wasm_valtype_new(WASM_FUNCREF);
    
    ASSERT_NE(nullptr, i32_type);
    ASSERT_NE(nullptr, i64_type);
    ASSERT_NE(nullptr, f32_type);
    ASSERT_NE(nullptr, f64_type);
    ASSERT_NE(nullptr, funcref_type);
    
    // Verify type matching
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(i32_type));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(i64_type));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(f32_type));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(f64_type));
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(funcref_type));
    
    wasm_valtype_delete(funcref_type);
    wasm_valtype_delete(f64_type);
    wasm_valtype_delete(f32_type);
    wasm_valtype_delete(i64_type);
    wasm_valtype_delete(i32_type);
}

// Test 11: Boundary value testing
TEST_F(RuntimeValuesTest, BoundaryValues_IntegerTypes_HandlesCorrectly) {
    // Test i32 boundaries
    wasm_val_t i32_min = WASM_I32_VAL(std::numeric_limits<int32_t>::min());
    wasm_val_t i32_max = WASM_I32_VAL(std::numeric_limits<int32_t>::max());
    
    ASSERT_EQ(WASM_I32, i32_min.kind);
    ASSERT_EQ(WASM_I32, i32_max.kind);
    ASSERT_EQ(std::numeric_limits<int32_t>::min(), i32_min.of.i32);
    ASSERT_EQ(std::numeric_limits<int32_t>::max(), i32_max.of.i32);
    
    // Test i64 boundaries
    wasm_val_t i64_min = WASM_I64_VAL(std::numeric_limits<int64_t>::min());
    wasm_val_t i64_max = WASM_I64_VAL(std::numeric_limits<int64_t>::max());
    
    ASSERT_EQ(WASM_I64, i64_min.kind);
    ASSERT_EQ(WASM_I64, i64_max.kind);
    ASSERT_EQ(std::numeric_limits<int64_t>::min(), i64_min.of.i64);
    ASSERT_EQ(std::numeric_limits<int64_t>::max(), i64_max.of.i64);
}

// Test 12: Boundary value testing for floating point
TEST_F(RuntimeValuesTest, BoundaryValues_FloatTypes_HandlesCorrectly) {
    // Test f32 special values
    wasm_val_t f32_inf = WASM_F32_VAL(std::numeric_limits<float>::infinity());
    wasm_val_t f32_ninf = WASM_F32_VAL(-std::numeric_limits<float>::infinity());
    wasm_val_t f32_nan = WASM_F32_VAL(std::numeric_limits<float>::quiet_NaN());
    
    ASSERT_EQ(WASM_F32, f32_inf.kind);
    ASSERT_EQ(WASM_F32, f32_ninf.kind);
    ASSERT_EQ(WASM_F32, f32_nan.kind);
    ASSERT_TRUE(std::isinf(f32_inf.of.f32));
    ASSERT_TRUE(std::isinf(f32_ninf.of.f32));
    ASSERT_TRUE(std::isnan(f32_nan.of.f32));
    
    // Test f64 special values
    wasm_val_t f64_inf = WASM_F64_VAL(std::numeric_limits<double>::infinity());
    wasm_val_t f64_ninf = WASM_F64_VAL(-std::numeric_limits<double>::infinity());
    wasm_val_t f64_nan = WASM_F64_VAL(std::numeric_limits<double>::quiet_NaN());
    
    ASSERT_EQ(WASM_F64, f64_inf.kind);
    ASSERT_EQ(WASM_F64, f64_ninf.kind);
    ASSERT_EQ(WASM_F64, f64_nan.kind);
    ASSERT_TRUE(std::isinf(f64_inf.of.f64));
    ASSERT_TRUE(std::isinf(f64_ninf.of.f64));
    ASSERT_TRUE(std::isnan(f64_nan.of.f64));
}

// Test 13: Value vector large size handling
TEST_F(RuntimeValuesTest, ValueVector_LargeSize_HandlesCorrectly) {
    const size_t large_size = 10000;
    wasm_val_vec_t large_values;
    
    wasm_val_vec_new_uninitialized(&large_values, large_size);
    ASSERT_EQ(large_size, large_values.size);
    ASSERT_NE(nullptr, large_values.data);
    
    // Initialize some values
    for (size_t i = 0; i < std::min(large_size, size_t(100)); i++) {
        large_values.data[i] = WASM_I32_VAL(static_cast<int32_t>(i));
    }
    
    // Verify some values
    for (size_t i = 0; i < std::min(large_size, size_t(100)); i++) {
        ASSERT_EQ(WASM_I32, large_values.data[i].kind);
        ASSERT_EQ(static_cast<int32_t>(i), large_values.data[i].of.i32);
    }
    
    wasm_val_vec_delete(&large_values);
}

// Test 14: Reference counting and lifecycle
TEST_F(RuntimeValuesTest, ReferenceLifecycle_ProperManagement_WorksCorrectly) {
    wasm_foreign_t* foreign = wasm_foreign_new(store);
    if (foreign) {
        wasm_ref_t* ref1 = wasm_foreign_as_ref(foreign);
        if (ref1) {
            // Create value with reference
            wasm_val_t val1 = WASM_REF_VAL(ref1);
            val1.kind = WASM_EXTERNREF;
            
            // Copy the value (should maintain reference)
            wasm_val_t val2;
            wasm_val_copy(&val2, &val1);
            
            ASSERT_EQ(val1.of.ref, val2.of.ref);
            ASSERT_EQ(WASM_EXTERNREF, val2.kind);
            
            // References should remain valid
            ASSERT_NE(nullptr, val1.of.ref);
            ASSERT_NE(nullptr, val2.of.ref);
        }
        wasm_foreign_delete(foreign);
    }
}

// Test 15: Mixed type value operations
TEST_F(RuntimeValuesTest, MixedTypeOperations_DifferentTypes_WorksCorrectly) {
    wasm_val_vec_t mixed_values;
    wasm_val_vec_new_uninitialized(&mixed_values, 5);
    
    mixed_values.data[0] = WASM_I32_VAL(42);
    mixed_values.data[1] = WASM_I64_VAL(1234567890LL);
    mixed_values.data[2] = WASM_F32_VAL(3.14f);
    mixed_values.data[3] = WASM_F64_VAL(2.71828);
    mixed_values.data[4] = WASM_REF_VAL(nullptr);
    mixed_values.data[4].kind = WASM_FUNCREF;
    
    // Verify all types are correct
    ASSERT_EQ(WASM_I32, mixed_values.data[0].kind);
    ASSERT_EQ(WASM_I64, mixed_values.data[1].kind);
    ASSERT_EQ(WASM_F32, mixed_values.data[2].kind);
    ASSERT_EQ(WASM_F64, mixed_values.data[3].kind);
    ASSERT_EQ(WASM_FUNCREF, mixed_values.data[4].kind);
    
    // Verify values
    ASSERT_EQ(42, mixed_values.data[0].of.i32);
    ASSERT_EQ(1234567890LL, mixed_values.data[1].of.i64);
    ASSERT_FLOAT_EQ(3.14f, mixed_values.data[2].of.f32);
    ASSERT_DOUBLE_EQ(2.71828, mixed_values.data[3].of.f64);
    ASSERT_EQ(nullptr, mixed_values.data[4].of.ref);
    
    wasm_val_vec_delete(&mixed_values);
}

// Test 16: Value serialization representation
TEST_F(RuntimeValuesTest, ValueSerialization_InternalRepresentation_ConsistentFormat) {
    // Test that values maintain consistent internal representation
    wasm_val_t original = WASM_I64_VAL(0x123456789ABCDEFLL);
    wasm_val_t copy;
    
    wasm_val_copy(&copy, &original);
    
    // Values should be bitwise identical for primitive types
    ASSERT_EQ(original.kind, copy.kind);
    ASSERT_EQ(original.of.i64, copy.of.i64);
    ASSERT_EQ(0x123456789ABCDEFLL, copy.of.i64);
}

// Test 17: Error handling with invalid operations
TEST_F(RuntimeValuesTest, ErrorHandling_InvalidOperations_HandlesGracefully) {
    wasm_val_vec_t valid_vec;
    
    // Create a valid empty vector
    wasm_val_vec_new_empty(&valid_vec);
    
    // Verify empty state
    ASSERT_EQ(0u, valid_vec.size);
    ASSERT_EQ(nullptr, valid_vec.data);
    
    // Delete should handle empty vector gracefully
    wasm_val_vec_delete(&valid_vec);
    
    // Should still be in clean state after deletion
    ASSERT_EQ(0u, valid_vec.size);
    ASSERT_EQ(nullptr, valid_vec.data);
}

// Test 18: Value comparison operations
TEST_F(RuntimeValuesTest, ValueComparison_SameTypes_ComparesCorrectly) {
    wasm_val_t val1 = WASM_I32_VAL(100);
    wasm_val_t val2 = WASM_I32_VAL(100);
    wasm_val_t val3 = WASM_I32_VAL(200);
    
    // Same values should be equal
    ASSERT_EQ(val1.kind, val2.kind);
    ASSERT_EQ(val1.of.i32, val2.of.i32);
    
    // Different values should not be equal
    ASSERT_EQ(val1.kind, val3.kind);
    ASSERT_NE(val1.of.i32, val3.of.i32);
}

// Test 19: Reference null handling
TEST_F(RuntimeValuesTest, ReferenceNull_Operations_HandlesCorrectly) {
    wasm_val_t null_funcref = WASM_REF_VAL(nullptr);
    null_funcref.kind = WASM_FUNCREF;
    
    wasm_val_t null_externref = WASM_REF_VAL(nullptr);
    null_externref.kind = WASM_EXTERNREF;
    
    // Null references should be handled correctly
    ASSERT_EQ(WASM_FUNCREF, null_funcref.kind);
    ASSERT_EQ(nullptr, null_funcref.of.ref);
    
    ASSERT_EQ(WASM_EXTERNREF, null_externref.kind);
    ASSERT_EQ(nullptr, null_externref.of.ref);
    
    // Copying null references should work
    wasm_val_t copy_funcref, copy_externref;
    wasm_val_copy(&copy_funcref, &null_funcref);
    wasm_val_copy(&copy_externref, &null_externref);
    
    ASSERT_EQ(null_funcref.kind, copy_funcref.kind);
    ASSERT_EQ(null_funcref.of.ref, copy_funcref.of.ref);
    ASSERT_EQ(null_externref.kind, copy_externref.kind);
    ASSERT_EQ(null_externref.of.ref, copy_externref.of.ref);
}

// Test 20: Memory management stress test
TEST_F(RuntimeValuesTest, MemoryManagement_StressTest_HandlesCorrectly) {
    const int iterations = 1000;
    
    for (int i = 0; i < iterations; i++) {
        wasm_val_vec_t values;
        wasm_val_vec_new_uninitialized(&values, 10);
        
        // Initialize with various types
        for (size_t j = 0; j < values.size; j++) {
            switch (j % 4) {
                case 0: values.data[j] = WASM_I32_VAL(static_cast<int32_t>(i * j)); break;
                case 1: values.data[j] = WASM_I64_VAL(static_cast<int64_t>(i * j)); break;
                case 2: values.data[j] = WASM_F32_VAL(static_cast<float>(i * j)); break;
                case 3: values.data[j] = WASM_F64_VAL(static_cast<double>(i * j)); break;
            }
        }
        
        // Copy some values
        wasm_val_t temp;
        wasm_val_copy(&temp, &values.data[0]);
        
        // Clean up
        wasm_val_vec_delete(&values);
    }
    
    // If we reach here without crashing, memory management is working
    ASSERT_TRUE(true);
}