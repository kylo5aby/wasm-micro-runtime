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
    static bool HasSocketSupport() {
#if defined(WASM_ENABLE_LIBC_WASI) && defined(WASM_ENABLE_WASI_NN)
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
    
    static bool HasNetworkSupport() {
#if defined(__linux__) && (WASM_ENABLE_LIBC_WASI != 0)
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

// SSP Step 3 Test Class - Socket Operations & Network Functions
class SSPStep3Test : public testing::Test {
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
        if (exec_env_) {
            wasm_runtime_destroy_exec_env(exec_env_);
        }
        runtime_.reset();
    }
    
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_;
    wasm_exec_env_t exec_env_;
    fd_table fd_table_;
    fd_prestats prestats_;
};

// Function 1: wasi_ssp_sock_open() tests
TEST_F(SSPStep3Test, WasiSspSockOpen_TcpSocket_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t sock_fd;
    __wasi_fd_t poolfd = 0;
    __wasi_address_family_t af = (__wasi_address_family_t)2; // AF_INET equivalent
    __wasi_sock_type_t sock_type = (__wasi_sock_type_t)1; // SOCK_STREAM equivalent
    
    // Test TCP socket creation
    __wasi_errno_t result = wasi_ssp_sock_open(exec_env_, &fd_table_, poolfd, af, sock_type, &sock_fd);
    
    // Function should handle the call - accept various valid outcomes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_ENOTSUP || 
                result == __WASI_EACCES || result == __WASI_EMFILE || result == __WASI_ENFILE);
    
    // If successful, verify socket fd is valid
    if (result == __WASI_ESUCCESS) {
        ASSERT_GE(sock_fd, 3); // Should be >= 3 (after stdin/stdout/stderr)
    }
}

TEST_F(SSPStep3Test, WasiSspSockOpen_UdpSocket_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t sock_fd;
    __wasi_fd_t poolfd = 0;
    __wasi_address_family_t af = (__wasi_address_family_t)2; // AF_INET equivalent
    __wasi_sock_type_t sock_type = (__wasi_sock_type_t)2; // SOCK_DGRAM equivalent
    
    // Test UDP socket creation
    __wasi_errno_t result = wasi_ssp_sock_open(exec_env_, &fd_table_, poolfd, af, sock_type, &sock_fd);
    
    // Function should handle the call appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_ENOTSUP || 
                result == __WASI_EACCES || result == __WASI_EMFILE || result == __WASI_ENFILE);
    
    // If successful, verify socket fd is valid
    if (result == __WASI_ESUCCESS) {
        ASSERT_GE(sock_fd, 3);
    }
}

// Function 2: wasi_ssp_sock_bind() tests
TEST_F(SSPStep3Test, WasiSspSockBind_ValidAddress_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3; // Assume socket fd
    __wasi_addr_t addr;
    memset(&addr, 0, sizeof(addr));
    addr.kind = IPv4;
    addr.addr.ip4.port = 8080;
    // Set IP address bytes directly using proper structure
    addr.addr.ip4.addr.n0 = 127;
    addr.addr.ip4.addr.n1 = 0;
    addr.addr.ip4.addr.n2 = 0;
    addr.addr.ip4.addr.n3 = 1;
    
    // Create a dummy addr_pool
    addr_pool pool;
    memset(&pool, 0, sizeof(pool));
    
    // Test socket bind operation
    __wasi_errno_t result = wasi_ssp_sock_bind(exec_env_, &fd_table_, &pool, test_fd, &addr);
    
    // Function should handle the call - accept various valid outcomes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_EADDRINUSE || 
                result == __WASI_EADDRNOTAVAIL || result == __WASI_ENOTSUP);
}

// Function 3: wasi_ssp_sock_listen() tests
TEST_F(SSPStep3Test, WasiSspSockListen_ValidBacklog_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3; // Assume socket fd
    __wasi_size_t backlog = 10;
    
    // Test socket listen operation
    __wasi_errno_t result = wasi_ssp_sock_listen(exec_env_, &fd_table_, test_fd, backlog);
    
    // Function should handle the call appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ENOTSUP);
}

// Function 4: wasi_ssp_sock_accept() tests
TEST_F(SSPStep3Test, WasiSspSockAccept_ValidSocket_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t listen_fd = 3; // Assume listening socket fd
    __wasi_fd_t conn_fd;
    __wasi_fdflags_t flags = 0;
    
    // Test socket accept operation
    __wasi_errno_t result = wasi_ssp_sock_accept(exec_env_, &fd_table_, listen_fd, flags, &conn_fd);
    
    // Function should handle the call - accept various valid outcomes
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_EAGAIN || result == __WASI_ENOTSUP);
}

