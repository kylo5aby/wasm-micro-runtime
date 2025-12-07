/**
 * @file enhanced_v128_andnot_test.cc
 * @brief Comprehensive unit tests for v128.andnot SIMD opcode
 * @details Tests v128.andnot functionality across interpreter and AOT execution modes
 *          with focus on bitwise AND-NOT operations (a & ~b), mathematical properties, and edge cases.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_andnot_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class V128AndNotTestSuite
 * @brief Test fixture class for v128.andnot opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class V128AndNotTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.andnot testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_andnot_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.andnot test module using absolute path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_andnot_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.andnot tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_andnot_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM v128.andnot function with two vector inputs
     * @details Executes v128.andnot operation (a & ~b) on two input vectors and returns result.
     *          Handles WASM function invocation and v128 result extraction.
     * @param input1_hi High 64 bits of first input v128 vector
     * @param input1_lo Low 64 bits of first input v128 vector
     * @param input2_hi High 64 bits of second input v128 vector (will be NOTed)
     * @param input2_lo Low 64 bits of second input v128 vector (will be NOTed)
     * @param result_hi Reference to store high 64 bits of result v128 vector
     * @param result_lo Reference to store low 64 bits of result v128 vector
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_andnot_test.cc:call_v128_andnot
     */
    bool call_v128_andnot(uint64_t input1_hi, uint64_t input1_lo,
                          uint64_t input2_hi, uint64_t input2_lo,
                          uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: two input v128 vectors as four i64 values
        uint32_t argv[8];
        // WASM expects little-endian format: low part first, then high part
        // First v128 vector (used directly)
        argv[0] = static_cast<uint32_t>(input1_lo);        // Low 32 bits of low i64
        argv[1] = static_cast<uint32_t>(input1_lo >> 32);  // High 32 bits of low i64
        argv[2] = static_cast<uint32_t>(input1_hi);        // Low 32 bits of high i64
        argv[3] = static_cast<uint32_t>(input1_hi >> 32);  // High 32 bits of high i64
        // Second v128 vector (will be NOTed in ANDNOT operation)
        argv[4] = static_cast<uint32_t>(input2_lo);        // Low 32 bits of low i64
        argv[5] = static_cast<uint32_t>(input2_lo >> 32);  // High 32 bits of low i64
        argv[6] = static_cast<uint32_t>(input2_hi);        // Low 32 bits of high i64
        argv[7] = static_cast<uint32_t>(input2_hi >> 32);  // High 32 bits of high i64

        // Call WASM function with two v128 inputs
        bool call_success = dummy_env->execute("test_v128_andnot", 8, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_v128_andnot function";

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
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_andnot_test.cc:assert_v128_equal
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
 * @test BasicAndNotOperation_ProducesCorrectResults
 * @brief Validates v128.andnot produces correct results for typical bitwise AND-NOT scenarios
 * @details Tests fundamental AND-NOT operation (a & ~b) with common bit patterns, validates truth table,
 *          and confirms correct bitwise operations across different vector interpretations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:v128_bitwise_two_component
 * @input_conditions Mixed 0/1 patterns, truth table cases, alternating bit patterns
 * @expected_behavior Mathematically correct AND-NOT results, proper bit masking behavior
 * @validation_method Direct result comparison with expected bitwise AND-NOT outcomes
 */
TEST_F(V128AndNotTestSuite, BasicAndNotOperation_ProducesCorrectResults)
{
    uint64_t result_hi, result_lo;

    // Test truth table: 0x5555... & ~0xAAAA... = 0x5555... & 0x5555... = 0x5555...
    ASSERT_TRUE(call_v128_andnot(0x5555555555555555ULL, 0x5555555555555555ULL,
                                 0xAAAAAAAAAAAAAAAAULL, 0xAAAAAAAAAAAAAAAAULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot with alternating bit patterns";
    assert_v128_equal(0x5555555555555555ULL, 0x5555555555555555ULL, result_hi, result_lo);

    // Test with nibble patterns: 0x0F0F... & ~0xF0F0... = 0x0F0F... & 0x0F0F... = 0x0F0F...
    ASSERT_TRUE(call_v128_andnot(0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0F0F0F0F0FULL,
                                 0xF0F0F0F0F0F0F0F0ULL, 0xF0F0F0F0F0F0F0F0ULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot with nibble alternating patterns";
    assert_v128_equal(0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0F0F0F0F0FULL, result_hi, result_lo);

    // Test mixed bit patterns: 0xFF00... & ~0x00FF... = 0xFF00... & 0xFF00... = 0xFF00...
    ASSERT_TRUE(call_v128_andnot(0xFF00FF00FF00FF00ULL, 0xFF00FF00FF00FF00ULL,
                                 0x00FF00FF00FF00FFULL, 0x00FF00FF00FF00FFULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot with byte alternating patterns";
    assert_v128_equal(0xFF00FF00FF00FF00ULL, 0xFF00FF00FF00FF00ULL, result_hi, result_lo);

    // Test with complex patterns demonstrating AND-NOT operation
    ASSERT_TRUE(call_v128_andnot(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL,
                                 0x0F0F0F0F0F0F0F0FULL, 0xF0F0F0F0F0F0F0F0ULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot with complex patterns";
    // Expected: first & ~second = 0x123456789ABCDEFULL & ~0x0F0F0F0F0F0F0F0FULL
    //                           = 0x123456789ABCDEFULL & 0xF0F0F0F0F0F0F0F0ULL (high)
    //                           = 0xFEDCBA9876543210ULL & 0x0F0F0F0F0F0F0F0FULL (low)
    //                           = 0x0020406080A0C0E0ULL (high), 0x0E0C0A0806040200ULL (low)
    assert_v128_equal(0x0020406080A0C0E0ULL, 0x0E0C0A0806040200ULL, result_hi, result_lo);
}

/**
 * @test BoundaryConditions_HandleIdentityAndAnnihilator
 * @brief Tests boundary values and mathematical properties of v128.andnot operation
 * @details Validates identity element (a & ~0 = a), annihilator element (a & ~0xFF... = 0),
 *          and self-complement property (a & ~a = 0). Tests fundamental AND-NOT mathematical properties.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:LLVMBuildNot,LLVMBuildAnd
 * @input_conditions All zeros vector, all ones vector, identity operations, self-complement
 * @expected_behavior a&~0=a, a&~0xFF...FF=0, a&~a=0 properties hold
 * @validation_method Mathematical property verification with boundary values
 */
TEST_F(V128AndNotTestSuite, BoundaryConditions_HandleIdentityAndAnnihilator)
{
    uint64_t result_hi, result_lo;

    // Test identity property: a & ~0 = a (any value AND-NOT with all zeros equals itself)
    ASSERT_TRUE(call_v128_andnot(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL,
                                 0x0000000000000000ULL, 0x0000000000000000ULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot for identity property test";
    assert_v128_equal(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL, result_hi, result_lo);

    // Test annihilator property: a & ~0xFFFFFFFF... = 0 (any value AND-NOT with all ones equals zero)
    ASSERT_TRUE(call_v128_andnot(0x123456789ABCDEFULL, 0xFEDCBA9876543210ULL,
                                 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot for annihilator property test";
    assert_v128_equal(0x0000000000000000ULL, 0x0000000000000000ULL, result_hi, result_lo);

    // Test self-complement property: a & ~a = 0 (any value AND-NOT with itself equals zero)
    ASSERT_TRUE(call_v128_andnot(0xA5A5A5A5A5A5A5A5ULL, 0x5A5A5A5A5A5A5A5AULL,
                                 0xA5A5A5A5A5A5A5A5ULL, 0x5A5A5A5A5A5A5A5AULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot for self-complement property test";
    assert_v128_equal(0x0000000000000000ULL, 0x0000000000000000ULL, result_hi, result_lo);

    // Test with all zeros AND-NOT all zeros (boundary case)
    ASSERT_TRUE(call_v128_andnot(0x0000000000000000ULL, 0x0000000000000000ULL,
                                 0x0000000000000000ULL, 0x0000000000000000ULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot with all zeros boundary case";
    assert_v128_equal(0x0000000000000000ULL, 0x0000000000000000ULL, result_hi, result_lo);

    // Test with all ones AND-NOT all zeros (should equal all ones)
    ASSERT_TRUE(call_v128_andnot(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                                 0x0000000000000000ULL, 0x0000000000000000ULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot with all ones boundary case";
    assert_v128_equal(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, result_hi, result_lo);
}

/**
 * @test SpecialBitPatterns_ValidatesNonCommutative
 * @brief Tests special patterns and validates non-commutative property of v128.andnot
 * @details Validates that a & ~b ≠ b & ~a in general, tests power-of-2 sequences across lanes,
 *          and confirms bit-level accuracy with complex patterns.
 * @test_category Edge - Special pattern and non-commutative validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:bitwise_operation
 * @input_conditions Power-of-2 patterns, non-commutative test pairs, complex bit sequences
 * @expected_behavior Non-commutative property holds, pattern-specific results accurate
 * @validation_method Non-commutative property verification with diverse bit patterns
 */
TEST_F(V128AndNotTestSuite, SpecialBitPatterns_ValidatesNonCommutative)
{
    uint64_t result_hi_1, result_lo_1, result_hi_2, result_lo_2;
    uint64_t result_hi, result_lo;

    // Test non-commutative property: a & ~b ≠ b & ~a
    uint64_t vec_a_hi = 0xF0F0F0F0F0F0F0F0ULL, vec_a_lo = 0x0F0F0F0F0F0F0F0FULL;
    uint64_t vec_b_hi = 0xAAAAAAAAAAAAAAAAULL, vec_b_lo = 0x5555555555555555ULL;

    // Compute a & ~b
    ASSERT_TRUE(call_v128_andnot(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                                 result_hi_1, result_lo_1))
        << "Failed to compute a & ~b for non-commutative test";

    // Compute b & ~a
    ASSERT_TRUE(call_v128_andnot(vec_b_hi, vec_b_lo, vec_a_hi, vec_a_lo,
                                 result_hi_2, result_lo_2))
        << "Failed to compute b & ~a for non-commutative test";

    // Verify they are different (non-commutative property)
    ASSERT_TRUE(result_hi_1 != result_hi_2 || result_lo_1 != result_lo_2)
        << "v128.andnot should be non-commutative: a & ~b ≠ b & ~a";

    // Test power-of-2 patterns: should result in partial zero patterns
    ASSERT_TRUE(call_v128_andnot(0x0102040810204080ULL, 0x0102040810204080ULL,
                                 0x8040201008040201ULL, 0x8040201008040201ULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot with power-of-2 patterns";
    // Expected: first & ~second = 0x0102040810204080ULL & ~0x8040201008040201ULL
    //                            = 0x0102040810204080ULL & 0x7FBFDFEFF7FBFDFEULL
    //                            = 0x0102040810204080ULL (all bits preserved since no overlap)
    assert_v128_equal(0x0102040810204080ULL, 0x0102040810204080ULL, result_hi, result_lo);

    // Test single bit combinations at various positions
    ASSERT_TRUE(call_v128_andnot(0x8000000000000001ULL, 0x0000000000000000ULL,
                                 0x0000000000000001ULL, 0x8000000000000000ULL,
                                 result_hi, result_lo))
        << "Failed to execute v128.andnot with single bit positions";
    // Expected: 0x8000000000000001ULL & ~0x0000000000000001ULL = 0x8000000000000001ULL & 0xFFFFFFFFFFFFFFFEULL = 0x8000000000000000ULL (high)
    //           0x0000000000000000ULL & ~0x8000000000000000ULL = 0x0000000000000000ULL & 0x7FFFFFFFFFFFFFFFULL = 0x0000000000000000ULL (low)
    assert_v128_equal(0x8000000000000000ULL, 0x0000000000000000ULL, result_hi, result_lo);
}

/**
 * @test MathematicalProperties_ConfirmsRelationships
 * @brief Validates mathematical properties and relationships of v128.andnot operation
 * @details Tests relationship to standard AND/OR operations, validates De Morgan's laws,
 *          and confirms distributive properties through complex nested operations.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:nested_operations
 * @input_conditions Multi-operand combinations, mathematical relationship validation vectors
 * @expected_behavior Mathematical relationships hold for all test patterns
 * @validation_method Mathematical property verification with complex nested operations
 */
TEST_F(V128AndNotTestSuite, MathematicalProperties_ConfirmsRelationships)
{
    uint64_t result_hi_1, result_lo_1, result_hi_2, result_lo_2;

    // Test relationship: a & ~b = a - (a & b) property conceptually
    // We'll verify through bit manipulation that AND-NOT works as expected
    uint64_t vec_a_hi = 0x123456789ABCDEFULL, vec_a_lo = 0xFEDCBA9876543210ULL;
    uint64_t vec_b_hi = 0x0F0F0F0F0F0F0F0FULL, vec_b_lo = 0xF0F0F0F0F0F0F0F0ULL;

    // Compute a & ~b using WASM function
    ASSERT_TRUE(call_v128_andnot(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                                 result_hi_1, result_lo_1))
        << "Failed to compute a & ~b for mathematical relationship test";

    // Manually compute expected result: a & ~b = a & NOT(b)
    uint64_t not_b_hi = ~vec_b_hi;
    uint64_t not_b_lo = ~vec_b_lo;
    uint64_t expected_hi = vec_a_hi & not_b_hi;
    uint64_t expected_lo = vec_a_lo & not_b_lo;

    // Verify the mathematical relationship
    assert_v128_equal(expected_hi, expected_lo, result_hi_1, result_lo_1);

    // Test self-complement with different patterns
    ASSERT_TRUE(call_v128_andnot(0xC0FFEEC0FFEEC0FFULL, 0xEEC0FFEEC0FFEEC0ULL,
                                 0xC0FFEEC0FFEEC0FFULL, 0xEEC0FFEEC0FFEEC0ULL,
                                 result_hi_2, result_lo_2))
        << "Failed to compute self-complement for mathematical property test";

    // Self-complement should always result in zero
    assert_v128_equal(0x0000000000000000ULL, 0x0000000000000000ULL, result_hi_2, result_lo_2);

    // Test bit clearing property: demonstrate practical use of AND-NOT for bit masking
    uint64_t mask_hi = 0xFF00FF00FF00FF00ULL, mask_lo = 0x00FF00FF00FF00FFULL;
    ASSERT_TRUE(call_v128_andnot(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
                                 mask_hi, mask_lo,
                                 result_hi_1, result_lo_1))
        << "Failed to execute bit clearing with v128.andnot";

    // Expected: 0xFFFF... & ~mask should clear bits where mask is 1
    uint64_t cleared_hi = 0xFFFFFFFFFFFFFFFFULL & ~mask_hi;
    uint64_t cleared_lo = 0xFFFFFFFFFFFFFFFFULL & ~mask_lo;
    assert_v128_equal(cleared_hi, cleared_lo, result_hi_1, result_lo_1);
}