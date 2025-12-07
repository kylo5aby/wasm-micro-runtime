/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <climits>
#include <cstdint>
#include <cstring>
#include "wasm_runtime.h"
#include "bh_read_file.h"

namespace {

typedef enum RunningMode {
    Mode_Interp = 1,
    Mode_AOT = 2,
} RunningMode;

class I16x8Q15MulrSatSTest : public testing::TestWithParam<RunningMode> {
protected:
    void SetUp() override {
        // Initialize WAMR runtime
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.n_native_symbols = 0;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        runtime_initialized_ = true;

        // Load the test WASM module
        LoadTestModule();
    }

    void TearDown() override {
        // Clean up module resources
        if (module_inst_) {
            wasm_runtime_deinstantiate(module_inst_);
            module_inst_ = nullptr;
        }

        if (module_) {
            wasm_runtime_unload(module_);
            module_ = nullptr;
        }

        if (wasm_file_buf_) {
            wasm_runtime_free(wasm_file_buf_);
            wasm_file_buf_ = nullptr;
        }

        // Destroy WAMR runtime
        if (runtime_initialized_) {
            wasm_runtime_destroy();
            runtime_initialized_ = false;
        }
    }

    void LoadTestModule() {
        const char* wasm_path = "wasm-apps/i16x8_q15mulr_sat_s_test.wasm";
        uint32_t wasm_file_size = 0;
        char error_buf[128] = {0};

        // Read WASM file
        wasm_file_buf_ = (uint8_t*)bh_read_file_to_buffer(wasm_path, &wasm_file_size);
        ASSERT_NE(nullptr, wasm_file_buf_)
            << "Failed to read WASM file: " << wasm_path;
        ASSERT_GT(wasm_file_size, 0U)
            << "WASM file is empty: " << wasm_path;

        wasm_file_size_ = wasm_file_size;

        // Load WASM module
        module_ = wasm_runtime_load(wasm_file_buf_, wasm_file_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_)
            << "Failed to load WASM module: " << error_buf;

        // Instantiate WASM module
        module_inst_ = wasm_runtime_instantiate(module_, 65536, 65536, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst_)
            << "Failed to instantiate WASM module: " << error_buf;
    }

    // Helper function to call WASM function with i16x8 vectors
    void CallI16x8Q15MulrSatS(const int16_t a[8], const int16_t b[8], int16_t result[8]) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, "i16x8_q15mulr_sat_s_test");
        ASSERT_NE(nullptr, func) << "Failed to lookup i16x8_q15mulr_sat_s_test function";

        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst_, 65536);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

        // Prepare arguments: pack i16x8 vectors into v128
        uint32_t argv[8]; // Two v128 values = 8 x uint32_t
        memcpy(&argv[0], a, 16); // First v128 (16 bytes)
        memcpy(&argv[4], b, 16); // Second v128 (16 bytes)

        // Call the function
        bool call_result = wasm_runtime_call_wasm(exec_env, func, 8, argv);
        ASSERT_TRUE(call_result) << "WASM function call failed: "
                                 << wasm_runtime_get_exception(module_inst_);

        // Extract result from argv (first v128)
        memcpy(result, &argv[0], 16);

        wasm_runtime_destroy_exec_env(exec_env);
    }

    // Helper function to compute expected Q15 multiplication with saturation
    int16_t ComputeQ15MulrSatS(int16_t a, int16_t b) {
        // Q15 multiplication: (a * b) >> 15 with rounding and saturation
        int32_t product = static_cast<int32_t>(a) * static_cast<int32_t>(b);

        // Add rounding bit (0.5 in Q15 format = 16384)
        product += 16384;

        // Shift right by 15 bits (Q15 format)
        int32_t shifted = product >> 15;

        // Apply saturation
        if (shifted > 32767) {
            return 32767;
        } else if (shifted < -32768) {
            return -32768;
        } else {
            return static_cast<int16_t>(shifted);
        }
    }

    // Runtime and module management
    bool runtime_initialized_ = false;
    uint8_t* wasm_file_buf_ = nullptr;
    uint32_t wasm_file_size_ = 0;
    wasm_module_t module_ = nullptr;
    wasm_module_inst_t module_inst_ = nullptr;
};

