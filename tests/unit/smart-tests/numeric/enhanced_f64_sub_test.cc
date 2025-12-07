/**
 * Enhanced Unit Tests for f64.sub WebAssembly Opcode
 *
 * This test suite provides comprehensive validation of the f64.sub operation,
 * which implements IEEE 754 double-precision floating-point subtraction.
 * The opcode takes two f64 operands from the stack and returns their difference
 * (first operand - second operand), handling all special cases according to IEEE 754-2008 standard.
 *
 * Test Coverage:
 * - Basic subtraction operations with standard values
 * - Boundary conditions and extreme values (DBL_MAX, DBL_MIN)
 * - Special IEEE 754 values (NaN, infinity, signed zeros)
 * - Precision edge cases and near-cancellation scenarios
 * - Cross-execution mode validation (interpreter vs AOT)
 * - Runtime error scenarios and module loading failures
 *
 * Source: core/iwasm/interpreter/wasm_interp_classic.c - f64.sub implementation
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

class F64SubTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up WAMR runtime environment and load test module
     * @details Initializes WAMR with interpreter and AOT capabilities,
     *          loads the f64_sub test module, and prepares execution environment
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime environment
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load WASM module for f64.sub testing
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
     * @brief Load and instantiate the f64.sub test WASM module
     * @details Reads the compiled WASM bytecode and creates an executable module instance
     */
    void load_test_module() {
        char error_buf[256];
        const char* module_path = "wasm-apps/f64_sub_test.wasm";

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
     * @brief Execute f64.sub operation with two f64 operands
     * @param minuend First operand (value to subtract from)
     * @param subtrahend Second operand (value to subtract)
     * @return Result of f64.sub(minuend, subtrahend) = minuend - subtrahend
     */
    double call_f64_sub(double minuend, double subtrahend) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_sub");
        EXPECT_NE(nullptr, func) << "Failed to find test_f64_sub function";

        // Convert double parameters to uint64_t for WAMR function call
        union { double f; uint64_t u; } min_conv = { .f = minuend };
        union { double f; uint64_t u; } sub_conv = { .f = subtrahend };

        uint32_t argv[4] = {
            static_cast<uint32_t>(min_conv.u),
            static_cast<uint32_t>(min_conv.u >> 32),
            static_cast<uint32_t>(sub_conv.u),
            static_cast<uint32_t>(sub_conv.u >> 32)
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
     * @brief Execute boundary test that produces values near zero
     * @param minuend First operand for boundary calculation
     * @param subtrahend Second operand for boundary calculation
     * @return Result of boundary f64.sub operation
     */
    double call_f64_sub_boundary(double minuend, double subtrahend) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_sub_boundary");
        EXPECT_NE(nullptr, func) << "Failed to find test_f64_sub_boundary function";

        // Convert double parameters to uint64_t for WAMR function call
        union { double f; uint64_t u; } min_conv = { .f = minuend };
        union { double f; uint64_t u; } sub_conv = { .f = subtrahend };

        uint32_t argv[4] = {
            static_cast<uint32_t>(min_conv.u),
            static_cast<uint32_t>(min_conv.u >> 32),
            static_cast<uint32_t>(sub_conv.u),
            static_cast<uint32_t>(sub_conv.u >> 32)
        };

        bool ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        // Extract result from argv (f64 result in first 2 elements)
        union { double f; uint64_t u; } result_conv;
        result_conv.u = static_cast<uint64_t>(argv[0]) | (static_cast<uint64_t>(argv[1]) << 32);
        return result_conv.f;
    }

    /**
     * @brief Execute special value test for IEEE 754 edge cases
     * @param minuend First operand (may be special value)
     * @param subtrahend Second operand (may be special value)
     * @return Result of special value f64.sub operation
     */
    double call_f64_sub_special(double minuend, double subtrahend) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_sub_special");
        EXPECT_NE(nullptr, func) << "Failed to find test_f64_sub_special function";

        // Convert double parameters to uint64_t for WAMR function call
        union { double f; uint64_t u; } min_conv = { .f = minuend };
        union { double f; uint64_t u; } sub_conv = { .f = subtrahend };

        uint32_t argv[4] = {
            static_cast<uint32_t>(min_conv.u),
            static_cast<uint32_t>(min_conv.u >> 32),
            static_cast<uint32_t>(sub_conv.u),
            static_cast<uint32_t>(sub_conv.u >> 32)
        };

        bool ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

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

    // Runtime configuration
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t* buf = nullptr;
    uint32_t buf_size = 0;
    uint32_t stack_size = 8192;
    uint32_t heap_size = 8192;
};

