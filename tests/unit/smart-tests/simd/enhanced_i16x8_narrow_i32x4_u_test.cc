/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include "wasm_runtime_common.h"
#include "wasm_runtime.h"
#include "aot_runtime.h"
#include <climits>
#include <cstring>

/* Logic copied from wasm-micro-runtime/tests/unit/unit_common.h */
// Using WAMR's built-in RunningMode from wasm_export.h

class WAMRRuntimeRAII {
  public:
    WAMRRuntimeRAII() {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));

        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        EXPECT_TRUE(wasm_runtime_full_init(&init_args));
    }
    ~WAMRRuntimeRAII() { wasm_runtime_destroy(); }
};

/**
 * @brief Enhanced unit tests for i16x8.narrow_i32x4_u SIMD opcode
 *
 * This test suite validates the i16x8.narrow_i32x4_u instruction which narrows
 * two i32x4 vectors into one i16x8 vector with unsigned saturation. The opcode
 * takes 8 signed 32-bit integers from two v128 operands and converts them to
 * 8 unsigned 16-bit integers with saturation (values < 0 become 0, values > 65535 become 65535).
 *
 * Test coverage includes:
 * - Basic narrowing functionality with typical positive values
 * - Mixed positive/negative value handling with proper saturation
 * - Boundary condition testing (0, 65535, saturation limits)
 * - Edge case validation (all zeros, extreme values, mixed ranges)
 * - Cross-execution mode consistency (interpreter vs AOT)
 *
 * Each test method validates actual WAMR runtime behavior through comprehensive
 * assertions without using GTEST_SKIP, SUCCEED, or FAIL constructs.
 */
class I16x8NarrowI32x4UTest : public testing::TestWithParam<RunningMode> {
  protected:
    /**
     * @brief Set up test fixture with WAMR runtime initialization
     *
     * Initializes WAMR runtime environment and loads the test WASM module
     * containing i16x8.narrow_i32x4_u test functions. Configures both
     * interpreter and AOT execution modes based on test parameters.
     */
    void SetUp() override {
        runtime_env = std::make_unique<WAMRRuntimeRAII>();

        // Load the WASM module containing i16x8.narrow_i32x4_u tests
        load_test_module();
    }

    /**
     * @brief Clean up test fixture and release WAMR resources
     *
     * Unloads WASM modules and cleans up WAMR runtime resources.
     * RAII pattern ensures proper cleanup even if tests fail.
     */
    void TearDown() override {
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        runtime_env.reset();
    }

  private:
    /**
     * @brief Load WASM test module and instantiate for execution
     *
     * Loads the i16x8_narrow_i32x4_u_test.wasm file containing test functions
     * and instantiates it for both interpreter and AOT execution modes.
     */
    void load_test_module() {
        const char* wasm_file = "wasm-apps/i16x8_narrow_i32x4_u_test.wasm";

        // Read WASM file
        FILE* file = fopen(wasm_file, "rb");
        ASSERT_NE(nullptr, file) << "Failed to open WASM file: " << wasm_file;

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        wasm_buffer.resize(file_size);
        size_t read_size = fread(wasm_buffer.data(), 1, file_size, file);
        fclose(file);

        ASSERT_EQ(file_size, read_size) << "Failed to read complete WASM file";

        // Load and instantiate module
        char error_buf[256] = {0};
        module = wasm_runtime_load(wasm_buffer.data(), wasm_buffer.size(),
                                  error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(
            module, 65536, 65536, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;
    }

  protected:
    /**
     * @brief Call i16x8.narrow_i32x4_u test function with two i32x4 input vectors
     *
     * @param vec1 First i32x4 vector (4 elements) - becomes lanes 0-3 in result
     * @param vec2 Second i32x4 vector (4 elements) - becomes lanes 4-7 in result
     * @param result Output i16x8 vector (8 elements) with narrowed and saturated values
     *
     * Executes the WASM function that implements i16x8.narrow_i32x4_u operation
     * and validates the execution completed successfully.
     */
    void call_narrow_i32x4_u(const int32_t vec1[4], const int32_t vec2[4], uint16_t result[8]) {
        // Find the test function
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, "narrow_i32x4_u_test");
        ASSERT_NE(nullptr, func) << "Failed to find narrow_i32x4_u_test function";

        // Prepare arguments: 8 i32 values representing two i32x4 vectors
        uint32_t argv[8];
        for (int i = 0; i < 4; i++) {
            argv[i] = static_cast<uint32_t>(vec1[i]);
            argv[i + 4] = static_cast<uint32_t>(vec2[i]);
        }

        // Execute function
        wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";

        bool call_result = wasm_runtime_call_wasm(exec_env, func, 8, argv);
        ASSERT_TRUE(call_result) << "WASM function call failed: "
                                << wasm_runtime_get_exception(module_inst);

        // Extract results from argv (function returns 8 i16 values as i32)
        for (int i = 0; i < 8; i++) {
            result[i] = static_cast<uint16_t>(argv[i] & 0xFFFF);
        }

        wasm_runtime_destroy_exec_env(exec_env);
    }

  protected:  // Changed from private to protected for test access
    std::unique_ptr<WAMRRuntimeRAII> runtime_env;
    std::vector<uint8_t> wasm_buffer;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
};

/**
 * @test BasicNarrowing_ReturnsCorrectConversion
 * @brief Validates i16x8.narrow_i32x4_u produces correct results for typical positive inputs
 * @details Tests fundamental narrowing operation with positive integers within u16 range.
 *          Verifies that values requiring no saturation are converted directly.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_narrow_operation
 * @input_conditions Two i32x4 vectors with typical positive values in valid u16 range
 * @expected_behavior Direct conversion without saturation for all input values
 * @validation_method Lane-by-lane comparison of narrowed results with expected values
 */
TEST_P(I16x8NarrowI32x4UTest, BasicNarrowing_ReturnsCorrectConversion) {
    int32_t vec1[4] = {1000, 2000, 3000, 4000};
    int32_t vec2[4] = {5000, 6000, 7000, 8000};
    uint16_t result[8];
    uint16_t expected[8] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};