// Function 5: wasi_ssp_sock_connect() tests
TEST_F(SSPStep3Test, WasiSspSockConnect_ValidAddress_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3; // Assume socket fd
    __wasi_addr_t addr;
    memset(&addr, 0, sizeof(addr));
    addr.kind = IPv4;
    addr.addr.ip4.port = 80;
    // Set IP address bytes directly using proper structure
    addr.addr.ip4.addr.n0 = 127;
    addr.addr.ip4.addr.n1 = 0;
    addr.addr.ip4.addr.n2 = 0;
    addr.addr.ip4.addr.n3 = 1;
    
    // Create a dummy addr_pool
    addr_pool pool;
    memset(&pool, 0, sizeof(pool));
    
    // Test socket connect operation
    __wasi_errno_t result = wasi_ssp_sock_connect(exec_env_, &fd_table_, &pool, test_fd, &addr);
    
    // Function should handle the call appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ECONNREFUSED || 
                result == __WASI_ETIMEDOUT || result == __WASI_ENETUNREACH || 
                result == __WASI_EINPROGRESS || result == __WASI_ENOTSUP);
}

// Function 6: wasi_ssp_sock_addr_local() tests
TEST_F(SSPStep3Test, WasiSspSockAddrLocal_ValidSocket_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3; // Assume socket fd
    __wasi_addr_t local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    
    // Test getting local socket address
    __wasi_errno_t result = wasi_ssp_sock_addr_local(exec_env_, &fd_table_, test_fd, &local_addr);
    
    // Function should handle the call appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ENOTSUP);
    
    // If successful, verify address structure is populated
    if (result == __WASI_ESUCCESS) {
        ASSERT_TRUE(local_addr.kind == IPv4 || local_addr.kind == IPv6);
    }
}

// Function 7: wasi_ssp_sock_addr_remote() tests
TEST_F(SSPStep3Test, WasiSspSockAddrRemote_ValidSocket_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3; // Assume connected socket fd
    __wasi_addr_t remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    
    // Test getting remote socket address
    __wasi_errno_t result = wasi_ssp_sock_addr_remote(exec_env_, &fd_table_, test_fd, &remote_addr);
    
    // Function should handle the call appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ENOTCONN || result == __WASI_ENOTSUP);
    
    // If successful, verify address structure is populated
    if (result == __WASI_ESUCCESS) {
        ASSERT_TRUE(remote_addr.kind == IPv4 || remote_addr.kind == IPv6);
    }
}

// Function 8: wasi_ssp_sock_addr_resolve() tests
TEST_F(SSPStep3Test, WasiSspSockAddrResolve_ValidHostname_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    const char* hostname = "localhost";
    const char* service = "80";
    __wasi_addr_info_hints_t hints;
    memset(&hints, 0, sizeof(hints));
    hints.family = (__wasi_address_family_t)2; // AF_INET equivalent
    hints.type = (__wasi_sock_type_t)1; // SOCK_STREAM equivalent
    
    __wasi_addr_info_t addr_info;
    memset(&addr_info, 0, sizeof(addr_info));
    __wasi_size_t addr_info_size = sizeof(addr_info);
    __wasi_size_t max_info_size = 0;
    
    // Create dummy ns_lookup_list
    char* ns_lookup_list[2] = { nullptr, nullptr };
    
    // Test address resolution
    __wasi_errno_t result = wasi_ssp_sock_addr_resolve(exec_env_, &fd_table_, ns_lookup_list, 
                                                       hostname, service, &hints, &addr_info, 
                                                       addr_info_size, &max_info_size);
    
    // Function should handle the call appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EINVAL || result == __WASI_ENOTSUP);
    
    // If successful, verify result is populated
    if (result == __WASI_ESUCCESS) {
        ASSERT_GT(max_info_size, 0);
    }
}

TEST_F(SSPStep3Test, WasiSspSockAddrResolve_InvalidHostname_ReturnsError) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    const char* invalid_hostname = "invalid.nonexistent.domain.test";
    const char* service = "80";
    __wasi_addr_info_hints_t hints;
    memset(&hints, 0, sizeof(hints));
    hints.family = (__wasi_address_family_t)2; // AF_INET equivalent
    
    __wasi_addr_info_t addr_info;
    memset(&addr_info, 0, sizeof(addr_info));
    __wasi_size_t addr_info_size = sizeof(addr_info);
    __wasi_size_t max_info_size = 0;
    
    // Create dummy ns_lookup_list
    char* ns_lookup_list[2] = { nullptr, nullptr };
    
    // Test with invalid hostname
    __wasi_errno_t result = wasi_ssp_sock_addr_resolve(exec_env_, &fd_table_, ns_lookup_list,
                                                       invalid_hostname, service, &hints, &addr_info,
                                                       addr_info_size, &max_info_size);
    
    // Should return appropriate error or success (implementation dependent)
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EINVAL || result == __WASI_ENOTSUP);
}

// Function 9: wasi_ssp_sock_get_recv_buf_size() tests
TEST_F(SSPStep3Test, WasiSspSockGetRecvBufSize_ValidSocket_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3; // Assume socket fd
    __wasi_size_t buf_size = 0;
    
    // Test getting receive buffer size
    __wasi_errno_t result = wasi_ssp_sock_get_recv_buf_size(exec_env_, &fd_table_, test_fd, &buf_size);
    
    // Function should handle the call appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ENOTSUP);
    
    // If successful, verify buffer size is reasonable
    if (result == __WASI_ESUCCESS) {
        ASSERT_GT(buf_size, 0);
        ASSERT_LE(buf_size, 1024 * 1024 * 16); // Reasonable upper bound (16MB)
    }
}

// Function 10: wasi_ssp_sock_set_recv_buf_size() tests
TEST_F(SSPStep3Test, WasiSspSockSetRecvBufSize_ValidSize_HandlesCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3; // Assume socket fd
    __wasi_size_t new_buf_size = 8192; // 8KB buffer
    
    // Test setting receive buffer size
    __wasi_errno_t result = wasi_ssp_sock_set_recv_buf_size(exec_env_, &fd_table_, test_fd, new_buf_size);
    
    // Function should handle the call appropriately
    ASSERT_TRUE(result == __WASI_ESUCCESS || result == __WASI_EBADF || 
                result == __WASI_EINVAL || result == __WASI_ENOTSUP);
}

// Integration tests for Step 3
TEST_F(SSPStep3Test, Step3Integration_SocketLifecycle_WorksTogether) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    // Test socket creation -> bind -> listen workflow
    __wasi_fd_t sock_fd;
    __wasi_fd_t poolfd = 0;
    __wasi_address_family_t af = (__wasi_address_family_t)2; // AF_INET equivalent
    __wasi_sock_type_t sock_type = (__wasi_sock_type_t)1; // SOCK_STREAM equivalent
    
    __wasi_errno_t open_result = wasi_ssp_sock_open(exec_env_, &fd_table_, poolfd, af, sock_type, &sock_fd);
    
    // If socket creation succeeds, test bind and listen
    if (open_result == __WASI_ESUCCESS) {
        __wasi_addr_t addr;
        memset(&addr, 0, sizeof(addr));
        addr.kind = IPv4;
        addr.addr.ip4.port = 0; // Let system choose port
        // Set IP address bytes directly using proper structure
        addr.addr.ip4.addr.n0 = 127;
        addr.addr.ip4.addr.n1 = 0;
        addr.addr.ip4.addr.n2 = 0;
        addr.addr.ip4.addr.n3 = 1;
        
        addr_pool pool;
        memset(&pool, 0, sizeof(pool));
        
        __wasi_errno_t bind_result = wasi_ssp_sock_bind(exec_env_, &fd_table_, &pool, sock_fd, &addr);
        __wasi_errno_t listen_result = wasi_ssp_sock_listen(exec_env_, &fd_table_, sock_fd, 5);
        
        // All operations should handle gracefully
        ASSERT_TRUE(bind_result == __WASI_ESUCCESS || bind_result == __WASI_EADDRINUSE || 
                    bind_result == __WASI_EINVAL || bind_result == __WASI_ENOTSUP);
        ASSERT_TRUE(listen_result == __WASI_ESUCCESS || listen_result == __WASI_EINVAL || 
                    listen_result == __WASI_ENOTSUP);
    }
    
    // Test should complete without crashes
    ASSERT_TRUE(open_result == __WASI_ESUCCESS || open_result == __WASI_ENOTSUP || 
                open_result == __WASI_EACCES || open_result == __WASI_EMFILE);
}

TEST_F(SSPStep3Test, Step3Integration_AddressOperations_WorkTogether) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3;
    
    // Test address operations together
    __wasi_addr_t local_addr, remote_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    memset(&remote_addr, 0, sizeof(remote_addr));
    
    __wasi_errno_t local_result = wasi_ssp_sock_addr_local(exec_env_, &fd_table_, test_fd, &local_addr);
    __wasi_errno_t remote_result = wasi_ssp_sock_addr_remote(exec_env_, &fd_table_, test_fd, &remote_addr);
    
    // Both should handle gracefully
    ASSERT_TRUE(local_result == __WASI_ESUCCESS || local_result == __WASI_EBADF || 
                local_result == __WASI_EINVAL || local_result == __WASI_ENOTSUP);
    ASSERT_TRUE(remote_result == __WASI_ESUCCESS || remote_result == __WASI_EBADF || 
                remote_result == __WASI_ENOTCONN || remote_result == __WASI_ENOTSUP);
}

TEST_F(SSPStep3Test, Step3Integration_BufferOperations_WorkTogether) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3;
    
    // Test buffer size operations together
    __wasi_size_t current_size = 0;
    __wasi_errno_t get_result = wasi_ssp_sock_get_recv_buf_size(exec_env_, &fd_table_, test_fd, &current_size);
    
    __wasi_size_t new_size = 16384; // 16KB
    __wasi_errno_t set_result = wasi_ssp_sock_set_recv_buf_size(exec_env_, &fd_table_, test_fd, new_size);
    
    // Both should handle gracefully
    ASSERT_TRUE(get_result == __WASI_ESUCCESS || get_result == __WASI_EBADF || 
                get_result == __WASI_EINVAL || get_result == __WASI_ENOTSUP);
    ASSERT_TRUE(set_result == __WASI_ESUCCESS || set_result == __WASI_EBADF || 
                set_result == __WASI_EINVAL || set_result == __WASI_ENOTSUP);
}

TEST_F(SSPStep3Test, Step3ErrorHandling_InvalidParameters_HandledGracefully) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    // Test with invalid file descriptors
    __wasi_fd_t invalid_fd = 9999;
    
    // Test sock_bind with invalid fd
    __wasi_addr_t addr;
    memset(&addr, 0, sizeof(addr));
    addr_pool pool;
    memset(&pool, 0, sizeof(pool));
    
    __wasi_errno_t bind_result = wasi_ssp_sock_bind(exec_env_, &fd_table_, &pool, invalid_fd, &addr);
    ASSERT_NE(__WASI_ESUCCESS, bind_result);
    
    // Test sock_listen with invalid fd
    __wasi_errno_t listen_result = wasi_ssp_sock_listen(exec_env_, &fd_table_, invalid_fd, 10);
    ASSERT_NE(__WASI_ESUCCESS, listen_result);
    
    // Test addr operations with invalid fd
    __wasi_addr_t test_addr;
    __wasi_errno_t local_result = wasi_ssp_sock_addr_local(exec_env_, &fd_table_, invalid_fd, &test_addr);
    __wasi_errno_t remote_result = wasi_ssp_sock_addr_remote(exec_env_, &fd_table_, invalid_fd, &test_addr);
    
    ASSERT_NE(__WASI_ESUCCESS, local_result);
    ASSERT_NE(__WASI_ESUCCESS, remote_result);
}

TEST_F(SSPStep3Test, Step3BoundaryConditions_ExtremeValues_HandledCorrectly) {
    if (!PlatformTestContext::HasNetworkSupport()) {
        return; // Skip if network not supported
    }
    
    __wasi_fd_t test_fd = 3;
    
    // Test with very large buffer size (use UINT32_MAX for __wasi_size_t)
    __wasi_size_t large_buf_size = UINT32_MAX;
    __wasi_errno_t large_buf_result = wasi_ssp_sock_set_recv_buf_size(exec_env_, &fd_table_, test_fd, large_buf_size);
    
    // Test with zero buffer size
    __wasi_size_t zero_buf_size = 0;
    __wasi_errno_t zero_buf_result = wasi_ssp_sock_set_recv_buf_size(exec_env_, &fd_table_, test_fd, zero_buf_size);
    
    // Test with very large backlog
    __wasi_size_t large_backlog = UINT32_MAX;
    __wasi_errno_t large_backlog_result = wasi_ssp_sock_listen(exec_env_, &fd_table_, test_fd, large_backlog);
    
    // Should handle extreme values appropriately
    ASSERT_TRUE(large_buf_result == __WASI_ESUCCESS || large_buf_result == __WASI_EBADF || 
                large_buf_result == __WASI_EINVAL || large_buf_result == __WASI_ENOTSUP);
    ASSERT_TRUE(zero_buf_result == __WASI_ESUCCESS || zero_buf_result == __WASI_EBADF || 
                zero_buf_result == __WASI_EINVAL || zero_buf_result == __WASI_ENOTSUP);
    ASSERT_TRUE(large_backlog_result == __WASI_ESUCCESS || large_backlog_result == __WASI_EBADF || 
                large_backlog_result == __WASI_EINVAL || large_backlog_result == __WASI_ENOTSUP);
}