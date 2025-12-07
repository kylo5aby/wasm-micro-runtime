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
 * @brief Test fixture for comprehensive f32.reinterpret_i32 opcode validation
 * @details Provides runtime initialization, module management, and cross-execution mode testing
 *          for IEEE 754 bit pattern reinterpretation functionality. Tests verify bit-exact
 *          reinterpretation of i32 values to f32 representation across interpreter and AOT modes.
 */
class F32ReinterpretI32Test : public testing::TestWithParam<RunningMode> {
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
     *          Source: tests/unit/enhanced_opcode/numeric/enhanced_f32_reinterpret_i32_test.cc
     */
    void SetUp() override {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;

        char *wasm_file = strdup(WASM_FILE.c_str());
        ASSERT_NE(wasm_file, nullptr) << "Failed to allocate memory for WASM file path";

        buf = (uint8_t *)bh_read_file_to_buffer(wasm_file, &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(
            module, stack_size, heap_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";

        free(wasm_file);
    }

    /**
     * @brief Clean up WAMR runtime resources
     * @details Destroys execution environment, module instance, unloads module, and frees buffers
     *          Source: tests/unit/enhanced_opcode/numeric/enhanced_f32_reinterpret_i32_test.cc
     */
    void TearDown() override {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
        }
        if (module) {
            wasm_runtime_unload(module);
        }
        if (buf) {
            BH_FREE(buf);
        }
    }

    /**
     * @brief Call WASM f32.reinterpret_i32 function with i32 input
     * @param input i32 value to be reinterpreted as f32
     * @return f32 result of bit pattern reinterpretation
     * @details Executes the WASM test function and returns the reinterpreted f32 value
     *          Source: tests/unit/enhanced_opcode/numeric/enhanced_f32_reinterpret_i32_test.cc
     */
    float call_f32_reinterpret_i32(int32_t input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f32_reinterpret_i32_test");
        EXPECT_NE(func, nullptr) << "Failed to lookup f32_reinterpret_i32_test function";

        uint32_t wasm_argv[1] = { (uint32_t)input };
        uint32_t wasm_ret[1] = { 0 };

        bool call_result = wasm_runtime_call_wasm(exec_env, func, 1, wasm_argv);
        if (!call_result) {
            exception = wasm_runtime_get_exception(module_inst);
            ADD_FAILURE() << "WASM function call failed: " << (exception ? exception : "unknown error");
            return 0.0f;
        }

        // Get return value as f32
        wasm_ret[0] = wasm_argv[0];
        return *(float*)&wasm_ret[0];
    }

    /**
     * @brief Utility function to create bit pattern for IEEE 754 f32
     * @param sign Sign bit (0 or 1)
     * @param exponent 8-bit exponent (biased)
     * @param mantissa 23-bit mantissa
     * @return i32 bit pattern representing the f32 value
     * @details Source: tests/unit/enhanced_opcode/numeric/enhanced_f32_reinterpret_i32_test.cc
     */
    int32_t make_f32_bits(uint32_t sign, uint32_t exponent, uint32_t mantissa) {
        return (int32_t)((sign << 31) | ((exponent & 0xFF) << 23) | (mantissa & 0x7FFFFF));
    }

    /**
     * @brief Convert f32 to its bit representation as i32
     * @param f The float value to convert
     * @return i32 bit pattern of the float
     * @details Source: tests/unit/enhanced_opcode/numeric/enhanced_f32_reinterpret_i32_test.cc
     */
    int32_t f32_to_bits(float f) {
        union { float f; int32_t i; } u;
        u.f = f;
        return u.i;
    }

    /**
     * @brief Convert i32 bit pattern to f32
     * @param i The i32 bit pattern to convert
     * @return f32 value represented by the bit pattern
     * @details Source: tests/unit/enhanced_opcode/numeric/enhanced_f32_reinterpret_i32_test.cc
     */
    float bits_to_f32(int32_t i) {
        union { float f; int32_t i; } u;
        u.i = i;
        return u.f;
    }
};

/**
 * @test BasicReinterpretation_CommonValues_ProducesCorrectFloat
 * @brief Validates f32.reinterpret_i32 produces correct IEEE 754 results for typical inputs
 * @details Tests fundamental bit reinterpretation operation with common integer values.
 *          Verifies that f32.reinterpret_i32 correctly reinterprets i32 bit patterns as f32 values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_reinterpret_i32_operation
 * @input_conditions Standard integer values with known f32 bit patterns
 * @expected_behavior Returns bit-exact f32 interpretation of i32 input
 * @validation_method Direct bit pattern comparison with expected f32 values
 */
TEST_P(F32ReinterpretI32Test, BasicReinterpretation_CommonValues_ProducesCorrectFloat) {
    // Test common integer values that correspond to normal float values

    // 0x3F800000 = 1.0f
    float result = call_f32_reinterpret_i32(0x3F800000);
    ASSERT_EQ(result, 1.0f) << "Reinterpretation of 0x3F800000 should produce 1.0f";

    // 0x40000000 = 2.0f
    result = call_f32_reinterpret_i32(0x40000000);
    ASSERT_EQ(result, 2.0f) << "Reinterpretation of 0x40000000 should produce 2.0f";

    // 0xBF800000 = -1.0f
    result = call_f32_reinterpret_i32(0xBF800000);
    ASSERT_EQ(result, -1.0f) << "Reinterpretation of 0xBF800000 should produce -1.0f";

    // 0x3F000000 = 0.5f
    result = call_f32_reinterpret_i32(0x3F000000);
    ASSERT_EQ(result, 0.5f) << "Reinterpretation of 0x3F000000 should produce 0.5f";
}

/**
 * @test SpecialValues_IEEEPatterns_ProducesExpectedResults
 * @brief Validates f32.reinterpret_i32 handles IEEE 754 special values correctly
 * @details Tests reinterpretation of integer bit patterns that correspond to special IEEE 754
 *          values including positive/negative zero, infinity, and canonical NaN representations.
 * @test_category Corner - IEEE 754 special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_reinterpret_i32_operation
 * @input_conditions Integer bit patterns for IEEE 754 special values
 * @expected_behavior Returns exact IEEE 754 special values: +0, -0, +∞, -∞, NaN
 * @validation_method Bit-exact validation and IEEE 754 property verification
 */
TEST_P(F32ReinterpretI32Test, SpecialValues_IEEEPatterns_ProducesExpectedResults) {
    // Test positive zero: 0x00000000 = +0.0f
    float result = call_f32_reinterpret_i32(0x00000000);
    ASSERT_EQ(result, 0.0f) << "Reinterpretation of 0x00000000 should produce +0.0f";
    ASSERT_FALSE(std::signbit(result)) << "Result should be positive zero";

    // Test negative zero: 0x80000000 = -0.0f
    result = call_f32_reinterpret_i32(0x80000000);
    ASSERT_EQ(result, -0.0f) << "Reinterpretation of 0x80000000 should produce -0.0f";
    ASSERT_TRUE(std::signbit(result)) << "Result should be negative zero";

    // Test positive infinity: 0x7F800000 = +∞
    result = call_f32_reinterpret_i32(0x7F800000);
    ASSERT_TRUE(std::isinf(result)) << "Reinterpretation of 0x7F800000 should produce infinity";
    ASSERT_FALSE(std::signbit(result)) << "Result should be positive infinity";

    // Test negative infinity: 0xFF800000 = -∞
    result = call_f32_reinterpret_i32(0xFF800000);
    ASSERT_TRUE(std::isinf(result)) << "Reinterpretation of 0xFF800000 should produce negative infinity";
    ASSERT_TRUE(std::signbit(result)) << "Result should be negative infinity";

    // Test canonical quiet NaN: 0x7FC00000
    result = call_f32_reinterpret_i32(0x7FC00000);
    ASSERT_TRUE(std::isnan(result)) << "Reinterpretation of 0x7FC00000 should produce NaN";
}

/**
 * @test BoundaryValues_ExtremeIntegers_ProducesValidFloats
 * @brief Validates f32.reinterpret_i32 handles boundary integer values correctly
 * @details Tests reinterpretation of extreme integer values including INT32_MIN, INT32_MAX,
 *          and values at the boundary between normal and denormal float representations.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_reinterpret_i32_operation
 * @input_conditions Extreme i32 values: INT32_MIN, INT32_MAX, denormal boundaries
 * @expected_behavior Returns valid f32 values for all i32 bit patterns
 * @validation_method Verify all results are valid floats with expected properties
 */
TEST_P(F32ReinterpretI32Test, BoundaryValues_ExtremeIntegers_ProducesValidFloats) {
    // Test INT32_MIN (0x80000000) - should be -0.0f
    float result = call_f32_reinterpret_i32(INT32_MIN);
    ASSERT_EQ(result, -0.0f) << "Reinterpretation of INT32_MIN should produce -0.0f";
    ASSERT_TRUE(std::signbit(result)) << "Result should be negative zero";

    // Test INT32_MAX (0x7FFFFFFF) - should be NaN
    result = call_f32_reinterpret_i32(INT32_MAX);
    ASSERT_TRUE(std::isnan(result)) << "Reinterpretation of INT32_MAX should produce NaN";

    // Test smallest positive denormal: 0x00000001
    result = call_f32_reinterpret_i32(0x00000001);
    ASSERT_GT(result, 0.0f) << "Smallest positive denormal should be positive";
    ASSERT_LT(result, FLT_MIN) << "Result should be smaller than FLT_MIN (denormal)";

    // Test largest denormal: 0x007FFFFF
    result = call_f32_reinterpret_i32(0x007FFFFF);
    ASSERT_GT(result, 0.0f) << "Largest denormal should be positive";
    ASSERT_LT(result, FLT_MIN) << "Result should be smaller than FLT_MIN (still denormal)";

    // Test smallest normal: 0x00800000
    result = call_f32_reinterpret_i32(0x00800000);
    ASSERT_EQ(result, FLT_MIN) << "Smallest normal should equal FLT_MIN";
}

/**
 * @test NaNValues_VariousPatterns_ProducesExpectedNaN
 * @brief Validates f32.reinterpret_i32 preserves NaN bit patterns correctly
 * @details Tests reinterpretation of various integer bit patterns that represent different
 *          NaN values including quiet NaN, signaling NaN, and NaN with different payloads.
 * @test_category Edge - NaN pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_reinterpret_i32_operation
 * @input_conditions Various NaN bit patterns with different mantissa payloads
 * @expected_behavior All patterns produce NaN values with preserved bit patterns
 * @validation_method Verify isnan() and check preservation of specific bit patterns
 */
TEST_P(F32ReinterpretI32Test, NaNValues_VariousPatterns_ProducesExpectedNaN) {
    // Test various NaN patterns - all should produce NaN

    // Canonical quiet NaN: 0x7FC00000
    float result = call_f32_reinterpret_i32(0x7FC00000);
    ASSERT_TRUE(std::isnan(result)) << "Canonical quiet NaN should produce NaN";

    // Signaling NaN: 0x7F800001 (mantissa != 0, quiet bit = 0)
    result = call_f32_reinterpret_i32(0x7F800001);
    ASSERT_TRUE(std::isnan(result)) << "Signaling NaN should produce NaN";

    // NaN with payload: 0x7FC12345
    result = call_f32_reinterpret_i32(0x7FC12345);
    ASSERT_TRUE(std::isnan(result)) << "NaN with payload should produce NaN";

    // Negative NaN: 0xFFC00000
    result = call_f32_reinterpret_i32(0xFFC00000);
    ASSERT_TRUE(std::isnan(result)) << "Negative NaN should produce NaN";

    // Maximum NaN: 0x7FFFFFFF
    result = call_f32_reinterpret_i32(0x7FFFFFFF);
    ASSERT_TRUE(std::isnan(result)) << "Maximum NaN pattern should produce NaN";
}

/**
 * @test InverseRelation_WithI32ReinterpretF32_MaintainsConsistency
 * @brief Validates f32.reinterpret_i32 and i32.reinterpret_f32 are inverse operations
 * @details Tests that applying f32.reinterpret_i32 followed by i32.reinterpret_f32 returns
 *          the original i32 value, demonstrating the bit-exact inverse relationship.
 * @test_category Edge - Inverse operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_reinterpret_i32_operation
 * @input_conditions Various i32 values including normal, special, and boundary values
 * @expected_behavior Round-trip conversion preserves original bit patterns exactly
 * @validation_method Compare round-trip result with original input for bit-exact equality
 */
TEST_P(F32ReinterpretI32Test, InverseRelation_WithI32ReinterpretF32_MaintainsConsistency) {
    // Test vectors for round-trip validation
    int32_t test_values[] = {
        0x00000000,  // +0.0f
        (int32_t)0x80000000,  // -0.0f
        0x3F800000,  // 1.0f
        (int32_t)0xBF800000,  // -1.0f
        0x7F800000,  // +∞
        (int32_t)0xFF800000,  // -∞
        0x7FC00000,  // NaN
        0x00000001,  // Smallest denormal
        0x007FFFFF,  // Largest denormal
        0x42280000,  // 42.0f
        0x12345678,  // Arbitrary pattern
        (int32_t)0xABCDEF01   // Another arbitrary pattern
    };

    for (int32_t original : test_values) {
        // Apply f32.reinterpret_i32
        float f32_result = call_f32_reinterpret_i32(original);

        // Apply i32.reinterpret_f32 (bit conversion back)
        int32_t reconstructed = f32_to_bits(f32_result);

        ASSERT_EQ(reconstructed, original)
            << "Round-trip conversion failed for 0x" << std::hex << original
            << ". Got 0x" << std::hex << reconstructed << " instead";
    }
}

/**
 * @test DenormalValues_SubnormalPatterns_ProducesCorrectFloat
 * @brief Validates f32.reinterpret_i32 handles denormal float patterns correctly
 * @details Tests reinterpretation of integer bit patterns that represent denormal (subnormal)
 *          floating-point values with various mantissa patterns and validates proper handling.
 * @test_category Edge - Denormal number validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_reinterpret_i32_operation
 * @input_conditions Integer patterns representing denormal f32 values
 * @expected_behavior Returns valid denormal f32 values with correct properties
 * @validation_method Verify denormal properties and value relationships
 */
TEST_P(F32ReinterpretI32Test, DenormalValues_SubnormalPatterns_ProducesCorrectFloat) {
    // Test various denormal patterns (exponent = 0, mantissa != 0)

    // Smallest positive denormal
    float result = call_f32_reinterpret_i32(0x00000001);
    ASSERT_GT(result, 0.0f) << "Smallest denormal should be positive";
    ASSERT_LT(result, FLT_MIN) << "Should be smaller than FLT_MIN";

    // Some denormal in the middle
    result = call_f32_reinterpret_i32(0x00400000);
    ASSERT_GT(result, 0.0f) << "Mid-range denormal should be positive";
    ASSERT_LT(result, FLT_MIN) << "Should be smaller than FLT_MIN";

    // Largest positive denormal
    result = call_f32_reinterpret_i32(0x007FFFFF);
    ASSERT_GT(result, 0.0f) << "Largest denormal should be positive";
    ASSERT_LT(result, FLT_MIN) << "Should be smaller than FLT_MIN";

    // Negative denormals
    result = call_f32_reinterpret_i32(0x80000001);
    ASSERT_LT(result, 0.0f) << "Negative denormal should be negative";
    ASSERT_GT(result, -FLT_MIN) << "Should be greater than -FLT_MIN";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, F32ReinterpretI32Test,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<RunningMode>& info) {
                             return info.param == Mode_Interp ? "Interpreter" : "AOT";
                         });

// Initialize paths for test execution
class F32ReinterpretI32TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        char *cwdRet = getcwd(NULL, 0);
        if (cwdRet == NULL) {
            perror("getcwd");
            exit(1);
        }

        CWD = std::string(cwdRet);
        free(cwdRet);

        WASM_FILE = CWD + "/wasm-apps/f32_reinterpret_i32_test.wasm";
        WASM_FILE_STACK_UNDERFLOW = CWD + "/wasm-apps/f32_reinterpret_i32_stack_underflow.wat";
    }
};

// Register test environment
[[maybe_unused]] static ::testing::Environment* const test_env =
    ::testing::AddGlobalTestEnvironment(new F32ReinterpretI32TestEnvironment);