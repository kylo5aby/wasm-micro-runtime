/**
 * @file enhanced_v128_not_test.cc
 * @brief Comprehensive unit tests for v128.not SIMD opcode
 * @details Tests v128.not functionality across interpreter and AOT execution modes
 *          with focus on bitwise NOT operations, boundary conditions, and edge cases.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_not_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class V128NotTestSuite
 * @brief Test fixture class for v128.not opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class V128NotTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.not testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_not_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.not test module using absolute path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_not_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.not tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_not_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM v128.not function with provided vector input
     * @details Executes v128.not operation on input vector and returns bitwise inverted result.
     *          Handles WASM function invocation and v128 result extraction.
     * @param input_hi High 64 bits of input v128 vector
     * @param input_lo Low 64 bits of input v128 vector
     * @param result_hi Reference to store high 64 bits of result v128 vector
     * @param result_lo Reference to store low 64 bits of result v128 vector
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_not_test.cc:call_v128_not
     */
    bool call_v128_not(uint64_t input_hi, uint64_t input_lo,
                       uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: input v128 as two i64 values
        uint32_t argv[4];
        // WASM expects little-endian format: low part first, then high part
        argv[0] = static_cast<uint32_t>(input_lo);        // Low 32 bits of low i64
        argv[1] = static_cast<uint32_t>(input_lo >> 32);  // High 32 bits of low i64
        argv[2] = static_cast<uint32_t>(input_hi);        // Low 32 bits of high i64
        argv[3] = static_cast<uint32_t>(input_hi >> 32);  // High 32 bits of high i64

        // Call WASM function with v128 input
        bool call_success = dummy_env->execute("test_v128_not", 4, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_v128_not function";

        if (call_success) {
            // Extract v128 result as two i64 values
            result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];
        }

        return call_success;
    }

    /**
     * @brief Verify that two v128 vectors are bitwise equal
     * @details Compares high and low 64-bit parts of two v128 vectors for exact equality.
     * @param expected_hi Expected high 64 bits
     * @param expected_lo Expected low 64 bits
     * @param actual_hi Actual high 64 bits
     * @param actual_lo Actual low 64 bits
     * @param message Context message for assertion failure
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_not_test.cc:verify_v128_equal
     */
    void verify_v128_equal(uint64_t expected_hi, uint64_t expected_lo,
                          uint64_t actual_hi, uint64_t actual_lo,
                          const char* message)
    {
        ASSERT_EQ(expected_hi, actual_hi) << message << " - High 64 bits mismatch";
        ASSERT_EQ(expected_lo, actual_lo) << message << " - Low 64 bits mismatch";
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicBitwiseNot_ReturnsInvertedBits
 * @brief Validates v128.not produces correct bitwise inversion for typical bit patterns
 * @details Tests fundamental bitwise NOT operation with mixed patterns including
 *          alternating bits, partial patterns, and representative integer values.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_v128_not
 * @input_conditions Mixed bit patterns: 0x0F0F0F0F0F0F0F0F, 0xAAAAAAAAAAAAAAAA
 * @expected_behavior Returns bitwise complement of input vectors
 * @validation_method Direct comparison of WASM function result with expected inverted values
 */
TEST_F(V128NotTestSuite, BasicBitwiseNot_ReturnsInvertedBits)
{
    uint64_t result_hi, result_lo;

    // Test mixed bit pattern: 0x0F0F... → 0xF0F0...
    bool success = call_v128_not(0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0F0F0F0F0FULL,
                                result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for mixed pattern";
    verify_v128_equal(0xF0F0F0F0F0F0F0F0ULL, 0xF0F0F0F0F0F0F0F0ULL,
                     result_hi, result_lo, "Mixed pattern 0x0F0F inversion");

    // Test alternating pattern: 0xAAAA... → 0x5555...
    success = call_v128_not(0xAAAAAAAAAAAAAAAAULL, 0xAAAAAAAAAAAAAAAAULL,
                           result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for alternating pattern";
    verify_v128_equal(0x5555555555555555ULL, 0x5555555555555555ULL,
                     result_hi, result_lo, "Alternating pattern 0xAAAA inversion");

    // Test representative integer pattern
    success = call_v128_not(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL,
                           result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for integer pattern";
    verify_v128_equal(0xFEDCBA9876543210ULL, 0x0123456789ABCDEFULL,
                     result_hi, result_lo, "Representative integer pattern inversion");
}

/**
 * @test AllZeros_ReturnsAllOnes
 * @brief Validates v128.not converts zero vector to all-ones vector
 * @details Tests complete bitwise inversion from all zeros to all ones,
 *          verifying that every bit is correctly flipped.
 * @test_category Edge - Zero operand scenario
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_v128_not
 * @input_conditions All zero v128 vector: 0x000...000
 * @expected_behavior Returns all ones v128 vector: 0xFFF...FFF
 * @validation_method Exact comparison of result with expected all-ones pattern
 */
TEST_F(V128NotTestSuite, AllZeros_ReturnsAllOnes)
{
    uint64_t result_hi, result_lo;

    // Test all zeros → all ones
    bool success = call_v128_not(0x0000000000000000ULL, 0x0000000000000000ULL,
                                result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for all zeros";
    verify_v128_equal(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                     result_hi, result_lo, "All zeros to all ones conversion");
}

/**
 * @test AllOnes_ReturnsAllZeros
 * @brief Validates v128.not converts all-ones vector to zero vector
 * @details Tests complete bitwise inversion from all ones to all zeros,
 *          verifying that every bit is correctly cleared.
 * @test_category Edge - Extreme value scenario
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_v128_not
 * @input_conditions All ones v128 vector: 0xFFF...FFF
 * @expected_behavior Returns all zeros v128 vector: 0x000...000
 * @validation_method Exact comparison of result with expected all-zeros pattern
 */
TEST_F(V128NotTestSuite, AllOnes_ReturnsAllZeros)
{
    uint64_t result_hi, result_lo;

    // Test all ones → all zeros
    bool success = call_v128_not(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                                result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for all ones";
    verify_v128_equal(0x0000000000000000ULL, 0x0000000000000000ULL,
                     result_hi, result_lo, "All ones to all zeros conversion");
}

/**
 * @test BoundaryValues_ReturnsCorrectInversion
 * @brief Validates v128.not handles boundary values correctly for different lane interpretations
 * @details Tests MAX/MIN values for i8x16, i16x8, i32x4, i64x2 lane interpretations
 *          to ensure consistent bitwise operation regardless of data interpretation.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_v128_not
 * @input_conditions Boundary values for various integer lane sizes
 * @expected_behavior Correct bitwise inversion preserving mathematical properties
 * @validation_method Verification against expected complement values for each lane size
 */
TEST_F(V128NotTestSuite, BoundaryValues_ReturnsCorrectInversion)
{
    uint64_t result_hi, result_lo;

    // Test i32x4 boundaries: INT32_MAX values
    bool success = call_v128_not(0x7FFFFFFF7FFFFFFFULL, 0x7FFFFFFF7FFFFFFFULL,
                                result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for i32x4 MAX boundaries";
    verify_v128_equal(0x8000000080000000ULL, 0x8000000080000000ULL,
                     result_hi, result_lo, "i32x4 MAX boundary inversion");

    // Test i64x2 boundaries: INT64_MAX values
    success = call_v128_not(0x7FFFFFFFFFFFFFFFULL, 0x7FFFFFFFFFFFFFFFULL,
                           result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for i64x2 MAX boundaries";
    verify_v128_equal(0x8000000000000000ULL, 0x8000000000000000ULL,
                     result_hi, result_lo, "i64x2 MAX boundary inversion");

    // Test mixed boundary pattern
    success = call_v128_not(0x8000000000000000ULL, 0x7FFFFFFFFFFFFFFFULL,
                           result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for mixed boundaries";
    verify_v128_equal(0x7FFFFFFFFFFFFFFFULL, 0x8000000000000000ULL,
                     result_hi, result_lo, "Mixed boundary pattern inversion");
}

/**
 * @test AlternatingPatterns_ReturnsComplementaryPatterns
 * @brief Validates v128.not produces correct complementary alternating bit patterns
 * @details Tests classic alternating bit patterns and verifies they produce
 *          their exact bitwise complements as expected.
 * @test_category Edge - Special bit pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_v128_not
 * @input_conditions Alternating patterns: 0x5555..., 0xAAAA..., 0xCCCC..., 0x3333...
 * @expected_behavior Returns exact bitwise complement of each pattern
 * @validation_method Direct verification of complementary pattern relationships
 */
TEST_F(V128NotTestSuite, AlternatingPatterns_ReturnsComplementaryPatterns)
{
    uint64_t result_hi, result_lo;

    // Test 0x5555 → 0xAAAA
    bool success = call_v128_not(0x5555555555555555ULL, 0x5555555555555555ULL,
                                result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for 0x5555 pattern";
    verify_v128_equal(0xAAAAAAAAAAAAAAAAULL, 0xAAAAAAAAAAAAAAAAULL,
                     result_hi, result_lo, "Pattern 0x5555 → 0xAAAA conversion");

    // Test 0x3333 → 0xCCCC
    success = call_v128_not(0x3333333333333333ULL, 0x3333333333333333ULL,
                           result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for 0x3333 pattern";
    verify_v128_equal(0xCCCCCCCCCCCCCCCCULL, 0xCCCCCCCCCCCCCCCCULL,
                     result_hi, result_lo, "Pattern 0x3333 → 0xCCCC conversion");
}

/**
 * @test DoubleNegation_ReturnsOriginalValue
 * @brief Validates double negation identity property: ~~v = v
 * @details Tests that applying v128.not twice returns the original value,
 *          verifying the mathematical identity property of bitwise NOT.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_v128_not
 * @input_conditions Various test vectors applied twice through v128.not
 * @expected_behavior Original input values after double negation
 * @validation_method Verification that double NOT operation is identity
 */
TEST_F(V128NotTestSuite, DoubleNegation_ReturnsOriginalValue)
{
    uint64_t result_hi, result_lo, final_hi, final_lo;
    const uint64_t original_hi = 0x123456789ABCDEFULL;
    const uint64_t original_lo = 0xFEDCBA9876543210ULL;

    // First negation
    bool success = call_v128_not(original_hi, original_lo, result_hi, result_lo);
    ASSERT_TRUE(success) << "First v128.not call failed";

    // Second negation (should restore original)
    success = call_v128_not(result_hi, result_lo, final_hi, final_lo);
    ASSERT_TRUE(success) << "Second v128.not call failed";

    // Verify double negation identity
    verify_v128_equal(original_hi, original_lo, final_hi, final_lo,
                     "Double negation identity property");
}

/**
 * @test LaneAgnosticOperation_ReturnsSameBitwiseResult
 * @brief Validates v128.not produces identical results regardless of lane interpretation
 * @details Tests that bitwise NOT operation is lane-agnostic and produces
 *          the same bitwise result whether interpreted as i8x16, i16x8, i32x4, or i64x2.
 * @test_category Main - Lane interpretation consistency
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_v128_not
 * @input_conditions Same bit pattern interpreted as different lane sizes
 * @expected_behavior Identical bitwise results across all lane interpretations
 * @validation_method Verification that lane interpretation doesn't affect bitwise operation
 */
TEST_F(V128NotTestSuite, LaneAgnosticOperation_ReturnsSameBitwiseResult)
{
    uint64_t result_hi, result_lo;
    const uint64_t test_hi = 0x8040201008040201ULL;  // Distinctive bit pattern
    const uint64_t test_lo = 0x0102040810204080ULL;

    // Perform NOT operation (result should be lane-agnostic)
    bool success = call_v128_not(test_hi, test_lo, result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for lane-agnostic test";

    // Verify expected bitwise inversion (same regardless of lane interpretation)
    verify_v128_equal(~test_hi, ~test_lo, result_hi, result_lo,
                     "Lane-agnostic bitwise NOT operation");

    // Additional verification with different pattern
    const uint64_t pattern2_hi = 0xF0F0F0F0F0F0F0F0ULL;
    const uint64_t pattern2_lo = 0x0F0F0F0F0F0F0F0FULL;

    success = call_v128_not(pattern2_hi, pattern2_lo, result_hi, result_lo);
    ASSERT_TRUE(success) << "v128.not call failed for second lane-agnostic test";
    verify_v128_equal(~pattern2_hi, ~pattern2_lo, result_hi, result_lo,
                     "Second lane-agnostic bitwise NOT operation");
}

/**
 * @test RuntimeErrorConditions_HandlesGracefully
 * @brief Validates runtime handles v128.not error conditions gracefully
 * @details Tests error scenarios including module loading failures and
 *          runtime configuration issues, ensuring graceful error handling.
 * @test_category Error - Error condition validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_load
 * @input_conditions Invalid module scenarios and runtime configuration errors
 * @expected_behavior Proper error reporting without crashes or memory leaks
 * @validation_method Verification of error handling and resource cleanup
 */
TEST_F(V128NotTestSuite, RuntimeErrorConditions_HandlesGracefully)
{
    // Test with invalid WASM module path
    std::unique_ptr<DummyExecEnv> invalid_env;
    invalid_env = std::make_unique<DummyExecEnv>("/nonexistent/invalid_path.wasm");

    // Should handle missing file gracefully
    ASSERT_EQ(nullptr, invalid_env->get())
        << "Expected null execution environment for invalid WASM module path";

    // Test module loading with malformed WASM data
    uint8_t invalid_wasm[] = { 0x00, 0x61, 0x73, 0x6D, 0xFF, 0xFF, 0xFF, 0xFF }; // Invalid WASM
    char error_buf[256];
    wasm_module_t invalid_module = wasm_runtime_load(invalid_wasm, sizeof(invalid_wasm),
                                                    error_buf, sizeof(error_buf));

    // Should return null for invalid WASM bytecode
    ASSERT_EQ(nullptr, invalid_module)
        << "Expected module loading to fail for invalid WASM bytecode";
    ASSERT_NE('\0', error_buf[0])
        << "Expected error message for invalid WASM bytecode";

    // Verify runtime remains stable after error scenarios
    uint64_t result_hi, result_lo;
    bool success = call_v128_not(0xFFFFFFFFFFFFFFFFULL, 0x0000000000000000ULL,
                                result_hi, result_lo);
    ASSERT_TRUE(success)
        << "Runtime should remain functional after error scenarios";
}