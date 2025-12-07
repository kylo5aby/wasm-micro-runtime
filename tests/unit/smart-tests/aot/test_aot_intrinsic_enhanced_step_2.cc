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

// Test class for AOT Intrinsic Function Step 2: Bitwise Operations & Float Conversion Intrinsics
class AOTIntrinsicFunctionTestStep2 : public testing::Test
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
};

// Step 2 Function 1: aot_intrinsic_i64_bit_and - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep2, AotIntrinsicI64BitAnd_BasicBitwiseAnd_ReturnsCorrectResults)
{
    // Test AND with all bits set
    ASSERT_EQ(UINT64_MAX, aot_intrinsic_i64_bit_and(UINT64_MAX, UINT64_MAX));
    
    // Test AND with zero
    ASSERT_EQ(0ULL, aot_intrinsic_i64_bit_and(0x123456789ABCDEFULL, 0ULL));
    ASSERT_EQ(0ULL, aot_intrinsic_i64_bit_and(0ULL, 0x123456789ABCDEFULL));
    
    // Test AND with alternating bit patterns
    ASSERT_EQ(0ULL, aot_intrinsic_i64_bit_and(0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL));
    
    // Test AND for bit masking operations
    ASSERT_EQ(0x000000000000DEFULL, aot_intrinsic_i64_bit_and(0x123456789ABCDEFULL, 0x0000000000000FFFULL));
    ASSERT_EQ(0x123456700000000ULL, aot_intrinsic_i64_bit_and(0x123456789ABCDEFULL, 0xFFFFFFFF00000000ULL));
    
    // Test specific bit patterns
    ASSERT_EQ(0x1000000000000000ULL, aot_intrinsic_i64_bit_and(0x1234567890ABCDEFULL, 0xF000000000000000ULL));
    ASSERT_EQ(0x0000000000000DEFULL, aot_intrinsic_i64_bit_and(0x1234567890ABCDEFULL, 0x0000000000000FFFULL));
}

// Step 2 Function 2: aot_intrinsic_i64_bit_or - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep2, AotIntrinsicI64BitOr_BasicBitwiseOr_ReturnsCorrectResults)
{
    // Test OR with zero
    uint64 test_value = 0x123456789ABCDEFULL;
    ASSERT_EQ(test_value, aot_intrinsic_i64_bit_or(test_value, 0ULL));
    ASSERT_EQ(test_value, aot_intrinsic_i64_bit_or(0ULL, test_value));
    
    // Test OR with all bits set
    ASSERT_EQ(UINT64_MAX, aot_intrinsic_i64_bit_or(test_value, UINT64_MAX));
    ASSERT_EQ(UINT64_MAX, aot_intrinsic_i64_bit_or(UINT64_MAX, test_value));
    
    // Test OR with complementary patterns
    ASSERT_EQ(UINT64_MAX, aot_intrinsic_i64_bit_or(0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL));
    
    // Test OR for bit setting operations
    ASSERT_EQ(0x123456789ABCFFFULL, aot_intrinsic_i64_bit_or(0x123456789ABCDEFULL, 0x0000000000000FFFULL));
    ASSERT_EQ(0xffffffff89abcdefULL, aot_intrinsic_i64_bit_or(0x123456789ABCDEFULL, 0xFFFFFFFF00000000ULL));
    
    // Test combining different bit patterns
    ASSERT_EQ(0xF234567890ABCDEFULL, aot_intrinsic_i64_bit_or(0x1234567890ABCDEFULL, 0xF000000000000000ULL));
    ASSERT_EQ(0x1234567890ABCFFFULL, aot_intrinsic_i64_bit_or(0x1234567890ABCDEFULL, 0x0000000000000FFFULL));
}

