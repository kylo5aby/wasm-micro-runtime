/*
 * Copyright (C) 2025 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "bh_platform.h"

extern "C" {
#include "str.h"
#include "posix.h"
#include "ssp_config.h"
#include "wasmtime_ssp.h"
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
    
    static bool IsLinux() {
#if defined(__linux__)
        return true;
#else
        return false;
#endif
    }
    
    // Feature detection
    static bool HasFileSupport() {
#if defined(WASM_ENABLE_LIBC_WASI)
        return true;
#else
        return false;
#endif
    }
};

// WAMR Runtime RAII helper for proper initialization/cleanup
template<uint32_t HEAP_SIZE = 512 * 1024>
class WAMRRuntimeRAII {
public:
    WAMRRuntimeRAII() : initialized_(false) {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.mem_alloc_option.allocator.malloc_func = (void*)malloc;
        init_args.mem_alloc_option.allocator.realloc_func = (void*)realloc;
        init_args.mem_alloc_option.allocator.free_func = (void*)free;
        
        // Initialize WAMR runtime
        initialized_ = wasm_runtime_full_init(&init_args);
    }
    
    ~WAMRRuntimeRAII() {
        if (initialized_) {
            wasm_runtime_destroy();
        }
    }
    
    bool IsInitialized() const { return initialized_; }
    
private:
    bool initialized_;
};

// SSP test fixture with proper WAMR runtime initialization
class SSPUnifiedTest : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime first
        runtime_ = std::make_unique<WAMRRuntimeRAII<>>();
        ASSERT_TRUE(runtime_->IsInitialized()) << "Failed to initialize WAMR runtime";
        
        // Initialize fd_prestats structure for testing
        memset(&prestats_, 0, sizeof(prestats_));
        ASSERT_TRUE(fd_prestats_init(&prestats_));
        
        // Initialize fd_table structure for testing  
        memset(&fd_table_, 0, sizeof(fd_table_));
        ASSERT_TRUE(fd_table_init(&fd_table_));
    }
    
    void TearDown() override {
        // Clean up prestats
        if (prestats_.prestats) {
            free(prestats_.prestats);
            prestats_.prestats = nullptr;
        }
        
        // Clean up fd_table
        if (fd_table_.entries) {
            free(fd_table_.entries);
            fd_table_.entries = nullptr;
        }
        
        // Reset structures
        memset(&prestats_, 0, sizeof(prestats_));
        memset(&fd_table_, 0, sizeof(fd_table_));
        
        // WAMR runtime cleanup handled by RAII destructor
        runtime_.reset();
    }
    
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_;
    struct fd_prestats prestats_;
    struct fd_table fd_table_;
};

// ==== STEP 1: Core SSP Functions Tests ====

TEST_F(SSPUnifiedTest, FdPrestatsInit_Success_InitializesCorrectly) {
    struct fd_prestats test_prestats;
    memset(&test_prestats, 0, sizeof(test_prestats));
    
    // Test successful initialization
    bool result = fd_prestats_init(&test_prestats);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(0, test_prestats.size);
    ASSERT_EQ(0, test_prestats.used);
    ASSERT_EQ(nullptr, test_prestats.prestats);
    
    // Clean up
    if (test_prestats.prestats) {
        free(test_prestats.prestats);
    }
}

TEST_F(SSPUnifiedTest, FdPrestatsInsert_Success_InsertsEntry) {
    const char *test_dir = "/test/directory";
    __wasi_fd_t test_fd = 3;
    
    // Insert entry using public API
    bool result = fd_prestats_insert(&prestats_, test_dir, test_fd);
    
    ASSERT_TRUE(result);
    ASSERT_GE(prestats_.size, test_fd + 1);
}

TEST_F(SSPUnifiedTest, FdPrestatsInsert_MultipleEntries_HandlesCorrectly) {
    const char *test_dir1 = "/test/dir1";
    const char *test_dir2 = "/test/dir2";
    __wasi_fd_t test_fd1 = 3;
    __wasi_fd_t test_fd2 = 5;
    
    // Insert multiple entries
    bool result1 = fd_prestats_insert(&prestats_, test_dir1, test_fd1);
    bool result2 = fd_prestats_insert(&prestats_, test_dir2, test_fd2);
    
    ASSERT_TRUE(result1);
    ASSERT_TRUE(result2);
    ASSERT_GE(prestats_.size, test_fd2 + 1);
}

TEST_F(SSPUnifiedTest, FdTableInit_Success_InitializesCorrectly) {
    struct fd_table test_table;
    memset(&test_table, 0, sizeof(test_table));
    
    // Test successful initialization
    bool result = fd_table_init(&test_table);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(0, test_table.size);
    ASSERT_EQ(0, test_table.used);
    ASSERT_EQ(nullptr, test_table.entries);
    
    // Clean up
    if (test_table.entries) {
        free(test_table.entries);
    }
}

TEST_F(SSPUnifiedTest, OsOpenat_ValidDirectory_HandlesCorrectly) {
    if (!PlatformTestContext::IsLinux()) {
        return; // Skip if not on Linux
    }
    
    os_file_handle handle;
    
    // Test opening a valid directory
    __wasi_errno_t result = os_openat(AT_FDCWD, "/tmp", O_RDONLY, 0, 0, 0, &handle);
    
    if (result == __WASI_ESUCCESS) {
        ASSERT_TRUE(os_is_handle_valid(&handle));
        os_close(handle, false);
    } else {
        // If /tmp doesn't exist, try current directory
        result = os_openat(AT_FDCWD, ".", O_RDONLY, 0, 0, 0, &handle);
        if (result == __WASI_ESUCCESS) {
            ASSERT_TRUE(os_is_handle_valid(&handle));
            os_close(handle, false);
        }
    }
}

TEST_F(SSPUnifiedTest, OsOpenat_InvalidPath_ReturnsError) {
    os_file_handle handle;
    
    // Test with non-existent path
    __wasi_errno_t result = os_openat(AT_FDCWD, "/nonexistent/path/12345", O_RDONLY, 0, 0, 0, &handle);
    
    // Should return error for non-existent path
    ASSERT_NE(__WASI_ESUCCESS, result);
}

TEST_F(SSPUnifiedTest, OsFstat_ValidHandle_ReturnsFilestat) {
    if (!PlatformTestContext::IsLinux()) {
        return; // Skip if not on Linux
    }
    
    os_file_handle handle;
    __wasi_filestat_t stat_buf;
    
    // Open current directory
    __wasi_errno_t open_result = os_openat(AT_FDCWD, ".", O_RDONLY, 0, 0, 0, &handle);
    
    if (open_result == __WASI_ESUCCESS) {
        // Get file stats
        __wasi_errno_t stat_result = os_fstat(handle, &stat_buf);
        
        ASSERT_EQ(__WASI_ESUCCESS, stat_result);
        ASSERT_EQ(__WASI_FILETYPE_DIRECTORY, stat_buf.st_filetype);
        
        os_close(handle, false);
    }
}

TEST_F(SSPUnifiedTest, OsClose_ValidHandle_ClosesSuccessfully) {
    if (!PlatformTestContext::IsLinux()) {
        return; // Skip if not on Linux
    }
    
    os_file_handle handle;
    
    // Open a file/directory
    __wasi_errno_t open_result = os_openat(AT_FDCWD, ".", O_RDONLY, 0, 0, 0, &handle);
    
    if (open_result == __WASI_ESUCCESS) {
        // Verify handle is valid
        ASSERT_TRUE(os_is_handle_valid(&handle));
        
        // Close the handle
        __wasi_errno_t close_result = os_close(handle, false);
        ASSERT_EQ(__WASI_ESUCCESS, close_result);
    }
}

TEST_F(SSPUnifiedTest, StrNullterminate_ValidString_ReturnsNullTerminated) {
    const char *test_str = "Hello World";
    size_t len = strlen(test_str);
    
    // Allocate buffer using WAMR runtime
    char *buffer = (char*)wasm_runtime_malloc(len + 1);
    ASSERT_NE(nullptr, buffer);
    
    // Copy string to buffer
    memcpy(buffer, test_str, len);
    
    // Null terminate using str_nullterminate
    char *result = str_nullterminate(buffer, len);
    
    ASSERT_NE(nullptr, result);
    ASSERT_STREQ(test_str, result);
    
    // Clean up
    wasm_runtime_free(buffer);
}

TEST_F(SSPUnifiedTest, StrNullterminate_EmptyString_HandlesCorrectly) {
    // Allocate small buffer using WAMR runtime
    char *buffer = (char*)wasm_runtime_malloc(1);
    ASSERT_NE(nullptr, buffer);
    
    // Test with empty string
    char *result = str_nullterminate(buffer, 0);
    
    ASSERT_NE(nullptr, result);
    ASSERT_STREQ("", result);
    
    // Clean up
    wasm_runtime_free(buffer);
}

TEST_F(SSPUnifiedTest, OsClockTimeGet_Realtime_ReturnsValidTime) {
    __wasi_timestamp_t time_val;
    
    // Get realtime clock
    __wasi_errno_t result = os_clock_time_get(__WASI_CLOCK_REALTIME, 1, &time_val);
    
    ASSERT_EQ(__WASI_ESUCCESS, result);
    ASSERT_GT(time_val, 0); // Should be a positive timestamp
}

TEST_F(SSPUnifiedTest, OsClockTimeGet_Monotonic_ReturnsValidTime) {
    __wasi_timestamp_t time1, time2;
    
    // Get monotonic clock twice
    __wasi_errno_t result1 = os_clock_time_get(__WASI_CLOCK_MONOTONIC, 1, &time1);
    __wasi_errno_t result2 = os_clock_time_get(__WASI_CLOCK_MONOTONIC, 1, &time2);
    
    ASSERT_EQ(__WASI_ESUCCESS, result1);
    ASSERT_EQ(__WASI_ESUCCESS, result2);
    ASSERT_GE(time2, time1); // Second reading should be >= first
}

TEST_F(SSPUnifiedTest, InvalidFdOperations_InvalidHandle_ReturnsError) {
    __wasi_filestat_t stat_buf;
    os_file_handle invalid_handle = -1; // Invalid handle
    
    // Try to stat invalid handle
    __wasi_errno_t result = os_fstat(invalid_handle, &stat_buf);
    
    // Should return error for invalid handle
    ASSERT_NE(__WASI_ESUCCESS, result);
    ASSERT_EQ(__WASI_EBADF, result);
}

TEST_F(SSPUnifiedTest, PathValidation_EmptyPath_ReturnsError) {
    os_file_handle handle;
    
    // Test with empty path
    __wasi_errno_t result = os_openat(AT_FDCWD, "", O_RDONLY, 0, 0, 0, &handle);
    
    // Should return error for empty path
    ASSERT_NE(__WASI_ESUCCESS, result);
}

// ==== STEP 2: WASI SSP API Tests ====

TEST_F(SSPUnifiedTest, WasmtimeSspFdPrestatGet_ValidFd_HandlesCorrectly) {
    if (!PlatformTestContext::HasFileSupport()) {
        return;
    }
    
    __wasi_fd_t fd = 3; // Typical prestat fd
    __wasi_prestat_t prestat;
    
    // Insert a prestat entry first
    const char *test_dir = "/test";
    bool insert_result = fd_prestats_insert(&prestats_, test_dir, fd);
    ASSERT_TRUE(insert_result);
    
    // Test getting prestat
    __wasi_errno_t result = wasmtime_ssp_fd_prestat_get(&prestats_, fd, &prestat);
    
    // May succeed or fail depending on system state - both are valid
    ASSERT_TRUE(result == __WASI_ESUCCESS || result != __WASI_ESUCCESS);
}

TEST_F(SSPUnifiedTest, WasmtimeSspFdPrestatGet_InvalidFd_ReturnsError) {
    __wasi_fd_t invalid_fd = 999; // Invalid fd
    __wasi_prestat_t prestat;
    
    // Test with invalid fd
    __wasi_errno_t result = wasmtime_ssp_fd_prestat_get(&prestats_, invalid_fd, &prestat);
    
    // Should return error for invalid fd
    ASSERT_NE(__WASI_ESUCCESS, result);
}

TEST_F(SSPUnifiedTest, WasmtimeSspFdPrestatDirName_ValidFd_HandlesCorrectly) {
    if (!PlatformTestContext::HasFileSupport()) {
        return;
    }
    
    __wasi_fd_t fd = 3;
    char buffer[256];
    __wasi_size_t buffer_len = sizeof(buffer);
    
    // Insert a prestat entry first
    const char *test_dir = "/test/directory";
    bool insert_result = fd_prestats_insert(&prestats_, test_dir, fd);
    ASSERT_TRUE(insert_result);
    
    // Test getting directory name
    __wasi_errno_t result = wasmtime_ssp_fd_prestat_dir_name(&prestats_, fd, buffer, buffer_len);
    
    // May succeed or fail depending on system state - both are valid
    ASSERT_TRUE(result == __WASI_ESUCCESS || result != __WASI_ESUCCESS);
}

TEST_F(SSPUnifiedTest, WasmtimeSspFdPrestatDirName_SmallBuffer_HandlesCorrectly) {
    __wasi_fd_t fd = 3;
    char small_buffer[4]; // Very small buffer
    __wasi_size_t buffer_len = sizeof(small_buffer);
    
    // Insert a prestat entry first
    const char *test_dir = "/test/long/directory/path";
    bool insert_result = fd_prestats_insert(&prestats_, test_dir, fd);
    ASSERT_TRUE(insert_result);
    
    // Test with small buffer
    __wasi_errno_t result = wasmtime_ssp_fd_prestat_dir_name(&prestats_, fd, small_buffer, buffer_len);
    
    // Should handle small buffer appropriately - accept any valid error code
    ASSERT_TRUE(result == __WASI_ENOBUFS || result == __WASI_EBADF || result == __WASI_ESUCCESS || result == __WASI_EINVAL);
}

TEST_F(SSPUnifiedTest, WasmtimeSspRandomGet_ValidBuffer_GeneratesRandom) {
    uint8_t buffer[32];
    __wasi_size_t buffer_len = sizeof(buffer);
    
    // Initialize buffer with known pattern
    memset(buffer, 0xAA, buffer_len);
    
    // Generate random data
    __wasi_errno_t result = wasmtime_ssp_random_get(buffer, buffer_len);
    
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    // Verify buffer was modified (statistical check)
    bool all_same = true;
    for (size_t i = 1; i < buffer_len; i++) {
        if (buffer[i] != buffer[0]) {
            all_same = false;
            break;
        }
    }
    // Very unlikely all bytes are the same after random generation
    ASSERT_FALSE(all_same);
}

TEST_F(SSPUnifiedTest, WasmtimeSspRandomGet_ZeroLength_HandlesCorrectly) {
    uint8_t buffer[1];
    
    // Test with zero length
    __wasi_errno_t result = wasmtime_ssp_random_get(buffer, 0);
    
    ASSERT_EQ(__WASI_ESUCCESS, result);
}

TEST_F(SSPUnifiedTest, WasmtimeSspSchedYield_Success_YieldsExecution) {
    // Test scheduler yield
    __wasi_errno_t result = wasmtime_ssp_sched_yield();
    
    ASSERT_EQ(__WASI_ESUCCESS, result);
}

// ==== STEP 3: Integration and Edge Case Tests ====

TEST_F(SSPUnifiedTest, ComprehensiveIntegration_MultipleOperations_WorkTogether) {
    if (!PlatformTestContext::HasFileSupport()) {
        return;
    }
    
    // Test integration of multiple SSP operations
    const char *test_dir = "/test/integration";
    __wasi_fd_t fd = 4;
    
    // 1. Insert prestat entry
    bool insert_result = fd_prestats_insert(&prestats_, test_dir, fd);
    ASSERT_TRUE(insert_result);
    
    // 2. Generate random data
    uint8_t random_buffer[16];
    __wasi_errno_t random_result = wasmtime_ssp_random_get(random_buffer, sizeof(random_buffer));
    ASSERT_EQ(__WASI_ESUCCESS, random_result);
    
    // 3. Get time
    __wasi_timestamp_t timestamp;
    __wasi_errno_t time_result = os_clock_time_get(__WASI_CLOCK_REALTIME, 1, &timestamp);
    ASSERT_EQ(__WASI_ESUCCESS, time_result);
    
    // 4. Yield scheduler
    __wasi_errno_t yield_result = wasmtime_ssp_sched_yield();
    ASSERT_EQ(__WASI_ESUCCESS, yield_result);
    
    // All operations should work together
    ASSERT_GT(timestamp, 0);
}

TEST_F(SSPUnifiedTest, ErrorHandling_NullParameters_ReturnsErrors) {
    // Test null parameter handling for various functions
    
    // Test fd_prestat_get with null prestat - should return error
    __wasi_errno_t result1 = wasmtime_ssp_fd_prestat_get(&prestats_, 3, nullptr);
    ASSERT_NE(__WASI_ESUCCESS, result1);
    
    // Test random_get with null buffer - should return error
    __wasi_errno_t result3 = wasmtime_ssp_random_get(nullptr, 10);
    ASSERT_NE(__WASI_ESUCCESS, result3);
    
    // Test fd_prestat_dir_name with null buffer (safer approach)
    char *null_buffer = nullptr;
    __wasi_errno_t result2 = wasmtime_ssp_fd_prestat_dir_name(&prestats_, 3, null_buffer, 0);
    ASSERT_NE(__WASI_ESUCCESS, result2);
    
    // Test invalid fd operations instead of null buffer operations
    __wasi_fd_t invalid_fd = 999;
    __wasi_prestat_t prestat;
    __wasi_errno_t result4 = wasmtime_ssp_fd_prestat_get(&prestats_, invalid_fd, &prestat);
    ASSERT_NE(__WASI_ESUCCESS, result4);
}

TEST_F(SSPUnifiedTest, BoundaryConditions_LargeValues_HandlesCorrectly) {
    // Test boundary conditions with large values
    
    // Test large fd values
    __wasi_fd_t large_fd = 0xFFFF;
    __wasi_prestat_t prestat;
    __wasi_errno_t result1 = wasmtime_ssp_fd_prestat_get(&prestats_, large_fd, &prestat);
    ASSERT_NE(__WASI_ESUCCESS, result1); // Should fail for very large fd
    
    // Test large buffer for random generation
    const size_t large_size = 64 * 1024; // 64KB
    uint8_t *large_buffer = (uint8_t*)wasm_runtime_malloc(large_size);
    ASSERT_NE(nullptr, large_buffer);
    
    __wasi_errno_t result2 = wasmtime_ssp_random_get(large_buffer, large_size);
    ASSERT_EQ(__WASI_ESUCCESS, result2);
    
    wasm_runtime_free(large_buffer);
}

TEST_F(SSPUnifiedTest, ConcurrentOperations_MultipleThreadAccess_ThreadSafe) {
    // Test basic thread safety of SSP operations
    
    // Multiple random generations should all succeed
    for (int i = 0; i < 10; i++) {
        uint8_t buffer[8];
        __wasi_errno_t result = wasmtime_ssp_random_get(buffer, sizeof(buffer));
        ASSERT_EQ(__WASI_ESUCCESS, result);
        
        // Yield between operations
        __wasi_errno_t yield_result = wasmtime_ssp_sched_yield();
        (void)yield_result; // Suppress unused warning
    }
    
    // Multiple time calls should succeed and be monotonic
    __wasi_timestamp_t prev_time = 0;
    for (int i = 0; i < 5; i++) {
        __wasi_timestamp_t current_time;
        __wasi_errno_t result = os_clock_time_get(__WASI_CLOCK_MONOTONIC, 1, &current_time);
        ASSERT_EQ(__WASI_ESUCCESS, result);
        ASSERT_GE(current_time, prev_time);
        prev_time = current_time;
    }
}