/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "../common/test_helper.h"
#include "gtest/gtest.h"

#include "platform_common.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"
#include "bh_platform.h"
#include "wasm_export.h"
#include "wasm_exec_env.h"
#include "wasm_suspend_flags.h"
#include <thread>
#include <chrono>

using namespace std;

// Enhanced test fixture for wasm_blocking_op.c functions - Lines 26-38
class EnhancedWasmBlockingOpTest : public testing::Test {
protected:
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_Pool;
        init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
        init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

        wasm_runtime_full_init(&init_args);

        module_inst = nullptr;
        exec_env = nullptr;
        test_exec_env = nullptr;

        // Initialize test data
        error_buf[0] = '\0';
        simple_wasm_size = 0;
        simple_wasm = nullptr;

        CreateSimpleWasmModule();
        CreateTestExecEnv();
    }

    void TearDown() override {
        if (test_exec_env) {
            // Clean up test exec env
            CleanupTestExecEnv();
        }
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (simple_wasm) {
            free(simple_wasm);
            simple_wasm = nullptr;
        }
        wasm_runtime_destroy();
    }

    // Create a simple WASM module for testing
    void CreateSimpleWasmModule() {
        // Minimal WASM module with a simple function
        uint8_t wasm_bytes[] = {
            0x00, 0x61, 0x73, 0x6d, // WASM magic
            0x01, 0x00, 0x00, 0x00, // version
            0x01, 0x07,             // type section
            0x01,                   // 1 type
            0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, // func type: (i32,i32)->i32
            0x03, 0x02,             // function section
            0x01, 0x00,             // 1 function, type 0
            0x0a, 0x09,             // code section
            0x01, 0x07,             // 1 function body
            0x00,                   // 0 locals
            0x20, 0x00,             // local.get 0
            0x20, 0x01,             // local.get 1
            0x6a,                   // i32.add
            0x0b                    // end
        };

        simple_wasm_size = sizeof(wasm_bytes);
        simple_wasm = (uint8_t*)malloc(simple_wasm_size);
        memcpy(simple_wasm, wasm_bytes, simple_wasm_size);
    }

    // Create a test exec_env with proper initialization
    void CreateTestExecEnv() {
        // Load and instantiate module
        wasm_module_t module = wasm_runtime_load(simple_wasm, simple_wasm_size, error_buf, sizeof(error_buf));
        if (!module) return;

        module_inst = wasm_runtime_instantiate(module, 8192, 0, error_buf, sizeof(error_buf));
        if (!module_inst) {
            wasm_runtime_unload(module);
            return;
        }

        exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
        if (!exec_env) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
            wasm_runtime_unload(module);
            return;
        }

        // Create manual test exec_env for flag manipulation
        test_exec_env = (WASMExecEnv*)malloc(sizeof(WASMExecEnv));
        memset(test_exec_env, 0, sizeof(WASMExecEnv));

        // Initialize wait_lock
        if (os_mutex_init(&test_exec_env->wait_lock) != 0) {
            free(test_exec_env);
            test_exec_env = nullptr;
            return;
        }

        // Initialize suspend flags to clean state
        BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, 0);

        // Initialize handle to current thread to avoid segfault in os_wakeup_blocking_op
        test_exec_env->handle = os_self_thread();

        wasm_runtime_unload(module);
    }

    void CleanupTestExecEnv() {
        if (test_exec_env) {
            os_mutex_destroy(&test_exec_env->wait_lock);
            free(test_exec_env);
            test_exec_env = nullptr;
        }
    }

public:
    char global_heap_buf[512 * 1024];
    RuntimeInitArgs init_args;
    WASMModuleInstanceCommon *module_inst;
    WASMExecEnv *exec_env;
    WASMExecEnv *test_exec_env;
    char error_buf[128];
    uint8_t *simple_wasm;
    uint32 simple_wasm_size;
};

/******
 * Test Case: wasm_runtime_begin_blocking_op_NormalSuccess_ReturnsTrue
 * Source: core/iwasm/common/wasm_blocking_op.c:26-38
 * Target Lines: 28 (LOCK), 29 (assert), 30 (SET BLOCKING), 36 (UNLOCK), 37 (os_begin_blocking_op), 38 (return true)
 * Functional Purpose: Tests normal successful execution path where no TERMINATE flag is set.
 *                     Verifies proper lock/unlock sequence and BLOCKING flag management.
 * Call Path: Direct API call to wasm_runtime_begin_blocking_op()
 * Coverage Goal: Exercise normal success path through all lock/unlock operations
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_begin_blocking_op_NormalSuccess_ReturnsTrue) {
    ASSERT_NE(nullptr, test_exec_env);

    // Start with clean suspend flags (no BLOCKING, no TERMINATE)
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, 0);

    // Call the function under test
    bool result = wasm_runtime_begin_blocking_op(test_exec_env);

    // Verify success
    ASSERT_TRUE(result);

    // Verify BLOCKING flag is now set
    uint32 flags = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags & WASM_SUSPEND_FLAG_BLOCKING);

    // Clean up for next tests
    wasm_runtime_end_blocking_op(test_exec_env);
}

/******
 * Test Case: wasm_runtime_begin_blocking_op_TerminateSet_ReturnsFalse
 * Source: core/iwasm/common/wasm_blocking_op.c:26-38
 * Target Lines: 28 (LOCK), 29 (assert), 30 (SET BLOCKING), 31 (if TERMINATE), 32 (CLR BLOCKING), 33 (UNLOCK), 34 (return false)
 * Functional Purpose: Tests early termination path when TERMINATE flag is already set.
 *                     Verifies proper cleanup of BLOCKING flag and early return.
 * Call Path: Direct API call to wasm_runtime_begin_blocking_op()
 * Coverage Goal: Exercise error/termination path with proper flag cleanup
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_begin_blocking_op_TerminateSet_ReturnsFalse) {
    ASSERT_NE(nullptr, test_exec_env);

    // Pre-set TERMINATE flag to trigger early return
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, WASM_SUSPEND_FLAG_TERMINATE);

    // Call the function under test
    bool result = wasm_runtime_begin_blocking_op(test_exec_env);

    // Verify failure return
    ASSERT_FALSE(result);

    // Verify BLOCKING flag is NOT set (should be cleared in error path)
    uint32 flags = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_EQ(0, flags & WASM_SUSPEND_FLAG_BLOCKING);

    // Verify TERMINATE flag is still set
    ASSERT_NE(0, flags & WASM_SUSPEND_FLAG_TERMINATE);
}

/******
 * Test Case: wasm_runtime_begin_blocking_op_ValidExecEnv_ProperlySetsFlags
 * Source: core/iwasm/common/wasm_blocking_op.c:26-38
 * Target Lines: 28 (LOCK), 29 (assert check), 30 (SET BLOCKING), 36 (UNLOCK), 37 (os_begin_blocking_op), 38 (return true)
 * Functional Purpose: Tests that the function properly manages suspend flags with proper
 *                     exec_env from actual WASM module instantiation.
 * Call Path: Direct API call to wasm_runtime_begin_blocking_op()
 * Coverage Goal: Verify flag management works with real exec_env structures
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_begin_blocking_op_ValidExecEnv_ProperlySetsFlags) {
    ASSERT_NE(nullptr, exec_env);

    // Ensure clean starting state
    BH_ATOMIC_32_STORE(exec_env->suspend_flags.flags, 0);

    // Call the function under test
    bool result = wasm_runtime_begin_blocking_op(exec_env);

    // Verify success
    ASSERT_TRUE(result);

    // Verify BLOCKING flag is set
    uint32 flags = WASM_SUSPEND_FLAGS_GET(exec_env->suspend_flags);
    ASSERT_NE(0, flags & WASM_SUSPEND_FLAG_BLOCKING);

    // Clean up properly
    wasm_runtime_end_blocking_op(exec_env);

    // Verify BLOCKING flag is cleared after end
    flags = WASM_SUSPEND_FLAGS_GET(exec_env->suspend_flags);
    ASSERT_EQ(0, flags & WASM_SUSPEND_FLAG_BLOCKING);
}

/******
 * Test Case: wasm_runtime_begin_blocking_op_MultipleFlags_HandlesCorrectly
 * Source: core/iwasm/common/wasm_blocking_op.c:26-38
 * Target Lines: 28 (LOCK), 29 (assert), 30 (SET BLOCKING), 31 (if TERMINATE check), 36 (UNLOCK), 37 (os_begin_blocking_op), 38 (return true)
 * Functional Purpose: Tests behavior when other suspend flags are set but not TERMINATE.
 *                     Verifies function continues normally with non-blocking flags.
 * Call Path: Direct API call to wasm_runtime_begin_blocking_op()
 * Coverage Goal: Test flag combination scenarios and proper flag isolation
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_begin_blocking_op_MultipleFlags_HandlesCorrectly) {
    ASSERT_NE(nullptr, test_exec_env);

    // Set some other suspend flags but NOT TERMINATE
    uint32 initial_flags = WASM_SUSPEND_FLAG_SUSPEND | WASM_SUSPEND_FLAG_BREAKPOINT;
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, initial_flags);

    // Call the function under test
    bool result = wasm_runtime_begin_blocking_op(test_exec_env);

    // Verify success (should not be affected by non-TERMINATE flags)
    ASSERT_TRUE(result);

    // Verify BLOCKING flag is now also set along with original flags
    uint32 flags = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags & WASM_SUSPEND_FLAG_BLOCKING);
    ASSERT_NE(0, flags & WASM_SUSPEND_FLAG_SUSPEND);
    ASSERT_NE(0, flags & WASM_SUSPEND_FLAG_BREAKPOINT);
    ASSERT_EQ(0, flags & WASM_SUSPEND_FLAG_TERMINATE);

    // Clean up
    wasm_runtime_end_blocking_op(test_exec_env);
}

// ===== NEW TEST CASES FOR LINES 42-51 (wasm_runtime_end_blocking_op function) =====

/******
 * Test Case: wasm_runtime_end_blocking_op_NormalExecution_ClearsBlockingFlag
 * Source: core/iwasm/common/wasm_blocking_op.c:42-51
 * Target Lines: 44 (save errno), 45 (LOCK), 46 (assert BLOCKING), 47 (CLR BLOCKING), 48 (UNLOCK), 49 (os_end_blocking_op), 50 (restore errno)
 * Functional Purpose: Tests normal execution path of wasm_runtime_end_blocking_op,
 *                     verifying proper errno preservation, lock sequence, and flag clearing.
 * Call Path: Direct API call to wasm_runtime_end_blocking_op()
 * Coverage Goal: Exercise all lines 42-51 in the normal execution flow
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_end_blocking_op_NormalExecution_ClearsBlockingFlag) {
    ASSERT_NE(nullptr, test_exec_env);

    // First set BLOCKING flag to satisfy the assert on line 46
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, WASM_SUSPEND_FLAG_BLOCKING);

    // Verify BLOCKING flag is set before the call
    uint32 flags_before = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_before & WASM_SUSPEND_FLAG_BLOCKING);

    // Set a specific errno value to test preservation
    errno = 42;

    // Call the function under test - this should execute all lines 42-51
    wasm_runtime_end_blocking_op(test_exec_env);

    // Verify BLOCKING flag is cleared after the call (line 47 effect)
    uint32 flags_after = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_EQ(0, flags_after & WASM_SUSPEND_FLAG_BLOCKING);

    // Verify errno was preserved (lines 44, 50 effect)
    ASSERT_EQ(42, errno);
}

/******
 * Test Case: wasm_runtime_end_blocking_op_WithOtherFlags_PreservesOtherFlags
 * Source: core/iwasm/common/wasm_blocking_op.c:42-51
 * Target Lines: 44 (save errno), 45 (LOCK), 46 (assert BLOCKING), 47 (CLR BLOCKING), 48 (UNLOCK), 49 (os_end_blocking_op), 50 (restore errno)
 * Functional Purpose: Tests that end_blocking_op only clears BLOCKING flag while preserving other flags.
 *                     Verifies selective flag manipulation behavior.
 * Call Path: Direct API call to wasm_runtime_end_blocking_op()
 * Coverage Goal: Exercise all lines 42-51 with complex flag state
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_end_blocking_op_WithOtherFlags_PreservesOtherFlags) {
    ASSERT_NE(nullptr, test_exec_env);

    // Set BLOCKING flag plus other flags to test selective clearing
    uint32 initial_flags = WASM_SUSPEND_FLAG_BLOCKING | WASM_SUSPEND_FLAG_SUSPEND | WASM_SUSPEND_FLAG_TERMINATE;
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, initial_flags);

    // Verify initial state
    uint32 flags_before = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_before & WASM_SUSPEND_FLAG_BLOCKING);
    ASSERT_NE(0, flags_before & WASM_SUSPEND_FLAG_SUSPEND);
    ASSERT_NE(0, flags_before & WASM_SUSPEND_FLAG_TERMINATE);

    // Set errno to verify preservation across lock operations
    errno = 123;

    // Call the function under test - executes all lines 42-51
    wasm_runtime_end_blocking_op(test_exec_env);

    // Verify only BLOCKING flag is cleared (line 47), others preserved
    uint32 flags_after = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_EQ(0, flags_after & WASM_SUSPEND_FLAG_BLOCKING);        // Should be cleared
    ASSERT_NE(0, flags_after & WASM_SUSPEND_FLAG_SUSPEND);        // Should remain set
    ASSERT_NE(0, flags_after & WASM_SUSPEND_FLAG_TERMINATE);      // Should remain set

    // Verify errno preservation (lines 44, 50)
    ASSERT_EQ(123, errno);
}

/******
 * Test Case: wasm_runtime_end_blocking_op_ValidExecEnv_CompletesSuccessfully
 * Source: core/iwasm/common/wasm_blocking_op.c:42-51
 * Target Lines: 44 (save errno), 45 (LOCK), 46 (assert BLOCKING), 47 (CLR BLOCKING), 48 (UNLOCK), 49 (os_end_blocking_op), 50 (restore errno)
 * Functional Purpose: Tests end_blocking_op with real exec_env from module instantiation,
 *                     ensuring lock/unlock sequence works with real mutex structures.
 * Call Path: Direct API call to wasm_runtime_end_blocking_op()
 * Coverage Goal: Exercise all lines 42-51 with production-like exec_env
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_end_blocking_op_ValidExecEnv_CompletesSuccessfully) {
    ASSERT_NE(nullptr, exec_env);

    // Set BLOCKING flag on real exec_env to satisfy assert
    BH_ATOMIC_32_STORE(exec_env->suspend_flags.flags, WASM_SUSPEND_FLAG_BLOCKING);

    // Verify BLOCKING flag is set
    uint32 flags_before = WASM_SUSPEND_FLAGS_GET(exec_env->suspend_flags);
    ASSERT_NE(0, flags_before & WASM_SUSPEND_FLAG_BLOCKING);

    // Test errno preservation with different value
    errno = 999;

    // Call the function under test - executes all lines 42-51
    wasm_runtime_end_blocking_op(exec_env);

    // Verify BLOCKING flag is cleared
    uint32 flags_after = WASM_SUSPEND_FLAGS_GET(exec_env->suspend_flags);
    ASSERT_EQ(0, flags_after & WASM_SUSPEND_FLAG_BLOCKING);

    // Verify errno preservation
    ASSERT_EQ(999, errno);
}

/******
 * Test Case: wasm_runtime_end_blocking_op_ErrnoModificationDuringCall_RestoresOriginal
 * Source: core/iwasm/common/wasm_blocking_op.c:42-51
 * Target Lines: 44 (save errno), 45-49 (operations that might modify errno), 50 (restore errno)
 * Functional Purpose: Tests that errno is properly saved at the beginning and restored at the end,
 *                     even if intermediate operations modify errno.
 * Call Path: Direct API call to wasm_runtime_end_blocking_op()
 * Coverage Goal: Exercise errno save/restore behavior (lines 44, 50) specifically
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_end_blocking_op_ErrnoModificationDuringCall_RestoresOriginal) {
    ASSERT_NE(nullptr, test_exec_env);

    // Set BLOCKING flag for proper function execution
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, WASM_SUSPEND_FLAG_BLOCKING);

    // Set a specific errno value that should be preserved
    errno = 555;

    // Call the function under test - should save errno at line 44 and restore at line 50
    wasm_runtime_end_blocking_op(test_exec_env);

    // Verify BLOCKING flag cleared (confirms function executed)
    uint32 flags_after = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_EQ(0, flags_after & WASM_SUSPEND_FLAG_BLOCKING);

    // Most important: verify errno was restored to original value
    ASSERT_EQ(555, errno);
}

/******
 * Test Case: wasm_runtime_end_blocking_op_CompleteCycle_BeginEndSequence
 * Source: core/iwasm/common/wasm_blocking_op.c:42-51
 * Target Lines: 44 (save errno), 45 (LOCK), 46 (assert BLOCKING), 47 (CLR BLOCKING), 48 (UNLOCK), 49 (os_end_blocking_op), 50 (restore errno)
 * Functional Purpose: Tests complete begin/end blocking operation cycle to ensure proper
 *                     flag state transitions and that end properly cleans up begin state.
 * Call Path: wasm_runtime_begin_blocking_op() followed by wasm_runtime_end_blocking_op()
 * Coverage Goal: Exercise all lines 42-51 in realistic usage pattern
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_end_blocking_op_CompleteCycle_BeginEndSequence) {
    ASSERT_NE(nullptr, test_exec_env);

    // Start with clean state
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, 0);

    // Set errno to verify it survives the complete cycle
    errno = 777;

    // First call begin_blocking_op to properly set BLOCKING flag
    bool begin_result = wasm_runtime_begin_blocking_op(test_exec_env);
    ASSERT_TRUE(begin_result);

    // Verify BLOCKING flag is set after begin
    uint32 flags_middle = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_middle & WASM_SUSPEND_FLAG_BLOCKING);

    // Now call end_blocking_op - this executes all target lines 42-51
    wasm_runtime_end_blocking_op(test_exec_env);

    // Verify complete cleanup: BLOCKING flag should be cleared
    uint32 flags_final = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_EQ(0, flags_final & WASM_SUSPEND_FLAG_BLOCKING);

    // Verify errno preservation through the complete cycle
    ASSERT_EQ(777, errno);
}

// ===== NEW TEST CASES FOR LINES 71-74 (wasm_runtime_interrupt_blocking_op function) =====

/******
 * Test Case: wasm_runtime_interrupt_blocking_op_BlockingFlagSet_ExecutesWhileLoop
 * Source: core/iwasm/common/wasm_blocking_op.c:71-74
 * Target Lines: 71 (os_wakeup_blocking_op call), 74 (os_usleep call)
 * Functional Purpose: Tests that wasm_runtime_interrupt_blocking_op executes the while loop
 *                     when BLOCKING flag is set. Uses threading to avoid infinite loop.
 * Call Path: Direct API call to wasm_runtime_interrupt_blocking_op()
 * Coverage Goal: Exercise lines 71-74 in the while(ISSET(env, BLOCKING)) loop
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_interrupt_blocking_op_BlockingFlagSet_ExecutesWhileLoop) {
    ASSERT_NE(nullptr, test_exec_env);

    // Set BLOCKING flag to trigger while loop execution
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, WASM_SUSPEND_FLAG_BLOCKING);

    // Verify BLOCKING flag is set
    uint32 flags_before = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_before & WASM_SUSPEND_FLAG_BLOCKING);

    // Create a background thread to clear BLOCKING flag after short delay
    // This simulates the real-world scenario where another thread calls wasm_runtime_end_blocking_op
    std::thread cleanup_thread([this]() {
        // Small delay to allow interrupt_blocking_op to enter the while loop
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Clear BLOCKING flag to exit the while loop (simulates wasm_runtime_end_blocking_op)
        BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, WASM_SUSPEND_FLAG_TERMINATE);
    });

    // Call the function under test - should execute lines 67-77 including target lines 71, 74
    wasm_runtime_interrupt_blocking_op(test_exec_env);

    // Wait for cleanup thread to complete
    cleanup_thread.join();

    // Verify TERMINATE flag is set (line 68 effect)
    uint32 flags_after = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_after & WASM_SUSPEND_FLAG_TERMINATE);

    // Verify BLOCKING flag was cleared by cleanup thread
    ASSERT_EQ(0, flags_after & WASM_SUSPEND_FLAG_BLOCKING);
}

/******
 * Test Case: wasm_runtime_interrupt_blocking_op_NoBlockingFlag_SkipsWhileLoop
 * Source: core/iwasm/common/wasm_blocking_op.c:67-77
 * Target Lines: 67 (LOCK), 68 (SET TERMINATE), 69 (while condition check), 77 (UNLOCK)
 * Functional Purpose: Tests that when BLOCKING flag is not set, the while loop is skipped,
 *                     avoiding lines 71-74 but executing setup and cleanup code.
 * Call Path: Direct API call to wasm_runtime_interrupt_blocking_op()
 * Coverage Goal: Exercise function flow that bypasses while loop (lines 71-74 not executed)
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_interrupt_blocking_op_NoBlockingFlag_SkipsWhileLoop) {
    ASSERT_NE(nullptr, test_exec_env);

    // Start with clean flags - no BLOCKING, no TERMINATE
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, 0);

    // Verify initial clean state
    uint32 flags_before = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_EQ(0, flags_before & WASM_SUSPEND_FLAG_TERMINATE);
    ASSERT_EQ(0, flags_before & WASM_SUSPEND_FLAG_BLOCKING);

    // Call the function under test - should execute lines 67-69, 77 but skip 70-76
    wasm_runtime_interrupt_blocking_op(test_exec_env);

    // Verify TERMINATE flag is set (line 68 effect)
    uint32 flags_after = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_after & WASM_SUSPEND_FLAG_TERMINATE);

    // Verify BLOCKING flag remains clear (while loop was skipped)
    ASSERT_EQ(0, flags_after & WASM_SUSPEND_FLAG_BLOCKING);
}

/******
 * Test Case: wasm_runtime_interrupt_blocking_op_ValidExecEnv_SetsTerminateFlag
 * Source: core/iwasm/common/wasm_blocking_op.c:67-77
 * Target Lines: 67 (LOCK), 68 (SET TERMINATE), 77 (UNLOCK)
 * Functional Purpose: Tests that the function properly sets TERMINATE flag and manages
 *                     lock/unlock sequence with real exec_env from module instantiation.
 * Call Path: Direct API call to wasm_runtime_interrupt_blocking_op()
 * Coverage Goal: Exercise function with production-like exec_env structure
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_interrupt_blocking_op_ValidExecEnv_SetsTerminateFlag) {
    ASSERT_NE(nullptr, exec_env);

    // Start with clean state on real exec_env
    BH_ATOMIC_32_STORE(exec_env->suspend_flags.flags, 0);

    // Verify clean initial state
    uint32 flags_before = WASM_SUSPEND_FLAGS_GET(exec_env->suspend_flags);
    ASSERT_EQ(0, flags_before & WASM_SUSPEND_FLAG_TERMINATE);
    ASSERT_EQ(0, flags_before & WASM_SUSPEND_FLAG_BLOCKING);

    // Call the function under test with real exec_env
    wasm_runtime_interrupt_blocking_op(exec_env);

    // Verify TERMINATE flag is properly set (line 68 effect)
    uint32 flags_after = WASM_SUSPEND_FLAGS_GET(exec_env->suspend_flags);
    ASSERT_NE(0, flags_after & WASM_SUSPEND_FLAG_TERMINATE);

    // Clean up for other tests
    BH_ATOMIC_32_STORE(exec_env->suspend_flags.flags, 0);
}

/******
 * Test Case: wasm_runtime_interrupt_blocking_op_RepeatedCalls_HandlesCorrectly
 * Source: core/iwasm/common/wasm_blocking_op.c:67-77
 * Target Lines: 67 (LOCK), 68 (SET TERMINATE), 69 (while check), 77 (UNLOCK)
 * Functional Purpose: Tests that repeated calls to interrupt_blocking_op work correctly,
 *                     ensuring TERMINATE flag handling and lock sequence are robust.
 * Call Path: Multiple direct API calls to wasm_runtime_interrupt_blocking_op()
 * Coverage Goal: Exercise function robustness with multiple invocations
 ******/
TEST_F(EnhancedWasmBlockingOpTest, wasm_runtime_interrupt_blocking_op_RepeatedCalls_HandlesCorrectly) {
    ASSERT_NE(nullptr, test_exec_env);

    // Start with clean state
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, 0);

    // First call - should set TERMINATE flag
    wasm_runtime_interrupt_blocking_op(test_exec_env);

    uint32 flags_after_first = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_after_first & WASM_SUSPEND_FLAG_TERMINATE);

    // Second call - TERMINATE already set, should handle gracefully
    wasm_runtime_interrupt_blocking_op(test_exec_env);

    uint32 flags_after_second = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_after_second & WASM_SUSPEND_FLAG_TERMINATE);

    // Third call with BLOCKING flag also set - use threading approach to avoid infinite loop
    BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags,
                       WASM_SUSPEND_FLAG_TERMINATE | WASM_SUSPEND_FLAG_BLOCKING);

    // Create cleanup thread for the third call as well
    std::thread third_cleanup_thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // Clear BLOCKING flag while keeping TERMINATE
        BH_ATOMIC_32_STORE(test_exec_env->suspend_flags.flags, WASM_SUSPEND_FLAG_TERMINATE);
    });

    wasm_runtime_interrupt_blocking_op(test_exec_env);
    third_cleanup_thread.join();

    uint32 flags_after_third = WASM_SUSPEND_FLAGS_GET(test_exec_env->suspend_flags);
    ASSERT_NE(0, flags_after_third & WASM_SUSPEND_FLAG_TERMINATE);

    // Verify function execution completed successfully
    ASSERT_TRUE(true); // Function completed without crash or assertion failure
}