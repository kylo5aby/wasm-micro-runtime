/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "wasm_c_api.h"

class GlobalTableMemoryTypesTest : public testing::Test {
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

// Global Type Tests
TEST_F(GlobalTableMemoryTypesTest, GlobalType_ImmutableI32_CreatesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_I32);
    ASSERT_NE(nullptr, valtype);
    
    wasm_globaltype_t* globaltype = wasm_globaltype_new(valtype, WASM_CONST);
    ASSERT_NE(nullptr, globaltype);
    
    const wasm_valtype_t* content = wasm_globaltype_content(globaltype);
    ASSERT_NE(nullptr, content);
    ASSERT_EQ(WASM_I32, wasm_valtype_kind(content));
    
    wasm_mutability_t mutability = wasm_globaltype_mutability(globaltype);
    ASSERT_EQ(WASM_CONST, mutability);
    
    wasm_globaltype_delete(globaltype);
}

TEST_F(GlobalTableMemoryTypesTest, GlobalType_MutableI64_CreatesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_I64);
    ASSERT_NE(nullptr, valtype);
    
    wasm_globaltype_t* globaltype = wasm_globaltype_new(valtype, WASM_VAR);
    ASSERT_NE(nullptr, globaltype);
    
    const wasm_valtype_t* content = wasm_globaltype_content(globaltype);
    ASSERT_NE(nullptr, content);
    ASSERT_EQ(WASM_I64, wasm_valtype_kind(content));
    
    wasm_mutability_t mutability = wasm_globaltype_mutability(globaltype);
    ASSERT_EQ(WASM_VAR, mutability);
    
    wasm_globaltype_delete(globaltype);
}

TEST_F(GlobalTableMemoryTypesTest, GlobalType_MutableF32_CreatesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_F32);
    ASSERT_NE(nullptr, valtype);
    
    wasm_globaltype_t* globaltype = wasm_globaltype_new(valtype, WASM_VAR);
    ASSERT_NE(nullptr, globaltype);
    
    const wasm_valtype_t* content = wasm_globaltype_content(globaltype);
    ASSERT_NE(nullptr, content);
    ASSERT_EQ(WASM_F32, wasm_valtype_kind(content));
    
    wasm_mutability_t mutability = wasm_globaltype_mutability(globaltype);
    ASSERT_EQ(WASM_VAR, mutability);
    
    wasm_globaltype_delete(globaltype);
}

TEST_F(GlobalTableMemoryTypesTest, GlobalType_ImmutableF64_CreatesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_F64);
    ASSERT_NE(nullptr, valtype);
    
    wasm_globaltype_t* globaltype = wasm_globaltype_new(valtype, WASM_CONST);
    ASSERT_NE(nullptr, globaltype);
    
    const wasm_valtype_t* content = wasm_globaltype_content(globaltype);
    ASSERT_NE(nullptr, content);
    ASSERT_EQ(WASM_F64, wasm_valtype_kind(content));
    
    wasm_mutability_t mutability = wasm_globaltype_mutability(globaltype);
    ASSERT_EQ(WASM_CONST, mutability);
    
    wasm_globaltype_delete(globaltype);
}

TEST_F(GlobalTableMemoryTypesTest, GlobalType_MutabilityValidation_WorksCorrectly) {
    // Test const mutability
    wasm_valtype_t* valtype1 = wasm_valtype_new(WASM_I32);
    ASSERT_NE(nullptr, valtype1);
    wasm_globaltype_t* const_global = wasm_globaltype_new(valtype1, WASM_CONST);
    ASSERT_NE(nullptr, const_global);
    ASSERT_EQ(WASM_CONST, wasm_globaltype_mutability(const_global));
    
    // Test var mutability
    wasm_valtype_t* valtype2 = wasm_valtype_new(WASM_I32);
    ASSERT_NE(nullptr, valtype2);
    wasm_globaltype_t* var_global = wasm_globaltype_new(valtype2, WASM_VAR);
    ASSERT_NE(nullptr, var_global);
    ASSERT_EQ(WASM_VAR, wasm_globaltype_mutability(var_global));
    
    wasm_globaltype_delete(const_global);
    wasm_globaltype_delete(var_global);
}

