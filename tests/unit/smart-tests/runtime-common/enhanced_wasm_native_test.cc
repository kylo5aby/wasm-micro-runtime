/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "../common/test_helper.h"
#include "gtest/gtest.h"

#include "platform_common.h"
#include "wasm_runtime_common.h"
#include "wasm_native.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"
#include "bh_platform.h"
#include "wasm_export.h"

using namespace std;

// Enhanced test fixture for wasm_native.c functions
class EnhancedWasmNativeTest : public testing::Test {
protected:
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_Pool;
        init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
        init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

        // Initialize WAMR runtime
        bool result = wasm_runtime_full_init(&init_args);
        ASSERT_TRUE(result);

        // Initialize test symbols
        InitializeTestSymbols();
    }

    void TearDown() override {
        // Clean up any registered natives
        if (test_symbols_registered) {
            wasm_native_unregister_natives("test_module", test_symbols);
            test_symbols_registered = false;
        }
        if (test_symbols2_registered) {
            wasm_native_unregister_natives("test_module2", test_symbols2);
            test_symbols2_registered = false;
        }

        wasm_runtime_destroy();
    }

    void InitializeTestSymbols() {
        // Initialize test native symbol arrays
        test_symbols[0].symbol = "test_func1";
        test_symbols[0].func_ptr = (void*)0x12345678;
        test_symbols[0].signature = "(i)i";
        test_symbols[0].attachment = nullptr;

        test_symbols[1].symbol = "test_func2";
        test_symbols[1].func_ptr = (void*)0x87654321;
        test_symbols[1].signature = "(ii)i";
        test_symbols[1].attachment = nullptr;

        test_symbols2[0].symbol = "test_func3";
        test_symbols2[0].func_ptr = (void*)0xAABBCCDD;
        test_symbols2[0].signature = "(f)f";
        test_symbols2[0].attachment = nullptr;

        test_symbols_registered = false;
        test_symbols2_registered = false;
    }

    // Simple test functions to use as native function pointers
    static int test_native_func1(int x) { return x + 1; }
    static int test_native_func2(int x, int y) { return x + y; }
    static float test_native_func3(float x) { return x * 2.0f; }

public:
    char global_heap_buf[512 * 1024];
    RuntimeInitArgs init_args;
    NativeSymbol test_symbols[2];
    NativeSymbol test_symbols2[1];
    bool test_symbols_registered;
    bool test_symbols2_registered;
};

/******
 * Test Case: wasm_native_unregister_natives_ValidMatch_ReturnsTrue
 * Source: core/iwasm/common/wasm_native.c:308-324
 * Target Lines: 308-309 (function signature), 311-312 (declarations), 314 (init prevp),
 *               315-320 (match found path: while loop condition, match check, remove node, free, return true)
 * Functional Purpose: Validates that wasm_native_unregister_natives() correctly finds
 *                     and removes a registered native symbol from the global list,
 *                     returns true indicating successful unregistration.
 * Call Path: wasm_native_unregister_natives() (direct public API call)
 * Coverage Goal: Exercise successful match and removal path in linked list traversal
 ******/
TEST_F(EnhancedWasmNativeTest, wasm_native_unregister_natives_ValidMatch_ReturnsTrue) {
    // Register test symbols first
    bool register_result = wasm_native_register_natives("test_module", test_symbols, 2);
    ASSERT_TRUE(register_result);
    test_symbols_registered = true;

    // Attempt to unregister the same symbols - should succeed
    bool unregister_result = wasm_native_unregister_natives("test_module", test_symbols);
    ASSERT_TRUE(unregister_result);
    test_symbols_registered = false;

    // Verify the symbols are actually removed by trying to unregister again
    bool second_unregister_result = wasm_native_unregister_natives("test_module", test_symbols);
    ASSERT_FALSE(second_unregister_result);
}

/******
 * Test Case: wasm_native_unregister_natives_NoMatch_ReturnsFalse
 * Source: core/iwasm/common/wasm_native.c:308-324
 * Target Lines: 308-309 (function signature), 311-312 (declarations), 314 (init prevp),
 *               315 (while loop condition), 324 (return false)
 * Functional Purpose: Validates that wasm_native_unregister_natives() correctly handles
 *                     the case when no matching native symbols are found in the list,
 *                     returns false indicating unsuccessful unregistration.
 * Call Path: wasm_native_unregister_natives() (direct public API call)
 * Coverage Goal: Exercise no-match path where traversal completes without finding target
 ******/
TEST_F(EnhancedWasmNativeTest, wasm_native_unregister_natives_NoMatch_ReturnsFalse) {
    // Register different symbols first
    bool register_result = wasm_native_register_natives("test_module", test_symbols, 2);
    ASSERT_TRUE(register_result);
    test_symbols_registered = true;

    // Try to unregister different symbols that don't match
    bool unregister_result = wasm_native_unregister_natives("test_module", test_symbols2);
    ASSERT_FALSE(unregister_result);

    // Verify original symbols are still registered
    bool unregister_original = wasm_native_unregister_natives("test_module", test_symbols);
    ASSERT_TRUE(unregister_original);
    test_symbols_registered = false;
}

/******
 * Test Case: wasm_native_unregister_natives_EmptyList_ReturnsFalse
 * Source: core/iwasm/common/wasm_native.c:308-324
 * Target Lines: 308-309 (function signature), 311-312 (declarations), 314 (init prevp),
 *               315 (while loop condition false), 324 (return false)
 * Functional Purpose: Validates that wasm_native_unregister_natives() correctly handles
 *                     the case when the global native symbols list is empty,
 *                     returns false immediately without attempting traversal.
 * Call Path: wasm_native_unregister_natives() (direct public API call)
 * Coverage Goal: Exercise empty list scenario where while loop never executes
 ******/
TEST_F(EnhancedWasmNativeTest, wasm_native_unregister_natives_EmptyList_ReturnsFalse) {
    // Don't register any symbols, list should be empty
    // Try to unregister from empty list
    bool unregister_result = wasm_native_unregister_natives("test_module", test_symbols);
    ASSERT_FALSE(unregister_result);
}

/******
 * Test Case: wasm_native_unregister_natives_WrongModuleName_ReturnsFalse
 * Source: core/iwasm/common/wasm_native.c:308-324
 * Target Lines: 308-309 (function signature), 311-312 (declarations), 314 (init prevp),
 *               315-317 (while loop, condition check fails on module name), 322 (move to next),
 *               324 (return false after traversal)
 * Functional Purpose: Validates that wasm_native_unregister_natives() correctly handles
 *                     the case when symbols are registered but module name doesn't match,
 *                     traverses the list but returns false due to no module name match.
 * Call Path: wasm_native_unregister_natives() (direct public API call)
 * Coverage Goal: Exercise traversal with module name mismatch
 ******/
TEST_F(EnhancedWasmNativeTest, wasm_native_unregister_natives_WrongModuleName_ReturnsFalse) {
    // Register symbols with one module name
    bool register_result = wasm_native_register_natives("test_module", test_symbols, 2);
    ASSERT_TRUE(register_result);
    test_symbols_registered = true;

    // Try to unregister with different module name but same symbols
    bool unregister_result = wasm_native_unregister_natives("different_module", test_symbols);
    ASSERT_FALSE(unregister_result);

    // Verify original symbols are still registered with correct module name
    bool unregister_correct = wasm_native_unregister_natives("test_module", test_symbols);
    ASSERT_TRUE(unregister_correct);
    test_symbols_registered = false;
}

/******
 * Test Case: wasm_native_unregister_natives_MultipleNodes_MatchMiddle_ReturnsTrue
 * Source: core/iwasm/common/wasm_native.c:308-324
 * Target Lines: 308-309 (function signature), 311-312 (declarations), 314 (init prevp),
 *               315-317 (while loop, first iteration no match), 322 (advance to next),
 *               315-320 (second iteration match found, remove node, free, return true)
 * Functional Purpose: Validates that wasm_native_unregister_natives() correctly handles
 *                     linked list traversal when the target node is not the first node,
 *                     successfully finds and removes a node from the middle of the list.
 * Call Path: wasm_native_unregister_natives() (direct public API call)
 * Coverage Goal: Exercise linked list traversal with match in non-first position
 ******/
TEST_F(EnhancedWasmNativeTest, wasm_native_unregister_natives_MultipleNodes_MatchMiddle_ReturnsTrue) {
    // Register first set of symbols
    bool register_result1 = wasm_native_register_natives("test_module1", test_symbols, 2);
    ASSERT_TRUE(register_result1);

    // Register second set of symbols (will be added to head, so first registered becomes second)
    bool register_result2 = wasm_native_register_natives("test_module2", test_symbols2, 1);
    ASSERT_TRUE(register_result2);
    test_symbols2_registered = true;

    // Now unregister the first registered symbols (which should be second in the list)
    bool unregister_result = wasm_native_unregister_natives("test_module1", test_symbols);
    ASSERT_TRUE(unregister_result);

    // Verify second registered symbols are still there
    bool unregister_second = wasm_native_unregister_natives("test_module2", test_symbols2);
    ASSERT_TRUE(unregister_second);
    test_symbols2_registered = false;
}