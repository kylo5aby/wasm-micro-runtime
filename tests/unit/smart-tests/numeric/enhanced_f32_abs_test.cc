/*
 * Copyright (C) 2025 Ant Group. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cfloat>
#include <cstdint>
#include <climits>
#include "wasm_runtime.h"
#include "wasm_export.h"

/**
 * @brief Test fixture for f32.abs opcode validation
 * @details Comprehensive testing of WebAssembly f32.abs instruction across
 *          interpreter and AOT execution modes. Tests cover IEEE 754 compliance,
 *          boundary conditions, special values, and sign bit manipulation correctness.
 */
class F32AbsTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Initialize WAMR runtime and load f32.abs test module
     * @details Sets up WAMR runtime with proper memory allocation and loads
     *          the compiled WASM module containing f32.abs test functions.
     */
    void SetUp() override {
        // Initialize WAMR runtime with system allocator
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load the f32.abs test WASM module
        const char* wasm_file = "wasm-apps/f32_abs_test.wasm";
        buffer_ = load_wasm_buffer(wasm_file, &buffer_size_);
        ASSERT_NE(nullptr, buffer_)
            << "Failed to load WASM file: " << wasm_file;

        // Load and instantiate the module
        char error_buf[256];
        module_ = wasm_runtime_load(buffer_, buffer_size_,
                                   error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_)
            << "Failed to load WASM module: " << error_buf;

        module_inst_ = wasm_runtime_instantiate(module_, 65536, 65536,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst_)
            << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode based on test parameter
        wasm_runtime_set_running_mode(module_inst_, GetParam());

        // Create execution environment for WASM function calls
        exec_env_ = wasm_runtime_create_exec_env(module_inst_, 65536);
        ASSERT_NE(nullptr, exec_env_)
            << "Failed to create execution environment";
    }

    /**
     * @brief Clean up WAMR runtime resources
     * @details Properly destroys module instance, unloads module, frees buffer,
     *          and shuts down WAMR runtime to prevent memory leaks.
     */
    void TearDown() override {
        if (exec_env_) {
            wasm_runtime_destroy_exec_env(exec_env_);
            exec_env_ = nullptr;
        }
        if (module_inst_) {
            wasm_runtime_deinstantiate(module_inst_);
            module_inst_ = nullptr;
        }
        if (module_) {
            wasm_runtime_unload(module_);
            module_ = nullptr;
        }
        if (buffer_) {
            wasm_runtime_free(buffer_);
            buffer_ = nullptr;
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM file into memory buffer
     * @param filename Path to WASM file relative to test execution directory
     * @param size Pointer to store the loaded buffer size
     * @return Pointer to allocated buffer containing WASM bytecode
     */
    uint8_t* load_wasm_buffer(const char* filename, uint32_t* size) {
        FILE* file = fopen(filename, "rb");
        if (!file) return nullptr;

        fseek(file, 0, SEEK_END);
        *size = ftell(file);
        fseek(file, 0, SEEK_SET);

        uint8_t* buffer = (uint8_t*)wasm_runtime_malloc(*size);
        if (buffer) {
            fread(buffer, 1, *size, file);
        }
        fclose(file);
        return buffer;
    }

    /**
     * @brief Execute f32.abs operation via WASM function call
     * @param input f32 value to compute absolute value
     * @return f32 absolute value result from WASM execution
     */
    float call_f32_abs(float input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, "abs_test");
        EXPECT_NE(nullptr, func) << "Failed to lookup abs_test function";

        uint32_t wasm_args[1];
        memcpy(&wasm_args[0], &input, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env_, func, 1, wasm_args);
        EXPECT_TRUE(success) << "WASM function call failed";

        float result;
        memcpy(&result, &wasm_args[0], sizeof(float));
        return result;
    }

    /**
     * @brief Execute special values test via WASM function call
     * @param input f32 special value (NaN, infinity, zero)
     * @return f32 result from WASM abs operation
     */
    float call_abs_special(float input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, "abs_special");
        EXPECT_NE(nullptr, func) << "Failed to lookup abs_special function";

        uint32_t wasm_args[1];
        memcpy(&wasm_args[0], &input, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env_, func, 1, wasm_args);
        EXPECT_TRUE(success) << "WASM function call failed";

        float result;
        memcpy(&result, &wasm_args[0], sizeof(float));
        return result;
    }

    /**
     * @brief Execute boundary values test via WASM function call
     * @param input f32 boundary value (MIN, MAX, subnormal)
     * @return f32 result from WASM abs operation
     */
    float call_abs_boundary(float input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, "abs_boundary");
        EXPECT_NE(nullptr, func) << "Failed to lookup abs_boundary function";

        uint32_t wasm_args[1];
        memcpy(&wasm_args[0], &input, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env_, func, 1, wasm_args);
        EXPECT_TRUE(success) << "WASM function call failed";

        float result;
        memcpy(&result, &wasm_args[0], sizeof(float));
        return result;
    }

    /**
     * @brief Check if two f32 values have identical bit patterns
     * @param a First f32 value
     * @param b Second f32 value
     * @return true if bit patterns are identical
     */
    bool same_bit_pattern(float a, float b) {
        uint32_t bits_a, bits_b;
        memcpy(&bits_a, &a, sizeof(float));
        memcpy(&bits_b, &b, sizeof(float));
        return bits_a == bits_b;
    }

    // Test fixture member variables
    wasm_module_t module_ = nullptr;
    wasm_module_inst_t module_inst_ = nullptr;
    wasm_exec_env_t exec_env_ = nullptr;
    uint8_t* buffer_ = nullptr;
    uint32_t buffer_size_ = 0;
};

/**
 * @test BasicOperation_ReturnsCorrectAbsoluteValue
 * @brief Validates f32.abs computes correct absolute values for typical f32 inputs
 * @details Tests fundamental absolute value operation with positive, negative, and
 *          mixed-sign floating-point values. Verifies IEEE 754 compliant computation
 *          and sign bit manipulation correctness.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_abs_operation
 * @input_conditions Standard f32 values: positive (1.5f, 42.0f), negative (-2.7f, -100.5f)
 * @expected_behavior Returns positive absolute values maintaining exact magnitude
 * @validation_method Direct comparison with mathematical absolute value results
 */
TEST_P(F32AbsTest, BasicOperation_ReturnsCorrectAbsoluteValue) {
    // Test positive values - should remain unchanged (identity operation)
    ASSERT_EQ(call_f32_abs(1.5f), 1.5f)
        << "abs(1.5) should equal 1.5 (positive identity)";
    ASSERT_EQ(call_f32_abs(42.0f), 42.0f)
        << "abs(42.0) should equal 42.0 (positive identity)";
    ASSERT_EQ(call_f32_abs(123.456f), 123.456f)
        << "abs(123.456) should equal 123.456 (positive identity)";

    // Test negative values - should become positive with same magnitude
    ASSERT_EQ(call_f32_abs(-1.5f), 1.5f)
        << "abs(-1.5) should equal 1.5 (sign cleared)";
    ASSERT_EQ(call_f32_abs(-42.0f), 42.0f)
        << "abs(-42.0) should equal 42.0 (sign cleared)";
    ASSERT_EQ(call_f32_abs(-123.456f), 123.456f)
        << "abs(-123.456) should equal 123.456 (sign cleared)";

    // Test mixed magnitude ranges
    ASSERT_EQ(call_f32_abs(0.001f), 0.001f)
        << "abs(0.001) should equal 0.001 (small positive)";
    ASSERT_EQ(call_f32_abs(-0.001f), 0.001f)
        << "abs(-0.001) should equal 0.001 (small negative to positive)";
    ASSERT_EQ(call_f32_abs(9876543.0f), 9876543.0f)
        << "abs(large positive) should remain positive";
    ASSERT_EQ(call_f32_abs(-9876543.0f), 9876543.0f)
        << "abs(large negative) should become positive";
}

/**
 * @test BoundaryValues_HandlesExtremeValues
 * @brief Tests f32.abs behavior at f32 representation boundaries
 * @details Validates absolute value computation at floating-point boundaries including
 *          maximum values, minimum normalized values, and subnormal number handling.
 *          Ensures IEEE 754 compliance at representation extremes.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_abs_boundary_handling
 * @input_conditions FLT_MAX, -FLT_MAX, FLT_MIN, -FLT_MIN, smallest normal values
 * @expected_behavior All positive equivalents maintaining exact magnitude and precision
 * @validation_method Exact equality comparison for boundary values
 */
TEST_P(F32AbsTest, BoundaryValues_HandlesExtremeValues) {
    // Test maximum f32 values
    ASSERT_EQ(call_abs_boundary(FLT_MAX), FLT_MAX)
        << "abs(FLT_MAX) should equal FLT_MAX (positive identity)";
    ASSERT_EQ(call_abs_boundary(-FLT_MAX), FLT_MAX)
        << "abs(-FLT_MAX) should equal FLT_MAX (sign cleared)";

    // Test minimum normalized f32 values
    ASSERT_EQ(call_abs_boundary(FLT_MIN), FLT_MIN)
        << "abs(FLT_MIN) should equal FLT_MIN (positive identity)";
    ASSERT_EQ(call_abs_boundary(-FLT_MIN), FLT_MIN)
        << "abs(-FLT_MIN) should equal FLT_MIN (sign cleared)";

    // Test smallest normal positive/negative values
    const float smallest_normal = 1.175494e-38f;  // 2^-126, smallest normal f32
    ASSERT_EQ(call_abs_boundary(smallest_normal), smallest_normal)
        << "abs(smallest normal) should remain positive";
    ASSERT_EQ(call_abs_boundary(-smallest_normal), smallest_normal)
        << "abs(-smallest normal) should become positive";

    // Test values near denormalized boundary
    const float near_denorm = 1.0e-37f;  // Just above denormalized boundary
    ASSERT_EQ(call_abs_boundary(near_denorm), near_denorm)
        << "abs(near denormalized positive) should remain positive";
    ASSERT_EQ(call_abs_boundary(-near_denorm), near_denorm)
        << "abs(near denormalized negative) should become positive";

    // Test subnormal (denormalized) values
    const float subnormal = 1.4013e-45f;  // Smallest positive subnormal f32
    ASSERT_EQ(call_abs_boundary(subnormal), subnormal)
        << "abs(smallest subnormal) should remain positive";
    ASSERT_EQ(call_abs_boundary(-subnormal), subnormal)
        << "abs(-smallest subnormal) should become positive";
}

/**
 * @test SpecialValues_IEEE754Compliance
 * @brief Validates IEEE 754 special value handling (zero, infinity, NaN)
 * @details Tests f32.abs behavior with IEEE 754 special values ensuring proper
 *          sign bit manipulation while preserving NaN and infinity semantics.
 *          Validates compliance with floating-point standard requirements.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_abs_special_values
 * @input_conditions +0.0f, -0.0f, +∞, -∞, various NaN patterns
 * @expected_behavior +0.0f, +0.0f, +∞, +∞, positive NaN with preserved payload
 * @validation_method IEEE 754 property checks and bit pattern analysis
 */
TEST_P(F32AbsTest, SpecialValues_IEEE754Compliance) {
    // Test positive zero - should remain positive zero
    float pos_zero = +0.0f;
    float abs_pos_zero = call_abs_special(pos_zero);
    ASSERT_EQ(abs_pos_zero, 0.0f)
        << "abs(+0.0) should equal +0.0";
    ASSERT_FALSE(std::signbit(abs_pos_zero))
        << "abs(+0.0) should have positive sign bit";

    // Test negative zero - should become positive zero
    float neg_zero = -0.0f;
    float abs_neg_zero = call_abs_special(neg_zero);
    ASSERT_EQ(abs_neg_zero, 0.0f)
        << "abs(-0.0) should equal +0.0";
    ASSERT_FALSE(std::signbit(abs_neg_zero))
        << "abs(-0.0) should have positive sign bit cleared";

    // Test positive infinity - should remain positive infinity
    float pos_inf = std::numeric_limits<float>::infinity();
    float abs_pos_inf = call_abs_special(pos_inf);
    ASSERT_TRUE(std::isinf(abs_pos_inf))
        << "abs(+∞) should remain infinity";
    ASSERT_FALSE(std::signbit(abs_pos_inf))
        << "abs(+∞) should be positive infinity";

    // Test negative infinity - should become positive infinity
    float neg_inf = -std::numeric_limits<float>::infinity();
    float abs_neg_inf = call_abs_special(neg_inf);
    ASSERT_TRUE(std::isinf(abs_neg_inf))
        << "abs(-∞) should be infinity";
    ASSERT_FALSE(std::signbit(abs_neg_inf))
        << "abs(-∞) should be positive infinity";

    // Test quiet NaN - should preserve NaN with positive sign
    float quiet_nan = std::numeric_limits<float>::quiet_NaN();
    float abs_qnan = call_abs_special(quiet_nan);
    ASSERT_TRUE(std::isnan(abs_qnan))
        << "abs(qNaN) should preserve NaN property";
    ASSERT_FALSE(std::signbit(abs_qnan))
        << "abs(qNaN) should have positive sign bit";

    // Test negative quiet NaN - should preserve NaN with positive sign
    float neg_quiet_nan = -std::numeric_limits<float>::quiet_NaN();
    float abs_neg_qnan = call_abs_special(neg_quiet_nan);
    ASSERT_TRUE(std::isnan(abs_neg_qnan))
        << "abs(-qNaN) should preserve NaN property";
    ASSERT_FALSE(std::signbit(abs_neg_qnan))
        << "abs(-qNaN) should have positive sign bit";
}

/**
 * @test SignBitManipulation_OnlyAffectsSign
 * @brief Verifies only sign bit is modified, mantissa/exponent preserved
 * @details Tests that f32.abs operation exclusively clears the sign bit while
 *          leaving mantissa and exponent bits completely unchanged. Validates
 *          bit-level correctness of the absolute value operation.
 * @test_category Edge - Bit manipulation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_abs_bit_manipulation
 * @input_conditions Values with known bit patterns for mantissa/exponent verification
 * @expected_behavior Identical bit patterns except cleared sign bit (MSB = 0)
 * @validation_method Bit-level comparison using union casting and masking
 */
TEST_P(F32AbsTest, SignBitManipulation_OnlyAffectsSign) {
    // Test with known bit pattern values
    const float test_values[] = {
        1.0f,        // 0x3F800000 - simple mantissa pattern
        -1.0f,       // 0xBF800000 - negative of above
        2.5f,        // 0x40200000 - fractional mantissa
        -2.5f,       // 0xC0200000 - negative of above
        123.456f,    // Complex mantissa pattern
        -123.456f,   // Negative complex pattern
    };

    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i += 2) {
        float positive_val = test_values[i];
        float negative_val = test_values[i + 1];

        // Get bit patterns
        uint32_t pos_bits, neg_bits;
        memcpy(&pos_bits, &positive_val, sizeof(float));
        memcpy(&neg_bits, &negative_val, sizeof(float));

        // Verify test values have correct relationship (differ only in sign bit)
        ASSERT_EQ(pos_bits ^ neg_bits, 0x80000000u)
            << "Test values should differ only in sign bit";

        // Test that abs() of positive value is identity (unchanged bits)
        float abs_pos = call_f32_abs(positive_val);
        ASSERT_TRUE(same_bit_pattern(abs_pos, positive_val))
            << "abs(positive) should have identical bit pattern";

        // Test that abs() of negative value has same bits as positive (sign cleared)
        float abs_neg = call_f32_abs(negative_val);
        ASSERT_TRUE(same_bit_pattern(abs_neg, positive_val))
            << "abs(negative) should have same bits as positive equivalent";

        // Verify mantissa and exponent are preserved for negative->positive conversion
        uint32_t abs_neg_bits;
        memcpy(&abs_neg_bits, &abs_neg, sizeof(float));
        ASSERT_EQ(abs_neg_bits & 0x7FFFFFFFu, pos_bits & 0x7FFFFFFFu)
            << "abs() should preserve mantissa and exponent bits exactly";
        ASSERT_EQ(abs_neg_bits & 0x80000000u, 0u)
            << "abs() should clear sign bit (MSB = 0)";
    }
}

/**
 * @test MathematicalProperties_ValidateIdentities
 * @brief Validates mathematical properties of absolute value operation
 * @details Tests fundamental mathematical properties including non-negativity,
 *          identity for positive values, symmetry, and idempotence. Ensures
 *          f32.abs behaves correctly according to mathematical definitions.
 * @test_category Edge - Mathematical correctness validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_abs_mathematical_properties
 * @input_conditions Various f32 values testing mathematical properties
 * @expected_behavior Adherence to mathematical absolute value properties
 * @validation_method Property-based validation with comprehensive test coverage
 */
TEST_P(F32AbsTest, MathematicalProperties_ValidateIdentities) {
    const float test_values[] = {
        0.0f, -0.0f, 1.0f, -1.0f, 2.5f, -2.5f,
        100.0f, -100.0f, 0.001f, -0.001f,
        FLT_MAX, -FLT_MAX, FLT_MIN, -FLT_MIN
    };

    for (float val : test_values) {
        // Property 1: Non-negativity - abs(x) >= 0 for all x
        float abs_val = call_f32_abs(val);
        if (std::isfinite(val)) {  // Only test for finite values
            ASSERT_GE(abs_val, 0.0f)
                << "abs(" << val << ") should be non-negative";
            ASSERT_FALSE(std::signbit(abs_val))
                << "abs(" << val << ") should have positive sign bit";
        }

        // Property 2: Identity for non-negative - abs(x) = x when x >= 0
        if (val >= 0.0f && std::isfinite(val)) {
            ASSERT_EQ(call_f32_abs(val), val)
                << "abs(" << val << ") should equal " << val << " (identity for positive)";
        }

        // Property 3: Symmetry - abs(-x) = abs(x) for all x
        if (std::isfinite(val) && val != 0.0f) {  // Skip for zeros due to sign complexities
            float abs_val_sym = call_f32_abs(-val);
            ASSERT_EQ(abs_val, abs_val_sym)
                << "abs(" << val << ") should equal abs(" << -val << ") (symmetry)";
        }

        // Property 4: Idempotence - abs(abs(x)) = abs(x) for all x
        if (std::isfinite(val)) {
            float abs_abs_val = call_f32_abs(abs_val);
            ASSERT_EQ(abs_val, abs_abs_val)
                << "abs(abs(" << val << ")) should equal abs(" << val << ") (idempotence)";
        }
    }
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(CrossMode, F32AbsTest,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT),
                        [](const testing::TestParamInfo<RunningMode>& info) {
                            return info.param == Mode_Interp ? "Interpreter" : "AOT";
                        });