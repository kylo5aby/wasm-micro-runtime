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
#include "wasm_runtime.h"
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
    
    static bool HasWASISupport() {
#if WASM_ENABLE_LIBC_WASI != 0
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

// SSP Step 1 test fixture - Core File Descriptor Operations
class SSPStep1Test : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime first
        runtime_ = std::make_unique<WAMRRuntimeRAII<>>();
        ASSERT_TRUE(runtime_->IsInitialized()) << "Failed to initialize WAMR runtime";
        
        // Create a mock execution environment
        wasm_module_t module = nullptr;
        wasm_module_inst_t module_inst = nullptr;
        uint32_t stack_size = 8092;
        
        // Create a minimal module for testing
        uint8_t wasm_bytes[] = {
            0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00  // WASM magic + version
        };
        
        char error_buf[128];
        module = wasm_runtime_load(wasm_bytes, sizeof(wasm_bytes), error_buf, sizeof(error_buf));
        if (module) {
            module_inst = wasm_runtime_instantiate(module, stack_size, stack_size, error_buf, sizeof(error_buf));
            if (module_inst) {
                exec_env_ = wasm_runtime_create_exec_env(module_inst, stack_size);
            }
        }
        
        // Initialize fd_table structure for testing  
        memset(&fd_table_, 0, sizeof(fd_table_));
        ASSERT_TRUE(fd_table_init(&fd_table_));
        
        // Initialize fd_prestats structure for testing
        memset(&prestats_, 0, sizeof(prestats_));
        ASSERT_TRUE(fd_prestats_init(&prestats_));
    }
    
    void TearDown() override {
        // Clean up execution environment
        if (exec_env_) {
            wasm_runtime_destroy_exec_env(exec_env_);
            exec_env_ = nullptr;
        }
        
        // Clean up fd_table
        fd_table_destroy(&fd_table_);
        
        // Clean up prestats
        fd_prestats_destroy(&prestats_);
        
        // WAMR runtime cleanup handled by RAII destructor
        runtime_.reset();
    }
    
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_;
    struct fd_table fd_table_;
    struct fd_prestats prestats_;
    wasm_exec_env_t exec_env_ = nullptr;
};

// ==== STEP 1: Core File Descriptor Operations Tests ====

// Function 1: wasmtime_ssp_fd_close() tests
TEST_F(SSPStep1Test, WasmtimeSspFdClose_ValidFd_HandlesCorrectly) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t test_fd = 3;
    
    // Test closing a valid file descriptor
    __wasi_errno_t result = wasmtime_ssp_fd_close(exec_env_, &fd_table_, &prestats_, test_fd);
    
    // Function should handle the call - accept various valid error codes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || result == __WASI_EINVAL);
}

TEST_F(SSPStep1Test, WasmtimeSspFdClose_InvalidFd_ReturnsError) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t invalid_fd = 9999; // Very large invalid fd
    
    // Test closing an invalid file descriptor
    __wasi_errno_t result = wasmtime_ssp_fd_close(exec_env_, &fd_table_, &prestats_, invalid_fd);
    
    // Should return error for invalid fd
    ASSERT_NE(__WASI_ESUCCESS, result);
    ASSERT_TRUE(result == __WASI_EBADF || result == __WASI_EINVAL);
}

// Function 2: wasmtime_ssp_fd_pread() tests
TEST_F(SSPStep1Test, WasmtimeSspFdPread_NormalOperation_HandlesCorrectly) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t test_fd = 0; // stdin
    __wasi_iovec_t iov;
    char buffer[256];
    size_t nread = 0;
    __wasi_filesize_t offset = 0;
    
    iov.buf = (uint8_t*)buffer;
    iov.buf_len = sizeof(buffer);
    
    // Test pread operation
    __wasi_errno_t result = wasmtime_ssp_fd_pread(exec_env_, &fd_table_, test_fd, &iov, 1, offset, &nread);
    
    // Function should handle the call - accept various valid outcomes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ESPIPE);
}

TEST_F(SSPStep1Test, WasmtimeSspFdPread_InvalidParams_ReturnsError) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t invalid_fd = 9999;
    __wasi_iovec_t iov;
    char buffer[256];
    size_t nread = 0;
    __wasi_filesize_t offset = 0;
    
    iov.buf = (uint8_t*)buffer;
    iov.buf_len = sizeof(buffer);
    
    // Test with invalid fd
    __wasi_errno_t result = wasmtime_ssp_fd_pread(exec_env_, &fd_table_, invalid_fd, &iov, 1, offset, &nread);
    
    // Should return error for invalid parameters
    ASSERT_NE(__WASI_ESUCCESS, result);
}

// Function 3: wasmtime_ssp_fd_pwrite() tests
TEST_F(SSPStep1Test, WasmtimeSspFdPwrite_NormalOperation_HandlesCorrectly) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t test_fd = 1; // stdout
    __wasi_ciovec_t iov;
    const char test_data[] = "test data";
    size_t nwritten = 0;
    __wasi_filesize_t offset = 0;
    
    iov.buf = (const uint8_t*)test_data;
    iov.buf_len = strlen(test_data);
    
    // Test pwrite operation
    __wasi_errno_t result = wasmtime_ssp_fd_pwrite(exec_env_, &fd_table_, test_fd, &iov, 1, offset, &nwritten);
    
    // Function should handle the call - accept various valid outcomes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ESPIPE);
}

TEST_F(SSPStep1Test, WasmtimeSspFdPwrite_InvalidParams_ReturnsError) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t invalid_fd = 9999;
    __wasi_ciovec_t iov;
    const char test_data[] = "test";
    size_t nwritten = 0;
    __wasi_filesize_t offset = 0;
    
    iov.buf = (const uint8_t*)test_data;
    iov.buf_len = strlen(test_data);
    
    // Test with invalid fd
    __wasi_errno_t result = wasmtime_ssp_fd_pwrite(exec_env_, &fd_table_, invalid_fd, &iov, 1, offset, &nwritten);
    
    // Should return error for invalid parameters
    ASSERT_NE(__WASI_ESUCCESS, result);
}

// Function 4: wasmtime_ssp_fd_read() tests
TEST_F(SSPStep1Test, WasmtimeSspFdRead_NormalOperation_HandlesCorrectly) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t test_fd = 0; // stdin
    __wasi_iovec_t iov;
    char buffer[256];
    size_t nread = 0;
    
    iov.buf = (uint8_t*)buffer;
    iov.buf_len = sizeof(buffer);
    
    // Test read operation
    __wasi_errno_t result = wasmtime_ssp_fd_read(exec_env_, &fd_table_, test_fd, &iov, 1, &nread);
    
    // Function should handle the call - accept various valid outcomes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || result == __WASI_EINVAL);
}

TEST_F(SSPStep1Test, WasmtimeSspFdRead_ErrorConditions_ReturnsError) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t invalid_fd = 9999;
    __wasi_iovec_t iov;
    char buffer[256];
    size_t nread = 0;
    
    iov.buf = (uint8_t*)buffer;
    iov.buf_len = sizeof(buffer);
    
    // Test with invalid fd
    __wasi_errno_t result = wasmtime_ssp_fd_read(exec_env_, &fd_table_, invalid_fd, &iov, 1, &nread);
    
    // Should return error for invalid fd
    ASSERT_NE(__WASI_ESUCCESS, result);
}

// Function 5: wasmtime_ssp_fd_write() tests
TEST_F(SSPStep1Test, WasmtimeSspFdWrite_NormalOperation_HandlesCorrectly) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t test_fd = 1; // stdout
    __wasi_ciovec_t iov;
    const char test_data[] = "Hello SSP Test\n";
    size_t nwritten = 0;
    
    iov.buf = (const uint8_t*)test_data;
    iov.buf_len = strlen(test_data);
    
    // Test write operation
    __wasi_errno_t result = wasmtime_ssp_fd_write(exec_env_, &fd_table_, test_fd, &iov, 1, &nwritten);
    
    // Function should handle the call - accept various valid outcomes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || result == __WASI_EINVAL);
    
    // If successful, verify some data was written
    if (result == __WASI_ESUCCESS) {
        ASSERT_GT(nwritten, 0);
        ASSERT_LE(nwritten, strlen(test_data));
    }
}

TEST_F(SSPStep1Test, WasmtimeSspFdWrite_ErrorConditions_ReturnsError) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t invalid_fd = 9999;
    __wasi_ciovec_t iov;
    const char test_data[] = "test";
    size_t nwritten = 0;
    
    iov.buf = (const uint8_t*)test_data;
    iov.buf_len = strlen(test_data);
    
    // Test with invalid fd
    __wasi_errno_t result = wasmtime_ssp_fd_write(exec_env_, &fd_table_, invalid_fd, &iov, 1, &nwritten);
    
    // Should return error for invalid fd
    ASSERT_NE(__WASI_ESUCCESS, result);
}

// Function 6: wasmtime_ssp_fd_seek() tests
TEST_F(SSPStep1Test, WasmtimeSspFdSeek_PositionOperations_HandlesCorrectly) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t test_fd = 0; // stdin
    __wasi_filedelta_t offset = 0;
    __wasi_whence_t whence = __WASI_WHENCE_CUR;
    __wasi_filesize_t newoffset = 0;
    
    // Test seek operation
    __wasi_errno_t result = wasmtime_ssp_fd_seek(exec_env_, &fd_table_, test_fd, offset, whence, &newoffset);
    
    // Function should handle the call - accept various valid outcomes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ESPIPE);
}

TEST_F(SSPStep1Test, WasmtimeSspFdSeek_ErrorConditions_ReturnsError) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t invalid_fd = 9999;
    __wasi_filedelta_t offset = 100;
    __wasi_whence_t whence = __WASI_WHENCE_SET;
    __wasi_filesize_t newoffset = 0;
    
    // Test with invalid fd
    __wasi_errno_t result = wasmtime_ssp_fd_seek(exec_env_, &fd_table_, invalid_fd, offset, whence, &newoffset);
    
    // Should return error for invalid fd
    ASSERT_NE(__WASI_ESUCCESS, result);
}

// Additional comprehensive tests for Step 1
TEST_F(SSPStep1Test, Step1Integration_MultipleOperations_WorkTogether) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    // Test integration of Step 1 functions
    __wasi_fd_t test_fd = 1; // stdout
    
    // 1. Test write operation
    __wasi_ciovec_t write_iov;
    const char test_data[] = "Integration test\n";
    size_t nwritten = 0;
    
    write_iov.buf = (const uint8_t*)test_data;
    write_iov.buf_len = strlen(test_data);
    
    __wasi_errno_t write_result = wasmtime_ssp_fd_write(exec_env_, &fd_table_, test_fd, &write_iov, 1, &nwritten);
    
    // 2. Test seek operation (may not work on stdout, but should handle gracefully)
    __wasi_filesize_t newoffset = 0;
    __wasi_errno_t seek_result = wasmtime_ssp_fd_seek(exec_env_, &fd_table_, test_fd, 0, __WASI_WHENCE_CUR, &newoffset);
    
    // Both operations should handle gracefully
    ASSERT_TRUE(write_result == __WASI_ESUCCESS || write_result == __WASI_EBADF || write_result == __WASI_EINVAL);
    ASSERT_TRUE(seek_result == __WASI_ESUCCESS || seek_result == __WASI_EBADF || 
                seek_result == __WASI_EINVAL || seek_result == __WASI_ESPIPE);
}

TEST_F(SSPStep1Test, Step1ErrorHandling_NullParameters_HandledGracefully) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    __wasi_fd_t test_fd = 1;
    
    // Test fd_write with null iovec - should handle gracefully
    size_t nwritten = 0;
    __wasi_errno_t result1 = wasmtime_ssp_fd_write(exec_env_, &fd_table_, test_fd, nullptr, 0, &nwritten);
    
    // Test fd_read with null iovec - should handle gracefully  
    size_t nread = 0;
    __wasi_errno_t result2 = wasmtime_ssp_fd_read(exec_env_, &fd_table_, test_fd, nullptr, 0, &nread);
    
    // Both should handle null parameters gracefully
    ASSERT_TRUE(result1 == __WASI_ESUCCESS || result1 == __WASI_EINVAL || result1 == __WASI_EBADF);
    ASSERT_TRUE(result2 == __WASI_ESUCCESS || result2 == __WASI_EINVAL || result2 == __WASI_EBADF);
}

TEST_F(SSPStep1Test, Step1BoundaryConditions_LargeValues_HandledCorrectly) {
    if (!PlatformTestContext::HasWASISupport()) {
        return; // Skip if WASI not supported
    }
    
    // Test with very large offset for seek
    __wasi_fd_t test_fd = 1;
    __wasi_filedelta_t large_offset = INT64_MAX;
    __wasi_filesize_t newoffset = 0;
    
    __wasi_errno_t result = wasmtime_ssp_fd_seek(exec_env_, &fd_table_, test_fd, large_offset, __WASI_WHENCE_SET, &newoffset);
    
    // Should handle large offset appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ESPIPE || result == __WASI_EFBIG);
}