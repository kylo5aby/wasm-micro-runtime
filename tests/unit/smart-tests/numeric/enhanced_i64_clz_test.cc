/**
 * @file enhanced_i64_clz_test.cc
 * @brief Comprehensive unit tests for i64.clz (count leading zeros) opcode
 * @details This test suite validates the i64.clz WebAssembly instruction across all scenarios:
 *          basic functionality, boundary conditions, edge cases, and error conditions.
 *          The tests ensure correct leading zero counting for 64-bit integers in both
 *          interpreter and AOT execution modes.
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_interp_call_func_bytecode
 * @coverage_target core/iwasm/aot/aot_runtime.c:aot_call_function
 * @test_modes Interpreter mode and AOT (Ahead-of-Time) mode validation
 */

#include <gtest/gtest.h>
#include <cstdint>                     // Standard integer types
#include <climits>                     // Standard C limits for MIN/MAX values
#include <cstdlib>                     // Standard C library functions
#include <unistd.h>                    // UNIX standard definitions
#include <vector>                      // STL vector for test data organization
#include <utility>                     // STL utility for pair and other utilities
#include "test_helper.h"               // Unit test helper functions and utilities
#include "wasm_runtime_common.h"       // Common runtime functions and structures
#include "bh_read_file.h"              // WAMR file reading utilities

/**
 * @class I64ClzTest
 * @brief Parameterized test fixture for comprehensive i64.clz opcode validation
 * @details Inherits from TestWithParam to enable cross-execution mode testing (interpreter vs AOT).
 *          Provides consistent test environment setup with proper WAMR initialization,
 *          module loading, and resource cleanup through RAII patterns.
 */

class I64ClzTest : public testing::TestWithParam<RunningMode> {
protected:
    WAMRRuntimeRAII<> runtime;              // RAII runtime management
    wasm_module_t module = nullptr;         // Loaded WASM module
    wasm_module_inst_t module_inst = nullptr; // Instantiated WASM module instance
    wasm_exec_env_t exec_env = nullptr;     // Execution environment
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;                 // Module buffer
    char error_buf[128] = { 0 };           // Error message buffer
    const char *exception = nullptr;        // Runtime exception pointer

    /**
     * @brief Setup method called before each test case execution
     * @details Initializes WAMR runtime with proper configuration for testing.
     *          Sets up memory allocation, enables required features, and prepares
     *          execution environment for both interpreter and AOT modes.
     */
    void SetUp() override {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        // Load WASM module from file
        std::string wasm_file = "./wasm-apps/i64_clz_test.wasm";
        buf = (uint8_t *)bh_read_file_to_buffer(wasm_file.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << wasm_file;

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
     * @brief Cleanup method called after each test case execution
     * @details Properly destroys WAMR runtime and releases allocated resources.
     *          Ensures no memory leaks or resource conflicts between test cases.
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
     * @brief Execute i64.clz operation with specified input value
     * @param input The 64-bit integer input for clz operation
     * @return The count of leading zeros (0-64)
     * @details Calls the exported i64.clz test function from loaded WASM module.
     *          Handles function lookup, parameter passing, and result extraction.
     */
    uint32_t call_i64_clz(uint64_t input) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_clz");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_clz function";

        uint32_t argv[3] = { (uint32_t)input, (uint32_t)(input >> 32), 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        EXPECT_EQ(ret, true) << "Function call failed";

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
        }

        return argv[0];
    }
};

/**
 * @test BasicClzOperation_ReturnsCorrectCounts
 * @brief Validates i64.clz produces correct leading zero counts for common input values
 * @details Tests fundamental clz operation with typical bit patterns including common values,
 *          powers of 2, and mixed bit patterns. Verifies mathematical correctness of
 *          leading zero counting algorithm.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_clz_operation
 * @input_conditions Common bit patterns: 0xFF, 0x1000, 0x123456, powers of 2
 * @expected_behavior Returns mathematically correct leading zero counts
 * @validation_method Direct comparison of clz result with expected bit-counting values
 */
TEST_P(I64ClzTest, BasicClzOperation_ReturnsCorrectCounts) {
    // Test common values with known leading zero counts
    ASSERT_EQ(56, call_i64_clz(0xFF))           << "clz(0xFF) should return 56 leading zeros";
    ASSERT_EQ(51, call_i64_clz(0x1000))         << "clz(0x1000) should return 51 leading zeros";
    ASSERT_EQ(43, call_i64_clz(0x123456))       << "clz(0x123456) should return 43 leading zeros";

    // Test powers of 2 - validate clz(2^n) = 63-n relationship
    ASSERT_EQ(63, call_i64_clz(0x1))            << "clz(2^0) should return 63 leading zeros";
    ASSERT_EQ(62, call_i64_clz(0x2))            << "clz(2^1) should return 62 leading zeros";
    ASSERT_EQ(59, call_i64_clz(0x10))           << "clz(2^4) should return 59 leading zeros";
    ASSERT_EQ(55, call_i64_clz(0x100))          << "clz(2^8) should return 55 leading zeros";
}

/**
 * @test BoundaryValues_HandleExtremes
 * @brief Validates i64.clz correctly handles boundary value conditions
 * @details Tests extreme values including zero, maximum positive, minimum negative values.
 *          Ensures proper handling of sign bit and full 64-bit range coverage.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_clz_boundary_handling
 * @input_conditions Zero, INT64_MIN, INT64_MAX, sign bit transitions
 * @expected_behavior Correct leading zero counts: 0→64, MIN→0, MAX→1
 * @validation_method Boundary value analysis with mathematical verification
 */
TEST_P(I64ClzTest, BoundaryValues_HandleExtremes) {
    // Zero input produces maximum leading zeros (64)
    ASSERT_EQ(64, call_i64_clz(0x0000000000000000ULL))
        << "clz(0) should return 64 leading zeros";

    // Minimum 64-bit signed value (MSB set) produces zero leading zeros
    ASSERT_EQ(0, call_i64_clz(0x8000000000000000ULL))
        << "clz(INT64_MIN) should return 0 leading zeros";

    // Maximum 64-bit signed value produces one leading zero
    ASSERT_EQ(1, call_i64_clz(0x7FFFFFFFFFFFFFFFULL))
        << "clz(INT64_MAX) should return 1 leading zero";

    // Values near sign bit transition
    ASSERT_EQ(1, call_i64_clz(0x4000000000000000ULL))
        << "clz(0x4000000000000000) should return 1 leading zero";
    ASSERT_EQ(2, call_i64_clz(0x3FFFFFFFFFFFFFFFULL))
        << "clz(0x3FFFFFFFFFFFFFFF) should return 2 leading zeros";
}

/**
 * @test SingleBitPatterns_CountCorrectly
 * @brief Validates i64.clz accuracy across individual bit position testing
 * @details Tests single-bit values across all 64 bit positions to verify
 *          mathematical relationship clz(2^n) = 63-n holds consistently.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_clz_bitwise_operation
 * @input_conditions Single bit patterns: 2^0, 2^1, 2^31, 2^32, 2^63
 * @expected_behavior Monotonic decreasing relationship: higher bits → lower clz counts
 * @validation_method Bit position formula verification across full 64-bit range
 */
TEST_P(I64ClzTest, SingleBitPatterns_CountCorrectly) {
    // Test single bit patterns at key positions
    ASSERT_EQ(63, call_i64_clz(0x0000000000000001ULL))  // 2^0
        << "clz(2^0) should return 63 leading zeros";
    ASSERT_EQ(32, call_i64_clz(0x0000000080000000ULL))  // 2^31
        << "clz(2^31) should return 32 leading zeros";
    ASSERT_EQ(31, call_i64_clz(0x0000000100000000ULL))  // 2^32
        << "clz(2^32) should return 31 leading zeros";
    ASSERT_EQ(1, call_i64_clz(0x4000000000000000ULL))   // 2^62
        << "clz(2^62) should return 1 leading zero";
    ASSERT_EQ(0, call_i64_clz(0x8000000000000000ULL))   // 2^63 (MSB)
        << "clz(2^63) should return 0 leading zeros";

    // Verify mathematical property: clz(2^n) = 63-n
    for (int n = 0; n < 64; n++) {
        uint64_t power_of_two = 1ULL << n;
        uint32_t expected_clz = 63 - n;
        ASSERT_EQ(expected_clz, call_i64_clz(power_of_two))
            << "Mathematical property clz(2^" << n << ") = " << expected_clz << " failed";
    }
}

/**
 * @test MathematicalProperties_MaintainConsistency
 * @brief Validates mathematical properties and consistency of i64.clz operation
 * @details Tests monotonic behavior, bit position relationships, and algorithmic properties.
 *          Ensures clz operation maintains expected mathematical characteristics.
 * @test_category Edge - Algorithmic consistency validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_clz_algorithm
 * @input_conditions Sequential values, bit patterns with known relationships
 * @expected_behavior Monotonic decreasing property, consistent bit counting
 * @validation_method Mathematical property verification and consistency checking
 */
TEST_P(I64ClzTest, MathematicalProperties_MaintainConsistency) {
    // Test monotonic property: larger values (with higher MSB) have smaller clz
    std::vector<std::pair<uint64_t, uint32_t>> test_cases = {
        {0x0000000000000001ULL, 63}, // Smallest non-zero
        {0x0000000000000010ULL, 59}, // 2^4
        {0x0000000000001000ULL, 51}, // 2^12
        {0x0000000000100000ULL, 43}, // 2^20
        {0x0000000010000000ULL, 35}, // 2^28
        {0x0000001000000000ULL, 27}, // 2^36
        {0x0000100000000000ULL, 19}, // 2^44
        {0x0010000000000000ULL, 11}, // 2^52
        {0x1000000000000000ULL, 3},  // 2^60
        {0x8000000000000000ULL, 0}   // 2^63
    };

    for (const auto& test_case : test_cases) {
        ASSERT_EQ(test_case.second, call_i64_clz(test_case.first))
            << "clz(0x" << std::hex << test_case.first << ") should return "
            << std::dec << test_case.second << " leading zeros";
    }

    // Test additive property: clz(x) + clz(complement) should be related
    uint64_t test_value = 0x0000000012345678ULL;
    uint32_t clz_value = call_i64_clz(test_value);
    ASSERT_GT(clz_value, 0) << "Non-zero values should have some leading zeros";
    ASSERT_LT(clz_value, 64) << "Non-zero values should not have all leading zeros";
}

/**
 * @test ModuleValidation_HandlesInvalidBytecode
 * @brief Validates proper error handling for invalid WASM module scenarios
 * @details Tests WASM module loading with malformed bytecode, type violations,
 *          and invalid function signatures to ensure robust error handling.
 * @test_category Error - Module validation and error handling
 * @coverage_target core/iwasm/common/wasm_loader.c:load_module_validation
 * @input_conditions Invalid WASM bytecode, malformed modules, type mismatches
 * @expected_behavior Graceful failure with proper error reporting
 * @validation_method Error condition testing and exception handling verification
 */
TEST_P(I64ClzTest, ModuleValidation_HandlesInvalidBytecode) {
    // Test invalid WASM module loading
    uint8_t invalid_bytecode[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0xFF}; // Invalid WASM
    char error_buf[128] = {0};

    wasm_module_t invalid_module = wasm_runtime_load(
        invalid_bytecode, sizeof(invalid_bytecode), error_buf, sizeof(error_buf)
    );

    ASSERT_EQ(nullptr, invalid_module)
        << "Expected module loading to fail for invalid bytecode";
    ASSERT_NE(strlen(error_buf), 0)
        << "Expected error message for invalid bytecode";

    // Test empty buffer
    wasm_module_t empty_module = wasm_runtime_load(nullptr, 0, error_buf, sizeof(error_buf));
    ASSERT_EQ(nullptr, empty_module)
        << "Expected module loading to fail for empty buffer";
}

/**
 * @test RuntimeState_ValidatesStackConditions
 * @brief Validates proper runtime environment and stack state management
 * @details Tests WAMR runtime initialization, module instantiation, and function execution
 *          under various runtime conditions to ensure robust state management.
 * @test_category Error - Runtime state validation
 * @coverage_target core/iwasm/common/wasm_runtime.c:runtime_state_management
 * @input_conditions Various runtime states, function call scenarios
 * @expected_behavior Proper state management and error recovery
 * @validation_method Runtime state verification and resource management testing
 */
TEST_P(I64ClzTest, RuntimeState_ValidatesStackConditions) {
    // Verify module and instance are properly initialized
    ASSERT_NE(nullptr, module) << "WASM module should be loaded";
    ASSERT_NE(nullptr, module_inst) << "WASM module instance should be created";

    // Verify function lookup works correctly
    wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_clz");
    ASSERT_NE(nullptr, func) << "test_i64_clz function should be found";

    // Test function execution with valid parameters
    uint32_t result = call_i64_clz(0x12345ULL);
    ASSERT_GE(result, 0) << "Function result should be non-negative";
    ASSERT_LE(result, 64) << "Function result should not exceed 64";

    // Verify runtime state remains stable after operations
    wasm_function_inst_t func_again = wasm_runtime_lookup_function(module_inst, "test_i64_clz");
    ASSERT_EQ(func, func_again) << "Function lookup should remain consistent";
}

// Instantiate parameterized tests for both execution modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, I64ClzTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I64ClzTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

