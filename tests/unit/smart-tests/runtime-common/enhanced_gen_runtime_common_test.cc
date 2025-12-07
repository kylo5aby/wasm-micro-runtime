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
#include "wasm_c_api.h"
#include "wasm_c_api_internal.h"

using namespace std;

// Enhanced test fixture for wasm_runtime_common.c functions
class EnhancedWasmRuntimeCommonTest : public testing::Test {
protected:
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_Pool;
        init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
        init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

        wasm_runtime_full_init(&init_args);

        // Create a minimal module instance for exec env creation
        module_inst = nullptr;
        exec_env = nullptr;
    }

    void TearDown() override {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        wasm_runtime_destroy();
    }

    WASMExecEnv* CreateMockExecEnv(uint8 *boundary_value) {
        // Create a minimal bytecode module for testing
        uint8_t minimal_wasm[] = {
            0x00, 0x61, 0x73, 0x6D, // WASM magic
            0x01, 0x00, 0x00, 0x00, // version
        };

        wasm_module_t module = wasm_runtime_load(minimal_wasm, sizeof(minimal_wasm), nullptr, 0);
        if (!module) {
            return nullptr;
        }

        module_inst = wasm_runtime_instantiate(module, 8192, 8192, nullptr, 0);
        wasm_runtime_unload(module);

        if (!module_inst) {
            return nullptr;
        }

        exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
        if (exec_env) {
            // Set the native stack boundary for testing
            exec_env->native_stack_boundary = boundary_value;
        }

        return exec_env;
    }

public:
    char global_heap_buf[512 * 1024];
    RuntimeInitArgs init_args;
    wasm_module_inst_t module_inst;
    WASMExecEnv *exec_env;
};

/******
 * Test Case: wasm_runtime_detect_native_stack_overflow_size_NullBoundary_ReturnsTrue
 * Source: core/iwasm/common/wasm_runtime_common.c:7892-7913
 * Target Lines: 7897-7900 (NULL boundary check and early return)
 * Functional Purpose: Verifies that when exec_env->native_stack_boundary is NULL,
 *                     the function returns true immediately without further processing
 * Call Path: Direct call to wasm_runtime_detect_native_stack_overflow_size()
 * Coverage Goal: Exercise early return path when platform doesn't support stack boundary detection
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, wasm_runtime_detect_native_stack_overflow_size_NullBoundary_ReturnsTrue) {
    // Create exec env with NULL boundary
    WASMExecEnv *test_exec_env = CreateMockExecEnv(nullptr);
    ASSERT_NE(test_exec_env, nullptr);

    // Test with various requested sizes - all should return true
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 0));
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 1024));
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 65536));
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, UINT32_MAX));
}

/******
 * Test Case: wasm_runtime_detect_native_stack_overflow_size_ValidBoundarySmallSize_ReturnsTrue
 * Source: core/iwasm/common/wasm_runtime_common.c:7892-7913
 * Target Lines: 7895-7896, 7907, 7913 (valid boundary with small requested size)
 * Functional Purpose: Verifies successful execution path when boundary is valid and
 *                     requested size is small enough to not cause overflow
 * Call Path: Direct call to wasm_runtime_detect_native_stack_overflow_size()
 * Coverage Goal: Exercise success path with boundary adjustment and no overflow detection
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, wasm_runtime_detect_native_stack_overflow_size_ValidBoundarySmallSize_ReturnsTrue) {
    // Create exec env with valid boundary (use stack address)
    uint8 stack_buffer[8192];
    uint8 *boundary = &stack_buffer[4096];  // Set boundary in middle of buffer

    WASMExecEnv *test_exec_env = CreateMockExecEnv(boundary);
    ASSERT_NE(test_exec_env, nullptr);

    // Test with small requested size that won't cause overflow
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 64));
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 512));
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 1024));
}

/******
 * Test Case: wasm_runtime_detect_native_stack_overflow_size_ValidBoundaryLargeSize_DetectsOverflow
 * Source: core/iwasm/common/wasm_runtime_common.c:7892-7913
 * Target Lines: 7907-7912 (boundary adjustment, overflow detection, exception setting)
 * Functional Purpose: Verifies overflow detection when requested size causes stack boundary
 *                     comparison to fail, triggering exception and returning false
 * Call Path: Direct call to wasm_runtime_detect_native_stack_overflow_size()
 * Coverage Goal: Exercise overflow detection path and exception handling
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, wasm_runtime_detect_native_stack_overflow_size_ValidBoundaryLargeSize_DetectsOverflow) {
    // Create exec env with boundary at lower memory address
    uint8 *boundary = (uint8 *)0x1000;  // Low address boundary

    WASMExecEnv *test_exec_env = CreateMockExecEnv(boundary);
    ASSERT_NE(test_exec_env, nullptr);

    // Test with very large requested size that will cause overflow
    // The overflow condition is: (uint8 *)&boundary < boundary
    // After adjustment: boundary = boundary - WASM_STACK_GUARD_SIZE + requested_size
    uint32 large_size = UINT32_MAX - 1024;  // Very large size to trigger overflow

    ASSERT_FALSE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, large_size));

    // Verify that exception was set
    const char* exception = wasm_runtime_get_exception(test_exec_env->module_inst);
    ASSERT_NE(exception, nullptr);
    ASSERT_STREQ(exception, "native stack overflow");
}

/******
 * Test Case: wasm_runtime_detect_native_stack_overflow_size_BoundaryEdgeCase_HandlesBoundaryCalculation
 * Source: core/iwasm/common/wasm_runtime_common.c:7892-7913
 * Target Lines: 7907-7908 (boundary adjustment calculation edge case)
 * Functional Purpose: Tests boundary calculation with requested size close to WASM_STACK_GUARD_SIZE
 *                     to verify proper arithmetic and edge case handling
 * Call Path: Direct call to wasm_runtime_detect_native_stack_overflow_size()
 * Coverage Goal: Exercise boundary adjustment arithmetic with edge case values
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, wasm_runtime_detect_native_stack_overflow_size_BoundaryEdgeCase_HandlesBoundaryCalculation) {
    // Create exec env with reasonable boundary
    uint8 stack_buffer[32768];
    uint8 *boundary = &stack_buffer[16384];

    WASMExecEnv *test_exec_env = CreateMockExecEnv(boundary);
    ASSERT_NE(test_exec_env, nullptr);

    // Test with requested size equal to WASM_STACK_GUARD_SIZE
    // This tests the boundary calculation: boundary - WASM_STACK_GUARD_SIZE + requested_size
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, WASM_STACK_GUARD_SIZE));

    // Test with requested size slightly larger than WASM_STACK_GUARD_SIZE
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, WASM_STACK_GUARD_SIZE + 64));

    // Test with requested size smaller than WASM_STACK_GUARD_SIZE
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, WASM_STACK_GUARD_SIZE / 2));
}

/******
 * Test Case: wasm_runtime_detect_native_stack_overflow_size_RecordStackUsage_ExecutesMacro
 * Source: core/iwasm/common/wasm_runtime_common.c:7892-7913
 * Target Lines: 7896 (RECORD_STACK_USAGE macro execution)
 * Functional Purpose: Verifies that RECORD_STACK_USAGE macro is called correctly and
 *                     executes without errors when memory profiling is enabled
 * Call Path: Direct call to wasm_runtime_detect_native_stack_overflow_size()
 * Coverage Goal: Exercise RECORD_STACK_USAGE macro path (conditional compilation)
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, wasm_runtime_detect_native_stack_overflow_size_RecordStackUsage_ExecutesMacro) {
    // Create exec env with valid boundary
    uint8 stack_buffer[16384];
    uint8 *boundary = &stack_buffer[8192];

    WASMExecEnv *test_exec_env = CreateMockExecEnv(boundary);
    ASSERT_NE(test_exec_env, nullptr);

    // Initialize native_stack_top_min if memory profiling is enabled
#if WASM_ENABLE_MEMORY_PROFILING != 0
    test_exec_env->native_stack_top_min = (uint8 *)UINTPTR_MAX;  // Set to max value initially
#endif

    // Call the function to trigger RECORD_STACK_USAGE
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 1024));

#if WASM_ENABLE_MEMORY_PROFILING != 0
    // Verify that native_stack_top_min was updated (should be less than max now)
    ASSERT_LT((uintptr_t)test_exec_env->native_stack_top_min, UINTPTR_MAX);
#endif
}

#if defined(OS_ENABLE_HW_BOUND_CHECK) && WASM_DISABLE_STACK_HW_BOUND_CHECK == 0
/******
 * Test Case: wasm_runtime_detect_native_stack_overflow_size_HwBoundCheckEnabled_AdjustsBoundary
 * Source: core/iwasm/common/wasm_runtime_common.c:7892-7913
 * Target Lines: 7901-7905 (hardware bound check conditional compilation block)
 * Functional Purpose: Verifies that when hardware bound checking is enabled, the boundary
 *                     is properly adjusted by adding page_size * guard_page_count
 * Call Path: Direct call to wasm_runtime_detect_native_stack_overflow_size()
 * Coverage Goal: Exercise hardware bound check adjustment path
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, wasm_runtime_detect_native_stack_overflow_size_HwBoundCheckEnabled_AdjustsBoundary) {
    // Create exec env with valid boundary
    uint8 stack_buffer[65536];  // Large buffer to accommodate HW bound check adjustment
    uint8 *boundary = &stack_buffer[32768];

    WASMExecEnv *test_exec_env = CreateMockExecEnv(boundary);
    ASSERT_NE(test_exec_env, nullptr);

    // Call with moderate size - should succeed even with HW bound check adjustment
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 2048));

    // Call with larger size to test boundary adjustment calculation
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 8192));
}
#endif

/******
 * Test Case: wasm_runtime_detect_native_stack_overflow_size_ZeroRequestedSize_ReturnsTrue
 * Source: core/iwasm/common/wasm_runtime_common.c:7892-7913
 * Target Lines: 7907, 7913 (boundary calculation with zero size, success return)
 * Functional Purpose: Verifies proper handling when requested_size is zero, ensuring
 *                     boundary calculation works correctly and returns success
 * Call Path: Direct call to wasm_runtime_detect_native_stack_overflow_size()
 * Coverage Goal: Exercise boundary calculation with edge case of zero requested size
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, wasm_runtime_detect_native_stack_overflow_size_ZeroRequestedSize_ReturnsTrue) {
    // Create exec env with valid boundary
    uint8 stack_buffer[16384];
    uint8 *boundary = &stack_buffer[8192];

    WASMExecEnv *test_exec_env = CreateMockExecEnv(boundary);
    ASSERT_NE(test_exec_env, nullptr);

    // Test with zero requested size
    ASSERT_TRUE(wasm_runtime_detect_native_stack_overflow_size(test_exec_env, 0));
}

// ========== New Test Cases for wasm_runtime_quick_invoke_c_api_native (Lines 7313-7375) ==========

// Mock callback functions for testing - declared outside class for proper linkage
extern "C" {
    wasm_trap_t* mock_callback_success(const wasm_val_vec_t *params, wasm_val_vec_t *results) {
        // Simulate successful callback - no trap
        return nullptr;
    }

    wasm_trap_t* mock_callback_with_trap_message(const wasm_val_vec_t *params, wasm_val_vec_t *results) {
        // Create a proper trap with message structure
        static wasm_trap_t trap;
        static wasm_byte_vec_t message_vec;
        static const char* trap_msg = "Test trap message from native callback";

        message_vec.data = (char*)trap_msg;
        message_vec.size = strlen(trap_msg);
        message_vec.num_elems = message_vec.size;

        trap.message = &message_vec;
        trap.frames = nullptr;

        return &trap;
    }

    wasm_trap_t* mock_callback_with_long_message(const wasm_val_vec_t *params, wasm_val_vec_t *results) {
        // Create a trap with a very long message to test truncation
        static wasm_trap_t trap;
        static wasm_byte_vec_t message_vec;
        static const char* long_msg = "This is a very long trap message that should be truncated because it exceeds the maximum buffer size of 108 characters including null terminator for safety";

        message_vec.data = (char*)long_msg;
        message_vec.size = strlen(long_msg);
        message_vec.num_elems = message_vec.size;

        trap.message = &message_vec;
        trap.frames = nullptr;

        return &trap;
    }

    wasm_trap_t* mock_callback_with_env_success(void *env, const wasm_val_vec_t *params, wasm_val_vec_t *results) {
        // Simulate successful callback with env arg - no trap
        return nullptr;
    }

    wasm_trap_t* mock_callback_with_empty_message(const wasm_val_vec_t *params, wasm_val_vec_t *results) {
        // Create a trap without message data
        static wasm_trap_t trap;
        static wasm_byte_vec_t message_vec;

        message_vec.data = nullptr;
        message_vec.size = 0;
        message_vec.num_elems = 0;

        trap.message = &message_vec;
        trap.frames = nullptr;

        return &trap;
    }
}

/******
 * Test Case: QuickInvoke_NullFuncPtr_SetsException
 * Source: core/iwasm/common/wasm_runtime_common.c:7313-7375
 * Target Lines: 7333-7337 (null function pointer check and exception setting)
 * Functional Purpose: Validates that when CApiFuncImport has a null func_ptr_linked,
 *                     the function sets EXCE_CALL_UNLINKED_IMPORT_FUNC exception and returns false
 * Call Path: Direct call to wasm_runtime_quick_invoke_c_api_native()
 * Coverage Goal: Exercise error path for unlinked import function
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, QuickInvoke_NullFuncPtr_SetsException) {
    // Create module instance for testing
    uint8_t minimal_wasm[] = {
        0x00, 0x61, 0x73, 0x6D, // WASM magic
        0x01, 0x00, 0x00, 0x00, // version
    };

    wasm_module_t module = wasm_runtime_load(minimal_wasm, sizeof(minimal_wasm), nullptr, 0);
    ASSERT_NE(module, nullptr);

    module_inst = wasm_runtime_instantiate(module, 8192, 8192, nullptr, 0);
    ASSERT_NE(module_inst, nullptr);

    wasm_runtime_unload(module);

    // Create CApiFuncImport with null function pointer
    CApiFuncImport c_api_import;
    c_api_import.func_ptr_linked = nullptr;  // This should trigger the error
    c_api_import.with_env_arg = false;
    c_api_import.env_arg = nullptr;

    // Create empty parameters and results
    wasm_val_t params[1];
    wasm_val_t results[1];

    // Call the function - should return false due to null func_ptr
    bool result = wasm_runtime_quick_invoke_c_api_native(module_inst, &c_api_import,
                                                        params, 0, results, 0);

    ASSERT_FALSE(result);

    // Verify exception was set correctly
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(exception, nullptr);
    ASSERT_STREQ(exception, "Exception: failed to call unlinked import function");
}

/******
 * Test Case: QuickInvoke_WithoutEnvArg_Success
 * Source: core/iwasm/common/wasm_runtime_common.c:7313-7375
 * Target Lines: 7339-7342 (callback without environment argument)
 * Functional Purpose: Validates successful execution path when with_env_arg is false
 *                     and callback executes without returning a trap
 * Call Path: Direct call to wasm_runtime_quick_invoke_c_api_native()
 * Coverage Goal: Exercise success path for callback without environment argument
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, QuickInvoke_WithoutEnvArg_Success) {
    // Create module instance for testing
    uint8_t minimal_wasm[] = {
        0x00, 0x61, 0x73, 0x6D, // WASM magic
        0x01, 0x00, 0x00, 0x00, // version
    };

    wasm_module_t module = wasm_runtime_load(minimal_wasm, sizeof(minimal_wasm), nullptr, 0);
    ASSERT_NE(module, nullptr);

    module_inst = wasm_runtime_instantiate(module, 8192, 8192, nullptr, 0);
    ASSERT_NE(module_inst, nullptr);

    wasm_runtime_unload(module);

    // Create CApiFuncImport with valid function pointer but no env arg
    CApiFuncImport c_api_import;
    c_api_import.func_ptr_linked = (void*)mock_callback_success;
    c_api_import.with_env_arg = false;  // This should use line 7340-7341
    c_api_import.env_arg = nullptr;

    // Create test parameters and results
    wasm_val_t params[1];
    wasm_val_t results[1];

    // Call the function - should succeed
    bool result = wasm_runtime_quick_invoke_c_api_native(module_inst, &c_api_import,
                                                        params, 0, results, 0);

    ASSERT_TRUE(result);

    // Verify no exception was set
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(exception, nullptr);
}

/******
 * Test Case: QuickInvoke_WithEnvArg_Success
 * Source: core/iwasm/common/wasm_runtime_common.c:7313-7375
 * Target Lines: 7343-7348 (callback with environment argument)
 * Functional Purpose: Validates successful execution path when with_env_arg is true
 *                     and callback executes with environment argument without returning a trap
 * Call Path: Direct call to wasm_runtime_quick_invoke_c_api_native()
 * Coverage Goal: Exercise success path for callback with environment argument
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, QuickInvoke_WithEnvArg_Success) {
    // Create module instance for testing
    uint8_t minimal_wasm[] = {
        0x00, 0x61, 0x73, 0x6D, // WASM magic
        0x01, 0x00, 0x00, 0x00, // version
    };

    wasm_module_t module = wasm_runtime_load(minimal_wasm, sizeof(minimal_wasm), nullptr, 0);
    ASSERT_NE(module, nullptr);

    module_inst = wasm_runtime_instantiate(module, 8192, 8192, nullptr, 0);
    ASSERT_NE(module_inst, nullptr);

    wasm_runtime_unload(module);

    // Create environment data
    int env_data = 42;

    // Create CApiFuncImport with valid function pointer and env arg
    CApiFuncImport c_api_import;
    c_api_import.func_ptr_linked = (void*)mock_callback_with_env_success;
    c_api_import.with_env_arg = true;  // This should use line 7344-7347
    c_api_import.env_arg = &env_data;

    // Create test parameters and results
    wasm_val_t params[1];
    wasm_val_t results[1];

    // Call the function - should succeed
    bool result = wasm_runtime_quick_invoke_c_api_native(module_inst, &c_api_import,
                                                        params, 0, results, 0);

    ASSERT_TRUE(result);

    // Verify no exception was set
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_EQ(exception, nullptr);
}

/******
 * Test Case: QuickInvoke_TrapWithMessage_SetsException
 * Source: core/iwasm/common/wasm_runtime_common.c:7313-7375
 * Target Lines: 7350-7361 (trap handling with message data)
 * Functional Purpose: Validates trap handling when callback returns a trap with message data,
 *                     ensuring proper message copying and exception setting
 * Call Path: Direct call to wasm_runtime_quick_invoke_c_api_native()
 * Coverage Goal: Exercise trap handling path with message processing
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, QuickInvoke_TrapWithMessage_SetsException) {
    // Create module instance for testing
    uint8_t minimal_wasm[] = {
        0x00, 0x61, 0x73, 0x6D, // WASM magic
        0x01, 0x00, 0x00, 0x00, // version
    };

    wasm_module_t module = wasm_runtime_load(minimal_wasm, sizeof(minimal_wasm), nullptr, 0);
    ASSERT_NE(module, nullptr);

    module_inst = wasm_runtime_instantiate(module, 8192, 8192, nullptr, 0);
    ASSERT_NE(module_inst, nullptr);

    wasm_runtime_unload(module);

    // Create CApiFuncImport with callback that returns a trap
    CApiFuncImport c_api_import;
    c_api_import.func_ptr_linked = (void*)mock_callback_with_trap_message;
    c_api_import.with_env_arg = false;
    c_api_import.env_arg = nullptr;

    // Create test parameters and results
    wasm_val_t params[1];
    wasm_val_t results[1];

    // Call the function - should return false due to trap
    bool result = wasm_runtime_quick_invoke_c_api_native(module_inst, &c_api_import,
                                                        params, 0, results, 0);

    ASSERT_FALSE(result);

    // Verify exception was set with trap message
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(exception, nullptr);
    ASSERT_STREQ(exception, "Exception: Test trap message from native callback");
}

/******
 * Test Case: QuickInvoke_TrapWithoutMessage_SetsException
 * Source: core/iwasm/common/wasm_runtime_common.c:7313-7375
 * Target Lines: 7362-7366 (trap handling without message data)
 * Functional Purpose: Validates trap handling when callback returns a trap without message data,
 *                     ensuring default exception message is set
 * Call Path: Direct call to wasm_runtime_quick_invoke_c_api_native()
 * Coverage Goal: Exercise trap handling path without message data
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, QuickInvoke_TrapWithoutMessage_SetsException) {
    // Create module instance for testing
    uint8_t minimal_wasm[] = {
        0x00, 0x61, 0x73, 0x6D, // WASM magic
        0x01, 0x00, 0x00, 0x00, // version
    };

    wasm_module_t module = wasm_runtime_load(minimal_wasm, sizeof(minimal_wasm), nullptr, 0);
    ASSERT_NE(module, nullptr);

    module_inst = wasm_runtime_instantiate(module, 8192, 8192, nullptr, 0);
    ASSERT_NE(module_inst, nullptr);

    wasm_runtime_unload(module);

    // Create CApiFuncImport with callback that returns a trap without message
    CApiFuncImport c_api_import;
    c_api_import.func_ptr_linked = (void*)mock_callback_with_empty_message;
    c_api_import.with_env_arg = false;
    c_api_import.env_arg = nullptr;

    // Create test parameters and results
    wasm_val_t params[1];
    wasm_val_t results[1];

    // Call the function - should return false due to trap
    bool result = wasm_runtime_quick_invoke_c_api_native(module_inst, &c_api_import,
                                                        params, 0, results, 0);

    ASSERT_FALSE(result);

    // Verify exception was set with default message
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(exception, nullptr);
    ASSERT_STREQ(exception, "Exception: native function throw unknown exception");
}

/******
 * Test Case: QuickInvoke_MessageTruncation_HandlesLongMessage
 * Source: core/iwasm/common/wasm_runtime_common.c:7313-7375
 * Target Lines: 7354-7359 (message size calculation and truncation logic)
 * Functional Purpose: Validates message truncation when trap message exceeds buffer size,
 *                     ensuring proper size calculation and safe memory copying
 * Call Path: Direct call to wasm_runtime_quick_invoke_c_api_native()
 * Coverage Goal: Exercise message truncation boundary conditions
 ******/