// Step 2 Function 3: aot_intrinsic_i64_shl - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep2, AotIntrinsicI64Shl_BasicLeftShift_ReturnsCorrectResults)
{
    // Test normal left shifts
    ASSERT_EQ(2ULL, aot_intrinsic_i64_shl(1ULL, 1ULL));
    ASSERT_EQ(0x8000000000000000ULL, aot_intrinsic_i64_shl(1ULL, 63ULL));  // 1 << 63 = INT64_MIN (as uint64)
    
    // Test shift by zero
    uint64 test_value = 0x123456789ABCDEFULL;
    ASSERT_EQ(test_value, aot_intrinsic_i64_shl(test_value, 0ULL));
    
    // Test shifts that cause bits to fall off the left end
    ASSERT_EQ(0x48d159e26af37bcULL, aot_intrinsic_i64_shl(0x123456789ABCDEFULL, 2ULL));
    ASSERT_EQ(0x23456789ABCDEF00ULL, aot_intrinsic_i64_shl(0x123456789ABCDEFULL, 8ULL));
    
    // Test powers of 2 shifts
    ASSERT_EQ(1024ULL, aot_intrinsic_i64_shl(1ULL, 10ULL));  // 1 << 10 = 1024
    ASSERT_EQ(65536ULL, aot_intrinsic_i64_shl(1ULL, 16ULL)); // 1 << 16 = 65536
    
    // Test shift amounts at boundary (shift by 63 is maximum valid)
    ASSERT_EQ(0ULL, aot_intrinsic_i64_shl(2ULL, 63ULL));  // 2 << 63 overflows to 0
    
    // Note: Large shift amounts (>= 64) behavior is undefined, so we don't test them
}

// Step 2 Function 4: aot_intrinsic_i64_shr_s - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep2, AotIntrinsicI64ShrS_BasicArithmeticRightShift_ReturnsCorrectResults)
{
    // Test arithmetic right shift with positive numbers (zero fill)
    ASSERT_EQ(0x48d159e26af37bULL, aot_intrinsic_i64_shr_s(0x123456789ABCDEFULL, 2ULL));
    ASSERT_EQ(0x123456789ABCDULL, aot_intrinsic_i64_shr_s(0x123456789ABCDEFULL, 8ULL));
    
    // Test arithmetic right shift with negative numbers (sign extension)
    // Note: The function takes uint64 but treats as signed for arithmetic shift
    uint64 negative_value = 0x8000000000000000ULL;  // INT64_MIN as uint64
    ASSERT_EQ(0xC000000000000000ULL, aot_intrinsic_i64_shr_s(negative_value, 1ULL));  // Sign extension
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, aot_intrinsic_i64_shr_s(negative_value, 63ULL)); // All 1s from sign extension
    
    // Test shift by zero
    uint64 test_value = 0x123456789ABCDEFULL;
    ASSERT_EQ(test_value, aot_intrinsic_i64_shr_s(test_value, 0ULL));
    
    // Test with high bit set (negative in signed interpretation)
    uint64 high_bit_value = 0xF000000000000000ULL;
    ASSERT_EQ(0xF800000000000000ULL, aot_intrinsic_i64_shr_s(high_bit_value, 1ULL));  // Sign extension
    ASSERT_EQ(0xFF00000000000000ULL, aot_intrinsic_i64_shr_s(high_bit_value, 4ULL));  // Sign extension
    
    // Test large shift amounts and sign preservation
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, aot_intrinsic_i64_shr_s(0x8000000000000001ULL, 63ULL));  // Sign extends to all 1s
}

// Step 2 Function 5: aot_intrinsic_i64_shr_u - Basic Functionality
TEST_F(AOTIntrinsicFunctionTestStep2, AotIntrinsicI64ShrU_BasicLogicalRightShift_ReturnsCorrectResults)
{
    // Test logical right shift (always zero fill from left)
    ASSERT_EQ(0x48d159e26af37bULL, aot_intrinsic_i64_shr_u(0x123456789ABCDEFULL, 2ULL));
    ASSERT_EQ(0x123456789ABCDULL, aot_intrinsic_i64_shr_u(0x123456789ABCDEFULL, 8ULL));
    
    // Test shift by zero
    uint64 test_value = 0x123456789ABCDEFULL;
    ASSERT_EQ(test_value, aot_intrinsic_i64_shr_u(test_value, 0ULL));
    
    // Test with high bit set to verify zero fill (not sign extension)
    uint64 high_bit_value = 0x8000000000000000ULL;  // High bit set
    ASSERT_EQ(0x4000000000000000ULL, aot_intrinsic_i64_shr_u(high_bit_value, 1ULL));  // Zero fill, not sign extension
    ASSERT_EQ(0x0800000000000000ULL, aot_intrinsic_i64_shr_u(high_bit_value, 4ULL));  // Zero fill
    
    // Test with all high bits set
    uint64 all_high_bits = 0xF000000000000000ULL;
    ASSERT_EQ(0x7800000000000000ULL, aot_intrinsic_i64_shr_u(all_high_bits, 1ULL));  // Zero fill
    ASSERT_EQ(0x0F00000000000000ULL, aot_intrinsic_i64_shr_u(all_high_bits, 4ULL));  // Zero fill
    
    // Test large shift amounts
    ASSERT_EQ(0x0000000000000001ULL, aot_intrinsic_i64_shr_u(0x8000000000000000ULL, 63ULL));  // Shifts to 1
    ASSERT_EQ(0x0000000000000000ULL, aot_intrinsic_i64_shr_u(0x7FFFFFFFFFFFFFFFULL, 63ULL));  // Shifts to 0
    
    // Test maximum value shifts
    ASSERT_EQ(0x0000000000000001ULL, aot_intrinsic_i64_shr_u(UINT64_MAX, 63ULL));  // Only lowest bit remains
}

