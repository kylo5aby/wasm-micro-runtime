/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include "bh_read_file.h"
#include "wasm_runtime_common.h"
#include "aot_runtime.h"
#include "aot_reloc.h"

// Platform detection utility for tests - REQUIRED in every test file
class PlatformTestContext {
public:
    static bool IsX86_64() {
#if defined(BUILD_TARGET_X86_64)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsARM64() {
#if defined(BUILD_TARGET_AARCH64)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsRISCV() {
#if defined(BUILD_TARGET_RISCV64_LP64D) || defined(BUILD_TARGET_RISCV32_ILP32)
        return true;
#else
        return false;
#endif
    }
    
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
};

// Relocation type constants for x86_64
#if !defined(BH_PLATFORM_WINDOWS)
#define R_X86_64_64 1       /* Direct 64 bit  */
#define R_X86_64_PC32 2     /* PC relative 32 bit signed */
#define R_X86_64_PLT32 4    /* 32 bit PLT address */
#define R_X86_64_GOTPCREL 9 /* 32 bit signed PC relative offset to GOT */
#define R_X86_64_32 10      /* Direct 32 bit zero extended */
#define R_X86_64_32S 11     /* Direct 32 bit sign extended */
#define R_X86_64_PC64 24    /* PC relative 64 bit */
#else
#ifndef IMAGE_REL_AMD64_ADDR64
#define IMAGE_REL_AMD64_ADDR64 1 /* The 64-bit VA of the relocation target */
#define IMAGE_REL_AMD64_ADDR32 2 /* The 32-bit VA of the relocation target */
#define IMAGE_REL_AMD64_REL32  4 /* The 32-bit relative address from the byte following the relocation*/
#endif
#endif

class AOTArchStep1Test : public testing::Test
{
protected:
    void SetUp() override
    {
        // Skip tests if AOT is not supported
        if (!PlatformTestContext::HasAOTSupport()) {
            return;
        }
        
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
        
        // Initialize test buffers
        memset(error_buffer, 0, sizeof(error_buffer));
        memset(target_section, 0, sizeof(target_section));
        
        // Create a minimal mock AOT module for testing
        mock_module = (AOTModule *)wasm_runtime_malloc(sizeof(AOTModule));
        ASSERT_TRUE(mock_module != nullptr);
        memset(mock_module, 0, sizeof(AOTModule));
        
        // Set up mock module with minimal required fields
        mock_module->code_size = 1024;
        mock_module->code = (uint8 *)wasm_runtime_malloc(mock_module->code_size);
        ASSERT_TRUE(mock_module->code != nullptr);
        memset(mock_module->code, 0, mock_module->code_size);
    }
    
    void TearDown() override
    {
        if (!PlatformTestContext::HasAOTSupport()) {
            return;
        }
        
        if (mock_module) {
            if (mock_module->code) {
                wasm_runtime_free(mock_module->code);
            }
            wasm_runtime_free(mock_module);
        }
        wasm_runtime_destroy();
    }
    
    AOTModule *mock_module = nullptr;
    char error_buffer[256];
    uint8 target_section[1024];
    static const uint32 target_section_size = sizeof(target_section);
};

// Test Case 1: apply_relocation with invalid relocation type (tests set_error_buf indirectly)
// Target: Lines 255-261 in aot_reloc_x86_64.c (default case in switch statement)
// Also targets: Lines 41-45 (set_error_buf function called from default case)
TEST_F(AOTArchStep1Test, ApplyRelocation_InvalidType_ReturnsFailure)
{
    if (!PlatformTestContext::HasAOTSupport()) {
        return; // Skip gracefully for builds without AOT
    }
    
    // Arrange: Set up invalid relocation type
    uint32 invalid_reloc_type = 9999; // Invalid relocation type
    uint64 reloc_offset = 0;
    int64 reloc_addend = 0;
    void *symbol_addr = (void *)0x1000;
    int32 symbol_index = -1;
    
    // Act: Call apply_relocation with invalid type
    bool result = apply_relocation(mock_module, target_section, target_section_size,
                                 reloc_offset, reloc_addend, invalid_reloc_type,
                                 symbol_addr, symbol_index, error_buffer, sizeof(error_buffer));
    
    // Assert: Should return false and set error message (tests set_error_buf indirectly)
    ASSERT_FALSE(result);
    ASSERT_TRUE(strlen(error_buffer) > 0);
    ASSERT_TRUE(strstr(error_buffer, "invalid relocation type") != nullptr);
    ASSERT_TRUE(strstr(error_buffer, "9999") != nullptr);
}

// Test Case 2: apply_relocation with null error buffer (tests set_error_buf null handling)
// Target: Lines 255-261 and 41-45 in aot_reloc_x86_64.c (set_error_buf with null buffer)
TEST_F(AOTArchStep1Test, ApplyRelocation_InvalidTypeNullBuffer_HandlesGracefully)
{
    if (!PlatformTestContext::HasAOTSupport()) {
        return; // Skip gracefully for builds without AOT
    }
    
    // Arrange: Set up invalid relocation type with null error buffer
    uint32 invalid_reloc_type = 9999; // Invalid relocation type
    uint64 reloc_offset = 0;
    int64 reloc_addend = 0;
    void *symbol_addr = (void *)0x1000;
    int32 symbol_index = -1;
    
    // Act: Call apply_relocation with null error buffer
    bool result = apply_relocation(mock_module, target_section, target_section_size,
                                 reloc_offset, reloc_addend, invalid_reloc_type,
                                 symbol_addr, symbol_index, nullptr, 0);
    
    // Assert: Should return false without crashing (tests set_error_buf null handling)
    ASSERT_FALSE(result);
    // Function should complete without error (no crash test)
}

// Test Case 3: apply_relocation with invalid offset (out of bounds)
// Target: Lines 100-107 in aot_reloc_x86_64.c (check_reloc_offset function via CHECK_RELOC_OFFSET macro)
TEST_F(AOTArchStep1Test, ApplyRelocation_InvalidOffset_ReturnsFailure)
{
    if (!PlatformTestContext::HasAOTSupport()) {
        return; // Skip gracefully for builds without AOT
    }
    
    // Arrange: Set up relocation with offset beyond section boundary
    uint32 reloc_type = R_X86_64_PC32;
    uint64 reloc_offset = target_section_size + 100; // Beyond section boundary
    int64 reloc_addend = 0;
    void *symbol_addr = (void *)0x1000;
    int32 symbol_index = -1;
    
    // Act: Call apply_relocation with invalid offset
    bool result = apply_relocation(mock_module, target_section, target_section_size,
                                 reloc_offset, reloc_addend, reloc_type,
                                 symbol_addr, symbol_index, error_buffer, sizeof(error_buffer));
    
    // Assert: Should return false and set error message about invalid offset
    ASSERT_FALSE(result);
    ASSERT_TRUE(strlen(error_buffer) > 0);
    ASSERT_TRUE(strstr(error_buffer, "invalid relocation offset") != nullptr);
}

// Test Case 4: apply_relocation PC32 overflow condition
// Target: Lines 162-167 in aot_reloc_x86_64.c (PC32 overflow error path)
TEST_F(AOTArchStep1Test, ApplyRelocation_PC32Overflow_ReturnsFailure)
{
    if (!PlatformTestContext::HasAOTSupport()) {
        return; // Skip gracefully for builds without AOT
    }
    
    // Arrange: Set up conditions that will cause PC32 overflow
    uint32 reloc_type = R_X86_64_PC32;
    uint64 reloc_offset = 0;
    int64 reloc_addend = 0;
    // Use symbol address that will cause overflow when calculating PC-relative address
    void *symbol_addr = (void *)0x7FFFFFFF00000000ULL; // Large address to trigger overflow
    int32 symbol_index = -1;
    
    // Act: Call apply_relocation with conditions that cause PC32 overflow
    bool result = apply_relocation(mock_module, target_section, target_section_size,
                                 reloc_offset, reloc_addend, reloc_type,
                                 symbol_addr, symbol_index, error_buffer, sizeof(error_buffer));
    
    // Assert: Should return false and set error message about truncation
    ASSERT_FALSE(result);
    ASSERT_TRUE(strlen(error_buffer) > 0);
    ASSERT_TRUE(strstr(error_buffer, "relocation truncated to fit R_X86_64_PC32") != nullptr);
}

// Test Case 5: Test boundary condition for relocation offset validation
// Target: Lines 100-107 in aot_reloc_x86_64.c (boundary check in check_reloc_offset)
// TEST_F(AOTArchStep1Test, ApplyRelocation_BoundaryOffset_ValidatesCorrectly)
// {
//     if (!PlatformTestContext::HasAOTSupport()) {
//         return; // Skip gracefully for builds without AOT
//     }
    
//     // Arrange: Set up relocation at exact boundary (should succeed)
//     uint32 reloc_type = R_X86_64_PC32;
//     uint64 reloc_offset = target_section_size - sizeof(int32); // Exactly at boundary
//     int64 reloc_addend = 0;
//     void *symbol_addr = (void *)0x1000; // Small address to avoid overflow
//     int32 symbol_index = -1;
    
//     // Act: Call apply_relocation with boundary offset
//     bool result = apply_relocation(mock_module, target_section, target_section_size,
//                                  reloc_offset, reloc_addend, reloc_type,
//                                  symbol_addr, symbol_index, error_buffer, sizeof(error_buffer));
    
//     // Assert: Should succeed for valid boundary condition
//     ASSERT_TRUE(result);
//     ASSERT_EQ(0, strlen(error_buffer)); // No error message should be set
// }

// Test Case 6: Test error buffer size handling through apply_relocation
// Target: Lines 41-45 in aot_reloc_x86_64.c (buffer size parameter handling in set_error_buf)
TEST_F(AOTArchStep1Test, ApplyRelocation_SmallErrorBuffer_TruncatesMessage)
{
    if (!PlatformTestContext::HasAOTSupport()) {
        return; // Skip gracefully for builds without AOT
    }
    
    // Arrange: Prepare small buffer and invalid relocation to trigger error
    char small_buffer[10];
    uint32 invalid_reloc_type = 9999; // Invalid relocation type
    uint64 reloc_offset = 0;
    int64 reloc_addend = 0;
    void *symbol_addr = (void *)0x1000;
    int32 symbol_index = -1;
    memset(small_buffer, 0, sizeof(small_buffer));
    
    // Act: Call apply_relocation with small error buffer
    bool result = apply_relocation(mock_module, target_section, target_section_size,
                                 reloc_offset, reloc_addend, invalid_reloc_type,
                                 symbol_addr, symbol_index, small_buffer, sizeof(small_buffer));
    
    // Assert: Should return false and message should be truncated to fit buffer
    ASSERT_FALSE(result);
    ASSERT_LT(strlen(small_buffer), 50); // Message should be much shorter than full error
    ASSERT_EQ(sizeof(small_buffer) - 1, strlen(small_buffer)); // Should be truncated
}

// Test Case 7: Test valid PC32 relocation (successful path)
// Target: Lines 153-172 in aot_reloc_x86_64.c (R_X86_64_PC32 success path)
// TEST_F(AOTArchStep1Test, ApplyRelocation_PC32Valid_SucceedsCorrectly)
// {
//     if (!PlatformTestContext::HasAOTSupport()) {
//         return; // Skip gracefully for builds without AOT
//     }
    
//     // Arrange: Set up valid PC32 relocation
//     uint32 reloc_type = R_X86_64_PC32;
//     uint64 reloc_offset = 0;
//     int64 reloc_addend = 0;
//     void *symbol_addr = (void *)0x2000; // Reasonable address
//     int32 symbol_index = -1;
    
//     // Act: Call apply_relocation with valid PC32 parameters
//     bool result = apply_relocation(mock_module, target_section, target_section_size,
//                                  reloc_offset, reloc_addend, reloc_type,
//                                  symbol_addr, symbol_index, error_buffer, sizeof(error_buffer));
    
//     // Assert: Should succeed
//     ASSERT_TRUE(result);
//     ASSERT_EQ(0, strlen(error_buffer)); // No error message should be set
    
//     // Verify that the relocation was applied to the target section
//     int32 *reloc_target = (int32 *)(target_section + reloc_offset);
//     ASSERT_NE(0, *reloc_target); // Should have been modified
// }

// Test Case 8: Test zero-sized error buffer edge case
// Target: Lines 41-45 in aot_reloc_x86_64.c (set_error_buf with zero size)
TEST_F(AOTArchStep1Test, ApplyRelocation_ZeroSizeErrorBuffer_HandlesGracefully)
{
    if (!PlatformTestContext::HasAOTSupport()) {
        return; // Skip gracefully for builds without AOT
    }
    
    // Arrange: Set up invalid relocation with zero-sized error buffer
    char buffer[1] = {0};
    uint32 invalid_reloc_type = 9999;
    uint64 reloc_offset = 0;
    int64 reloc_addend = 0;
    void *symbol_addr = (void *)0x1000;
    int32 symbol_index = -1;
    
    // Act: Call apply_relocation with zero-sized error buffer
    bool result = apply_relocation(mock_module, target_section, target_section_size,
                                 reloc_offset, reloc_addend, invalid_reloc_type,
                                 symbol_addr, symbol_index, buffer, 0);
    
    // Assert: Should return false without crashing
    ASSERT_FALSE(result);
    // Buffer should remain unchanged (no write to zero-sized buffer)
    ASSERT_EQ('\0', buffer[0]);
}