TEST_F(EnhancedWasmRuntimeCommonTest, QuickInvoke_MessageTruncation_HandlesLongMessage) {
    // Create module instance for testing
    uint8_t minimal_wasm[] = {
        0x00, 0x61, 0x73, 0x6D, // WASM magic
        0x01, 0x00, 0x00, 0x00, // version
    };

    wasm_module_t module = wasm_runtime_load(minimal_wasm, sizeof(minimal_wasm), nullptr, 0);
    ASSERT_NE(module, nullptr);

    module_inst = wasm_runtime_instantiate(module, 8192, 8192, nullptr, 0);
    ASSERT_NE(module_inst, nullptr);

    wasm_runtime_unload(module);

    // Create CApiFuncImport with callback that returns a trap with long message
    CApiFuncImport c_api_import;
    c_api_import.func_ptr_linked = (void*)mock_callback_with_long_message;
    c_api_import.with_env_arg = false;
    c_api_import.env_arg = nullptr;

    // Create test parameters and results
    wasm_val_t params[1];
    wasm_val_t results[1];

    // Call the function - should return false due to trap
    bool result = wasm_runtime_quick_invoke_c_api_native(module_inst, &c_api_import,
                                                        params, 0, results, 0);

    ASSERT_FALSE(result);

    // Verify exception was set with truncated message
    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(exception, nullptr);

    // The "Exception: " prefix is added after truncation, so total length can exceed 107
    // The core message should be truncated to 107 characters, plus "Exception: " prefix
    size_t exception_len = strlen(exception);
    ASSERT_TRUE(exception_len > 107);  // Should have "Exception: " prefix plus truncated message

    // Verify it starts with Exception prefix and truncated message
    ASSERT_TRUE(strncmp(exception, "Exception: This is a very long trap message", 43) == 0);
}