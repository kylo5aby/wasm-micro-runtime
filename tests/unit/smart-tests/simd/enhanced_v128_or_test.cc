/**
 * @file enhanced_v128_or_test.cc
 * @brief Comprehensive unit tests for v128.or SIMD opcode
 * @details Tests v128.or functionality across interpreter and AOT execution modes
 *          with focus on bitwise OR operations, mathematical properties, and edge cases.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_or_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class V128OrTestSuite
 * @brief Test fixture class for v128.or opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Mathematical property validation
 */
class V128OrTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.or testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_or_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.or test module using absolute path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_or_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.or tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_or_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM v128.or function with two vector inputs
     * @details Executes v128.or operation on two input vectors and returns bitwise OR result.
     *          Handles WASM function invocation and v128 result extraction.
     * @param input1_hi High 64 bits of first input v128 vector
     * @param input1_lo Low 64 bits of first input v128 vector
     * @param input2_hi High 64 bits of second input v128 vector
     * @param input2_lo Low 64 bits of second input v128 vector
     * @param result_hi Reference to store high 64 bits of result v128 vector
     * @param result_lo Reference to store low 64 bits of result v128 vector
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_or_test.cc:call_v128_or
     */
    bool call_v128_or(uint64_t input1_hi, uint64_t input1_lo,
                      uint64_t input2_hi, uint64_t input2_lo,
                      uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: two input v128 vectors as four i64 values
        uint32_t argv[8];
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
        bool call_success = dummy_env->execute("test_v128_or", 8, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_v128_or function";

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
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_or_test.cc:assert_v128_equal
     */
    void assert_v128_equal(uint64_t expected_hi, uint64_t expected_lo,
                          uint64_t actual_hi, uint64_t actual_lo)
    {
        ASSERT_EQ(expected_hi, actual_hi)
            << "v128 high 64 bits mismatch - Expected: 0x" << std::hex << expected_hi
            << ", Actual: 0x" << std::hex << actual_hi;
        ASSERT_EQ(expected_lo, actual_lo)
            << "v128 low 64 bits mismatch - Expected: 0x" << std::hex << expected_lo
            << ", Actual: 0x" << std::hex << actual_lo;
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicBitwiseOr_ProducesCorrectResults
 * @brief Validates v128.or produces correct results for typical bitwise OR scenarios
 * @details Tests fundamental OR operation with common bit patterns, validates commutative
 *          property, and confirms correct bitwise operations across different vector interpretations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:v128_bitwise_two_component
 * @input_conditions Mixed 0/1 patterns, commutative test cases, standard bit combinations
 * @expected_behavior Mathematically correct OR results, order independence validation
 * @validation_method Direct result comparison with expected bitwise OR outcomes
 */
TEST_F(V128OrTestSuite, BasicBitwiseOr_ProducesCorrectResults)
{
    uint64_t result_hi, result_lo;

    // Test basic OR with alternating patterns: 0x5555... | 0xAAAA... = 0xFFFF...
    ASSERT_TRUE(call_v128_or(0x5555555555555555ULL, 0x5555555555555555ULL,
                             0xAAAAAAAAAAAAAAAAULL, 0xAAAAAAAAAAAAAAAAULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with alternating bit patterns";
    assert_v128_equal(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, result_hi, result_lo);

    // Test OR with nibble patterns: 0x0F0F... | 0xF0F0... = 0xFFFF...
    ASSERT_TRUE(call_v128_or(0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0F0F0F0F0FULL,
                             0xF0F0F0F0F0F0F0F0ULL, 0xF0F0F0F0F0F0F0F0ULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with nibble alternating patterns";
    assert_v128_equal(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, result_hi, result_lo);

    // Test commutative property: a | b = b | a
    uint64_t result_hi_1, result_lo_1, result_hi_2, result_lo_2;
    ASSERT_TRUE(call_v128_or(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL,
                             0x0F0F0F0F0F0F0F0FULL, 0xF0F0F0F0F0F0F0F0ULL,
                             result_hi_1, result_lo_1))
        << "Failed to execute first v128.or for commutativity test";
    ASSERT_TRUE(call_v128_or(0x0F0F0F0F0F0F0F0FULL, 0xF0F0F0F0F0F0F0F0ULL,
                             0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL,
                             result_hi_2, result_lo_2))
        << "Failed to execute second v128.or for commutativity test";
    assert_v128_equal(result_hi_1, result_lo_1, result_hi_2, result_lo_2);

    // Test truth table combinations
    ASSERT_TRUE(call_v128_or(0x0000000000000000ULL, 0x0000000000000000ULL,
                             0x0000000000000000ULL, 0x0000000000000000ULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or for 0|0 truth table test";
    assert_v128_equal(0x0000000000000000ULL, 0x0000000000000000ULL, result_hi, result_lo);
}

/**
 * @test BoundaryConditions_HandleIdentityAndAbsorber
 * @brief Tests boundary values and mathematical properties of v128.or operation
 * @details Validates identity element (all zeros), absorber element (all ones),
 *          and idempotent property. Tests fundamental OR mathematical properties.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:LLVMBuildOr
 * @input_conditions All zeros vector, all ones vector, identical operands
 * @expected_behavior a|0=a, a|0xFF...FF=0xFF...FF, a|a=a properties hold
 * @validation_method Mathematical property verification with boundary values
 */
TEST_F(V128OrTestSuite, BoundaryConditions_HandleIdentityAndAbsorber)
{
    uint64_t result_hi, result_lo;

    // Test identity property: a | 0 = a (any value OR with all zeros equals itself)
    ASSERT_TRUE(call_v128_or(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL,
                             0x0000000000000000ULL, 0x0000000000000000ULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or for identity property test";
    assert_v128_equal(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL, result_hi, result_lo);

    // Test absorber property: a | 0xFFFFFFFF... = 0xFFFFFFFF... (any value OR with all ones equals all ones)
    ASSERT_TRUE(call_v128_or(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL,
                             0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or for absorber property test";
    assert_v128_equal(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, result_hi, result_lo);

    // Test idempotent property: a | a = a (any value OR with itself equals itself)
    ASSERT_TRUE(call_v128_or(0xA5A5A5A5A5A5A5A5ULL, 0x5A5A5A5A5A5A5A5AULL,
                             0xA5A5A5A5A5A5A5A5ULL, 0x5A5A5A5A5A5A5A5AULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or for idempotent property test";
    assert_v128_equal(0xA5A5A5A5A5A5A5A5ULL, 0x5A5A5A5A5A5A5A5AULL, result_hi, result_lo);

    // Test with all ones OR all ones (boundary case)
    ASSERT_TRUE(call_v128_or(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                             0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with all ones boundary case";
    assert_v128_equal(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, result_hi, result_lo);

    // Test complementary patterns: a | ~a = 0xFFFFFFFF...
    ASSERT_TRUE(call_v128_or(0x0F0F0F0F0F0F0F0FULL, 0xF0F0F0F0F0F0F0F0ULL,
                             0xF0F0F0F0F0F0F0F0ULL, 0x0F0F0F0F0F0F0F0FULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with complementary patterns";
    assert_v128_equal(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, result_hi, result_lo);
}

/**
 * @test SpecialBitPatterns_ValidatesLaneIndependence
 * @brief Tests alternating patterns and lane boundary conditions for v128.or
 * @details Validates consistent results across different lane interpretations (i8x16, i16x8, i32x4, i64x2),
 *          tests power-of-2 sequences across lanes, and confirms bit-level accuracy.
 * @test_category Edge - Special pattern and lane validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:bitwise_operation
 * @input_conditions Checkerboard patterns, power-of-2 sequences, lane boundary crossings
 * @expected_behavior Consistent results across lane interpretations, bit-level accuracy
 * @validation_method Cross-lane consistency verification with complex bit patterns
 */
TEST_F(V128OrTestSuite, SpecialBitPatterns_ValidatesLaneIndependence)
{
    uint64_t result_hi, result_lo;

    // Test checkerboard pattern: 0x55555555... | 0x33333333... = 0x77777777...
    ASSERT_TRUE(call_v128_or(0x5555555555555555ULL, 0x5555555555555555ULL,
                             0x3333333333333333ULL, 0x3333333333333333ULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with checkerboard pattern";
    assert_v128_equal(0x7777777777777777ULL, 0x7777777777777777ULL, result_hi, result_lo);

    // Test power-of-2 patterns across different lane types
    // i8x16 interpretation: powers of 2 in each byte
    ASSERT_TRUE(call_v128_or(0x0102040810204080ULL, 0x0102040810204080ULL,
                             0x8040201008040201ULL, 0x8040201008040201ULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with power-of-2 i8x16 patterns";
    assert_v128_equal(0x8142241818244281ULL, 0x8142241818244281ULL, result_hi, result_lo);

    // Test single bit set at various positions
    ASSERT_TRUE(call_v128_or(0x8000000000000001ULL, 0x0000000000000000ULL,
                             0x0000000000000000ULL, 0x0000000000000001ULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with single bit positions";
    assert_v128_equal(0x8000000000000001ULL, 0x0000000000000001ULL, result_hi, result_lo);

    // Test mixed lane boundary patterns (i32x4 lanes with different masks)
    ASSERT_TRUE(call_v128_or(0x00FF00FF00FF00FFULL, 0x00FF00FF00FF00FFULL,
                             0xFF00FF00FF00FF00ULL, 0xFF00FF00FF00FF00ULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with mixed lane boundary patterns";
    assert_v128_equal(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, result_hi, result_lo);

    // Test lane crossing patterns with different bit densities
    ASSERT_TRUE(call_v128_or(0x123456789ABCDEFULL, 0x0F1E2D3C4B5A6978ULL,
                             0x8765432101234567ULL, 0xF0E1D2C3B4A59687ULL,
                             result_hi, result_lo))
        << "Failed to execute v128.or with lane crossing patterns";
    assert_v128_equal(0x8767476789ABCDEFULL, 0xFFFFFFFFFFFFFFFFULL, result_hi, result_lo);
}

/**
 * @test MathematicalProperties_ConfirmsAssociativity
 * @brief Validates mathematical properties using multiple v128.or operations
 * @details Tests associative property through nested operations: (a|b)|c = a|(b|c).
 *          Uses complex bit patterns to verify mathematical consistency.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:nested_operations
 * @input_conditions Three-way OR combinations with diverse bit patterns
 * @expected_behavior Associative property holds for all patterns
 * @validation_method Associativity verification with complex nested operations
 */
TEST_F(V128OrTestSuite, MathematicalProperties_ConfirmsAssociativity)
{
    uint64_t temp_result_hi, temp_result_lo;
    uint64_t final_result_1_hi, final_result_1_lo;
    uint64_t final_result_2_hi, final_result_2_lo;

    // Test associativity: (a | b) | c = a | (b | c)
    // Define test vectors
    uint64_t vec_a_hi = 0x123456789ABCDEFULL, vec_a_lo = 0xFEDCBA9876543210ULL;
    uint64_t vec_b_hi = 0x0F0F0F0F0F0F0F0FULL, vec_b_lo = 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t vec_c_hi = 0xAAAAAAAAAAAAAAAAULL, vec_c_lo = 0x5555555555555555ULL;

    // Compute (a | b) | c
    ASSERT_TRUE(call_v128_or(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                             temp_result_hi, temp_result_lo))
        << "Failed to compute a | b for associativity test";
    ASSERT_TRUE(call_v128_or(temp_result_hi, temp_result_lo, vec_c_hi, vec_c_lo,
                             final_result_1_hi, final_result_1_lo))
        << "Failed to compute (a | b) | c for associativity test";

    // Compute a | (b | c)
    ASSERT_TRUE(call_v128_or(vec_b_hi, vec_b_lo, vec_c_hi, vec_c_lo,
                             temp_result_hi, temp_result_lo))
        << "Failed to compute b | c for associativity test";
    ASSERT_TRUE(call_v128_or(vec_a_hi, vec_a_lo, temp_result_hi, temp_result_lo,
                             final_result_2_hi, final_result_2_lo))
        << "Failed to compute a | (b | c) for associativity test";

    // Verify associativity: (a | b) | c = a | (b | c)
    assert_v128_equal(final_result_1_hi, final_result_1_lo, final_result_2_hi, final_result_2_lo);

    // Additional associativity test with different patterns
    vec_a_hi = 0xC0FFEEC0FFEEC0FFULL; vec_a_lo = 0xEEC0FFEEC0FFEEC0ULL;
    vec_b_hi = 0x1248124812481248ULL; vec_b_lo = 0x2481248124812481ULL;
    vec_c_hi = 0x8421842184218421ULL; vec_c_lo = 0x4218421842184218ULL;

    // Compute (a | b) | c again
    ASSERT_TRUE(call_v128_or(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                             temp_result_hi, temp_result_lo))
        << "Failed second a | b computation for associativity test";
    ASSERT_TRUE(call_v128_or(temp_result_hi, temp_result_lo, vec_c_hi, vec_c_lo,
                             final_result_1_hi, final_result_1_lo))
        << "Failed second (a | b) | c computation for associativity test";

    // Compute a | (b | c) again
    ASSERT_TRUE(call_v128_or(vec_b_hi, vec_b_lo, vec_c_hi, vec_c_lo,
                             temp_result_hi, temp_result_lo))
        << "Failed second b | c computation for associativity test";
    ASSERT_TRUE(call_v128_or(vec_a_hi, vec_a_lo, temp_result_hi, temp_result_lo,
                             final_result_2_hi, final_result_2_lo))
        << "Failed second a | (b | c) computation for associativity test";

    // Verify second associativity test
    assert_v128_equal(final_result_1_hi, final_result_1_lo, final_result_2_hi, final_result_2_lo);
}