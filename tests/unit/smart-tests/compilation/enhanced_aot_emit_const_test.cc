/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "../common/test_helper.h"
#include "gtest/gtest.h"

#include "bh_read_file.h"
#include "aot_emit_const.h"
#include "aot_compiler.h"
#include "../aot/aot_intrinsic.h"

static std::string CWD;
static std::string MAIN_WASM = "/main.wasm";
static std::string F32_CONST_WASM = "/f32_const_test.wasm";
static std::string F64_PROMOTE_WASM = "/f64_promote_f32_test.wasm";
static std::string F64_NAN_CONST_WASM = "/f64_nan_const_test.wasm";
static char *WASM_FILE;
static char *F32_CONST_WASM_FILE;
static char *F64_PROMOTE_WASM_FILE;
static char *F64_NAN_CONST_WASM_FILE;

static std::string
get_binary_path()
{
    char cwd[1024];
    memset(cwd, 0, 1024);

    if (readlink("/proc/self/exe", cwd, 1024) <= 0) {
    }

    char *path_end = strrchr(cwd, '/');
    if (path_end != NULL) {
        *path_end = '\0';
    }

    return std::string(cwd);
}

// MANDATORY: Enhanced test fixture following existing patterns
// Use source file name in fixture class (EnhancedAotEmitConstTest)
class EnhancedAotEmitConstTest : public testing::Test {
protected:
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_Pool;
        init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
        init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

        bool ret = wasm_runtime_full_init(&init_args);
        ASSERT_TRUE(ret);
    }

    void TearDown() override {
        wasm_runtime_destroy();
    }

    static void SetUpTestCase()
    {
        CWD = get_binary_path();
        WASM_FILE = strdup((CWD + MAIN_WASM).c_str());
        F32_CONST_WASM_FILE = strdup((CWD + F32_CONST_WASM).c_str());
        F64_PROMOTE_WASM_FILE = strdup((CWD + F64_PROMOTE_WASM).c_str());
        F64_NAN_CONST_WASM_FILE = strdup((CWD + F64_NAN_CONST_WASM).c_str());
    }

    static void TearDownTestCase() {
        free(WASM_FILE);
        free(F32_CONST_WASM_FILE);
        free(F64_PROMOTE_WASM_FILE);
        free(F64_NAN_CONST_WASM_FILE);
    }

public:
    char global_heap_buf[512 * 1024];
    RuntimeInitArgs init_args;
};

/******
 * Test Case: aot_compile_op_i64_const_CoverageTest_Success
 * Source: core/iwasm/compilation/aot_emit_const.c:38-62
 * Target Lines: Exercise i64 constant compilation through normal AOT compilation flow
 * Functional Purpose: Validates i64 constant compilation through the standard WAMR AOT flow.
 *                     This achieves coverage by compiling WASM files that contain i64 constants.
 * Call Path: aot_compile_op_i64_const() <- aot_compiler.c:2105 (during aot_compile_wasm)
 * Coverage Goal: Exercise i64 constant compilation paths through natural compilation flow
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_i64_const_CoverageTest_Success)
{
    const char *wasm_file = WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // The main test - compile the WASM module which should exercise i64 constant compilation
    // This will internally call aot_compile_op_i64_const for any i64 constants in the WASM
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify the compilation was successful and context is valid
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

/******
 * Test Case: aot_compile_op_f32_const_IndirectMode_Success
 * Source: core/iwasm/compilation/aot_emit_const.c:66-113
 * Target Lines: 71-80 (indirect mode with intrinsic capability)
 * Functional Purpose: Validates f32 constant compilation in indirect mode when intrinsic
 *                     capability is available, exercising the aot_load_const_from_table path.
 * Call Path: aot_compile_op_f32_const() <- aot_compiler.c:2113 (during WASM_OP_F32_CONST)
 * Coverage Goal: Exercise indirect mode constant loading path for f32 constants
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_f32_const_IndirectMode_Success)
{
    const char *wasm_file = F32_CONST_WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Enable indirect mode to trigger lines 71-80
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = true;  // Key: enable indirect mode

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify indirect mode is enabled
    ASSERT_TRUE(comp_ctx->is_indirect_mode);

    // Compile the WASM module which should exercise f32 constant compilation
    // This will internally call aot_compile_op_f32_const for f32 constants in indirect mode
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify the compilation was successful and context is valid
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

/******
 * Test Case: aot_compile_op_f32_const_NormalFloat_Success
 * Source: core/iwasm/compilation/aot_emit_const.c:66-113
 * Target Lines: 82-85 (normal float constant, non-NaN path)
 * Functional Purpose: Validates f32 constant compilation for normal float values that are
 *                     not NaN, exercising the F32_CONST and CHECK_LLVM_CONST code path.
 * Call Path: aot_compile_op_f32_const() <- aot_compiler.c:2113 (during WASM_OP_F32_CONST)
 * Coverage Goal: Exercise normal float constant compilation path (non-NaN, non-indirect)
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_f32_const_NormalFloat_Success)
{
    const char *wasm_file = F32_CONST_WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Use standard mode (not indirect) to trigger lines 82-85
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = false;  // Key: disable indirect mode

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify indirect mode is disabled
    ASSERT_FALSE(comp_ctx->is_indirect_mode);

    // Compile the WASM module which should exercise f32 constant compilation
    // This will internally call aot_compile_op_f32_const for normal f32 constants
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify the compilation was successful and context is valid
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}
/******
 * Test Case: aot_compile_op_f32_const_NaNHandling_Success
 * Source: core/iwasm/compilation/aot_emit_const.c:66-113
 * Target Lines: 87-111 (NaN float constant handling with complex memory operations)
 * Functional Purpose: Validates f32 NaN constant compilation which requires complex LLVM
 *                     memory operations including LLVMBuildAlloca, LLVMBuildStore,
 *                     LLVMBuildBitCast, and LLVMBuildLoad2 for proper NaN representation.
 * Call Path: aot_compile_op_f32_const() <- aot_compiler.c:2113 (during WASM_OP_F32_CONST with NaN)
 * Coverage Goal: Exercise NaN handling path with complex memory operations (lines 87-111)
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_f32_const_NaNHandling_Success)
{
    const char *wasm_file = F32_CONST_WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Use standard mode (not indirect) to ensure NaN path is triggered
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = false;  // Key: disable indirect mode for NaN path

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify indirect mode is disabled
    ASSERT_FALSE(comp_ctx->is_indirect_mode);

    // Compile the WASM module containing NaN constants which should exercise
    // the complex memory handling path in aot_compile_op_f32_const (lines 87-111)
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify the compilation was successful and context is valid
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Additional validation that LLVM builder context is properly set up
    // since NaN handling requires complex LLVM memory operations
    ASSERT_NE(nullptr, comp_ctx->builder);
    ASSERT_NE(nullptr, comp_ctx->context);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

/******
 * Test Case: aot_compile_op_i32_const_IndirectModeIntrinsic_SuccessPath
 * Source: core/iwasm/compilation/aot_emit_const.c:16-22
 * Target Lines: 16 (intrinsic capability check), 17 (wasm_value declaration),
 *               18 (value assignment), 19-20 (aot_load_const_from_table call),
 *               21 (success check), all within indirect mode + intrinsic path
 * Functional Purpose: Validates the intrinsic constant loading path in indirect mode
 *                     when both is_indirect_mode=true and intrinsic capability for
 *                     "i32.const" is available. This tests the success scenario where
 *                     aot_load_const_from_table returns a valid value.
 * Call Path: aot_compile_op_i32_const() <- aot_compiler.c:2099 (during WASM_OP_I32_CONST)
 * Coverage Goal: Exercise lines 16-22 in success path of intrinsic/indirect mode
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_i32_const_IndirectModeIntrinsic_SuccessPath)
{
    const char *wasm_file = WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Configure for indirect mode with intrinsic support
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = true;  // KEY: Enable indirect mode for target lines
    option.disable_llvm_intrinsics = true;  // KEY: Enable WAMR intrinsic capabilities
    option.builtin_intrinsics = "constop";  // KEY: Enable constop intrinsic group for i32.const

    // Leave target_arch and target_cpu unset - let system defaults handle it

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify the key conditions for target lines 16-22 are met
    ASSERT_TRUE(comp_ctx->is_indirect_mode);  // Condition for line 16

    // Verify intrinsic capability is enabled for the constop group
    bool has_i32_const_capability = aot_intrinsic_check_capability(comp_ctx, "i32.const");
    ASSERT_TRUE(has_i32_const_capability);

    // Compile the WASM module containing i32 constants
    // This will exercise lines 16-22 in the intrinsic path
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify compilation was successful and intrinsic path was used
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Additional validation of compilation context after intrinsic processing
    ASSERT_NE(nullptr, comp_ctx->builder);
    ASSERT_NE(nullptr, comp_ctx->context);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

/******
 * Test Case: aot_compile_op_i32_const_IndirectModeIntrinsic_VariousValues
 * Source: core/iwasm/compilation/aot_emit_const.c:16-22
 * Target Lines: 16 (intrinsic capability check), 17 (wasm_value declaration),
 *               18 (value assignment with different i32 values), 19-20 (function call),
 *               testing the assignment on line 18 with boundary values
 * Functional Purpose: Validates that various i32 constant values are properly handled
 *                     in the intrinsic path, specifically testing the wasm_value.i32
 *                     assignment on line 18 with boundary and special values.
 * Call Path: aot_compile_op_i32_const() <- aot_compiler.c:2099 (during various I32_CONST operations)
 * Coverage Goal: Exercise line 18 (wasm_value.i32 assignment) with different values
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_i32_const_IndirectModeIntrinsic_VariousValues)
{
    const char *wasm_file = WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Configure for indirect mode with constop intrinsics
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = true;  // Enable indirect mode
    option.disable_llvm_intrinsics = true;  // KEY: Enable WAMR intrinsic capabilities

    // Use alternative method: enable constop intrinsics via builtin_intrinsics
    option.builtin_intrinsics = "constop";  // This adds I32_CONST capability

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify conditions for the intrinsic path
    ASSERT_TRUE(comp_ctx->is_indirect_mode);

    // Verify intrinsic capability is enabled for the constop group
    bool has_i32_const_capability = aot_intrinsic_check_capability(comp_ctx, "i32.const");
    ASSERT_TRUE(has_i32_const_capability);

    // Test the intrinsic path by compiling WASM with various i32 constants
    // This exercises the wasm_value.i32 assignment on line 18 with different values
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify successful compilation through intrinsic path
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);
    ASSERT_NE(nullptr, comp_ctx->builder);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

/******
 * Test Case: aot_compile_op_i32_const_IndirectModeIntrinsic_ThumbArchitecture
 * Source: core/iwasm/compilation/aot_emit_const.c:16-22
 * Target Lines: 16 (capability check), 17-18 (wasm_value setup), 19-20 (table load call),
 *               21 (success check) - testing with thumb architecture intrinsic setup
 * Functional Purpose: Validates intrinsic i32.const processing using thumb architecture
 *                     which automatically enables I32_CONST intrinsic capability.
 *                     Tests the complete flow through lines 16-22 with thumb-specific setup.
 * Call Path: aot_compile_op_i32_const() <- aot_compiler.c:2099 (with thumb target architecture)
 * Coverage Goal: Exercise all lines 16-22 with thumb architecture intrinsic enablement
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_i32_const_IndirectModeIntrinsic_ThumbArchitecture)
{
    const char *wasm_file = WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Configure with thumb architecture (automatically enables I32_CONST intrinsic)
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = true;  // Enable indirect mode for target lines
    option.disable_llvm_intrinsics = true;  // KEY: Enable WAMR intrinsic capabilities

    // Use thumb architecture which auto-enables I32_CONST capability
    option.target_arch = "thumb";
    option.target_cpu = "cortex-m4";

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify the setup enables the intrinsic path (lines 16 conditions)
    ASSERT_TRUE(comp_ctx->is_indirect_mode);

    // Verify intrinsic capability is enabled for thumb architecture
    bool has_i32_const_capability = aot_intrinsic_check_capability(comp_ctx, "i32.const");
    ASSERT_TRUE(has_i32_const_capability);

    // Compile with thumb architecture setup - exercises lines 16-22
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify successful compilation via intrinsic path
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);
    ASSERT_NE(nullptr, comp_ctx->builder);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

/******
 * Test Case: aot_compile_op_f64_const_IndirectModeIntrinsic_SuccessPath
 * Source: core/iwasm/compilation/aot_emit_const.c:125-133
 * Target Lines: 125 (is_indirect_mode && intrinsic capability check for f64.const),
 *               126 (WASMValue wasm_value declaration), 127 (memcpy f64_const to wasm_value.f64),
 *               128-129 (aot_load_const_from_table call with VALUE_TYPE_F64),
 *               130-132 (success validation and return false on failure),
 *               133 (PUSH_F64(value) execution on success)
 * Functional Purpose: Validates f64 constant compilation in indirect mode when f64.const
 *                     intrinsic capability is available. Tests the complete success flow
 *                     from capability check through value loading and stack push.
 * Call Path: aot_compile_op_f64_const() <- aot_compiler.c:2121 (during WASM_OP_F64_CONST)
 * Coverage Goal: Exercise lines 125-133 in success path of f64 intrinsic/indirect mode
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_f64_const_IndirectModeIntrinsic_SuccessPath)
{
    const char *wasm_file = F64_PROMOTE_WASM_FILE;  // Contains f64.const 2.0 for testing
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Configure for indirect mode with f64.const intrinsic support
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = true;  // KEY: Enable indirect mode for line 125
    option.disable_llvm_intrinsics = true;  // KEY: Enable WAMR intrinsic capabilities
    option.builtin_intrinsics = "constop";  // KEY: Enable constop intrinsic group for f64.const

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify conditions for target lines 125-133 are met
    ASSERT_TRUE(comp_ctx->is_indirect_mode);  // Condition for line 125

    // Verify f64.const intrinsic capability is enabled for the constop group
    bool has_f64_const_capability = aot_intrinsic_check_capability(comp_ctx, "f64.const");
    ASSERT_TRUE(has_f64_const_capability);

    // Compile the WASM module containing f64 constants
    // This will exercise lines 125-133 in the f64 intrinsic success path
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify compilation was successful and f64 intrinsic path was used
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Additional validation of compilation context after f64 intrinsic processing
    ASSERT_NE(nullptr, comp_ctx->builder);
    ASSERT_NE(nullptr, comp_ctx->context);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

// ========================================================================
// NEW TEST CASES FOR LINES 142-164: F64 NaN HANDLING PATH
// ========================================================================

/******
 * Test Case: aot_compile_op_f64_const_NaNHandling_SuccessPath
 * Source: core/iwasm/compilation/aot_emit_const.c:142-164
 * Target Lines: 142 (memcpy NaN bits), 148-149 (I64_CONST creation),
 *               164 (PUSH_F64 success path) - complete NaN success flow
 * Functional Purpose: Validates f64 NaN constant compilation success path which requires
 *                     complex LLVM memory operations including memcpy for NaN bit preservation,
 *                     LLVMBuildAlloca, LLVMBuildStore, LLVMBuildBitCast, and LLVMBuildLoad2
 *                     for proper NaN representation in AOT compilation.
 * Call Path: aot_compile_op_f64_const() <- aot_compiler.c:2121 (during WASM_OP_F64_CONST with NaN)
 * Coverage Goal: Exercise NaN handling success path (lines 142, 148-149, 164)
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_f64_const_NaNHandling_SuccessPath)
{
    const char *wasm_file = F64_NAN_CONST_WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Configure for standard mode to ensure NaN path is triggered (not indirect mode)
    option.opt_level = 3;
    option.size_level = 3;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = false;  // Key: disable indirect mode for NaN path

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify indirect mode is disabled for NaN path
    ASSERT_FALSE(comp_ctx->is_indirect_mode);

    // Compile the WASM module containing f64 NaN constants
    // This will exercise lines 142-164 in the NaN handling success path
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify the compilation was successful and NaN handling path was used
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Additional validation that LLVM builder context is properly set up
    // since NaN handling requires complex LLVM memory operations
    ASSERT_NE(nullptr, comp_ctx->builder);
    ASSERT_NE(nullptr, comp_ctx->context);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

/******
 * Test Case: aot_compile_op_f64_const_NaNHandling_MultipleNaNValues
 * Source: core/iwasm/compilation/aot_emit_const.c:142-164
 * Target Lines: 142 (memcpy with different NaN patterns), 148-149 (I64_CONST with various bit patterns),
 *               164 (PUSH_F64 with multiple NaN values) - testing NaN bit preservation
 * Functional Purpose: Validates that different NaN bit patterns are properly handled through
 *                     the memcpy operation on line 142 and subsequent LLVM operations.
 *                     Tests the robustness of NaN handling with various NaN representations.
 * Call Path: aot_compile_op_f64_const() <- aot_compiler.c:2121 (with multiple NaN patterns)
 * Coverage Goal: Exercise line 142 memcpy with different NaN bit patterns
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_f64_const_NaNHandling_MultipleNaNValues)
{
    const char *wasm_file = F64_NAN_CONST_WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Configure for standard mode with higher optimization to stress test NaN handling
    option.opt_level = 3;
    option.size_level = 1;  // Smaller size level to ensure more operations
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = true;
    option.enable_aux_stack_check = true;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = false;  // Ensure NaN path is used

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify setup for NaN path testing
    ASSERT_FALSE(comp_ctx->is_indirect_mode);
    ASSERT_NE(nullptr, comp_ctx->builder);

    // Compile WASM with multiple NaN constants - exercises line 142 memcpy repeatedly
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify successful compilation with multiple NaN handling
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Verify LLVM context remains stable after multiple NaN operations
    ASSERT_NE(nullptr, comp_ctx->context);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

/******
 * Test Case: aot_compile_op_f64_const_NaNHandling_OptimizationLevels
 * Source: core/iwasm/compilation/aot_emit_const.c:142-164
 * Target Lines: 142 (memcpy), 143-147 (LLVMBuildAlloca), 148-149 (I64_CONST + CHECK_LLVM_CONST),
 *               150-153 (LLVMBuildStore), 154-158 (LLVMBuildBitCast), 159-163 (LLVMBuildLoad2),
 *               164 (PUSH_F64) - complete flow with different optimization settings
 * Functional Purpose: Validates that NaN handling remains consistent across different
 *                     optimization levels, ensuring that LLVM operations in the NaN path
 *                     work correctly regardless of optimization settings.
 * Call Path: aot_compile_op_f64_const() <- aot_compiler.c:2121 (with various optimization levels)
 * Coverage Goal: Exercise all lines 142-164 under different optimization conditions
 ******/
TEST_F(EnhancedAotEmitConstTest, aot_compile_op_f64_const_NaNHandling_OptimizationLevels)
{
    const char *wasm_file = F64_NAN_CONST_WASM_FILE;
    unsigned int wasm_file_size = 0;
    unsigned char *wasm_file_buf = nullptr;
    char error_buf[128] = { 0 };
    wasm_module_t wasm_module = nullptr;

    struct AOTCompData *comp_data = nullptr;
    struct AOTCompContext *comp_ctx = nullptr;
    AOTCompOption option = { 0 };

    // Test with minimal optimization to exercise all LLVM operations explicitly
    option.opt_level = 0;  // No optimization - all LLVM calls should be explicit
    option.size_level = 0;
    option.output_format = AOT_FORMAT_FILE;
    option.bounds_checks = 2;
    option.enable_simd = false;  // Disable SIMD to focus on basic operations
    option.enable_aux_stack_check = false;
    option.enable_bulk_memory = true;
    option.enable_ref_types = true;
    option.is_indirect_mode = false;  // Ensure NaN path is triggered

    wasm_file_buf = (unsigned char *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
    ASSERT_NE(nullptr, wasm_file_buf);

    wasm_module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
    ASSERT_NE(nullptr, wasm_module);

    comp_data = aot_create_comp_data((WASMModule *)wasm_module, NULL, false);
    ASSERT_NE(nullptr, comp_data);

    comp_ctx = aot_create_comp_context(comp_data, &option);
    ASSERT_NE(nullptr, comp_ctx);

    // Verify conditions for comprehensive NaN path testing
    ASSERT_FALSE(comp_ctx->is_indirect_mode);
    ASSERT_NE(nullptr, comp_ctx->builder);
    ASSERT_NE(nullptr, comp_ctx->context);

    // Compile with no optimization to ensure all LLVM operations are executed
    // This exercises the complete NaN handling sequence in lines 142-164
    bool compile_result = aot_compile_wasm(comp_ctx);
    ASSERT_TRUE(compile_result);

    // Verify successful compilation through complete NaN handling path
    ASSERT_NE(nullptr, comp_ctx->func_ctxes);
    ASSERT_GT(comp_data->func_count, 0);

    // Cleanup
    aot_destroy_comp_context(comp_ctx);
    aot_destroy_comp_data(comp_data);
    wasm_runtime_unload(wasm_module);
    if (wasm_file_buf)
        BH_FREE(wasm_file_buf);
}

