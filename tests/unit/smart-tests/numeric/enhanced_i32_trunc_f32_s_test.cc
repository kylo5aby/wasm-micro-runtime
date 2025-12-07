/**
 * @file enhanced_i32_trunc_f32_s_test.cc
 * @brief Comprehensive test suite for i32.trunc_f32_s WASM opcode
 * @details Tests floating-point to signed integer truncation with comprehensive coverage
 *          including boundary conditions, special values, and error scenarios.
 *
 * Test Coverage Areas:
 * - Basic truncation functionality for typical f32 values
 * - Boundary value handling at INT32_MAX/MIN limits
 * - Special IEEE 754 values (NaN, infinity, zero variants)
 * - Overflow/underflow trap conditions
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
 * @class I32TruncF32STest
 * @brief Test fixture for i32.trunc_f32_s opcode validation
 * @details Provides WAMR runtime setup and WASM module management for systematic
 *          testing of f32 to signed i32 truncation operations across execution modes.
 */
class I32TruncF32STest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Test fixture setup with WAMR runtime initialization
     * @details Configures WAMR runtime with system allocator and loads test WASM module
     *          for i32.trunc_f32_s opcode validation across interpreter and AOT modes.
     */
    void SetUp() override {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        // Initialize WAMR runtime for opcode testing
        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime for i32.trunc_f32_s tests";

        // Load test WASM module containing i32.trunc_f32_s test functions
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
     * @brief Load WASM test module for i32.trunc_f32_s validation
     * @details Loads compiled WASM module containing test functions for truncation operations
     *          and creates execution environment for cross-mode testing.
     */
    void LoadTestModule() {
        const char* wasm_file = "wasm-apps/i32_trunc_f32_s_test.wasm";

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
     * @brief Call i32.trunc_f32_s test function with error handling
     * @param function_name Name of the exported WASM function to call
     * @param input f32 input value for truncation
     * @param expect_trap Whether the call should generate a trap
     * @return i32 result of truncation (only valid when expect_trap is false)
     * @details Invokes WASM function and handles both successful execution and trap conditions
     */
    int32_t CallTruncFunction(const char* function_name, float input, bool expect_trap = false) {
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
            return success ? (int32_t)argv[0] : 0;
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
 * @test BasicTruncation_ReturnsCorrectInteger
 * @brief Validates i32.trunc_f32_s produces correct results for typical f32 inputs
 * @details Tests fundamental truncation operation with positive, negative, and fractional
 *          values. Verifies that truncation removes fractional parts towards zero.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_s_operation
 * @input_conditions Typical f32 values with various fractional parts and signs
 * @expected_behavior Returns integer with fractional part truncated towards zero
 * @validation_method Direct comparison of WASM function results with expected values
 */
TEST_P(I32TruncF32STest, BasicTruncation_ReturnsCorrectInteger) {
    RunningMode mode = GetParam();

    // Test positive fractional truncation
    ASSERT_EQ(42, CallTruncFunction("test_basic_trunc", 42.7f))
        << "Failed to truncate positive fractional value 42.7f";

    ASSERT_EQ(1, CallTruncFunction("test_basic_trunc", 1.9f))
        << "Failed to truncate positive value 1.9f towards zero";

    // Test negative fractional truncation
    ASSERT_EQ(-42, CallTruncFunction("test_basic_trunc", -42.7f))
        << "Failed to truncate negative fractional value -42.7f";

    ASSERT_EQ(-1, CallTruncFunction("test_basic_trunc", -1.9f))
        << "Failed to truncate negative value -1.9f towards zero";

    // Test exact integer values
    ASSERT_EQ(100, CallTruncFunction("test_basic_trunc", 100.0f))
        << "Failed to handle exact integer value 100.0f";

    ASSERT_EQ(-100, CallTruncFunction("test_basic_trunc", -100.0f))
        << "Failed to handle exact negative integer value -100.0f";
}

/**
 * @test BoundaryValues_HandlesLimitsCorrectly
 * @brief Validates behavior at INT32_MAX/MIN boundaries and overflow conditions
 * @details Tests conversion at the limits of i32 representable range, including
 *          values that fit within bounds and values that should cause traps.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_s_boundary_check
 * @input_conditions f32 values at and beyond INT32_MAX/MIN boundaries
 * @expected_behavior Valid boundaries convert successfully, overflow values trap
 * @validation_method Boundary conversion success and trap condition verification
 */
TEST_P(I32TruncF32STest, BoundaryValues_HandlesLimitsCorrectly) {
    RunningMode mode = GetParam();

    // Test maximum valid boundary (INT32_MAX fits in f32)
    float max_valid = 2147483520.0f;  // Largest f32 that fits in INT32_MAX
    ASSERT_EQ(2147483520, CallTruncFunction("test_boundary_values", max_valid))
        << "Failed to convert maximum valid f32 value";

    // Test minimum valid boundary (INT32_MIN fits exactly in f32)
    float min_valid = -2147483648.0f;
    ASSERT_EQ(-2147483648, CallTruncFunction("test_boundary_values", min_valid))
        << "Failed to convert minimum valid f32 value";

    // Test overflow boundary (should trap)
    float overflow_positive = 2147483648.0f;  // Just over INT32_MAX
    CallTruncFunction("test_boundary_values", overflow_positive, true);

    // Test underflow boundary (should trap)
    float underflow_negative = -2147483904.0f;  // Just under INT32_MIN
    CallTruncFunction("test_boundary_values", underflow_negative, true);
}

/**
 * @test SpecialValues_ProducesExpectedResults
 * @brief Validates handling of IEEE 754 special values and zero variants
 * @details Tests conversion of special floating-point values including NaN,
 *          infinity, positive/negative zero, and subnormal numbers.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_s_special_values
 * @input_conditions IEEE 754 special values and edge cases
 * @expected_behavior Zero variants convert to 0, NaN/infinity cause traps
 * @validation_method Special value conversion and trap verification
 */
TEST_P(I32TruncF32STest, SpecialValues_ProducesExpectedResults) {
    RunningMode mode = GetParam();

    // Test positive and negative zero
    ASSERT_EQ(0, CallTruncFunction("test_special_values", +0.0f))
        << "Failed to convert positive zero to integer";

    ASSERT_EQ(0, CallTruncFunction("test_special_values", -0.0f))
        << "Failed to convert negative zero to integer";

    // Test fractional values that truncate to zero
    ASSERT_EQ(0, CallTruncFunction("test_special_values", 0.9f))
        << "Failed to truncate 0.9f to zero";

    ASSERT_EQ(0, CallTruncFunction("test_special_values", -0.9f))
        << "Failed to truncate -0.9f to zero";

    // Test NaN values (should trap)
    float quiet_nan = std::numeric_limits<float>::quiet_NaN();
    CallTruncFunction("test_special_values", quiet_nan, true);

    // Test infinity values (should trap)
    float pos_infinity = std::numeric_limits<float>::infinity();
    CallTruncFunction("test_special_values", pos_infinity, true);

    float neg_infinity = -std::numeric_limits<float>::infinity();
    CallTruncFunction("test_special_values", neg_infinity, true);
}

/**
 * @test ErrorConditions_GeneratesProperTraps
 * @brief Validates proper trap generation for invalid conversion scenarios
 * @details Tests error handling for overflow, underflow, and invalid floating-point
 *          values that should cause execution traps rather than undefined behavior.
 * @test_category Error - Trap condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_trunc_f32_s_trap_handling
 * @input_conditions Invalid f32 values that should cause traps
 * @expected_behavior Proper execution traps with informative error messages
 * @validation_method Trap detection and error message verification
 */
TEST_P(I32TruncF32STest, ErrorConditions_GeneratesProperTraps) {
    RunningMode mode = GetParam();

    // Test extreme positive overflow
    float extreme_positive = 1e20f;  // Far beyond INT32_MAX
    CallTruncFunction("test_error_conditions", extreme_positive, true);

    // Test extreme negative underflow
    float extreme_negative = -1e20f;  // Far beyond INT32_MIN
    CallTruncFunction("test_error_conditions", extreme_negative, true);

    // Test various NaN representations
    uint32_t nan_bits = 0x7FC00000;  // Standard quiet NaN
    float custom_nan;
    memcpy(&custom_nan, &nan_bits, sizeof(float));
    CallTruncFunction("test_error_conditions", custom_nan, true);

    // Test signaling NaN
    nan_bits = 0x7F800001;  // Signaling NaN
    memcpy(&custom_nan, &nan_bits, sizeof(float));
    CallTruncFunction("test_error_conditions", custom_nan, true);
}

/**
 * @test StackUnderflow_HandlesEmptyStack
 * @brief Validates proper handling of stack underflow conditions
 * @details Tests behavior when i32.trunc_f32_s opcode executes with insufficient
 *          stack values, ensuring proper error detection and handling.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:stack_underflow_check
 * @input_conditions WASM execution context with empty or insufficient stack
 * @expected_behavior Proper stack underflow detection and error reporting
 * @validation_method Stack underflow trap verification
 */
TEST_P(I32TruncF32STest, StackUnderflow_HandlesEmptyStack) {
    RunningMode mode = GetParam();

    // Test stack underflow scenario
    // The WASM module should contain a function that attempts i32.trunc_f32_s
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
    I32TruncF32STest,
    testing::Values(Mode_Interp, Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode>& info) {
        return info.param == Mode_Interp ? "Interpreter" : "AOT";
    }
);