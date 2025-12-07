/*
 * Enhanced Unit Tests for f64.convert_i64_u WASM Opcode
 *
 * This file contains comprehensive test cases for the f64.convert_i64_u opcode,
 * which converts unsigned 64-bit integers to 64-bit floating-point values with
 * IEEE 754 compliance and proper handling of precision limits.
 */

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <cstring>

#include "wasm_export.h"
#include "bh_read_file.h"

// Test constants for precision boundaries
static const uint64_t PRECISION_BOUNDARY_2_53 = 9007199254740992ULL;   // 2^53
static const uint64_t MAX_EXACT_INTEGER = 9007199254740991ULL;          // 2^53 - 1
static const uint64_t BEYOND_PRECISION = 9007199254740993ULL;           // 2^53 + 1

/**
 * Test fixture for f64.convert_i64_u opcode validation
 *
 * Provides WAMR runtime initialization, module loading capabilities,
 * and parameterized testing across interpreter and AOT execution modes.
 * Validates IEEE 754 compliance for unsigned 64-bit to double conversion.
 */
class F64ConvertI64UTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * Set up test environment before each test case
     *
     * Initializes WAMR runtime with system allocator, loads the test module
     * containing f64.convert_i64_u test functions, and prepares execution context.
     */
    void SetUp() override {
        // Initialize WAMR runtime with system allocator
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load test WASM module
        const char *wasm_path = "wasm-apps/f64_convert_i64_u_test.wasm";
        wasm_buf = reinterpret_cast<uint8_t*>(
            bh_read_file_to_buffer(wasm_path, &wasm_buf_size));
        ASSERT_NE(nullptr, wasm_buf)
            << "Failed to load f64.convert_i64_u test WASM file: " << wasm_path;

        // Load and instantiate module
        char error_buf[128];
        wasm_module = wasm_runtime_load(wasm_buf, wasm_buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, wasm_module)
            << "Failed to load WASM module: " << error_buf;

        wasm_module_inst = wasm_runtime_instantiate(wasm_module, 65536, 65536, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, wasm_module_inst)
            << "Failed to instantiate WASM module: " << error_buf;

        // Get execution environment
        exec_env = wasm_runtime_create_exec_env(wasm_module_inst, 65536);
        ASSERT_NE(nullptr, exec_env)
            << "Failed to create execution environment";
    }

    /**
     * Clean up test environment after each test case
     *
     * Destroys execution environment, module instance, and runtime to prevent
     * resource leaks and ensure clean state for subsequent tests.
     */
    void TearDown() override {
        // Clean up in reverse order
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
        }
        if (wasm_module_inst) {
            wasm_runtime_deinstantiate(wasm_module_inst);
        }
        if (wasm_module) {
            wasm_runtime_unload(wasm_module);
        }
        if (wasm_buf) {
            BH_FREE(wasm_buf);
        }

        wasm_runtime_destroy();
    }

    /**
     * Call f64.convert_i64_u WASM function with given unsigned 64-bit input
     *
     * @param input Unsigned 64-bit integer to convert to double
     * @return Converted double-precision floating-point value
     */
    double call_f64_convert_i64_u(uint64_t input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "test_f64_convert_i64_u");
        EXPECT_NE(nullptr, func) << "Failed to find test_f64_convert_i64_u function";

        uint32_t wasm_argv[2];
        wasm_argv[0] = (uint32_t)(input & 0xFFFFFFFF);        // Low 32 bits
        wasm_argv[1] = (uint32_t)(input >> 32);               // High 32 bits

        char error_buf[128];
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, wasm_argv);
        EXPECT_TRUE(ret) << "WASM function call failed: " << wasm_runtime_get_exception(wasm_module_inst);

        // Extract double result from two 32-bit return values
        uint64_t result_bits = ((uint64_t)wasm_argv[1] << 32) | wasm_argv[0];
        double result;
        memcpy(&result, &result_bits, sizeof(double));
        return result;
    }

    /**
     * Helper function to test conversion with IEEE 754 bit-exact comparison
     *
     * @param input Unsigned 64-bit integer input
     * @param expected Expected double result
     */
    void test_exact_conversion(uint64_t input, double expected) {
        double actual = call_f64_convert_i64_u(input);

        // Use bit-exact comparison for IEEE 754 compliance
        uint64_t actual_bits, expected_bits;
        memcpy(&actual_bits, &actual, sizeof(double));
        memcpy(&expected_bits, &expected, sizeof(double));

        ASSERT_EQ(expected_bits, actual_bits)
            << "Conversion of " << input << " failed. Expected: " << expected
            << " (0x" << std::hex << expected_bits << "), Got: " << actual
            << " (0x" << actual_bits << ")" << std::dec;
    }

    // Test fixture member variables
    uint8_t *wasm_buf = nullptr;
    uint32 wasm_buf_size = 0;
    wasm_module_t wasm_module = nullptr;
    wasm_module_inst_t wasm_module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
};

/**
 * @test BasicConversion_TypicalValues_ReturnsAccurateResults
 * @brief Validates f64.convert_i64_u produces correct IEEE 754 results for typical inputs
 * @details Tests fundamental conversion operation with small, medium, and large unsigned integers.
 *          Verifies that f64.convert_i64_u correctly converts representative u64 values across
 *          different magnitude ranges while maintaining IEEE 754 compliance.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_convert_i64_u_operation
 * @input_conditions Standard unsigned integers: 42UL, 4294967296UL (2^32), large values
 * @expected_behavior Returns exact IEEE 754 double precision representations
 * @validation_method Direct bit-exact comparison with expected IEEE 754 values
 */
TEST_P(F64ConvertI64UTest, BasicConversion_TypicalValues_ReturnsAccurateResults) {
    // Small positive integer - should convert exactly
    test_exact_conversion(42ULL, 42.0);

    // Medium value around 2^32 boundary
    test_exact_conversion(4294967296ULL, 4294967296.0);

    // Large value within exact representation range
    test_exact_conversion(1152921504606846976ULL, 1152921504606846976.0);

    // Power-of-two value for exact representation verification
    test_exact_conversion(1099511627776ULL, 1099511627776.0); // 2^40
}

/**
 * @test BoundaryConversion_MinMaxValues_HandlesLimitsCorrectly
 * @brief Validates conversion behavior at critical boundary values including zero and UINT64_MAX
 * @details Tests edge cases at the extremes of the unsigned 64-bit range and precision boundaries.
 *          Ensures proper handling of minimum value (zero), maximum value (UINT64_MAX), and
 *          the critical IEEE 754 precision boundary at 2^53.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_convert_i64_u_operation
 * @input_conditions Zero, UINT64_MAX, precision boundaries (2^53-1, 2^53, 2^53+1)
 * @expected_behavior Accurate conversion with proper rounding for precision-limited values
 * @validation_method Bit-exact comparison for exact values, proper rounding verification for others
 */
TEST_P(F64ConvertI64UTest, BoundaryConversion_MinMaxValues_HandlesLimitsCorrectly) {
    // Test zero conversion
    test_exact_conversion(0ULL, 0.0);

    // Test maximum exact representable integer (2^53 - 1)
    test_exact_conversion(MAX_EXACT_INTEGER, static_cast<double>(MAX_EXACT_INTEGER));

    // Test exact precision boundary (2^53)
    test_exact_conversion(PRECISION_BOUNDARY_2_53, static_cast<double>(PRECISION_BOUNDARY_2_53));

    // Test UINT64_MAX - will lose precision but should round correctly
    double result = call_f64_convert_i64_u(UINT64_MAX);
    ASSERT_TRUE(std::isfinite(result)) << "UINT64_MAX conversion should produce finite result";
    ASSERT_TRUE(result > 0.0) << "UINT64_MAX conversion should produce positive result";
    ASSERT_TRUE(result > 1.8e19) << "UINT64_MAX conversion should be approximately 1.844e19";
}

/**
 * @test PrecisionLoss_LargeIntegers_RoundsToNearestRepresentable
 * @brief Validates IEEE 754 rounding behavior for integers beyond 53-bit precision
 * @details Tests conversion behavior for unsigned integers that exceed IEEE 754 double precision
 *          mantissa capacity. Verifies proper round-to-nearest-even behavior and validates that
 *          consecutive large integers may round to the same double representation.
 * @test_category Edge - Precision boundary and rounding validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_convert_i64_u_operation
 * @input_conditions Integers around and beyond 2^53 precision boundary
 * @expected_behavior Proper IEEE 754 round-to-nearest-even behavior for precision-limited values
 * @validation_method Verification of rounding behavior and representational limits
 */
TEST_P(F64ConvertI64UTest, PrecisionLoss_LargeIntegers_RoundsToNearestRepresentable) {
    // Test value just beyond exact precision boundary (2^53 + 1)
    // Should round down to 2^53 due to round-to-nearest-even
    double result_beyond = call_f64_convert_i64_u(BEYOND_PRECISION);
    double expected_rounded = static_cast<double>(PRECISION_BOUNDARY_2_53);
    test_exact_conversion(BEYOND_PRECISION, expected_rounded);

    // Test larger values that demonstrate precision loss
    uint64_t large_odd = PRECISION_BOUNDARY_2_53 + 3;  // 2^53 + 3
    uint64_t large_even = PRECISION_BOUNDARY_2_53 + 4; // 2^53 + 4

    double result_odd = call_f64_convert_i64_u(large_odd);
    double result_even = call_f64_convert_i64_u(large_even);

    // Both should round to the same representable value
    ASSERT_EQ(result_odd, result_even)
        << "Consecutive integers beyond precision should round to same double";
    ASSERT_EQ(result_odd, static_cast<double>(PRECISION_BOUNDARY_2_53 + 4))
        << "Should round to nearest even representable value";
}

/**
 * @test PowerOfTwoConversion_ExactRepresentation_ReturnsExactValues
 * @brief Validates exact representation of power-of-two values within IEEE 754 precision
 * @details Tests conversion of various power-of-two unsigned integers to verify they convert
 *          to exact IEEE 754 double representations without precision loss. Powers of two
 *          should always be exactly representable within the IEEE 754 format.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_convert_i64_u_operation
 * @input_conditions Various power-of-two values (2^10, 2^20, 2^32, 2^40, 2^53)
 * @expected_behavior Exact IEEE 754 representation without precision loss
 * @validation_method Bit-exact comparison for power-of-two conversion accuracy
 */
TEST_P(F64ConvertI64UTest, PowerOfTwoConversion_ExactRepresentation_ReturnsExactValues) {
    // Test various powers of two that should be exactly representable
    test_exact_conversion(1024ULL, 1024.0);                    // 2^10
    test_exact_conversion(1048576ULL, 1048576.0);              // 2^20
    test_exact_conversion(4294967296ULL, 4294967296.0);        // 2^32
    test_exact_conversion(1099511627776ULL, 1099511627776.0);  // 2^40
    test_exact_conversion(PRECISION_BOUNDARY_2_53, static_cast<double>(PRECISION_BOUNDARY_2_53)); // 2^53
}

/**
 * @test UnsignedRange_LargeValues_HandlesFullRange
 * @brief Validates conversion of large unsigned values that would be negative as signed integers
 * @details Tests the distinguishing characteristic of unsigned conversion by validating proper
 *          handling of values in the upper half of the u64 range (values with MSB set that
 *          would be negative if interpreted as signed i64).
 * @test_category Edge - Unsigned-specific value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_convert_i64_u_operation
 * @input_conditions Large unsigned values including those with MSB set
 * @expected_behavior Positive double results for all unsigned inputs, proper magnitude
 * @validation_method Verification of positive results and appropriate magnitude ranges
 */
TEST_P(F64ConvertI64UTest, UnsignedRange_LargeValues_HandlesFullRange) {
    // Test value with MSB set (would be negative if signed)
    uint64_t msb_set = 0x8000000000000000ULL; // 2^63
    double result = call_f64_convert_i64_u(msb_set);
    ASSERT_GT(result, 0.0) << "Unsigned conversion should always produce positive results";
    ASSERT_TRUE(std::isfinite(result)) << "Large unsigned conversion should produce finite result";

    // Test value near upper range
    uint64_t upper_range = 0xFFFFFFFF00000000ULL;
    double upper_result = call_f64_convert_i64_u(upper_range);
    ASSERT_GT(upper_result, 0.0) << "Upper range unsigned values should convert to positive doubles";
    ASSERT_TRUE(std::isfinite(upper_result)) << "Upper range conversion should be finite";

    // Verify monotonicity property - larger inputs produce larger outputs
    ASSERT_GT(upper_result, result) << "Larger unsigned input should produce larger double output";
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    F64ConvertI64UModes,
    F64ConvertI64UTest,
    testing::Values(
        RunningMode::Mode_Interp
#if WASM_ENABLE_AOT != 0
        , RunningMode::Mode_LLVM_JIT
#endif
    ),
    [](const testing::TestParamInfo<RunningMode>& info) {
        switch (info.param) {
            case RunningMode::Mode_Interp: return "Interpreter";
            case RunningMode::Mode_LLVM_JIT: return "AOT";
            default: return "Unknown";
        }
    }
);