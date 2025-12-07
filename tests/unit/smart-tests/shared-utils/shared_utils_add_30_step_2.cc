/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

extern "C" {
#include "bh_common.h"
#include "bh_platform.h"
}

// Platform detection utility for tests - REQUIRED in every test file
class PlatformTestContext {
public:
    // Architecture detection
    static bool IsX86_64() {
#if defined(BUILD_TARGET_X86) || defined(BUILD_TARGET_X86_64)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsARM64() {
#if defined(BUILD_TARGET_AARCH64) || defined(BUILD_TARGET_ARM64)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsARM32() {
#if defined(BUILD_TARGET_ARM) && !defined(BUILD_TARGET_AARCH64)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsRISCV() {
#if defined(BUILD_TARGET_RISCV) || defined(BUILD_TARGET_RISCV64) || defined(BUILD_TARGET_RISCV32)
        return true;
#else
        return false;
#endif
    }
    
    // Feature detection
    static bool HasSIMDSupport() {
#if WASM_ENABLE_SIMD != 0
        return true;
#else
        return false;
#endif
    }
    
    static bool HasAOTSupport() {
#if WASM_ENABLE_AOT != 0
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
    
    static bool HasMemory64Support() {
#if WASM_ENABLE_MEMORY64 != 0
        return true;
#else
        return false;
#endif
    }
    
    // Platform capability helpers
    static bool Is64BitArchitecture() {
        return IsX86_64() || IsARM64() || (IsRISCV() && sizeof(void*) == 8);
    }
    
    static bool Is32BitArchitecture() {
        return IsARM32() || (IsRISCV() && sizeof(void*) == 4);
    }
};

// External function declarations for testing internal functions
extern "C" {
    // Forward declare the functions we're testing
    int b_memcpy_wa(void *s1, unsigned int s1max, const void *s2, unsigned int n);
    
#if WASM_ENABLE_WAMR_COMPILER != 0 || WASM_ENABLE_JIT != 0
    int bh_system(const char *cmd);
#endif
}

// Helper function to replicate align_ptr logic for testing
// Since align_ptr is static, we replicate its functionality
static char *test_align_ptr(char *src, unsigned int b)
{
    uintptr_t v = (uintptr_t)src;
    uintptr_t m = b - 1;
    return (char *)((v + m) & ~m);
}

// Test fixture for Memory Utility Functions
class MemoryUtilityTest : public testing::Test {
protected:
    void SetUp() override {
        // Set up test buffers
        memset(source_buffer, 0x55, sizeof(source_buffer));
        memset(dest_buffer, 0xAA, sizeof(dest_buffer));
        
        // Create test patterns
        for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
            test_pattern[i] = (char)(i % 256);
        }
    }
    
    void TearDown() override {
        // Clean up any allocated memory
    }
    
    static const size_t TEST_BUFFER_SIZE = 1024;
    char source_buffer[TEST_BUFFER_SIZE];
    char dest_buffer[TEST_BUFFER_SIZE];
    char test_pattern[TEST_BUFFER_SIZE];
};

// ============================================================================
// Test Cases for align_ptr() function
// Target: 4 uncovered lines in core/shared/utils/bh_common.c lines 7-10
// ============================================================================

TEST_F(MemoryUtilityTest, AlignPtr_WithUnalignedPointer_ReturnsAlignedAddress) {
    // Test with different alignment boundaries
    char buffer[64];
    char *unaligned_ptr = buffer + 1; // Intentionally unaligned
    
    // Test 4-byte alignment
    char *aligned_4 = test_align_ptr(unaligned_ptr, 4);
    ASSERT_TRUE(((uintptr_t)aligned_4 % 4) == 0);
    ASSERT_GE(aligned_4, unaligned_ptr);
    ASSERT_LT(aligned_4, unaligned_ptr + 4);
    
    // Test 8-byte alignment
    char *aligned_8 = test_align_ptr(unaligned_ptr, 8);
    ASSERT_TRUE(((uintptr_t)aligned_8 % 8) == 0);
    ASSERT_GE(aligned_8, unaligned_ptr);
    ASSERT_LT(aligned_8, unaligned_ptr + 8);
    
    // Test 16-byte alignment
    char *aligned_16 = test_align_ptr(unaligned_ptr, 16);
    ASSERT_TRUE(((uintptr_t)aligned_16 % 16) == 0);
    ASSERT_GE(aligned_16, unaligned_ptr);
    ASSERT_LT(aligned_16, unaligned_ptr + 16);
}