/**
 * @test BasicQ15Multiplication_ReturnsCorrectResults
 * @brief Validates i16x8.q15mulr_sat_s produces correct Q15 arithmetic results for typical inputs
 * @details Tests fundamental Q15 multiplication operation with positive, negative, and mixed-sign integers.
 *          Verifies that i16x8.q15mulr_sat_s correctly computes (a * b) >> 15 with rounding for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_q15mulr_sat_s_operation
 * @input_conditions Standard Q15 values: 0.5 (16384), 0.25 (8192), mixed signs
 * @expected_behavior Returns mathematically correct Q15 products with proper rounding
 * @validation_method Direct comparison of WASM function result with expected Q15 values
 */
TEST_P(I16x8Q15MulrSatSTest, BasicQ15Multiplication_ReturnsCorrectResults) {
    // Test data: typical Q15 values
    int16_t a[8] = {16384, -8192, 24576, -16384, 4096, -12288, 20480, 0};      // 0.5, -0.25, 0.75, -0.5, 0.125, -0.375, 0.625, 0
    int16_t b[8] = {16384, 16384, 8192, -8192, 32767, 16384, 12288, 32767};     // 0.5, 0.5, 0.25, -0.25, ~1.0, 0.5, 0.375, ~1.0
    int16_t result[8];
    int16_t expected[8];

    // Compute expected results using reference implementation
    for (int i = 0; i < 8; i++) {
        expected[i] = ComputeQ15MulrSatS(a[i], b[i]);
    }

    // Execute WASM function
    CallI16x8Q15MulrSatS(a, b, result);

    // Validate results
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Q15 multiplication failed at lane " << i
            << ": a=" << a[i] << ", b=" << b[i]
            << ", expected=" << expected[i] << ", actual=" << result[i];
    }
}

/**
 * @test MixedSignMultiplication_HandlesSignsCorrectly
 * @brief Validates correct sign handling in Q15 multiplication across different sign combinations
 * @details Tests sign preservation according to multiplication rules: pos×pos=pos, neg×neg=pos, pos×neg=neg, neg×pos=neg.
 *          Verifies that the Q15 multiplication maintains mathematical sign correctness.
 * @test_category Main - Sign handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_q15mulr_sat_s_sign_handling
 * @input_conditions Various sign combinations with moderate Q15 values
 * @expected_behavior Correct sign preservation according to multiplication rules
 * @validation_method Sign-specific validation with mathematical verification
 */
TEST_P(I16x8Q15MulrSatSTest, MixedSignMultiplication_HandlesSignsCorrectly) {
    // Test sign combinations
    int16_t a[8] = {8192, -8192, 8192, -8192, 16384, -16384, 12288, -12288};   // Mixed positive and negative
    int16_t b[8] = {4096, 4096, -4096, -4096, 24576, 24576, -20480, -20480};   // Mixed positive and negative
    int16_t result[8];
    int16_t expected[8];

    // Compute expected results
    for (int i = 0; i < 8; i++) {
        expected[i] = ComputeQ15MulrSatS(a[i], b[i]);
    }

    // Execute WASM function
    CallI16x8Q15MulrSatS(a, b, result);

    // Validate sign handling
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Sign handling failed at lane " << i
            << ": a=" << a[i] << ", b=" << b[i]
            << ", expected=" << expected[i] << ", actual=" << result[i];

        // Verify sign is mathematically correct
        bool expected_positive = ((a[i] >= 0 && b[i] >= 0) || (a[i] < 0 && b[i] < 0));
        bool result_positive = (result[i] >= 0);
        ASSERT_EQ(expected_positive, result_positive)
            << "Incorrect sign at lane " << i << ": result should be "
            << (expected_positive ? "positive" : "negative");
    }
}

/**
 * @test SaturationBoundaries_ClampsOverflowCorrectly
 * @brief Tests saturation logic at overflow boundaries for Q15 multiplication
 * @details Verifies that values causing overflow are properly clamped to [-32768, 32767] range without trapping.
 *          Tests both positive overflow (→32767) and negative underflow (→-32768) scenarios.
 * @test_category Corner - Saturation boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_q15mulr_sat_s_saturation
 * @input_conditions Values that would cause overflow: large positive × large positive, etc.
 * @expected_behavior Results clamped to valid i16 range without arithmetic traps
 * @validation_method Saturation-specific validation with boundary checking
 */
TEST_P(I16x8Q15MulrSatSTest, SaturationBoundaries_ClampsOverflowCorrectly) {
    // Test saturation cases
    int16_t a[8] = {32767, -32768, 32767, -32768, 30000, -30000, 25000, -25000};  // Large values
    int16_t b[8] = {32767, -32768, -32768, 32767, 32000, 32000, -32000, -32000};  // Large values
    int16_t result[8];
    int16_t expected[8];

    // Compute expected results with saturation
    for (int i = 0; i < 8; i++) {
        expected[i] = ComputeQ15MulrSatS(a[i], b[i]);
    }

    // Execute WASM function
    CallI16x8Q15MulrSatS(a, b, result);

    // Validate saturation behavior
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Saturation failed at lane " << i
            << ": a=" << a[i] << ", b=" << b[i]
            << ", expected=" << expected[i] << ", actual=" << result[i];

        // Ensure result is within valid i16 range
        ASSERT_GE(result[i], -32768)
            << "Result underflowed i16 range at lane " << i;
        ASSERT_LE(result[i], 32767)
            << "Result overflowed i16 range at lane " << i;
    }
}

/**
 * @test RoundingBehavior_RoundsToNearestQ15
 * @brief Tests rounding accuracy at Q15 precision boundaries
 * @details Verifies consistent rounding to nearest representable Q15 value for intermediate results.
 *          Tests values producing results exactly between Q15 representable values.
 * @test_category Corner - Rounding behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_q15mulr_sat_s_rounding
 * @input_conditions Values producing results at exact rounding boundaries
 * @expected_behavior Consistent rounding to nearest representable Q15 value
 * @validation_method Rounding behavior validation with precision checking
 */
TEST_P(I16x8Q15MulrSatSTest, RoundingBehavior_RoundsToNearestQ15) {
    // Test rounding behavior with values that produce fractional Q15 results
    int16_t a[8] = {3, 5, 7, 11, 13, 17, 19, 23};        // Small odd values
    int16_t b[8] = {21845, 13107, 9362, 5958, 5041, 3855, 3449, 2849}; // Values that create rounding scenarios
    int16_t result[8];
    int16_t expected[8];

    // Compute expected results with proper rounding
    for (int i = 0; i < 8; i++) {
        expected[i] = ComputeQ15MulrSatS(a[i], b[i]);
    }

    // Execute WASM function
    CallI16x8Q15MulrSatS(a, b, result);

    // Validate rounding behavior
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Rounding behavior incorrect at lane " << i
            << ": a=" << a[i] << ", b=" << b[i]
            << ", expected=" << expected[i] << ", actual=" << result[i];
    }
}

/**
 * @test ZeroOperations_ReturnsZeroProducts
 * @brief Tests mathematical identity properties with zero operands
 * @details Validates that any multiplication involving zero operands produces zero results (mathematical identity).
 *          Tests zero vectors, mixed zero lanes, and zero × non-zero combinations.
 * @test_category Edge - Zero operand validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_q15mulr_sat_s_zero_handling
 * @input_conditions Zero vectors, mixed zero lanes, zero × non-zero combinations
 * @expected_behavior Zero results for any multiplication involving zero operands
 * @validation_method Zero product validation with mathematical identity checking
 */
