/*
 * Copyright (C) 2024 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "platform_api_extension.h"
#include "wasm_export.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/**
 * POSIX Coverage Improvement Step 2: Directory Operations
 * 
 * Target Functions (8 functions, ~175 lines):
 * 1. os_mkdirat() - Directory creation
 * 2. os_readdir() - Reading directory entries
 * 3. os_rewinddir() - Directory stream reset
 * 4. os_seekdir() - Directory stream positioning
 * 5. os_open_preopendir() - Preopen directory opening
 * 6. os_linkat() - Hard link creation
 * 7. os_unlinkat() - File/directory removal
 * 8. os_renameat() - File/directory renaming
 */

class PosixDirectoryOperationsTest : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime for POSIX API testing
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
        
        // Create test directory structure
        test_dir_base = "/tmp/wamr_posix_dir_test";
        cleanup_test_directories();
        create_test_directories();
    }
    
    void TearDown() override {
        cleanup_test_directories();
        wasm_runtime_destroy();
    }
    
    void create_test_directories() {
        mkdir(test_dir_base.c_str(), 0755);
        mkdir((test_dir_base + "/subdir1").c_str(), 0755);
        mkdir((test_dir_base + "/subdir2").c_str(), 0755);
        
        // Create test files
        int fd1 = open((test_dir_base + "/test_file1.txt").c_str(), O_CREAT | O_WRONLY, 0644);
        if (fd1 >= 0) {
            write(fd1, "test content 1", 14);
            close(fd1);
        }
        
        int fd2 = open((test_dir_base + "/test_file2.txt").c_str(), O_CREAT | O_WRONLY, 0644);
        if (fd2 >= 0) {
            write(fd2, "test content 2", 14);
            close(fd2);
        }
    }
    
    void cleanup_test_directories() {
        system(("rm -rf " + test_dir_base).c_str());
    }
    
    std::string test_dir_base;
};

// Test 1: os_mkdirat() - Directory creation
TEST_F(PosixDirectoryOperationsTest, os_mkdirat_create_directory) {
    os_file_handle base_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &base_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Test successful directory creation
    result = os_mkdirat(base_handle, "new_directory");
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Verify directory was created
    struct stat st;
    std::string new_dir_path = test_dir_base + "/new_directory";
    ASSERT_EQ(0, stat(new_dir_path.c_str(), &st));
    ASSERT_TRUE(S_ISDIR(st.st_mode));
    
    os_close(base_handle, false);
}

TEST_F(PosixDirectoryOperationsTest, os_mkdirat_invalid_path) {
    os_file_handle base_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &base_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Test directory creation with invalid path (already exists)
    result = os_mkdirat(base_handle, "subdir1");
    ASSERT_NE(__WASI_ESUCCESS, result);
    
    // Test directory creation with invalid nested path
    result = os_mkdirat(base_handle, "nonexistent/nested/path");
    ASSERT_NE(__WASI_ESUCCESS, result);
    
    os_close(base_handle, false);
}

// Test 2: os_readdir() - Reading directory entries
TEST_F(PosixDirectoryOperationsTest, os_readdir_entries) {
    os_file_handle dir_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &dir_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    os_dir_stream dir_stream;
    result = os_fdopendir(dir_handle, &dir_stream);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Read directory entries
    __wasi_dirent_t entry;
    const char *d_name = nullptr;
    int entry_count = 0;
    bool found_subdir1 = false, found_subdir2 = false;
    bool found_file1 = false, found_file2 = false;
    
    while (true) {
        result = os_readdir(dir_stream, &entry, &d_name);
        ASSERT_EQ(__WASI_ESUCCESS, result);
        
        if (d_name == nullptr) {
            break; // End of directory
        }
        
        entry_count++;
        std::string name(d_name);
        
        if (name == "subdir1") found_subdir1 = true;
        if (name == "subdir2") found_subdir2 = true;
        if (name == "test_file1.txt") found_file1 = true;
        if (name == "test_file2.txt") found_file2 = true;
    }
    
    // Verify we found expected entries
    ASSERT_GT(entry_count, 0);
    ASSERT_TRUE(found_subdir1);
    ASSERT_TRUE(found_subdir2);
    ASSERT_TRUE(found_file1);
    ASSERT_TRUE(found_file2);
    
    os_closedir(dir_stream);
}

TEST_F(PosixDirectoryOperationsTest, os_readdir_end_of_dir) {
    os_file_handle dir_handle;
    __wasi_errno_t result = os_open_preopendir((test_dir_base + "/subdir1").c_str(), &dir_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    os_dir_stream dir_stream;
    result = os_fdopendir(dir_handle, &dir_stream);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Read all entries until end
    __wasi_dirent_t entry;
    const char *d_name = nullptr;
    int reads = 0;
    
    do {
        result = os_readdir(dir_stream, &entry, &d_name);
        ASSERT_EQ(__WASI_ESUCCESS, result);
        reads++;
    } while (d_name != nullptr && reads < 100); // Safety limit
    
    // Verify we reached end of directory
    ASSERT_EQ(nullptr, d_name);
    
    os_closedir(dir_stream);
}

// Test 3: os_rewinddir() - Directory stream reset
TEST_F(PosixDirectoryOperationsTest, os_rewinddir_reset) {
    os_file_handle dir_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &dir_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    os_dir_stream dir_stream;
    result = os_fdopendir(dir_handle, &dir_stream);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Read first entry
    __wasi_dirent_t entry1, entry2;
    const char *first_name = nullptr, *second_name = nullptr;
    
    result = os_readdir(dir_stream, &entry1, &first_name);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    if (first_name != nullptr) {
        // Read second entry
        result = os_readdir(dir_stream, &entry2, &second_name);
        ASSERT_EQ(__WASI_ESUCCESS, result);
        
        // Rewind directory stream
        result = os_rewinddir(dir_stream);
        ASSERT_EQ(__WASI_ESUCCESS, result);
        
        // Read first entry again after rewind
        const char *rewound_name = nullptr;
        __wasi_dirent_t rewound_entry;
        result = os_readdir(dir_stream, &rewound_entry, &rewound_name);
        ASSERT_EQ(__WASI_ESUCCESS, result);
        
        // Verify we're back at the beginning
        if (rewound_name != nullptr && first_name != nullptr) {
            ASSERT_STREQ(first_name, rewound_name);
        }
    }
    
    os_closedir(dir_stream);
}

// Test 4: os_seekdir() - Directory stream positioning
TEST_F(PosixDirectoryOperationsTest, os_seekdir_position) {
    os_file_handle dir_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &dir_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    os_dir_stream dir_stream;
    result = os_fdopendir(dir_handle, &dir_stream);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Read first few entries to get positions
    __wasi_dirent_t entry;
    const char *d_name = nullptr;
    __wasi_dircookie_t first_position = 0;
    
    result = os_readdir(dir_stream, &entry, &d_name);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    if (d_name != nullptr) {
        first_position = entry.d_next;
        
        // Seek to a specific position
        result = os_seekdir(dir_stream, first_position);
        ASSERT_EQ(__WASI_ESUCCESS, result);
        
        // Read entry at that position
        const char *seek_name = nullptr;
        result = os_readdir(dir_stream, &entry, &seek_name);
        ASSERT_EQ(__WASI_ESUCCESS, result);
        
        // Position should be valid (we don't crash)
        // The exact behavior may vary by platform
    }
    
    os_closedir(dir_stream);
}

// Test 5: os_open_preopendir() - Preopen directory opening
TEST_F(PosixDirectoryOperationsTest, os_open_preopendir_valid) {
    os_file_handle dir_handle;
    
    // Test opening valid directory
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &dir_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Verify handle is valid by using it
    os_dir_stream dir_stream;
    result = os_fdopendir(dir_handle, &dir_stream);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    os_closedir(dir_stream);
}

// Test 6: os_linkat() - Hard link creation
TEST_F(PosixDirectoryOperationsTest, os_linkat_create_link) {
    os_file_handle base_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &base_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Create hard link
    result = os_linkat(base_handle, "test_file1.txt", base_handle, "hardlink_to_file1.txt", 0);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Verify both files exist and have same content
    std::string original = test_dir_base + "/test_file1.txt";
    std::string link = test_dir_base + "/hardlink_to_file1.txt";
    
    struct stat st1, st2;
    ASSERT_EQ(0, stat(original.c_str(), &st1));
    ASSERT_EQ(0, stat(link.c_str(), &st2));
    
    // Hard links should have same inode
    ASSERT_EQ(st1.st_ino, st2.st_ino);
    ASSERT_EQ(st1.st_size, st2.st_size);
    
    os_close(base_handle, false);
}

TEST_F(PosixDirectoryOperationsTest, os_linkat_invalid_path) {
    os_file_handle base_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &base_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Try to create link to nonexistent file
    result = os_linkat(base_handle, "nonexistent.txt", base_handle, "link_to_nothing.txt", 0);
    ASSERT_NE(__WASI_ESUCCESS, result);
    
    os_close(base_handle, false);
}

// Test 7: os_unlinkat() - File/directory removal
TEST_F(PosixDirectoryOperationsTest, os_unlinkat_remove_file) {
    os_file_handle base_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &base_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Create a temporary file to remove
    std::string temp_file = test_dir_base + "/temp_file.txt";
    int fd = open(temp_file.c_str(), O_CREAT | O_WRONLY, 0644);
    ASSERT_GE(fd, 0);
    close(fd);
    
    // Verify file exists
    struct stat st;
    ASSERT_EQ(0, stat(temp_file.c_str(), &st));
    
    // Remove file
    result = os_unlinkat(base_handle, "temp_file.txt", false);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Verify file is gone
    ASSERT_NE(0, stat(temp_file.c_str(), &st));
    
    os_close(base_handle, false);
}

TEST_F(PosixDirectoryOperationsTest, os_unlinkat_remove_directory) {
    os_file_handle base_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &base_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Create empty directory to remove
    std::string temp_dir = test_dir_base + "/temp_dir";
    ASSERT_EQ(0, mkdir(temp_dir.c_str(), 0755));
    
    // Verify directory exists
    struct stat st;
    ASSERT_EQ(0, stat(temp_dir.c_str(), &st));
    ASSERT_TRUE(S_ISDIR(st.st_mode));
    
    // Remove directory
    result = os_unlinkat(base_handle, "temp_dir", true);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Verify directory is gone
    ASSERT_NE(0, stat(temp_dir.c_str(), &st));
    
    os_close(base_handle, false);
}

// Test 8: os_renameat() - File/directory renaming
TEST_F(PosixDirectoryOperationsTest, os_renameat_move_file) {
    os_file_handle base_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &base_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Create temporary file to rename
    std::string old_file = test_dir_base + "/old_name.txt";
    std::string new_file = test_dir_base + "/new_name.txt";
    
    int fd = open(old_file.c_str(), O_CREAT | O_WRONLY, 0644);
    ASSERT_GE(fd, 0);
    write(fd, "rename test", 11);
    close(fd);
    
    // Verify old file exists
    struct stat st;
    ASSERT_EQ(0, stat(old_file.c_str(), &st));
    
    // Rename file
    result = os_renameat(base_handle, "old_name.txt", base_handle, "new_name.txt");
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Verify old file is gone and new file exists
    ASSERT_NE(0, stat(old_file.c_str(), &st));
    ASSERT_EQ(0, stat(new_file.c_str(), &st));
    
    // Verify content is preserved
    fd = open(new_file.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    char buffer[20];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    
    ASSERT_GT(bytes_read, 0);
    buffer[bytes_read] = '\0';
    ASSERT_STREQ("rename test", buffer);
    
    os_close(base_handle, false);
}

// Test for os_get_invalid_dir_stream() and os_is_dir_stream_valid()
TEST_F(PosixDirectoryOperationsTest, os_invalid_dir_stream_operations) {
    // Test getting invalid directory stream
    os_dir_stream invalid_stream = os_get_invalid_dir_stream();
    
    // Test validity check
    ASSERT_FALSE(os_is_dir_stream_valid(&invalid_stream));
    
    // Test with valid directory stream
    os_file_handle dir_handle;
    __wasi_errno_t result = os_open_preopendir(test_dir_base.c_str(), &dir_handle);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    os_dir_stream valid_stream;
    result = os_fdopendir(dir_handle, &valid_stream);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Valid stream should pass validity check
    ASSERT_TRUE(os_is_dir_stream_valid(&valid_stream));
    
    os_closedir(valid_stream);
}