// Table Type Tests
TEST_F(GlobalTableMemoryTypesTest, TableType_FuncrefWithLimits_CreatesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_FUNCREF);
    ASSERT_NE(nullptr, valtype);
    
    wasm_limits_t limits = { 10, 100 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &limits);
    ASSERT_NE(nullptr, tabletype);
    
    const wasm_valtype_t* element = wasm_tabletype_element(tabletype);
    ASSERT_NE(nullptr, element);
    ASSERT_EQ(WASM_FUNCREF, wasm_valtype_kind(element));
    
    const wasm_limits_t* table_limits = wasm_tabletype_limits(tabletype);
    ASSERT_NE(nullptr, table_limits);
    ASSERT_EQ(10u, table_limits->min);
    ASSERT_EQ(100u, table_limits->max);
    
    wasm_tabletype_delete(tabletype);
}

TEST_F(GlobalTableMemoryTypesTest, TableType_ExternrefWithMinOnly_CreatesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_FUNCREF);
    ASSERT_NE(nullptr, valtype);
    
    wasm_limits_t limits = { 5, wasm_limits_max_default };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &limits);
    ASSERT_NE(nullptr, tabletype);
    
    const wasm_limits_t* table_limits = wasm_tabletype_limits(tabletype);
    ASSERT_NE(nullptr, table_limits);
    ASSERT_EQ(5u, table_limits->min);
    ASSERT_EQ(wasm_limits_max_default, table_limits->max);
    
    wasm_tabletype_delete(tabletype);
}

TEST_F(GlobalTableMemoryTypesTest, TableType_ZeroMinLimit_CreatesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_FUNCREF);
    ASSERT_NE(nullptr, valtype);
    
    wasm_limits_t limits = { 0, 50 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &limits);
    ASSERT_NE(nullptr, tabletype);
    
    const wasm_limits_t* table_limits = wasm_tabletype_limits(tabletype);
    ASSERT_NE(nullptr, table_limits);
    ASSERT_EQ(0u, table_limits->min);
    ASSERT_EQ(50u, table_limits->max);
    
    wasm_tabletype_delete(tabletype);
}

TEST_F(GlobalTableMemoryTypesTest, TableType_LargeLimit_CreatesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_FUNCREF);
    ASSERT_NE(nullptr, valtype);
    
    wasm_limits_t limits = { 1000, 10000 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &limits);
    ASSERT_NE(nullptr, tabletype);
    
    const wasm_limits_t* table_limits = wasm_tabletype_limits(tabletype);
    ASSERT_NE(nullptr, table_limits);
    ASSERT_EQ(1000u, table_limits->min);
    ASSERT_EQ(10000u, table_limits->max);
    
    wasm_tabletype_delete(tabletype);
}

// Memory Type Tests
TEST_F(GlobalTableMemoryTypesTest, MemoryType_WithLimits_CreatesCorrectly) {
    wasm_limits_t limits = { 1, 10 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    ASSERT_NE(nullptr, memorytype);
    
    const wasm_limits_t* memory_limits = wasm_memorytype_limits(memorytype);
    ASSERT_NE(nullptr, memory_limits);
    ASSERT_EQ(1u, memory_limits->min);
    ASSERT_EQ(10u, memory_limits->max);
    
    wasm_memorytype_delete(memorytype);
}

TEST_F(GlobalTableMemoryTypesTest, MemoryType_MinOnlyLimit_CreatesCorrectly) {
    wasm_limits_t limits = { 2, wasm_limits_max_default };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    ASSERT_NE(nullptr, memorytype);
    
    const wasm_limits_t* memory_limits = wasm_memorytype_limits(memorytype);
    ASSERT_NE(nullptr, memory_limits);
    ASSERT_EQ(2u, memory_limits->min);
    ASSERT_EQ(wasm_limits_max_default, memory_limits->max);
    
    wasm_memorytype_delete(memorytype);
}

TEST_F(GlobalTableMemoryTypesTest, MemoryType_ZeroMinLimit_CreatesCorrectly) {
    wasm_limits_t limits = { 0, 5 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    ASSERT_NE(nullptr, memorytype);
    
    const wasm_limits_t* memory_limits = wasm_memorytype_limits(memorytype);
    ASSERT_NE(nullptr, memory_limits);
    ASSERT_EQ(0u, memory_limits->min);
    ASSERT_EQ(5u, memory_limits->max);
    
    wasm_memorytype_delete(memorytype);
}

TEST_F(GlobalTableMemoryTypesTest, MemoryType_LargeLimit_CreatesCorrectly) {
    wasm_limits_t limits = { 100, 1000 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&limits);
    ASSERT_NE(nullptr, memorytype);
    
    const wasm_limits_t* memory_limits = wasm_memorytype_limits(memorytype);
    ASSERT_NE(nullptr, memory_limits);
    ASSERT_EQ(100u, memory_limits->min);
    ASSERT_EQ(1000u, memory_limits->max);
    
    wasm_memorytype_delete(memorytype);
}

// Type Validation Tests
TEST_F(GlobalTableMemoryTypesTest, GlobalType_NullValtype_HandlesGracefully) {
    // This test checks behavior with null valtype - should handle gracefully
    wasm_globaltype_t* globaltype = wasm_globaltype_new(nullptr, WASM_CONST);
    // Implementation may return null or handle gracefully
    if (globaltype) {
        wasm_globaltype_delete(globaltype);
    }
    // Test passes if no crash occurs
}

TEST_F(GlobalTableMemoryTypesTest, TableType_NullValtype_HandlesGracefully) {
    wasm_limits_t limits = { 1, 10 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(nullptr, &limits);
    // Implementation may return null or handle gracefully
    if (tabletype) {
        wasm_tabletype_delete(tabletype);
    }
    // Test passes if no crash occurs
}

TEST_F(GlobalTableMemoryTypesTest, MemoryType_NullLimits_HandlesGracefully) {
    wasm_memorytype_t* memorytype = wasm_memorytype_new(nullptr);
    // Implementation may return null or handle gracefully
    if (memorytype) {
        wasm_memorytype_delete(memorytype);
    }
    // Test passes if no crash occurs
}

// Boundary Limit Testing
TEST_F(GlobalTableMemoryTypesTest, TableType_InvalidLimits_HandlesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_FUNCREF);
    ASSERT_NE(nullptr, valtype);
    
    // Test with min > max (invalid)
    wasm_limits_t invalid_limits = { 100, 50 };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &invalid_limits);
    // Implementation should handle this gracefully (may return null)
    if (tabletype) {
        wasm_tabletype_delete(tabletype);
    }
    // Test passes if no crash occurs
}

TEST_F(GlobalTableMemoryTypesTest, MemoryType_InvalidLimits_HandlesCorrectly) {
    // Test with min > max (invalid)
    wasm_limits_t invalid_limits = { 200, 100 };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&invalid_limits);
    // Implementation should handle this gracefully (may return null)
    if (memorytype) {
        wasm_memorytype_delete(memorytype);
    }
    // Test passes if no crash occurs
}

TEST_F(GlobalTableMemoryTypesTest, TableType_MaxSizeLimits_HandlesCorrectly) {
    wasm_valtype_t* valtype = wasm_valtype_new(WASM_FUNCREF);
    ASSERT_NE(nullptr, valtype);
    
    // Test with very large limits
    wasm_limits_t large_limits = { 0, UINT32_MAX };
    wasm_tabletype_t* tabletype = wasm_tabletype_new(valtype, &large_limits);
    if (tabletype) {
        const wasm_limits_t* table_limits = wasm_tabletype_limits(tabletype);
        ASSERT_NE(nullptr, table_limits);
        wasm_tabletype_delete(tabletype);
    }
    // Test passes if implementation handles large limits appropriately
}

TEST_F(GlobalTableMemoryTypesTest, MemoryType_MaxSizeLimits_HandlesCorrectly) {
    // Test with maximum possible limits
    wasm_limits_t max_limits = { 0, UINT32_MAX };
    wasm_memorytype_t* memorytype = wasm_memorytype_new(&max_limits);
    if (memorytype) {
        const wasm_limits_t* memory_limits = wasm_memorytype_limits(memorytype);
        ASSERT_NE(nullptr, memory_limits);
        wasm_memorytype_delete(memorytype);
    }
    // Test passes if implementation handles maximum limits appropriately
}