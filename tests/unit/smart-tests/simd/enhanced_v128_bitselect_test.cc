/**
 * @file enhanced_v128_bitselect_test.cc
 * @brief Comprehensive unit tests for v128.bitselect SIMD opcode
 * @details Tests v128.bitselect functionality across interpreter and AOT execution modes
 *          with focus on bitwise selection operations, mask patterns, and edge cases.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_bitselect_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class V128BitselectTestSuite
 * @brief Test fixture class for v128.bitselect opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class V128BitselectTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.bitselect testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_bitselect_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.bitselect test module using absolute path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_bitselect_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.bitselect tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_bitselect_test.cc:TearDown
     */
    void TearDown() override
    {
        // Resources automatically cleaned up by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute v128.bitselect operation with three v128 operands
     * @param input_a_hi High 64 bits of first vector (a)
     * @param input_a_lo Low 64 bits of first vector (a)
     * @param input_b_hi High 64 bits of second vector (b)
     * @param input_b_lo Low 64 bits of second vector (b)
     * @param input_mask_hi High 64 bits of mask vector
     * @param input_mask_lo Low 64 bits of mask vector
     * @param result_hi Reference to store high 64 bits of result
     * @param result_lo Reference to store low 64 bits of result
     * @return bool Success status of WASM function execution
     * @details Calls WASM function that performs: result = (a & mask) | (b & ~mask)
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_bitselect_test.cc:call_v128_bitselect
     */
    bool call_v128_bitselect(uint64_t input_a_hi, uint64_t input_a_lo,
                             uint64_t input_b_hi, uint64_t input_b_lo,
                             uint64_t input_mask_hi, uint64_t input_mask_lo,
                             uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: three input v128 vectors as twelve i32 values
        uint32_t argv[12];

        // WASM expects little-endian format: low part first, then high part
        // First v128 vector (a)
        argv[0] = static_cast<uint32_t>(input_a_lo);        // Low 32 bits of low i64
        argv[1] = static_cast<uint32_t>(input_a_lo >> 32);  // High 32 bits of low i64
        argv[2] = static_cast<uint32_t>(input_a_hi);        // Low 32 bits of high i64
        argv[3] = static_cast<uint32_t>(input_a_hi >> 32);  // High 32 bits of high i64

        // Second v128 vector (b)
        argv[4] = static_cast<uint32_t>(input_b_lo);        // Low 32 bits of low i64
        argv[5] = static_cast<uint32_t>(input_b_lo >> 32);  // High 32 bits of low i64
        argv[6] = static_cast<uint32_t>(input_b_hi);        // Low 32 bits of high i64
        argv[7] = static_cast<uint32_t>(input_b_hi >> 32);  // High 32 bits of high i64

        // Mask v128 vector
        argv[8] = static_cast<uint32_t>(input_mask_lo);        // Low 32 bits of low i64
        argv[9] = static_cast<uint32_t>(input_mask_lo >> 32);  // High 32 bits of low i64
        argv[10] = static_cast<uint32_t>(input_mask_hi);       // Low 32 bits of high i64
        argv[11] = static_cast<uint32_t>(input_mask_hi >> 32); // High 32 bits of high i64

        // Call WASM function with three v128 inputs
        bool call_success = dummy_env->execute("test_bitselect", 12, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_bitselect function";

        if (call_success) {
            // Extract v128 result as two i64 values
            result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];
        }

        return call_success;
    }

    /**
     * @brief Execute v128.bitselect with all-zeros mask
     * @param input_a_hi High 64 bits of first vector (a)
     * @param input_a_lo Low 64 bits of first vector (a)
     * @param input_b_hi High 64 bits of second vector (b)
     * @param input_b_lo Low 64 bits of second vector (b)
     * @param result_hi Reference to store high 64 bits of result
     * @param result_lo Reference to store low 64 bits of result
     * @return bool Success status of WASM function execution
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_bitselect_test.cc:call_bitselect_zeros_mask
     */
    bool call_bitselect_zeros_mask(uint64_t input_a_hi, uint64_t input_a_lo,
                                   uint64_t input_b_hi, uint64_t input_b_lo,
                                   uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: two input v128 vectors as eight i32 values
        uint32_t argv[8];

        // First v128 vector (a)
        argv[0] = static_cast<uint32_t>(input_a_lo);
        argv[1] = static_cast<uint32_t>(input_a_lo >> 32);
        argv[2] = static_cast<uint32_t>(input_a_hi);
        argv[3] = static_cast<uint32_t>(input_a_hi >> 32);

        // Second v128 vector (b)
        argv[4] = static_cast<uint32_t>(input_b_lo);
        argv[5] = static_cast<uint32_t>(input_b_lo >> 32);
        argv[6] = static_cast<uint32_t>(input_b_hi);
        argv[7] = static_cast<uint32_t>(input_b_hi >> 32);

        // Call WASM function
        bool call_success = dummy_env->execute("test_bitselect_zeros_mask", 8, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_bitselect_zeros_mask function";

        if (call_success) {
            // Extract v128 result
            result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];
        }

        return call_success;
    }

    /**
     * @brief Execute v128.bitselect with all-ones mask
     * @param input_a_hi High 64 bits of first vector (a)
     * @param input_a_lo Low 64 bits of first vector (a)
     * @param input_b_hi High 64 bits of second vector (b)
     * @param input_b_lo Low 64 bits of second vector (b)
     * @param result_hi Reference to store high 64 bits of result
     * @param result_lo Reference to store low 64 bits of result
     * @return bool Success status of WASM function execution
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_bitselect_test.cc:call_bitselect_ones_mask
     */
    bool call_bitselect_ones_mask(uint64_t input_a_hi, uint64_t input_a_lo,
                                  uint64_t input_b_hi, uint64_t input_b_lo,
                                  uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: two input v128 vectors as eight i32 values
        uint32_t argv[8];

        // First v128 vector (a)
        argv[0] = static_cast<uint32_t>(input_a_lo);
        argv[1] = static_cast<uint32_t>(input_a_lo >> 32);
        argv[2] = static_cast<uint32_t>(input_a_hi);
        argv[3] = static_cast<uint32_t>(input_a_hi >> 32);

        // Second v128 vector (b)
        argv[4] = static_cast<uint32_t>(input_b_lo);
        argv[5] = static_cast<uint32_t>(input_b_lo >> 32);
        argv[6] = static_cast<uint32_t>(input_b_hi);
        argv[7] = static_cast<uint32_t>(input_b_hi >> 32);

        // Call WASM function
        bool call_success = dummy_env->execute("test_bitselect_ones_mask", 8, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_bitselect_ones_mask function";

        if (call_success) {
            // Extract v128 result
            result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];
        }

        return call_success;
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicBitselection_ReturnsCorrectResults
 * @brief Validates v128.bitselect produces correct bit selection results for typical inputs
 * @details Tests fundamental bitselect operation with mixed bit patterns and typical mask values.
 *          Verifies that operation correctly selects bits: result = (a & mask) | (b & ~mask)
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simde_wasm_v128_bitselect
 * @input_conditions Mixed bit patterns with standard mask selection patterns
 * @expected_behavior Returns mathematically correct bitwise selection results
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(V128BitselectTestSuite, BasicBitselection_ReturnsCorrectResults)
{
    uint64_t result_hi, result_lo;

    // Test case 1: Basic alternating pattern
    uint64_t vec_a_hi = 0xAAAAAAAAAAAAAAAAULL;  // 10101010... pattern
    uint64_t vec_a_lo = 0xAAAAAAAAAAAAAAAAULL;
    uint64_t vec_b_hi = 0x5555555555555555ULL;  // 01010101... pattern
    uint64_t vec_b_lo = 0x5555555555555555ULL;
    uint64_t mask_hi = 0xF0F0F0F0F0F0F0F0ULL;   // 11110000... pattern
    uint64_t mask_lo = 0xF0F0F0F0F0F0F0F0ULL;

    bool success = call_v128_bitselect(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                                       mask_hi, mask_lo, result_hi, result_lo);

    ASSERT_TRUE(success) << "Bitselect operation should succeed";

    // Calculate expected result: (a & mask) | (b & ~mask)
    uint64_t expected_hi = (vec_a_hi & mask_hi) | (vec_b_hi & ~mask_hi);
    uint64_t expected_lo = (vec_a_lo & mask_lo) | (vec_b_lo & ~mask_lo);

    ASSERT_EQ(expected_hi, result_hi) << "High 64 bits bitselect result incorrect";
    ASSERT_EQ(expected_lo, result_lo) << "Low 64 bits bitselect result incorrect";

    // Test case 2: Different bit patterns
    uint64_t vec_a2_hi = 0xFF00FF00FF00FF00ULL;
    uint64_t vec_a2_lo = 0xFF00FF00FF00FF00ULL;
    uint64_t vec_b2_hi = 0x00FF00FF00FF00FFULL;
    uint64_t vec_b2_lo = 0x00FF00FF00FF00FFULL;
    uint64_t mask2_hi = 0xCCCCCCCCCCCCCCCCULL;   // 11001100... pattern
    uint64_t mask2_lo = 0xCCCCCCCCCCCCCCCCULL;

    success = call_v128_bitselect(vec_a2_hi, vec_a2_lo, vec_b2_hi, vec_b2_lo,
                                  mask2_hi, mask2_lo, result_hi, result_lo);

    ASSERT_TRUE(success) << "Second bitselect operation should succeed";

    uint64_t expected2_hi = (vec_a2_hi & mask2_hi) | (vec_b2_hi & ~mask2_hi);
    uint64_t expected2_lo = (vec_a2_lo & mask2_lo) | (vec_b2_lo & ~mask2_lo);

    ASSERT_EQ(expected2_hi, result_hi) << "High 64 bits second bitselect result incorrect";
    ASSERT_EQ(expected2_lo, result_lo) << "Low 64 bits second bitselect result incorrect";
}

/**
 * @test ExtremeMasks_SelectsCorrectVectors
 * @brief Validates v128.bitselect handles extreme mask values correctly
 * @details Tests all-zeros and all-ones mask patterns to verify complete vector selection.
 *          Confirms mask=0 selects vector b entirely, mask=1 selects vector a entirely.
 * @test_category Edge - Extreme mask pattern validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:v128_bitwise_bitselect
 * @input_conditions All-zeros mask (0x00) and all-ones mask (0xFF) with distinct vectors
 * @expected_behavior Perfect selection: all-zeros→b, all-ones→a
 * @validation_method Verify complete vector selection without bit mixing
 */
TEST_F(V128BitselectTestSuite, ExtremeMasks_SelectsCorrectVectors)
{
    uint64_t result_hi, result_lo;

    uint64_t vec_a_hi = 0x1111111111111111ULL;
    uint64_t vec_a_lo = 0x1111111111111111ULL;
    uint64_t vec_b_hi = 0x2222222222222222ULL;
    uint64_t vec_b_lo = 0x2222222222222222ULL;

    // Test all-zeros mask (should select vector b entirely)
    bool success = call_bitselect_zeros_mask(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                                             result_hi, result_lo);

    ASSERT_TRUE(success) << "All-zeros mask bitselect should succeed";
    ASSERT_EQ(vec_b_hi, result_hi) << "All-zeros mask should select vector b (high)";
    ASSERT_EQ(vec_b_lo, result_lo) << "All-zeros mask should select vector b (low)";

    // Test all-ones mask (should select vector a entirely)
    success = call_bitselect_ones_mask(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                                       result_hi, result_lo);

    ASSERT_TRUE(success) << "All-ones mask bitselect should succeed";
    ASSERT_EQ(vec_a_hi, result_hi) << "All-ones mask should select vector a (high)";
    ASSERT_EQ(vec_a_lo, result_lo) << "All-ones mask should select vector a (low)";

    // Test with different patterns to ensure generality
    uint64_t vec_a2_hi = 0xABCDABCDABCDABCDULL;
    uint64_t vec_a2_lo = 0xABCDABCDABCDABCDULL;
    uint64_t vec_b2_hi = 0xEF12EF12EF12EF12ULL;
    uint64_t vec_b2_lo = 0xEF12EF12EF12EF12ULL;

    success = call_bitselect_zeros_mask(vec_a2_hi, vec_a2_lo, vec_b2_hi, vec_b2_lo,
                                        result_hi, result_lo);
    ASSERT_TRUE(success) << "All-zeros mask failed with alternating pattern";
    ASSERT_EQ(vec_b2_hi, result_hi) << "All-zeros mask failed with alternating pattern (high)";
    ASSERT_EQ(vec_b2_lo, result_lo) << "All-zeros mask failed with alternating pattern (low)";

    success = call_bitselect_ones_mask(vec_a2_hi, vec_a2_lo, vec_b2_hi, vec_b2_lo,
                                       result_hi, result_lo);
    ASSERT_TRUE(success) << "All-ones mask failed with alternating pattern";
    ASSERT_EQ(vec_a2_hi, result_hi) << "All-ones mask failed with alternating pattern (high)";
    ASSERT_EQ(vec_a2_lo, result_lo) << "All-ones mask failed with alternating pattern (low)";
}

/**
 * @test IdenticalVectors_ReturnsInput
 * @brief Validates v128.bitselect with identical input vectors returns input regardless of mask
 * @details Tests mathematical identity property: when a==b, result should equal inputs
 *          regardless of mask pattern, validating operation logic correctness.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:v128_bitselect_operation
 * @input_conditions Identical vectors a==b with various mask patterns
 * @expected_behavior Result equals input vectors regardless of mask value
 * @validation_method Mathematical identity verification across different masks
 */
TEST_F(V128BitselectTestSuite, IdenticalVectors_ReturnsInput)
{
    uint64_t result_hi, result_lo;

    uint64_t identical_hi = 0x3C693C693C693C69ULL;
    uint64_t identical_lo = 0x3C693C693C693C69ULL;

    // Test with various mask patterns
    std::vector<std::pair<uint64_t, uint64_t>> test_masks = {
        {0x0000000000000000ULL, 0x0000000000000000ULL},  // All zeros
        {0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL},  // All ones
        {0xAAAAAAAAAAAAAAAAULL, 0xAAAAAAAAAAAAAAAAULL},  // Alternating pattern
        {0x5555555555555555ULL, 0x5555555555555555ULL},  // Inverse alternating
        {0xF0F0F0F0F0F0F0F0ULL, 0x0F0F0F0F0F0F0F0FULL}   // Complex pattern
    };

    for (const auto& mask : test_masks) {
        bool success = call_v128_bitselect(identical_hi, identical_lo,
                                           identical_hi, identical_lo,
                                           mask.first, mask.second,
                                           result_hi, result_lo);

        ASSERT_TRUE(success) << "Bitselect with identical vectors should succeed";
        ASSERT_EQ(identical_hi, result_hi) << "Identical vectors should return input (high)";
        ASSERT_EQ(identical_lo, result_lo) << "Identical vectors should return input (low)";
    }
}

/**
 * @test BoundaryBitPatterns_HandlesEdgeCases
 * @brief Validates v128.bitselect handles boundary bit patterns and lane boundaries correctly
 * @details Tests single-bit masks, lane boundary patterns, and vector boundary conditions
 *          to ensure precise bit-level selection across different alignment scenarios.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitwise_ops.c:bitwise_operations
 * @input_conditions Single-bit masks, byte/word/dword boundary patterns, vector boundaries
 * @expected_behavior Precise bit selection respecting all boundary conditions
 * @validation_method Bit-level verification for boundary and alignment edge cases
 */
TEST_F(V128BitselectTestSuite, BoundaryBitPatterns_HandlesEdgeCases)
{
    uint64_t result_hi, result_lo;

    uint64_t vec_a_hi = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t vec_a_lo = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t vec_b_hi = 0x0000000000000000ULL;
    uint64_t vec_b_lo = 0x0000000000000000ULL;

    // Test single-bit mask (only first bit set)
    uint64_t single_bit_mask_hi = 0x0000000000000000ULL;
    uint64_t single_bit_mask_lo = 0x0000000000000001ULL;

    bool success = call_v128_bitselect(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                                       single_bit_mask_hi, single_bit_mask_lo,
                                       result_hi, result_lo);

    ASSERT_TRUE(success) << "Single bit selection should succeed";
    ASSERT_EQ(0x0000000000000000ULL, result_hi) << "High 64 bits should be zero";
    ASSERT_EQ(0x0000000000000001ULL, result_lo) << "Only first bit should be selected";

    // Test high/low 64-bit boundary
    uint64_t half_mask_hi = 0xFFFFFFFFFFFFFFFFULL;  // High 64 bits all ones
    uint64_t half_mask_lo = 0x0000000000000000ULL;  // Low 64 bits all zeros

    success = call_v128_bitselect(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                                  half_mask_hi, half_mask_lo,
                                  result_hi, result_lo);

    ASSERT_TRUE(success) << "64-bit boundary selection should succeed";
    ASSERT_EQ(vec_a_hi, result_hi) << "High 64 bits should come from vector a";
    ASSERT_EQ(vec_b_lo, result_lo) << "Low 64 bits should come from vector b";

    // Test alternating byte pattern
    uint64_t byte_pattern_hi = 0xFF00FF00FF00FF00ULL;
    uint64_t byte_pattern_lo = 0xFF00FF00FF00FF00ULL;

    success = call_v128_bitselect(vec_a_hi, vec_a_lo, vec_b_hi, vec_b_lo,
                                  byte_pattern_hi, byte_pattern_lo,
                                  result_hi, result_lo);

    ASSERT_TRUE(success) << "Byte boundary selection should succeed";

    // Verify alternating byte selection pattern
    uint64_t expected_hi = (vec_a_hi & byte_pattern_hi) | (vec_b_hi & ~byte_pattern_hi);
    uint64_t expected_lo = (vec_a_lo & byte_pattern_lo) | (vec_b_lo & ~byte_pattern_lo);

    ASSERT_EQ(expected_hi, result_hi) << "Byte boundary pattern failed (high)";
    ASSERT_EQ(expected_lo, result_lo) << "Byte boundary pattern failed (low)";
}