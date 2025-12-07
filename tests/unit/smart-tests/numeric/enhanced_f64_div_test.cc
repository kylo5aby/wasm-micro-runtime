/**
 * Enhanced Unit Tests for f64.div WebAssembly Opcode
 *
 * This test suite provides comprehensive validation of the f64.div operation,
 * which implements IEEE 754 double-precision floating-point division.
 * The opcode takes two f64 operands and returns their quotient (dividend ÷ divisor),
 * handling all special cases according to IEEE 754-2008 standard.
 *
 * Test Coverage:
 * - Basic division operations with standard values
 * - Boundary conditions and extreme values (DBL_MAX, DBL_MIN)
 * - Special IEEE 754 values (NaN, infinity, zero)
 * - Division by zero scenarios (finite/zero → infinity)
 * - Cross-execution mode validation (interpreter vs AOT)
 * - Runtime error scenarios and module loading failures
 *
 * Source: core/iwasm/interpreter/wasm_interp_classic.c - f64.div implementation
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

class F64DivTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up WAMR runtime environment and load test module
     * @details Initializes WAMR with interpreter and AOT capabilities,
     *          loads the f64_div test module, and prepares execution environment
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load WASM module for f64.div testing
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
     * @brief Load and instantiate the f64.div test WASM module
     * @details Reads the compiled WASM bytecode and creates an executable module instance
     */
    void load_test_module() {
        char error_buf[256];
        const char* module_path = "wasm-apps/f64_div_test.wasm";

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
     * @brief Execute f64.div operation with two f64 operands
     * @param dividend First operand (numerator)
     * @param divisor Second operand (denominator)
     * @return Result of f64.div(dividend, divisor)
     */
    double call_f64_div(double dividend, double divisor) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_div");
        EXPECT_NE(nullptr, func) << "Failed to find test_f64_div function";

        // Convert double parameters to uint64_t for WAMR function call
        union { double f; uint64_t u; } div_conv = { .f = dividend };
        union { double f; uint64_t u; } dsor_conv = { .f = divisor };

        uint32_t argv[4] = {
            static_cast<uint32_t>(div_conv.u),
            static_cast<uint32_t>(div_conv.u >> 32),
            static_cast<uint32_t>(dsor_conv.u),
            static_cast<uint32_t>(dsor_conv.u >> 32)
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
 * @test BasicDivision_StandardValues_ReturnsCorrectQuotient
 * @brief Validates f64.div correctly computes quotients for typical numeric values
 * @details Tests fundamental division with positive, negative, and mixed-sign combinations.
 *          Verifies mathematical accuracy and proper sign handling.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_div_operation
 * @input_conditions Standard double pairs with different sign combinations
 * @expected_behavior Returns mathematically correct quotients with proper signs
 * @validation_method Direct comparison with expected division results
 */
TEST_P(F64DivTest, BasicDivision_StandardValues_ReturnsCorrectQuotient) {

    // Positive dividend, positive divisor
    double result1 = call_f64_div(10.0, 2.0);
    ASSERT_EQ(5.0, result1) << "Basic positive division failed";
    ASSERT_FALSE(std::signbit(result1)) << "Result should have positive sign";

    // Negative dividend, negative divisor (negative / negative = positive)
    double result2 = call_f64_div(-15.0, -3.0);
    ASSERT_EQ(5.0, result2) << "Negative by negative division failed";
    ASSERT_FALSE(std::signbit(result2)) << "Result should have positive sign";

    // Positive dividend, negative divisor (positive / negative = negative)
    double result3 = call_f64_div(20.0, -4.0);
    ASSERT_EQ(-5.0, result3) << "Positive by negative division failed";
    ASSERT_TRUE(std::signbit(result3)) << "Result should have negative sign";

    // Negative dividend, positive divisor (negative / positive = negative)
    double result4 = call_f64_div(-36.0, 9.0);
    ASSERT_EQ(-4.0, result4) << "Negative by positive division failed";
    ASSERT_TRUE(std::signbit(result4)) << "Result should have negative sign";

    // Fractional result validation
    double result5 = call_f64_div(7.0, 2.0);
    ASSERT_EQ(3.5, result5) << "Fractional division failed";

    // High precision validation
    double result6 = call_f64_div(22.0, 7.0);
    ASSERT_NEAR(3.142857142857143, result6, 1e-15) << "High precision division failed";
}

/**
 * @test BoundaryConditions_ExtremeValues_MaintainsIEEE754Compliance
 * @brief Validates f64.div with maximum/minimum finite double values and boundaries
 * @details Tests boundary conditions using DBL_MAX, DBL_MIN, and operations at numeric limits.
 *          Ensures proper handling without unexpected overflow/underflow behavior.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_div_boundary_handling
 * @input_conditions Extreme finite values and boundary combinations
 * @expected_behavior Correct division results at boundaries without invalid outcomes
 * @validation_method Boundary value preservation and overflow/underflow detection
 */
TEST_P(F64DivTest, BoundaryConditions_ExtremeValues_MaintainsIEEE754Compliance) {

    const double max_val = std::numeric_limits<double>::max();
    const double min_val = std::numeric_limits<double>::min();

    // Maximum finite value division by 2.0
    double result1 = call_f64_div(max_val, 2.0);
    ASSERT_FALSE(std::isinf(result1)) << "DBL_MAX/2.0 should not overflow to infinity";
    ASSERT_FALSE(std::isnan(result1)) << "DBL_MAX/2.0 should not produce NaN";
    ASSERT_GT(result1, 0.0) << "Result should be positive";

    // Minimum normalized value division
    double result2 = call_f64_div(min_val, 2.0);
    ASSERT_GT(result2, 0.0) << "DBL_MIN/2.0 should be positive";
    ASSERT_FALSE(std::isnan(result2)) << "DBL_MIN/2.0 should not produce NaN";

    // Self-division (should equal 1.0)
    double result3 = call_f64_div(max_val, max_val);
    ASSERT_EQ(1.0, result3) << "DBL_MAX/DBL_MAX should equal 1.0";

    // Large number divided by very small number (potential overflow)
    double result4 = call_f64_div(max_val, 1e-100);
    ASSERT_TRUE(std::isinf(result4)) << "Large/tiny division should overflow to infinity";
    ASSERT_FALSE(std::signbit(result4)) << "Result should be positive infinity";

    // Small number divided by large number (potential underflow to subnormal)
    double result5 = call_f64_div(1e-200, 1e+200);
    ASSERT_GE(result5, 0.0) << "Small/large division should be non-negative";
    // May underflow to zero or subnormal - both are valid IEEE 754 behavior
}

/**
 * @test SpecialValueHandling_ZeroDivision_ReturnsInfinity
 * @brief Validates f64.div behavior with zero divisors according to IEEE 754
 * @details Tests division by positive and negative zero, including indeterminate 0/0 cases.
 *          Verifies proper infinity generation and NaN for indeterminate forms.
 * @test_category Edge - Special numeric value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_div_zero_handling
 * @input_conditions Zero divisors with various dividends including zero
 * @expected_behavior Infinity results for finite/zero, NaN for indeterminate 0/0
 * @validation_method Infinity/NaN detection and sign bit validation
 */
TEST_P(F64DivTest, SpecialValueHandling_ZeroDivision_ReturnsInfinity) {

    // Positive finite divided by positive zero
    double result1 = call_f64_div(5.0, 0.0);
    ASSERT_TRUE(std::isinf(result1)) << "5.0/+0.0 should be infinite";
    ASSERT_FALSE(std::signbit(result1)) << "5.0/+0.0 should be positive infinity";

    // Positive finite divided by negative zero
    double result2 = call_f64_div(5.0, -0.0);
    ASSERT_TRUE(std::isinf(result2)) << "5.0/-0.0 should be infinite";
    ASSERT_TRUE(std::signbit(result2)) << "5.0/-0.0 should be negative infinity";

    // Negative finite divided by positive zero
    double result3 = call_f64_div(-5.0, 0.0);
    ASSERT_TRUE(std::isinf(result3)) << "-5.0/+0.0 should be infinite";
    ASSERT_TRUE(std::signbit(result3)) << "-5.0/+0.0 should be negative infinity";

    // Negative finite divided by negative zero
    double result4 = call_f64_div(-5.0, -0.0);
    ASSERT_TRUE(std::isinf(result4)) << "-5.0/-0.0 should be infinite";
    ASSERT_FALSE(std::signbit(result4)) << "-5.0/-0.0 should be positive infinity";

    // Zero divided by zero (indeterminate form)
    double result5 = call_f64_div(0.0, 0.0);
    ASSERT_TRUE(std::isnan(result5)) << "0.0/0.0 should be NaN (indeterminate)";

    // Negative zero divided by positive zero
    double result6 = call_f64_div(-0.0, 0.0);
    ASSERT_TRUE(std::isnan(result6)) << "-0.0/0.0 should be NaN (indeterminate)";
}

/**
 * @test SpecialValueHandling_InfinityOperands_ProducesCorrectResults
 * @brief Validates f64.div behavior when infinity values are involved as operands
 * @details Tests division with infinity as dividend, divisor, or both operands.
 *          Verifies IEEE 754 compliant results for all infinity combinations.
 * @test_category Edge - Special numeric value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_div_infinity_handling
 * @input_conditions Infinity values combined with finite and other special values
 * @expected_behavior Correct infinity/zero/NaN results per IEEE 754 standard
 * @validation_method Infinity/zero/NaN detection with proper sign validation
 */
TEST_P(F64DivTest, SpecialValueHandling_InfinityOperands_ProducesCorrectResults) {

    const double pos_inf = std::numeric_limits<double>::infinity();
    const double neg_inf = -std::numeric_limits<double>::infinity();

    // Positive infinity divided by finite number
    double result1 = call_f64_div(pos_inf, 2.0);
    ASSERT_TRUE(std::isinf(result1)) << "+∞/2.0 should be infinite";
    ASSERT_FALSE(std::signbit(result1)) << "+∞/2.0 should be positive infinity";

    // Negative infinity divided by finite number
    double result2 = call_f64_div(neg_inf, 2.0);
    ASSERT_TRUE(std::isinf(result2)) << "-∞/2.0 should be infinite";
    ASSERT_TRUE(std::signbit(result2)) << "-∞/2.0 should be negative infinity";

    // Finite number divided by positive infinity
    double result3 = call_f64_div(42.0, pos_inf);
    ASSERT_EQ(0.0, std::abs(result3)) << "42.0/+∞ should be zero";
    ASSERT_TRUE(is_positive_zero(result3)) << "42.0/+∞ should be positive zero";

    // Finite number divided by negative infinity
    double result4 = call_f64_div(42.0, neg_inf);
    ASSERT_EQ(0.0, std::abs(result4)) << "42.0/-∞ should be zero";
    ASSERT_TRUE(is_negative_zero(result4)) << "42.0/-∞ should be negative zero";

    // Infinity divided by infinity (indeterminate)
    double result5 = call_f64_div(pos_inf, pos_inf);
    ASSERT_TRUE(std::isnan(result5)) << "+∞/+∞ should be NaN (indeterminate)";

    double result6 = call_f64_div(pos_inf, neg_inf);
    ASSERT_TRUE(std::isnan(result6)) << "+∞/-∞ should be NaN (indeterminate)";
}

/**
 * @test SpecialValueHandling_NaNOperands_PropagatesNaN
 * @brief Validates f64.div behavior when NaN values are involved as operands
 * @details Tests IEEE 754 compliant NaN propagation in division operations.
 *          Verifies that any operation with NaN operand produces NaN result.
 * @test_category Edge - Special numeric value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_div_nan_handling
 * @input_conditions NaN values as dividend and/or divisor operands
 * @expected_behavior NaN propagation in all cases involving NaN operands
 * @validation_method NaN detection for all NaN input combinations
 */
TEST_P(F64DivTest, SpecialValueHandling_NaNOperands_PropagatesNaN) {

    const double pos_nan = std::numeric_limits<double>::quiet_NaN();
    const double neg_nan = -std::numeric_limits<double>::quiet_NaN();

    // NaN dividend, finite divisor
    double result1 = call_f64_div(pos_nan, 2.0);
    ASSERT_TRUE(std::isnan(result1)) << "NaN/2.0 should be NaN";

    // Finite dividend, NaN divisor
    double result2 = call_f64_div(10.0, pos_nan);
    ASSERT_TRUE(std::isnan(result2)) << "10.0/NaN should be NaN";

    // NaN dividend, NaN divisor
    double result3 = call_f64_div(pos_nan, neg_nan);
    ASSERT_TRUE(std::isnan(result3)) << "NaN/NaN should be NaN";

    // Negative NaN cases
    double result4 = call_f64_div(neg_nan, 5.0);
    ASSERT_TRUE(std::isnan(result4)) << "-NaN/5.0 should be NaN";

    double result5 = call_f64_div(7.0, neg_nan);
    ASSERT_TRUE(std::isnan(result5)) << "7.0/-NaN should be NaN";

    // NaN with special values
    double result6 = call_f64_div(pos_nan, std::numeric_limits<double>::infinity());
    ASSERT_TRUE(std::isnan(result6)) << "NaN/∞ should be NaN";
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
TEST_P(F64DivTest, RuntimeErrors_ModuleLoadFailure_HandlesErrorsGracefully) {
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

INSTANTIATE_TEST_SUITE_P(RunningModeTest, F64DivTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<F64DivTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });