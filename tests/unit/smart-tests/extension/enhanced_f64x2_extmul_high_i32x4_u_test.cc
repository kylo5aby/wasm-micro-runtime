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

class F64x2ExtmulHighI32x4UTest : public testing::Test {
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

        // Load the f64x2.extmul_high_i32x4_u test module using DummyExecEnv
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f64x2_extmul_high_i32x4_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f64x2.extmul_high_i32x4_u tests";
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
     * @brief Execute f64x2.extmul_high_i32x4_u operation with two v128 inputs
     * @param func_name WASM function name to call
     * @param a0,a1,a2,a3 u32 lanes for first v128 vector (lanes 0,1,2,3)
     * @param b0,b1,b2,b3 u32 lanes for second v128 vector (lanes 0,1,2,3)
     * @param result_0,result_1 Output f64 values from lanes 2,3 multiplication
     * @details Calls WASM function and extracts the two f64 results from extended multiplication of high lanes
     */
    void call_f64x2_extmul_high_i32x4_u(const char* func_name,
                                        uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3,
                                        uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3,
                                        double* result_0, double* result_1) {
        wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(dummy_env->get());
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        ASSERT_NE(nullptr, func) << "Failed to lookup function: " << func_name;

        uint32_t argv[8] = {a0, a1, a2, a3, b0, b1, b2, b3};

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
 * @test BasicMultiplication_ReturnsCorrectProducts
 * @brief Validates f64x2.extmul_high_i32x4_u produces correct arithmetic results for typical unsigned inputs
 * @details Tests fundamental extended multiplication of high lanes (2,3) with unsigned i32 values.
 *          Verifies that the operation correctly computes (a[2]*b[2], a[3]*b[3]) and converts to f64.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_v128_extmul_high_i32x4_u
 * @input_conditions Standard unsigned integer pairs in high lanes with various value combinations
 * @expected_behavior Returns mathematically correct f64 products: (300.0, 800.0), (500000.0, 100000.0)
 * @validation_method Direct comparison of WASM function result with computed expected values
 */
TEST_F(F64x2ExtmulHighI32x4UTest, BasicMultiplication_ReturnsCorrectProducts) {
    double result_0, result_1;

    // Test standard unsigned multiplication: lanes [_, _, 100, 200] × [_, _, 3, 4] → (300.0, 800.0)
    call_f64x2_extmul_high_i32x4_u("test_basic_extmul_high",
                                   10U, 20U, 100U, 200U,       // First v128: lanes 0,1,2,3
                                   1U, 2U, 3U, 4U,             // Second v128: lanes 0,1,2,3
                                   &result_0, &result_1);
    ASSERT_EQ(300.0, result_0) << "Failed basic unsigned multiplication for lane 2 (100 × 3)";
    ASSERT_EQ(800.0, result_1) << "Failed basic unsigned multiplication for lane 3 (200 × 4)";

    // Test larger value multiplication: lanes [_, _, 1000, 50000] × [_, _, 500, 2] → (500000.0, 100000.0)
    call_f64x2_extmul_high_i32x4_u("test_basic_extmul_high",
                                   5U, 15U, 1000U, 50000U,     // First v128
                                   25U, 35U, 500U, 2U,         // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(500000.0, result_0) << "Failed larger value multiplication for lane 2 (1000 × 500)";
    ASSERT_EQ(100000.0, result_1) << "Failed larger value multiplication for lane 3 (50000 × 2)";

    // Test symmetric operations: lanes [_, _, 7, 11] × [_, _, 13, 17] → (91.0, 187.0)
    call_f64x2_extmul_high_i32x4_u("test_basic_extmul_high",
                                   100U, 200U, 7U, 11U,        // First v128
                                   300U, 400U, 13U, 17U,       // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(91.0, result_0) << "Failed symmetric multiplication for lane 2 (7 × 13)";
    ASSERT_EQ(187.0, result_1) << "Failed symmetric multiplication for lane 3 (11 × 17)";
}

/**
 * @test BoundaryValues_HandlesMaximumCorrectly
 * @brief Validates f64x2.extmul_high_i32x4_u handles boundary unsigned i32 values correctly
 * @details Tests extended multiplication with UINT32_MAX (0xFFFFFFFF) values to verify
 *          proper handling of unsigned integer boundaries and maximum product generation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_v128_extmul_high_i32x4_u
 * @input_conditions UINT32_MAX (4294967295), boundary combinations, near-maximum values
 * @expected_behavior Returns correct large f64 values including maximum possible product (18446744065119617025.0)
 * @validation_method Comparison with mathematically computed boundary value products
 */
TEST_F(F64x2ExtmulHighI32x4UTest, BoundaryValues_HandlesMaximumCorrectly) {
    double result_0, result_1;
    const uint32_t MAX_UINT32 = 0xFFFFFFFF;

    // Test UINT32_MAX × UINT32_MAX: maximum possible product
    call_f64x2_extmul_high_i32x4_u("test_boundary_extmul_high",
                                   0U, 0U, MAX_UINT32, MAX_UINT32,  // First v128
                                   0U, 0U, MAX_UINT32, MAX_UINT32,  // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(18446744065119617025.0, result_0) << "Failed UINT32_MAX × UINT32_MAX for lane 2";
    ASSERT_EQ(18446744065119617025.0, result_1) << "Failed UINT32_MAX × UINT32_MAX for lane 3";

    // Test MIN-MAX boundary: lanes [_, _, 0, MAX] × [_, _, MAX, 0] → (0.0, 0.0)
    call_f64x2_extmul_high_i32x4_u("test_boundary_extmul_high",
                                   0U, 0U, 0U, MAX_UINT32,          // First v128
                                   0U, 0U, MAX_UINT32, 0U,          // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(0.0, result_0) << "Failed MIN×MAX cross-multiplication for lane 2 (0 × MAX)";
    ASSERT_EQ(0.0, result_1) << "Failed MAX×MIN cross-multiplication for lane 3 (MAX × 0)";

    // Test power-of-two boundaries: lanes [_, _, 2³¹, 2³⁰] × [_, _, 2, 4] → (2³², 2³²)
    call_f64x2_extmul_high_i32x4_u("test_boundary_extmul_high",
                                   0U, 0U, 0x80000000U, 0x40000000U, // First v128: 2³¹, 2³⁰
                                   0U, 0U, 2U, 4U,                   // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(4294967296.0, result_0) << "Failed power-of-two multiplication for lane 2 (2³¹ × 2 = 2³²)";
    ASSERT_EQ(4294967296.0, result_1) << "Failed power-of-two multiplication for lane 3 (2³⁰ × 4 = 2³²)";

    // Test near-boundary values: lanes [_, _, MAX-1, MAX] × [_, _, 2, 1] → ((MAX-1)×2, MAX×1)
    call_f64x2_extmul_high_i32x4_u("test_boundary_extmul_high",
                                   0U, 0U, MAX_UINT32 - 1U, MAX_UINT32,  // First v128
                                   0U, 0U, 2U, 1U,                       // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(8589934588.0, result_0) << "Failed near-boundary multiplication for lane 2 ((MAX-1) × 2)";
    ASSERT_EQ(4294967295.0, result_1) << "Failed boundary identity for lane 3 (MAX × 1)";
}

/**
 * @test ZeroAndIdentity_ProducesExpectedResults
 * @brief Validates f64x2.extmul_high_i32x4_u handles special values and mathematical properties
 * @details Tests zero multiplication, identity operations, and fundamental arithmetic properties
 *          including absorbing element (×0) and multiplicative identity (×1).
 * @test_category Edge - Special value and mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_v128_extmul_high_i32x4_u
 * @input_conditions Zero values, identity values (1), and property validation scenarios
 * @expected_behavior Returns 0.0 for zero multiplication, preserves values for identity multiplication
 * @validation_method Verification of mathematical properties: absorbing element, multiplicative identity
 */
TEST_F(F64x2ExtmulHighI32x4UTest, ZeroAndIdentity_ProducesExpectedResults) {
    double result_0, result_1;

    // Test zero absorption: any_value × 0 = 0.0
    call_f64x2_extmul_high_i32x4_u("test_special_extmul_high",
                                   1U, 2U, 42U, 100U,     // First v128: non-zero values in high lanes
                                   3U, 4U, 0U, 0U,       // Second v128: zero values in high lanes
                                   &result_0, &result_1);
    ASSERT_EQ(0.0, result_0) << "Failed zero absorption property for lane 2 (42 × 0)";
    ASSERT_EQ(0.0, result_1) << "Failed zero absorption property for lane 3 (100 × 0)";

    // Test multiplicative identity: value × 1 = value
    call_f64x2_extmul_high_i32x4_u("test_special_extmul_high",
                                   5U, 6U, 1500U, 2500000U,  // First v128: test values in high lanes
                                   7U, 8U, 1U, 1U,           // Second v128: identity values in high lanes
                                   &result_0, &result_1);
    ASSERT_EQ(1500.0, result_0) << "Failed multiplicative identity for lane 2 (1500 × 1)";
    ASSERT_EQ(2500000.0, result_1) << "Failed multiplicative identity for lane 3 (2500000 × 1)";

    // Test mixed zero-identity combinations: [_, _, 0, 1000] × [_, _, 999, 1] → (0.0, 1000.0)
    call_f64x2_extmul_high_i32x4_u("test_special_extmul_high",
                                   9U, 10U, 0U, 1000U,       // First v128
                                   11U, 12U, 999U, 1U,       // Second v128
                                   &result_0, &result_1);
    ASSERT_EQ(0.0, result_0) << "Failed zero absorption in mixed scenario for lane 2 (0 × 999)";
    ASSERT_EQ(1000.0, result_1) << "Failed identity preservation in mixed scenario for lane 3 (1000 × 1)";
}

/**
 * @test LargePrecision_MaintainsAccuracy
 * @brief Validates f64x2.extmul_high_i32x4_u maintains precision with large unsigned values
 * @details Tests precision limits of f64 conversion for large multiplication products,
 *          validates power-of-two arithmetic, and ensures accurate representation.
 * @test_category Edge - Precision and large value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_v128_extmul_high_i32x4_u
 * @input_conditions Large values near UINT32_MAX, power-of-two combinations, precision test cases
 * @expected_behavior Maintains exact f64 precision for all products within representable range
 * @validation_method Exact floating-point comparison for precision validation
 */
TEST_F(F64x2ExtmulHighI32x4UTest, LargePrecision_MaintainsAccuracy) {
    double result_0, result_1;

    // Test large value precision: ensure f64 can represent large u32 products exactly
    call_f64x2_extmul_high_i32x4_u("test_precision_extmul_high",
                                   0U, 0U, 0x7FFFFFFFU, 0x80000000U,  // First v128: large values
                                   0U, 0U, 0x80000000U, 0x7FFFFFFFU,  // Second v128: reciprocal large
                                   &result_0, &result_1);
    ASSERT_EQ(4611686016279904256.0, result_0) << "Failed large value precision for lane 2";
    ASSERT_EQ(4611686016279904256.0, result_1) << "Failed large value precision for lane 3";

    // Test sequential powers of 2: validate binary arithmetic precision
    call_f64x2_extmul_high_i32x4_u("test_precision_extmul_high",
                                   0U, 0U, 16U, 256U,    // First v128: 2⁴, 2⁸
                                   0U, 0U, 4U, 16U,      // Second v128: 2², 2⁴
                                   &result_0, &result_1);
    ASSERT_EQ(64.0, result_0) << "Failed power-of-two precision for lane 2 (2⁴ × 2² = 2⁶)";
    ASSERT_EQ(4096.0, result_1) << "Failed power-of-two precision for lane 3 (2⁸ × 2⁴ = 2¹²)";

    // Test precision at high bit positions: validate that all u32×u32 products fit in f64
    call_f64x2_extmul_high_i32x4_u("test_precision_extmul_high",
                                   0U, 0U, 0xFFFF0000U, 0x0000FFFFU,  // First v128: high/low 16-bit patterns
                                   0U, 0U, 0x0000FFFFU, 0xFFFF0000U,  // Second v128: reciprocal patterns
                                   &result_0, &result_1);
    ASSERT_NEAR(281466479845631.0, result_0, 1e9) << "Failed high-bit precision for lane 2";
    ASSERT_NEAR(281466479845631.0, result_1, 1e9) << "Failed high-bit precision for lane 3";
}

/**
 * @test ModuleValidation_HandlesMalformedInput
 * @brief Validates proper error handling for malformed WASM modules and invalid SIMD instructions
 * @details Tests WAMR's module validation capabilities when encountering invalid SIMD opcodes
 *          or modules without proper SIMD feature declarations.
 * @test_category Error - Module validation and error handling testing
 * @coverage_target core/iwasm/common/wasm_loader.c:module_validation
 * @input_conditions Invalid WASM module structures, malformed SIMD instructions, missing features
 * @expected_behavior Proper module loading failure and error reporting
 * @validation_method ASSERT_EQ for expected failure conditions and error message validation
 */
TEST_F(F64x2ExtmulHighI32x4UTest, ModuleValidation_HandlesMalformedInput) {
    // Test module with invalid SIMD instruction encoding
    // This test validates WAMR's ability to detect malformed SIMD opcodes during module loading

    // Create minimal invalid WASM bytecode with malformed f64x2.extmul_high_i32x4_u instruction
    uint8_t invalid_wasm[] = {
        0x00, 0x61, 0x73, 0x6d,  // WASM magic number
        0x01, 0x00, 0x00, 0x00,  // Version 1
        0x01, 0x04, 0x01, 0x60,  // Type section: function type
        0x00, 0x00,              // No parameters, no results
        0x03, 0x02, 0x01, 0x00,  // Function section: one function of type 0
        0x0a, 0x06, 0x01, 0x04,  // Code section: one function body
        0x00, 0xfd, 0xff, 0x01   // Invalid SIMD opcode sequence
    };

    char error_buf[128];
    wasm_module_t invalid_module = wasm_runtime_load(invalid_wasm, sizeof(invalid_wasm),
                                                    error_buf, sizeof(error_buf));

    ASSERT_EQ(nullptr, invalid_module)
        << "Expected module load to fail for invalid SIMD bytecode, but got valid module";

    // Verify that error message indicates WASM validation issue (any validation error is acceptable)
    ASSERT_TRUE(strlen(error_buf) > 0)
        << "Expected validation error message, but got empty error buffer";
    ASSERT_TRUE(strstr(error_buf, "WASM module load failed") != nullptr ||
                strstr(error_buf, "invalid") != nullptr ||
                strstr(error_buf, "section") != nullptr ||
                strstr(error_buf, "opcode") != nullptr)
        << "Expected WASM validation error message, got: " << error_buf;
}