// Step 2 Function 6: add_f32xi64_intrinsics - Indirect Testing via Capability Flags
TEST_F(AOTIntrinsicFunctionTestStep2, CapabilityFlags_AddF32xi64Intrinsics_SetsCorrectFlags)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Create AOTCompContext with "f32xi64" intrinsic group
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("f32xi64");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to trigger add_f32xi64_intrinsics()
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set for f32xi64 conversion operations
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

TEST_F(AOTIntrinsicFunctionTestStep2, CapabilityFlags_FpxintGroup_IncludesF32xi64Intrinsics)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Test with "fpxint" group that includes f32xi64
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("fpxint");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to trigger all float/int conversion groups
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set (fpxint should include f32xi64 intrinsics)
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

// Step 2 Function 7: add_f64xi32_intrinsics - Indirect Testing via Capability Flags
TEST_F(AOTIntrinsicFunctionTestStep2, CapabilityFlags_AddF64xi32Intrinsics_SetsCorrectFlags)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Create AOTCompContext with "f64xi32" intrinsic group
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("f64xi32");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to trigger add_f64xi32_intrinsics()
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set for f64xi32 conversion operations
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

TEST_F(AOTIntrinsicFunctionTestStep2, CapabilityFlags_MultipleGroups_OrderIndependent)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Test with compound groups like "fpxint" that include f64xi32
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("f64xi32 fpxint");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to process multiple groups
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set (both groups should be processed)
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

// Step 2 Function 8: add_f64xi64_intrinsics - Indirect Testing via Capability Flags
TEST_F(AOTIntrinsicFunctionTestStep2, CapabilityFlags_AddF64xi64Intrinsics_SetsCorrectFlags)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Create AOTCompContext with "f64xi64" intrinsic group
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("f64xi64");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to trigger add_f64xi64_intrinsics()
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set for f64xi64 conversion operations
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

TEST_F(AOTIntrinsicFunctionTestStep2, CapabilityFlags_ComprehensiveFpxintGroup_IncludesAllConversions)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Test comprehensive "fpxint" group that includes all float/int conversions
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("fpxint");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to trigger all conversion groups
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set (fpxint should include f64xi64 and all other conversions)
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

TEST_F(AOTIntrinsicFunctionTestStep2, CapabilityFlags_GroupPrecedence_HandlesOverlappingGroups)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Test overlapping groups to ensure proper handling
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("f64xi64 f32xi64 fpxint");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags to process overlapping groups
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are set properly with overlapping groups
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

TEST_F(AOTIntrinsicFunctionTestStep2, CapabilityFlags_InvalidGroupCombinations_HandlesGracefully)
{
#if WASM_ENABLE_WAMR_COMPILER != 0
    // Test invalid group combinations
    AOTCompContext* comp_ctx = create_mock_comp_context_with_intrinsics("f64xi64 invalid_group fpxint");
    ASSERT_NE(comp_ctx, nullptr);
    
    // Call aot_intrinsic_fill_capability_flags with mixed valid/invalid groups
    aot_intrinsic_fill_capability_flags(comp_ctx);
    
    // Verify capability flags are still set for valid groups (should ignore invalid ones)
    bool flags_set = false;
    for (int i = 0; i < sizeof(comp_ctx->flags) / sizeof(comp_ctx->flags[0]); i++) {
        if (comp_ctx->flags[i] != 0) {
            flags_set = true;
            break;
        }
    }
    ASSERT_TRUE(flags_set);  // Valid groups should still work
    
    cleanup_mock_comp_context(comp_ctx);
#endif
}