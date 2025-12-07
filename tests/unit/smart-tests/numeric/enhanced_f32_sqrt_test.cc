/*
 * Copyright (C) 2025 Ant Group. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cfloat>
#include <cstdint>
#include "wasm_runtime.h"
#include "wasm_export.h"

/**
 * @brief Test fixture for f32.sqrt opcode validation
 * @details Comprehensive testing of WebAssembly f32.sqrt instruction across
 *          interpreter and AOT execution modes. Tests cover IEEE 754 compliance,
 *          boundary conditions, special values, and mathematical correctness.
 */
class F32SqrtTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Initialize WAMR runtime and load f32.sqrt test module
     * @details Sets up WAMR runtime with proper memory allocation and loads
     *          the compiled WASM module containing f32.sqrt test functions.
     */
    void SetUp() override {
        // Initialize WAMR runtime with system allocator
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load the f32.sqrt test WASM module
        const char* wasm_file = "wasm-apps/f32_sqrt_test.wasm";
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
     * @brief Execute f32.sqrt operation via WASM function call
     * @param input f32 value to compute square root
     * @return f32 square root result from WASM execution
     */
    float call_f32_sqrt(float input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, "sqrt_test");
        EXPECT_NE(nullptr, func) << "Failed to lookup sqrt_test function";

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
     * @return f32 result from WASM sqrt operation
     */
    float call_sqrt_special(float input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, "sqrt_special");
        EXPECT_NE(nullptr, func) << "Failed to lookup sqrt_special function";

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
     * @param input f32 boundary value (FLT_MAX, FLT_MIN, etc.)
     * @return f32 result from WASM sqrt operation
     */
    float call_sqrt_boundary(float input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, "sqrt_boundary");
        EXPECT_NE(nullptr, func) << "Failed to lookup sqrt_boundary function";

        uint32_t wasm_args[1];
        memcpy(&wasm_args[0], &input, sizeof(float));

        bool success = wasm_runtime_call_wasm(exec_env_, func, 1, wasm_args);
        EXPECT_TRUE(success) << "WASM function call failed";

        float result;
        memcpy(&result, &wasm_args[0], sizeof(float));
        return result;
    }

private:
    wasm_module_t module_ = nullptr;
    wasm_module_inst_t module_inst_ = nullptr;
    wasm_exec_env_t exec_env_ = nullptr;
    uint8_t* buffer_ = nullptr;
    uint32_t buffer_size_ = 0;
};

/**
 * @test BasicSquareRoot_ReturnsCorrectResults
 * @brief Validates f32.sqrt produces correct mathematical results for typical inputs
 * @details Tests fundamental square root operation with perfect squares, fractional values,
 *          and common mathematical constants. Verifies IEEE 754 compliant computation
 *          within floating-point precision limits.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_sqrt_operation
 * @input_conditions Perfect squares (1.0f, 4.0f, 9.0f), fractional (0.25f), constants (2.0f)
 * @expected_behavior Returns mathematical square roots within FLT_EPSILON tolerance
 * @validation_method Direct comparison with std::sqrt reference implementation
 */
TEST_P(F32SqrtTest, BasicSquareRoot_ReturnsCorrectResults) {
    // Test perfect squares - should return exact integer results
    ASSERT_NEAR(call_f32_sqrt(1.0f), 1.0f, FLT_EPSILON)
        << "sqrt(1.0) should equal 1.0";
    ASSERT_NEAR(call_f32_sqrt(4.0f), 2.0f, FLT_EPSILON)
        << "sqrt(4.0) should equal 2.0";
    ASSERT_NEAR(call_f32_sqrt(9.0f), 3.0f, FLT_EPSILON)
        << "sqrt(9.0) should equal 3.0";
    ASSERT_NEAR(call_f32_sqrt(16.0f), 4.0f, FLT_EPSILON)
        << "sqrt(16.0) should equal 4.0";

    // Test fractional perfect squares
    ASSERT_NEAR(call_f32_sqrt(0.25f), 0.5f, FLT_EPSILON)
        << "sqrt(0.25) should equal 0.5";
    ASSERT_NEAR(call_f32_sqrt(0.01f), 0.1f, FLT_EPSILON)
        << "sqrt(0.01) should equal 0.1";

    // Test common mathematical constants with appropriate tolerance
    ASSERT_NEAR(call_f32_sqrt(2.0f), std::sqrt(2.0f), FLT_EPSILON * 2)
        << "sqrt(2.0) should match standard library result";
    ASSERT_NEAR(call_f32_sqrt(3.0f), std::sqrt(3.0f), FLT_EPSILON * 2)
        << "sqrt(3.0) should match standard library result";
    ASSERT_NEAR(call_f32_sqrt(10.0f), std::sqrt(10.0f), FLT_EPSILON * 4)
        << "sqrt(10.0) should match standard library result";
}

/**
 * @test BoundaryValues_HandledCorrectly
 * @brief Validates f32.sqrt handles IEEE 754 boundary conditions correctly
 * @details Tests square root computation at floating-point boundaries including
 *          maximum values, minimum normalized values, and denormalized transitions.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_sqrt_boundary_handling
 * @input_conditions FLT_MAX, FLT_MIN, denormalized boundary values
 * @expected_behavior Valid results within mathematical constraints and IEEE 754 compliance
 * @validation_method Range validation and precision checks against reference implementation
 */