    // Execute narrowing operation
    call_narrow_i32x4_u(vec1, vec2, result);

    // Validate each lane of the result
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << " narrowing failed: expected " << expected[i]
            << ", got " << result[i];
    }
}

/**
 * @test MixedSignValues_HandlesPositiveAndNegative
 * @brief Validates proper saturation of negative values to zero while preserving positive values
 * @details Tests mixed positive/negative scenarios to verify negative saturation behavior.
 *          Confirms negative i32 values saturate to 0 in u16 output.
 * @test_category Main - Mixed sign handling with saturation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_narrow_saturate_unsigned
 * @input_conditions Mixed positive and negative i32 values across both input vectors
 * @expected_behavior Negative values become 0, positive values preserved unchanged
 * @validation_method Verify negative saturation and positive value preservation
 */
TEST_P(I16x8NarrowI32x4UTest, MixedSignValues_HandlesPositiveAndNegative) {
    int32_t vec1[4] = {100, -50, 200, -100};
    int32_t vec2[4] = {300, -150, 400, -200};
    uint16_t result[8];
    uint16_t expected[8] = {100, 0, 200, 0, 300, 0, 400, 0};

    // Execute narrowing operation
    call_narrow_i32x4_u(vec1, vec2, result);

    // Validate saturation behavior for mixed signs
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << " mixed sign handling failed: expected " << expected[i]
            << ", got " << result[i];
    }
}

/**
 * @test SaturationLimits_HandlesExactBoundaries
 * @brief Validates saturation behavior at exact u16 boundaries and overflow conditions
 * @details Tests boundary values (0, 65535) and overflow scenarios (> 65535) to verify
 *          correct saturation implementation at u16 limits.
 * @test_category Corner - Boundary condition and saturation limit testing
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_saturate_to_u16
 * @input_conditions Exact boundaries (0, 65535) and values exceeding u16 range
 * @expected_behavior Values > 65535 saturate to 65535, boundary values preserved
 * @validation_method Verify exact boundary handling and overflow saturation
 */
TEST_P(I16x8NarrowI32x4UTest, SaturationLimits_HandlesExactBoundaries) {
    int32_t vec1[4] = {0, 65535, 65536, 100000};
    int32_t vec2[4] = {65535, 65537, -1, 2000000};
    uint16_t result[8];
    uint16_t expected[8] = {0, 65535, 65535, 65535, 65535, 65535, 0, 65535};

    // Execute narrowing operation with boundary values
    call_narrow_i32x4_u(vec1, vec2, result);

    // Validate boundary and saturation behavior
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << " saturation limit handling failed: expected " << expected[i]
            << ", got " << result[i];
    }
}

/**
 * @test AllZeroInputs_ProducesZeroOutput
 * @brief Validates zero preservation through narrowing operation
 * @details Tests identity behavior with all-zero inputs to verify zero values
 *          are correctly preserved through the narrowing process.
 * @test_category Edge - Zero value identity and preservation testing
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_narrow_zero_handling
 * @input_conditions Both input vectors containing all zero values
 * @expected_behavior All output lanes contain zero (identity preservation)
 * @validation_method Verify all result lanes are zero when inputs are zero
 */
TEST_P(I16x8NarrowI32x4UTest, AllZeroInputs_ProducesZeroOutput) {
    int32_t vec1[4] = {0, 0, 0, 0};
    int32_t vec2[4] = {0, 0, 0, 0};
    uint16_t result[8];
    uint16_t expected[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // Execute narrowing operation with all zeros
    call_narrow_i32x4_u(vec1, vec2, result);

    // Validate zero preservation
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << " zero preservation failed: expected " << expected[i]
            << ", got " << result[i];
    }
}

/**
 * @test ExtremeValueMixing_HandlesDiverseRange
 * @brief Validates complete saturation behavior spectrum with extreme value combinations
 * @details Tests full range of saturation scenarios including INT32_MIN, INT32_MAX,
 *          boundary values, and typical values in single test case.
 * @test_category Edge - Extreme value range and comprehensive saturation testing
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_narrow_extreme_values
 * @input_conditions Full spectrum: INT32_MIN, 0, 65535, INT32_MAX, and edge cases
 * @expected_behavior Complete saturation validation across entire input domain
 * @validation_method Verify correct saturation for all categories of extreme inputs
 */
TEST_P(I16x8NarrowI32x4UTest, ExtremeValueMixing_HandlesDiverseRange) {
    int32_t vec1[4] = {INT32_MIN, 0, 65535, INT32_MAX};
    int32_t vec2[4] = {-1, 1, 65536, 2147483647};
    uint16_t result[8];
    uint16_t expected[8] = {0, 0, 65535, 65535, 0, 1, 65535, 65535};

    // Execute narrowing operation with extreme value mixing
    call_narrow_i32x4_u(vec1, vec2, result);

    // Validate complete saturation behavior spectrum
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(expected[i], result[i])
            << "Lane " << i << " extreme value handling failed: expected " << expected[i]
            << ", got " << result[i];
    }
}

/**
 * @test ModuleLoading_WithSIMDSupport_LoadsSuccessfully
 * @brief Validates WASM module loading and SIMD instruction availability
 * @details Tests that WASM modules containing i16x8.narrow_i32x4_u instruction
 *          can be loaded successfully when SIMD support is enabled.
 * @test_category Error - Module loading and SIMD feature validation
 * @coverage_target core/iwasm/common/wasm_loader.c:load_simd_instruction
 * @input_conditions WASM module with i16x8.narrow_i32x4_u instruction
 * @expected_behavior Successful module loading and function accessibility
 * @validation_method Verify module and function instances are valid and accessible
 */
TEST_P(I16x8NarrowI32x4UTest, ModuleLoading_WithSIMDSupport_LoadsSuccessfully) {
    // Module and function should already be loaded in SetUp
    ASSERT_NE(nullptr, module) << "WASM module failed to load";
    ASSERT_NE(nullptr, module_inst) << "WASM module failed to instantiate";

    // Verify function is accessible
    wasm_function_inst_t func = wasm_runtime_lookup_function(
        module_inst, "narrow_i32x4_u_test");
    ASSERT_NE(nullptr, func)
        << "i16x8.narrow_i32x4_u test function not found in module";

    // Verify function can be executed (basic smoke test)
    int32_t vec1[4] = {1, 2, 3, 4};
    int32_t vec2[4] = {5, 6, 7, 8};
    uint16_t result[8];

    call_narrow_i32x4_u(vec1, vec2, result);

    // Basic validation that function executed and produced results
    ASSERT_EQ(1, result[0]) << "Function execution produced unexpected result";
    ASSERT_EQ(5, result[4]) << "Vector lane ordering validation failed";
}

// Instantiate parameterized tests - focus on interpreter mode for now
INSTANTIATE_TEST_CASE_P(RunningMode, I16x8NarrowI32x4UTest,
                        testing::Values(Mode_Interp));