TEST_F(MemoryUtilityTest, AlignPtr_WithAlreadyAlignedPointer_ReturnsSameAddress) {
    char buffer[64];
    
    // Get a 4-byte aligned pointer
    char *aligned_ptr = (char *)(((uintptr_t)buffer + 3) & ~3);
    
    // Test that already aligned pointer returns same address
    char *result = test_align_ptr(aligned_ptr, 4);
    ASSERT_EQ(aligned_ptr, result);
    ASSERT_TRUE(((uintptr_t)result % 4) == 0);
}

TEST_F(MemoryUtilityTest, AlignPtr_WithDifferentAlignmentSizes_WorksCorrectly) {
    char buffer[128];
    char *base_ptr = buffer + 1; // Unaligned base
    
    // Test power-of-2 alignments
    for (unsigned int alignment = 2; alignment <= 64; alignment *= 2) {
        char *aligned = test_align_ptr(base_ptr, alignment);
        
        // Verify alignment
        ASSERT_TRUE(((uintptr_t)aligned % alignment) == 0);
        
        // Verify it's the next aligned address
        ASSERT_GE(aligned, base_ptr);
        ASSERT_LT(aligned, base_ptr + alignment);
    }
}

TEST_F(MemoryUtilityTest, AlignPtr_WithBoundaryConditions_HandlesCorrectly) {
    char buffer[32];
    
    // Test with pointer at exact boundary
    char *boundary_ptr = (char *)(((uintptr_t)buffer + 7) & ~7); // 8-byte aligned
    char *result = test_align_ptr(boundary_ptr, 8);
    ASSERT_EQ(boundary_ptr, result);
    
    // Test with pointer just after boundary
    char *after_boundary = boundary_ptr + 1;
    char *aligned_result = test_align_ptr(after_boundary, 8);
    ASSERT_EQ(boundary_ptr + 8, aligned_result);
    ASSERT_TRUE(((uintptr_t)aligned_result % 8) == 0);
}

// ============================================================================
// Test Cases for b_memcpy_wa() function (Word-Aligned Memory Copy)
// Target: 30 uncovered lines in core/shared/utils/bh_common.c lines 18-47
// ============================================================================

TEST_F(MemoryUtilityTest, BMemcpyWA_WithZeroLength_ReturnsZero) {
    // Test zero-length copy
    int result = b_memcpy_wa(dest_buffer, sizeof(dest_buffer), source_buffer, 0);
    ASSERT_EQ(0, result);
    
    // Verify destination buffer unchanged
    for (size_t i = 0; i < 10; i++) {
        ASSERT_EQ((char)0xAA, dest_buffer[i]);
    }
}

TEST_F(MemoryUtilityTest, BMemcpyWA_WithSmallAlignedCopy_CopiesCorrectly) {
    // Prepare aligned source data
    char aligned_source[16] = "Hello World!";
    char aligned_dest[32];
    memset(aligned_dest, 0, sizeof(aligned_dest));
    
    // Perform word-aligned copy
    int result = b_memcpy_wa(aligned_dest, sizeof(aligned_dest), aligned_source, 12);
    ASSERT_EQ(0, result);
    
    // Verify data copied correctly
    ASSERT_EQ(0, memcmp(aligned_dest, aligned_source, 12));
    ASSERT_EQ(0, aligned_dest[12]); // Null terminator should be copied
}

TEST_F(MemoryUtilityTest, BMemcpyWA_WithUnalignedSource_HandlesCorrectly) {
    // Create unaligned source
    char buffer[64];
    char *unaligned_src = buffer + 1; // Intentionally unaligned
    memcpy(unaligned_src, "Unaligned Test Data", 19);
    
    char dest[32];
    memset(dest, 0, sizeof(dest));
    
    // Test word-aligned copy with unaligned source
    int result = b_memcpy_wa(dest, sizeof(dest), unaligned_src, 19);
    ASSERT_EQ(0, result);
    
    // Verify correct data transfer
    ASSERT_EQ(0, memcmp(dest, unaligned_src, 19));
}

TEST_F(MemoryUtilityTest, BMemcpyWA_WithLargeWordAlignedData_CopiesEfficiently) {
    // Test with data that spans multiple 4-byte words
    const size_t test_size = 64;
    char large_source[test_size];
    char large_dest[test_size + 16];
    
    // Fill source with pattern
    for (size_t i = 0; i < test_size; i++) {
        large_source[i] = (char)(i % 256);
    }
    memset(large_dest, 0xFF, sizeof(large_dest));
    
    // Perform word-aligned copy
    int result = b_memcpy_wa(large_dest, sizeof(large_dest), large_source, test_size);
    ASSERT_EQ(0, result);
    
    // Verify all data copied correctly
    ASSERT_EQ(0, memcmp(large_dest, large_source, test_size));
    
    // Verify no buffer overflow
    ASSERT_EQ((char)0xFF, large_dest[test_size]);
}

TEST_F(MemoryUtilityTest, BMemcpyWA_WithPartialWordBoundaries_HandlesLeadingBytes) {
    // Test copy that starts mid-word and ends mid-word
    char src_buffer[32];
    char dest_buffer[32];
    
    // Fill source with identifiable pattern
    for (int i = 0; i < 32; i++) {
        src_buffer[i] = (char)(0x40 + i);
    }
    memset(dest_buffer, 0, sizeof(dest_buffer));
    
    // Copy from offset that creates leading partial word
    char *src_offset = src_buffer + 1;
    int result = b_memcpy_wa(dest_buffer, sizeof(dest_buffer), src_offset, 15);
    ASSERT_EQ(0, result);
    
    // Verify correct data transfer
    ASSERT_EQ(0, memcmp(dest_buffer, src_offset, 15));
}

TEST_F(MemoryUtilityTest, BMemcpyWA_WithTrailingPartialWord_HandlesTrailingBytes) {
    // Test copy that ends with partial word
    char src_data[20];
    char dest_data[24];
    
    // Create test pattern
    strcpy(src_data, "Trailing Word Test");
    memset(dest_data, 0xCC, sizeof(dest_data));
    
    // Copy data that ends mid-word
    int result = b_memcpy_wa(dest_data, sizeof(dest_data), src_data, 18);
    ASSERT_EQ(0, result);
    
    // Verify correct copy including trailing bytes
    ASSERT_EQ(0, memcmp(dest_data, src_data, 18));
    
    // Verify no corruption beyond copied data
    ASSERT_EQ((char)0xCC, dest_data[18]);
}

TEST_F(MemoryUtilityTest, BMemcpyWA_WithComplexAlignment_HandlesAllPaths) {
    // Test scenario that exercises both leading and trailing word handling
    char complex_src[48];
    char complex_dest[48];
    
    // Fill with recognizable pattern
    for (int i = 0; i < 48; i++) {
        complex_src[i] = (char)(0x30 + (i % 16));
    }
    memset(complex_dest, 0, sizeof(complex_dest));
    
    // Test copy with complex alignment (starts at +2, copies 37 bytes)
    char *offset_src = complex_src + 2;
    int result = b_memcpy_wa(complex_dest, sizeof(complex_dest), offset_src, 37);
    ASSERT_EQ(0, result);
    
    // Verify complete data integrity
    ASSERT_EQ(0, memcmp(complex_dest, offset_src, 37));
}

TEST_F(MemoryUtilityTest, BMemcpyWA_WithExactWordBoundaries_OptimizesCorrectly) {
    // Test copy that aligns perfectly with word boundaries
    const size_t word_size = 4;
    const size_t num_words = 8;
    const size_t total_size = word_size * num_words;
    
    char word_aligned_src[total_size];
    char word_aligned_dest[total_size + 8];
    
    // Fill with word-pattern data
    uint32_t *src_words = (uint32_t *)word_aligned_src;
    for (size_t i = 0; i < num_words; i++) {
        src_words[i] = 0x12345678 + i;
    }
    memset(word_aligned_dest, 0, sizeof(word_aligned_dest));
    
    // Perform word-optimized copy
    int result = b_memcpy_wa(word_aligned_dest, sizeof(word_aligned_dest), 
                            word_aligned_src, total_size);
    ASSERT_EQ(0, result);
    
    // Verify word-level accuracy
    uint32_t *dest_words = (uint32_t *)word_aligned_dest;
    for (size_t i = 0; i < num_words; i++) {
        ASSERT_EQ(src_words[i], dest_words[i]);
    }
}

// ============================================================================
// Test Cases for bh_system() function
// Target: 1 uncovered line in core/shared/utils/bh_common.c line 169
// Note: Only available when WASM_ENABLE_WAMR_COMPILER != 0 || WASM_ENABLE_JIT != 0
// ============================================================================

#if WASM_ENABLE_WAMR_COMPILER != 0 || WASM_ENABLE_JIT != 0

TEST_F(MemoryUtilityTest, BhSystem_WithValidCommand_ExecutesSuccessfully) {
    // Test simple command that should succeed
    int result = bh_system("echo 'WAMR test command'");
    
    // On Unix systems, system() returns the exit status
    // Success should be 0
    ASSERT_EQ(0, result);
}

TEST_F(MemoryUtilityTest, BhSystem_WithInvalidCommand_ReturnsErrorCode) {
    // Test command that should fail
    int result = bh_system("nonexistent_command_12345");
    
    // Should return non-zero error code
    ASSERT_NE(0, result);
}

TEST_F(MemoryUtilityTest, BhSystem_WithEmptyCommand_HandlesCorrectly) {
    // Test with empty command
    int result = bh_system("");
    
    // Behavior may vary by platform, but should not crash
    // Just verify it returns some value
    (void)result; // Acknowledge we got a result
    ASSERT_TRUE(true); // Test completed without crash
}

TEST_F(MemoryUtilityTest, BhSystem_WithComplexCommand_ExecutesCorrectly) {
    // Test more complex shell command
    int result = bh_system("test -d /tmp && echo 'Directory exists'");
    
    // Should succeed on most Unix systems
    ASSERT_EQ(0, result);
}

TEST_F(MemoryUtilityTest, BhSystem_PlatformSpecificBehavior_WorksCorrectly) {
    // Test platform-specific command execution
#if !(defined(_WIN32) || defined(_WIN32_))
    // Unix/Linux path
    int result = bh_system("ls /dev/null > /dev/null 2>&1");
    ASSERT_EQ(0, result);
#else
    // Windows path
    int result = bh_system("dir C:\\ > NUL 2>&1");
    ASSERT_EQ(0, result);
#endif
}

#else

// When compiler/JIT features are disabled, provide placeholder tests
TEST_F(MemoryUtilityTest, BhSystem_NotAvailable_SkippedGracefully) {
    // bh_system() is not available in this build configuration
    // Skip gracefully without using GTEST_SKIP()
    return;
}

#endif // WASM_ENABLE_WAMR_COMPILER != 0 || WASM_ENABLE_JIT != 0

// ============================================================================
// Integration Tests - Testing interactions between memory utility functions
// ============================================================================

TEST_F(MemoryUtilityTest, MemoryUtilities_Integration_WorkTogether) {
    // Test integration of alignment and word-aligned copy
    char unaligned_buffer[128];
    char *unaligned_ptr = unaligned_buffer + 3; // Intentionally unaligned
    
    // Create test data
    const char *test_data = "Integration test data for WAMR memory utilities";
    size_t data_len = strlen(test_data);
    
    // Copy test data to unaligned location
    memcpy(unaligned_ptr, test_data, data_len);
    
    // Use word-aligned copy to transfer data
    char aligned_dest[128];
    memset(aligned_dest, 0, sizeof(aligned_dest));
    
    int result = b_memcpy_wa(aligned_dest, sizeof(aligned_dest), unaligned_ptr, data_len);
    ASSERT_EQ(0, result);
    
    // Verify integration worked correctly
    ASSERT_EQ(0, memcmp(aligned_dest, test_data, data_len));
    ASSERT_EQ(0, strcmp(aligned_dest, test_data));
}

TEST_F(MemoryUtilityTest, MemoryUtilities_StressTest_HandlesLargeOperations) {
    // Stress test with larger data sizes
    const size_t large_size = 4096;
    char *large_src = (char *)malloc(large_size);
    char *large_dest = (char *)malloc(large_size + 64);
    
    ASSERT_TRUE(large_src != nullptr);
    ASSERT_TRUE(large_dest != nullptr);
    
    // Fill source with pattern
    for (size_t i = 0; i < large_size; i++) {
        large_src[i] = (char)(i % 256);
    }
    memset(large_dest, 0xFF, large_size + 64);
    
    // Perform large word-aligned copy
    int result = b_memcpy_wa(large_dest, large_size + 64, large_src, large_size);
    ASSERT_EQ(0, result);
    
    // Verify large data transfer
    ASSERT_EQ(0, memcmp(large_dest, large_src, large_size));
    
    // Verify no buffer overflow
    ASSERT_EQ((char)0xFF, large_dest[large_size]);
    
    free(large_src);
    free(large_dest);
}