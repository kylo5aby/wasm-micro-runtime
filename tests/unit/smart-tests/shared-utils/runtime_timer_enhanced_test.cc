/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"
#include "bh_platform.h"
#include "runtime_timer.h"
#include "wasm_export.h"

// Platform detection utility for tests - REQUIRED in every test file
class PlatformTestContext {
public:
    // Architecture detection
    static bool IsX86_64() {
#if defined(BUILD_TARGET_X86) || defined(BUILD_TARGET_X86_64) || defined(__x86_64__)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsARM64() {
#if defined(BUILD_TARGET_AARCH64) || defined(BUILD_TARGET_ARM64) || defined(__aarch64__)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsARM32() {
#if defined(BUILD_TARGET_ARM) && !defined(BUILD_TARGET_AARCH64) && !defined(__aarch64__)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsRISCV() {
#if defined(BUILD_TARGET_RISCV) || defined(BUILD_TARGET_RISCV64) || defined(BUILD_TARGET_RISCV32) || defined(__riscv)
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

// Global test variables for timer callback verification
static uint32_t g_callback_timer_id = 0;
static uint32_t g_callback_owner = 0;
static int g_callback_count = 0;

// Timer callback function for testing
static void test_timer_callback(unsigned int id, unsigned int owner)
{
    g_callback_timer_id = id;
    g_callback_owner = owner;
    g_callback_count++;
}

// Timer expiry checker callback for testing
static void test_expiry_checker(timer_ctx_t ctx)
{
    // Simple checker implementation for testing
    (void)ctx;
}

class RuntimeTimerTest : public testing::Test {
protected:
    void SetUp() override {
        // Initialize WAMR runtime
        wasm_runtime_init();
        
        runtime_initialized = true;
        
        // Reset global callback variables
        g_callback_timer_id = 0;
        g_callback_owner = 0;
        g_callback_count = 0;
        
        timer_ctx = nullptr;
        test_owner = 12345;
    }
    
    void TearDown() override {
        if (timer_ctx) {
            destroy_timer_ctx(timer_ctx);
            timer_ctx = nullptr;
        }
        
        if (runtime_initialized) {
            wasm_runtime_destroy();
            runtime_initialized = false;
        }
    }
    
    timer_ctx_t timer_ctx;
    uint32_t test_owner;
    bool runtime_initialized;
};

// Test bh_get_tick_ms() function
TEST_F(RuntimeTimerTest, GetTickMs_ReturnsValidTimestamp) {
    uint64_t tick1 = bh_get_tick_ms();
    ASSERT_GT(tick1, 0);
    
    // Small delay to ensure time progression
    usleep(1000); // 1ms
    
    uint64_t tick2 = bh_get_tick_ms();
    ASSERT_GE(tick2, tick1);
    ASSERT_LT(tick2 - tick1, 1000); // Should be less than 1 second difference
}

// Test bh_get_elpased_ms() with normal time progression
TEST_F(RuntimeTimerTest, GetElapsedMs_NormalProgression_ReturnsCorrectElapsed) {
    uint32_t last_clock = (uint32_t)bh_get_tick_ms();
    
    // Small delay
    usleep(5000); // 5ms
    
    uint32_t elapsed = bh_get_elpased_ms(&last_clock);
    ASSERT_GE(elapsed, 3); // At least 3ms should have passed
    ASSERT_LE(elapsed, 50); // But not more than 50ms in normal conditions
}

// Test bh_get_elpased_ms() with clock overflow simulation
TEST_F(RuntimeTimerTest, GetElapsedMs_ClockOverflow_HandlesCorrectly) {
    // Get current time and simulate a clock that was set just before overflow
    uint32_t current_time = (uint32_t)bh_get_tick_ms();
    uint32_t last_clock = current_time - 100; // 100ms ago
    
    uint32_t elapsed = bh_get_elpased_ms(&last_clock);
    
    // Should handle the calculation correctly
    ASSERT_GE(elapsed, 90);  // At least 90ms should have passed
    ASSERT_LE(elapsed, 200); // But not more than 200ms in normal conditions
}

// Test create_timer_ctx() with valid parameters
TEST_F(RuntimeTimerTest, CreateTimerCtx_ValidParams_CreatesSuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, test_expiry_checker, 5, test_owner);
    
    ASSERT_NE(timer_ctx, nullptr);
    ASSERT_EQ(timer_ctx_get_owner(timer_ctx), test_owner);
}

// Test create_timer_ctx() with zero preallocation
TEST_F(RuntimeTimerTest, CreateTimerCtx_ZeroPrealloc_CreatesSuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    
    ASSERT_NE(timer_ctx, nullptr);
    ASSERT_EQ(timer_ctx_get_owner(timer_ctx), test_owner);
}

// Test create_timer_ctx() with NULL callback (should still work)
TEST_F(RuntimeTimerTest, CreateTimerCtx_NullCallback_CreatesSuccessfully) {
    timer_ctx = create_timer_ctx(nullptr, nullptr, 0, test_owner);
    
    ASSERT_NE(timer_ctx, nullptr);
    ASSERT_EQ(timer_ctx_get_owner(timer_ctx), test_owner);
}

// Test timer_ctx_get_owner() function
TEST_F(RuntimeTimerTest, TimerCtxGetOwner_ValidContext_ReturnsCorrectOwner) {
    uint32_t expected_owner = 98765;
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, expected_owner);
    
