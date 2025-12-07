/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "aot_intrinsic.h"
#include "aot_compiler.h"
#include "bh_platform.h"
#include "wasm_runtime.h"

#ifdef __cplusplus
}
#endif

#include <climits>

// Test class for AOT Intrinsic Function Step 1: Integer Arithmetic Operations
class AOTIntrinsicFunctionTestStep1 : public testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize WAMR runtime for AOT intrinsic testing
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
    }
    
    void TearDown() override
    {
        // Clean up WAMR runtime
        wasm_runtime_destroy();
    }

    // Helper to create mock AOTCompContext for testing static functions
    AOTCompContext* create_mock_comp_context_with_intrinsics(const char* intrinsic_groups)
    {
#if WASM_ENABLE_WAMR_COMPILER != 0
        AOTCompContext* comp_ctx = (AOTCompContext*)wasm_runtime_malloc(sizeof(AOTCompContext));
        if (!comp_ctx) {
            return nullptr;
        }
        
        memset(comp_ctx, 0, sizeof(AOTCompContext));
        
        // Set builtin_intrinsics to trigger specific intrinsic groups
        if (intrinsic_groups) {
            size_t len = strlen(intrinsic_groups) + 1;
            char* intrinsic_str = (char*)wasm_runtime_malloc(len);
            if (intrinsic_str) {
                bh_strcpy_s(intrinsic_str, len, intrinsic_groups);
                comp_ctx->builtin_intrinsics = intrinsic_str;
            }
        }
        
        return comp_ctx;
#else
        return nullptr;
#endif
    }
    
    void cleanup_mock_comp_context(AOTCompContext* comp_ctx)
    {
#if WASM_ENABLE_WAMR_COMPILER != 0
        if (comp_ctx) {
            if (comp_ctx->builtin_intrinsics) {
                wasm_runtime_free((void*)comp_ctx->builtin_intrinsics);
            }
            wasm_runtime_free(comp_ctx);
        }
#endif
    }
    
    bool verify_capability_flag(AOTCompContext* comp_ctx, uint64 flag)
    {
#if WASM_ENABLE_WAMR_COMPILER != 0
        if (!comp_ctx) return false;
        
        // Check if the specific flag is set in the capability flags
        uint32 flag_index = (uint32)(flag >> 32);
        uint32 flag_bit = (uint32)(flag & 0xFFFFFFFF);
        
        if (flag_index < sizeof(comp_ctx->flags) / sizeof(comp_ctx->flags[0])) {
            return (comp_ctx->flags[flag_index] & flag_bit) != 0;
        }
#endif
        return false;
    }
};

// Step 1 Function 1: aot_intrinsic_i32_div_s - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI32DivS_BasicDivision_ReturnsCorrectResults)
{
    // Test normal division cases
    ASSERT_EQ(3, aot_intrinsic_i32_div_s(10, 3));
    ASSERT_EQ(-3, aot_intrinsic_i32_div_s(-10, 3));
    ASSERT_EQ(-3, aot_intrinsic_i32_div_s(10, -3));
    ASSERT_EQ(3, aot_intrinsic_i32_div_s(-10, -3));
    
    // Test division by 1 and -1
    ASSERT_EQ(42, aot_intrinsic_i32_div_s(42, 1));
    ASSERT_EQ(-42, aot_intrinsic_i32_div_s(42, -1));
    
    // Test zero dividend
    ASSERT_EQ(0, aot_intrinsic_i32_div_s(0, 5));
    ASSERT_EQ(0, aot_intrinsic_i32_div_s(0, -5));
    
    // Test truncation toward zero for negative results
    ASSERT_EQ(-1, aot_intrinsic_i32_div_s(-5, 3));  // -5/3 = -1.67 -> -1 (toward zero)
    ASSERT_EQ(1, aot_intrinsic_i32_div_s(-5, -3));  // -5/-3 = 1.67 -> 1 (toward zero)
}

// Step 1 Function 1: aot_intrinsic_i32_div_s - Edge Cases
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI32DivS_EdgeCases_HandlesCorrectly)
{
    // Test maximum positive and negative values
    ASSERT_EQ(1, aot_intrinsic_i32_div_s(INT32_MAX, INT32_MAX));
    ASSERT_EQ(1, aot_intrinsic_i32_div_s(INT32_MIN, INT32_MIN));
    
    // Test division of maximum values by small numbers
    ASSERT_EQ(INT32_MAX / 2, aot_intrinsic_i32_div_s(INT32_MAX, 2));
    ASSERT_EQ(INT32_MIN / 2, aot_intrinsic_i32_div_s(INT32_MIN, 2));
    
    // Note: Division by zero and INT32_MIN / -1 are undefined behavior
    // We don't test these cases as they may cause traps or undefined results
}

// Step 1 Function 2: aot_intrinsic_i32_div_u - Basic Functionality  
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI32DivU_BasicDivision_ReturnsCorrectResults)
{
    // Test normal unsigned division
    ASSERT_EQ(3U, aot_intrinsic_i32_div_u(10U, 3U));
    ASSERT_EQ(UINT32_MAX / 2, aot_intrinsic_i32_div_u(UINT32_MAX, 2U));
    
    // Test division by 1
    ASSERT_EQ(42U, aot_intrinsic_i32_div_u(42U, 1U));
    ASSERT_EQ(UINT32_MAX, aot_intrinsic_i32_div_u(UINT32_MAX, 1U));
    
    // Test zero dividend
    ASSERT_EQ(0U, aot_intrinsic_i32_div_u(0U, 5U));
    
    // Test large unsigned values
    ASSERT_EQ(1U, aot_intrinsic_i32_div_u(UINT32_MAX, UINT32_MAX));
    ASSERT_EQ(0U, aot_intrinsic_i32_div_u(5U, UINT32_MAX));
}

// Step 1 Function 3: aot_intrinsic_i32_rem_s - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI32RemS_BasicRemainder_ReturnsCorrectResults)
{
    // Test normal remainder cases
    ASSERT_EQ(1, aot_intrinsic_i32_rem_s(10, 3));
    ASSERT_EQ(-1, aot_intrinsic_i32_rem_s(-10, 3));  // Remainder has same sign as dividend
    ASSERT_EQ(1, aot_intrinsic_i32_rem_s(10, -3));   // Remainder has same sign as dividend
    ASSERT_EQ(-1, aot_intrinsic_i32_rem_s(-10, -3)); // Remainder has same sign as dividend
    
    // Test remainder by 1 (always 0)
    ASSERT_EQ(0, aot_intrinsic_i32_rem_s(42, 1));
    ASSERT_EQ(0, aot_intrinsic_i32_rem_s(-42, 1));
    
    // Test zero dividend
    ASSERT_EQ(0, aot_intrinsic_i32_rem_s(0, 5));
    ASSERT_EQ(0, aot_intrinsic_i32_rem_s(0, -5));
    
    // Test edge cases
    ASSERT_EQ(1, aot_intrinsic_i32_rem_s(INT32_MAX, 2));
    ASSERT_EQ(0, aot_intrinsic_i32_rem_s(INT32_MIN, 2));
}

// Step 1 Function 4: aot_intrinsic_i32_rem_u - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI32RemU_BasicRemainder_ReturnsCorrectResults)
{
    // Test normal unsigned remainder
    ASSERT_EQ(1U, aot_intrinsic_i32_rem_u(10U, 3U));
    ASSERT_EQ(1U, aot_intrinsic_i32_rem_u(UINT32_MAX, 2U));
    
    // Test remainder by 1 (always 0)
    ASSERT_EQ(0U, aot_intrinsic_i32_rem_u(42U, 1U));
    ASSERT_EQ(0U, aot_intrinsic_i32_rem_u(UINT32_MAX, 1U));
    
    // Test zero dividend
    ASSERT_EQ(0U, aot_intrinsic_i32_rem_u(0U, 5U));
    
    // Test large unsigned values
    ASSERT_EQ(0U, aot_intrinsic_i32_rem_u(UINT32_MAX, UINT32_MAX));
    ASSERT_EQ(5U, aot_intrinsic_i32_rem_u(5U, UINT32_MAX));
    
    // Test power of 2 modulo (equivalent to bitwise AND)
    ASSERT_EQ(3U, aot_intrinsic_i32_rem_u(19U, 16U));  // 19 % 16 = 3
    ASSERT_EQ(7U, aot_intrinsic_i32_rem_u(23U, 8U));   // 23 % 8 = 7
}

// Step 1 Function 5: aot_intrinsic_i64_div_s - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI64DivS_BasicDivision_ReturnsCorrectResults)
{
    // Test normal 64-bit division with large numbers
    ASSERT_EQ(3333333333LL, aot_intrinsic_i64_div_s(10000000000LL, 3LL));
    ASSERT_EQ(-3333333333LL, aot_intrinsic_i64_div_s(-10000000000LL, 3LL));
    
    // Test division with mixed positive/negative operands
    ASSERT_EQ(-5000000000LL, aot_intrinsic_i64_div_s(10000000000LL, -2LL));
    ASSERT_EQ(5000000000LL, aot_intrinsic_i64_div_s(-10000000000LL, -2LL));
    
    // Test division by powers of 2
    ASSERT_EQ(1250000000LL, aot_intrinsic_i64_div_s(10000000000LL, 8LL));
    ASSERT_EQ(625000000LL, aot_intrinsic_i64_div_s(10000000000LL, 16LL));
    
    // Test truncation toward zero
    ASSERT_EQ(-1LL, aot_intrinsic_i64_div_s(-5LL, 3LL));
    ASSERT_EQ(1LL, aot_intrinsic_i64_div_s(-5LL, -3LL));
    
    // Test with maximum values
    ASSERT_EQ(1LL, aot_intrinsic_i64_div_s(INT64_MAX, INT64_MAX));
    ASSERT_EQ(INT64_MAX / 2, aot_intrinsic_i64_div_s(INT64_MAX, 2LL));
}

// Step 1 Function 6: aot_intrinsic_i64_div_u - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI64DivU_BasicDivision_ReturnsCorrectResults)
{
    // Test division with very large 64-bit unsigned numbers
    ASSERT_EQ(3333333333ULL, aot_intrinsic_i64_div_u(10000000000ULL, 3ULL));
    ASSERT_EQ(UINT64_MAX / 2, aot_intrinsic_i64_div_u(UINT64_MAX, 2ULL));
    
    // Test division by powers of 2
    ASSERT_EQ(1250000000ULL, aot_intrinsic_i64_div_u(10000000000ULL, 8ULL));
    ASSERT_EQ(625000000ULL, aot_intrinsic_i64_div_u(10000000000ULL, 16ULL));
    
    // Test cases where result uses full 64-bit range
    ASSERT_EQ(1ULL, aot_intrinsic_i64_div_u(UINT64_MAX, UINT64_MAX));
    ASSERT_EQ(0ULL, aot_intrinsic_i64_div_u(5ULL, UINT64_MAX));
    
    // Test no overflow with maximum unsigned values
    ASSERT_EQ(UINT64_MAX / 3, aot_intrinsic_i64_div_u(UINT64_MAX, 3ULL));
    ASSERT_EQ(1ULL, aot_intrinsic_i64_div_u(UINT64_MAX / 2 + 1, UINT64_MAX / 2 + 1));
}

// Step 1 Function 7: aot_intrinsic_i64_rem_s - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI64RemS_BasicRemainder_ReturnsCorrectResults)
{
    // Test remainder with large 64-bit signed numbers
    ASSERT_EQ(1LL, aot_intrinsic_i64_rem_s(10000000000LL, 3LL));
    ASSERT_EQ(-1LL, aot_intrinsic_i64_rem_s(-10000000000LL, 3LL));
    
    // Test remainder with mixed positive/negative operands
    ASSERT_EQ(1LL, aot_intrinsic_i64_rem_s(10000000000LL, -3LL));  // Remainder sign matches dividend
    ASSERT_EQ(-1LL, aot_intrinsic_i64_rem_s(-10000000000LL, -3LL)); // Remainder sign matches dividend
    
    // Test remainder sign matches dividend sign
    ASSERT_EQ(2LL, aot_intrinsic_i64_rem_s(17LL, 5LL));
    ASSERT_EQ(-2LL, aot_intrinsic_i64_rem_s(-17LL, 5LL));
    ASSERT_EQ(2LL, aot_intrinsic_i64_rem_s(17LL, -5LL));
    ASSERT_EQ(-2LL, aot_intrinsic_i64_rem_s(-17LL, -5LL));
    
    // Test edge cases with maximum/minimum values
    ASSERT_EQ(1LL, aot_intrinsic_i64_rem_s(INT64_MAX, 2LL));
    ASSERT_EQ(0LL, aot_intrinsic_i64_rem_s(INT64_MIN, 2LL));
}

// Step 1 Function 8: aot_intrinsic_i64_rem_u - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI64RemU_BasicRemainder_ReturnsCorrectResults)
{
    // Test remainder with large 64-bit unsigned numbers
    ASSERT_EQ(1ULL, aot_intrinsic_i64_rem_u(10000000000ULL, 3ULL));
    ASSERT_EQ(1ULL, aot_intrinsic_i64_rem_u(UINT64_MAX, 2ULL));
    
    // Test remainder by powers of 2 (equivalent to bitwise AND)
    ASSERT_EQ(3ULL, aot_intrinsic_i64_rem_u(19ULL, 16ULL));  // 19 % 16 = 3
    ASSERT_EQ(7ULL, aot_intrinsic_i64_rem_u(23ULL, 8ULL));   // 23 % 8 = 7
    ASSERT_EQ(0ULL, aot_intrinsic_i64_rem_u(64ULL, 16ULL));  // 64 % 16 = 0
    
    // Test cases with maximum unsigned values
    ASSERT_EQ(0ULL, aot_intrinsic_i64_rem_u(UINT64_MAX, UINT64_MAX));
    ASSERT_EQ(5ULL, aot_intrinsic_i64_rem_u(5ULL, UINT64_MAX));
    
    // Test proper handling of full 64-bit range
    ASSERT_EQ(UINT64_MAX % 3, aot_intrinsic_i64_rem_u(UINT64_MAX, 3ULL));
    ASSERT_EQ(0ULL, aot_intrinsic_i64_rem_u(UINT64_MAX / 2 + 1, UINT64_MAX / 2 + 1));
}

// Step 1 Function 9: aot_intrinsic_i64_mul - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep1, AotIntrinsicI64Mul_BasicMultiplication_ReturnsCorrectResults)
{
    // Test normal 64-bit multiplication with various operand sizes
    ASSERT_EQ(30000000000ULL, aot_intrinsic_i64_mul(10000000ULL, 3000ULL));
    ASSERT_EQ(1000000000000ULL, aot_intrinsic_i64_mul(1000000ULL, 1000000ULL));
    
    // Test multiplication by 0, 1, and -1 (as uint64)
    ASSERT_EQ(0ULL, aot_intrinsic_i64_mul(12345ULL, 0ULL));
    ASSERT_EQ(12345ULL, aot_intrinsic_i64_mul(12345ULL, 1ULL));
    ASSERT_EQ(UINT64_MAX - 12344ULL, aot_intrinsic_i64_mul(12345ULL, UINT64_MAX)); // 12345 * -1 in two's complement
    
    // Test multiplication that causes overflow (wraparound behavior)
    uint64 large_val = UINT64_MAX / 2 + 1;  // This will cause overflow when multiplied by 2
    ASSERT_EQ(0ULL, aot_intrinsic_i64_mul(large_val, 2ULL));  // Should wrap around to 0
    
    // Test multiplication with large operands
    ASSERT_EQ(1ULL, aot_intrinsic_i64_mul(UINT64_MAX, UINT64_MAX)); // Overflow wraps to 1
    
    // Test powers of 2
    ASSERT_EQ(1024ULL, aot_intrinsic_i64_mul(32ULL, 32ULL));
    ASSERT_EQ(65536ULL, aot_intrinsic_i64_mul(256ULL, 256ULL));
}

// Step 1 Function 10: add_f32xi32_intrinsics - Indirect Testing via Capability Flags
TEST_F(AOTIntrinsicFunctionTestStep1, CapabilityFlags_AddF32xi32Intrinsics_SetsCorrectFlags)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Create AOTCompContext with "f32xi32" intrinsic group
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("f32xi32");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to trigger add_f32xi32_intrinsics()
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set for f32xi32 conversion operations
    // Note: We can't directly access the flag constants, so we test the behavior
    // by checking if the flags array is modified (non-zero)
    bool flags_set = false;
    for (int i = 0; i < sizeof(comp_ctx->flags) / sizeof(comp_ctx->flags[0]); i++) {
        if (comp_ctx->flags[i] != 0) {
            flags_set = true;
            break;
        }
    }
    ASSERT_TRUE(flags_set);
    
    cleanup_mock_comp_context(comp_ctx);
#endif
}

TEST_F(AOTIntrinsicFunctionTestStep1, CapabilityFlags_FpxintGroup_IncludesF32xi32Intrinsics)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Test with "fpxint" group that includes f32xi32
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("fpxint");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to trigger all float/int conversion groups
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set (fpxint should include f32xi32 intrinsics)
    bool flags_set = false;
    for (int i = 0; i < sizeof(comp_ctx->flags) / sizeof(comp_ctx->flags[0]); i++) {
        if (comp_ctx->flags[i] != 0) {
            flags_set = true;
            break;
        }
    }
    ASSERT_TRUE(flags_set);
    
    cleanup_mock_comp_context(comp_ctx);
#endif
}

TEST_F(AOTIntrinsicFunctionTestStep1, CapabilityFlags_EmptyIntrinsicString_NoFlagsSet)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Test edge case with empty intrinsic string
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags with empty string
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify no capability flags are set for empty string
    bool flags_set = false;
    for (int i = 0; i < sizeof(comp_ctx->flags) / sizeof(comp_ctx->flags[0]); i++) {
        if (comp_ctx->flags[i] != 0) {
            flags_set = true;
            break;
        }
    }
    ASSERT_FALSE(flags_set);
    
    cleanup_mock_comp_context(comp_ctx);
#endif
}

TEST_F(AOTIntrinsicFunctionTestStep1, CapabilityFlags_InvalidGroupName_NoFlagsSet)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Test edge case with invalid intrinsic group name
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("invalid_group_name");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags with invalid group
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify no capability flags are set for invalid group name
    bool flags_set = false;
    for (int i = 0; i < sizeof(comp_ctx->flags) / sizeof(comp_ctx->flags[0]); i++) {
        if (comp_ctx->flags[i] != 0) {
            flags_set = true;
            break;
        }
    }
    ASSERT_FALSE(flags_set);
    
    cleanup_mock_comp_context(comp_ctx);
#endif
}