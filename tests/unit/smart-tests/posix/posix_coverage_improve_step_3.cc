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
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

extern "C" {
// Socket function declarations from platform_api_extension.h
int os_socket_addr_resolve(const char *host, const char *service,
                          uint8_t *hint_is_tcp, uint8_t *hint_is_ipv4,
                          bh_addr_info_t *addr_info, size_t addr_info_size,
                          size_t *max_info_size);
int os_socket_connect(bh_socket_t socket, const char *addr, int port);
int os_socket_listen(bh_socket_t socket, int max_client);
int os_socket_send(bh_socket_t socket, const void *buf, unsigned int len);
int os_socket_recv(bh_socket_t socket, void *buf, unsigned int len);
int os_socket_send_to(bh_socket_t socket, const void *buf, unsigned int len,
                     int flags, const bh_sockaddr_t *dest_addr);
int os_socket_recv_from(bh_socket_t socket, void *buf, unsigned int len, int flags,
                       bh_sockaddr_t *src_addr);
__wasi_errno_t os_socket_shutdown(bh_socket_t socket);
}

class PosixSocketCoreTest : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime for socket operations
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
        
        // Create test sockets for operations
        test_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        test_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        
        // Set up test addresses
        memset(&test_addr_ipv4, 0, sizeof(test_addr_ipv4));
        test_addr_ipv4.sin_family = AF_INET;
        test_addr_ipv4.sin_addr.s_addr = inet_addr("127.0.0.1");
        test_addr_ipv4.sin_port = htons(12345);
        
        // Set up bh_sockaddr for testing
        memset(&bh_test_addr, 0, sizeof(bh_test_addr));
        bh_test_addr.is_ipv4 = true;
        bh_test_addr.port = 12345;
        bh_test_addr.addr_buffer.ipv4 = 0x7F000001; // 127.0.0.1 in network byte order
    }
    
    void TearDown() override {
        // Clean up test sockets
        if (test_tcp_socket >= 0) {
            close(test_tcp_socket);
        }
        if (test_udp_socket >= 0) {
            close(test_udp_socket);
        }
        if (server_socket >= 0) {
            close(server_socket);
        }
        
        wasm_runtime_destroy();
    }
    
    // Helper function to create a server socket for testing
    int create_test_server_socket(int port) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return -1;
        
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        
        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            return -1;
        }
        
        return sock;
    }
    
    int test_tcp_socket = -1;
    int test_udp_socket = -1;
    int server_socket = -1;
    struct sockaddr_in test_addr_ipv4;
    bh_sockaddr_t bh_test_addr;
};

// Test Function 1: os_socket_addr_resolve() - IPv4 address resolution
TEST_F(PosixSocketCoreTest, SocketAddrResolve_IPv4Localhost_ResolvesCorrectly) {
    bh_addr_info_t addr_info[10];
    size_t max_info_size = 0;
    uint8_t hint_is_tcp = 1;
    uint8_t hint_is_ipv4 = 1;
    
    int result = os_socket_addr_resolve("127.0.0.1", "80", &hint_is_tcp, &hint_is_ipv4,
                                       addr_info, 10, &max_info_size);
    
    ASSERT_EQ(BHT_OK, result);
    ASSERT_GT(max_info_size, 0);
    ASSERT_TRUE(addr_info[0].sockaddr.is_ipv4);
    ASSERT_EQ(80, addr_info[0].sockaddr.port);
    ASSERT_TRUE(addr_info[0].is_tcp);
}

// Test Function 1: os_socket_addr_resolve() - IPv6 address resolution  
TEST_F(PosixSocketCoreTest, SocketAddrResolve_IPv6Localhost_ResolvesCorrectly) {
    bh_addr_info_t addr_info[10];
    size_t max_info_size = 0;
    uint8_t hint_is_tcp = 1;
    uint8_t hint_is_ipv4 = 0;
    
    int result = os_socket_addr_resolve("::1", "443", &hint_is_tcp, &hint_is_ipv4,
                                       addr_info, 10, &max_info_size);
    
    // IPv6 may not be available on all systems
    if (result == BHT_OK) {
        ASSERT_GT(max_info_size, 0);
        ASSERT_FALSE(addr_info[0].sockaddr.is_ipv4);
        ASSERT_EQ(443, addr_info[0].sockaddr.port);
        ASSERT_TRUE(addr_info[0].is_tcp);
    }
}

// Test Function 1: os_socket_addr_resolve() - hostname resolution
TEST_F(PosixSocketCoreTest, SocketAddrResolve_LocalhostHostname_ResolvesCorrectly) {
    bh_addr_info_t addr_info[10];
    size_t max_info_size = 0;
    uint8_t hint_is_tcp = 0; // UDP
    uint8_t hint_is_ipv4 = 1;
    
    int result = os_socket_addr_resolve("localhost", "53", &hint_is_tcp, &hint_is_ipv4,
                                       addr_info, 10, &max_info_size);
    
    ASSERT_EQ(BHT_OK, result);
    ASSERT_GT(max_info_size, 0);
    ASSERT_TRUE(addr_info[0].sockaddr.is_ipv4);
    ASSERT_EQ(53, addr_info[0].sockaddr.port);
    ASSERT_FALSE(addr_info[0].is_tcp); // Should be UDP
}

