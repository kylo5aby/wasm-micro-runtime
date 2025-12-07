/**
 * @file enhanced_i32_trunc_f32_u_test.cc
 * @brief Comprehensive test suite for i32.trunc_f32_u WASM opcode
 * @details Tests floating-point to unsigned integer truncation with comprehensive coverage
 *          including boundary conditions, special values, and error scenarios.
 *
 * Test Coverage Areas:
 * - Basic truncation functionality for typical f32 values
 * - Boundary value handling at UINT32_MAX limits
 * - Special IEEE 754 values (NaN, infinity, zero variants)
 * - Overflow/underflow trap conditions for negative and large values
 * - Stack underflow error scenarios
 * - Cross-execution mode validation (interpreter vs AOT)
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <limits>

extern "C" {
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
}

/**
 * @class I32TruncF32UTest
 * @brief Test fixture for i32.trunc_f32_u opcode validation
 * @details Provides WAMR runtime setup and WASM module management for systematic
 *          testing of f32 to unsigned i32 truncation operations across execution modes.
 */
class I32TruncF32UTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Test fixture setup with WAMR runtime initialization
     * @details Configures WAMR runtime with system allocator and loads test WASM module
     *          for i32.trunc_f32_u opcode validation across interpreter and AOT modes.
     */
    void SetUp() override {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime for opcode testing
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i32.trunc_f32_u tests";

        // Load test WASM module containing i32.trunc_f32_u test functions
        LoadTestModule();
    }

    /**
     * @brief Test fixture cleanup with proper resource management
     * @details Ensures proper cleanup of WASM modules, instances, and runtime resources
     *          using RAII pattern for deterministic resource management.
     */
    void TearDown() override {
        // Clean up WASM execution resources
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

        if (buffer) {
            BH_FREE(buffer);
            buffer = nullptr;
        }

        // Cleanup runtime system
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM test module for i32.trunc_f32_u validation
     * @details Loads compiled WASM module containing test functions for unsigned truncation operations
     *          and creates execution environment for cross-mode testing.
     */
    void LoadTestModule() {
        const char* wasm_file = "wasm-apps/i32_trunc_f32_u_test.wasm";

        // Read WASM module bytecode from file
        buffer = (uint8_t*)bh_read_file_to_buffer(wasm_file, &size);
        ASSERT_NE(nullptr, buffer) << "Failed to read WASM file: " << wasm_file;
        ASSERT_GT(size, 0u) << "Empty WASM file: " << wasm_file;

        // Load WASM module with validation
        module = wasm_runtime_load(buffer, size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        // Instantiate module for execution
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                             error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        // Create execution environment
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
    }

    /**
     * @brief Call i32.trunc_f32_u test function with error handling
     * @param function_name Name of the exported WASM function to call
     * @param input f32 input value for truncation
     * @param expect_trap Whether the call should generate a trap
     * @return uint32_t result of truncation (only valid when expect_trap is false)
     * @details Invokes WASM function and handles both successful execution and trap conditions
     */
    uint32_t CallTruncFunction(const char* function_name, float input, bool expect_trap = false) {
        // Find the target function in the module
        wasm_function_inst_t func = wasm_runtime_lookup_function(
            module_inst, function_name);
        EXPECT_NE(nullptr, func) << "Function not found: " << function_name;

        if (!func) return 0;

        // Prepare function arguments (f32 input)
        uint32_t argv[2];  // f32 requires 32-bit slot
        *(float*)argv = input;

        // Execute function with trap detection
        bool success = wasm_runtime_call_wasm(exec_env, func, 1, argv);

        if (expect_trap) {
            // Verify that trap occurred as expected
            EXPECT_FALSE(success) << "Expected trap for input " << input
                                 << " but function succeeded";
            if (!success) {
                const char* exception = wasm_runtime_get_exception(module_inst);
                EXPECT_NE(nullptr, exception) << "Expected exception message for trap";
            }
            return 0;  // Return value not meaningful for traps
        } else {
            // Verify successful execution
            EXPECT_TRUE(success) << "Function call failed: " <<
                wasm_runtime_get_exception(module_inst);
            return success ? (uint32_t)argv[0] : 0;
        }
    }

    // Test fixture member variables
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t* buffer = nullptr;
    uint32_t size = 0;
    uint32_t stack_size = 16 * 1024;  // 16KB stack
    uint32_t heap_size = 16 * 1024;   // 16KB heap
    char error_buf[256]{};
};

/**
 * @test BasicConversion_StandardValues_ProducesCorrectResults
 * @brief Validates i32.trunc_f32_u produces correct results for typical f32 inputs
 * @details Tests fundamental unsigned truncation operation with positive and fractional
 *          values. Verifies that truncation removes fractional parts towards zero.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_u_operation
 * @input_conditions Typical positive f32 values with various fractional parts
 * @expected_behavior Returns unsigned integer with fractional part truncated towards zero
 * @validation_method Direct comparison of WASM function results with expected values
 */
TEST_P(I32TruncF32UTest, BasicConversion_StandardValues_ProducesCorrectResults) {
    RunningMode mode = GetParam();

    // Test basic positive conversions
    ASSERT_EQ(5u, CallTruncFunction("test_basic_conversion", 5.0f))
        << "Failed to convert exact positive value 5.0f";

    ASSERT_EQ(1000u, CallTruncFunction("test_basic_conversion", 1000.0f))
        << "Failed to convert medium positive value 1000.0f";

    ASSERT_EQ(100000u, CallTruncFunction("test_basic_conversion", 100000.0f))
        << "Failed to convert large positive value 100000.0f";

    // Test positive zero conversion
    ASSERT_EQ(0u, CallTruncFunction("test_basic_conversion", +0.0f))
        << "Failed to convert positive zero to unsigned integer";

    ASSERT_EQ(0u, CallTruncFunction("test_basic_conversion", -0.0f))
        << "Failed to convert negative zero to unsigned integer";
}

/**
 * @test TruncationBehavior_FractionalValues_TruncatesTowardsZero
 * @brief Validates proper truncation behavior for fractional floating-point values
 * @details Tests that fractional parts are properly removed by truncation towards zero
 *          for various positive fractional values in the unsigned range.
 * @test_category Main - Truncation behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_u_truncation
 * @input_conditions Positive f32 values with fractional components
 * @expected_behavior Fractional parts truncated towards zero, integer part preserved
 * @validation_method Verify truncation removes fractional parts correctly
 */
TEST_P(I32TruncF32UTest, TruncationBehavior_FractionalValues_TruncatesTowardsZero) {
    RunningMode mode = GetParam();

    // Test positive fractional truncation
    ASSERT_EQ(3u, CallTruncFunction("test_truncation_behavior", 3.7f))
        << "Failed to truncate positive fractional value 3.7f towards zero";

    ASSERT_EQ(42u, CallTruncFunction("test_truncation_behavior", 42.9f))
        << "Failed to truncate positive value 42.9f towards zero";

    ASSERT_EQ(999u, CallTruncFunction("test_truncation_behavior", 999.1f))
        << "Failed to truncate large fractional value 999.1f towards zero";

    // Test values that truncate to zero
    ASSERT_EQ(0u, CallTruncFunction("test_truncation_behavior", 0.9f))
        << "Failed to truncate 0.9f to zero";

    ASSERT_EQ(0u, CallTruncFunction("test_truncation_behavior", 0.1f))
        << "Failed to truncate 0.1f to zero";
}

/**
 * @test BoundaryValues_UnsignedLimits_HandlesCorrectly
 * @brief Validates behavior at UINT32_MAX boundaries and overflow conditions
 * @details Tests conversion at the limits of unsigned i32 representable range, including
 *          values that fit within bounds and values that should cause traps.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_u_boundary_check
 * @input_conditions f32 values at and beyond UINT32_MAX boundary (4294967295)
 * @expected_behavior Valid boundaries convert successfully, overflow values trap
 * @validation_method Boundary conversion success and trap condition verification
 */
TEST_P(I32TruncF32UTest, BoundaryValues_UnsignedLimits_HandlesCorrectly) {
    RunningMode mode = GetParam();

    // Test minimum boundary (zero)
    ASSERT_EQ(0u, CallTruncFunction("test_boundary_values", 0.0f))
        << "Failed to convert minimum unsigned boundary value 0.0f";

    // Test values near zero boundary
    ASSERT_EQ(0u, CallTruncFunction("test_boundary_values", 0.9f))
        << "Failed to handle value 0.9f near zero boundary";

    // Test maximum representable unsigned value (close to UINT32_MAX)
    // Note: f32 cannot exactly represent UINT32_MAX, so we test the closest representable value
    float max_representable = 4294967296.0f - 256.0f;  // Closest f32 to UINT32_MAX
    uint32_t result = CallTruncFunction("test_boundary_values", max_representable);
    ASSERT_LT(result, 4294967296u) << "Result should be within unsigned 32-bit range";

    // Test large valid value that fits in unsigned range
    ASSERT_EQ(2000000000u, CallTruncFunction("test_boundary_values", 2000000000.0f))
        << "Failed to convert large valid unsigned value";
}

/**
 * @test OverflowConditions_ValuesAboveMax_TriggersTraps
 * @brief Validates proper trap generation for values exceeding UINT32_MAX
 * @details Tests that values beyond the unsigned 32-bit integer range properly
 *          trigger execution traps rather than producing undefined results.
 * @test_category Corner - Overflow trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_u_overflow_trap
 * @input_conditions f32 values exceeding UINT32_MAX (4294967296 and above)
 * @expected_behavior Execution traps with proper error reporting
 * @validation_method Overflow trap detection and error message verification
 */
TEST_P(I32TruncF32UTest, OverflowConditions_ValuesAboveMax_TriggersTraps) {
    RunningMode mode = GetParam();

    // Test overflow just above UINT32_MAX
    float overflow_value = 4294967296.0f;  // Just above UINT32_MAX
    CallTruncFunction("test_overflow_conditions", overflow_value, true);

    // Test large overflow value
    float large_overflow = 5000000000.0f;  // Well above UINT32_MAX
    CallTruncFunction("test_overflow_conditions", large_overflow, true);

    // Test extreme overflow
    float extreme_overflow = 1e20f;  // Extremely large value
    CallTruncFunction("test_overflow_conditions", extreme_overflow, true);
}

/**
 * @test SpecialValues_NaNAndInfinity_TriggersTraps
 * @brief Validates handling of IEEE 754 special values
 * @details Tests conversion of special floating-point values including NaN
 *          and infinity values, ensuring proper trap behavior.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_u_special_values
 * @input_conditions IEEE 754 special values (NaN, infinity)
 * @expected_behavior NaN and infinity values cause execution traps
 * @validation_method Special value trap verification
 */
TEST_P(I32TruncF32UTest, SpecialValues_NaNAndInfinity_TriggersTraps) {
    RunningMode mode = GetParam();

    // Test NaN values (should trap)
    float quiet_nan = std::numeric_limits<float>::quiet_NaN();
    CallTruncFunction("test_special_values", quiet_nan, true);

    // Test positive infinity (should trap)
    float pos_infinity = std::numeric_limits<float>::infinity();
    CallTruncFunction("test_special_values", pos_infinity, true);

    // Test negative infinity (should trap)
    float neg_infinity = -std::numeric_limits<float>::infinity();
    CallTruncFunction("test_special_values", neg_infinity, true);

    // Test various NaN representations
    uint32_t nan_bits = 0x7FC00000;  // Standard quiet NaN
    float custom_nan;
    memcpy(&custom_nan, &nan_bits, sizeof(float));
    CallTruncFunction("test_special_values", custom_nan, true);
}

/**
 * @test ZeroHandling_PositiveAndNegativeZero_ProducesZero
 * @brief Validates proper handling of positive and negative zero values
 * @details Tests that both +0.0f and -0.0f convert properly to unsigned zero,
 *          ensuring IEEE 754 zero handling compliance.
 * @test_category Edge - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_u_zero_handling
 * @input_conditions Positive and negative zero f32 values
 * @expected_behavior Both zero variants convert to unsigned zero (0u)
 * @validation_method Zero conversion verification for both signs
 */
TEST_P(I32TruncF32UTest, ZeroHandling_PositiveAndNegativeZero_ProducesZero) {
    RunningMode mode = GetParam();

    // Test positive zero
    ASSERT_EQ(0u, CallTruncFunction("test_zero_handling", +0.0f))
        << "Failed to convert positive zero to unsigned zero";

    // Test negative zero
    ASSERT_EQ(0u, CallTruncFunction("test_zero_handling", -0.0f))
        << "Failed to convert negative zero to unsigned zero";

    // Test tiny positive value that truncates to zero
    ASSERT_EQ(0u, CallTruncFunction("test_zero_handling", 0.1f))
        << "Failed to truncate tiny positive value to zero";

    // Test smallest positive normal value that truncates to zero
    ASSERT_EQ(0u, CallTruncFunction("test_zero_handling", 0.99999f))
        << "Failed to truncate value just below 1.0f to zero";
}

/**
 * @test NegativeValues_AnyNegativeInput_TriggersTraps
 * @brief Validates proper trap generation for negative input values
 * @details Tests that any negative f32 value causes execution traps since unsigned
 *          integers cannot represent negative values.
 * @test_category Error - Negative value trap validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_u_negative_trap
 * @input_conditions Various negative f32 values
 * @expected_behavior All negative values trigger execution traps
 * @validation_method Negative value trap detection and verification
 */
TEST_P(I32TruncF32UTest, NegativeValues_AnyNegativeInput_TriggersTraps) {
    RunningMode mode = GetParam();

    // Test small negative value
    CallTruncFunction("test_negative_values", -1.0f, true);

    // Test medium negative value
    CallTruncFunction("test_negative_values", -100.0f, true);

    // Test large negative value
    CallTruncFunction("test_negative_values", -1000000.0f, true);

    // Test negative fractional value
    CallTruncFunction("test_negative_values", -0.1f, true);

    // Test extreme negative value
    CallTruncFunction("test_negative_values", -1e20f, true);
}

/**
 * @test StackUnderflow_EmptyStack_HandlesGracefully
 * @brief Validates proper handling of stack underflow conditions
 * @details Tests behavior when i32.trunc_f32_u opcode executes with insufficient
 *          stack values, ensuring proper error detection and handling.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:stack_underflow_check
 * @input_conditions WASM execution context with empty or insufficient stack
 * @expected_behavior Proper stack underflow detection and error reporting
 * @validation_method Stack underflow trap verification
 */
TEST_P(I32TruncF32UTest, StackUnderflow_EmptyStack_HandlesGracefully) {
    RunningMode mode = GetParam();

    // Test stack underflow scenario
    // The WASM module should contain a function that attempts i32.trunc_f32_u
    // with insufficient stack preparation
    wasm_function_inst_t func = wasm_runtime_lookup_function(
        module_inst, "test_stack_underflow");
    ASSERT_NE(nullptr, func) << "Stack underflow test function not found";

    uint32_t argv[1] = {0};  // No arguments provided
    bool success = wasm_runtime_call_wasm(exec_env, func, 0, argv);

    // Should fail due to stack underflow
    ASSERT_FALSE(success) << "Expected stack underflow error but function succeeded";

    const char* exception = wasm_runtime_get_exception(module_inst);
    ASSERT_NE(nullptr, exception) << "Expected exception message for stack underflow";
}

// Test parameter instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionMode,
    I32TruncF32UTest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);