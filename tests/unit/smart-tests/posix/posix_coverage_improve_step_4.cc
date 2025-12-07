/*
 * Copyright (C) 2024 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "gtest/gtest.h"
#include "test_helper.h"
#include "wasm_export.h"
#include "bh_platform.h"
#include "platform_api_extension.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

extern "C" {
// Socket configuration function declarations
int os_socket_get_recv_buf_size(bh_socket_t socket, size_t *bufsiz);
int os_socket_set_recv_buf_size(bh_socket_t socket, size_t bufsiz);
int os_socket_get_send_buf_size(bh_socket_t socket, size_t *bufsiz);
int os_socket_set_send_buf_size(bh_socket_t socket, size_t bufsiz);
int os_socket_get_recv_timeout(bh_socket_t socket, uint64 *timeout_us);
int os_socket_set_recv_timeout(bh_socket_t socket, uint64 timeout_us);
int os_socket_get_send_timeout(bh_socket_t socket, uint64 *timeout_us);
int os_socket_set_send_timeout(bh_socket_t socket, uint64 timeout_us);
int os_socket_get_reuse_addr(bh_socket_t socket, bool *is_enabled);
int os_socket_set_reuse_addr(bh_socket_t socket, bool is_enabled);
int os_socket_get_reuse_port(bh_socket_t socket, bool *is_enabled);
int os_socket_set_reuse_port(bh_socket_t socket, bool is_enabled);
}

class PosixSocketConfigTest : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime for socket operations
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
        
        // Create test sockets for configuration operations
        test_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        test_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        
        ASSERT_NE(-1, test_tcp_socket) << "Failed to create TCP test socket";
        ASSERT_NE(-1, test_udp_socket) << "Failed to create UDP test socket";
    }
    
    void TearDown() override {
        // Clean up test sockets
        if (test_tcp_socket != -1) {
            close(test_tcp_socket);
        }
        if (test_udp_socket != -1) {
            close(test_udp_socket);
        }
        
        wasm_runtime_destroy();
    }
    
    bh_socket_t test_tcp_socket = -1;
    bh_socket_t test_udp_socket = -1;
};

// Buffer Size Configuration Tests

TEST_F(PosixSocketConfigTest, os_socket_get_recv_buf_size_ValidSocket_ReturnsSize) {
    size_t buffer_size = 0;
    
    int result = os_socket_get_recv_buf_size(test_tcp_socket, &buffer_size);
    
    ASSERT_EQ(BHT_OK, result) << "Should successfully get receive buffer size";
    ASSERT_GT(buffer_size, 0) << "Buffer size should be positive";
}

TEST_F(PosixSocketConfigTest, os_socket_set_recv_buf_size_ValidSize_SetsCorrectly) {
    size_t new_size = 8192;
    size_t retrieved_size = 0;
    
    int set_result = os_socket_set_recv_buf_size(test_tcp_socket, new_size);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set receive buffer size";
    
    int get_result = os_socket_get_recv_buf_size(test_tcp_socket, &retrieved_size);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get receive buffer size";
    ASSERT_GE(retrieved_size, new_size) << "Retrieved size should be at least the set size";
}

TEST_F(PosixSocketConfigTest, os_socket_get_send_buf_size_ValidSocket_ReturnsSize) {
    size_t buffer_size = 0;
    
    int result = os_socket_get_send_buf_size(test_tcp_socket, &buffer_size);
    
    ASSERT_EQ(BHT_OK, result) << "Should successfully get send buffer size";
    ASSERT_GT(buffer_size, 0) << "Buffer size should be positive";
}

TEST_F(PosixSocketConfigTest, os_socket_set_send_buf_size_ValidSize_SetsCorrectly) {
    size_t new_size = 16384;
    size_t retrieved_size = 0;
    
    int set_result = os_socket_set_send_buf_size(test_tcp_socket, new_size);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set send buffer size";
    
    int get_result = os_socket_get_send_buf_size(test_tcp_socket, &retrieved_size);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get send buffer size";
    ASSERT_GE(retrieved_size, new_size) << "Retrieved size should be at least the set size";
}

// Timeout Configuration Tests

TEST_F(PosixSocketConfigTest, os_socket_get_recv_timeout_ValidSocket_ReturnsTimeout) {
    uint64 timeout_us = 0;
    
    int result = os_socket_get_recv_timeout(test_tcp_socket, &timeout_us);
    
    ASSERT_EQ(BHT_OK, result) << "Should successfully get receive timeout";
    // Note: Default timeout might be 0 (no timeout)
}

TEST_F(PosixSocketConfigTest, os_socket_set_recv_timeout_ValidTimeout_SetsCorrectly) {
    uint64 new_timeout = 5000000; // 5 seconds in microseconds
    uint64 retrieved_timeout = 0;
    
    int set_result = os_socket_set_recv_timeout(test_tcp_socket, new_timeout);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set receive timeout";
    
    int get_result = os_socket_get_recv_timeout(test_tcp_socket, &retrieved_timeout);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get receive timeout";
    ASSERT_EQ(new_timeout, retrieved_timeout) << "Retrieved timeout should match set timeout";
}

TEST_F(PosixSocketConfigTest, os_socket_get_send_timeout_ValidSocket_ReturnsTimeout) {
    uint64 timeout_us = 0;
    
    int result = os_socket_get_send_timeout(test_tcp_socket, &timeout_us);
    
    ASSERT_EQ(BHT_OK, result) << "Should successfully get send timeout";
    // Note: Default timeout might be 0 (no timeout)
}

TEST_F(PosixSocketConfigTest, os_socket_set_send_timeout_ValidTimeout_SetsCorrectly) {
    uint64 new_timeout = 3000000; // 3 seconds in microseconds
    uint64 retrieved_timeout = 0;
    
    int set_result = os_socket_set_send_timeout(test_tcp_socket, new_timeout);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set send timeout";
    
    int get_result = os_socket_get_send_timeout(test_tcp_socket, &retrieved_timeout);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get send timeout";
    ASSERT_EQ(new_timeout, retrieved_timeout) << "Retrieved timeout should match set timeout";
}

// Address Reuse Configuration Tests

TEST_F(PosixSocketConfigTest, os_socket_get_reuse_addr_ValidSocket_ReturnsState) {
    bool is_enabled = false;
    
    int result = os_socket_get_reuse_addr(test_tcp_socket, &is_enabled);
    
    ASSERT_EQ(BHT_OK, result) << "Should successfully get reuse address state";
    // Note: Default state is typically false
}

TEST_F(PosixSocketConfigTest, os_socket_set_reuse_addr_EnableFlag_SetsCorrectly) {
    bool enable_flag = true;
    bool retrieved_flag = false;
    
    int set_result = os_socket_set_reuse_addr(test_tcp_socket, enable_flag);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set reuse address flag";
    
    int get_result = os_socket_get_reuse_addr(test_tcp_socket, &retrieved_flag);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get reuse address flag";
    ASSERT_EQ(enable_flag, retrieved_flag) << "Retrieved flag should match set flag";
}

TEST_F(PosixSocketConfigTest, os_socket_set_reuse_addr_DisableFlag_SetsCorrectly) {
    bool enable_flag = false;
    bool retrieved_flag = true;
    
    int set_result = os_socket_set_reuse_addr(test_tcp_socket, enable_flag);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set reuse address flag";
    
    int get_result = os_socket_get_reuse_addr(test_tcp_socket, &retrieved_flag);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get reuse address flag";
    ASSERT_EQ(enable_flag, retrieved_flag) << "Retrieved flag should match set flag";
}

// Port Reuse Configuration Tests (Platform-dependent)

TEST_F(PosixSocketConfigTest, os_socket_get_reuse_port_ValidSocket_ReturnsStateOrError) {
    bool is_enabled = false;
    
    int result = os_socket_get_reuse_port(test_tcp_socket, &is_enabled);
    
    // Platform-dependent: SO_REUSEPORT might not be available on all systems
    if (result == BHT_OK) {
        // SO_REUSEPORT is supported on this platform
        ASSERT_TRUE(true) << "Successfully got reuse port state";
    } else {
        // SO_REUSEPORT is not supported (e.g., NuttX)
        ASSERT_EQ(BHT_ERROR, result) << "Expected error for unsupported SO_REUSEPORT";
    }
}

TEST_F(PosixSocketConfigTest, os_socket_set_reuse_port_EnableFlag_SetsOrReturnsError) {
    bool enable_flag = true;
    
    int set_result = os_socket_set_reuse_port(test_tcp_socket, enable_flag);
    
    // Platform-dependent: SO_REUSEPORT might not be available on all systems
    if (set_result == BHT_OK) {
        // SO_REUSEPORT is supported, verify the setting
        bool retrieved_flag = false;
        int get_result = os_socket_get_reuse_port(test_tcp_socket, &retrieved_flag);
        ASSERT_EQ(BHT_OK, get_result) << "Should successfully get reuse port flag";
        ASSERT_EQ(enable_flag, retrieved_flag) << "Retrieved flag should match set flag";
    } else {
        // SO_REUSEPORT is not supported (e.g., NuttX)
        ASSERT_EQ(BHT_ERROR, set_result) << "Expected error for unsupported SO_REUSEPORT";
    }
}

// Error Handling Tests

TEST_F(PosixSocketConfigTest, os_socket_get_recv_buf_size_InvalidSocket_ReturnsError) {
    size_t buffer_size = 0;
    bh_socket_t invalid_socket = -1;
    
    int result = os_socket_get_recv_buf_size(invalid_socket, &buffer_size);
    
    ASSERT_EQ(BHT_ERROR, result) << "Should return error for invalid socket";
}

TEST_F(PosixSocketConfigTest, os_socket_set_send_buf_size_InvalidSocket_ReturnsError) {
    size_t buffer_size = 8192;
    bh_socket_t invalid_socket = -1;
    
    int result = os_socket_set_send_buf_size(invalid_socket, buffer_size);
    
    ASSERT_EQ(BHT_ERROR, result) << "Should return error for invalid socket";
}

TEST_F(PosixSocketConfigTest, os_socket_get_recv_timeout_InvalidSocket_ReturnsError) {
    uint64 timeout_us = 0;
    bh_socket_t invalid_socket = -1;
    
    int result = os_socket_get_recv_timeout(invalid_socket, &timeout_us);
    
    ASSERT_EQ(BHT_ERROR, result) << "Should return error for invalid socket";
}

TEST_F(PosixSocketConfigTest, os_socket_set_send_timeout_InvalidSocket_ReturnsError) {
    uint64 timeout_us = 5000000;
    bh_socket_t invalid_socket = -1;
    
    int result = os_socket_set_send_timeout(invalid_socket, timeout_us);
    
    ASSERT_EQ(BHT_ERROR, result) << "Should return error for invalid socket";
}

TEST_F(PosixSocketConfigTest, os_socket_get_reuse_addr_InvalidSocket_ReturnsError) {
    bool is_enabled = false;
    bh_socket_t invalid_socket = -1;
    
    int result = os_socket_get_reuse_addr(invalid_socket, &is_enabled);
    
    ASSERT_EQ(BHT_ERROR, result) << "Should return error for invalid socket";
}

TEST_F(PosixSocketConfigTest, os_socket_set_reuse_addr_InvalidSocket_ReturnsError) {
    bool is_enabled = true;
    bh_socket_t invalid_socket = -1;
    
    int result = os_socket_set_reuse_addr(invalid_socket, is_enabled);
    
    ASSERT_EQ(BHT_ERROR, result) << "Should return error for invalid socket";
}

// UDP Socket Configuration Tests

TEST_F(PosixSocketConfigTest, os_socket_get_recv_buf_size_UDPSocket_ReturnsSize) {
    size_t buffer_size = 0;
    
    int result = os_socket_get_recv_buf_size(test_udp_socket, &buffer_size);
    
    ASSERT_EQ(BHT_OK, result) << "Should successfully get UDP receive buffer size";
    ASSERT_GT(buffer_size, 0) << "UDP buffer size should be positive";
}

TEST_F(PosixSocketConfigTest, os_socket_set_send_buf_size_UDPSocket_SetsCorrectly) {
    size_t new_size = 32768;
    size_t retrieved_size = 0;
    
    int set_result = os_socket_set_send_buf_size(test_udp_socket, new_size);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set UDP send buffer size";
    
    int get_result = os_socket_get_send_buf_size(test_udp_socket, &retrieved_size);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get UDP send buffer size";
    ASSERT_GE(retrieved_size, new_size) << "Retrieved UDP size should be at least the set size";
}

TEST_F(PosixSocketConfigTest, os_socket_set_recv_timeout_UDPSocket_SetsCorrectly) {
    uint64 new_timeout = 2000000; // 2 seconds in microseconds
    uint64 retrieved_timeout = 0;
    
    int set_result = os_socket_set_recv_timeout(test_udp_socket, new_timeout);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set UDP receive timeout";
    
    int get_result = os_socket_get_recv_timeout(test_udp_socket, &retrieved_timeout);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get UDP receive timeout";
    ASSERT_EQ(new_timeout, retrieved_timeout) << "Retrieved UDP timeout should match set timeout";
}

TEST_F(PosixSocketConfigTest, os_socket_set_reuse_addr_UDPSocket_SetsCorrectly) {
    bool enable_flag = true;
    bool retrieved_flag = false;
    
    int set_result = os_socket_set_reuse_addr(test_udp_socket, enable_flag);
    ASSERT_EQ(BHT_OK, set_result) << "Should successfully set UDP reuse address flag";
    
    int get_result = os_socket_get_reuse_addr(test_udp_socket, &retrieved_flag);
    ASSERT_EQ(BHT_OK, get_result) << "Should successfully get UDP reuse address flag";
    ASSERT_EQ(enable_flag, retrieved_flag) << "Retrieved UDP flag should match set flag";
}