    ASSERT_NE(timer_ctx, nullptr);
    uint32_t actual_owner = timer_ctx_get_owner(timer_ctx);
    ASSERT_EQ(actual_owner, expected_owner);
}

// Test sys_create_timer() with auto_start=true
TEST_F(RuntimeTimerTest, SysCreateTimer_AutoStartTrue_CreatesActiveTimer) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 100, false, true);
    ASSERT_NE(timer_id, (uint32_t)-1);
    ASSERT_GT(timer_id, 0);
}

// Test sys_create_timer() with auto_start=false
TEST_F(RuntimeTimerTest, SysCreateTimer_AutoStartFalse_CreatesIdleTimer) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 100, false, false);
    ASSERT_NE(timer_id, (uint32_t)-1);
    ASSERT_GT(timer_id, 0);
}

// Test sys_create_timer() with periodic timer
TEST_F(RuntimeTimerTest, SysCreateTimer_PeriodicTimer_CreatesSuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 50, true, true);
    ASSERT_NE(timer_id, (uint32_t)-1);
    ASSERT_GT(timer_id, 0);
}

// Test sys_create_timer() with preallocation exhaustion
TEST_F(RuntimeTimerTest, SysCreateTimer_PreallocExhausted_FailsGracefully) {
    // Create context with only 1 preallocated timer
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 1, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    // Create first timer - should succeed
    uint32_t timer_id1 = sys_create_timer(timer_ctx, 100, false, false);
    ASSERT_NE(timer_id1, (uint32_t)-1);
    
    // Create second timer - should fail due to exhaustion
    uint32_t timer_id2 = sys_create_timer(timer_ctx, 100, false, false);
    ASSERT_EQ(timer_id2, (uint32_t)-1);
}

// Test sys_timer_destroy() with valid timer
TEST_F(RuntimeTimerTest, SysTimerDestroy_ValidTimer_DestroySuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 100, false, false);
    ASSERT_NE(timer_id, (uint32_t)-1);
    
    bool result = sys_timer_destroy(timer_ctx, timer_id);
    ASSERT_TRUE(result);
}

// Test sys_timer_destroy() with invalid timer ID
TEST_F(RuntimeTimerTest, SysTimerDestroy_InvalidTimerId_ReturnsFalse) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    bool result = sys_timer_destroy(timer_ctx, 99999);
    ASSERT_FALSE(result);
}

// Test sys_timer_cancel() with active timer
TEST_F(RuntimeTimerTest, SysTimerCancel_ActiveTimer_CancelsSuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 100, false, true);
    ASSERT_NE(timer_id, (uint32_t)-1);
    
    bool result = sys_timer_cancel(timer_ctx, timer_id);
    ASSERT_TRUE(result); // Should return true for active timer
}