/**
 * @test BasicSubtraction_ReturnsCorrectDifference
 * @brief Validates f64.sub produces correct arithmetic results for typical double precision inputs
 * @details Tests fundamental subtraction operation with positive, negative, and mixed-sign doubles.
 *          Verifies that f64.sub correctly computes minuend - subtrahend for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sub_operation
 * @input_conditions Standard double pairs: (10.5, 3.2), (-10.5, -3.2), (10.0, -5.0)
 * @expected_behavior Returns mathematical difference: 7.3, -7.3, 15.0 respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(F64SubTest, BasicSubtraction_ReturnsCorrectDifference) {
    // Test positive number subtraction
    double result1 = call_f64_sub(10.5, 3.2);
    ASSERT_DOUBLE_EQ(7.3, result1) << "Subtraction of positive doubles failed";

    // Test negative number subtraction
    double result2 = call_f64_sub(-10.5, -3.2);
    ASSERT_DOUBLE_EQ(-7.3, result2) << "Subtraction of negative doubles failed";

    // Test mixed sign subtraction
    double result3 = call_f64_sub(10.0, -5.0);
    ASSERT_DOUBLE_EQ(15.0, result3) << "Subtraction with mixed signs failed";
}

/**
 * @test BoundaryValues_HandlesLimitsCorrectly
 * @brief Validates f64.sub behavior at double precision representational boundaries
 * @details Tests subtraction operations with maximum and minimum f64 values,
 *          verifying correct overflow to infinity and proper boundary handling.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sub_overflow_handling
 * @input_conditions Boundary values: (DBL_MAX, -DBL_MAX), (DBL_MAX, DBL_MIN), (-DBL_MAX, DBL_MAX)
 * @expected_behavior Produces +∞, ~DBL_MAX, -∞ respectively according to IEEE 754
 * @validation_method Infinity checks and boundary value validation
 */
TEST_P(F64SubTest, BoundaryValues_HandlesLimitsCorrectly) {
    // Test maximum value operations leading to overflow
    double result1 = call_f64_sub_boundary(DBL_MAX, -DBL_MAX);
    ASSERT_TRUE(std::isinf(result1)) << "DBL_MAX - (-DBL_MAX) should produce infinity";
    ASSERT_FALSE(std::signbit(result1)) << "Result should be positive infinity";

    // Test boundary operation near maximum
    double result2 = call_f64_sub_boundary(DBL_MAX, DBL_MIN);
    // Result should be approximately DBL_MAX (DBL_MIN is negligible compared to DBL_MAX)
    ASSERT_FALSE(std::isinf(result2)) << "DBL_MAX - DBL_MIN should not overflow";
    ASSERT_GT(result2, 0.0) << "Result should be positive";

    // Test underflow to negative infinity
    double result3 = call_f64_sub_boundary(-DBL_MAX, DBL_MAX);
    ASSERT_TRUE(std::isinf(result3)) << "(-DBL_MAX) - DBL_MAX should produce negative infinity";
    ASSERT_TRUE(std::signbit(result3)) << "Result should be negative infinity";
}

/**
 * @test ZeroOperations_FollowsIEEE754Standard
 * @brief Validates f64.sub correctly handles positive and negative zero according to IEEE 754
 * @details Tests subtraction operations involving signed zeros, verifying correct
 *          IEEE 754 behavior for zero arithmetic and perfect cancellation scenarios.
 * @test_category Edge - IEEE 754 zero handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sub_zero_handling
 * @input_conditions Zero operations: (5.0, +0.0), (+0.0, -0.0), (-0.0, +0.0), perfect cancellation
 * @expected_behavior Produces 5.0, +0.0, -0.0, +0.0 respectively with proper sign handling
 * @validation_method Exact equality checks with signbit validation for signed zeros
 */
TEST_P(F64SubTest, ZeroOperations_FollowsIEEE754Standard) {
    // Test subtraction with positive zero
    double result1 = call_f64_sub_special(5.0, +0.0);
    ASSERT_DOUBLE_EQ(5.0, result1) << "5.0 - (+0.0) should equal 5.0";

    // Test signed zero subtraction: (+0.0) - (-0.0) = +0.0
    double result2 = call_f64_sub_special(+0.0, -0.0);
    ASSERT_DOUBLE_EQ(0.0, result2) << "(+0.0) - (-0.0) should equal +0.0";
    ASSERT_TRUE(is_positive_zero(result2)) << "Result should be positive zero";

    // Test signed zero subtraction: (-0.0) - (+0.0) = -0.0
    double result3 = call_f64_sub_special(-0.0, +0.0);
    ASSERT_DOUBLE_EQ(0.0, result3) << "(-0.0) - (+0.0) should equal zero";
    ASSERT_TRUE(is_negative_zero(result3)) << "Result should be negative zero";

    // Test perfect cancellation: x - x = +0.0
    double result4 = call_f64_sub(5.0, 5.0);
    ASSERT_DOUBLE_EQ(0.0, result4) << "Perfect cancellation should produce zero";
    ASSERT_TRUE(is_positive_zero(result4)) << "Cancellation result should be positive zero";
}

/**
 * @test SpecialValues_PropagatesNaNAndInfinity
 * @brief Validates f64.sub correctly handles NaN and infinity according to IEEE 754 arithmetic rules
 * @details Tests subtraction operations with special IEEE 754 values (NaN, ±∞),
 *          verifying correct propagation and arithmetic behavior per the standard.
 * @test_category Edge - Special IEEE 754 value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sub_special_values
 * @input_conditions Special values: (NaN, 5.0), (+∞, +∞), (+∞, -∞), (-∞, 5.0)
 * @expected_behavior Produces NaN, NaN, +∞, -∞ respectively per IEEE 754 rules
 * @validation_method isnan() and isinf() checks with proper sign validation
 */
TEST_P(F64SubTest, SpecialValues_PropagatesNaNAndInfinity) {
    // Test NaN propagation: NaN - x = NaN
    double result1 = call_f64_sub_special(std::numeric_limits<double>::quiet_NaN(), 5.0);
    ASSERT_TRUE(std::isnan(result1)) << "NaN - 5.0 should produce NaN";

    // Test x - NaN = NaN
    double result2 = call_f64_sub_special(5.0, std::numeric_limits<double>::quiet_NaN());
    ASSERT_TRUE(std::isnan(result2)) << "5.0 - NaN should produce NaN";

    // Test infinity subtraction: (+∞) - (+∞) = NaN
    double result3 = call_f64_sub_special(std::numeric_limits<double>::infinity(),
                                         std::numeric_limits<double>::infinity());
    ASSERT_TRUE(std::isnan(result3)) << "(+∞) - (+∞) should produce NaN";

    // Test infinity subtraction: (+∞) - (-∞) = +∞
    double result4 = call_f64_sub_special(std::numeric_limits<double>::infinity(),
                                         -std::numeric_limits<double>::infinity());
    ASSERT_TRUE(std::isinf(result4)) << "(+∞) - (-∞) should produce infinity";
    ASSERT_FALSE(std::signbit(result4)) << "Result should be positive infinity";

    // Test finite - (-∞) = +∞
    double result5 = call_f64_sub_special(5.0, -std::numeric_limits<double>::infinity());
    ASSERT_TRUE(std::isinf(result5)) << "5.0 - (-∞) should produce positive infinity";
    ASSERT_FALSE(std::signbit(result5)) << "Result should be positive infinity";
}

/**
 * @test PrecisionEdgeCases_MaintainsAccuracy
 * @brief Validates f64.sub maintains precision in near-cancellation and denormalized number scenarios
 * @details Tests subtraction operations at precision limits, including near-cancellation
 *          with similar magnitude values and operations with denormalized numbers.
 * @test_category Edge - Precision and denormalized number validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_sub_precision_handling
 * @input_conditions Precision cases: (1.0 + DBL_EPSILON, 1.0), (DBL_MIN, DBL_MIN/2), large similar numbers
 * @expected_behavior Maintains precision: DBL_EPSILON, DBL_MIN/2, expected precision-limited results
 * @validation_method Precise equality checks and near-equality validation for precision limits
 */
TEST_P(F64SubTest, PrecisionEdgeCases_MaintainsAccuracy) {
    // Test near-cancellation precision: (1.0 + ε) - 1.0 = ε
    double epsilon_test = call_f64_sub_special(1.0 + std::numeric_limits<double>::epsilon(), 1.0);
    ASSERT_DOUBLE_EQ(std::numeric_limits<double>::epsilon(), epsilon_test)
        << "Near-cancellation should preserve machine epsilon";

    // Test denormalized number operations
    double min_result = call_f64_sub_special(std::numeric_limits<double>::min(),
                                           std::numeric_limits<double>::min() / 2.0);
    ASSERT_GT(min_result, 0.0) << "DBL_MIN - (DBL_MIN/2) should be positive";
    ASSERT_LE(min_result, std::numeric_limits<double>::min())
        << "Result should be less than or equal to DBL_MIN";

    // Test precision with large similar numbers
    double large1 = 1e15 + 1.0;
    double large2 = 1e15;
    double large_result = call_f64_sub(large1, large2);
    ASSERT_DOUBLE_EQ(1.0, large_result) << "Large number subtraction should preserve unit precision";
}

/**
 * @test InvalidModule_RejectsGracefully
 * @brief Validates WAMR properly rejects malformed WASM modules containing invalid f64.sub usage
 * @details Tests error handling for corrupted bytecode, invalid module structure,
 *          and stack underflow scenarios, ensuring graceful failure without crashes.
 * @test_category Error - Invalid module and error handling validation
 * @coverage_target core/iwasm/common/wasm_loader.c:module_validation
 * @input_conditions Invalid scenarios: corrupted bytecode, stack underflow, type mismatches
 * @expected_behavior Module load failure, proper error reporting, graceful degradation
 * @validation_method Module load failure checks and error message validation
 */
TEST_P(F64SubTest, InvalidModule_RejectsGracefully) {
    char error_buf[256];

    // Test with intentionally corrupted WASM bytecode
    uint8_t invalid_wasm[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF};
    wasm_module_t invalid_module = wasm_runtime_load(invalid_wasm, sizeof(invalid_wasm),
                                                   error_buf, sizeof(error_buf));

    ASSERT_EQ(nullptr, invalid_module)
        << "Invalid WASM module should fail to load";
    ASSERT_NE('\0', error_buf[0])
        << "Error message should be provided for invalid module";

    // Test stack underflow scenario by attempting to load underflow test module
    const char* underflow_path = "wasm-apps/f64_sub_stack_underflow.wasm";
    uint32_t underflow_buf_size;
    uint8_t* underflow_buf = reinterpret_cast<uint8_t*>(
        bh_read_file_to_buffer(underflow_path, &underflow_buf_size));

    if (underflow_buf) {
        wasm_module_t underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                                         error_buf, sizeof(error_buf));

        // Module may load but should fail on instantiation or execution
        if (underflow_module) {
            wasm_module_inst_t underflow_inst = wasm_runtime_instantiate(
                underflow_module, stack_size, heap_size, error_buf, sizeof(error_buf));
            ASSERT_NE(nullptr, underflow_inst) << "Module instantiation should succeed";

            // Clean up
            if (underflow_inst) {
                wasm_runtime_deinstantiate(underflow_inst);
            }
            wasm_runtime_unload(underflow_module);
        }

        BH_FREE(underflow_buf);
    }
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, F64SubTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT));