/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>

#include "bh_platform.h"
#include "wasm_c_api.h"
#include "wasm_c_api_internal.h"

#ifndef own
#define own
#endif

class FunctionTypeTest : public testing::Test {
protected:
    void SetUp() override {
        bh_log_set_verbose_level(5);
        engine = wasm_engine_new();
        ASSERT_NE(nullptr, engine);
        store = wasm_store_new(engine);
        ASSERT_NE(nullptr, store);
    }
    
    void TearDown() override {
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

// Test 1: Function type with no parameters and no results
TEST_F(FunctionTypeTest, FunctionType_NoParamsNoResults_CreatesCorrectly) {
    wasm_valtype_vec_t params = { 0 };
    wasm_valtype_vec_t results = { 0 };
    wasm_valtype_vec_new_empty(&params);
    wasm_valtype_vec_new_empty(&results);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    
    ASSERT_NE(nullptr, retrieved_params);
    ASSERT_NE(nullptr, retrieved_results);
    ASSERT_EQ(0, retrieved_params->size);
    ASSERT_EQ(0, retrieved_results->size);
    
    wasm_functype_delete(func_type);
}

// Test 2: Function type with one parameter and no results
TEST_F(FunctionTypeTest, FunctionType_OneParamNoResults_CreatesCorrectly) {
    wasm_valtype_t* param_types[1] = { wasm_valtype_new_i32() };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 1, param_types);
    
    wasm_valtype_vec_t results = { 0 };
    wasm_valtype_vec_new_empty(&results);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    
    ASSERT_EQ(1, retrieved_params->size);
    ASSERT_EQ(0, retrieved_results->size);
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_params->data[0]));
    
    wasm_functype_delete(func_type);
}

// Test 3: Function type with no parameters and one result
TEST_F(FunctionTypeTest, FunctionType_NoParamsOneResult_CreatesCorrectly) {
    wasm_valtype_vec_t params = { 0 };
    wasm_valtype_vec_new_empty(&params);
    
    wasm_valtype_t* result_types[1] = { wasm_valtype_new_f64() };
    wasm_valtype_vec_t results;
    wasm_valtype_vec_new(&results, 1, result_types);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    
    ASSERT_EQ(0, retrieved_params->size);
    ASSERT_EQ(1, retrieved_results->size);
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(retrieved_results->data[0]));
    
    wasm_functype_delete(func_type);
}

// Test 4: Function type with one parameter and one result
TEST_F(FunctionTypeTest, FunctionType_OneParamOneResult_CreatesCorrectly) {
    wasm_valtype_t* param_types[1] = { wasm_valtype_new_i64() };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 1, param_types);
    
    wasm_valtype_t* result_types[1] = { wasm_valtype_new_f32() };
    wasm_valtype_vec_t results;
    wasm_valtype_vec_new(&results, 1, result_types);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    
    ASSERT_EQ(1, retrieved_params->size);
    ASSERT_EQ(1, retrieved_results->size);
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(retrieved_params->data[0]));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(retrieved_results->data[0]));
    
    wasm_functype_delete(func_type);
}

// Test 5: Function type with two parameters and two results
TEST_F(FunctionTypeTest, FunctionType_TwoParamsTwoResults_CreatesCorrectly) {
    wasm_valtype_t* param_types[2] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_f32()
    };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 2, param_types);
    
    wasm_valtype_t* result_types[2] = {
        wasm_valtype_new_i64(),
        wasm_valtype_new_f64()
    };
    wasm_valtype_vec_t results;
    wasm_valtype_vec_new(&results, 2, result_types);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    
    ASSERT_EQ(2, retrieved_params->size);
    ASSERT_EQ(2, retrieved_results->size);
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_params->data[0]));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(retrieved_params->data[1]));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(retrieved_results->data[0]));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(retrieved_results->data[1]));
    
    wasm_functype_delete(func_type);
}

// Test 6: Function type with three parameters and three results
TEST_F(FunctionTypeTest, FunctionType_ThreeParamsThreeResults_CreatesCorrectly) {
    // Create parameter types
    wasm_valtype_t* param_types[3] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_i64(),
        wasm_valtype_new_f32()
    };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 3, param_types);
    
    // Create result types
    wasm_valtype_t* result_types[3] = {
        wasm_valtype_new_f64(),
        wasm_valtype_new_funcref(),
        wasm_valtype_new_i32()  // Replace externref with i32
    };
    wasm_valtype_vec_t results;
    wasm_valtype_vec_new(&results, 3, result_types);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* func_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* func_results = wasm_functype_results(func_type);
    
    ASSERT_NE(nullptr, func_params);
    ASSERT_NE(nullptr, func_results);
    ASSERT_EQ(3, func_params->size);
    ASSERT_EQ(3, func_results->size);
    
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(func_params->data[0]));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(func_params->data[1]));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(func_params->data[2]));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(func_results->data[0]));
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(func_results->data[1]));
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(func_results->data[2]));
    
    wasm_functype_delete(func_type);
}

// Test 7: Function type with custom parameter and result vectors
TEST_F(FunctionTypeTest, FunctionType_CustomVectors_CreatesCorrectly) {
    // Create parameter vector
    wasm_valtype_vec_t params = { 0 };
    wasm_valtype_t* param_types[4] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_i64(),
        wasm_valtype_new_f32(),
        wasm_valtype_new_f64()
    };
    wasm_valtype_vec_new(&params, 4, param_types);
    
    // Create result vector
    wasm_valtype_vec_t results = { 0 };
    wasm_valtype_t* result_types[2] = {
        wasm_valtype_new_funcref(),
        wasm_valtype_new_i32()
    };
    wasm_valtype_vec_new(&results, 2, result_types);
    
    // Create function type
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    
    ASSERT_NE(nullptr, retrieved_params);
    ASSERT_NE(nullptr, retrieved_results);
    ASSERT_EQ(4, retrieved_params->size);
    ASSERT_EQ(2, retrieved_results->size);
    
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_params->data[0]));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(retrieved_params->data[1]));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(retrieved_params->data[2]));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(retrieved_params->data[3]));
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(retrieved_results->data[0]));
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_results->data[1]));
    
    wasm_functype_delete(func_type);
}

// Test 8: Function type copy functionality
TEST_F(FunctionTypeTest, FunctionType_Copy_CreatesIndependentCopy) {
    // Create original function type
    wasm_valtype_t* param_types[2] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_f64()
    };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 2, param_types);
    
    wasm_valtype_t* result_types[1] = { wasm_valtype_new_i64() };
    wasm_valtype_vec_t results;
    wasm_valtype_vec_new(&results, 1, result_types);
    
    wasm_functype_t* original = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, original);
    
    wasm_functype_t* copy = wasm_functype_copy(original);
    ASSERT_NE(nullptr, copy);
    
    // Verify both have same signature
    const wasm_valtype_vec_t* orig_params = wasm_functype_params(original);
    const wasm_valtype_vec_t* copy_params = wasm_functype_params(copy);
    const wasm_valtype_vec_t* orig_results = wasm_functype_results(original);
    const wasm_valtype_vec_t* copy_results = wasm_functype_results(copy);
    
    ASSERT_EQ(orig_params->size, copy_params->size);
    ASSERT_EQ(orig_results->size, copy_results->size);
    
    for (size_t i = 0; i < orig_params->size; ++i) {
        ASSERT_EQ(wasm_valtype_kind(orig_params->data[i]),
                  wasm_valtype_kind(copy_params->data[i]));
    }
    
    for (size_t i = 0; i < orig_results->size; ++i) {
        ASSERT_EQ(wasm_valtype_kind(orig_results->data[i]),
                  wasm_valtype_kind(copy_results->data[i]));
    }
    
    wasm_functype_delete(original);
    wasm_functype_delete(copy);
}

// Test 9: Function type with reference types
TEST_F(FunctionTypeTest, FunctionType_ReferenceTypes_HandlesCorrectly) {
    // Create parameters with funcref and i32
    wasm_valtype_t* param_types[2] = {
        wasm_valtype_new_funcref(),
        wasm_valtype_new_i32()
    };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 2, param_types);
    
    // Create results with i32 and funcref (replace externref with i32)
    wasm_valtype_t* result_types[2] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_funcref()
    };
    wasm_valtype_vec_t results;
    wasm_valtype_vec_new(&results, 2, result_types);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(retrieved_params->data[0]));
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_params->data[1]));
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_results->data[0]));
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(retrieved_results->data[1]));
    
    wasm_functype_delete(func_type);
}

// Test 10: Function type with mixed numeric and reference types
TEST_F(FunctionTypeTest, FunctionType_MixedTypes_HandlesCorrectly) {
    // Create parameters with mixed types
    wasm_valtype_t* param_types[3] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_i64(),
        wasm_valtype_new_f32()
    };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 3, param_types);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, nullptr);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    
    ASSERT_EQ(3, retrieved_params->size);
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_params->data[0]));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(retrieved_params->data[1]));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(retrieved_params->data[2]));
    
    wasm_functype_delete(func_type);
}

// Test 11: Function type null handling
TEST_F(FunctionTypeTest, FunctionType_NullHandling_WorksCorrectly) {
    wasm_valtype_vec_t empty_params = { 0 };
    wasm_valtype_vec_t empty_results = { 0 };
    wasm_valtype_vec_new_empty(&empty_params);
    wasm_valtype_vec_new_empty(&empty_results);
    
    wasm_functype_t* func_type = wasm_functype_new(&empty_params, &empty_results);
    ASSERT_NE(nullptr, func_type);
    
    // Test delete on valid function type
    wasm_functype_delete(func_type);
    
    // Test delete on null (should not crash)
    wasm_functype_delete(nullptr);
}

// Test 12: Function type parameter retrieval
TEST_F(FunctionTypeTest, FunctionType_ParameterRetrieval_WorksCorrectly) {
    wasm_valtype_t* param_types[3] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_f64(),
        wasm_valtype_new_funcref()
    };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 3, param_types);
    
    wasm_valtype_vec_t results = { 0 };
    wasm_valtype_vec_new_empty(&results);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    ASSERT_NE(nullptr, retrieved_params);
    ASSERT_EQ(3, retrieved_params->size);
    
    wasm_functype_delete(func_type);
}

// Test 13: Function type result retrieval
TEST_F(FunctionTypeTest, FunctionType_ResultRetrieval_WorksCorrectly) {
    wasm_valtype_vec_t params = { 0 };
    wasm_valtype_vec_new_empty(&params);
    
    wasm_valtype_t* result_types[2] = {
        wasm_valtype_new_i64(),
        wasm_valtype_new_f32()
    };
    wasm_valtype_vec_t results;
    wasm_valtype_vec_new(&results, 2, result_types);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    ASSERT_NE(nullptr, retrieved_results);
    ASSERT_EQ(2, retrieved_results->size);
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(retrieved_results->data[0]));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(retrieved_results->data[1]));
    
    wasm_functype_delete(func_type);
}

// Test 14: Function type complex signatures
TEST_F(FunctionTypeTest, FunctionType_ComplexSignatures_HandlesCorrectly) {
    // Create complex function signature with many parameters and results
    wasm_valtype_t* param_types[6] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_i64(),
        wasm_valtype_new_f32(),
        wasm_valtype_new_f64(),
        wasm_valtype_new_funcref(),
        wasm_valtype_new_i32()
    };
    wasm_valtype_vec_t params;
    wasm_valtype_vec_new(&params, 6, param_types);
    
    wasm_valtype_t* result_types[4] = {
        wasm_valtype_new_i32(),
        wasm_valtype_new_f64(),
        wasm_valtype_new_funcref(),
        wasm_valtype_new_i64()
    };
    wasm_valtype_vec_t results;
    wasm_valtype_vec_new(&results, 4, result_types);
    
    wasm_functype_t* func_type = wasm_functype_new(&params, &results);
    ASSERT_NE(nullptr, func_type);
    
    const wasm_valtype_vec_t* retrieved_params = wasm_functype_params(func_type);
    const wasm_valtype_vec_t* retrieved_results = wasm_functype_results(func_type);
    
    ASSERT_EQ(6, retrieved_params->size);
    ASSERT_EQ(4, retrieved_results->size);
    
    // Verify all parameter types
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_params->data[0]));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(retrieved_params->data[1]));
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(retrieved_params->data[2]));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(retrieved_params->data[3]));
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(retrieved_params->data[4]));
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_params->data[5]));
    
    // Verify all result types
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(retrieved_results->data[0]));
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(retrieved_results->data[1]));
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(retrieved_results->data[2]));
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(retrieved_results->data[3]));
    
    wasm_functype_delete(func_type);
}

// Test 15: Function type memory management
TEST_F(FunctionTypeTest, FunctionType_MemoryManagement_WorksCorrectly) {
    std::vector<wasm_functype_t*> function_types;
    
    // Create multiple function types
    for (int i = 0; i < 10; ++i) {
        wasm_valtype_t* param_types[2] = {
            wasm_valtype_new_i32(),
            wasm_valtype_new_f64()
        };
        wasm_valtype_vec_t params;
        wasm_valtype_vec_new(&params, 2, param_types);
        
        wasm_valtype_t* result_types[1] = { wasm_valtype_new_i64() };
        wasm_valtype_vec_t results;
        wasm_valtype_vec_new(&results, 1, result_types);
        
        wasm_functype_t* func_type = wasm_functype_new(&params, &results);
        ASSERT_NE(nullptr, func_type);
        function_types.push_back(func_type);
    }
    
    // Verify all function types are valid
    for (auto func_type : function_types) {
        const wasm_valtype_vec_t* params = wasm_functype_params(func_type);
        const wasm_valtype_vec_t* results = wasm_functype_results(func_type);
        
        ASSERT_EQ(2, params->size);
        ASSERT_EQ(1, results->size);
    }
    
    // Clean up
    for (auto func_type : function_types) {
        wasm_functype_delete(func_type);
    }
}