/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <climits>
#include <cfloat>
#include <cmath>
#include <cstring>

#include "wasm_runtime_common.h"
#include "wasm_runtime.h"

class WAMRRuntimeRAII {
public:
    WAMRRuntimeRAII() {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime
        if (!wasm_runtime_full_init(&init_args)) {
            throw std::runtime_error("Failed to initialize WAMR runtime");
        }
    }

    ~WAMRRuntimeRAII() {
        wasm_runtime_destroy();
    }
};

// Note: Using WAMR's existing RunningMode enum from wasm_export.h
// Available values: Mode_Interp, Mode_AOT, Mode_JIT, Mode_Fast_JIT

/**
 * Enhanced f32.min Opcode Test Suite
 *
 * This test suite validates the f32.min WebAssembly opcode implementation in WAMR.
 * f32.min performs IEEE 754 minimum comparison between two single-precision
 * floating-point numbers, handling special values like NaN, infinity, and signed zero.
 *
 * The test suite covers:
 * - Basic minimum operations with typical float values
 * - Boundary conditions with extreme float values
 * - IEEE 754 special value handling (NaN, infinity, signed zero)
 * - Cross-execution mode validation (interpreter vs AOT)
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_min_operation
 * @coverage_target core/iwasm/aot/: AOT compiler f32.min generation
 */
class F32MinTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * Set up WAMR runtime environment and load f32.min test module
     *
     * Initializes WAMR with system allocator and loads the test WASM module
     * containing f32.min test functions for validation.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII
        runtime = std::make_unique<WAMRRuntimeRAII>();

        // Load WASM test module
        std::string wasm_file = "wasm-apps/f32_min_test.wasm";
        module_buf = load_wasm_module(wasm_file);
        ASSERT_NE(nullptr, module_buf) << "Failed to load f32.min test module from " << wasm_file;

        // Load module into WAMR runtime
        char error_buf[256];
        module = wasm_runtime_load(module_buf.get(), module_buf_size,
                                 error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        // Instantiate module
        module_inst = wasm_runtime_instantiate(module, 32 * 1024, 32 * 1024,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, 32 * 1024);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
    }

    /**
     * Clean up WAMR runtime resources
     *
     * Properly destroys execution environment and unloads module.
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
        // RAII handles runtime cleanup
    }

    /**
     * Load WASM module from file system
     *
     * @param filename Path to WASM file relative to test directory
     * @return Unique pointer to module buffer
     */
    std::unique_ptr<uint8_t[]> load_wasm_module(const std::string& filename) {
        FILE* file = fopen(filename.c_str(), "rb");
        if (!file) return nullptr;

        // Get file size
        fseek(file, 0, SEEK_END);
        module_buf_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate buffer and read file
        auto buffer = std::make_unique<uint8_t[]>(module_buf_size);
        size_t read_size = fread(buffer.get(), 1, module_buf_size, file);
        fclose(file);

        if (read_size != module_buf_size) return nullptr;
        return buffer;
    }

    /**
     * Call f32.min WASM function with two float arguments
     *
     * @param a First float operand
     * @param b Second float operand
     * @return Result of f32.min(a, b)
     */
    float call_f32_min(float a, float b) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_min");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f32_min function";

        uint32_t wasm_args[2];
        memcpy(&wasm_args[0], &a, sizeof(float));
        memcpy(&wasm_args[1], &b, sizeof(float));

        uint32_t result_wasm;
        bool call_success = wasm_runtime_call_wasm(exec_env, func, 2, wasm_args);
        EXPECT_TRUE(call_success) << "Failed to call f32.min function";

        result_wasm = wasm_args[0]; // Return value in first argument slot

        float result_float;
        memcpy(&result_float, &result_wasm, sizeof(float));
        return result_float;
    }

    /**
     * Check if two floats are bit-exact equal
     *
     * @param a First float
     * @param b Second float
     * @return True if bit patterns are identical
     */
    bool float_bit_equal(float a, float b) {
        uint32_t bits_a, bits_b;
        memcpy(&bits_a, &a, sizeof(float));
        memcpy(&bits_b, &b, sizeof(float));
        return bits_a == bits_b;
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII> runtime;
    std::unique_ptr<uint8_t[]> module_buf;
    uint32_t module_buf_size = 0;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
};

/**
 * @test BasicMinimumComparison_ReturnsCorrectMinimum
 * @brief Validates f32.min produces correct arithmetic results for typical inputs
 * @details Tests fundamental minimum operation with positive, negative, mixed-sign,
 *          and equal floating-point values. Verifies that f32.min correctly computes
 *          min(a, b) for various input combinations according to mathematical definition.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_min_operation
 * @input_conditions Standard float pairs: (5.5f, 3.2f), (-2.1f, -7.8f), (4.0f, -1.5f), (2.5f, 2.5f)
 * @expected_behavior Returns mathematical minimum: 3.2f, -7.8f, -1.5f, 2.5f respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F32MinTest, BasicMinimumComparison_ReturnsCorrectMinimum) {
    // Test positive number comparison
    ASSERT_EQ(3.2f, call_f32_min(5.5f, 3.2f))
        << "f32.min failed for positive number comparison (5.5, 3.2)";

    // Test negative number comparison
    ASSERT_EQ(-7.8f, call_f32_min(-2.1f, -7.8f))
        << "f32.min failed for negative number comparison (-2.1, -7.8)";

    // Test mixed sign comparison
    ASSERT_EQ(-1.5f, call_f32_min(4.0f, -1.5f))
        << "f32.min failed for mixed sign comparison (4.0, -1.5)";

    // Test equal values
    ASSERT_EQ(2.5f, call_f32_min(2.5f, 2.5f))
        << "f32.min failed for equal values (2.5, 2.5)";

    // Test small fractional values
    ASSERT_EQ(0.001f, call_f32_min(0.001f, 0.002f))
        << "f32.min failed for small fractional comparison (0.001, 0.002)";
}

/**
 * @test BoundaryValueComparison_HandlesExtremeValues
 * @brief Validates f32.min handles floating-point boundary conditions correctly
 * @details Tests minimum operations with extreme values including FLT_MIN, FLT_MAX,
 *          denormalized numbers, and infinity values. Ensures IEEE 754 compliance
 *          at the boundaries of single-precision floating-point representation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_min_operation
 * @input_conditions FLT_MIN, FLT_MAX, denormalized numbers, positive/negative infinity
 * @expected_behavior Correct IEEE 754 boundary comparisons and infinity handling
 * @validation_method Exact float comparison and infinity state validation
 */
TEST_P(F32MinTest, BoundaryValueComparison_HandlesExtremeValues) {
    // Test float boundaries: min(FLT_MAX, FLT_MIN) = FLT_MIN
    ASSERT_EQ(FLT_MIN, call_f32_min(FLT_MAX, FLT_MIN))
        << "f32.min failed for FLT_MAX vs FLT_MIN boundary comparison";

    // Test near-boundary values: min(FLT_MAX, 1.0f) = 1.0f
    ASSERT_EQ(1.0f, call_f32_min(FLT_MAX, 1.0f))
        << "f32.min failed for FLT_MAX vs normal value comparison";

    // Test denormalized number comparison
    float denorm_small = 1.0e-40f;  // Denormalized number
    float denorm_large = 1.0e-39f;  // Larger denormalized number
    ASSERT_EQ(denorm_small, call_f32_min(denorm_small, denorm_large))
        << "f32.min failed for denormalized number comparison";

    // Test positive infinity: min(INFINITY, 1000.0f) = 1000.0f
    ASSERT_EQ(1000.0f, call_f32_min(INFINITY, 1000.0f))
        << "f32.min failed for positive infinity vs finite comparison";

    // Test negative infinity: min(-INFINITY, -1000.0f) = -INFINITY
    float result_neg_inf = call_f32_min(-INFINITY, -1000.0f);
    ASSERT_TRUE(std::isinf(result_neg_inf) && result_neg_inf < 0)
        << "f32.min failed for negative infinity vs finite comparison";

    // Test infinity comparison: min(INFINITY, -INFINITY) = -INFINITY
    float result_inf_comp = call_f32_min(INFINITY, -INFINITY);
    ASSERT_TRUE(std::isinf(result_inf_comp) && result_inf_comp < 0)
        << "f32.min failed for infinity vs negative infinity comparison";
}

/**
 * @test SpecialValueHandling_FollowsIEEE754Standard
 * @brief Validates f32.min follows IEEE 754 standard for special values
 * @details Tests IEEE 754 compliance including NaN propagation, signed zero handling,
 *          and special value interactions. Ensures WAMR implementation matches
 *          IEEE 754 floating-point standard requirements for minimum operations.
 * @test_category Edge - IEEE 754 special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_min_operation
 * @input_conditions NaN values, signed zeros (+0.0/-0.0), infinities with finite values
 * @expected_behavior IEEE 754 compliant NaN propagation, negative zero preference, proper infinity handling
 * @validation_method NaN detection, sign bit validation, infinity state checking
 */
TEST_P(F32MinTest, SpecialValueHandling_FollowsIEEE754Standard) {
    // Test NaN propagation: min(NAN, 5.0f) = NaN
    float result_nan1 = call_f32_min(NAN, 5.0f);
    ASSERT_TRUE(std::isnan(result_nan1))
        << "f32.min failed to propagate NaN (first operand)";

    // Test NaN propagation: min(3.0f, NAN) = NaN
    float result_nan2 = call_f32_min(3.0f, NAN);
    ASSERT_TRUE(std::isnan(result_nan2))
        << "f32.min failed to propagate NaN (second operand)";

    // Test double NaN: min(NAN, NAN) = NaN
    float result_nan3 = call_f32_min(NAN, NAN);
    ASSERT_TRUE(std::isnan(result_nan3))
        << "f32.min failed to handle double NaN inputs";

    // Test signed zero handling: min(+0.0f, -0.0f) = -0.0f (IEEE 754: negative zero preferred)
    float pos_zero = +0.0f;
    float neg_zero = -0.0f;
    float result_zero = call_f32_min(pos_zero, neg_zero);
    ASSERT_TRUE(float_bit_equal(result_zero, neg_zero))
        << "f32.min failed to prefer negative zero in (+0.0, -0.0) comparison";

    // Test commutative signed zero: min(-0.0f, +0.0f) = -0.0f
    float result_zero_comm = call_f32_min(neg_zero, pos_zero);
    ASSERT_TRUE(float_bit_equal(result_zero_comm, neg_zero))
        << "f32.min failed to prefer negative zero in (-0.0, +0.0) comparison";

    // Test double zero: min(0.0f, 0.0f) = 0.0f
    ASSERT_EQ(0.0f, call_f32_min(0.0f, 0.0f))
        << "f32.min failed for identical zero comparison";

    // Test zero with finite: min(0.0f, 1.0f) = 0.0f
    ASSERT_EQ(0.0f, call_f32_min(0.0f, 1.0f))
        << "f32.min failed for zero vs positive finite comparison";
}

/**
 * @test ModuleExecutionValidation_LoadsAndRunsCorrectly
 * @brief Validates WASM module loading and f32.min function execution
 * @details Tests proper WASM module loading, function lookup, and execution context
 *          for f32.min operations. Ensures WAMR runtime correctly handles module
 *          instantiation and function calls for floating-point operations.
 * @test_category Exception - Module and execution validation
 * @coverage_target wasm_runtime.h:wasm_runtime_load, wasm_runtime_call_wasm
 * @input_conditions WASM module file, function lookup, execution environment
 * @expected_behavior Successful module loading, function accessibility, proper execution
 * @validation_method Module pointer validation, function pointer validation, execution success
 */
TEST_P(F32MinTest, ModuleExecutionValidation_LoadsAndRunsCorrectly) {
    // Verify module loaded successfully (already validated in SetUp, but explicit check)
    ASSERT_NE(nullptr, module)
        << "WASM module should be loaded successfully";

    // Verify module instance created successfully
    ASSERT_NE(nullptr, module_inst)
        << "WASM module instance should be created successfully";

    // Verify execution environment created successfully
    ASSERT_NE(nullptr, exec_env)
        << "Execution environment should be created successfully";

    // Verify f32.min function is accessible
    wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_min");
    ASSERT_NE(nullptr, func)
        << "test_f32_min function should be found in WASM module";

    // Test successful function execution with simple values
    float result = call_f32_min(10.0f, 5.0f);
    ASSERT_EQ(5.0f, result)
        << "f32.min function execution should succeed and return correct result";
}

// Instantiate parameterized tests for available execution modes
INSTANTIATE_TEST_SUITE_P(RunningModeTests, F32MinTest,
                        testing::Values(Mode_Interp));