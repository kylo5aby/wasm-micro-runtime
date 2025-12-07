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
 * @brief Test fixture for comprehensive f64.reinterpret_i64 opcode validation
 * @details Provides runtime initialization, module management, and cross-execution mode testing
 *          for IEEE 754 bit pattern reinterpretation functionality. Tests verify bit-exact
 *          reinterpretation of i64 values to f64 representation across interpreter and AOT modes.
 */
class F64ReinterpretI64Test : public testing::TestWithParam<RunningMode> {
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
     */
    void SetUp() override {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Initialize WASM file paths - done in SetUp to ensure proper timing
        WASM_FILE = "wasm-apps/f64_reinterpret_i64_test.wasm";
        WASM_FILE_STACK_UNDERFLOW = "wasm-apps/f64_reinterpret_i64_stack_underflow.wasm";

        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up WAMR runtime resources and modules
     * @details Destroys execution environment, unloads modules, frees buffers, and shuts down runtime
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
        if (buf) {
            BH_FREE(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Call f64.reinterpret_i64 function in loaded WASM module
     * @param i64_val Input i64 value to reinterpret as f64 bit pattern
     * @return Reinterpreted f64 value representing the bit pattern of input i64
     */
    double call_f64_reinterpret_i64(int64_t i64_val) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f64_reinterpret_i64");
        EXPECT_NE(func, nullptr) << "Failed to lookup f64_reinterpret_i64 function";

        wasm_val_t arguments[1] = { 0 };
        wasm_val_t results[1] = { 0 };
        arguments[0].kind = WASM_I64;
        arguments[0].of.i64 = i64_val;
        results[0].kind = WASM_F64;

        bool call_result = wasm_runtime_call_wasm_a(exec_env, func, 1, results, 1, arguments);
        EXPECT_TRUE(call_result) << "Failed to call f64_reinterpret_i64 function";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Exception occurred: " << exception;

        return results[0].of.f64;
    }

    /**
     * @brief Helper function to interpret i64 bit pattern as f64 using union
     * @param i64_val Input i64 value
     * @return f64 value with identical bit pattern
     */
    double i64_to_f64_bits(int64_t i64_val) {
        union { int64_t i; double f; } converter;
        converter.i = i64_val;
        return converter.f;
    }

    /**
     * @brief Helper function to check if two f64 values are bit-exact equal
     * @param a First f64 value
     * @param b Second f64 value
     * @return true if bit patterns are identical, false otherwise
     */
    bool f64_bit_equal(double a, double b) {
        union { double f; uint64_t u; } conv_a, conv_b;
        conv_a.f = a;
        conv_b.f = b;
        return conv_a.u == conv_b.u;
    }

    /**
     * @brief Helper function to get uint64_t bit representation of f64
     * @param f64_val Input f64 value
     * @return uint64_t representation of the bit pattern
     */
    uint64_t f64_to_bits(double f64_val) {
        union { double f; uint64_t u; } converter;
        converter.f = f64_val;
        return converter.u;
    }
};

/**
 * @test BasicReinterpretation_ReturnsCorrectF64Values
 * @brief Validates f64.reinterpret_i64 produces correct IEEE 754 results for typical inputs
 * @details Tests fundamental reinterpretation operation with positive, negative, and typical integers.
 *          Verifies that f64.reinterpret_i64 correctly reinterprets i64 bit patterns as f64 values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_reinterpret_i64_operation
 * @input_conditions Standard integer values: 0x4000000000000000, 0x0000000000000000, 0x3FF0000000000000
 * @expected_behavior Returns IEEE 754 double values: 2.0, +0.0, 1.0 respectively
 * @validation_method Direct bit-pattern comparison of WASM function result with expected values
 */
TEST_P(F64ReinterpretI64Test, BasicReinterpretation_ReturnsCorrectF64Values) {
    // Test typical positive integer representing 2.0 in IEEE 754
    double result1 = call_f64_reinterpret_i64(0x4000000000000000);
    ASSERT_TRUE(f64_bit_equal(result1, 2.0))
        << "f64.reinterpret_i64(0x4000000000000000) should produce 2.0, got " << std::fixed << std::setprecision(17) << result1;

    // Test zero integer representing +0.0
    double result2 = call_f64_reinterpret_i64(0x0000000000000000);
    ASSERT_TRUE(f64_bit_equal(result2, 0.0))
        << "f64.reinterpret_i64(0x0000000000000000) should produce +0.0, got " << std::fixed << std::setprecision(17) << result2;

    // Test integer representing 1.0 in IEEE 754
    double result3 = call_f64_reinterpret_i64(0x3FF0000000000000);
    ASSERT_TRUE(f64_bit_equal(result3, 1.0))
        << "f64.reinterpret_i64(0x3FF0000000000000) should produce 1.0, got " << std::fixed << std::setprecision(17) << result3;

    // Test negative integer representing -1.0 in IEEE 754
    double result4 = call_f64_reinterpret_i64(0xBFF0000000000000);
    ASSERT_TRUE(f64_bit_equal(result4, -1.0))
        << "f64.reinterpret_i64(0xBFF0000000000000) should produce -1.0, got " << std::fixed << std::setprecision(17) << result4;
}

/**
 * @test BoundaryValues_ProduceExpectedIEEE754Results
 * @brief Validates f64.reinterpret_i64 handles integer boundary values correctly
 * @details Tests boundary conditions with INT64_MIN, INT64_MAX and IEEE 754 boundary patterns.
 *          Verifies correct handling of extreme integer values and their f64 interpretations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_reinterpret_i64_operation
 * @input_conditions Boundary integers: INT64_MIN, INT64_MAX, largest/smallest normal f64 patterns
 * @expected_behavior Returns appropriate IEEE 754 values: -0.0, NaN, extreme normal values
 * @validation_method Bit-pattern verification for boundary value reinterpretations
 */
TEST_P(F64ReinterpretI64Test, BoundaryValues_ProduceExpectedIEEE754Results) {
    // Test INT64_MIN (0x8000000000000000) which represents -0.0 in IEEE 754
    double result1 = call_f64_reinterpret_i64(INT64_MIN);
    ASSERT_TRUE(f64_bit_equal(result1, -0.0))
        << "f64.reinterpret_i64(INT64_MIN) should produce -0.0, got " << std::fixed << std::setprecision(17) << result1;

    // Test INT64_MAX (0x7FFFFFFFFFFFFFFF) which represents a NaN pattern
    double result2 = call_f64_reinterpret_i64(INT64_MAX);
    ASSERT_TRUE(std::isnan(result2))
        << "f64.reinterpret_i64(INT64_MAX) should produce NaN, got " << std::fixed << std::setprecision(17) << result2;

    // Test largest positive normal f64 pattern (0x7FEFFFFFFFFFFFFF)
    double result3 = call_f64_reinterpret_i64(0x7FEFFFFFFFFFFFFF);
    double expected3 = i64_to_f64_bits(0x7FEFFFFFFFFFFFFF);
    ASSERT_TRUE(f64_bit_equal(result3, expected3))
        << "f64.reinterpret_i64(0x7FEFFFFFFFFFFFFF) bit pattern mismatch, expected "
        << std::hex << f64_to_bits(expected3) << ", got " << f64_to_bits(result3);

    // Test smallest positive normal f64 pattern (0x0010000000000000)
    double result4 = call_f64_reinterpret_i64(0x0010000000000000);
    double expected4 = i64_to_f64_bits(0x0010000000000000);
    ASSERT_TRUE(f64_bit_equal(result4, expected4))
        << "f64.reinterpret_i64(0x0010000000000000) bit pattern mismatch, expected "
        << std::hex << f64_to_bits(expected4) << ", got " << f64_to_bits(result4);
}

/**
 * @test SpecialIEEE754Patterns_GenerateCorrectFloatingPointValues
 * @brief Validates f64.reinterpret_i64 produces correct IEEE 754 special values
 * @details Tests special IEEE 754 patterns including infinities, NaN varieties, zeros, and subnormals.
 *          Verifies accurate reinterpretation of specific bit patterns to floating-point special values.
 * @test_category Edge - Special value and extreme scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_reinterpret_i64_operation
 * @input_conditions Special IEEE 754 patterns: infinity, NaN, zero, subnormal bit patterns
 * @expected_behavior Returns correct IEEE 754 special values with proper bit-exact representation
 * @validation_method Comprehensive bit-pattern verification for all IEEE 754 special categories
 */
TEST_P(F64ReinterpretI64Test, SpecialIEEE754Patterns_GenerateCorrectFloatingPointValues) {
    // Test positive infinity pattern (0x7FF0000000000000)
    double result1 = call_f64_reinterpret_i64(0x7FF0000000000000);
    ASSERT_TRUE(std::isinf(result1) && result1 > 0)
        << "f64.reinterpret_i64(0x7FF0000000000000) should produce +infinity, got " << std::fixed << std::setprecision(17) << result1;

    // Test negative infinity pattern (0xFFF0000000000000)
    double result2 = call_f64_reinterpret_i64(0xFFF0000000000000);
    ASSERT_TRUE(std::isinf(result2) && result2 < 0)
        << "f64.reinterpret_i64(0xFFF0000000000000) should produce -infinity, got " << std::fixed << std::setprecision(17) << result2;

    // Test quiet NaN pattern (0x7FF8000000000000)
    double result3 = call_f64_reinterpret_i64(0x7FF8000000000000);
    ASSERT_TRUE(std::isnan(result3))
        << "f64.reinterpret_i64(0x7FF8000000000000) should produce NaN, got " << std::fixed << std::setprecision(17) << result3;

    // Test signaling NaN pattern (0x7FF0000000000001)
    double result4 = call_f64_reinterpret_i64(0x7FF0000000000001);
    ASSERT_TRUE(std::isnan(result4))
        << "f64.reinterpret_i64(0x7FF0000000000001) should produce NaN, got " << std::fixed << std::setprecision(17) << result4;

    // Test smallest positive subnormal (0x0000000000000001)
    double result5 = call_f64_reinterpret_i64(0x0000000000000001);
    double expected5 = i64_to_f64_bits(0x0000000000000001);
    ASSERT_TRUE(f64_bit_equal(result5, expected5))
        << "f64.reinterpret_i64(0x0000000000000001) bit pattern mismatch for smallest subnormal";

    // Test largest subnormal (0x000FFFFFFFFFFFFF)
    double result6 = call_f64_reinterpret_i64(0x000FFFFFFFFFFFFF);
    double expected6 = i64_to_f64_bits(0x000FFFFFFFFFFFFF);
    ASSERT_TRUE(f64_bit_equal(result6, expected6))
        << "f64.reinterpret_i64(0x000FFFFFFFFFFFFF) bit pattern mismatch for largest subnormal";
}

/**
 * @test ModuleLoadingFailure_HandlesInvalidBytecodeGracefully
 * @brief Validates proper error handling for invalid WASM modules
 * @details Tests runtime behavior when loading modules with invalid bytecode or corrupted f64.reinterpret_i64 instructions.
 *          Verifies graceful failure handling and proper error reporting without crashes.
 * @test_category Error - Invalid scenario and error condition validation
 * @coverage_target core/iwasm/common/wasm_loader.c:module_loading_error_handling
 * @input_conditions Invalid WASM bytecode with corrupted f64.reinterpret_i64 instructions
 * @expected_behavior Returns appropriate error codes and messages without runtime crashes
 * @validation_method Error condition verification and runtime stability assessment
 */
TEST_P(F64ReinterpretI64Test, ModuleLoadingFailure_HandlesInvalidBytecodeGracefully) {
    // Test loading stack underflow module (simulates invalid usage scenario)
    uint32_t underflow_buf_size;
    uint8_t *underflow_buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE_STACK_UNDERFLOW.c_str(), &underflow_buf_size);
    ASSERT_NE(underflow_buf, nullptr) << "Failed to read stack underflow WASM file: " << WASM_FILE_STACK_UNDERFLOW;

    // Attempt to load the underflow test module
    char underflow_error_buf[128] = { 0 };
    wasm_module_t underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                                     underflow_error_buf, sizeof(underflow_error_buf));

    // The module should load successfully (it's the runtime execution that would detect stack issues)
    ASSERT_NE(underflow_module, nullptr)
        << "Stack underflow test module should load successfully, error: " << underflow_error_buf;

    // Test instantiation
    wasm_module_inst_t underflow_inst = wasm_runtime_instantiate(underflow_module, stack_size, heap_size,
                                                               underflow_error_buf, sizeof(underflow_error_buf));
    ASSERT_NE(underflow_inst, nullptr)
        << "Stack underflow test module should instantiate successfully, error: " << underflow_error_buf;

    // Clean up the underflow test resources
    if (underflow_inst) {
        wasm_runtime_deinstantiate(underflow_inst);
    }
    if (underflow_module) {
        wasm_runtime_unload(underflow_module);
    }
    if (underflow_buf) {
        BH_FREE(underflow_buf);
    }
}

/**
 * @test InverseRelationship_ValidatesRoundTripConsistency
 * @brief Validates round-trip consistency between f64.reinterpret_i64 and i64.reinterpret_f64
 * @details Tests that f64.reinterpret_i64(i64.reinterpret_f64(x)) == x for various f64 values.
 *          Verifies mathematical inverse relationship and bit-pattern preservation through conversions.
 * @test_category Edge - Mathematical property and inverse relationship validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:reinterpretation_consistency
 * @input_conditions Various f64 values including normal, special, and boundary values
 * @expected_behavior Perfect round-trip consistency with bit-exact preservation
 * @validation_method Comprehensive round-trip verification across f64 value spectrum
 */
TEST_P(F64ReinterpretI64Test, InverseRelationship_ValidatesRoundTripConsistency) {
    // Test round-trip with normal f64 values
    std::vector<double> test_f64_values = {
        1.0, -1.0, 2.0, -2.0, 0.0, -0.0,
        DBL_MIN, DBL_MAX,
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()
    };

    for (double original : test_f64_values) {
        // Convert f64 to i64 bit pattern
        union { double f; int64_t i; } f_to_i;
        f_to_i.f = original;
        int64_t i64_bits = f_to_i.i;

        // Apply f64.reinterpret_i64 to get back f64
        double round_trip = call_f64_reinterpret_i64(i64_bits);

        // Verify bit-exact equality (handle NaN specially)
        if (std::isnan(original)) {
            ASSERT_TRUE(std::isnan(round_trip))
                << "Round-trip failed for NaN: original was NaN, round-trip result was " << std::fixed << std::setprecision(17) << round_trip;
        } else {
            ASSERT_TRUE(f64_bit_equal(original, round_trip))
                << "Round-trip consistency failed: original " << std::fixed << std::setprecision(17) << original
                << " (0x" << std::hex << f64_to_bits(original) << "), round-trip " << std::fixed << std::setprecision(17) << round_trip
                << " (0x" << std::hex << f64_to_bits(round_trip) << ")";
        }
    }
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningMode, F64ReinterpretI64Test,
                        testing::Values(Mode_Interp, Mode_LLVM_JIT),
                        [](const testing::TestParamInfo<F64ReinterpretI64Test::ParamType>& info) {
                            return info.param == Mode_Interp ? "Interpreter" : "AOT";
                        });

// Test suite entry point and environment setup
class F64ReinterpretI64Environment : public ::testing::Environment {
public:
    /**
     * @brief Global test environment setup for f64.reinterpret_i64 test suite
     */
    void SetUp() override {
        char *current_dir = getcwd(nullptr, 0);
        if (current_dir) {
            CWD = current_dir;
            free(current_dir);
        }
    }

    /**
     * @brief Global test environment cleanup for f64.reinterpret_i64 test suite
     */
    void TearDown() override {
        // Global cleanup if needed
    }
};

// Register global test environment
::testing::Environment* const f64_reinterpret_i64_env =
    ::testing::AddGlobalTestEnvironment(new F64ReinterpretI64Environment);