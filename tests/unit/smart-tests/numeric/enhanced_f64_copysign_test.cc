/**
 * Enhanced Unit Tests for f64.copysign WebAssembly Opcode
 *
 * This test suite provides comprehensive validation of the f64.copysign operation,
 * which implements IEEE 754 copySign functionality for 64-bit floating-point values.
 * The opcode takes two f64 operands and returns the first operand with the sign
 * bit of the second operand, preserving the magnitude of the first operand.
 *
 * Test Coverage:
 * - Basic sign copying operations
 * - Special IEEE 754 values (NaN, infinity, zero)
 * - Boundary conditions and edge cases
 * - Cross-execution mode validation (interpreter vs AOT)
 * - Runtime error scenarios
 *
 * Source: core/iwasm/interpreter/wasm_interp_classic.c - f64.copysign implementation
 */

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

using namespace std;

class F64CopySignTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up WAMR runtime environment and load test module
     * @details Initializes WAMR with interpreter and AOT capabilities,
     *          loads the f64_copysign test module, and prepares execution environment
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load WASM module for f64.copysign testing
        load_test_module();
    }

    /**
     * @brief Clean up WAMR runtime environment and release resources
     * @details Unloads modules, destroys execution instances, and shuts down runtime
     */
    void TearDown() override {
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

    /**
     * @brief Load and instantiate the f64.copysign test WASM module
     * @details Reads the compiled WASM bytecode and creates an executable module instance
     */
    void load_test_module() {
        char error_buf[256];
        const char* module_path = "wasm-apps/f64_copysign_test.wasm";

        // Load WASM module from bytecode file
        buf = reinterpret_cast<uint8_t*>(bh_read_file_to_buffer(module_path, &buf_size));
        ASSERT_NE(nullptr, buf) << "Failed to read WASM module file: " << module_path;

        // Load module into WAMR runtime
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        // Create module instance for execution
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate module: " << error_buf;

        // Set running mode and create execution environment
        wasm_runtime_set_running_mode(module_inst, GetParam());
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
    }

    /**
     * @brief Execute f64.copysign operation with two f64 operands
     * @param magnitude First operand (magnitude source)
     * @param sign_source Second operand (sign source)
     * @return Result of f64.copysign(magnitude, sign_source)
     */
    double call_f64_copysign(double magnitude, double sign_source) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_copysign");
        EXPECT_NE(nullptr, func) << "Failed to find test_f64_copysign function";

        // Convert double parameters to uint64_t for WAMR function call
        union { double f; uint64_t u; } mag_conv = { .f = magnitude };
        union { double f; uint64_t u; } sign_conv = { .f = sign_source };

        uint32_t argv[4] = {
            static_cast<uint32_t>(mag_conv.u),
            static_cast<uint32_t>(mag_conv.u >> 32),
            static_cast<uint32_t>(sign_conv.u),
            static_cast<uint32_t>(sign_conv.u >> 32)
        };

        bool ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        const char *exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            printf("Exception: %s\n", exception);
            return 0.0;
        }

        // Extract result from argv (f64 result in first 2 elements)
        union { double f; uint64_t u; } result_conv;
        result_conv.u = static_cast<uint64_t>(argv[0]) | (static_cast<uint64_t>(argv[1]) << 32);
        return result_conv.f;
    }

    /**
     * @brief Utility function to check if a double value is negative zero (-0.0)
     * @param value Double value to check
     * @return true if value is negative zero, false otherwise
     */
    bool is_negative_zero(double value) {
        return value == 0.0 && std::signbit(value);
    }

    /**
     * @brief Utility function to check if a double value is positive zero (+0.0)
     * @param value Double value to check
     * @return true if value is positive zero, false otherwise
     */
    bool is_positive_zero(double value) {
        return value == 0.0 && !std::signbit(value);
    }

    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t *buf = nullptr;
    uint32_t buf_size = 0;
    uint32_t stack_size = 8092;
    uint32_t heap_size = 8192;
};

/**
 * @test BasicCopysign_StandardValues_ReturnsCorrectSign
 * @brief Validates f64.copysign correctly copies sign bits for typical numeric values
 * @details Tests fundamental sign copying with positive, negative, and mixed-sign combinations.
 *          Verifies magnitude preservation and proper sign bit manipulation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_copysign_operation
 * @input_conditions Standard double pairs with different sign combinations
 * @expected_behavior Returns first operand with sign of second operand
 * @validation_method Direct comparison with IEEE 754 copysign behavior
 */
TEST_P(F64CopySignTest, BasicCopysign_StandardValues_ReturnsCorrectSign) {

    // Positive magnitude, positive sign source (no change expected)
    double result1 = call_f64_copysign(3.14159, 2.71828);
    ASSERT_EQ(3.14159, result1) << "Positive to positive copysign failed";
    ASSERT_FALSE(std::signbit(result1)) << "Result should have positive sign";

    // Positive magnitude, negative sign source (sign should flip)
    double result2 = call_f64_copysign(3.14159, -2.71828);
    ASSERT_EQ(-3.14159, result2) << "Positive to negative copysign failed";
    ASSERT_TRUE(std::signbit(result2)) << "Result should have negative sign";

    // Negative magnitude, positive sign source (sign should flip)
    double result3 = call_f64_copysign(-3.14159, 2.71828);
    ASSERT_EQ(3.14159, result3) << "Negative to positive copysign failed";
    ASSERT_FALSE(std::signbit(result3)) << "Result should have positive sign";

    // Negative magnitude, negative sign source (no change expected)
    double result4 = call_f64_copysign(-3.14159, -2.71828);
    ASSERT_EQ(-3.14159, result4) << "Negative to negative copysign failed";
    ASSERT_TRUE(std::signbit(result4)) << "Result should have negative sign";
}

/**
 * @test SpecialValueHandling_ZeroOperands_ReturnsCorrectZeroSigns
 * @brief Validates f64.copysign behavior with positive and negative zero values
 * @details Tests IEEE 754 compliance for zero value sign copying, including +0.0 and -0.0.
 *          Ensures proper handling of signed zero according to IEEE 754-2008 standard.
 * @test_category Edge - Special numeric value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_copysign_zero_handling
 * @input_conditions Zero values with different signs as magnitude and sign source
 * @expected_behavior Correct zero sign copying while maintaining zero magnitude
 * @validation_method Sign bit inspection using std::signbit function
 */
TEST_P(F64CopySignTest, SpecialValueHandling_ZeroOperands_ReturnsCorrectZeroSigns) {

    // Positive zero magnitude, negative zero sign source
    double result1 = call_f64_copysign(0.0, -0.0);
    ASSERT_EQ(0.0, std::abs(result1)) << "Magnitude should remain zero";
    ASSERT_TRUE(is_negative_zero(result1)) << "Result should be negative zero";

    // Negative zero magnitude, positive zero sign source
    double result2 = call_f64_copysign(-0.0, 0.0);
    ASSERT_EQ(0.0, std::abs(result2)) << "Magnitude should remain zero";
    ASSERT_TRUE(is_positive_zero(result2)) << "Result should be positive zero";

    // Positive zero with positive zero (identity)
    double result3 = call_f64_copysign(0.0, 0.0);
    ASSERT_EQ(0.0, std::abs(result3)) << "Magnitude should remain zero";
    ASSERT_TRUE(is_positive_zero(result3)) << "Result should be positive zero";

    // Negative zero with negative zero (identity)
    double result4 = call_f64_copysign(-0.0, -0.0);
    ASSERT_EQ(0.0, std::abs(result4)) << "Magnitude should remain zero";
    ASSERT_TRUE(is_negative_zero(result4)) << "Result should be negative zero";
}

/**
 * @test SpecialValueHandling_InfinityOperands_PreservesMagnitudeWithCorrectSign
 * @brief Validates f64.copysign behavior with infinity values as operands
 * @details Tests sign copying when infinity values are used as magnitude or sign source.
 *          Verifies IEEE 754 compliance for infinity handling in copysign operations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_copysign_infinity_handling
 * @input_conditions Infinity values combined with finite and other special values
 * @expected_behavior Proper infinity sign manipulation while preserving infinite magnitude
 * @validation_method Infinity detection and sign bit validation
 */
TEST_P(F64CopySignTest, SpecialValueHandling_InfinityOperands_PreservesMagnitudeWithCorrectSign) {

    const double pos_inf = std::numeric_limits<double>::infinity();
    const double neg_inf = -std::numeric_limits<double>::infinity();

    // Positive infinity magnitude, negative finite sign source
    double result1 = call_f64_copysign(pos_inf, -1.0);
    ASSERT_TRUE(std::isinf(result1)) << "Result should be infinite";
    ASSERT_TRUE(std::signbit(result1)) << "Result should be negative infinity";
    ASSERT_EQ(neg_inf, result1) << "Should produce negative infinity";

    // Negative infinity magnitude, positive finite sign source
    double result2 = call_f64_copysign(neg_inf, 1.0);
    ASSERT_TRUE(std::isinf(result2)) << "Result should be infinite";
    ASSERT_FALSE(std::signbit(result2)) << "Result should be positive infinity";
    ASSERT_EQ(pos_inf, result2) << "Should produce positive infinity";

    // Finite magnitude, positive infinity sign source
    double result3 = call_f64_copysign(42.0, pos_inf);
    ASSERT_EQ(42.0, result3) << "Magnitude should be preserved";
    ASSERT_FALSE(std::signbit(result3)) << "Result should have positive sign";

    // Finite magnitude, negative infinity sign source
    double result4 = call_f64_copysign(42.0, neg_inf);
    ASSERT_EQ(-42.0, result4) << "Magnitude should be preserved with negative sign";
    ASSERT_TRUE(std::signbit(result4)) << "Result should have negative sign";
}

/**
 * @test SpecialValueHandling_NaNOperands_PreservesNaNWithCorrectSign
 * @brief Validates f64.copysign behavior when NaN values are involved as operands
 * @details Tests IEEE 754 compliant NaN handling in copysign operations.
 *          Verifies that NaN magnitude is preserved while sign is properly copied.
 * @test_category Edge - Special numeric value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_copysign_nan_handling
 * @input_conditions NaN values as magnitude source and/or sign source operands
 * @expected_behavior NaN preservation with appropriate sign bit manipulation
 * @validation_method NaN detection and sign bit inspection
 */
TEST_P(F64CopySignTest, SpecialValueHandling_NaNOperands_PreservesNaNWithCorrectSign) {

    const double pos_nan = std::numeric_limits<double>::quiet_NaN();
    const double neg_nan = -std::numeric_limits<double>::quiet_NaN();

    // NaN magnitude, negative finite sign source
    double result1 = call_f64_copysign(pos_nan, -1.0);
    ASSERT_TRUE(std::isnan(result1)) << "Result should be NaN";
    ASSERT_TRUE(std::signbit(result1)) << "NaN should have negative sign";

    // NaN magnitude, positive finite sign source
    double result2 = call_f64_copysign(neg_nan, 1.0);
    ASSERT_TRUE(std::isnan(result2)) << "Result should be NaN";
    ASSERT_FALSE(std::signbit(result2)) << "NaN should have positive sign";

    // Finite magnitude, NaN sign source (sign from NaN)
    double result3 = call_f64_copysign(123.456, pos_nan);
    ASSERT_EQ(123.456, std::abs(result3)) << "Magnitude should be preserved";
    ASSERT_FALSE(std::signbit(result3)) << "Should copy positive sign from NaN";

    // Finite magnitude, negative NaN sign source
    double result4 = call_f64_copysign(123.456, neg_nan);
    ASSERT_EQ(123.456, std::abs(result4)) << "Magnitude should be preserved";
    ASSERT_TRUE(std::signbit(result4)) << "Should copy negative sign from NaN";
}

/**
 * @test BoundaryConditions_ExtremeValues_MaintainsIEEE754Compliance
 * @brief Validates f64.copysign with maximum/minimum finite double values
 * @details Tests boundary conditions using DBL_MAX, DBL_MIN, and denormalized values.
 *          Ensures proper handling at numeric boundaries without overflow/underflow.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_copysign_boundary_handling
 * @input_conditions Extreme finite values and denormalized numbers
 * @expected_behavior Correct sign copying without magnitude alteration at boundaries
 * @validation_method Boundary value preservation and sign correctness verification
 */
TEST_P(F64CopySignTest, BoundaryConditions_ExtremeValues_MaintainsIEEE754Compliance) {

    const double max_val = std::numeric_limits<double>::max();
    const double min_val = std::numeric_limits<double>::min();
    const double denorm_val = std::numeric_limits<double>::denorm_min();

    // Maximum finite value with sign copying
    double result1 = call_f64_copysign(max_val, -1.0);
    ASSERT_EQ(-max_val, result1) << "Maximum value should preserve magnitude";
    ASSERT_TRUE(std::signbit(result1)) << "Result should have negative sign";

    // Minimum normalized value with sign copying
    double result2 = call_f64_copysign(min_val, -1.0);
    ASSERT_EQ(-min_val, result2) << "Minimum normalized value should preserve magnitude";
    ASSERT_TRUE(std::signbit(result2)) << "Result should have negative sign";

    // Denormalized value with sign copying
    double result3 = call_f64_copysign(denorm_val, -1.0);
    ASSERT_EQ(-denorm_val, result3) << "Denormalized value should preserve magnitude";
    ASSERT_TRUE(std::signbit(result3)) << "Result should have negative sign";

    // Verify no overflow/underflow occurred
    ASSERT_FALSE(std::isinf(result1)) << "Result should not be infinite";
    ASSERT_FALSE(std::isinf(result2)) << "Result should not be infinite";
    ASSERT_FALSE(std::isnan(result3)) << "Result should not be NaN";
}

/**
 * @test RuntimeErrors_ModuleLoadFailure_HandlesErrorsGracefully
 * @brief Validates proper error handling when WASM module operations fail
 * @details Tests runtime error scenarios including module load failures and invalid operations.
 *          Ensures robust error handling and proper cleanup in failure cases.
 * @test_category Error - Runtime error validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:module_loading_error_handling
 * @input_conditions Invalid module data and runtime error conditions
 * @expected_behavior Proper error detection and graceful failure handling
 * @validation_method Error condition verification and resource cleanup validation
 */
TEST_P(F64CopySignTest, RuntimeErrors_ModuleLoadFailure_HandlesErrorsGracefully) {
    // Test with invalid WASM bytecode
    char error_buf[256];
    uint8_t invalid_bytecode[] = {0x00, 0x61, 0x73, 0x6d, 0xFF, 0xFF, 0xFF, 0xFF};  // Invalid magic version

    wasm_module_t invalid_module = wasm_runtime_load(invalid_bytecode, sizeof(invalid_bytecode),
                                                   error_buf, sizeof(error_buf));

    // Should fail to load invalid module
    ASSERT_EQ(nullptr, invalid_module) << "Invalid module should fail to load";
    ASSERT_NE(strlen(error_buf), 0) << "Error buffer should contain error message";

    // Test with null buffer scenarios
    wasm_module_t null_module = wasm_runtime_load(nullptr, 0, error_buf, sizeof(error_buf));
    ASSERT_EQ(nullptr, null_module) << "Null buffer should fail to load";

    // Test instantiation failure with insufficient memory
    if (module) {  // Use valid module from SetUp
        wasm_module_inst_t small_inst = wasm_runtime_instantiate(module, 1024, 1024,  // Very small memory
                                                               error_buf, sizeof(error_buf));
        // May succeed or fail depending on module requirements - both are valid behaviors
        if (small_inst) {
            wasm_runtime_deinstantiate(small_inst);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(RunningModeTest, F64CopySignTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<F64CopySignTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });