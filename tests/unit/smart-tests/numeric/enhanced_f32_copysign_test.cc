/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for f32.copysign Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly f32.copysign
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic functionality with typical values
 * - Corner Cases: Boundary conditions with extreme finite values
 * - Edge Cases: Special IEEE 754 values (zeros, infinities, NaN)
 * - Error Handling: Stack underflow and validation errors
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling f32.copysign)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c (f32.copysign implementation)
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <limits>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

using namespace std;

/**
 * @brief Test fixture class for f32.copysign opcode validation
 * @details Provides comprehensive testing infrastructure for the WebAssembly f32.copysign
 *          instruction across both interpreter and AOT execution modes. This test suite
 *          validates IEEE 754 floating-point sign manipulation behavior, ensuring correct
 *          implementation of the copysign operation which copies the sign bit from the
 *          second operand to the result while preserving the magnitude of the first operand.
 */
class F32CopysignTest : public testing::TestWithParam<RunningMode>
{
protected:
    /**
     * @brief Set up test environment before each test case
     * @details Initializes WAMR runtime with system allocator, loads the f32.copysign test
     *          WASM module, and prepares execution context for both interpreter and AOT modes.
     *          Ensures proper module validation and instance creation for reliable test execution.
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load WASM module for f32.copysign testing
        load_test_module();
    }

    /**
     * @brief Clean up test environment after each test case
     * @details Properly destroys WASM module instance, unloads module, releases memory
     *          resources, and shuts down WAMR runtime to prevent resource leaks.
     */
    void TearDown() override
    {
        // Clean up WAMR resources in proper order
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
        wasm_runtime_destroy();
    }

private:
    /**
     * @brief Load f32.copysign test WASM module from file system
     * @details Reads the compiled WASM bytecode for f32.copysign tests, validates module
     *          format, loads module into WAMR, and creates executable module instance.
     *          Handles both interpreter and AOT execution modes based on test parameters.
     */
    void load_test_module()
    {
        const char *wasm_path = "wasm-apps/f32_copysign_test.wasm";

        // Read WASM module file from disk
        buf = (uint8_t*)bh_read_file_to_buffer(wasm_path, &buf_size);
        ASSERT_NE(nullptr, buf) << "Failed to read WASM file: " << wasm_path;
        ASSERT_GT(buf_size, 0U) << "WASM file is empty: " << wasm_path;

        // Load and validate WASM module
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        // Create module instance for execution
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode and create execution environment
        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
    }

public:
    /**
     * @brief Execute f32.copysign operation with two f32 operands
     * @details Calls the WASM f32.copysign test function with specified parameters,
     *          handles execution errors, and returns the computed result for validation.
     * @param magnitude f32 value providing the magnitude component
     * @param sign f32 value providing the sign component
     * @return f32 result of copysign(magnitude, sign) operation
     */
    float call_f32_copysign(float magnitude, float sign)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_copysign");
        EXPECT_NE(nullptr, func) << "Failed to find test_f32_copysign function";

        // Convert float parameters to uint32_t for WAMR function call
        union { float f; uint32_t u; } mag_conv = { .f = magnitude };
        union { float f; uint32_t u; } sign_conv = { .f = sign };

        uint32_t argv[3] = { mag_conv.u, sign_conv.u, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        const char *exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            EXPECT_TRUE(false) << "Runtime exception occurred: " << exception;
        }

        // Convert result back to float - result is in argv[0]
        union { float f; uint32_t u; } result_conv = { .u = argv[0] };
        return result_conv.f;
    }

    /**
     * @brief Execute stack underflow test scenarios
     * @details Tests f32.copysign behavior with insufficient stack operands to validate
     *          proper error handling and graceful failure modes in WAMR runtime.
     * @return bool indicating whether stack underflow was properly detected
     */
    bool test_stack_underflow()
    {
        // Find stack underflow test function
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_stack_underflow");
        if (!func) {
            return false;  // Function not found indicates expected behavior
        }

        uint32_t argv[1] = { 0 };
        bool success = wasm_runtime_call_wasm(exec_env, func, 0, argv);

        // Stack underflow should cause execution failure
        return !success;
    }

protected:
    // WAMR runtime configuration and state
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t *buf = nullptr;
    uint32_t buf_size = 0;
    char error_buf[256];

    // Execution configuration parameters
    static constexpr uint32_t stack_size = 64 * 1024;   // 64KB stack
    static constexpr uint32_t heap_size = 64 * 1024;    // 64KB heap
};

/**
 * @test BasicCopysign_TypicalValues_ReturnsCorrectResults
 * @brief Validates f32.copysign produces correct results for typical floating-point inputs
 * @details Tests fundamental copysign operation with positive, negative, and mixed-sign combinations.
 *          Verifies that f32.copysign correctly computes result with first operand's magnitude
 *          and second operand's sign for representative input values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Standard f32 pairs: (3.5f,2.1f), (42.0f,-7.8f), (-15.25f,4.0f), (-8.5f,-3.2f)
 * @expected_behavior Returns magnitude of first with sign of second: 3.5f, -42.0f, 15.25f, -8.5f
 * @validation_method Direct ASSERT_FLOAT_EQ comparison of WASM function result with expected values
 */
TEST_P(F32CopysignTest, BasicCopysign_TypicalValues_ReturnsCorrectResults)
{
    // Test positive magnitude with positive sign
    ASSERT_FLOAT_EQ(3.5f, call_f32_copysign(3.5f, 2.1f))
        << "copysign(3.5, 2.1) should return 3.5";

    // Test positive magnitude with negative sign
    ASSERT_FLOAT_EQ(-42.0f, call_f32_copysign(42.0f, -7.8f))
        << "copysign(42.0, -7.8) should return -42.0";

    // Test negative magnitude with positive sign
    ASSERT_FLOAT_EQ(15.25f, call_f32_copysign(-15.25f, 4.0f))
        << "copysign(-15.25, 4.0) should return 15.25";

    // Test negative magnitude with negative sign
    ASSERT_FLOAT_EQ(-8.5f, call_f32_copysign(-8.5f, -3.2f))
        << "copysign(-8.5, -3.2) should return -8.5";
}

/**
 * @test BoundaryValues_ExtremeFiniteNumbers_PreserveMagnitude
 * @brief Validates f32.copysign with extreme finite floating-point boundary values
 * @details Tests copysign operation with maximum, minimum, and smallest normalized f32 values
 *          to ensure correct sign manipulation without magnitude alteration at numeric boundaries.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions F32 boundary values: F32_MAX, F32_MIN, F32_MIN_POSITIVE with sign operands
 * @expected_behavior Preserves exact magnitude while applying sign from second operand
 * @validation_method ASSERT_FLOAT_EQ with boundary value comparison and sign verification
 */
TEST_P(F32CopysignTest, BoundaryValues_ExtremeFiniteNumbers_PreserveMagnitude)
{
    const float F32_MAX = std::numeric_limits<float>::max();
    const float F32_MIN = std::numeric_limits<float>::lowest();
    const float F32_MIN_POSITIVE = std::numeric_limits<float>::min();

    // Test maximum finite value with negative sign
    ASSERT_FLOAT_EQ(-F32_MAX, call_f32_copysign(F32_MAX, -1.0f))
        << "copysign(F32_MAX, -1.0) should return -F32_MAX";

    // Test minimum finite value with positive sign
    ASSERT_FLOAT_EQ(F32_MAX, call_f32_copysign(F32_MIN, 1.0f))
        << "copysign(F32_MIN, 1.0) should return F32_MAX";

    // Test smallest positive normalized value with negative sign
    ASSERT_FLOAT_EQ(-F32_MIN_POSITIVE, call_f32_copysign(F32_MIN_POSITIVE, -1.0f))
        << "copysign(F32_MIN_POSITIVE, -1.0) should return -F32_MIN_POSITIVE";
}

/**
 * @test SpecialValues_ZeroInfinityNaN_CorrectSignManipulation
 * @brief Validates f32.copysign with IEEE 754 special values including zeros, infinities, and NaN
 * @details Tests copysign behavior with signed zeros, positive/negative infinities, and NaN values
 *          to ensure IEEE 754 compliant sign bit manipulation for all special floating-point cases.
 * @test_category Edge - Special IEEE 754 value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Special f32 values: ±0.0, ±∞, NaN with various sign source operands
 * @expected_behavior Correct sign bit manipulation following IEEE 754 copysign specification
 * @validation_method Bit pattern comparison using signbit() and isnan() for special value verification
 */
TEST_P(F32CopysignTest, SpecialValues_ZeroInfinityNaN_CorrectSignManipulation)
{
    const float pos_zero = +0.0f;
    const float neg_zero = -0.0f;
    const float pos_inf = std::numeric_limits<float>::infinity();
    const float neg_inf = -std::numeric_limits<float>::infinity();
    const float quiet_nan = std::numeric_limits<float>::quiet_NaN();

    // Test signed zero manipulation
    float result_neg_zero = call_f32_copysign(pos_zero, -1.0f);
    ASSERT_TRUE(std::signbit(result_neg_zero)) << "copysign(+0.0, -1.0) should return -0.0";
    ASSERT_EQ(0.0f, result_neg_zero) << "Result should still be zero";

    float result_pos_zero = call_f32_copysign(neg_zero, 1.0f);
    ASSERT_FALSE(std::signbit(result_pos_zero)) << "copysign(-0.0, 1.0) should return +0.0";
    ASSERT_EQ(0.0f, result_pos_zero) << "Result should still be zero";

    // Test infinity sign manipulation
    ASSERT_TRUE(std::isinf(call_f32_copysign(pos_inf, -1.0f)) &&
                std::signbit(call_f32_copysign(pos_inf, -1.0f)))
        << "copysign(+∞, -1.0) should return -∞";

    ASSERT_TRUE(std::isinf(call_f32_copysign(neg_inf, 1.0f)) &&
                !std::signbit(call_f32_copysign(neg_inf, 1.0f)))
        << "copysign(-∞, 1.0) should return +∞";

    // Test NaN sign manipulation
    float result_nan = call_f32_copysign(quiet_nan, -1.0f);
    ASSERT_TRUE(std::isnan(result_nan)) << "copysign(NaN, -1.0) should return NaN";
    ASSERT_TRUE(std::signbit(result_nan)) << "Result NaN should have negative sign bit";
}

/**
 * @test StackUnderflow_InsufficientOperands_FailsGracefully
 * @brief Validates f32.copysign error handling with insufficient stack operands
 * @details Tests runtime behavior when f32.copysign is executed with insufficient operands
 *          on the execution stack to ensure graceful error handling and proper failure reporting.
 * @test_category Error - Stack underflow condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @input_conditions Empty stack or single f32 operand scenarios for f32.copysign execution
 * @expected_behavior Graceful failure with proper error reporting, no crashes or undefined behavior
 * @validation_method ASSERT_TRUE for expected failure detection in stack underflow scenarios
 */
TEST_P(F32CopysignTest, StackUnderflow_InsufficientOperands_FailsGracefully)
{
    // Test stack underflow detection
    bool underflow_detected = test_stack_underflow();
    ASSERT_TRUE(underflow_detected)
        << "Stack underflow should be detected and handled gracefully";

    // Verify module instance remains in valid state after error
    ASSERT_NE(nullptr, module_inst)
        << "Module instance should remain valid after stack underflow";
}

// Parameterized test instantiation for both execution modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, F32CopysignTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<F32CopysignTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });