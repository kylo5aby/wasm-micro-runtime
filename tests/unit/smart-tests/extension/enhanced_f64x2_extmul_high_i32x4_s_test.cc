/*
 * Copyright (C) 2024 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <cstring>
#include <gtest/gtest.h>
#include <climits>
#include <cfloat>
#include <memory>

#include "wasm_runtime.h"
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

class F64x2ExtmulHighI32x4STest : public testing::Test {
protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Set up the test fixture with WAMR runtime initialization and module loading
     * @details Initializes WAMR with SIMD support, loads the test WASM module, and creates execution environment
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f64x2.extmul_high_i32x4_s test module using DummyExecEnv
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_extmul_high_i32x4_s_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.extmul_high_i32x4_s tests";
    }

    /**
     * @brief Clean up test resources and destroy WAMR runtime components
     * @details Properly deallocates execution environment and WAMR runtime using RAII pattern
     */
    void TearDown() override {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute f64x2.extmul_high_i32x4_s operation with two v128 inputs
     * @param func_name WASM function name to call
     * @param a0,a1,a2,a3 i32 lanes for first v128 vector (lanes 0,1,2,3)
     * @param b0,b1,b2,b3 i32 lanes for second v128 vector (lanes 0,1,2,3)
     * @param result_0,result_1 Output f64 values from lanes 2,3 multiplication
     * @details Calls WASM function and extracts the two f64 results from extended multiplication of high lanes
     */
    void call_f64x2_extmul_high_i32x4_s(const char* func_name,
                                        int32_t a0, int32_t a1, int32_t a2, int32_t a3,
                                        int32_t b0, int32_t b1, int32_t b2, int32_t b3,
                                        double* result_0, double* result_1) {
        wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(dummy_env->get());
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        ASSERT_NE(nullptr, func) << "Failed to lookup function: " << func_name;

        uint32_t argv[8] = {
            static_cast<uint32_t>(a0), static_cast<uint32_t>(a1),
            static_cast<uint32_t>(a2), static_cast<uint32_t>(a3),
            static_cast<uint32_t>(b0), static_cast<uint32_t>(b1),
            static_cast<uint32_t>(b2), static_cast<uint32_t>(b3)
        };

        bool ret = wasm_runtime_call_wasm(dummy_env->get(), func, 8, argv);
        ASSERT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        // Extract f64 results from v128 return value stored in argv[0] and argv[1]
        // The WASM function returns f64x2 as two consecutive f64 values
        uint64_t result_bits_0 = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
        uint64_t result_bits_1 = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];

        memcpy(result_0, &result_bits_0, sizeof(double));
        memcpy(result_1, &result_bits_1, sizeof(double));
    }
};

/**
 * @test BasicExtendedMultiplication_ReturnsCorrectResults
 * @brief Validates f64x2.extmul_high_i32x4_s produces correct arithmetic results for typical inputs
 * @details Tests fundamental extended multiplication of high lanes (2,3) with signed i32 values.
 *          Verifies that the operation correctly computes (a[2]*b[2], a[3]*b[3]) and converts to f64.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_v128_extmul_high_i32x4_s
 * @input_conditions Standard i32 pairs in high lanes with various sign combinations
 * @expected_behavior Returns mathematically correct f64 products: (3000.0, 8000.0), (-2000.0, -3000.0), (1000.0, -1000.0)
 * @validation_method Direct comparison of WASM function result with computed expected values
 */
TEST_F(F64x2ExtmulHighI32x4STest, BasicExtendedMultiplication_ReturnsCorrectResults) {
    double result_0, result_1;

    // Test positive × positive: lanes [_, _, 1000, 2000] × [_, _, 3, 4] → (3000.0, 8000.0)
    call_f64x2_extmul_high_i32x4_s("test_basic_extmul_high",
                                   100, 200, 1000, 2000,    // First v128: lanes 0,1,2,3
                                   1, 2, 3, 4,              // Second v128: lanes 0,1,2,3
                                   &result_0, &result_1);
    ASSERT_EQ(3000.0, result_0) << "Failed positive×positive multiplication for lane 2";
    ASSERT_EQ(8000.0, result_1) << "Failed positive×positive multiplication for lane 3";

    // Test negative × positive: lanes [_, _, -500, 1500] × [_, _, 4, -2] → (-2000.0, -3000.0)
    call_f64x2_extmul_high_i32x4_s("test_basic_extmul_high",
                                   10, 20, -500, 1500,     // First v128
                                   1, 2, 4, -2,            // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(-2000.0, result_0) << "Failed negative×positive multiplication for lane 2";
    ASSERT_EQ(-3000.0, result_1) << "Failed positive×negative multiplication for lane 3";

    // Test negative × negative: lanes [_, _, -100, -200] × [_, _, -10, 5] → (1000.0, -1000.0)
    call_f64x2_extmul_high_i32x4_s("test_basic_extmul_high",
                                   5, 15, -100, -200,      // First v128
                                   1, 3, -10, 5,           // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(1000.0, result_0) << "Failed negative×negative multiplication for lane 2";
    ASSERT_EQ(-1000.0, result_1) << "Failed negative×positive multiplication for lane 3";
}

/**
 * @test BoundaryValues_ProduceCorrectResults
 * @brief Validates f64x2.extmul_high_i32x4_s handles boundary i32 values correctly
 * @details Tests extended multiplication with INT32_MAX, INT32_MIN values to verify
 *          proper handling of signed integer boundaries and large product generation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_v128_extmul_high_i32x4_s
 * @input_conditions INT32_MAX (2147483647), INT32_MIN (-2147483648) in various combinations
 * @expected_behavior Returns correct large f64 values: 4611686014132420609.0, -4611686016279904256.0, 4611686018427387904.0
 * @validation_method Comparison with mathematically computed boundary value products
 */
TEST_F(F64x2ExtmulHighI32x4STest, BoundaryValues_ProduceCorrectResults) {
    double result_0, result_1;

    // Test INT32_MAX × INT32_MAX: lanes [_, _, MAX, 1] × [_, _, MAX, MAX] → (MAX²≈4.61e18, MAX≈2.15e9)
    call_f64x2_extmul_high_i32x4_s("test_boundary_extmul_high",
                                   0, 0, INT32_MAX, 1,        // First v128
                                   0, 0, INT32_MAX, INT32_MAX, // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(4611686014132420609.0, result_0) << "Failed INT32_MAX × INT32_MAX for lane 2";
    ASSERT_EQ(2147483647.0, result_1) << "Failed 1 × INT32_MAX for lane 3";

    // Test INT32_MIN × INT32_MIN: lanes [_, _, MIN, -1] × [_, _, MIN, MIN] → (MIN²≈4.61e18, MIN≈2.15e9)
    call_f64x2_extmul_high_i32x4_s("test_boundary_extmul_high",
                                   0, 0, INT32_MIN, -1,       // First v128
                                   0, 0, INT32_MIN, INT32_MIN, // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(4611686018427387904.0, result_0) << "Failed INT32_MIN × INT32_MIN for lane 2";
    ASSERT_EQ(2147483648.0, result_1) << "Failed (-1) × INT32_MIN for lane 3";

    // Test mixed boundary: lanes [_, _, MAX, MIN] × [_, _, MIN, -1] → (MAX×MIN≈-4.61e18, MIN×(-1)≈2.15e9)
    call_f64x2_extmul_high_i32x4_s("test_boundary_extmul_high",
                                   0, 0, INT32_MAX, INT32_MIN, // First v128
                                   0, 0, INT32_MIN, -1,        // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(-4611686016279904256.0, result_0) << "Failed INT32_MAX × INT32_MIN for lane 2";
    ASSERT_EQ(2147483648.0, result_1) << "Failed INT32_MIN × (-1) for lane 3";
}

/**
 * @test SpecialValues_HandleCorrectly
 * @brief Validates f64x2.extmul_high_i32x4_s handles special values and mathematical properties
 * @details Tests zero multiplication, identity operations, and fundamental arithmetic properties
 *          including absorbing element (×0) and multiplicative identity (×1, ×-1).
 * @test_category Edge - Special value and mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_v128_extmul_high_i32x4_s
 * @input_conditions Zero values, identity values (±1), and property validation scenarios
 * @expected_behavior Returns 0.0 for zero multiplication, preserves/negates values for ±1 multiplication
 * @validation_method Verification of mathematical properties: absorbing element, multiplicative identity
 */
TEST_F(F64x2ExtmulHighI32x4STest, SpecialValues_HandleCorrectly) {
    double result_0, result_1;

    // Test zero absorption: any_value × 0 = 0.0
    call_f64x2_extmul_high_i32x4_s("test_special_extmul_high",
                                   1, 2, 42, -100,    // First v128: non-zero values in high lanes
                                   3, 4, 0, 0,        // Second v128: zero values in high lanes
                                   &result_0, &result_1);
    ASSERT_EQ(0.0, result_0) << "Failed zero absorption property for lane 2 (42 × 0)";
    ASSERT_EQ(0.0, result_1) << "Failed zero absorption property for lane 3 (-100 × 0)";

    // Test multiplicative identity: value × 1 = value, value × (-1) = -value
    call_f64x2_extmul_high_i32x4_s("test_special_extmul_high",
                                   5, 6, 1500, -2500,  // First v128: test values in high lanes
                                   7, 8, 1, -1,        // Second v128: identity values in high lanes
                                   &result_0, &result_1);
    ASSERT_EQ(1500.0, result_0) << "Failed multiplicative identity for lane 2 (1500 × 1)";
    ASSERT_EQ(2500.0, result_1) << "Failed sign negation identity for lane 3 (-2500 × -1)";

    // Test commutativity: a × b = b × a
    double result_0_ab, result_1_ab, result_0_ba, result_1_ba;

    // Calculate a × b
    call_f64x2_extmul_high_i32x4_s("test_special_extmul_high",
                                   9, 10, 123, -456,   // First v128
                                   11, 12, 789, -321,  // Second v128
                                   &result_0_ab, &result_1_ab);

    // Calculate b × a (swapped)
    call_f64x2_extmul_high_i32x4_s("test_special_extmul_high",
                                   11, 12, 789, -321,  // Second v128 (now first)
                                   9, 10, 123, -456,   // First v128 (now second)
                                   &result_0_ba, &result_1_ba);

    ASSERT_EQ(result_0_ab, result_0_ba) << "Failed commutativity for lane 2: 123×789 ≠ 789×123";
    ASSERT_EQ(result_1_ab, result_1_ba) << "Failed commutativity for lane 3: (-456)×(-321) ≠ (-321)×(-456)";
}