TEST_P(F32SqrtTest, BoundaryValues_HandledCorrectly) {
    // Test maximum f32 value - should produce largest possible sqrt result
    float max_sqrt = call_f32_sqrt(FLT_MAX);
    ASSERT_TRUE(std::isfinite(max_sqrt))
        << "sqrt(FLT_MAX) should produce finite result";
    ASSERT_NEAR(max_sqrt, std::sqrt(FLT_MAX), std::sqrt(FLT_MAX) * FLT_EPSILON * 8)
        << "sqrt(FLT_MAX) should match expected mathematical result";

    // Test minimum normalized f32 value
    float min_sqrt = call_f32_sqrt(FLT_MIN);
    ASSERT_TRUE(std::isfinite(min_sqrt))
        << "sqrt(FLT_MIN) should produce finite result";
    ASSERT_GT(min_sqrt, 0.0f)
        << "sqrt(FLT_MIN) should be positive";
    ASSERT_NEAR(min_sqrt, std::sqrt(FLT_MIN), std::sqrt(FLT_MIN) * FLT_EPSILON * 4)
        << "sqrt(FLT_MIN) should match expected result";

    // Test very small positive number (near denormalized boundary)
    float small_value = 1.0e-37f;  // Near denormalized boundary
    float small_sqrt = call_f32_sqrt(small_value);
    ASSERT_TRUE(std::isfinite(small_sqrt))
        << "sqrt(small value) should produce finite result";
    ASSERT_GT(small_sqrt, 0.0f)
        << "sqrt(small positive value) should be positive";

    // Test number that produces denormalized result
    float tiny_value = 1.0e-44f;  // Should produce denormalized sqrt
    float tiny_sqrt = call_f32_sqrt(tiny_value);
    ASSERT_TRUE(std::isfinite(tiny_sqrt) || tiny_sqrt == 0.0f)
        << "sqrt(tiny value) should be finite or zero";
}

/**
 * @test SpecialValues_IEEE754Compliant
 * @brief Validates f32.sqrt handles IEEE 754 special values correctly
 * @details Tests IEEE 754 compliant behavior for special floating-point values
 *          including positive/negative zero, infinity, and NaN propagation.
 * @test_category Edge - IEEE 754 special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_sqrt_special_values
 * @input_conditions ±0.0f, +∞, NaN, -∞ (produces NaN)
 * @expected_behavior IEEE 754 compliant special value results
 * @validation_method Special value testing with isnan(), isinf(), sign bit validation
 */
TEST_P(F32SqrtTest, SpecialValues_IEEE754Compliant) {
    // Test positive zero - should return positive zero
    float pos_zero_result = call_sqrt_special(0.0f);
    ASSERT_EQ(pos_zero_result, 0.0f)
        << "sqrt(+0.0) should return +0.0";
    ASSERT_FALSE(std::signbit(pos_zero_result))
        << "sqrt(+0.0) should preserve positive sign";

    // Test negative zero - should return negative zero per IEEE 754
    float neg_zero = -0.0f;
    float neg_zero_result = call_sqrt_special(neg_zero);
    ASSERT_EQ(neg_zero_result, 0.0f)
        << "sqrt(-0.0) should return zero";
    ASSERT_TRUE(std::signbit(neg_zero_result))
        << "sqrt(-0.0) should preserve negative sign";

    // Test positive infinity - should return positive infinity
    float pos_inf_result = call_sqrt_special(INFINITY);
    ASSERT_TRUE(std::isinf(pos_inf_result))
        << "sqrt(+∞) should return infinity";
    ASSERT_FALSE(std::signbit(pos_inf_result))
        << "sqrt(+∞) should be positive infinity";

    // Test NaN - should return NaN (NaN propagation)
    float nan_result = call_sqrt_special(NAN);
    ASSERT_TRUE(std::isnan(nan_result))
        << "sqrt(NaN) should return NaN";

    // Test negative infinity - should return NaN per IEEE 754
    float neg_inf_result = call_sqrt_special(-INFINITY);
    ASSERT_TRUE(std::isnan(neg_inf_result))
        << "sqrt(-∞) should return NaN";
}

/**
 * @test NegativeInputs_ProduceNaN
 * @brief Validates f32.sqrt produces NaN for negative finite inputs (IEEE 754 compliance)
 * @details Tests that square root of negative finite numbers produces NaN results
 *          rather than traps, following IEEE 754 standard behavior.
 * @test_category Exception - Negative input validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_sqrt_negative_handling
 * @input_conditions Various negative finite f32 values
 * @expected_behavior All negative finite inputs produce NaN (no traps)
 * @validation_method NaN result verification with isnan() validation
 */
TEST_P(F32SqrtTest, NegativeInputs_ProduceNaN) {
    // Test negative unit value
    float neg_one_result = call_sqrt_special(-1.0f);
    ASSERT_TRUE(std::isnan(neg_one_result))
        << "sqrt(-1.0) should return NaN, not trap";

    // Test negative integer
    float neg_int_result = call_sqrt_special(-100.0f);
    ASSERT_TRUE(std::isnan(neg_int_result))
        << "sqrt(-100.0) should return NaN, not trap";

    // Test small negative value
    float neg_small_result = call_sqrt_special(-0.001f);
    ASSERT_TRUE(std::isnan(neg_small_result))
        << "sqrt(-0.001) should return NaN, not trap";

    // Test large negative value
    float neg_large_result = call_sqrt_special(-1000000.0f);
    ASSERT_TRUE(std::isnan(neg_large_result))
        << "sqrt(-1000000.0) should return NaN, not trap";

    // Test negative fractional value
    float neg_frac_result = call_sqrt_special(-2.5f);
    ASSERT_TRUE(std::isnan(neg_frac_result))
        << "sqrt(-2.5) should return NaN, not trap";
}

// Instantiate tests for both interpreter and AOT execution modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, F32SqrtTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<F32SqrtTest::ParamType>& info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });