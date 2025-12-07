/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include "wasm_export.h"
#include "bh_read_file.h"
#include "test_helper.h"

// Platform test context for feature detection
class PlatformTestContext {
public:
    static bool HasSIMDSupport() {
#if WASM_ENABLE_SIMD != 0
        return true;
#else
        return false;
#endif
    }
    
    static bool HasJITSupport() {
#if WASM_ENABLE_JIT != 0
        return true;
#else
        return false;
#endif
    }
    
    static bool HasFastJITSupport() {
#if WASM_ENABLE_FAST_JIT != 0
        return true;
#else
        return false;
#endif
    }
    
    static bool HasMemory64Support() {
#if WASM_ENABLE_MEMORY64 != 0
        return true;
#else
        return false;
#endif
    }
};

// Helper functions for i64 parameter handling
static inline uint64_t
get_u64_from_addr(uint32_t *addr)
{
    union {
        uint64_t val;
        uint32_t parts[2];
    } u;
    u.parts[0] = addr[0];
    u.parts[1] = addr[1];
    return u.val;
}

static inline void
put_u64_to_addr(uint32_t *addr, uint64_t value)
{
    uint32_t *addr_u32 = (uint32_t *)(addr);
    union {
        uint64_t val;
        uint32_t parts[2];
    } u;
    u.val = value;
    addr_u32[0] = u.parts[0];
    addr_u32[1] = u.parts[1];
}

/**
 * Interpreter Coverage Improvement - Step 1: Arithmetic and Bitwise Operations
 * 
 * Target Functions (10 functions, ~140 lines coverage):
 * - clz32, clz64 (Count Leading Zeros)
 * - ctz32, ctz64 (Count Trailing Zeros)  
 * - rotl32, rotr32, rotl64, rotr64 (Rotate Left/Right)
 * - popcount32, popcount64 (Population Count)
 * 
 * Expected Coverage Improvement: +1.5% (140+ lines)
 */

class ArithmeticBitwiseTest : public testing::Test
{
protected:
    void SetUp() override
    {
        runtime = std::make_unique<WAMRRuntimeRAII<512 * 1024>>();
        
        // Load the arithmetic_bitwise.wasm module using standard WAMR pattern
        wasm_file_buf = (unsigned char *)bh_read_file_to_buffer("arithmetic_bitwise.wasm", &wasm_file_size);
        ASSERT_NE(wasm_file_buf, nullptr) << "Failed to read WASM file";
        
        module = std::make_unique<WAMRModule>(wasm_file_buf, wasm_file_size);
        ASSERT_NE(module->get(), nullptr) << "Failed to load WASM module";
        
        instance = std::make_unique<WAMRInstance>(*module, 8192, 8192);
        ASSERT_NE(instance->get(), nullptr) << "Failed to instantiate WASM module";
        
        exec_env = std::make_unique<WAMRExecEnv>(*instance, 8192);
        ASSERT_NE(exec_env->get(), nullptr) << "Failed to create execution environment";
    }
    
    void TearDown() override
    {
        exec_env.reset();
        instance.reset();
        module.reset();
        if (wasm_file_buf) {
            BH_FREE(wasm_file_buf);
        }
        runtime.reset();
    }
    
    // Helper function to call WASM functions with i32 parameter and return
    uint32_t call_wasm_i32_i32(const char* func_name, uint32_t param)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(instance->get(), func_name);
        EXPECT_NE(func, nullptr) << "Function not found: " << func_name;
        
        uint32_t wasm_argv[1] = { param };
        bool success = wasm_runtime_call_wasm(exec_env->get(), func, 1, wasm_argv);
        EXPECT_TRUE(success) << "Function call failed: " << wasm_runtime_get_exception(instance->get());
        
        return wasm_argv[0];
    }
    
    // Helper function to call WASM functions with i64 parameter and return
    uint64_t call_wasm_i64_i64(const char* func_name, uint64_t param)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(instance->get(), func_name);
        EXPECT_NE(func, nullptr) << "Function not found: " << func_name;
        
        uint32_t wasm_argv[2];
        put_u64_to_addr(wasm_argv, param);
        bool success = wasm_runtime_call_wasm(exec_env->get(), func, 2, wasm_argv);
        EXPECT_TRUE(success) << "Function call failed: " << wasm_runtime_get_exception(instance->get());
        
        return get_u64_from_addr(wasm_argv);
    }
    
    // Helper function to call WASM functions with two i32 parameters
    uint32_t call_wasm_i32_i32_i32(const char* func_name, uint32_t param1, uint32_t param2)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(instance->get(), func_name);
        EXPECT_NE(func, nullptr) << "Function not found: " << func_name;
        
        uint32_t wasm_argv[2] = { param1, param2 };
        bool success = wasm_runtime_call_wasm(exec_env->get(), func, 2, wasm_argv);
        EXPECT_TRUE(success) << "Function call failed: " << wasm_runtime_get_exception(instance->get());
        
        return wasm_argv[0];
    }
    
    // Helper function to call WASM functions with two i64 parameters
    uint64_t call_wasm_i64_i64_i64(const char* func_name, uint64_t param1, uint64_t param2)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(instance->get(), func_name);
        EXPECT_NE(func, nullptr) << "Function not found: " << func_name;
        
        uint32_t wasm_argv[4];
        put_u64_to_addr(&wasm_argv[0], param1);
        put_u64_to_addr(&wasm_argv[2], param2);
        bool success = wasm_runtime_call_wasm(exec_env->get(), func, 4, wasm_argv);
        EXPECT_TRUE(success) << "Function call failed: " << wasm_runtime_get_exception(instance->get());
        
        return get_u64_from_addr(wasm_argv);
    }
    
    std::unique_ptr<WAMRRuntimeRAII<512 * 1024>> runtime;
    std::unique_ptr<WAMRModule> module;
    std::unique_ptr<WAMRInstance> instance;
    std::unique_ptr<WAMRExecEnv> exec_env;
    unsigned char *wasm_file_buf = nullptr;
    uint32_t wasm_file_size = 0;
};

// Test Count Leading Zeros operations
TEST_F(ArithmeticBitwiseTest, CountLeadingZeros_VariousInputs_ReturnsCorrectCount)
{
    // Test i32.clz with various values
    ASSERT_EQ(call_wasm_i32_i32("test_clz32", 0x80000000), 0U);
    ASSERT_EQ(call_wasm_i32_i32("test_clz32", 0x40000000), 1U);
    ASSERT_EQ(call_wasm_i32_i32("test_clz32", 0x00000001), 31U);
    ASSERT_EQ(call_wasm_i32_i32("test_clz32", 0x00000000), 32U);
    
    // Test i64.clz with various values
    ASSERT_EQ(call_wasm_i64_i64("test_clz64", 0x8000000000000000ULL), 0ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_clz64", 0x4000000000000000ULL), 1ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_clz64", 0x0000000000000001ULL), 63ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_clz64", 0x0000000000000000ULL), 64ULL);
}

// Test Count Trailing Zeros operations
TEST_F(ArithmeticBitwiseTest, CountTrailingZeros_VariousInputs_ReturnsCorrectCount)
{
    // Test i32.ctz with various values
    ASSERT_EQ(call_wasm_i32_i32("test_ctz32", 0x00000001), 0U);
    ASSERT_EQ(call_wasm_i32_i32("test_ctz32", 0x00000002), 1U);
    ASSERT_EQ(call_wasm_i32_i32("test_ctz32", 0x80000000), 31U);
    ASSERT_EQ(call_wasm_i32_i32("test_ctz32", 0x00000000), 32U);
    
    // Test i64.ctz with various values
    ASSERT_EQ(call_wasm_i64_i64("test_ctz64", 0x0000000000000001ULL), 0ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_ctz64", 0x0000000000000002ULL), 1ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_ctz64", 0x8000000000000000ULL), 63ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_ctz64", 0x0000000000000000ULL), 64ULL);
}

// Test Population Count operations
TEST_F(ArithmeticBitwiseTest, PopulationCount_VariousInputs_ReturnsCorrectCount)
{
    // Test i32.popcnt with various values
    ASSERT_EQ(call_wasm_i32_i32("test_popcnt32", 0x00000000), 0U);
    ASSERT_EQ(call_wasm_i32_i32("test_popcnt32", 0x00000001), 1U);
    ASSERT_EQ(call_wasm_i32_i32("test_popcnt32", 0x00000003), 2U);
    ASSERT_EQ(call_wasm_i32_i32("test_popcnt32", 0xFFFFFFFF), 32U);
    ASSERT_EQ(call_wasm_i32_i32("test_popcnt32", 0xAAAAAAAA), 16U);
    
    // Test i64.popcnt with various values
    ASSERT_EQ(call_wasm_i64_i64("test_popcnt64", 0x0000000000000000ULL), 0ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_popcnt64", 0x0000000000000001ULL), 1ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_popcnt64", 0x0000000000000003ULL), 2ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_popcnt64", 0xFFFFFFFFFFFFFFFFULL), 64ULL);
    ASSERT_EQ(call_wasm_i64_i64("test_popcnt64", 0xAAAAAAAAAAAAAAAAULL), 32ULL);
}

// Test Rotate Left operations
TEST_F(ArithmeticBitwiseTest, RotateLeft_VariousInputs_ReturnsCorrectRotation)
{
    // Test i32.rotl with various values
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotl32", 0x80000000, 1), 0x00000001U);
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotl32", 0x00000001, 1), 0x00000002U);
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotl32", 0x12345678, 4), 0x23456781U);
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotl32", 0x12345678, 0), 0x12345678U);
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotl32", 0x12345678, 32), 0x12345678U);
    
    // Test i64.rotl with various values
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotl64", 0x8000000000000000ULL, 1), 0x0000000000000001ULL);
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotl64", 0x0000000000000001ULL, 1), 0x0000000000000002ULL);
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotl64", 0x123456789ABCDEF0ULL, 4), 0x23456789ABCDEF01ULL);
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotl64", 0x123456789ABCDEF0ULL, 0), 0x123456789ABCDEF0ULL);
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotl64", 0x123456789ABCDEF0ULL, 64), 0x123456789ABCDEF0ULL);
}

// Test Rotate Right operations
TEST_F(ArithmeticBitwiseTest, RotateRight_VariousInputs_ReturnsCorrectRotation)
{
    // Test i32.rotr with various values
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotr32", 0x00000001, 1), 0x80000000U);
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotr32", 0x80000000, 1), 0x40000000U);
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotr32", 0x12345678, 4), 0x81234567U);
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotr32", 0x12345678, 0), 0x12345678U);
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotr32", 0x12345678, 32), 0x12345678U);
    
    // Test i64.rotr with various values
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotr64", 0x0000000000000001ULL, 1), 0x8000000000000000ULL);
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotr64", 0x8000000000000000ULL, 1), 0x4000000000000000ULL);
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotr64", 0x123456789ABCDEF0ULL, 4), 0x0123456789ABCDEFULL);
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotr64", 0x123456789ABCDEF0ULL, 0), 0x123456789ABCDEF0ULL);
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotr64", 0x123456789ABCDEF0ULL, 64), 0x123456789ABCDEF0ULL);
}

// Test edge cases and boundary conditions
TEST_F(ArithmeticBitwiseTest, EdgeCases_BoundaryConditions_HandledCorrectly)
{
    // Test with maximum values
    ASSERT_EQ(call_wasm_i32_i32("test_clz32", 0xFFFFFFFF), 0U);
    ASSERT_EQ(call_wasm_i64_i64("test_clz64", 0xFFFFFFFFFFFFFFFFULL), 0ULL);
    
    // Test with alternating bit patterns
    ASSERT_EQ(call_wasm_i32_i32("test_popcnt32", 0x55555555), 16U);
    ASSERT_EQ(call_wasm_i64_i64("test_popcnt64", 0x5555555555555555ULL), 32ULL);
    
    // Test rotation with large shift amounts (should wrap around)
    ASSERT_EQ(call_wasm_i32_i32_i32("test_rotl32", 0x12345678, 36), 0x23456781U); // 36 % 32 = 4
    ASSERT_EQ(call_wasm_i64_i64_i64("test_rotl64", 0x123456789ABCDEF0ULL, 68), 0x23456789ABCDEF01ULL); // 68 % 64 = 4
}