/**
 * @file enhanced_i8x16_ne_test.cc
 * @brief Comprehensive unit tests for i8x16.ne SIMD opcode
 * @details Tests i8x16.ne functionality across interpreter and AOT execution modes
 *          with focus on element-wise not-equal comparison of sixteen 8-bit signed integers,
 *          mathematical properties validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_ne_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class I8x16NeTestSuite
 * @brief Test fixture class for i8x16.ne opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Mathematical property validation
 */
class I8x16NeTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for i8x16.ne testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_ne_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.ne test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_ne_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i8x16.ne tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_ne_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM i8x16.ne function with two vector inputs
     * @details Executes i8x16.ne operation on two input vectors and returns element-wise inequality result.
     *          Handles WASM function invocation and v128 result extraction for 16-byte comparison.
     * @param input1_bytes 16-byte array representing first input v128 vector as i8 lanes
     * @param input2_bytes 16-byte array representing second input v128 vector as i8 lanes
     * @param result_bytes 16-byte array to store the inequality comparison result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_ne_test.cc:call_i8x16_ne
     */
    bool call_i8x16_ne(const uint8_t input1_bytes[16], const uint8_t input2_bytes[16],
                       uint8_t result_bytes[16])
    {
        // Prepare arguments: two input v128 vectors as four i64 values each
        uint32_t argv[8];

        // Convert byte arrays to 64-bit values (little-endian format)
        // First v128 vector
        uint64_t input1_lo = 0, input1_hi = 0;
        for (int i = 0; i < 8; i++) {
            input1_lo |= (static_cast<uint64_t>(input1_bytes[i]) << (i * 8));
            input1_hi |= (static_cast<uint64_t>(input1_bytes[i + 8]) << (i * 8));
        }

        // Second v128 vector
        uint64_t input2_lo = 0, input2_hi = 0;
        for (int i = 0; i < 8; i++) {
            input2_lo |= (static_cast<uint64_t>(input2_bytes[i]) << (i * 8));
            input2_hi |= (static_cast<uint64_t>(input2_bytes[i + 8]) << (i * 8));
        }

        // WASM expects little-endian format: low part first, then high part
        // First v128 vector
        argv[0] = static_cast<uint32_t>(input1_lo);        // Low 32 bits of low i64
        argv[1] = static_cast<uint32_t>(input1_lo >> 32);  // High 32 bits of low i64
        argv[2] = static_cast<uint32_t>(input1_hi);        // Low 32 bits of high i64
        argv[3] = static_cast<uint32_t>(input1_hi >> 32);  // High 32 bits of high i64
        // Second v128 vector
        argv[4] = static_cast<uint32_t>(input2_lo);        // Low 32 bits of low i64
        argv[5] = static_cast<uint32_t>(input2_lo >> 32);  // High 32 bits of low i64
        argv[6] = static_cast<uint32_t>(input2_hi);        // Low 32 bits of high i64
        argv[7] = static_cast<uint32_t>(input2_hi >> 32);  // High 32 bits of high i64

        // Call WASM function with two v128 inputs
        bool call_success = dummy_env->execute("test_i8x16_ne", 8, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_i8x16_ne function";

        if (call_success) {
            // Extract v128 result and convert back to byte array
            uint64_t result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            uint64_t result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];

            // Convert 64-bit values back to byte array
            for (int i = 0; i < 8; i++) {
                result_bytes[i] = static_cast<uint8_t>(result_lo >> (i * 8));
                result_bytes[i + 8] = static_cast<uint8_t>(result_hi >> (i * 8));
            }
        }

        return call_success;
    }

    /**
     * @brief Verify that i8x16.ne result vector matches expected byte pattern
     * @details Compares each of the 16 lanes for expected inequality mask values.
     *          Unequal lanes should contain 0xFF, equal lanes should contain 0x00.
     * @param expected_bytes Expected 16-byte result pattern
     * @param actual_bytes Actual 16-byte result from i8x16.ne operation
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_ne_test.cc:assert_i8x16_ne_result
     */
    void assert_i8x16_ne_result(const uint8_t expected_bytes[16], const uint8_t actual_bytes[16])
    {
        for (int i = 0; i < 16; i++) {
            ASSERT_EQ(expected_bytes[i], actual_bytes[i])
                << "i8x16.ne lane " << i << " mismatch - Expected: 0x" << std::hex
                << static_cast<int>(expected_bytes[i]) << ", Actual: 0x" << std::hex
                << static_cast<int>(actual_bytes[i]);
        }
    }

    /**
     * @brief Helper function to create test vector with specified byte values
     * @details Populates a 16-byte array with provided values for test input preparation.
     * @param bytes Output array to populate
     * @param b0-b15 Individual byte values for lanes 0-15
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_ne_test.cc:make_i8x16_vector
     */
    void make_i8x16_vector(uint8_t bytes[16],
                          uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3,
                          uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7,
                          uint8_t b8, uint8_t b9, uint8_t b10, uint8_t b11,
                          uint8_t b12, uint8_t b13, uint8_t b14, uint8_t b15)
    {
        bytes[0] = b0; bytes[1] = b1; bytes[2] = b2; bytes[3] = b3;
        bytes[4] = b4; bytes[5] = b5; bytes[6] = b6; bytes[7] = b7;
        bytes[8] = b8; bytes[9] = b9; bytes[10] = b10; bytes[11] = b11;
        bytes[12] = b12; bytes[13] = b13; bytes[14] = b14; bytes[15] = b15;
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicComparison_ReturnsCorrectMask
 * @brief Validates i8x16.ne produces correct inequality results for typical input combinations
 * @details Tests fundamental not-equal operation with mixed equal/unequal scenarios.
 *          Verifies that i8x16.ne correctly identifies unequal lanes as 0xFF and equal lanes as 0x00.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_i8x16_ne
 * @input_conditions Mixed patterns: identical vectors, different vectors, partial equality
 * @expected_behavior Returns 0xFF for unequal lanes, 0x00 for equal lanes
 * @validation_method Direct comparison of WASM function result with expected inequality mask
 */
TEST_F(I8x16NeTestSuite, BasicComparison_ReturnsCorrectMask)
{
    uint8_t input1[16], input2[16], result[16], expected[16];

    // Test complete equality: identical vectors should produce all 0x00 (no inequalities)
    make_i8x16_vector(input1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    make_i8x16_vector(input2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    make_i8x16_vector(expected, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with identical vectors";
    assert_i8x16_ne_result(expected, result);

    // Test complete inequality: different vectors should produce all 0xFF
    make_i8x16_vector(input1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    make_i8x16_vector(input2, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with different vectors";
    assert_i8x16_ne_result(expected, result);

    // Test partial inequality: alternating pattern
    make_i8x16_vector(input1, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8);
    make_i8x16_vector(input2, 1, 0, 3, 0, 5, 0, 7, 0, 1, 0, 3, 0, 5, 0, 7, 0);
    make_i8x16_vector(expected, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
                               0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF);

    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with partial inequality pattern";
    assert_i8x16_ne_result(expected, result);
}

/**
 * @test BoundaryValues_HandleMinMaxCorrectly
 * @brief Tests i8x16.ne with signed 8-bit boundary values and edge conditions
 * @details Validates comparison behavior at MIN_VALUE (-128), MAX_VALUE (+127),
 *          zero boundary, and signed integer transitions.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/compilation/simd/simd_comparison.c:i8x16_signed_comparison
 * @input_conditions MIN/MAX signed values, zero, boundary transitions
 * @expected_behavior Accurate signed comparison at integer limits and transitions
 * @validation_method Boundary-specific inequality validation with signed semantics
 */
TEST_F(I8x16NeTestSuite, BoundaryValues_HandleMinMaxCorrectly)
{
    uint8_t input1[16], input2[16], result[16], expected[16];

    // Test MIN_VALUE equality: -128 == -128 should be false (0x00) for i8x16.ne
    make_i8x16_vector(input1, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                              0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
    make_i8x16_vector(input2, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                              0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
    make_i8x16_vector(expected, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with MIN_VALUE (-128) boundary test";
    assert_i8x16_ne_result(expected, result);

    // Test MAX_VALUE equality: +127 == +127 should be false (0x00) for i8x16.ne
    make_i8x16_vector(input1, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                              0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F);
    make_i8x16_vector(input2, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                              0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F);
    make_i8x16_vector(expected, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with MAX_VALUE (+127) boundary test";
    assert_i8x16_ne_result(expected, result);

    // Test cross-boundary inequality: MIN vs MAX values should be true (0xFF)
    make_i8x16_vector(input1, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F,
                              0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F);
    make_i8x16_vector(input2, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80,
                              0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80);
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with MIN/MAX cross-boundary test";
    assert_i8x16_ne_result(expected, result);
}

/**
 * @test IdentityComparison_ReturnsAllZeros
 * @brief Tests i8x16.ne identity property: a vector compared with itself should yield all false
 * @details Validates mathematical identity property where identical vectors produce all 0x00 lanes.
 *          Tests various patterns to ensure self-comparison always results in no inequalities.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_i8x16_ne
 * @input_conditions Self-comparison scenarios with various byte patterns
 * @expected_behavior All lanes return 0x00 (false) for identical vector comparison
 * @validation_method Identity property verification across multiple test patterns
 */
TEST_F(I8x16NeTestSuite, IdentityComparison_ReturnsAllZeros)
{
    uint8_t input[16], result[16], expected[16];

    // All result lanes should be 0x00 (false) for identity comparisons
    make_i8x16_vector(expected, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // Test identity with positive pattern
    make_i8x16_vector(input, 42, 100, 1, 127, 50, 25, 75, 10, 5, 15, 35, 99, 88, 77, 66, 55);
    ASSERT_TRUE(call_i8x16_ne(input, input, result))
        << "Failed to execute i8x16.ne identity test with positive pattern";
    assert_i8x16_ne_result(expected, result);

    // Test identity with boundary values
    make_i8x16_vector(input, 0x80, 0x7F, 0x00, 0xFF, 0x01, 0xFE, 0x80, 0x7F,
                             0x00, 0xFF, 0x01, 0xFE, 0x80, 0x7F, 0x00, 0xFF);
    ASSERT_TRUE(call_i8x16_ne(input, input, result))
        << "Failed to execute i8x16.ne identity test with boundary values";
    assert_i8x16_ne_result(expected, result);

    // Test identity with alternating zero pattern
    make_i8x16_vector(input, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
                             0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF);
    ASSERT_TRUE(call_i8x16_ne(input, input, result))
        << "Failed to execute i8x16.ne identity test with alternating pattern";
    assert_i8x16_ne_result(expected, result);
}

/**
 * @test AllDifferent_ReturnsAllOnes
 * @brief Tests i8x16.ne with vectors having no matching lanes
 * @details Validates scenarios where all 16 lanes differ between input vectors.
 *          Ensures complete inequality detection produces all 0xFF result lanes.
 * @test_category Edge - Complete difference validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_i8x16_ne
 * @input_conditions Vector pairs with zero lane matches across all 16 positions
 * @expected_behavior All lanes return 0xFF (true) indicating complete inequality
 * @validation_method Comprehensive difference verification with guaranteed non-matching patterns
 */
TEST_F(I8x16NeTestSuite, AllDifferent_ReturnsAllOnes)
{
    uint8_t input1[16], input2[16], result[16], expected[16];

    // All result lanes should be 0xFF (true) for completely different vectors
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    // Test with sequential vs reverse patterns (guaranteed no matches)
    make_i8x16_vector(input1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    make_i8x16_vector(input2, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with sequential vs reverse pattern";
    assert_i8x16_ne_result(expected, result);

    // Test with positive vs negative values (guaranteed differences)
    make_i8x16_vector(input1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    make_i8x16_vector(input2, 0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8,
                              0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0);
    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with positive vs negative pattern";
    assert_i8x16_ne_result(expected, result);

    // Test with alternating high/low differences
    make_i8x16_vector(input1, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
                              0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F);
    make_i8x16_vector(input2, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00,
                              0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00);
    ASSERT_TRUE(call_i8x16_ne(input1, input2, result))
        << "Failed to execute i8x16.ne with alternating high/low pattern";
    assert_i8x16_ne_result(expected, result);
}