/*
 * Copyright (C) 2024 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/**
 * POSIX Multicast TTL Coverage Test
 * 
 * This file implements targeted test cases for the 2 genuinely uncovered 
 * POSIX socket functions identified in LCOV analysis:
 * 
 * Target Functions (Actually Uncovered):
 * 1. os_socket_get_ip_multicast_ttl() - 0 hits
 * 2. os_socket_set_ip_multicast_ttl() - 0 hits
 * 
 * Expected Coverage Impact: ~20 lines (targeted improvement)
 */

#include "gtest/gtest.h"
#include "platform_api_extension.h"
#include "test_helper.h"
#include "wasm_export.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>

class POSIXMulticastTTLTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize WAMR runtime for proper context
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
        ASSERT_TRUE(wasm_runtime_full_init(&init_args));
        
        // Create UDP socket for multicast operations
        test_socket = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT_NE(test_socket, -1);
    }
    
    void TearDown() override
    {
        if (test_socket != -1) {
            close(test_socket);
        }
        wasm_runtime_destroy();
    }
    
    RuntimeInitArgs init_args;
    int test_socket = -1;
};

/**
 * Test os_socket_set_ip_multicast_ttl() with valid TTL values
 * This function has 0 hits in LCOV coverage report
 */
TEST_F(POSIXMulticastTTLTest, SetMulticastTTL_ValidValues_SucceedsCorrectly)
{
    // Test with default multicast TTL (1)
    int result = os_socket_set_ip_multicast_ttl(test_socket, 1);
    ASSERT_EQ(result, BHT_OK);
    
    // Test with maximum valid TTL (255)
    result = os_socket_set_ip_multicast_ttl(test_socket, 255);
    ASSERT_EQ(result, BHT_OK);
    
    // Test with common multicast TTL values
    result = os_socket_set_ip_multicast_ttl(test_socket, 32);
    ASSERT_EQ(result, BHT_OK);
    
    result = os_socket_set_ip_multicast_ttl(test_socket, 64);
    ASSERT_EQ(result, BHT_OK);
}

/**
 * Test os_socket_get_ip_multicast_ttl() after setting values
 * This function has 0 hits in LCOV coverage report
 */
TEST_F(POSIXMulticastTTLTest, GetMulticastTTL_AfterSetting_ReturnsCorrectValue)
{
    uint8_t expected_ttl = 42;
    uint8_t retrieved_ttl = 0;
    
    // Set a specific TTL value
    int result = os_socket_set_ip_multicast_ttl(test_socket, expected_ttl);
    ASSERT_EQ(result, BHT_OK);
    
    // Retrieve the TTL value
    result = os_socket_get_ip_multicast_ttl(test_socket, &retrieved_ttl);
    ASSERT_EQ(result, BHT_OK);
    ASSERT_EQ(retrieved_ttl, expected_ttl);
}

/**
 * Test os_socket_set_ip_multicast_ttl() with invalid socket
 */
TEST_F(POSIXMulticastTTLTest, SetMulticastTTL_InvalidSocket_FailsGracefully)
{
    int invalid_socket = -1;
    
    int result = os_socket_set_ip_multicast_ttl(invalid_socket, 1);
    ASSERT_NE(result, BHT_OK);
}

/**
 * Test os_socket_get_ip_multicast_ttl() with invalid socket
 */
TEST_F(POSIXMulticastTTLTest, GetMulticastTTL_InvalidSocket_FailsGracefully)
{
    bh_socket_t invalid_socket = -1;
    uint8_t ttl_value = 0;
    
    int result = os_socket_get_ip_multicast_ttl(invalid_socket, &ttl_value);
    ASSERT_NE(result, BHT_OK);
}

/**
 * Test os_socket_get_ip_multicast_ttl() with NULL buffer
 */
TEST_F(POSIXMulticastTTLTest, GetMulticastTTL_NullBuffer_FailsGracefully)
{
    int result = os_socket_get_ip_multicast_ttl(test_socket, nullptr);
    ASSERT_NE(result, BHT_OK);
}

/**
 * Test os_socket_set_ip_multicast_ttl() with boundary values
 */
TEST_F(POSIXMulticastTTLTest, SetMulticastTTL_BoundaryValues_HandlesCorrectly)
{
    // Test with minimum valid TTL (0)
    int result = os_socket_set_ip_multicast_ttl(test_socket, 0);
    ASSERT_EQ(result, BHT_OK);
    
    // Test with maximum valid TTL (255)
    result = os_socket_set_ip_multicast_ttl(test_socket, 255);
    ASSERT_EQ(result, BHT_OK);
}

/**
 * Test multicast TTL operations with different socket types
 * Only UDP sockets should support multicast TTL operations
 */
TEST_F(POSIXMulticastTTLTest, MulticastTTL_TCPSocket_HandlesAppropriately)
{
    // Create TCP socket
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(tcp_socket, -1);
    
    uint8_t ttl_value = 0;
    
    // TCP sockets may not support multicast TTL operations
    // The function should handle this gracefully
    int result = os_socket_get_ip_multicast_ttl(tcp_socket, &ttl_value);
    // Result may be success or failure depending on platform implementation
    // Just ensure it doesn't crash
    
    result = os_socket_set_ip_multicast_ttl(tcp_socket, 1);
    // Result may be success or failure depending on platform implementation
    // Just ensure it doesn't crash
    
    close(tcp_socket);
}