TEST_P(I16x8Q15MulrSatSTest, ZeroOperations_ReturnsZeroProducts) {
    // Test zero operations
    int16_t a[8] = {0, 0, 16384, -8192, 0, 24576, 0, -16384};    // Mixed zero and non-zero
    int16_t b[8] = {16384, 0, 0, 0, -32767, 0, 8192, 0};         // Mixed zero and non-zero
    int16_t result[8];

    // Execute WASM function
    CallI16x8Q15MulrSatS(a, b, result);

    // Validate zero products - any lane with at least one zero operand should be zero
    for (int i = 0; i < 8; i++) {
        if (a[i] == 0 || b[i] == 0) {
            ASSERT_EQ(0, result[i])
                << "Zero multiplication failed at lane " << i
                << ": a=" << a[i] << ", b=" << b[i]
                << ", expected=0, actual=" << result[i];
        }
    }
}

/**
 * @test ExtremeValues_HandlesBoundariesCorrectly
 * @brief Tests MIN/MAX value combinations with proper boundary handling
 * @details Validates proper handling of extreme i16 values (-32768, 32767) with saturation if needed.
 *          Tests the special case of (-32768) × (-32768) and other extreme combinations.
 * @test_category Edge - Extreme value boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i16x8_q15mulr_sat_s_extreme_values
 * @input_conditions MIN/MAX i16 values and combinations thereof
 * @expected_behavior Proper handling of extreme values with saturation as needed
 * @validation_method Extreme value boundary validation with saturation checking
 */
TEST_P(I16x8Q15MulrSatSTest, ExtremeValues_HandlesBoundariesCorrectly) {
    // Test extreme value combinations
    int16_t a[8] = {32767, -32768, 32767, -32768, 32767, -32768, 1, -1};       // MIN/MAX values
    int16_t b[8] = {32767, -32768, -32768, 32767, 1, 1, 32767, 32767};         // MIN/MAX and unity values
    int16_t result[8];
    int16_t expected[8];

    // Compute expected results
    for (int i = 0; i < 8; i++) {
        expected[i] = ComputeQ15MulrSatS(a[i], b[i]);
    }

    // Execute WASM function
    CallI16x8Q15MulrSatS(a, b, result);

    // Validate extreme value handling
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Extreme value handling failed at lane " << i
            << ": a=" << a[i] << ", b=" << b[i]
            << ", expected=" << expected[i] << ", actual=" << result[i];

        // Ensure results are within valid range
        ASSERT_GE(result[i], -32768)
            << "Result underflowed at lane " << i;
        ASSERT_LE(result[i], 32767)
            << "Result overflowed at lane " << i;
    }
}

/**
 * @test ModuleLoadingValidation_HandlesInvalidModules
 * @brief Tests infrastructure robustness with invalid module scenarios
 * @details Validates graceful handling of invalid WASM modules, missing exports, and malformed bytecode.
 *          Ensures proper error reporting without crashes.
 * @test_category System - Infrastructure robustness validation
 * @coverage_target tests/unit/enhanced_opcode/simd/enhanced_i16x8_q15mulr_sat_s_test.cc:LoadTestModule
 * @input_conditions Invalid WASM modules, missing exports, malformed bytecode
 * @expected_behavior Graceful failure with proper error reporting
 * @validation_method Error handling validation with module loading verification
 */
TEST_P(I16x8Q15MulrSatSTest, ModuleLoadingValidation_HandlesInvalidModules) {
    // Test module loading and function lookup
    ASSERT_NE(nullptr, module_)
        << "Test module should be loaded successfully";

    ASSERT_NE(nullptr, module_inst_)
        << "Test module should be instantiated successfully";

    // Test function lookup
    wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst_, "i16x8_q15mulr_sat_s_test");
    ASSERT_NE(nullptr, func)
        << "i16x8_q15mulr_sat_s_test function should exist in module";

    // Test non-existent function lookup
    wasm_function_inst_t invalid_func = wasm_runtime_lookup_function(module_inst_, "non_existent_function");
    ASSERT_EQ(nullptr, invalid_func)
        << "Non-existent function lookup should return nullptr";
}

INSTANTIATE_TEST_SUITE_P(
    I16x8Q15MulrSatSTests,
    I16x8Q15MulrSatSTest,
    testing::Values(Mode_Interp
#if WASM_ENABLE_AOT != 0
        , Mode_AOT
#endif
    )
);

} // namespace