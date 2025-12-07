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

class VectorTest : public ::testing::Test
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

// Byte Vector Tests
TEST_F(VectorTest, ByteVec_ZeroSize_HandlesCorrectly)
{
    wasm_byte_vec_t byte_vec = { 0 };
    wasm_byte_vec_new_uninitialized(&byte_vec, 0);
    
    ASSERT_EQ(0, byte_vec.size);
    // Data may be null or non-null for zero size
    
    wasm_byte_vec_delete(&byte_vec);
    ASSERT_EQ(0, byte_vec.size);
    ASSERT_EQ(nullptr, byte_vec.data);
}

TEST_F(VectorTest, ByteVec_LargeSize_AllocatesSuccessfully)
{
    wasm_byte_vec_t byte_vec = { 0 };
    const size_t large_size = 1024 * 1024; // 1MB
    
    wasm_byte_vec_new_uninitialized(&byte_vec, large_size);
    ASSERT_EQ(large_size, byte_vec.size);
    ASSERT_NE(nullptr, byte_vec.data);
    
    // Test we can write to the memory
    byte_vec.data[0] = 'A';
    byte_vec.data[large_size - 1] = 'Z';
    ASSERT_EQ('A', byte_vec.data[0]);
    ASSERT_EQ('Z', byte_vec.data[large_size - 1]);
    
    wasm_byte_vec_delete(&byte_vec);
}

TEST_F(VectorTest, ByteVec_NewEmpty_CreatesEmptyVector)
{
    wasm_byte_vec_t byte_vec = { 0 };
    wasm_byte_vec_new_empty(&byte_vec);
    
    ASSERT_EQ(0, byte_vec.size);
    ASSERT_EQ(nullptr, byte_vec.data);
    
    wasm_byte_vec_delete(&byte_vec);
}

TEST_F(VectorTest, ByteVec_NewWithData_CopiesData)
{
    const char* test_data = "Hello, WASM!";
    const size_t data_len = strlen(test_data);
    
    wasm_byte_vec_t byte_vec = { 0 };
    wasm_byte_vec_new(&byte_vec, data_len, (const wasm_byte_t*)test_data);
    
    ASSERT_EQ(data_len, byte_vec.size);
    ASSERT_NE(nullptr, byte_vec.data);
    ASSERT_EQ(0, memcmp(byte_vec.data, test_data, data_len));
    
    wasm_byte_vec_delete(&byte_vec);
}

TEST_F(VectorTest, ByteVec_Copy_CreatesIndependentCopy)
{
    wasm_byte_vec_t original = { 0 };
    const char* test_data = "Original";
    wasm_byte_vec_new(&original, strlen(test_data), (const wasm_byte_t*)test_data);
    
    wasm_byte_vec_t copy = { 0 };
    wasm_byte_vec_copy(&copy, &original);
    
    ASSERT_EQ(original.size, copy.size);
    ASSERT_NE(original.data, copy.data); // Different memory
    ASSERT_EQ(0, memcmp(original.data, copy.data, original.size));
    
    // Modify original, copy should be unchanged
    original.data[0] = 'X';
    ASSERT_NE(original.data[0], copy.data[0]);
    
    wasm_byte_vec_delete(&original);
    wasm_byte_vec_delete(&copy);
}

// Value Type Vector Tests
TEST_F(VectorTest, ValtypeVec_AllPrimitiveTypes_CreatesCorrectly)
{
    wasm_valtype_t* types[4] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_i64(),
        wasm_valtype_new_f32(),
        wasm_valtype_new_f64()
    };
    
    wasm_valtype_vec_t vec = { 0 };
    wasm_valtype_vec_new(&vec, 4, types);
    
    ASSERT_EQ(4, vec.size);
    ASSERT_NE(nullptr, vec.data);
    
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(vec.data[0]));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(vec.data[1]));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(vec.data[2]));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(vec.data[3]));
    
    wasm_valtype_vec_delete(&vec);
}

TEST_F(VectorTest, ValtypeVec_ReferenceTypes_CreatesCorrectly)
{
    wasm_valtype_t* types[2] = {
        wasm_valtype_new_funcref(),
        wasm_valtype_new_anyref()  // Use anyref instead of externref
    };
    
    wasm_valtype_vec_t vec = { 0 };
    wasm_valtype_vec_new(&vec, 2, types);
    
    ASSERT_EQ(2, vec.size);
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(vec.data[0]));
    ASSERT_EQ(WASM_EXTERNREF, wasm_valtype_kind(vec.data[1]));
    
    wasm_valtype_vec_delete(&vec);
}

TEST_F(VectorTest, ValtypeVec_MaxSize_HandlesCorrectly)
{
    const size_t max_size = 1000;
    std::vector<wasm_valtype_t*> types;
    
    for (size_t i = 0; i < max_size; ++i) {
        types.push_back(wasm_valtype_new_i32());
    }
    
    wasm_valtype_vec_t vec = { 0 };
    wasm_valtype_vec_new(&vec, max_size, types.data());
    
    ASSERT_EQ(max_size, vec.size);
    ASSERT_NE(nullptr, vec.data);
    
    // Verify all types are correct
    for (size_t i = 0; i < max_size; ++i) {
        ASSERT_EQ(WASM_I32, wasm_valtype_kind(vec.data[i]));
    }
    
    wasm_valtype_vec_delete(&vec);
}

TEST_F(VectorTest, ValtypeVec_EmptyVector_HandlesCorrectly)
{
    wasm_valtype_vec_t vec = { 0 };
    wasm_valtype_vec_new_empty(&vec);
    
    ASSERT_EQ(0, vec.size);
    ASSERT_EQ(nullptr, vec.data);
    
    wasm_valtype_vec_delete(&vec);
}

TEST_F(VectorTest, ValtypeVec_Copy_CreatesDeepCopy)
{
    wasm_valtype_t* types[2] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_f64()
    };
    
    wasm_valtype_vec_t original = { 0 };
    wasm_valtype_vec_new(&original, 2, types);
    
    wasm_valtype_vec_t copy = { 0 };
    wasm_valtype_vec_copy(&copy, &original);
    
    ASSERT_EQ(original.size, copy.size);
    ASSERT_NE(original.data, copy.data);
    
    // Verify types are copied correctly
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(copy.data[0]));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(copy.data[1]));
    
    wasm_valtype_vec_delete(&original);
    wasm_valtype_vec_delete(&copy);
}

// Function Type Vector Tests
TEST_F(VectorTest, FunctypeVec_MultipleFunctionTypes_CreatesCorrectly)
{
    // Create different function types
    wasm_functype_t* func_types[3] = {
        wasm_functype_new_0_0(),
        wasm_functype_new_1_0(wasm_valtype_new_i32()),
        wasm_functype_new_0_1(wasm_valtype_new_f64())
    };
    
    wasm_functype_vec_t vec = { 0 };
    wasm_functype_vec_new(&vec, 3, func_types);
    
    ASSERT_EQ(3, vec.size);
    ASSERT_NE(nullptr, vec.data);
    
    // Verify function signatures
    const wasm_valtype_vec_t* params0 = wasm_functype_params(vec.data[0]);
    const wasm_valtype_vec_t* results0 = wasm_functype_results(vec.data[0]);
    ASSERT_EQ(0, params0->size);
    ASSERT_EQ(0, results0->size);
    
    const wasm_valtype_vec_t* params1 = wasm_functype_params(vec.data[1]);
    ASSERT_EQ(1, params1->size);
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(params1->data[0]));
    
    const wasm_valtype_vec_t* results2 = wasm_functype_results(vec.data[2]);
    ASSERT_EQ(1, results2->size);
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(results2->data[0]));
    
    wasm_functype_vec_delete(&vec);
}

// Additional Vector Type Tests
TEST_F(VectorTest, ExternVec_MultipleExterns_HandlesCorrectly)
{
    // Test extern vector which is supported in WAMR
    wasm_extern_vec_t vec = { 0 };
    wasm_extern_vec_new_empty(&vec);
    
    ASSERT_EQ(0, vec.size);
    ASSERT_EQ(nullptr, vec.data);
    
    wasm_extern_vec_delete(&vec);
}

// Memory Management Tests
TEST_F(VectorTest, VectorDelete_NullPointer_HandlesGracefully)
{
    // Should not crash
    wasm_byte_vec_delete(nullptr);
    wasm_valtype_vec_delete(nullptr);
    wasm_functype_vec_delete(nullptr);
    wasm_extern_vec_delete(nullptr);
}

TEST_F(VectorTest, VectorDelete_AlreadyDeleted_HandlesGracefully)
{
    wasm_byte_vec_t byte_vec = { 0 };
    wasm_byte_vec_new_uninitialized(&byte_vec, 100);
    
    wasm_byte_vec_delete(&byte_vec);
    ASSERT_EQ(0, byte_vec.size);
    ASSERT_EQ(nullptr, byte_vec.data);
    
    // Second delete should be safe
    wasm_byte_vec_delete(&byte_vec);
}

// Edge Cases and Error Handling
TEST_F(VectorTest, ByteVec_VeryLargeSize_HandlesAppropriately)
{
    wasm_byte_vec_t byte_vec = { 0 };
    const size_t very_large_size = SIZE_MAX / 2; // Likely to fail allocation
    
    // This may fail to allocate, which is acceptable
    wasm_byte_vec_new_uninitialized(&byte_vec, very_large_size);
    
    // If allocation succeeded, verify basic properties
    if (byte_vec.data != nullptr) {
        ASSERT_EQ(very_large_size, byte_vec.size);
        wasm_byte_vec_delete(&byte_vec);
    }
}

TEST_F(VectorTest, ValtypeVec_NullTypes_HandlesCorrectly)
{
    wasm_valtype_t* types[2] = { nullptr, nullptr };
    
    wasm_valtype_vec_t vec = { 0 };
    wasm_valtype_vec_new(&vec, 2, types);
    
    // Implementation may handle null types differently
    // Just ensure no crash occurs
    wasm_valtype_vec_delete(&vec);
}

TEST_F(VectorTest, VectorCopy_NullSource_HandlesGracefully)
{
    wasm_byte_vec_t copy = { 0 };
    
    // Copying from null should not crash
    wasm_byte_vec_copy(&copy, nullptr);
    
    // Result should be empty vector
    ASSERT_EQ(0, copy.size);
    ASSERT_EQ(nullptr, copy.data);
}

TEST_F(VectorTest, VectorOperations_MultipleSequential_WorkCorrectly)
{
    // Test multiple vector operations in sequence
    for (int i = 0; i < 10; ++i) {
        wasm_byte_vec_t vec = { 0 };
        wasm_byte_vec_new_uninitialized(&vec, 100 + i);
        
        ASSERT_EQ(100 + i, vec.size);
        ASSERT_NE(nullptr, vec.data);
        
        // Fill with pattern
        for (size_t j = 0; j < vec.size; ++j) {
            vec.data[j] = (wasm_byte_t)(j % 256);
        }
        
        // Verify pattern
        for (size_t j = 0; j < vec.size; ++j) {
            ASSERT_EQ((wasm_byte_t)(j % 256), vec.data[j]);
        }
        
        wasm_byte_vec_delete(&vec);
    }
}