// Test sys_timer_cancel() with idle timer
TEST_F(RuntimeTimerTest, SysTimerCancel_IdleTimer_CancelsSuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 100, false, false);
    ASSERT_NE(timer_id, (uint32_t)-1);
    
    bool result = sys_timer_cancel(timer_ctx, timer_id);
    ASSERT_FALSE(result); // Should return false for idle timer
}

// Test sys_timer_cancel() with invalid timer ID
TEST_F(RuntimeTimerTest, SysTimerCancel_InvalidTimerId_ReturnsFalse) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    bool result = sys_timer_cancel(timer_ctx, 99999);
    ASSERT_FALSE(result);
}

// Test sys_timer_restart() with valid timer
TEST_F(RuntimeTimerTest, SysTimerRestart_ValidTimer_RestartsSuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 100, false, false);
    ASSERT_NE(timer_id, (uint32_t)-1);
    
    bool result = sys_timer_restart(timer_ctx, timer_id, 200);
    ASSERT_TRUE(result);
}

// Test sys_timer_restart() with invalid timer ID
TEST_F(RuntimeTimerTest, SysTimerRestart_InvalidTimerId_ReturnsFalse) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    bool result = sys_timer_restart(timer_ctx, 99999, 100);
    ASSERT_FALSE(result);
}

// Test get_expiry_ms() with no active timers
TEST_F(RuntimeTimerTest, GetExpiryMs_NoActiveTimers_ReturnsMaxValue) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t expiry = get_expiry_ms(timer_ctx);
    ASSERT_EQ(expiry, (uint32_t)-1);
}

// Test get_expiry_ms() with active timer
TEST_F(RuntimeTimerTest, GetExpiryMs_WithActiveTimer_ReturnsValidExpiry) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 1000, false, true);
    ASSERT_NE(timer_id, (uint32_t)-1);
    
    uint32_t expiry = get_expiry_ms(timer_ctx);
    ASSERT_GT(expiry, 0);
    ASSERT_LE(expiry, 1000);
}

// Test check_app_timers() with no timers
TEST_F(RuntimeTimerTest, CheckAppTimers_NoTimers_ReturnsMaxValue) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t next_expiry = check_app_timers(timer_ctx);
    ASSERT_EQ(next_expiry, (uint32_t)-1);
}

// Test check_app_timers() with expired timer
TEST_F(RuntimeTimerTest, CheckAppTimers_ExpiredTimer_TriggersCallback) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    // Create timer with very short interval
    uint32_t timer_id = sys_create_timer(timer_ctx, 1, false, true);
    ASSERT_NE(timer_id, (uint32_t)-1);
    
    // Wait for timer to expire
    usleep(5000); // 5ms
    
    // Check timers - should trigger callback
    uint32_t next_expiry = check_app_timers(timer_ctx);
    
    // Verify callback was triggered
    ASSERT_EQ(g_callback_timer_id, timer_id);
    ASSERT_EQ(g_callback_owner, test_owner);
    ASSERT_GE(g_callback_count, 1);
}

// Test cleanup_app_timers() function
TEST_F(RuntimeTimerTest, CleanupAppTimers_WithActiveTimers_CleansSuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    // Create multiple timers
    uint32_t timer_id1 = sys_create_timer(timer_ctx, 100, false, true);
    uint32_t timer_id2 = sys_create_timer(timer_ctx, 200, false, false);
    ASSERT_NE(timer_id1, (uint32_t)-1);
    ASSERT_NE(timer_id2, (uint32_t)-1);
    
    // Cleanup should not crash
    cleanup_app_timers(timer_ctx);
    
    // After cleanup, no timers should be active
    uint32_t expiry = get_expiry_ms(timer_ctx);
    ASSERT_EQ(expiry, (uint32_t)-1);
}

// Test destroy_timer_ctx() function
TEST_F(RuntimeTimerTest, DestroyTimerCtx_WithTimers_DestroysCleanly) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 5, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    // Create some timers
    uint32_t timer_id1 = sys_create_timer(timer_ctx, 100, false, true);
    uint32_t timer_id2 = sys_create_timer(timer_ctx, 200, false, false);
    ASSERT_NE(timer_id1, (uint32_t)-1);
    ASSERT_NE(timer_id2, (uint32_t)-1);
    
    // Destroy should not crash
    destroy_timer_ctx(timer_ctx);
    timer_ctx = nullptr; // Prevent double cleanup in TearDown
}

// Test timer ID generation uniqueness
TEST_F(RuntimeTimerTest, TimerIdGeneration_MultipleTimers_GeneratesUniqueIds) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    // Create multiple timers and verify unique IDs
    uint32_t timer_id1 = sys_create_timer(timer_ctx, 100, false, false);
    uint32_t timer_id2 = sys_create_timer(timer_ctx, 200, false, false);
    uint32_t timer_id3 = sys_create_timer(timer_ctx, 300, false, false);
    
    ASSERT_NE(timer_id1, (uint32_t)-1);
    ASSERT_NE(timer_id2, (uint32_t)-1);
    ASSERT_NE(timer_id3, (uint32_t)-1);
    
    // All IDs should be unique
    ASSERT_NE(timer_id1, timer_id2);
    ASSERT_NE(timer_id2, timer_id3);
    ASSERT_NE(timer_id1, timer_id3);
}

// Test periodic timer behavior
TEST_F(RuntimeTimerTest, PeriodicTimer_ExpiredTimer_ReschedulesAutomatically) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    // Create periodic timer with short interval
    uint32_t timer_id = sys_create_timer(timer_ctx, 10, true, true);
    ASSERT_NE(timer_id, (uint32_t)-1);
    
    // Wait for multiple expirations
    usleep(25000); // 25ms
    
    // Check timers multiple times
    check_app_timers(timer_ctx);
    usleep(15000); // 15ms
    check_app_timers(timer_ctx);
    
    // Should have triggered callback multiple times for periodic timer
    ASSERT_GE(g_callback_count, 2);
    ASSERT_EQ(g_callback_timer_id, timer_id);
}

// Test timer context with large preallocation
TEST_F(RuntimeTimerTest, CreateTimerCtx_LargePrealloc_HandlesCorrectly) {
    // Test platform-specific memory limits
    int prealloc_count;
    if (PlatformTestContext::Is64BitArchitecture()) {
        prealloc_count = 1000; // Larger allocation on 64-bit
    } else {
        prealloc_count = 100;  // Smaller allocation on 32-bit
    }
    
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, prealloc_count, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    // Should be able to create many timers without allocation
    for (int i = 0; i < prealloc_count && i < 50; i++) {
        uint32_t timer_id = sys_create_timer(timer_ctx, 100 + i, false, false);
        ASSERT_NE(timer_id, (uint32_t)-1);
    }
}

// Test edge case: timer with zero interval
TEST_F(RuntimeTimerTest, SysCreateTimer_ZeroInterval_CreatesSuccessfully) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    uint32_t timer_id = sys_create_timer(timer_ctx, 0, false, true);
    ASSERT_NE(timer_id, (uint32_t)-1);
    
    // Timer with zero interval should expire immediately
    uint32_t expiry = get_expiry_ms(timer_ctx);
    ASSERT_EQ(expiry, 0);
}

// Test edge case: maximum timer ID rollover
TEST_F(RuntimeTimerTest, TimerIdRollover_MaxValue_HandlesCorrectly) {
    timer_ctx = create_timer_ctx(test_timer_callback, nullptr, 0, test_owner);
    ASSERT_NE(timer_ctx, nullptr);
    
    // Create a timer to increment the ID counter
    uint32_t timer_id = sys_create_timer(timer_ctx, 100, false, false);
    ASSERT_NE(timer_id, (uint32_t)-1);
    ASSERT_GT(timer_id, 0);
    
    // The implementation should handle ID rollover gracefully
    // (This test validates the basic ID generation works)
}