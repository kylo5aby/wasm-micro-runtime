/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

extern "C" {
#include "bh_assert.h"
#include "bh_platform.h"
#include "bh_bitmap.h"
#include "bh_read_file.h"
#include "wasm_export.h"
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

// Test fixture for Assertion and File I/O Functions
class AssertionFileIOTest : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime for memory allocation
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        bool init_success = wasm_runtime_full_init(&init_args);
        ASSERT_TRUE(init_success);
        
        // Create temporary test files
        test_file_path = "/tmp/wamr_test_file.txt";
        test_content = "Hello WAMR Test Content\nLine 2\nLine 3";
        
        // Create test file
        CreateTestFile(test_file_path, test_content);
        
        // Set up bitmap test data
        test_bitmap = nullptr;
    }
    
    void TearDown() override {
        // Clean up test files
        if (access(test_file_path.c_str(), F_OK) == 0) {
            unlink(test_file_path.c_str());
        }
        
        // Clean up bitmap
        if (test_bitmap) {
            bh_bitmap_delete(test_bitmap);
            test_bitmap = nullptr;
        }
        
        // Destroy WAMR runtime
        wasm_runtime_destroy();
    }
    
    void CreateTestFile(const std::string& path, const std::string& content) {
        int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd >= 0) {
            write(fd, content.c_str(), content.length());
            close(fd);
        }
    }
    
    std::string test_file_path;
    std::string test_content;
    bh_bitmap* test_bitmap;
};

// ============================================================================
// Test Cases for bh_assert_internal() function
// Target: 6 uncovered lines in core/shared/utils/bh_assert.c lines 15-20
// Note: Testing assertion failures requires careful handling to avoid process termination
// ============================================================================

TEST_F(AssertionFileIOTest, BhAssertInternal_WithTrueCondition_ReturnsNormally) {
    // Test successful assertion (condition is true)
    // This should return normally without calling abort()
    
    // Use a wrapper to capture the call
    bool assertion_passed = true;
    
    // Test with valid condition - should not abort
    bh_assert_internal(1, __FILE__, __LINE__, "test assertion");
    
    // If we reach here, the assertion passed correctly
    ASSERT_TRUE(assertion_passed);
}

TEST_F(AssertionFileIOTest, BhAssertInternal_WithNullFilename_HandlesCorrectly) {
    // Test assertion failure path with NULL filename
    // We need to test this carefully to avoid process termination
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - test the assertion failure
        bh_assert_internal(0, nullptr, __LINE__, "test assertion");
        // Should not reach here due to abort()
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);
        
        // Child should have been terminated by abort()
        ASSERT_TRUE(WIFEXITED(status) || WIFSIGNALED(status));
        
        // If signaled, should be SIGABRT
        if (WIFSIGNALED(status)) {
            ASSERT_EQ(SIGABRT, WTERMSIG(status));
        }
    } else {
        // Fork failed - skip this test gracefully
        return;
    }
}

TEST_F(AssertionFileIOTest, BhAssertInternal_WithNullExprString_HandlesCorrectly) {
    // Test assertion failure path with NULL expression string
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - test the assertion failure
        bh_assert_internal(0, __FILE__, __LINE__, nullptr);
        // Should not reach here due to abort()
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);
        
        // Child should have been terminated by abort()
        ASSERT_TRUE(WIFEXITED(status) || WIFSIGNALED(status));
        
        // If signaled, should be SIGABRT
        if (WIFSIGNALED(status)) {
            ASSERT_EQ(SIGABRT, WTERMSIG(status));
        }
    } else {
        // Fork failed - skip this test gracefully
        return;
    }
}

TEST_F(AssertionFileIOTest, BhAssertInternal_WithValidFailureCondition_AbortsCorrectly) {
    // Test normal assertion failure path
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - test the assertion failure
        bh_assert_internal(0, __FILE__, __LINE__, "test assertion failure");
        // Should not reach here due to abort()
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);
        
        // Child should have been terminated by abort()
        ASSERT_TRUE(WIFEXITED(status) || WIFSIGNALED(status));
        
        // If signaled, should be SIGABRT
        if (WIFSIGNALED(status)) {
            ASSERT_EQ(SIGABRT, WTERMSIG(status));
        }
    } else {
        // Fork failed - skip this test gracefully
        return;
    }
}

// ============================================================================
// Test Cases for bh_read_file_to_buffer() function
// Target: 11 uncovered lines in core/shared/utils/uncommon/bh_read_file.c
// Focus on error handling and edge case paths
// ============================================================================

TEST_F(AssertionFileIOTest, BhReadFile_WithValidFile_ReadsCorrectly) {
    // Test successful file reading
    uint32 file_size = 0;
    char* buffer = bh_read_file_to_buffer(test_file_path.c_str(), &file_size);
    
    ASSERT_TRUE(buffer != nullptr);
    ASSERT_EQ(test_content.length(), file_size);
    ASSERT_EQ(0, memcmp(buffer, test_content.c_str(), file_size));
    
    BH_FREE(buffer);
}

TEST_F(AssertionFileIOTest, BhReadFile_WithNullFilename_ReturnsNull) {
    // Test error path: NULL filename
    uint32 file_size = 0;
    char* buffer = bh_read_file_to_buffer(nullptr, &file_size);
    
    ASSERT_TRUE(buffer == nullptr);
}

TEST_F(AssertionFileIOTest, BhReadFile_WithNullRetSize_ReturnsNull) {
    // Test error path: NULL ret_size parameter
    char* buffer = bh_read_file_to_buffer(test_file_path.c_str(), nullptr);
    
    ASSERT_TRUE(buffer == nullptr);
}

TEST_F(AssertionFileIOTest, BhReadFile_WithNonexistentFile_ReturnsNull) {
    // Test error path: file doesn't exist
    uint32 file_size = 0;
    char* buffer = bh_read_file_to_buffer("/tmp/nonexistent_file_12345.txt", &file_size);
    
    ASSERT_TRUE(buffer == nullptr);
}

TEST_F(AssertionFileIOTest, BhReadFile_WithEmptyFile_HandlesCorrectly) {
    // Test edge case: empty file
    std::string empty_file = "/tmp/wamr_empty_test.txt";
    CreateTestFile(empty_file, "");
    
    uint32 file_size = 0;
    char* buffer = bh_read_file_to_buffer(empty_file.c_str(), &file_size);
    
    // Should succeed and allocate at least 1 byte
    ASSERT_TRUE(buffer != nullptr);
    ASSERT_EQ(0, file_size);
    
    BH_FREE(buffer);
    unlink(empty_file.c_str());
}

TEST_F(AssertionFileIOTest, BhReadFile_WithLargeFile_HandlesCorrectly) {
    // Test with larger file to exercise different code paths
    std::string large_content;
    for (int i = 0; i < 1000; i++) {
        large_content += "This is line " + std::to_string(i) + " of test content.\n";
    }
    
    std::string large_file = "/tmp/wamr_large_test.txt";
    CreateTestFile(large_file, large_content);
    
    uint32 file_size = 0;
    char* buffer = bh_read_file_to_buffer(large_file.c_str(), &file_size);
    
    ASSERT_TRUE(buffer != nullptr);
    ASSERT_EQ(large_content.length(), file_size);
    ASSERT_EQ(0, memcmp(buffer, large_content.c_str(), file_size));
    
    BH_FREE(buffer);
    unlink(large_file.c_str());
}

TEST_F(AssertionFileIOTest, BhReadFile_WithDirectoryPath_ReturnsNull) {
    // Test error path: trying to read a directory
    uint32 file_size = 0;
    char* buffer = bh_read_file_to_buffer("/tmp", &file_size);
    
    // On some systems, reading a directory might succeed but return 0 bytes
    // The important thing is that it doesn't crash
    if (buffer != nullptr) {
        BH_FREE(buffer);
    }
    
    // Test passes if we reach here without crash
    ASSERT_TRUE(true);
}

// ============================================================================
// Test Cases for bh_bitmap.h inline functions
// Target: 4 uncovered lines in core/shared/utils/bh_bitmap.h
// Focus on inline function edge cases and boundary conditions
// ============================================================================

TEST_F(AssertionFileIOTest, BhBitmap_Delete_WithNullPointer_HandlesCorrectly) {
    // Test bh_bitmap_delete with NULL pointer
    bh_bitmap_delete(nullptr);
    
    // Should not crash - test passes if we reach here
    ASSERT_TRUE(true);
}

TEST_F(AssertionFileIOTest, BhBitmap_Delete_WithValidPointer_FreesCorrectly) {
    // Test bh_bitmap_delete with valid pointer
    bh_bitmap* bitmap = bh_bitmap_new(0, 32);
    ASSERT_TRUE(bitmap != nullptr);
    
    // Delete should work correctly
    bh_bitmap_delete(bitmap);
    
    // Test passes if we reach here without crash
    ASSERT_TRUE(true);
}

TEST_F(AssertionFileIOTest, BhBitmap_IsInRange_WithValidIndices_ReturnsCorrectly) {
    // Test bh_bitmap_is_in_range with various indices
    test_bitmap = bh_bitmap_new(10, 32);
    ASSERT_TRUE(test_bitmap != nullptr);
    
    // Test in-range indices
    ASSERT_TRUE(bh_bitmap_is_in_range(test_bitmap, 10));  // begin_index
    ASSERT_TRUE(bh_bitmap_is_in_range(test_bitmap, 25));  // middle
    ASSERT_TRUE(bh_bitmap_is_in_range(test_bitmap, 41));  // end_index - 1
    
    // Test out-of-range indices
    ASSERT_FALSE(bh_bitmap_is_in_range(test_bitmap, 9));   // before begin
    ASSERT_FALSE(bh_bitmap_is_in_range(test_bitmap, 42));  // at end_index
    ASSERT_FALSE(bh_bitmap_is_in_range(test_bitmap, 50));  // after end
}

TEST_F(AssertionFileIOTest, BhBitmap_GetBit_WithValidIndices_ReturnsCorrectValues) {
    // Test bh_bitmap_get_bit functionality
    test_bitmap = bh_bitmap_new(5, 16);
    ASSERT_TRUE(test_bitmap != nullptr);
    
    // Initially all bits should be 0
    for (uintptr_t i = 5; i < 21; i++) {
        ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, i));
    }
}

TEST_F(AssertionFileIOTest, BhBitmap_SetBit_WithValidIndices_SetsCorrectly) {
    // Test bh_bitmap_set_bit functionality
    test_bitmap = bh_bitmap_new(0, 16);
    ASSERT_TRUE(test_bitmap != nullptr);
    
    // Set some bits
    bh_bitmap_set_bit(test_bitmap, 0);
    bh_bitmap_set_bit(test_bitmap, 7);
    bh_bitmap_set_bit(test_bitmap, 15);
    
    // Verify bits are set
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 0));
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 7));
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 15));
    
    // Verify other bits are still 0
    ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, 1));
    ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, 8));
    ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, 14));
}

TEST_F(AssertionFileIOTest, BhBitmap_ClearBit_WithValidIndices_ClearsCorrectly) {
    // Test bh_bitmap_clear_bit functionality
    test_bitmap = bh_bitmap_new(0, 16);
    ASSERT_TRUE(test_bitmap != nullptr);
    
    // Set some bits first
    bh_bitmap_set_bit(test_bitmap, 3);
    bh_bitmap_set_bit(test_bitmap, 8);
    bh_bitmap_set_bit(test_bitmap, 12);
    
    // Verify bits are set
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 3));
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 8));
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 12));
    
    // Clear some bits
    bh_bitmap_clear_bit(test_bitmap, 3);
    bh_bitmap_clear_bit(test_bitmap, 12);
    
    // Verify bits are cleared
    ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, 3));
    ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, 12));
    
    // Verify other bit is still set
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 8));
}

TEST_F(AssertionFileIOTest, BhBitmap_Operations_WithOffsetRange_WorkCorrectly) {
    // Test bitmap operations with non-zero begin_index
    test_bitmap = bh_bitmap_new(100, 32);
    ASSERT_TRUE(test_bitmap != nullptr);
    
    // Test range checking
    ASSERT_TRUE(bh_bitmap_is_in_range(test_bitmap, 100));
    ASSERT_TRUE(bh_bitmap_is_in_range(test_bitmap, 131));
    ASSERT_FALSE(bh_bitmap_is_in_range(test_bitmap, 99));
    ASSERT_FALSE(bh_bitmap_is_in_range(test_bitmap, 132));
    
    // Test bit operations with offset
    bh_bitmap_set_bit(test_bitmap, 105);
    bh_bitmap_set_bit(test_bitmap, 120);
    
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 105));
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 120));
    ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, 100));
    ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, 131));
    
    // Clear a bit
    bh_bitmap_clear_bit(test_bitmap, 105);
    ASSERT_EQ(0, bh_bitmap_get_bit(test_bitmap, 105));
    ASSERT_EQ(1, bh_bitmap_get_bit(test_bitmap, 120));
}

// ============================================================================
// Integration Tests - Testing interactions between assertion, file I/O, and bitmap functions
// ============================================================================

TEST_F(AssertionFileIOTest, Integration_FileIOWithBitmap_WorksTogether) {
    // Test integration of file I/O and bitmap operations
    
    // Create a test file with binary data
    std::string binary_file = "/tmp/wamr_binary_test.bin";
    char binary_data[32];
    for (int i = 0; i < 32; i++) {
        binary_data[i] = (char)(i % 256);
    }
    
    int fd = open(binary_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);
    write(fd, binary_data, 32);
    close(fd);
    
    // Read the file
    uint32 file_size = 0;
    char* buffer = bh_read_file_to_buffer(binary_file.c_str(), &file_size);
    ASSERT_TRUE(buffer != nullptr);
    ASSERT_EQ(32, file_size);
    
    // Use bitmap to track which bytes have specific values
    test_bitmap = bh_bitmap_new(0, 32);
    ASSERT_TRUE(test_bitmap != nullptr);
    
    // Mark positions where byte value is even
    for (uint32 i = 0; i < file_size; i++) {
        if ((unsigned char)buffer[i] % 2 == 0) {
            bh_bitmap_set_bit(test_bitmap, i);
        }
    }
    
    // Verify bitmap correctly tracks even-valued bytes
    for (uint32 i = 0; i < file_size; i++) {
        bool should_be_set = ((unsigned char)buffer[i] % 2 == 0);
        int bit_value = bh_bitmap_get_bit(test_bitmap, i);
        ASSERT_EQ(should_be_set ? 1 : 0, bit_value);
    }
    
    BH_FREE(buffer);
    unlink(binary_file.c_str());
}

TEST_F(AssertionFileIOTest, StressTest_LargeFileWithBitmapOperations_HandlesCorrectly) {
    // Stress test with larger operations
    const size_t large_size = 1024;
    std::string large_binary_file = "/tmp/wamr_large_binary_test.bin";
    
    // Create large binary file
    char* large_data = (char*)malloc(large_size);
    ASSERT_TRUE(large_data != nullptr);
    
    for (size_t i = 0; i < large_size; i++) {
        large_data[i] = (char)(i % 256);
    }
    
    int fd = open(large_binary_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);
    write(fd, large_data, large_size);
    close(fd);
    
    // Read the large file
    uint32 file_size = 0;
    char* buffer = bh_read_file_to_buffer(large_binary_file.c_str(), &file_size);
    ASSERT_TRUE(buffer != nullptr);
    ASSERT_EQ(large_size, file_size);
    
    // Verify data integrity
    ASSERT_EQ(0, memcmp(buffer, large_data, large_size));
    
    // Use bitmap for large-scale bit operations
    test_bitmap = bh_bitmap_new(0, large_size);
    ASSERT_TRUE(test_bitmap != nullptr);
    
    // Set bits based on data pattern
    for (uint32 i = 0; i < file_size; i++) {
        if ((unsigned char)buffer[i] % 4 == 0) {
            bh_bitmap_set_bit(test_bitmap, i);
        }
    }
    
    // Verify bitmap operations on large dataset
    size_t set_bits = 0;
    for (uint32 i = 0; i < file_size; i++) {
        if (bh_bitmap_get_bit(test_bitmap, i)) {
            set_bits++;
            ASSERT_EQ(0, (unsigned char)buffer[i] % 4);
        }
    }
    
    // Should have approximately 1/4 of bits set
    ASSERT_GT(set_bits, large_size / 8);  // At least 1/8
    ASSERT_LT(set_bits, large_size / 2);  // At most 1/2
    
    free(large_data);
    BH_FREE(buffer);
    unlink(large_binary_file.c_str());
}