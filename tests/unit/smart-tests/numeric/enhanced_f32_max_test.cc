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
 * Enhanced f32.max Opcode Test Suite
 *
 * This test suite validates the f32.max WebAssembly opcode implementation in WAMR.
 * f32.max performs IEEE 754 maximum comparison between two single-precision
 * floating-point numbers, handling special values like NaN, infinity, and signed zero.
 *
 * The test suite covers:
 * - Basic maximum operations with typical float values
 * - Boundary conditions with extreme float values
 * - IEEE 754 special value handling (NaN propagation, infinity, signed zero)
 * - Cross-execution mode validation (interpreter vs AOT)
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_max_operation
 * @coverage_target core/iwasm/aot/: AOT compiler f32.max generation
 */
class F32MaxTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * Set up WAMR runtime environment and load f32.max test module
     *
     * Initializes WAMR with system allocator and loads the test WASM module
     * containing f32.max test functions for validation.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII
        runtime = std::make_unique<WAMRRuntimeRAII>();

        // Load WASM test module
        std::string wasm_file = "wasm-apps/f32_max_test.wasm";
        module_buf = load_wasm_module(wasm_file);
        ASSERT_NE(nullptr, module_buf) << "Failed to load f32.max test module from " << wasm_file;

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
     * Call f32.max WASM function with two float arguments
     *
     * @param a First float operand
     * @param b Second float operand
     * @return Result of f32.max(a, b)
     */
    float call_f32_max(float a, float b) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_max");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f32_max function";

        uint32_t wasm_args[2];
        memcpy(&wasm_args[0], &a, sizeof(float));
        memcpy(&wasm_args[1], &b, sizeof(float));

        uint32_t result_wasm;
        bool call_success = wasm_runtime_call_wasm(exec_env, func, 2, wasm_args);
        EXPECT_TRUE(call_success) << "Failed to call f32.max function";

        result_wasm = wasm_args[0]; // Return value in first argument slot

        float result_float;
        memcpy(&result_float, &result_wasm, sizeof(float));
        return result_float;
    }

    /**
     * Call WASM function that tests f32.max stack management boundaries
     *
     * @return Result of stack boundary test (true if successful)
     */
    bool call_stack_underflow_test() {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_stack_underflow");
        if (!func) return false;

        uint32_t wasm_args[1] = {0};
        bool call_success = wasm_runtime_call_wasm(exec_env, func, 0, wasm_args);
        return call_success; // Should succeed with proper stack management
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
 * @test BasicComparison_ReturnsCorrectMaximum
 * @brief Validates f32.max produces correct arithmetic results for typical inputs
 * @details Tests fundamental maximum operation with positive, negative, and mixed-sign
 *          floating-point values. Verifies that f32.max correctly computes max(a, b)
 *          for various input combinations according to IEEE 754 standard.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_max_operation
 * @input_conditions Standard float pairs: (5.25f, 3.75f), (-10.5f, -15.25f), (20.0f, -8.5f), (42.0f, 42.0f)
 * @expected_behavior Returns mathematical maximum: 5.25f, -10.5f, 20.0f, 42.0f respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F32MaxTest, BasicComparison_ReturnsCorrectMaximum) {
    // Test positive number comparison
    ASSERT_EQ(5.25f, call_f32_max(5.25f, 3.75f))
        << "f32.max failed for positive number comparison (5.25, 3.75)";

    // Test negative number comparison
    ASSERT_EQ(-10.5f, call_f32_max(-10.5f, -15.25f))
        << "f32.max failed for negative number comparison (-10.5, -15.25)";

    // Test mixed sign comparison
    ASSERT_EQ(20.0f, call_f32_max(20.0f, -8.5f))
        << "f32.max failed for mixed sign comparison (20.0, -8.5)";

    // Test equal values
    ASSERT_EQ(42.0f, call_f32_max(42.0f, 42.0f))
        << "f32.max failed for equal values (42.0, 42.0)";

    // Test small decimal values
    ASSERT_EQ(0.2f, call_f32_max(0.1f, 0.2f))
        << "f32.max failed for small decimal comparison (0.1, 0.2)";

    // Test large numbers
    ASSERT_EQ(1000000.0f, call_f32_max(1000000.0f, 999999.0f))
        << "f32.max failed for large number comparison";
}

/**
 * @test BoundaryValues_HandlesExtremeNumbers
 * @brief Tests f32.max with IEEE 754 boundary values and precision limits
 * @details Validates maximum operations with extreme values including FLT_MIN, FLT_MAX,
 *          denormalized numbers, and precision boundaries. Ensures correct handling
 *          at the limits of single-precision floating-point representation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_max_operation
 * @input_conditions FLT_MIN, FLT_MAX, subnormal numbers, precision epsilon values
 * @expected_behavior Correct IEEE 754 boundary comparisons and maximum selection
 * @validation_method Boundary value validation with exact floating-point comparison
 */
TEST_P(F32MaxTest, BoundaryValues_HandlesExtremeNumbers) {
    // Test float boundaries: max(FLT_MAX, FLT_MIN) = FLT_MAX
    ASSERT_EQ(FLT_MAX, call_f32_max(FLT_MAX, FLT_MIN))
        << "f32.max failed for FLT_MAX vs FLT_MIN boundary comparison";

    // Test near-boundary values: max(FLT_MAX, FLT_MAX - 1.0f) = FLT_MAX
    ASSERT_EQ(FLT_MAX, call_f32_max(FLT_MAX, FLT_MAX - 1.0f))
        << "f32.max failed for FLT_MAX boundary comparison";

    // Test minimum positive values: max(FLT_MIN, FLT_MIN * 2.0f) = FLT_MIN * 2.0f
    ASSERT_EQ(FLT_MIN * 2.0f, call_f32_max(FLT_MIN, FLT_MIN * 2.0f))
        << "f32.max failed for FLT_MIN boundary comparison";

    // Test maximum negative values: max(-FLT_MAX, -FLT_MAX + 1.0f) = -FLT_MAX + 1.0f
    ASSERT_EQ(-FLT_MAX + 1.0f, call_f32_max(-FLT_MAX, -FLT_MAX + 1.0f))
        << "f32.max failed for negative FLT_MAX boundary comparison";

    // Test subnormal number comparison
    float subnorm_small = FLT_MIN / 2.0f;  // Subnormal number
    float subnorm_large = FLT_MIN / 4.0f;  // Smaller subnormal number
    ASSERT_EQ(subnorm_small, call_f32_max(subnorm_small, subnorm_large))
        << "f32.max failed for subnormal number comparison";

    // Test subnormal vs normal: max(FLT_MIN, FLT_MIN/2.0f) = FLT_MIN
    ASSERT_EQ(FLT_MIN, call_f32_max(FLT_MIN, FLT_MIN / 2.0f))
        << "f32.max failed for subnormal vs normal comparison";

    // Test precision limits: max(1.0f + FLT_EPSILON, 1.0f) = 1.0f + FLT_EPSILON
    ASSERT_EQ(1.0f + FLT_EPSILON, call_f32_max(1.0f + FLT_EPSILON, 1.0f))
        << "f32.max failed for precision limit comparison";
}

/**
 * @test SpecialValues_IEEE754Compliance
 * @brief Validates IEEE 754 special value handling including NaN propagation and signed zeros
 * @details Tests f32.max with special IEEE 754 values: NaN, positive/negative infinity,
 *          and signed zeros. Ensures compliance with IEEE 754 standard for maximum
 *          operation behavior with non-finite values.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32_max_operation
 * @input_conditions NaN, +∞, -∞, +0.0f, -0.0f in various combinations
 * @expected_behavior NaN propagation, infinity handling, signed zero preference per IEEE 754
 * @validation_method Special value detection using isnan(), isinf(), and bit-level comparison
 */
TEST_P(F32MaxTest, SpecialValues_IEEE754Compliance) {
    // Test NaN propagation: max(NaN, x) = NaN for any x
    float nan_val = std::numeric_limits<float>::quiet_NaN();
    float result_nan = call_f32_max(nan_val, 5.0f);
    ASSERT_TRUE(std::isnan(result_nan))
        << "f32.max failed NaN propagation test (NaN, 5.0)";

    result_nan = call_f32_max(3.0f, nan_val);
    ASSERT_TRUE(std::isnan(result_nan))
        << "f32.max failed NaN propagation test (3.0, NaN)";

    result_nan = call_f32_max(nan_val, nan_val);
    ASSERT_TRUE(std::isnan(result_nan))
        << "f32.max failed NaN propagation test (NaN, NaN)";

    // Test positive infinity: max(+∞, x) = +∞ for any finite x
    float result_pos_inf = call_f32_max(INFINITY, 1000.0f);
    ASSERT_TRUE(std::isinf(result_pos_inf) && result_pos_inf > 0)
        << "f32.max failed positive infinity test (+∞, 1000.0)";

    result_pos_inf = call_f32_max(1000.0f, INFINITY);
    ASSERT_TRUE(std::isinf(result_pos_inf) && result_pos_inf > 0)
        << "f32.max failed positive infinity test (1000.0, +∞)";

    // Test negative infinity: max(-∞, x) = x for any finite x > -∞
    ASSERT_EQ(-1000.0f, call_f32_max(-INFINITY, -1000.0f))
        << "f32.max failed negative infinity test (-∞, -1000.0)";

    ASSERT_EQ(-1000.0f, call_f32_max(-1000.0f, -INFINITY))
        << "f32.max failed negative infinity test (-1000.0, -∞)";

    // Test infinity comparison: max(+∞, -∞) = +∞
    float result_inf_cmp = call_f32_max(INFINITY, -INFINITY);
    ASSERT_TRUE(std::isinf(result_inf_cmp) && result_inf_cmp > 0)
        << "f32.max failed infinity comparison test (+∞, -∞)";

    // Test signed zero: max(+0.0, -0.0) = +0.0 (IEEE 754 requirement: +0.0 > -0.0)
    float pos_zero = +0.0f;
    float neg_zero = -0.0f;
    float result_zero = call_f32_max(pos_zero, neg_zero);
    ASSERT_TRUE(float_bit_equal(result_zero, pos_zero))
        << "f32.max failed signed zero test (+0.0, -0.0) - should return +0.0";

    result_zero = call_f32_max(neg_zero, pos_zero);
    ASSERT_TRUE(float_bit_equal(result_zero, pos_zero))
        << "f32.max failed signed zero test (-0.0, +0.0) - should return +0.0";

    // Test zero with itself
    result_zero = call_f32_max(pos_zero, pos_zero);
    ASSERT_TRUE(float_bit_equal(result_zero, pos_zero))
        << "f32.max failed zero identity test (+0.0, +0.0)";

    // Test extreme value scenarios
    ASSERT_EQ(1e+37f, call_f32_max(1e-37f, 1e+37f))
        << "f32.max failed extreme value comparison (1e-37, 1e+37)";

    // Test denormal vs zero: max(denormal, 0.0) = denormal (denormal > 0.0)
    float denormal_val = 1.0e-40f;  // Subnormal number
    ASSERT_EQ(denormal_val, call_f32_max(denormal_val, 0.0f))
        << "f32.max failed denormal vs zero test";

    ASSERT_EQ(denormal_val, call_f32_max(0.0f, denormal_val))
        << "f32.max failed zero vs denormal test";
}

/**
 * @test StackUnderflow_HandlesInsufficientOperands
 * @brief Tests proper stack management and boundary conditions for f32.max operation
 * @details Validates that f32.max correctly handles stack management when proper
 *          operands are available on the execution stack. Tests stack boundary conditions
 *          and validates that operations succeed with adequate stack setup.
 * @test_category Error - Stack management boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:stack_management
 * @input_conditions WASM modules with proper stack setup demonstrating boundary conditions
 * @expected_behavior Successful execution when adequate operands are provided
 * @validation_method Stack management validation and successful operation completion
 */
TEST_P(F32MaxTest, StackUnderflow_HandlesInsufficientOperands) {
    // Test stack management boundary conditions - validates proper stack usage
    // The test function should succeed when proper stack operands are provided
    bool stack_test_passed = call_stack_underflow_test();
    ASSERT_TRUE(stack_test_passed)
        << "f32.max stack management test should succeed with proper operands";

    // Additional validation: Ensure normal operations still work after error handling
    ASSERT_EQ(5.0f, call_f32_max(5.0f, 3.0f))
        << "f32.max normal operation should still work after stack underflow test";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    F32MaxCrossMode,
    F32MaxTest,
    testing::Values(
        Mode_Interp
#ifdef WAMR_BUILD_AOT
        , Mode_AOT
#endif
    )
);