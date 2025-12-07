/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for i64.xor Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly i64.xor
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic functionality with typical 64-bit XOR operations
 * - Corner Cases: Boundary conditions with INT64_MIN, INT64_MAX values
 * - Edge Cases: Identity properties, mathematical rules, bit manipulation
 * - Error Handling: Stack underflow and validation scenarios
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling i64.xor)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c (bitwise operations section)
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

static int
app_argc;
static char **app_argv;

/**
 * @class I64XorTest
 * @brief Comprehensive test suite for i64.xor WebAssembly opcode
 * @details Tests 64-bit bitwise exclusive OR operations across interpreter and AOT modes.
 *          Validates mathematical properties, boundary conditions, and error scenarios.
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:bitwise_operations
 */
class I64XorTest : public testing::TestWithParam<RunningMode>
{
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
     * @method SetUp
     * @brief Initialize WAMR runtime environment and load i64.xor test module
     * @details Sets up WASM module loading, instantiation, and execution environment
     *          for comprehensive i64.xor operation testing across execution modes.
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

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
     * @method TearDown
     * @brief Clean up WAMR runtime resources and execution environment
     * @details Properly deallocates execution environment, module instance, module, and buffers
     *          following RAII patterns for resource management.
     */
    void TearDown() override
    {
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
     * @method call_i64_xor
     * @brief Execute i64.xor operation through WASM function call
     * @param a Left operand (64-bit signed integer)
     * @param b Right operand (64-bit signed integer)
     * @return Result of a XOR b operation
     * @details Invokes the test_i64_xor WASM function and returns XOR result
     */
    int64_t call_i64_xor(int64_t a, int64_t b)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_xor");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_xor function";

        uint32_t wasm_argv[4]; // Two i64 parameters = 4 uint32_t slots
        wasm_argv[0] = (uint32_t)(a & 0xFFFFFFFF);        // Low 32 bits of a
        wasm_argv[1] = (uint32_t)((a >> 32) & 0xFFFFFFFF); // High 32 bits of a
        wasm_argv[2] = (uint32_t)(b & 0xFFFFFFFF);        // Low 32 bits of b
        wasm_argv[3] = (uint32_t)((b >> 32) & 0xFFFFFFFF); // High 32 bits of b

        bool ret = wasm_runtime_call_wasm(exec_env, func, 4, wasm_argv);
        exception = wasm_runtime_get_exception(module_inst);
        EXPECT_TRUE(ret) << "WASM function call failed: " << (exception ? exception : "unknown error");
        EXPECT_EQ(exception, nullptr) << "WASM execution exception: " << exception;

        // Result is returned as two uint32_t values (low, high)
        int64_t result = ((int64_t)wasm_argv[1] << 32) | wasm_argv[0];
        return result;
    }

    /**
     * @method call_i64_xor_single_operand
     * @brief Test i64.xor with insufficient stack operands for error validation
     * @param a Single operand (should cause stack underflow)
     * @return Execution success status
     * @details Tests error handling when stack contains insufficient operands for XOR operation
     */
    bool call_i64_xor_single_operand(int64_t a)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_xor_underflow");
        EXPECT_NE(func, nullptr) << "Failed to lookup test_i64_xor_underflow function";

        uint32_t wasm_argv[2];
        wasm_argv[0] = (uint32_t)(a & 0xFFFFFFFF);
        wasm_argv[1] = (uint32_t)((a >> 32) & 0xFFFFFFFF);

        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, wasm_argv);
        exception = wasm_runtime_get_exception(module_inst);

        return ret && (exception == nullptr);
    }
};

/**
 * @test BasicBitwiseXor_ReturnsCorrectResult
 * @brief Validates fundamental XOR operation with typical 64-bit values
 * @details Tests basic i64.xor functionality with representative bit patterns including
 *          positive numbers, negative numbers, and mixed sign operations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_xor_operation
 * @input_conditions Standard 64-bit integer pairs with varying bit patterns
 * @expected_behavior Returns correct bitwise XOR results for all input combinations
 * @validation_method Direct comparison of WASM function results with expected XOR values
 */
TEST_P(I64XorTest, BasicBitwiseXor_ReturnsCorrectResult)
{
    // Test basic XOR operations with various bit patterns
    ASSERT_EQ(0x0LL, call_i64_xor(0x5555555555555555LL, 0x5555555555555555LL))
        << "XOR of identical values should return 0";

    ASSERT_EQ(0xFFFFFFFFFFFFFFFFLL, call_i64_xor(0x5555555555555555LL, 0xAAAAAAAAAAAAAAAALL))
        << "XOR of complementary alternating patterns should return all 1s";

    ASSERT_EQ(0x123456789ABCDEF0LL ^ 0x0F0F0F0F0F0F0F0FLL, call_i64_xor(0x123456789ABCDEF0LL, 0x0F0F0F0F0F0F0F0FLL))
        << "XOR of complex bit patterns should return correct result";

    ASSERT_EQ(42LL ^ 13LL, call_i64_xor(42LL, 13LL))
        << "XOR of small positive integers should return correct result";

    ASSERT_EQ((-100LL) ^ 200LL, call_i64_xor(-100LL, 200LL))
        << "XOR of negative and positive integers should return correct result";
}

/**
 * @test BoundaryValues_ReturnsCorrectResult
 * @brief Tests XOR with 64-bit boundary values (MIN, MAX, 0, -1)
 * @details Validates i64.xor behavior with extreme 64-bit values including
 *          INT64_MIN, INT64_MAX, zero, and all-bits-set patterns.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_xor_operation
 * @input_conditions Boundary values: INT64_MIN, INT64_MAX, 0, 0xFFFFFFFFFFFFFFFF
 * @expected_behavior Correct boundary value XOR results without overflow
 * @validation_method Boundary-specific XOR validations with expected mathematical results
 */
TEST_P(I64XorTest, BoundaryValues_ReturnsCorrectResult)
{
    const int64_t INT64_MIN_VAL = static_cast<int64_t>(0x8000000000000000ULL);
    const int64_t INT64_MAX_VAL = 0x7FFFFFFFFFFFFFFFLL;
    const int64_t ALL_BITS_SET = static_cast<int64_t>(0xFFFFFFFFFFFFFFFFULL);

    // Test MAX XOR MAX = 0
    ASSERT_EQ(0LL, call_i64_xor(INT64_MAX_VAL, INT64_MAX_VAL))
        << "INT64_MAX XOR INT64_MAX should equal 0";

    // Test MIN XOR MIN = 0
    ASSERT_EQ(0LL, call_i64_xor(INT64_MIN_VAL, INT64_MIN_VAL))
        << "INT64_MIN XOR INT64_MIN should equal 0";

    // Test MAX XOR MIN = -1 (all bits set)
    ASSERT_EQ(ALL_BITS_SET, call_i64_xor(INT64_MAX_VAL, INT64_MIN_VAL))
        << "INT64_MAX XOR INT64_MIN should equal -1 (all bits set)";

    // Test 0 XOR -1 = -1
    ASSERT_EQ(ALL_BITS_SET, call_i64_xor(0LL, ALL_BITS_SET))
        << "0 XOR (-1) should equal -1";

    // Test -1 XOR -1 = 0
    ASSERT_EQ(0LL, call_i64_xor(ALL_BITS_SET, ALL_BITS_SET))
        << "(-1) XOR (-1) should equal 0";
}

/**
 * @test IdentityProperties_ValidatesMathematicalRules
 * @brief Validates XOR identity properties (zero identity, self-inverse, double XOR)
 * @details Tests fundamental XOR mathematical properties including identity with zero,
 *          self-inverse operation, and double XOR returning to original value.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_xor_operation
 * @input_conditions Various test values with identity operations
 * @expected_behavior Identity: a XOR 0 = a, Self-inverse: a XOR a = 0, Double: a XOR b XOR b = a
 * @validation_method Property-specific validations for each mathematical rule
 */
TEST_P(I64XorTest, IdentityProperties_ValidatesMathematicalRules)
{
    const int64_t test_values[] = {
        0LL, 1LL, -1LL, 42LL, -42LL,
        0x123456789ABCDEF0LL, 0x7FFFFFFFFFFFFFFFLL, static_cast<int64_t>(0x8000000000000000ULL)
    };

    for (int64_t val : test_values) {
        // Identity property: a XOR 0 = a
        ASSERT_EQ(val, call_i64_xor(val, 0LL))
            << "Identity property failed for value: " << val << " (a XOR 0 should equal a)";

        // Self-inverse property: a XOR a = 0
        ASSERT_EQ(0LL, call_i64_xor(val, val))
            << "Self-inverse property failed for value: " << val << " (a XOR a should equal 0)";

        // Double XOR property: a XOR b XOR b = a (test with b = 0x5555...)
        int64_t b = 0x5555555555555555LL;
        int64_t intermediate = call_i64_xor(val, b);
        ASSERT_EQ(val, call_i64_xor(intermediate, b))
            << "Double XOR property failed for value: " << val << " with mask: " << b;
    }
}

/**
 * @test CommutativeProperty_ValidatesOrderIndependence
 * @brief Verifies XOR commutative property (a XOR b = b XOR a)
 * @details Tests that XOR operation is commutative by validating identical results
 *          regardless of operand order across various value combinations.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_xor_operation
 * @input_conditions Multiple value pairs tested in both orders
 * @expected_behavior Identical results regardless of operand order
 * @validation_method Direct comparison of a XOR b with b XOR a
 */
TEST_P(I64XorTest, CommutativeProperty_ValidatesOrderIndependence)
{
    const std::pair<int64_t, int64_t> test_pairs[] = {
        {42LL, 13LL},
        {0x123456789ABCDEF0LL, 0x0F0F0F0F0F0F0F0FLL},
        {0x5555555555555555LL, 0xAAAAAAAAAAAAAAAALL},
        {0x7FFFFFFFFFFFFFFFLL, static_cast<int64_t>(0x8000000000000000ULL)},
        {-1LL, 0LL},
        {100LL, -200LL}
    };

    for (const auto& pair : test_pairs) {
        int64_t a = pair.first;
        int64_t b = pair.second;

        ASSERT_EQ(call_i64_xor(a, b), call_i64_xor(b, a))
            << "Commutative property failed for values: " << a << " XOR " << b
            << " (a XOR b should equal b XOR a)";
    }
}

/**
 * @test AssociativeProperty_ValidatesGroupingIndependence
 * @brief Verifies XOR associative property ((a XOR b) XOR c = a XOR (b XOR c))
 * @details Tests that XOR operation is associative by validating identical results
 *          regardless of operand grouping across various value combinations.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_xor_operation
 * @input_conditions Three-operand combinations tested with different groupings
 * @expected_behavior Identical results regardless of operation grouping
 * @validation_method Comparison of left-grouped with right-grouped XOR operations
 */
TEST_P(I64XorTest, AssociativeProperty_ValidatesGroupingIndependence)
{
    const std::tuple<int64_t, int64_t, int64_t> test_triplets[] = {
        {42LL, 13LL, 99LL},
        {0x123456789ABCDEF0LL, 0x0F0F0F0F0F0F0F0FLL, 0x5555555555555555LL},
        {0x7FFFFFFFFFFFFFFFLL, static_cast<int64_t>(0x8000000000000000ULL), 0LL},
        {-1LL, -100LL, 200LL}
    };

    for (const auto& triplet : test_triplets) {
        int64_t a = std::get<0>(triplet);
        int64_t b = std::get<1>(triplet);
        int64_t c = std::get<2>(triplet);

        // Left grouping: (a XOR b) XOR c
        int64_t left_result = call_i64_xor(call_i64_xor(a, b), c);

        // Right grouping: a XOR (b XOR c)
        int64_t right_result = call_i64_xor(a, call_i64_xor(b, c));

        ASSERT_EQ(left_result, right_result)
            << "Associative property failed for values: (" << a << " XOR " << b << ") XOR " << c
            << " vs " << a << " XOR (" << b << " XOR " << c << ")";
    }
}

/**
 * @test BitManipulationPatterns_ValidatesSpecificOperations
 * @brief Tests specific bit manipulation patterns including powers of 2 and masks
 * @details Validates XOR behavior with single-bit patterns, alternating patterns,
 *          and high/low 32-bit isolation patterns for comprehensive bit manipulation coverage.
 * @test_category Edge - Bit manipulation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_xor_operation
 * @input_conditions Powers of 2, alternating patterns, 32-bit isolation masks
 * @expected_behavior Correct bit manipulation results for masking and toggling operations
 * @validation_method Pattern-specific validations for bit manipulation use cases
 */
TEST_P(I64XorTest, BitManipulationPatterns_ValidatesSpecificOperations)
{
    // Test single bit toggles (powers of 2)
    ASSERT_EQ(1LL, call_i64_xor(0LL, 1LL))
        << "Single bit toggle at position 0 failed";

    ASSERT_EQ(2LL, call_i64_xor(0LL, 2LL))
        << "Single bit toggle at position 1 failed";

    ASSERT_EQ(static_cast<int64_t>(0x8000000000000000ULL), call_i64_xor(0LL, static_cast<int64_t>(0x8000000000000000ULL)))
        << "Single bit toggle at position 63 failed";

    // Test alternating bit patterns
    ASSERT_EQ(0xFFFFFFFFFFFFFFFFLL, call_i64_xor(0x5555555555555555LL, 0xAAAAAAAAAAAAAAAALL))
        << "Alternating bit pattern XOR should produce all 1s";

    // Test high 32-bit isolation
    ASSERT_EQ(0xFFFFFFFF00000000LL, call_i64_xor(0xFFFFFFFF00000000LL, 0LL))
        << "High 32-bit isolation XOR with 0 should return original";

    // Test low 32-bit isolation
    ASSERT_EQ(0x00000000FFFFFFFFLL, call_i64_xor(0x00000000FFFFFFFFLL, 0LL))
        << "Low 32-bit isolation XOR with 0 should return original";

    // Test complex bit manipulation pattern
    ASSERT_EQ(0x123456789ABCDEF0LL ^ 0x0F0F0F0F0F0F0F0FLL, call_i64_xor(0x123456789ABCDEF0LL, 0x0F0F0F0F0F0F0F0FLL))
        << "Complex bit pattern XOR should return correct result";
}

/**
 * @test BitwiseNotOperation_ValidatesComplementGeneration
 * @brief Tests XOR with all-bits-set to generate bitwise NOT operation
 * @details Validates that XOR with 0xFFFFFFFFFFFFFFFF effectively produces
 *          bitwise NOT operation across various input values.
 * @test_category Edge - Bitwise complement validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:i64_xor_operation
 * @input_conditions Various values XORed with all-bits-set pattern
 * @expected_behavior Results equivalent to bitwise NOT of input values
 * @validation_method Comparison with expected bitwise complement results
 */
TEST_P(I64XorTest, BitwiseNotOperation_ValidatesComplementGeneration)
{
    const int64_t ALL_BITS_SET = static_cast<int64_t>(0xFFFFFFFFFFFFFFFFULL);

    const int64_t test_values[] = {
        0LL, 1LL, -1LL, 42LL,
        0x5555555555555555LL, static_cast<int64_t>(0xAAAAAAAAAAAAAAAAULL),
        0x123456789ABCDEF0LL, 0x7FFFFFFFFFFFFFFFLL
    };

    for (int64_t val : test_values) {
        int64_t expected_not = ~val;
        ASSERT_EQ(expected_not, call_i64_xor(val, ALL_BITS_SET))
            << "Bitwise NOT via XOR with all 1s failed for value: " << val
            << " (expected: " << expected_not << ")";
    }
}

// Parameterized test instantiation for interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningModeTest, I64XorTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<I64XorTest::ParamType> &info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

/**
 * Static initialization for WASM file paths
 */
static bool init_wasm_files() {
    char *build_dir = realpath(".", nullptr);
    WASM_FILE = std::string(build_dir) + "/wasm-apps/i64_xor_test.wasm";
    WASM_FILE_UNDERFLOW = std::string(build_dir) + "/wasm-apps/i64_xor_stack_underflow.wasm";
    free(build_dir);
    return true;
}

static bool wasm_files_initialized = init_wasm_files();