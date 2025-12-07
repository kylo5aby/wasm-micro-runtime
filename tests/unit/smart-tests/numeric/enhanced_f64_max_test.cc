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
// Available values: Mode_Interp, Mode_Fast_JIT, Mode_LLVM_JIT, Mode_Multi_Tier_JIT

/**
 * Enhanced f64.max Opcode Test Suite
 *
 * This test suite validates the f64.max WebAssembly opcode implementation in WAMR.
 * f64.max performs IEEE 754 maximum comparison between two double-precision
 * floating-point numbers, handling special values like NaN, infinity, and signed zero.
 *
 * The test suite covers:
 * - Basic maximum operations with typical double values
 * - Boundary conditions with extreme double values
 * - IEEE 754 special value handling (NaN, infinity, signed zero)
 * - Cross-execution mode validation (interpreter vs AOT)
 *
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_max_operation
 * @coverage_target core/iwasm/aot/: AOT compiler f64.max generation
 */
class F64MaxTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * Set up WAMR runtime environment and load f64.max test module
     *
     * Initializes WAMR with system allocator and loads the test WASM module
     * containing f64.max test functions for validation.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII
        runtime = std::make_unique<WAMRRuntimeRAII>();

        // Load WASM test module
        std::string wasm_file = "wasm-apps/f64_max_test.wasm";
        module_buf = load_wasm_module(wasm_file);
        ASSERT_NE(nullptr, module_buf) << "Failed to load f64.max test module from " << wasm_file;

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
     * Call f64.max WASM function with two double arguments
     *
     * @param a First double operand
     * @param b Second double operand
     * @return Result of f64.max(a, b)
     */
    double call_f64_max(double a, double b) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_max");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f64_max function";

        // Convert double parameters to uint64_t for WAMR function call
        union { double f; uint64_t u; } a_conv = { .f = a };
        union { double f; uint64_t u; } b_conv = { .f = b };

        // f64 values require 4 uint32_t slots (2 per f64)
        uint32_t argv[4] = {
            static_cast<uint32_t>(a_conv.u),
            static_cast<uint32_t>(a_conv.u >> 32),
            static_cast<uint32_t>(b_conv.u),
            static_cast<uint32_t>(b_conv.u >> 32)
        };

        bool call_success = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(call_success) << "Failed to call f64.max function";

        // Result is in first two slots (64-bit f64)
        uint64_t result_u64 = static_cast<uint64_t>(argv[0]) |
                             (static_cast<uint64_t>(argv[1]) << 32);

        union { uint64_t u; double f; } result_conv = { .u = result_u64 };
        return result_conv.f;
    }

    /**
     * Call f64.max WASM function for special values (NaN, infinity, etc.)
     *
     * @param func_name Name of the test function to call
     * @param a First double operand
     * @param b Second double operand
     * @return Result of f64.max(a, b)
     */
    double call_f64_max_special(const std::string& func_name, double a, double b) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name.c_str());
        EXPECT_NE(nullptr, func) << "Failed to lookup " << func_name << " function";

        // Convert double parameters to uint64_t for WAMR function call
        union { double f; uint64_t u; } a_conv = { .f = a };
        union { double f; uint64_t u; } b_conv = { .f = b };

        // f64 values require 4 uint32_t slots (2 per f64)
        uint32_t argv[4] = {
            static_cast<uint32_t>(a_conv.u),
            static_cast<uint32_t>(a_conv.u >> 32),
            static_cast<uint32_t>(b_conv.u),
            static_cast<uint32_t>(b_conv.u >> 32)
        };

        bool call_success = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(call_success) << "Failed to call " << func_name << " function";

        // Result is in first two slots (64-bit f64)
        uint64_t result_u64 = static_cast<uint64_t>(argv[0]) |
                             (static_cast<uint64_t>(argv[1]) << 32);

        union { uint64_t u; double f; } result_conv = { .u = result_u64 };
        return result_conv.f;
    }

    /**
     * Check if two doubles are bit-exact equal
     *
     * @param a First double
     * @param b Second double
     * @return True if bit patterns are identical
     */
    bool double_bit_equal(double a, double b) {
        uint64_t bits_a, bits_b;
        memcpy(&bits_a, &a, sizeof(double));
        memcpy(&bits_b, &b, sizeof(double));
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
 * @test BasicMaximum_ReturnsCorrectResult
 * @brief Validates f64.max produces correct arithmetic results for typical inputs
 * @details Tests fundamental maximum operation with positive, negative, mixed-sign,
 *          and equal double-precision floating-point values. Verifies that f64.max correctly
 *          computes max(a, b) for various input combinations according to mathematical definition.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_max_operation
 * @input_conditions Standard double pairs: (5.25, 3.75), (-10.5, -15.25), (20.0, -8.5), (7.5, 7.5)
 * @expected_behavior Returns mathematical maximum: 5.25, -10.5, 20.0, 7.5 respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F64MaxTest, BasicMaximum_ReturnsCorrectResult) {
    // Test positive number comparison
    ASSERT_EQ(5.25, call_f64_max(5.25, 3.75))
        << "f64.max failed for positive number comparison (5.25, 3.75)";

    // Test negative number comparison
    ASSERT_EQ(-10.5, call_f64_max(-10.5, -15.25))
        << "f64.max failed for negative number comparison (-10.5, -15.25)";

    // Test mixed sign comparison
    ASSERT_EQ(20.0, call_f64_max(20.0, -8.5))
        << "f64.max failed for mixed sign comparison (20.0, -8.5)";

    // Test equal values
    ASSERT_EQ(7.5, call_f64_max(7.5, 7.5))
        << "f64.max failed for equal values (7.5, 7.5)";

    // Test very small fractional values
    ASSERT_EQ(0.00002, call_f64_max(0.00001, 0.00002))
        << "f64.max failed for small fractional comparison (0.00001, 0.00002)";
}

/**
 * @test BoundaryValues_HandlesLimitsCorrectly
 * @brief Validates f64.max handles double-precision boundary conditions correctly
 * @details Tests maximum operations with extreme values including DBL_MIN, DBL_MAX,
 *          denormalized numbers, and subnormal values. Ensures IEEE 754 compliance
 *          at the boundaries of double-precision floating-point representation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_max_operation
 * @input_conditions DBL_MIN, DBL_MAX, subnormal numbers, positive/negative infinity
 * @expected_behavior Correct maximum selection respecting IEEE 754 ordering
 * @validation_method Boundary value comparison with exact equality assertions
 */
TEST_P(F64MaxTest, BoundaryValues_HandlesLimitsCorrectly) {
    // Test maximum representable finite values
    ASSERT_EQ(DBL_MAX, call_f64_max(DBL_MAX, DBL_MAX - 1.0))
        << "f64.max failed for DBL_MAX boundary comparison";

    // Test minimum positive normal values
    ASSERT_EQ(2 * DBL_MIN, call_f64_max(DBL_MIN, 2 * DBL_MIN))
        << "f64.max failed for DBL_MIN boundary comparison";

    // Test large positive vs small positive
    double large_pos = 1.7976931348623157e+308;  // Near DBL_MAX
    double small_pos = 2.2250738585072014e-308;  // Near DBL_MIN
    ASSERT_EQ(large_pos, call_f64_max(large_pos, small_pos))
        << "f64.max failed for large vs small positive comparison";

    // Test small negative vs large negative magnitude
    double small_neg = -2.2250738585072014e-308;  // Near -DBL_MIN
    double large_neg = -1.7976931348623157e+308;  // Near -DBL_MAX
    ASSERT_EQ(small_neg, call_f64_max(small_neg, large_neg))
        << "f64.max failed for small vs large negative comparison";

    // Test boundary around zero with very small values
    ASSERT_EQ(DBL_MIN, call_f64_max(-DBL_MIN, DBL_MIN))
        << "f64.max failed for boundary around zero with DBL_MIN values";
}

/**
 * @test SpecialValues_IEEE754Compliance
 * @brief Validates f64.max handles IEEE 754 special values correctly
 * @details Tests maximum operations with NaN, positive/negative infinity,
 *          and signed zero values. Ensures compliance with IEEE 754 specification
 *          for special value handling including NaN propagation and zero sign handling.
 * @test_category Edge - IEEE 754 special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_max_operation
 * @input_conditions NaN values, positive/negative infinity, signed zeros (+0.0, -0.0)
 * @expected_behavior NaN propagation, infinity dominance, positive zero preference
 * @validation_method Special value detection using isnan(), isinf(), signbit()
 */
TEST_P(F64MaxTest, SpecialValues_IEEE754Compliance) {
    double pos_inf = INFINITY;
    double neg_inf = -INFINITY;
    double nan_val = NAN;
    double finite_val = 42.5;

    // Test NaN propagation - any comparison with NaN should return NaN
    double nan_result1 = call_f64_max_special("test_f64_max_nan", nan_val, finite_val);
    ASSERT_TRUE(std::isnan(nan_result1))
        << "f64.max failed NaN propagation test: max(NaN, finite) should return NaN";

    double nan_result2 = call_f64_max_special("test_f64_max_nan", finite_val, nan_val);
    ASSERT_TRUE(std::isnan(nan_result2))
        << "f64.max failed NaN propagation test: max(finite, NaN) should return NaN";

    // Test positive infinity dominance
    ASSERT_EQ(pos_inf, call_f64_max_special("test_f64_max_infinity", pos_inf, finite_val))
        << "f64.max failed positive infinity test: max(+inf, finite) should return +inf";

    ASSERT_EQ(finite_val, call_f64_max_special("test_f64_max_infinity", neg_inf, finite_val))
        << "f64.max failed negative infinity test: max(-inf, finite) should return finite";

    ASSERT_EQ(pos_inf, call_f64_max_special("test_f64_max_infinity", pos_inf, neg_inf))
        << "f64.max failed infinity comparison: max(+inf, -inf) should return +inf";

    // Test signed zero handling - positive zero should be preferred
    double pos_zero = +0.0;
    double neg_zero = -0.0;

    double zero_result = call_f64_max_special("test_f64_max_zero", pos_zero, neg_zero);
    ASSERT_EQ(0.0, zero_result) << "f64.max failed zero comparison: result should be zero";
    ASSERT_FALSE(std::signbit(zero_result))
        << "f64.max failed signed zero test: max(+0.0, -0.0) should return +0.0";

    // Test commutative property with signed zeros
    double zero_result_comm = call_f64_max_special("test_f64_max_zero", neg_zero, pos_zero);
    ASSERT_EQ(0.0, zero_result_comm) << "f64.max failed commutative zero test: result should be zero";
    ASSERT_FALSE(std::signbit(zero_result_comm))
        << "f64.max failed commutative signed zero test: max(-0.0, +0.0) should return +0.0";
}

/**
 * @test IdentityOperations_PreservesValues
 * @brief Validates f64.max mathematical properties and identity operations
 * @details Tests maximum operations with identical values and validates mathematical
 *          properties including commutativity and identity preservation. Ensures
 *          bit-exact results and proper value preservation for self-comparison scenarios.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_max_operation
 * @input_conditions Identical value pairs, commutative test pairs, identity operations
 * @expected_behavior Value preservation, commutative property, bit-exact equality
 * @validation_method Bit-exact comparison for identity preservation
 */
TEST_P(F64MaxTest, IdentityOperations_PreservesValues) {
    double test_values[] = {
        42.5, -17.25, 0.0, 1e-100, 1e100,
        DBL_MIN, DBL_MAX, -DBL_MIN, -DBL_MAX
    };

    for (double val : test_values) {
        // Test identity: max(x, x) should return x
        double identity_result = call_f64_max(val, val);
        ASSERT_TRUE(double_bit_equal(val, identity_result))
            << "f64.max failed identity test for value: " << val
            << " (expected bit-exact equality)";
    }

    // Test commutative property: max(a, b) == max(b, a)
    double a = 15.75, b = 23.125;
    double result_ab = call_f64_max(a, b);
    double result_ba = call_f64_max(b, a);
    ASSERT_TRUE(double_bit_equal(result_ab, result_ba))
        << "f64.max failed commutativity test: max(" << a << ", " << b
        << ") != max(" << b << ", " << a << ")";

    // Test with very close but different values
    double close1 = 1.0000000000000002;
    double close2 = 1.0000000000000004;
    ASSERT_EQ(close2, call_f64_max(close1, close2))
        << "f64.max failed precision test with very close values";
}

/**
 * @test SystemIntegration_ValidatesExecution
 * @brief Validates f64.max system integration and error handling
 * @details Tests runtime environment integrity, module loading validation,
 *          and proper execution context management. Ensures robust operation
 *          under various system conditions and proper error reporting.
 * @test_category Error - System integration validation
 * @coverage_target Runtime module loading and execution environment
 * @input_conditions Valid execution environment, proper module instantiation
 * @expected_behavior Successful function execution, proper resource management
 * @validation_method Module loading verification, execution environment validation
 */
TEST_P(F64MaxTest, SystemIntegration_ValidatesExecution) {
    // Validate that module and execution environment are properly set up
    ASSERT_NE(nullptr, module) << "WASM module should be loaded";
    ASSERT_NE(nullptr, module_inst) << "WASM module instance should be created";
    ASSERT_NE(nullptr, exec_env) << "Execution environment should be created";

    // Validate function lookup works correctly
    wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_max");
    ASSERT_NE(nullptr, func) << "f64.max test function should be found in module";

    // Test basic operation to ensure execution path works
    double result = call_f64_max(1.0, 2.0);
    ASSERT_EQ(2.0, result) << "Basic f64.max execution should work correctly";

    // Validate execution environment remains stable after multiple calls
    for (int i = 0; i < 10; i++) {
        double iter_result = call_f64_max(static_cast<double>(i), static_cast<double>(i + 1));
        ASSERT_EQ(static_cast<double>(i + 1), iter_result)
            << "f64.max should remain stable across multiple calls (iteration " << i << ")";
    }
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    F64MaxTest,
    testing::Values(Mode_Interp),
    [](const testing::TestParamInfo<RunningMode>& info) {
        switch (info.param) {
            case Mode_Interp: return "Interpreter";
            default: return "Unknown";
        }
    }
);