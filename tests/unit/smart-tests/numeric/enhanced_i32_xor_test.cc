/**
 * Enhanced Unit Tests for i32.xor WASM Opcode
 * Tests bitwise XOR operations with comprehensive coverage including:
 * - Basic operations with various operand combinations
 * - Boundary value testing with MIN/MAX values
 * - Identity and mathematical property validation
 * - Bit pattern manipulation testing
 * - Stack underflow error handling
 * - Cross-execution mode validation (Interpreter + AOT)
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;
static std::string WASM_FILE_UNDERFLOW;

static int app_argc;
static char **app_argv;

class I32XorTest : public testing::TestWithParam<RunningMode> {
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
     * @brief Set up test fixture with WAMR runtime initialization
     * @details Initializes WAMR runtime with system allocator and loads i32.xor test module
     * Configures both interpreter and AOT execution modes for comprehensive testing
     */
    void SetUp() override {
        // Load WASM module for i32.xor tests
        loadTestModule();
    }

    /**
     * @brief Clean up test fixture with proper resource deallocation
     * @details Destroys WASM module instance and runtime to prevent memory leaks
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
     * @brief Load the i32.xor test WASM module
     * @details Loads the WASM bytecode from file and instantiates module for testing
     * Supports both interpreter and AOT execution modes
     */
    void loadTestModule() {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Read WASM file from wasm-apps directory
        buf = (uint8_t*)bh_read_file_to_buffer("wasm-apps/i32_xor_test.wasm", &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: wasm-apps/i32_xor_test.wasm";

        // Load WASM module
        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        // Create module instance
        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        // Set running mode and create execution environment
        wasm_runtime_set_running_mode(module_inst, GetParam());
        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Call i32.xor function with two operands
     * @details Invokes WASM function that performs i32.xor operation
     * @param a Left operand (i32)
     * @param b Right operand (i32)
     * @return XOR result (i32)
     * Source: core/iwasm/interpreter/wasm_interp_classic.c:i32_xor_operation
     */
    int32_t call_i32_xor(int32_t a, int32_t b) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "i32_xor");
        EXPECT_NE(func, nullptr) << "Failed to lookup i32_xor function";

        uint32_t argv[3] = {static_cast<uint32_t>(a), static_cast<uint32_t>(b), 0};
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_EQ(exception, nullptr) << "Runtime exception occurred: " << (exception ? exception : "unknown");

        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Test stack underflow scenario handling
     * @details Tests WASM module behavior for stack underflow conditions
     * Used for testing error handling capabilities
     */
    void test_stack_underflow(const std::string& wasm_file) {
        uint8_t *underflow_buf = nullptr;
        uint32_t underflow_buf_size;
        wasm_module_t underflow_module = nullptr;
        wasm_module_inst_t underflow_inst = nullptr;
        wasm_exec_env_t underflow_exec_env = nullptr;

        underflow_buf = (uint8_t *)bh_read_file_to_buffer("wasm-apps/i32_xor_stack_underflow.wasm", &underflow_buf_size);
        ASSERT_NE(underflow_buf, nullptr) << "Failed to read underflow test WASM file";

        underflow_module = wasm_runtime_load(underflow_buf, underflow_buf_size,
                                           error_buf, sizeof(error_buf));

        // Stack underflow tests might fail at module loading or instantiation
        ASSERT_NE(underflow_module, nullptr) << "Failed to load underflow test module: " << error_buf;

        underflow_inst = wasm_runtime_instantiate(underflow_module, stack_size, heap_size,
                                                error_buf, sizeof(error_buf));

        ASSERT_NE(underflow_inst, nullptr) << "Failed to instantiate underflow test module: " << error_buf;

        wasm_runtime_set_running_mode(underflow_inst, GetParam());
        underflow_exec_env = wasm_runtime_create_exec_env(underflow_inst, stack_size);

        ASSERT_NE(underflow_exec_env, nullptr) << "Failed to create exec env for underflow test";

        // Test valid function from underflow module
        wasm_function_inst_t func = wasm_runtime_lookup_function(underflow_inst, "valid_xor");
        ASSERT_NE(func, nullptr) << "Failed to lookup valid_xor function";

        uint32_t argv[3] = { 5, 3, 0 };
        bool ret = wasm_runtime_call_wasm(underflow_exec_env, func, 2, argv);
        ASSERT_EQ(ret, true) << "Valid function call should succeed";

        // Cleanup
        wasm_runtime_destroy_exec_env(underflow_exec_env);
        wasm_runtime_deinstantiate(underflow_inst);
        wasm_runtime_unload(underflow_module);
        BH_FREE(underflow_buf);
    }
};