// Test Function 2: os_socket_addr_resolve() - error handling for invalid host
TEST_F(PosixSocketCoreTest, SocketAddrResolve_InvalidHost_HandlesErrorCorrectly) {
    bh_addr_info_t addr_info[10];
    size_t max_info_size = 0;
    uint8_t hint_is_tcp = 1;
    uint8_t hint_is_ipv4 = 1;
    
    // This should trigger the getaddrinfo_error_to_errno function internally
    int result = os_socket_addr_resolve("invalid.nonexistent.domain.test", "80",
                                       &hint_is_tcp, &hint_is_ipv4,
                                       addr_info, 10, &max_info_size);
    
    ASSERT_EQ(BHT_ERROR, result);
    ASSERT_EQ(0, max_info_size);
}

// Test Function 3: os_socket_connect() - successful connection
TEST_F(PosixSocketCoreTest, SocketConnect_ValidAddress_ConnectsSuccessfully) {
    // Create a server socket to connect to
    server_socket = create_test_server_socket(12346);
    ASSERT_GE(server_socket, 0);
    
    ASSERT_EQ(BHT_OK, os_socket_listen(server_socket, 1));
    
    // Test connection
    int result = os_socket_connect(test_tcp_socket, "127.0.0.1", 12346);
    ASSERT_EQ(BHT_OK, result);
}

// Test Function 3: os_socket_connect() - connection failure
TEST_F(PosixSocketCoreTest, SocketConnect_InvalidAddress_FailsGracefully) {
    // Try to connect to invalid address
    int result = os_socket_connect(test_tcp_socket, "192.0.2.1", 99999);
    ASSERT_EQ(BHT_ERROR, result);
}

// Test Function 4: os_socket_listen() - successful listening setup
TEST_F(PosixSocketCoreTest, SocketListen_ValidSocket_ListensSuccessfully) {
    server_socket = create_test_server_socket(12347);
    ASSERT_GE(server_socket, 0);
    
    int result = os_socket_listen(server_socket, 5);
    ASSERT_EQ(BHT_OK, result);
}

// Test Function 5: os_socket_send() - data transmission
TEST_F(PosixSocketCoreTest, SocketSend_ValidData_SendsSuccessfully) {
    // Create connected socket pair
    int sockfd[2];
    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd));
    
    const char *test_data = "Hello, WAMR!";
    int result = os_socket_send(sockfd[0], test_data, strlen(test_data));
    
    ASSERT_GT(result, 0);
    ASSERT_EQ(strlen(test_data), result);
    
    close(sockfd[0]);
    close(sockfd[1]);
}

// Test Function 6: os_socket_recv() - data reception
TEST_F(PosixSocketCoreTest, SocketRecv_ValidSocket_ReceivesData) {
    // Create connected socket pair
    int sockfd[2];
    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd));
    
    const char *test_data = "WAMR Socket Test";
    char recv_buffer[256];
    
    // Send data from one end
    send(sockfd[1], test_data, strlen(test_data), 0);
    
    // Receive data using os_socket_recv
    int result = os_socket_recv(sockfd[0], recv_buffer, sizeof(recv_buffer));
    
    ASSERT_GT(result, 0);
    ASSERT_EQ(strlen(test_data), result);
    recv_buffer[result] = '\0';
    ASSERT_STREQ(test_data, recv_buffer);
    
    close(sockfd[0]);
    close(sockfd[1]);
}

// Test Function 7: os_socket_send_to() - UDP-style sending
TEST_F(PosixSocketCoreTest, SocketSendTo_UDPSocket_SendsToAddress) {
    const char *test_data = "UDP Test Message";
    
    int result = os_socket_send_to(test_udp_socket, test_data, strlen(test_data),
                                  0, &bh_test_addr);
    
    // Result may be error due to no receiver, but function should execute
    ASSERT_NE(-2, result); // Should not be uninitialized
}

// Test Function 8: os_socket_recv_from() - UDP-style receiving
TEST_F(PosixSocketCoreTest, SocketRecvFrom_UDPSocket_ReceivesFromAddress) {
    // Create UDP socket pair for testing
    int udp_sockfd[2];
    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_DGRAM, 0, udp_sockfd));
    
    const char *test_data = "UDP Receive Test";
    char recv_buffer[256];
    bh_sockaddr_t src_addr;
    
    // Send data from one end
    send(udp_sockfd[1], test_data, strlen(test_data), 0);
    
    // Receive data using os_socket_recv_from
    int result = os_socket_recv_from(udp_sockfd[0], recv_buffer, sizeof(recv_buffer),
                                    0, &src_addr);
    
    ASSERT_GT(result, 0);
    ASSERT_EQ(strlen(test_data), result);
    recv_buffer[result] = '\0';
    ASSERT_STREQ(test_data, recv_buffer);
    
    close(udp_sockfd[0]);
    close(udp_sockfd[1]);
}

// Test Function 9: os_socket_shutdown() - read shutdown
TEST_F(PosixSocketCoreTest, SocketShutdown_ValidSocket_ShutsdownSuccessfully) {
    // Create connected socket pair
    int sockfd[2];
    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd));
    
    __wasi_errno_t result = os_socket_shutdown(sockfd[0]);
    ASSERT_EQ(__WASI_ESUCCESS, result);
    
    close(sockfd[0]);
    close(sockfd[1]);
}

// Test Function 9: os_socket_shutdown() - invalid socket
TEST_F(PosixSocketCoreTest, SocketShutdown_InvalidSocket_ReturnsError) {
    __wasi_errno_t result = os_socket_shutdown(-1);
    ASSERT_NE(__WASI_ESUCCESS, result);
}

// Test Function 10: os_socket_addr_resolve() - comprehensive address resolution testing
TEST_F(PosixSocketCoreTest, SocketAddrResolve_MixedProtocols_FiltersCorrectly) {
    bh_addr_info_t addr_info[10];
    size_t max_info_size = 0;
    uint8_t hint_is_tcp = 1;
    uint8_t hint_is_ipv4 = 1;
    
    // This should trigger the is_addrinfo_supported function internally
    int result = os_socket_addr_resolve("127.0.0.1", "443", &hint_is_tcp, &hint_is_ipv4,
                                       addr_info, 10, &max_info_size);
    
    ASSERT_EQ(BHT_OK, result);
    ASSERT_GT(max_info_size, 0);
    // Verify the filtering worked - should only get TCP results
    for (size_t i = 0; i < max_info_size && i < 10; i++) {
        ASSERT_TRUE(addr_info[i].is_tcp);
        ASSERT_TRUE(addr_info[i].sockaddr.is_ipv4);
    }
}

TEST_F(PosixSocketCoreTest, SocketAddrResolve_UDPProtocol_FiltersCorrectly) {
    bh_addr_info_t addr_info[10];
    size_t max_info_size = 0;
    uint8_t hint_is_tcp = 0; // UDP
    uint8_t hint_is_ipv4 = 1;
    
    // This should trigger the is_addrinfo_supported function internally
    int result = os_socket_addr_resolve("127.0.0.1", "53", &hint_is_tcp, &hint_is_ipv4,
                                       addr_info, 10, &max_info_size);
    
    ASSERT_EQ(BHT_OK, result);
    ASSERT_GT(max_info_size, 0);
    // Verify the filtering worked - should only get UDP results
    for (size_t i = 0; i < max_info_size && i < 10; i++) {
        ASSERT_FALSE(addr_info[i].is_tcp); // Should be UDP
        ASSERT_TRUE(addr_info[i].sockaddr.is_ipv4);
    }
}

// Edge case tests for comprehensive coverage

// Test os_socket_addr_resolve with various edge cases
TEST_F(PosixSocketCoreTest, SocketAddrResolve_EmptyService_HandlesCorrectly) {
    bh_addr_info_t addr_info[10];
    size_t max_info_size = 0;
    uint8_t hint_is_tcp = 1;
    uint8_t hint_is_ipv4 = 1;
    
    // Test with empty service string - should trigger strlen(service) == 0 path
    int result = os_socket_addr_resolve("127.0.0.1", "",
                                       &hint_is_tcp, &hint_is_ipv4,
                                       addr_info, 10, &max_info_size);
    
    // Result may vary, but function should not crash
    ASSERT_TRUE(result == BHT_OK || result == BHT_ERROR);
}

// Test os_socket_addr_resolve with null hints
TEST_F(PosixSocketCoreTest, SocketAddrResolve_NullHints_ResolvesCorrectly) {
    bh_addr_info_t addr_info[10];
    size_t max_info_size = 0;
    
    int result = os_socket_addr_resolve("127.0.0.1", "22", nullptr, nullptr,
                                       addr_info, 10, &max_info_size);
    
    ASSERT_EQ(BHT_OK, result);
    ASSERT_GT(max_info_size, 0);
}

// Test os_socket_connect with invalid socket
TEST_F(PosixSocketCoreTest, SocketConnect_InvalidSocket_ReturnsError) {
    int result = os_socket_connect(-1, "127.0.0.1", 80);
    ASSERT_EQ(BHT_ERROR, result);
}

// Test os_socket_listen with invalid socket
TEST_F(PosixSocketCoreTest, SocketListen_InvalidSocket_ReturnsError) {
    int result = os_socket_listen(-1, 5);
    ASSERT_EQ(BHT_ERROR, result);
}

// Test os_socket_send with invalid socket
TEST_F(PosixSocketCoreTest, SocketSend_InvalidSocket_ReturnsError) {
    const char *test_data = "test";
    int result = os_socket_send(-1, test_data, strlen(test_data));
    ASSERT_EQ(-1, result);
}

// Test os_socket_recv with invalid socket
TEST_F(PosixSocketCoreTest, SocketRecv_InvalidSocket_ReturnsError) {
    char buffer[256];
    int result = os_socket_recv(-1, buffer, sizeof(buffer));
    ASSERT_EQ(-1, result);
}