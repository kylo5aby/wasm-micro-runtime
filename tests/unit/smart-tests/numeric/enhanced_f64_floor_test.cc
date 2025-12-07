/**
 * Enhanced Unit Tests for f64.floor opcode
 *
 * This file contains comprehensive test cases for the f64.floor WebAssembly opcode,
 * which implements the IEEE 754 floor operation (rounds toward negative infinity).
 *
 * Test Coverage:
 * - Basic floor operations with positive and negative values
 * - IEEE 754 special values (NaN, infinities, signed zeros)
 * - Boundary conditions and precision limits
 * - Mathematical property validation
 *
 * Location: tests/unit/enhanced_opcode/numeric/enhanced_f64_floor_test.cc
 */

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"

/**
 * Test fixture for f64.floor opcode validation.
 * Provides setup and teardown for WAMR runtime environment and test module loading.
 */
class F64FloorTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * Set up WAMR runtime environment for each test case.
     * Initializes runtime with system allocator and loads test module.
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        load_test_module();
    }

    /**
     * Clean up WAMR runtime environment after each test case.
     * Destroys module instance, module, and runtime.
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
        wasm_runtime_destroy();
    }

private:
    /**
     * Load the f64.floor test WebAssembly module.
     * Reads module from file and instantiates it for testing.
     */
    void load_test_module() {
        const char* file_path = "wasm-apps/f64_floor_test.wasm";

        buf = (uint8_t*)bh_read_file_to_buffer(file_path, &buf_size);
        ASSERT_NE(nullptr, buf) << "Failed to read test module: " << file_path;
        ASSERT_GT(buf_size, 0U) << "WASM file is empty: " << file_path;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate module: " << error_buf;

        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
    }

public:
    /**
     * Call f64.floor test function with a single f64 input.
     *
     * @param input The f64 value to apply floor operation to
     * @return The result of f64.floor(input)
     */
    double call_f64_floor(double input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_floor");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f64_floor function";

        // Convert double to uint64_t for WAMR function call
        union { double d; uint64_t u; } input_conv = { .d = input };

        uint32_t argv[3];
        // For f64 parameters, we need to split the 64-bit value into two 32-bit values
        argv[0] = (uint32_t)(input_conv.u & 0xFFFFFFFF);        // Lower 32 bits
        argv[1] = (uint32_t)((input_conv.u >> 32) & 0xFFFFFFFF); // Upper 32 bits
        argv[2] = 0;  // Result placeholder

        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_TRUE(ret) << "Function call failed: " << wasm_runtime_get_exception(module_inst);

        const char *exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            EXPECT_TRUE(false) << "Runtime exception occurred: " << exception;
        }

        // Convert result back to double - result is in argv[0] and argv[1]
        uint64_t result_bits = ((uint64_t)argv[1] << 32) | argv[0];
        union { double d; uint64_t u; } result_conv = { .u = result_bits };
        return result_conv.d;
    }

    /**
     * Check if two f64 values are bitwise equal, handling NaN and signed zeros correctly.
     *
     * @param expected Expected f64 value
     * @param actual Actual f64 value
     * @return True if values are bitwise identical
     */
    bool f64_bitwise_equal(double expected, double actual) {
        uint64_t expected_bits = *reinterpret_cast<uint64_t*>(&expected);
        uint64_t actual_bits = *reinterpret_cast<uint64_t*>(&actual);
        return expected_bits == actual_bits;
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
 * @test BasicFloorOperation_ReturnsCorrectResult
 * @brief Validates f64.floor produces mathematically correct results for typical values
 * @details Tests floor operation with positive fractional, negative fractional, and integer values.
 *          Verifies that f64.floor correctly rounds toward negative infinity in all cases.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_floor_operation
 * @input_conditions Various f64 values: positive fractional, negative fractional, integers
 * @expected_behavior Returns floor(x) = largest integer ≤ x for all inputs
 * @validation_method Direct comparison of WASM function result with expected mathematical values
 */
TEST_P(F64FloorTest, BasicFloorOperation_ReturnsCorrectResult) {
    // Test positive fractional values - should round down
    ASSERT_EQ(3.0, call_f64_floor(3.7)) << "floor(3.7) should equal 3.0";
    ASSERT_EQ(42.0, call_f64_floor(42.2)) << "floor(42.2) should equal 42.0";
    ASSERT_EQ(0.0, call_f64_floor(0.9)) << "floor(0.9) should equal 0.0";

    // Test negative fractional values - should round toward negative infinity
    ASSERT_EQ(-4.0, call_f64_floor(-3.7)) << "floor(-3.7) should equal -4.0";
    ASSERT_EQ(-43.0, call_f64_floor(-42.2)) << "floor(-42.2) should equal -43.0";
    ASSERT_EQ(-1.0, call_f64_floor(-0.1)) << "floor(-0.1) should equal -1.0";

    // Test integer values - should remain unchanged
    ASSERT_EQ(5.0, call_f64_floor(5.0)) << "floor(5.0) should equal 5.0";
    ASSERT_EQ(-17.0, call_f64_floor(-17.0)) << "floor(-17.0) should equal -17.0";
    ASSERT_EQ(0.0, call_f64_floor(0.0)) << "floor(0.0) should equal 0.0";
}

/**
 * @test SpecialValues_HandledCorrectly
 * @brief Validates f64.floor handles IEEE 754 special values according to specification
 * @details Tests behavior with NaN, positive/negative infinity, and signed zero values.
 *          Ensures IEEE 754 compliance for all special floating-point cases.
 * @test_category Edge - Special IEEE 754 value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_floor_special_cases
 * @input_conditions IEEE 754 special values: NaN, ±infinity, ±0.0
 * @expected_behavior Returns input unchanged for special values, preserving bit patterns
 * @validation_method Bitwise comparison to ensure exact IEEE 754 compliance
 */
TEST_P(F64FloorTest, SpecialValues_HandledCorrectly) {
    // Test NaN - should return NaN
    double nan_input = std::numeric_limits<double>::quiet_NaN();
    double nan_result = call_f64_floor(nan_input);
    ASSERT_TRUE(std::isnan(nan_result)) << "floor(NaN) should return NaN";

    // Test positive infinity - should return positive infinity
    double pos_inf = std::numeric_limits<double>::infinity();
    ASSERT_EQ(pos_inf, call_f64_floor(pos_inf)) << "floor(+infinity) should return +infinity";

    // Test negative infinity - should return negative infinity
    double neg_inf = -std::numeric_limits<double>::infinity();
    ASSERT_EQ(neg_inf, call_f64_floor(neg_inf)) << "floor(-infinity) should return -infinity";

    // Test positive zero - should preserve sign
    double pos_zero = +0.0;
    double pos_zero_result = call_f64_floor(pos_zero);
    ASSERT_TRUE(f64_bitwise_equal(pos_zero, pos_zero_result)) << "floor(+0.0) should preserve positive zero";

    // Test negative zero - should preserve sign
    double neg_zero = -0.0;
    double neg_zero_result = call_f64_floor(neg_zero);
    ASSERT_TRUE(f64_bitwise_equal(neg_zero, neg_zero_result)) << "floor(-0.0) should preserve negative zero";
}

/**
 * @test BoundaryConditions_ProduceExpectedResults
 * @brief Validates f64.floor behavior at floating-point precision boundaries
 * @details Tests very large values, very small values, and precision limit cases.
 *          Ensures correct behavior when floating-point precision affects results.
 * @test_category Corner - Boundary value and precision validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_floor_precision_handling
 * @input_conditions Boundary values: large numbers, denormals, precision limits
 * @expected_behavior Maintains mathematical correctness at precision boundaries
 * @validation_method Verification against expected boundary behavior and precision handling
 */
TEST_P(F64FloorTest, BoundaryConditions_ProduceExpectedResults) {
    // Test very large positive value near f64 maximum
    double large_pos = 1.7976931348623157e+307;
    ASSERT_EQ(large_pos, call_f64_floor(large_pos)) << "floor of very large value should be unchanged";

    // Test very large negative value near f64 minimum
    double large_neg = -1.7976931348623157e+307;
    ASSERT_EQ(large_neg, call_f64_floor(large_neg)) << "floor of very large negative should be unchanged";

    // Test smallest positive denormal number
    double min_denormal = std::numeric_limits<double>::denorm_min();
    ASSERT_EQ(0.0, call_f64_floor(min_denormal)) << "floor of minimum denormal should be 0.0";

    // Test largest value less than 1.0
    double almost_one = 0.9999999999999999;
    ASSERT_EQ(0.0, call_f64_floor(almost_one)) << "floor of value just less than 1.0 should be 0.0";

    // Test value where floating-point precision matters
    double precision_test = 9007199254740992.5; // 2^53 + 0.5
    double expected = 9007199254740992.0;
    ASSERT_EQ(expected, call_f64_floor(precision_test)) << "floor should handle precision boundary correctly";
}

/**
 * @test MathematicalProperties_Validated
 * @brief Validates mathematical properties and consistency of f64.floor operation
 * @details Tests fundamental mathematical properties: floor(x) ≤ x, idempotence, and monotonicity.
 *          Ensures the floor function maintains expected mathematical relationships.
 * @test_category Edge - Mathematical property verification
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_floor_mathematical_correctness
 * @input_conditions Various test values to validate mathematical properties
 * @expected_behavior Maintains mathematical properties: floor(x) ≤ x, idempotence
 * @validation_method Property-based validation of mathematical relationships
 */
TEST_P(F64FloorTest, MathematicalProperties_Validated) {
    // Test that floor(x) <= x for various values
    double test_values[] = {1.1, -1.1, 42.7, -42.7, 0.1, -0.1};
    for (double value : test_values) {
        double floor_result = call_f64_floor(value);
        ASSERT_LE(floor_result, value) << "floor(" << value << ") should be ≤ " << value;
    }

    // Test idempotence: floor(floor(x)) = floor(x)
    double test_val = 3.14159;
    double first_floor = call_f64_floor(test_val);
    double second_floor = call_f64_floor(first_floor);
    ASSERT_EQ(first_floor, second_floor) << "floor should be idempotent";

    // Test floor of integer equals the integer
    double integer_val = 123.0;
    ASSERT_EQ(integer_val, call_f64_floor(integer_val)) << "floor of integer should equal the integer";

    // Test monotonicity property with ordered values
    double val1 = 2.3, val2 = 2.8;
    double floor1 = call_f64_floor(val1), floor2 = call_f64_floor(val2);
    ASSERT_LE(floor1, floor2) << "floor function should preserve ordering";
}

// Parameterized test instantiation for both execution modes
INSTANTIATE_TEST_SUITE_P(
    RunningMode,
    F64FloorTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);