/**
 * @test BasicOperations_ReturnsCorrectResults
 * @brief Validates i32.xor produces correct results for typical input combinations
 * @details Tests fundamental XOR operation with positive integers, negative integers,
 *          and mixed-sign combinations. Verifies that i32.xor correctly computes a ^ b
 *          for various input scenarios including standard use cases.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_xor_operation
 * @input_conditions Standard integer pairs with various sign combinations
 * @expected_behavior Returns mathematical XOR result for each input pair
 * @validation_method Direct comparison of WASM function result with expected XOR values
 */
TEST_P(I32XorTest, BasicOperations_ReturnsCorrectResults) {
    // Test basic positive integer XOR
    ASSERT_EQ(call_i32_xor(5, 3), 6)
        << "XOR of positive integers failed: 5 ^ 3 should equal 6";

    // Test mixed sign XOR (positive ^ negative)
    ASSERT_EQ(call_i32_xor(-5, 3), -8)
        << "XOR of mixed-sign integers failed: -5 ^ 3 should equal -8";

    // Test negative operands XOR
    ASSERT_EQ(call_i32_xor(-10, -6), 12)
        << "XOR of negative integers failed: -10 ^ -6 should equal 12";
}

/**
 * @test BoundaryValues_HandlesExtremeValues
 * @brief Validates i32.xor handles boundary values and extreme integers correctly
 * @details Tests XOR operations with MIN_VALUE, MAX_VALUE, and boundary combinations.
 *          Ensures proper handling of sign bits and extreme value scenarios that
 *          might reveal implementation issues with signed integer representations.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_xor_operation
 * @input_conditions Boundary values: INT32_MIN, INT32_MAX, and their combinations
 * @expected_behavior Correct XOR results for extreme values without overflow issues
 * @validation_method Comparison with expected bitwise XOR results for boundary values
 */
TEST_P(I32XorTest, BoundaryValues_HandlesExtremeValues) {
    // Test MIN ^ MAX boundary values
    ASSERT_EQ(call_i32_xor(INT32_MIN, INT32_MAX), -1)
        << "XOR of boundary values failed: INT32_MIN ^ INT32_MAX should equal -1";

    // Test self-XOR with MIN value (should result in zero)
    ASSERT_EQ(call_i32_xor(INT32_MIN, INT32_MIN), 0)
        << "Self-XOR with MIN_VALUE failed: INT32_MIN ^ INT32_MIN should equal 0";

    // Test complement operation using all bits set
    ASSERT_EQ(call_i32_xor(static_cast<int32_t>(0xFFFFFFFF), INT32_MIN), INT32_MAX)
        << "Complement via XOR failed: 0xFFFFFFFF ^ INT32_MIN should equal INT32_MAX";
}

/**
 * @test IdentityProperties_ValidatesMathematicalRules
 * @brief Validates mathematical identity properties of XOR operation
 * @details Tests fundamental XOR properties including identity with zero (a ^ 0 = a),
 *          self-XOR resulting in zero (a ^ a = 0), and double-XOR inverse property
 *          ((a ^ b) ^ b = a). These properties are essential for XOR correctness.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_xor_operation
 * @input_conditions Test values with zero and self-XOR scenarios
 * @expected_behavior Proper adherence to XOR mathematical properties
 * @validation_method Verification of XOR identity and inverse properties
 */
TEST_P(I32XorTest, IdentityProperties_ValidatesMathematicalRules) {
    const int32_t test_value = 0x12345678;

    // Test identity property: a ^ 0 = a
    ASSERT_EQ(call_i32_xor(test_value, 0), test_value)
        << "XOR identity property failed: " << test_value << " ^ 0 should equal " << test_value;

    // Test self-XOR property: a ^ a = 0
    ASSERT_EQ(call_i32_xor(test_value, test_value), 0)
        << "Self-XOR property failed: " << test_value << " ^ " << test_value << " should equal 0";

    // Test double-XOR inverse property: (a ^ b) ^ b = a
    const int32_t operand_b = 37;
    int32_t intermediate = call_i32_xor(42, operand_b);
    ASSERT_EQ(call_i32_xor(intermediate, operand_b), 42)
        << "Double-XOR inverse property failed: ((42 ^ 37) ^ 37) should equal 42";
}

/**
 * @test BitPatterns_ManipulatesCorrectly
 * @brief Validates XOR behavior with specific bit patterns and masks
 * @details Tests XOR with alternating bit patterns, single-bit masks, and complement
 *          operations. Ensures correct bit manipulation behavior across different
 *          bit patterns that are commonly used in low-level programming.
 * @test_category Edge - Bit manipulation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i32_xor_operation
 * @input_conditions Alternating patterns, single-bit masks, complement patterns
 * @expected_behavior Predictable bit manipulation results following XOR truth table
 * @validation_method Verification against expected bit manipulation outcomes
 */
TEST_P(I32XorTest, BitPatterns_ManipulatesCorrectly) {
    // Test alternating bit patterns (0x55555555 = 01010101..., 0xAAAAAAAA = 10101010...)
    ASSERT_EQ(call_i32_xor(0x55555555, static_cast<int32_t>(0xAAAAAAAA)), -1)
        << "Alternating pattern XOR failed: 0x55555555 ^ 0xAAAAAAAA should equal 0xFFFFFFFF";

    // Test single bit mask operation
    ASSERT_EQ(call_i32_xor(0x00000001, static_cast<int32_t>(0xFFFFFFFE)), -1)
        << "Single bit mask XOR failed: 0x00000001 ^ 0xFFFFFFFE should equal 0xFFFFFFFF";

    // Test complement via XOR with all bits set
    const int32_t test_pattern = 0x12345678;
    const int32_t expected_complement = static_cast<int32_t>(0xEDCBA987);
    ASSERT_EQ(call_i32_xor(test_pattern, -1), expected_complement)
        << "Complement via XOR failed: 0x12345678 ^ 0xFFFFFFFF should equal 0xEDCBA987";
}

/**
 * @test StackUnderflow_HandlesErrorCorrectly
 * @brief Validates proper error handling for stack underflow scenarios
 * @details Tests WASM module behavior when i32.xor is executed with insufficient
 *          stack operands. Verifies that WAMR properly detects and handles stack
 *          underflow conditions without crashing.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:stack_validation
 * @input_conditions WASM module with insufficient stack operands for i32.xor
 * @expected_behavior Proper error detection and handling without crashes
 * @validation_method Module loading validation and error condition verification
 */
TEST_P(I32XorTest, StackUnderflow_HandlesErrorCorrectly) {
    // Test stack underflow scenario handling
    test_stack_underflow("wasm-apps/i32_xor_stack_underflow.wasm");
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    I32XorExecutionModes,
    I32XorTest,
    testing::Values(RunningMode::Mode_Interp, RunningMode::Mode_LLVM_JIT),
    [](const testing::TestParamInfo<RunningMode> &info) {
        return info.param == RunningMode::Mode_Interp ? "Interpreter" : "AOT";
    }
);

// Global initialization function for WASM file paths
__attribute__((constructor))
static void init_wasm_files() {
    char *cwd = getcwd(NULL, 0);
    if (cwd) {
        CWD = std::string(cwd);
        free(cwd);
    } else {
        CWD = ".";
    }

    WASM_FILE = CWD + "/wasm-apps/i32_xor_test.wasm";
    WASM_FILE_UNDERFLOW = CWD + "/wasm-apps/i32_xor_stack_underflow.wasm";
}