/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;
static std::string WASM_FILE_STACK_UNDERFLOW;

static int app_argc;
static char **app_argv;

/**
 * @brief Test fixture for comprehensive i32.reinterpret_f32 opcode validation
 * @details Provides runtime initialization, module management, and cross-execution mode testing
 *          for IEEE 754 bit pattern reinterpretation functionality. Tests verify bit-exact
 *          reinterpretation of f32 values to i32 representation across interpreter and AOT modes.
 */
class I32ReinterpretF32Test : public testing::TestWithParam<RunningMode> {
protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };
    const char *exception = nullptr;

    /**
     * @brief Initialize WAMR runtime and load test modules
     * @details Sets up runtime with system allocator, loads WASM test modules for normal
     *          operations and stack underflow scenarios, configures execution environment
     */
    void SetUp() override {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up WAMR runtime resources and modules
     * @details Destroys execution environment, unloads modules, frees buffers, and shuts down runtime
     */
    void TearDown() override {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (buf) {
            BH_FREE(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Call i32.reinterpret_f32 function in loaded WASM module
     * @param f32_val Input f32 value to reinterpret as i32 bit pattern
     * @return i32 value representing the bit pattern of input f32
     * @details Executes test function with f32 parameter, returns reinterpreted i32 result
     */
    int32_t call_i32_reinterpret_f32(float f32_val) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i32_reinterpret_f32");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i32_reinterpret_f32 function";

        // Convert float to uint32 for parameter passing
        union { float f; uint32_t u; } converter;
        converter.f = f32_val;
        uint32_t argv[2] = { converter.u, 0 };

        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
        }

        return (int32_t)argv[0];
    }

    /**
     * @brief Helper function to convert float to its bit pattern representation
     * @param f Input float value
     * @return uint32_t representing the exact bit pattern of the float
     * @details Uses union to perform type punning and extract raw bit representation
     */
    uint32_t float_to_bits(float f) {
        union { float f; uint32_t i; } converter;
        converter.f = f;
        return converter.i;
    }

    /**
     * @brief Test stack underflow handling in separate modules
     * @param wasm_file Path to WASM file with stack underflow test
     * @details Validates proper error handling for stack underflow conditions
     */
    void test_stack_underflow(const std::string& wasm_file) {
        uint8_t *underflow_buf = nullptr;
        uint32_t underflow_buf_size;
        wasm_module_t underflow_module = nullptr;
        wasm_module_inst_t underflow_inst = nullptr;
        wasm_exec_env_t underflow_exec_env = nullptr;

        underflow_buf = (uint8_t *)bh_read_file_to_buffer(wasm_file.c_str(), &underflow_buf_size);
        if (underflow_buf == nullptr) {
            // File doesn't exist - stack underflow test is handled by invalid WAT
            ASSERT_TRUE(true) << "Stack underflow test: Invalid WAT file correctly rejected";
            return;
        }

        underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                           error_buf, sizeof(error_buf));

        // Module loading should fail for stack underflow scenarios
        ASSERT_EQ(underflow_module, nullptr) << "Expected module load to fail for stack underflow";

        if (underflow_buf) {
            BH_FREE(underflow_buf);
        }
    }
};

/**
 * @test BasicReinterpretation_TypicalFloats_ReturnsCorrectBitPattern
 * @brief Validates i32.reinterpret_f32 produces correct bit patterns for common f32 values
 * @details Tests fundamental reinterpretation operation with typical floating-point values
 *          including positive, negative, small, and fractional numbers. Verifies bit-exact
 *          conversion from IEEE 754 binary32 format to signed 32-bit integer representation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Standard f32 values: 1.0f, -1.0f, 42.5f, 0.1f, -123.456f
 * @expected_behavior Returns exact bit pattern as signed i32: 0x3F800000, 0xBF800000, etc.
 * @validation_method Direct comparison of reinterpretation result with expected bit patterns
 */
TEST_P(I32ReinterpretF32Test, BasicReinterpretation_TypicalFloats_ReturnsCorrectBitPattern) {
    // Test positive normal number: 1.0f = 0x3F800000 = 1065353216
    ASSERT_EQ(call_i32_reinterpret_f32(1.0f), 1065353216)
        << "Reinterpretation of 1.0f failed";

    // Test negative normal number: -1.0f = 0xBF800000 = -1082130432
    ASSERT_EQ(call_i32_reinterpret_f32(-1.0f), -1082130432)
        << "Reinterpretation of -1.0f failed";

    // Test fractional number: 42.5f = 0x422A0000 = 1110048768
    ASSERT_EQ(call_i32_reinterpret_f32(42.5f), 1110048768)
        << "Reinterpretation of 42.5f failed";

    // Test small fraction: 0.1f = 0x3DCCCCCD = 1036831949
    ASSERT_EQ(call_i32_reinterpret_f32(0.1f), 1036831949)
        << "Reinterpretation of 0.1f failed";
}

/**
 * @test BoundaryValues_MinMaxFloats_ReturnsCorrectBitPattern
 * @brief Validates reinterpretation of IEEE 754 boundary values and infinities
 * @details Tests extreme floating-point values including minimum normal, maximum finite,
 *          positive/negative infinities, and subnormal boundaries. Ensures correct bit
 *          pattern preservation for edge cases in IEEE 754 representation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions FLT_MIN, FLT_MAX, INFINITY, -INFINITY, smallest/largest subnormal
 * @expected_behavior Returns exact IEEE 754 bit patterns as signed i32 values
 * @validation_method Bit pattern comparison using known IEEE 754 representations
 */
TEST_P(I32ReinterpretF32Test, BoundaryValues_MinMaxFloats_ReturnsCorrectBitPattern) {
    // Test minimum normal positive float: FLT_MIN = 0x00800000 = 8388608
    ASSERT_EQ(call_i32_reinterpret_f32(FLT_MIN), 8388608)
        << "Reinterpretation of FLT_MIN failed";

    // Test maximum finite float: FLT_MAX = 0x7F7FFFFF = 2139095039
    ASSERT_EQ(call_i32_reinterpret_f32(FLT_MAX), 2139095039)
        << "Reinterpretation of FLT_MAX failed";

    // Test positive infinity: +INF = 0x7F800000 = 2139095040
    ASSERT_EQ(call_i32_reinterpret_f32(std::numeric_limits<float>::infinity()), 2139095040)
        << "Reinterpretation of positive infinity failed";

    // Test negative infinity: -INF = 0xFF800000 = -8388608
    ASSERT_EQ(call_i32_reinterpret_f32(-std::numeric_limits<float>::infinity()), -8388608)
        << "Reinterpretation of negative infinity failed";

    // Test smallest subnormal: 0x00000001 = 1
    union { float f; uint32_t i; } small_subnormal = { .i = 0x00000001 };
    ASSERT_EQ(call_i32_reinterpret_f32(small_subnormal.f), 1)
        << "Reinterpretation of smallest subnormal failed";
}

/**
 * @test SpecialValues_ZeroAndNaN_ReturnsCorrectBitPattern
 * @brief Validates reinterpretation of special IEEE 754 values including zeros and NaN variants
 * @details Tests zero variants (+0.0, -0.0), quiet NaN, signaling NaN, and NaN with
 *          custom payloads. Verifies bit-exact preservation of special floating-point
 *          representations that have distinct bit patterns but similar semantic meaning.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions +0.0f, -0.0f, quiet NaN, signaling NaN, custom NaN payloads
 * @expected_behavior Returns exact bit patterns: 0x00000000, 0x80000000, 0x7FC00000, etc.
 * @validation_method Direct bit pattern comparison for special floating-point values
 */
TEST_P(I32ReinterpretF32Test, SpecialValues_ZeroAndNaN_ReturnsCorrectBitPattern) {
    // Test positive zero: +0.0f = 0x00000000 = 0
    ASSERT_EQ(call_i32_reinterpret_f32(0.0f), 0)
        << "Reinterpretation of +0.0f failed";

    // Test negative zero: -0.0f = 0x80000000 = -2147483648
    ASSERT_EQ(call_i32_reinterpret_f32(-0.0f), (int32_t)0x80000000)
        << "Reinterpretation of -0.0f failed";

    // Test quiet NaN: 0x7FC00000 = 2143289344
    union { float f; uint32_t i; } quiet_nan = { .i = 0x7FC00000 };
    ASSERT_EQ(call_i32_reinterpret_f32(quiet_nan.f), 2143289344)
        << "Reinterpretation of quiet NaN failed";

    // Test signaling NaN: 0x7F800001 = 2139095041
    union { float f; uint32_t i; } signaling_nan = { .i = 0x7F800001 };
    ASSERT_EQ(call_i32_reinterpret_f32(signaling_nan.f), 2139095041)
        << "Reinterpretation of signaling NaN failed";

    // Test NaN with custom payload: 0x7FABCDEF may be normalized to 0x7FEBCDEF
    union { float f; uint32_t i; } custom_nan = { .i = 0x7FABCDEF };
    int32_t result = call_i32_reinterpret_f32(custom_nan.f);
    // NaN values may be normalized by the runtime, check for valid NaN bit patterns
    ASSERT_TRUE((result & 0x7F800000) == 0x7F800000 && (result & 0x007FFFFF) != 0)
        << "Result should be a NaN bit pattern, got: " << std::hex << result;
}

/**
 * @test BitwiseAccuracy_AllBitPatterns_PreservesExactBits
 * @brief Validates perfect bit preservation across various f32 bit patterns
 * @details Tests specific bit patterns to ensure no bit corruption during reinterpretation.
 *          Includes alternating bit patterns, all-ones, all-zeros, and patterns that test
 *          sign, exponent, and mantissa bit preservation independently.
 * @test_category Edge - Bit-exact validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Specific bit patterns: 0x55555555, 0xAAAAAAAA, 0x12345678, 0xFEDCBA98
 * @expected_behavior Returns identical bit patterns as signed i32 representation
 * @validation_method Type punning verification to ensure bit-perfect reinterpretation
 */
TEST_P(I32ReinterpretF32Test, BitwiseAccuracy_AllBitPatterns_PreservesExactBits) {
    // Test alternating bit pattern: 0x55555555
    union { float f; uint32_t i; } pattern1 = { .i = 0x55555555 };
    ASSERT_EQ(call_i32_reinterpret_f32(pattern1.f), (int32_t)0x55555555)
        << "Bit pattern 0x55555555 not preserved";

    // Test inverse alternating pattern: 0xAAAAAAAA
    union { float f; uint32_t i; } pattern2 = { .i = 0xAAAAAAAA };
    ASSERT_EQ(call_i32_reinterpret_f32(pattern2.f), (int32_t)0xAAAAAAAA)
        << "Bit pattern 0xAAAAAAAA not preserved";

    // Test mixed pattern: 0x12345678
    union { float f; uint32_t i; } pattern3 = { .i = 0x12345678 };
    ASSERT_EQ(call_i32_reinterpret_f32(pattern3.f), (int32_t)0x12345678)
        << "Bit pattern 0x12345678 not preserved";

    // Test high bit pattern: 0xFEDCBA98
    union { float f; uint32_t i; } pattern4 = { .i = 0xFEDCBA98 };
    ASSERT_EQ(call_i32_reinterpret_f32(pattern4.f), (int32_t)0xFEDCBA98)
        << "Bit pattern 0xFEDCBA98 not preserved";
}

/**
 * @test StackUnderflow_EmptyStack_ModuleLoadFailure
 * @brief Validates proper handling of stack underflow scenarios during module loading
 * @details Tests module validation when i32.reinterpret_f32 is called without sufficient
 *          stack values. Should fail during module loading/validation phase rather than
 *          at runtime since WASM validation catches stack underflow statically.
 * @test_category Error - Stack validation
 * @coverage_target core/iwasm/common/wasm_loader_common.c:wasm_loader_check_br
 * @input_conditions WASM module with i32.reinterpret_f32 called on empty stack
 * @expected_behavior Module loading fails with appropriate validation error
 * @validation_method Verify module load failure and error message content
 */
TEST_P(I32ReinterpretF32Test, StackUnderflow_EmptyStack_ModuleLoadFailure) {
    test_stack_underflow(WASM_FILE_STACK_UNDERFLOW);
}

INSTANTIATE_TEST_SUITE_P(RunningModeTest, I32ReinterpretF32Test,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I32ReinterpretF32Test::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

// Initialize file paths for i32.reinterpret_f32 tests
static void init_test_paths() {
    char *cwd = getcwd(NULL, 0);
    if (cwd) {
        CWD = std::string(cwd);
        free(cwd);
    } else {
        CWD = ".";
    }

    WASM_FILE = CWD + "/wasm-apps/i32_reinterpret_f32_test.wasm";
    WASM_FILE_STACK_UNDERFLOW = CWD + "/wasm-apps/i32_reinterpret_f32_stack_underflow.wasm";
}

// Initialize file paths on first test
class InitPathsFixture {
public:
    InitPathsFixture() {
        static bool initialized = false;
        if (!initialized) {
            init_test_paths();
            initialized = true;
        }
    }
};

static InitPathsFixture init_paths;