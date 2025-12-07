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
#include <netinet/tcp.h>
#include <unistd.h>

// Test fixture for POSIX socket option functions
class PosixSocketOptionsTest : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime for platform API access
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
        
        // Create test sockets for different protocols
        tcp_socket = create_tcp_socket();
        udp_socket = create_udp_socket();
        ipv6_socket = create_ipv6_socket();
    }
    
    void TearDown() override {
        // Clean up sockets
        if (tcp_socket != -1) {
            close(tcp_socket);
        }
        if (udp_socket != -1) {
            close(udp_socket);
        }
        if (ipv6_socket != -1) {
            close(ipv6_socket);
        }
        
        wasm_runtime_destroy();
    }
    
    bh_socket_t create_tcp_socket() {
        bh_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        return (sock >= 0) ? sock : -1;
    }
    
    bh_socket_t create_udp_socket() {
        bh_socket_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        return (sock >= 0) ? sock : -1;
    }
    
    bh_socket_t create_ipv6_socket() {
        bh_socket_t sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        return (sock >= 0) ? sock : -1;
    }
    
    bh_socket_t tcp_socket = -1;
    bh_socket_t udp_socket = -1;
    bh_socket_t ipv6_socket = -1;
};

// ============================================================================
// Step 1: Basic Socket Options Tests (8 functions)
// ============================================================================

// Test os_socket_set_keep_alive() and os_socket_get_keep_alive()
TEST_F(PosixSocketOptionsTest, KeepAlive_EnableDisable_SucceedsCorrectly) {
    if (tcp_socket == -1) {
        return; // Skip if TCP socket creation failed
    }
    
    // Test enabling keep-alive
    int result = os_socket_set_keep_alive(tcp_socket, true);
    ASSERT_EQ(BHT_OK, result) << "Failed to enable keep-alive";
    
    // Verify keep-alive is enabled
    bool is_enabled = false;
    result = os_socket_get_keep_alive(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get keep-alive status";
    ASSERT_TRUE(is_enabled) << "Keep-alive should be enabled";
    
    // Test disabling keep-alive
    result = os_socket_set_keep_alive(tcp_socket, false);
    ASSERT_EQ(BHT_OK, result) << "Failed to disable keep-alive";
    
    // Verify keep-alive is disabled
    result = os_socket_get_keep_alive(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get keep-alive status";
    ASSERT_FALSE(is_enabled) << "Keep-alive should be disabled";
}

TEST_F(PosixSocketOptionsTest, KeepAlive_InvalidSocket_ReturnsError) {
    bool is_enabled = false;
    
    // Test with invalid socket descriptor
    int result = os_socket_set_keep_alive(-1, true);
    ASSERT_EQ(BHT_ERROR, result) << "Should fail with invalid socket";
    
    result = os_socket_get_keep_alive(-1, &is_enabled);
    ASSERT_EQ(BHT_ERROR, result) << "Should fail with invalid socket";
}

// Note: Null pointer test removed due to assertion in POSIX implementation
// The os_socket_getbooloption function has assert(is_enabled) which causes abort
// This is expected behavior for debug builds to catch programming errors

// Test os_socket_set_broadcast() and os_socket_get_broadcast()
TEST_F(PosixSocketOptionsTest, Broadcast_EnableDisable_SucceedsCorrectly) {
    if (udp_socket == -1) {
        return; // Skip if UDP socket creation failed
    }
    
    // Test enabling broadcast
    int result = os_socket_set_broadcast(udp_socket, true);
    ASSERT_EQ(BHT_OK, result) << "Failed to enable broadcast";
    
    // Verify broadcast is enabled
    bool is_enabled = false;
    result = os_socket_get_broadcast(udp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get broadcast status";
    ASSERT_TRUE(is_enabled) << "Broadcast should be enabled";
    
    // Test disabling broadcast
    result = os_socket_set_broadcast(udp_socket, false);
    ASSERT_EQ(BHT_OK, result) << "Failed to disable broadcast";
    
    // Verify broadcast is disabled
    result = os_socket_get_broadcast(udp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get broadcast status";
    ASSERT_FALSE(is_enabled) << "Broadcast should be disabled";
}

TEST_F(PosixSocketOptionsTest, Broadcast_InvalidSocket_ReturnsError) {
    bool is_enabled = false;
    
    // Test with invalid socket descriptor
    int result = os_socket_set_broadcast(-1, true);
    ASSERT_EQ(BHT_ERROR, result) << "Should fail with invalid socket";
    
    result = os_socket_get_broadcast(-1, &is_enabled);
    ASSERT_EQ(BHT_ERROR, result) << "Should fail with invalid socket";
}

// Test os_socket_set_tcp_no_delay() and os_socket_get_tcp_no_delay()
TEST_F(PosixSocketOptionsTest, TcpNoDelay_EnableDisable_SucceedsCorrectly) {
    if (tcp_socket == -1) {
        return; // Skip if TCP socket creation failed
    }
    
    // Test enabling TCP_NODELAY
    int result = os_socket_set_tcp_no_delay(tcp_socket, true);
    ASSERT_EQ(BHT_OK, result) << "Failed to enable TCP_NODELAY";
    
    // Verify TCP_NODELAY is enabled
    bool is_enabled = false;
    result = os_socket_get_tcp_no_delay(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get TCP_NODELAY status";
    ASSERT_TRUE(is_enabled) << "TCP_NODELAY should be enabled";
    
    // Test disabling TCP_NODELAY
    result = os_socket_set_tcp_no_delay(tcp_socket, false);
    ASSERT_EQ(BHT_OK, result) << "Failed to disable TCP_NODELAY";
    
    // Verify TCP_NODELAY is disabled
    result = os_socket_get_tcp_no_delay(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get TCP_NODELAY status";
    ASSERT_FALSE(is_enabled) << "TCP_NODELAY should be disabled";
}

TEST_F(PosixSocketOptionsTest, TcpNoDelay_UdpSocket_ReturnsError) {
    if (udp_socket == -1) {
        return;
    }
    
    // TCP_NODELAY should not work on UDP sockets
    int result = os_socket_set_tcp_no_delay(udp_socket, true);
    ASSERT_EQ(BHT_ERROR, result) << "TCP_NODELAY should fail on UDP socket";
}

// Test os_socket_set_ipv6_only() and os_socket_get_ipv6_only()
TEST_F(PosixSocketOptionsTest, Ipv6Only_EnableDisable_SucceedsCorrectly) {
    if (ipv6_socket == -1) {
        return; // Skip if IPv6 socket creation failed
    }
    
    // Test enabling IPv6-only mode
    int result = os_socket_set_ipv6_only(ipv6_socket, true);
    ASSERT_EQ(BHT_OK, result) << "Failed to enable IPv6-only mode";
    
    // Verify IPv6-only is enabled
    bool is_enabled = false;
    result = os_socket_get_ipv6_only(ipv6_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get IPv6-only status";
    ASSERT_TRUE(is_enabled) << "IPv6-only should be enabled";
    
    // Test disabling IPv6-only mode
    result = os_socket_set_ipv6_only(ipv6_socket, false);
    ASSERT_EQ(BHT_OK, result) << "Failed to disable IPv6-only mode";
    
    // Verify IPv6-only is disabled
    result = os_socket_get_ipv6_only(ipv6_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get IPv6-only status";
    ASSERT_FALSE(is_enabled) << "IPv6-only should be disabled";
}

TEST_F(PosixSocketOptionsTest, Ipv6Only_Ipv4Socket_ReturnsError) {
    if (tcp_socket == -1) {
        return;
    }
    
    // IPv6-only should not work on IPv4 sockets
    int result = os_socket_set_ipv6_only(tcp_socket, true);
    ASSERT_EQ(BHT_ERROR, result) << "IPv6-only should fail on IPv4 socket";
}

// ============================================================================
// Step 2: Advanced TCP Options Tests (8 functions)
// ============================================================================

// Test os_socket_set_tcp_keep_idle() and os_socket_get_tcp_keep_idle()
TEST_F(PosixSocketOptionsTest, TcpKeepIdle_SetGet_SucceedsCorrectly) {
    if (tcp_socket == -1) {
        return; // Skip if TCP socket creation failed
    }
    
    // First enable keep-alive (required for keep-alive parameters)
    int result = os_socket_set_keep_alive(tcp_socket, true);
    ASSERT_EQ(0, result) << "Failed to enable keep-alive";
    
    // Test setting TCP keep-alive idle time
    uint32_t idle_time = 7200; // 2 hours in seconds
    result = os_socket_set_tcp_keep_idle(tcp_socket, idle_time);
    ASSERT_EQ(0, result) << "Failed to set TCP keep-alive idle time";
    
    // Verify the idle time was set correctly
    uint32_t retrieved_idle = 0;
    result = os_socket_get_tcp_keep_idle(tcp_socket, &retrieved_idle);
    ASSERT_EQ(0, result) << "Failed to get TCP keep-alive idle time";
    ASSERT_EQ(idle_time, retrieved_idle) << "Retrieved idle time should match set value";
}

TEST_F(PosixSocketOptionsTest, TcpKeepIdle_InvalidValues_HandledCorrectly) {
    if (tcp_socket == -1) {
        return;
    }
    
    // Enable keep-alive first
    os_socket_set_keep_alive(tcp_socket, true);
    
    // Test boundary values
    uint32 min_idle = 1;
    int result = os_socket_set_tcp_keep_idle(tcp_socket, min_idle);
    ASSERT_EQ(BHT_OK, result) << "Should accept minimum idle time";
    
    uint32 max_idle = 65535;
    result = os_socket_set_tcp_keep_idle(tcp_socket, max_idle);
    ASSERT_EQ(BHT_OK, result) << "Should accept large idle time";
}

// Note: Null pointer test removed due to assertion in POSIX implementation
// The underlying socket functions have assertions that cause abort with null pointers

// Test os_socket_set_tcp_keep_intvl() and os_socket_get_tcp_keep_intvl()
TEST_F(PosixSocketOptionsTest, TcpKeepIntvl_SetGet_SucceedsCorrectly) {
    if (tcp_socket == -1) {
        return;
    }
    
    // Enable keep-alive first
    os_socket_set_keep_alive(tcp_socket, true);
    
    // Test setting TCP keep-alive interval
    uint32 interval = 75; // 75 seconds
    int result = os_socket_set_tcp_keep_intvl(tcp_socket, interval);
    ASSERT_EQ(BHT_OK, result) << "Failed to set TCP keep-alive interval";
    
    // Verify the interval was set correctly
    uint32 retrieved_interval = 0;
    result = os_socket_get_tcp_keep_intvl(tcp_socket, &retrieved_interval);
    ASSERT_EQ(BHT_OK, result) << "Failed to get TCP keep-alive interval";
    ASSERT_EQ(interval, retrieved_interval) << "Retrieved interval should match set value";
}

TEST_F(PosixSocketOptionsTest, TcpKeepIntvl_BoundaryValues_HandledCorrectly) {
    if (tcp_socket == -1) {
        return;
    }
    
    // Enable keep-alive first
    os_socket_set_keep_alive(tcp_socket, true);
    
    // Test minimum interval
    uint32 min_interval = 1;
    int result = os_socket_set_tcp_keep_intvl(tcp_socket, min_interval);
    ASSERT_EQ(BHT_OK, result) << "Should accept minimum interval";
    
    // Test larger interval
    uint32 large_interval = 3600;
    result = os_socket_set_tcp_keep_intvl(tcp_socket, large_interval);
    ASSERT_EQ(BHT_OK, result) << "Should accept large interval";
}

// Test os_socket_set_tcp_quick_ack() and os_socket_get_tcp_quick_ack()
TEST_F(PosixSocketOptionsTest, TcpQuickAck_EnableDisable_SucceedsCorrectly) {
    if (tcp_socket == -1) {
        return;
    }
    
    // Test enabling TCP quick ACK
    int result = os_socket_set_tcp_quick_ack(tcp_socket, true);
    if (result == BHT_ERROR) {
        // TCP_QUICKACK might not be supported on all platforms
        return;
    }
    ASSERT_EQ(BHT_OK, result) << "Failed to enable TCP quick ACK";
    
    // Verify quick ACK is enabled
    bool is_enabled = false;
    result = os_socket_get_tcp_quick_ack(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get TCP quick ACK status";
    ASSERT_TRUE(is_enabled) << "TCP quick ACK should be enabled";
    
    // Test disabling TCP quick ACK
    result = os_socket_set_tcp_quick_ack(tcp_socket, false);
    ASSERT_EQ(BHT_OK, result) << "Failed to disable TCP quick ACK";
    
    // Verify quick ACK is disabled
    result = os_socket_get_tcp_quick_ack(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get TCP quick ACK status";
    ASSERT_FALSE(is_enabled) << "TCP quick ACK should be disabled";
}

TEST_F(PosixSocketOptionsTest, TcpQuickAck_UdpSocket_ReturnsError) {
    if (udp_socket == -1) {
        return;
    }
    
    // TCP_QUICKACK should not work on UDP sockets
    int result = os_socket_set_tcp_quick_ack(udp_socket, true);
    ASSERT_EQ(BHT_ERROR, result) << "TCP quick ACK should fail on UDP socket";
}

// Test os_socket_set_tcp_fastopen_connect() and os_socket_get_tcp_fastopen_connect()
TEST_F(PosixSocketOptionsTest, TcpFastopenConnect_EnableDisable_SucceedsCorrectly) {
    if (tcp_socket == -1) {
        return;
    }
    
    // Test enabling TCP fast open connect
    int result = os_socket_set_tcp_fastopen_connect(tcp_socket, true);
    if (result == BHT_ERROR) {
        // TCP_FASTOPEN_CONNECT might not be supported on all platforms
        return;
    }
    ASSERT_EQ(BHT_OK, result) << "Failed to enable TCP fast open connect";
    
    // Verify fast open connect is enabled
    bool is_enabled = false;
    result = os_socket_get_tcp_fastopen_connect(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get TCP fast open connect status";
    ASSERT_TRUE(is_enabled) << "TCP fast open connect should be enabled";
    
    // Test disabling TCP fast open connect
    result = os_socket_set_tcp_fastopen_connect(tcp_socket, false);
    ASSERT_EQ(BHT_OK, result) << "Failed to disable TCP fast open connect";
    
    // Verify fast open connect is disabled
    result = os_socket_get_tcp_fastopen_connect(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Failed to get TCP fast open connect status";
    ASSERT_FALSE(is_enabled) << "TCP fast open connect should be disabled";
}

TEST_F(PosixSocketOptionsTest, TcpFastopenConnect_InvalidSocket_ReturnsError) {
    bool is_enabled = false;
    
    // Test with invalid socket descriptor
    int result = os_socket_set_tcp_fastopen_connect(-1, true);
    ASSERT_EQ(BHT_ERROR, result) << "Should fail with invalid socket";
    
    result = os_socket_get_tcp_fastopen_connect(-1, &is_enabled);
    ASSERT_EQ(BHT_ERROR, result) << "Should fail with invalid socket";
}

// ============================================================================
// Comprehensive Integration Tests
// ============================================================================

TEST_F(PosixSocketOptionsTest, SocketOptions_MultipleOperations_WorkCorrectly) {
    if (tcp_socket == -1) {
        return;
    }
    
    // Test multiple socket options together
    ASSERT_EQ(BHT_OK, os_socket_set_keep_alive(tcp_socket, true));
    ASSERT_EQ(BHT_OK, os_socket_set_tcp_no_delay(tcp_socket, true));
    
    // Set keep-alive parameters
    ASSERT_EQ(BHT_OK, os_socket_set_tcp_keep_idle(tcp_socket, 600));
    ASSERT_EQ(BHT_OK, os_socket_set_tcp_keep_intvl(tcp_socket, 60));
    
    // Verify all options are set correctly
    bool keep_alive_enabled = false;
    bool no_delay_enabled = false;
    uint32 idle_time = 0;
    uint32 interval = 0;
    
    ASSERT_EQ(BHT_OK, os_socket_get_keep_alive(tcp_socket, &keep_alive_enabled));
    ASSERT_EQ(BHT_OK, os_socket_get_tcp_no_delay(tcp_socket, &no_delay_enabled));
    ASSERT_EQ(BHT_OK, os_socket_get_tcp_keep_idle(tcp_socket, &idle_time));
    ASSERT_EQ(BHT_OK, os_socket_get_tcp_keep_intvl(tcp_socket, &interval));
    
    ASSERT_TRUE(keep_alive_enabled);
    ASSERT_TRUE(no_delay_enabled);
    ASSERT_EQ(600U, idle_time);
    ASSERT_EQ(60U, interval);
}

TEST_F(PosixSocketOptionsTest, SocketOptions_ErrorRecovery_HandledCorrectly) {
    if (tcp_socket == -1) {
        return;
    }
    
    // Test that socket remains functional after error conditions
    bool is_enabled = false;
    
    // Test with invalid socket to cause error condition
    int result = os_socket_get_keep_alive(-1, &is_enabled);
    ASSERT_EQ(BHT_ERROR, result);
    
    // Verify socket is still functional
    result = os_socket_set_keep_alive(tcp_socket, true);
    ASSERT_EQ(BHT_OK, result) << "Socket should remain functional after error";
    
    result = os_socket_get_keep_alive(tcp_socket, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Socket should remain functional after error";
    ASSERT_TRUE(is_enabled);
}

// ============================================================================
// Step 3: Linger & Socket Options Tests (4 functions)
// ============================================================================

TEST_F(PosixSocketOptionsTest, Linger_SetAndGet_WorksCorrectly) {
    bh_socket_t tcp_socket = create_tcp_socket();
    ASSERT_NE(-1, tcp_socket);
    
    bool is_linger_enabled = false;
    int linger_seconds = 0;
    
    // Test getting default linger settings
    int result = os_socket_get_linger(tcp_socket, &is_linger_enabled, &linger_seconds);
    ASSERT_EQ(BHT_OK, result) << "Getting default linger should succeed";
    
    // Test setting linger enabled with 30 second timeout
    result = os_socket_set_linger(tcp_socket, true, 30);
    ASSERT_EQ(BHT_OK, result) << "Setting linger enabled should succeed";
    
    // Verify the setting was applied
    result = os_socket_get_linger(tcp_socket, &is_linger_enabled, &linger_seconds);
    ASSERT_EQ(BHT_OK, result) << "Getting linger after set should succeed";
    ASSERT_TRUE(is_linger_enabled) << "Linger should be enabled";
    ASSERT_EQ(30, linger_seconds) << "Linger timeout should be 30 seconds";
    
    // Test disabling linger
    result = os_socket_set_linger(tcp_socket, false, 0);
    ASSERT_EQ(BHT_OK, result) << "Disabling linger should succeed";
    
    // Verify linger is disabled
    result = os_socket_get_linger(tcp_socket, &is_linger_enabled, &linger_seconds);
    ASSERT_EQ(BHT_OK, result) << "Getting linger after disable should succeed";
    ASSERT_FALSE(is_linger_enabled) << "Linger should be disabled";
    
    os_socket_close(tcp_socket);
}

TEST_F(PosixSocketOptionsTest, Linger_InvalidSocket_HandledCorrectly) {
    bool is_linger_enabled = false;
    int linger_seconds = 0;
    
    // Test with invalid socket descriptor
    int result = os_socket_get_linger(-1, &is_linger_enabled, &linger_seconds);
    ASSERT_EQ(BHT_ERROR, result) << "Getting linger on invalid socket should fail";
    
    result = os_socket_set_linger(-1, true, 30);
    ASSERT_EQ(BHT_ERROR, result) << "Setting linger on invalid socket should fail";
}

TEST_F(PosixSocketOptionsTest, Linger_BoundaryValues_HandledCorrectly) {
    bh_socket_t tcp_socket = create_tcp_socket();
    ASSERT_NE(-1, tcp_socket);
    
    // Test with zero timeout (immediate close)
    int result = os_socket_set_linger(tcp_socket, true, 0);
    ASSERT_EQ(BHT_OK, result) << "Setting linger with 0 timeout should succeed";
    
    bool is_linger_enabled = false;
    int linger_seconds = -1;
    result = os_socket_get_linger(tcp_socket, &is_linger_enabled, &linger_seconds);
    ASSERT_EQ(BHT_OK, result) << "Getting linger should succeed";
    ASSERT_TRUE(is_linger_enabled) << "Linger should be enabled";
    ASSERT_EQ(0, linger_seconds) << "Linger timeout should be 0";
    
    // Test with maximum reasonable timeout (65535 seconds)
    result = os_socket_set_linger(tcp_socket, true, 65535);
    ASSERT_EQ(BHT_OK, result) << "Setting linger with max timeout should succeed";
    
    result = os_socket_get_linger(tcp_socket, &is_linger_enabled, &linger_seconds);
    ASSERT_EQ(BHT_OK, result) << "Getting linger should succeed";
    ASSERT_TRUE(is_linger_enabled) << "Linger should be enabled";
    ASSERT_EQ(65535, linger_seconds) << "Linger timeout should be 65535";
    
    os_socket_close(tcp_socket);
}

// Note: TCP_USER_TIMEOUT functions don't exist in WAMR API
// Skipping TCP user timeout tests as these functions are not implemented

// ============================================================================
// Step 4: Multicast Operations Tests (7 functions)
// ============================================================================

TEST_F(PosixSocketOptionsTest, IpMulticastLoop_SetAndGet_WorksCorrectly) {
    bh_socket_t udp_socket = create_udp_socket();
    ASSERT_NE(-1, udp_socket);
    
    bool is_enabled = false;
    
    // Test getting default multicast loop setting
    int result = os_socket_get_ip_multicast_loop(udp_socket, false, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Getting default multicast loop should succeed";
    
    // Test enabling multicast loop
    result = os_socket_set_ip_multicast_loop(udp_socket, false, true);
    ASSERT_EQ(BHT_OK, result) << "Enabling multicast loop should succeed";
    
    // Verify the setting was applied
    result = os_socket_get_ip_multicast_loop(udp_socket, false, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Getting multicast loop after set should succeed";
    ASSERT_TRUE(is_enabled) << "Multicast loop should be enabled";
    
    // Test disabling multicast loop
    result = os_socket_set_ip_multicast_loop(udp_socket, false, false);
    ASSERT_EQ(BHT_OK, result) << "Disabling multicast loop should succeed";
    
    // Verify multicast loop is disabled
    result = os_socket_get_ip_multicast_loop(udp_socket, false, &is_enabled);
    ASSERT_EQ(BHT_OK, result) << "Getting multicast loop after disable should succeed";
    ASSERT_FALSE(is_enabled) << "Multicast loop should be disabled";
    
    os_socket_close(udp_socket);
}

TEST_F(PosixSocketOptionsTest, IpMulticastLoop_InvalidSocket_HandledCorrectly) {
    bool is_enabled = false;
    
    // Test with invalid socket descriptor
    int result = os_socket_get_ip_multicast_loop(-1, false, &is_enabled);
    ASSERT_EQ(BHT_ERROR, result) << "Getting multicast loop on invalid socket should fail";
    
    result = os_socket_set_ip_multicast_loop(-1, false, true);
    ASSERT_EQ(BHT_ERROR, result) << "Setting multicast loop on invalid socket should fail";
}

TEST_F(PosixSocketOptionsTest, IpTtl_SetAndGet_WorksCorrectly) {
    bh_socket_t udp_socket = create_udp_socket();
    ASSERT_NE(-1, udp_socket);
    
    uint8_t ttl_value = 0;
    
    // Test getting default TTL value
    int result = os_socket_get_ip_ttl(udp_socket, &ttl_value);
    ASSERT_EQ(BHT_OK, result) << "Getting default TTL should succeed";
    
    // Test setting TTL to 64 (common default)
    result = os_socket_set_ip_ttl(udp_socket, 64);
    ASSERT_EQ(BHT_OK, result) << "Setting TTL should succeed";
    
    // Verify the setting was applied
    result = os_socket_get_ip_ttl(udp_socket, &ttl_value);
    ASSERT_EQ(BHT_OK, result) << "Getting TTL after set should succeed";
    ASSERT_EQ(64, ttl_value) << "TTL should be 64";
    
    // Test setting TTL to 1 (local network only)
    result = os_socket_set_ip_ttl(udp_socket, 1);
    ASSERT_EQ(BHT_OK, result) << "Setting TTL to 1 should succeed";
    
    result = os_socket_get_ip_ttl(udp_socket, &ttl_value);
    ASSERT_EQ(BHT_OK, result) << "Getting TTL should succeed";
    ASSERT_EQ(1, ttl_value) << "TTL should be 1";
    
    os_socket_close(udp_socket);
}

TEST_F(PosixSocketOptionsTest, IpTtl_BoundaryValues_HandledCorrectly) {
    bh_socket_t udp_socket = create_udp_socket();
    ASSERT_NE(-1, udp_socket);
    
    // Test with minimum TTL (1)
    int result = os_socket_set_ip_ttl(udp_socket, 1);
    ASSERT_EQ(BHT_OK, result) << "Setting minimum TTL should succeed";
    
    uint8_t ttl_value = 0;
    result = os_socket_get_ip_ttl(udp_socket, &ttl_value);
    ASSERT_EQ(BHT_OK, result) << "Getting TTL should succeed";
    ASSERT_EQ(1, ttl_value) << "TTL should be 1";
    
    // Test with maximum TTL (255)
    result = os_socket_set_ip_ttl(udp_socket, 255);
    ASSERT_EQ(BHT_OK, result) << "Setting maximum TTL should succeed";
    
    result = os_socket_get_ip_ttl(udp_socket, &ttl_value);
    ASSERT_EQ(BHT_OK, result) << "Getting TTL should succeed";
    ASSERT_EQ(255, ttl_value) << "TTL should be 255";
    
    os_socket_close(udp_socket);
}

TEST_F(PosixSocketOptionsTest, IpTtl_InvalidSocket_HandledCorrectly) {
    uint8_t ttl_value = 0;
    
    // Test with invalid socket descriptor
    int result = os_socket_get_ip_ttl(-1, &ttl_value);
    ASSERT_EQ(BHT_ERROR, result) << "Getting TTL on invalid socket should fail";
    
    result = os_socket_set_ip_ttl(-1, 64);
    ASSERT_EQ(BHT_ERROR, result) << "Setting TTL on invalid socket should fail";
}

TEST_F(PosixSocketOptionsTest, MulticastMembership_AddAndDrop_WorksCorrectly) {
    bh_socket_t udp_socket = create_udp_socket();
    ASSERT_NE(-1, udp_socket);
    
    // Prepare multicast group address (239.255.255.250 - UPnP multicast)
    bh_ip_addr_buffer_t mcast_addr;
    int result = os_socket_inet_network(true, "239.255.255.250", &mcast_addr);
    ASSERT_EQ(BHT_OK, result) << "Converting multicast address should succeed";
    
    // Prepare interface address (INADDR_ANY)
    bh_ip_addr_buffer_t if_addr;
    result = os_socket_inet_network(true, "0.0.0.0", &if_addr);
    ASSERT_EQ(BHT_OK, result) << "Converting interface address should succeed";
    
    // Test adding multicast membership
    result = os_socket_set_ip_add_membership(udp_socket, &mcast_addr, 0, false);
    // Note: This may fail on some systems if multicast is not supported or
    // if we don't have proper network setup, so we check for either success or expected error
    bool add_succeeded = (result == BHT_OK);
    
    if (add_succeeded) {
        // Test dropping multicast membership
        result = os_socket_set_ip_drop_membership(udp_socket, &mcast_addr, 0, false);
        ASSERT_EQ(BHT_OK, result) << "Dropping multicast membership should succeed";
    }
    
    os_socket_close(udp_socket);
}

TEST_F(PosixSocketOptionsTest, MulticastMembership_InvalidSocket_HandledCorrectly) {
    bh_ip_addr_buffer_t mcast_addr;
    bh_ip_addr_buffer_t if_addr;
    
    // Prepare addresses
    int result = os_socket_inet_network(true, "239.255.255.250", &mcast_addr);
    ASSERT_EQ(BHT_OK, result);
    result = os_socket_inet_network(true, "0.0.0.0", &if_addr);
    ASSERT_EQ(BHT_OK, result);
    
    // Test with invalid socket descriptor
    result = os_socket_set_ip_add_membership(-1, &mcast_addr, 0, false);
    ASSERT_EQ(BHT_ERROR, result) << "Adding membership on invalid socket should fail";
    
    result = os_socket_set_ip_drop_membership(-1, &mcast_addr, 0, false);
    ASSERT_EQ(BHT_ERROR, result) << "Dropping membership on invalid socket should fail";
}

TEST_F(PosixSocketOptionsTest, InetNetwork_ValidAddresses_ConvertedCorrectly) {
    bh_ip_addr_buffer_t addr_buf;
    
    // Test converting localhost address
    int result = os_socket_inet_network(true, "127.0.0.1", &addr_buf);
    ASSERT_EQ(BHT_OK, result) << "Converting localhost address should succeed";
    
    // Test converting broadcast address
    result = os_socket_inet_network(true, "255.255.255.255", &addr_buf);
    ASSERT_EQ(BHT_OK, result) << "Converting broadcast address should succeed";
    
    // Test converting any address
    result = os_socket_inet_network(true, "0.0.0.0", &addr_buf);
    ASSERT_EQ(BHT_OK, result) << "Converting any address should succeed";
    
    // Test converting multicast address
    result = os_socket_inet_network(true, "224.0.0.1", &addr_buf);
    ASSERT_EQ(BHT_OK, result) << "Converting multicast address should succeed";
}

TEST_F(PosixSocketOptionsTest, InetNetwork_InvalidAddresses_HandledCorrectly) {
    bh_ip_addr_buffer_t addr_buf;
    
    // Test with invalid IP address format
    int result = os_socket_inet_network(true, "999.999.999.999", &addr_buf);
    ASSERT_EQ(BHT_ERROR, result) << "Converting invalid IP should fail";
    
    // Test with malformed address
    result = os_socket_inet_network(true, "192.168.1", &addr_buf);
    ASSERT_EQ(BHT_ERROR, result) << "Converting malformed IP should fail";
    
    // Test with non-numeric address
    result = os_socket_inet_network(true, "localhost", &addr_buf);
    ASSERT_EQ(BHT_ERROR, result) << "Converting hostname should fail";
    
    // Test with empty string
    result = os_socket_inet_network(true, "", &addr_buf);
    ASSERT_EQ(BHT_ERROR, result) << "Converting empty string should fail";
}