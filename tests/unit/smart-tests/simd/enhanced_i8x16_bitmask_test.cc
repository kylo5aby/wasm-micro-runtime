/**
 * @file enhanced_i8x16_bitmask_test.cc
 * @brief Comprehensive unit tests for i8x16.bitmask SIMD opcode
 * @details Tests i8x16.bitmask functionality across interpreter and AOT execution modes
 *          with focus on MSB extraction, bit pattern validation, and cross-mode consistency.
 *          Validates WAMR SIMD implementation correctness for i8x16 bitmask extraction.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_bitmask_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class I8x16BitmaskTestSuite
 * @brief Test fixture class for i8x16.bitmask opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD bitmask result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge validation for MSB extraction patterns
 */
class I8x16BitmaskTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for i8x16.bitmask testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_bitmask_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.bitmask test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_bitmask_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i8x16.bitmask tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_bitmask_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM i8x16.bitmask function with 16 i8 lane values
     * @details Creates i8x16 vector from individual lane values and extracts MSB bitmask.
     *          Handles WASM function invocation and i32 bitmask result extraction.
     * @param lanes Array of 16 i8 values representing each lane of the vector
     * @param result Reference to store i32 bitmask result (16-bit pattern in lower bits)
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_bitmask_test.cc:call_i8x16_bitmask
     */
    bool call_i8x16_bitmask(const int8_t lanes[16], uint32_t& result)
    {
        // Prepare arguments: 16 i8 values as WASM function parameters
        uint32_t argv[16];
        for (int i = 0; i < 16; i++) {
            argv[i] = static_cast<uint32_t>(static_cast<int32_t>(lanes[i]));
        }

        // Call WASM function with 16 i8 inputs to create vector and extract bitmask
        bool call_success = dummy_env->execute("test_i8x16_bitmask_lanes", 16, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_i8x16_bitmask_lanes function";

        if (call_success) {
            // Extract i32 bitmask result (16-bit pattern in lower bits)
            result = argv[0];
        }

        return call_success;
    }

    /**
     * @brief Helper function to call WASM i8x16.bitmask function with two 64-bit inputs
     * @details Creates i8x16 vector from high/low 64-bit parts and extracts MSB bitmask.
     *          Alternative input method for testing specific bit patterns.
     * @param input_hi High 64 bits of input v128 vector
     * @param input_lo Low 64 bits of input v128 vector
     * @param result Reference to store i32 bitmask result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_bitmask_test.cc:call_i8x16_bitmask_v128
     */
    bool call_i8x16_bitmask_v128(uint64_t input_hi, uint64_t input_lo, uint32_t& result)
    {
        // Prepare arguments: one input v128 vector as two i64 values
        uint32_t argv[4];
        // WASM expects little-endian format: low part first, then high part
        argv[0] = static_cast<uint32_t>(input_lo);        // Low 32 bits of low i64
        argv[1] = static_cast<uint32_t>(input_lo >> 32);  // High 32 bits of low i64
        argv[2] = static_cast<uint32_t>(input_hi);        // Low 32 bits of high i64
        argv[3] = static_cast<uint32_t>(input_hi >> 32);  // High 32 bits of high i64

        // Call WASM function with one v128 input
        bool call_success = dummy_env->execute("test_i8x16_bitmask_v128", 4, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_i8x16_bitmask_v128 function";

        if (call_success) {
            // Extract i32 bitmask result
            result = argv[0];
        }

        return call_success;
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicBitmask_ReturnsCorrectPattern
 * @brief Validates i8x16.bitmask produces correct MSB extraction for typical lane patterns
 * @details Tests fundamental bitmask operation with mixed positive/negative values, validates
 *          that MSB of each i8 lane correctly contributes to the 16-bit result pattern.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i8x16_bitmask
 * @input_conditions Mixed positive/negative i8 values in different lane arrangements
 * @expected_behavior MSB=1 lanes set corresponding bits, MSB=0 lanes clear bits
 * @validation_method Direct comparison of WASM function result with calculated bitmask
 */
TEST_F(I8x16BitmaskTestSuite, BasicBitmask_ReturnsCorrectPattern)
{
    uint32_t result;

    // Test alternating positive/negative pattern: [1, -1, 2, -2, 3, -3, ...]
    // Expected: 0xAAAA (bits 1,3,5,7,9,11,13,15 set for negative values)
    int8_t alternating_lanes[16] = {1, -1, 2, -2, 3, -3, 4, -4, 5, -5, 6, -6, 7, -7, 8, -8};
    ASSERT_TRUE(call_i8x16_bitmask(alternating_lanes, result))
        << "Failed to execute i8x16.bitmask with alternating positive/negative pattern";
    ASSERT_EQ(0xAAAAU, result) << "i8x16.bitmask should return 0xAAAA for alternating pos/neg pattern";

    // Test mixed pattern with known MSBs: some negative, some positive in specific positions
    // Expected: 0x00F0 (bits 4,5,6,7 set for lanes with negative values)
    int8_t mixed_lanes[16] = {10, 20, 30, 40, -50, -60, -70, -80, 90, 100, 110, 120, 13, 14, 15, 16};
    ASSERT_TRUE(call_i8x16_bitmask(mixed_lanes, result))
        << "Failed to execute i8x16.bitmask with mixed positive/negative pattern";
    ASSERT_EQ(0x00F0U, result) << "i8x16.bitmask should return 0x00F0 for mixed pattern";

    // Test first and last lanes negative: lanes 0 and 15
    // Expected: 0x8001 (bits 0 and 15 set)
    int8_t first_last_lanes[16] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, -15};
    ASSERT_TRUE(call_i8x16_bitmask(first_last_lanes, result))
        << "Failed to execute i8x16.bitmask with first and last lanes negative";
    ASSERT_EQ(0x8001U, result) << "i8x16.bitmask should return 0x8001 for first/last negative pattern";
}

/**
 * @test BoundaryValues_ProducesExpectedMasks
 * @brief Validates i8x16.bitmask behavior at i8 boundary conditions and extreme values
 * @details Tests MSB extraction with MIN_VALUE (-128), MAX_VALUE (127), and transition values.
 *          Validates proper handling of signed i8 boundaries and sign bit detection.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/compilation/simd/simd_bitmask_extracts.c:simd_build_bitmask
 * @input_conditions i8 MIN/MAX values, zero, -1, boundary transitions
 * @expected_behavior Correct MSB detection: -128→1, 127→0, -1→1, 0→0
 * @validation_method Verify bitmask patterns for boundary value combinations
 */
TEST_F(I8x16BitmaskTestSuite, BoundaryValues_ProducesExpectedMasks)
{
    uint32_t result;

    // Test all MIN_VALUE (-128): all MSBs = 1
    // Expected: 0xFFFF (all 16 bits set)
    int8_t all_min_lanes[16] = {-128, -128, -128, -128, -128, -128, -128, -128,
                                -128, -128, -128, -128, -128, -128, -128, -128};
    ASSERT_TRUE(call_i8x16_bitmask(all_min_lanes, result))
        << "Failed to execute i8x16.bitmask with all MIN_VALUE lanes";
    ASSERT_EQ(0xFFFFU, result) << "i8x16.bitmask should return 0xFFFF for all MIN_VALUE (-128) lanes";

    // Test all MAX_VALUE (127): all MSBs = 0
    // Expected: 0x0000 (no bits set)
    int8_t all_max_lanes[16] = {127, 127, 127, 127, 127, 127, 127, 127,
                                127, 127, 127, 127, 127, 127, 127, 127};
    ASSERT_TRUE(call_i8x16_bitmask(all_max_lanes, result))
        << "Failed to execute i8x16.bitmask with all MAX_VALUE lanes";
    ASSERT_EQ(0x0000U, result) << "i8x16.bitmask should return 0x0000 for all MAX_VALUE (127) lanes";

    // Test all -1: all MSBs = 1
    // Expected: 0xFFFF (all bits set)
    int8_t all_neg_one_lanes[16] = {-1, -1, -1, -1, -1, -1, -1, -1,
                                    -1, -1, -1, -1, -1, -1, -1, -1};
    ASSERT_TRUE(call_i8x16_bitmask(all_neg_one_lanes, result))
        << "Failed to execute i8x16.bitmask with all -1 lanes";
    ASSERT_EQ(0xFFFFU, result) << "i8x16.bitmask should return 0xFFFF for all -1 lanes";

    // Test all zero: all MSBs = 0
    // Expected: 0x0000 (no bits set)
    int8_t all_zero_lanes[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_TRUE(call_i8x16_bitmask(all_zero_lanes, result))
        << "Failed to execute i8x16.bitmask with all zero lanes";
    ASSERT_EQ(0x0000U, result) << "i8x16.bitmask should return 0x0000 for all zero lanes";
}

/**
 * @test SingleLanePatterns_ProducesPowerOfTwo
 * @brief Validates i8x16.bitmask produces single-bit results for isolated negative lanes
 * @details Tests MSB extraction when only one lane has MSB set, verifying bit position
 *          correspondence and proper isolation of individual lane contributions.
 * @test_category Edge - Lane isolation and bit position validation
 * @coverage_target Bit position mapping: lane N → bit N in result
 * @input_conditions Single negative lane at different positions, rest positive
 * @expected_behavior Single bit set at position corresponding to negative lane
 * @validation_method Verify power-of-2 results for each individual lane position
 */
TEST_F(I8x16BitmaskTestSuite, SingleLanePatterns_ProducesPowerOfTwo)
{
    uint32_t result;

    // Test single lane 0 negative: only bit 0 should be set
    // Expected: 0x0001
    int8_t single_lane0[16] = {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    ASSERT_TRUE(call_i8x16_bitmask(single_lane0, result))
        << "Failed to execute i8x16.bitmask with single lane 0 negative";
    ASSERT_EQ(0x0001U, result) << "i8x16.bitmask should return 0x0001 for single lane 0 negative";

    // Test single lane 7 negative: only bit 7 should be set
    // Expected: 0x0080
    int8_t single_lane7[16] = {1, 1, 1, 1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1};
    ASSERT_TRUE(call_i8x16_bitmask(single_lane7, result))
        << "Failed to execute i8x16.bitmask with single lane 7 negative";
    ASSERT_EQ(0x0080U, result) << "i8x16.bitmask should return 0x0080 for single lane 7 negative";

    // Test single lane 15 negative: only bit 15 should be set
    // Expected: 0x8000
    int8_t single_lane15[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1};
    ASSERT_TRUE(call_i8x16_bitmask(single_lane15, result))
        << "Failed to execute i8x16.bitmask with single lane 15 negative";
    ASSERT_EQ(0x8000U, result) << "i8x16.bitmask should return 0x8000 for single lane 15 negative";

    // Test single lane 8 negative: only bit 8 should be set (middle position)
    // Expected: 0x0100
    int8_t single_lane8[16] = {1, 1, 1, 1, 1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1, 1};
    ASSERT_TRUE(call_i8x16_bitmask(single_lane8, result))
        << "Failed to execute i8x16.bitmask with single lane 8 negative";
    ASSERT_EQ(0x0100U, result) << "i8x16.bitmask should return 0x0100 for single lane 8 negative";
}

/**
 * @test SymmetricPatterns_ReflectMirrorBehavior
 * @brief Validates i8x16.bitmask produces symmetric bitmasks for mirror lane patterns
 * @details Tests MSB extraction with symmetric and mirrored lane arrangements, validates
 *          mathematical properties and pattern recognition in bitmask generation.
 * @test_category Edge - Pattern symmetry and mathematical property validation
 * @coverage_target Symmetric pattern handling and bit arrangement validation
 * @input_conditions Mirror patterns, symmetric arrangements, mathematical sequences
 * @expected_behavior Symmetric bitmask results reflecting input lane symmetry
 * @validation_method Verify bitmask symmetry properties and mathematical correctness
 */
TEST_F(I8x16BitmaskTestSuite, SymmetricPatterns_ReflectMirrorBehavior)
{
    uint32_t result;

    // Test symmetric pattern: first 8 lanes positive, last 8 lanes negative
    // Expected: 0xFF00 (upper 8 bits set)
    int8_t symmetric_pattern[16] = {10, 20, 30, 40, 50, 60, 70, 80, -10, -20, -30, -40, -50, -60, -70, -80};
    ASSERT_TRUE(call_i8x16_bitmask(symmetric_pattern, result))
        << "Failed to execute i8x16.bitmask with symmetric positive/negative split";
    ASSERT_EQ(0xFF00U, result) << "i8x16.bitmask should return 0xFF00 for symmetric pos/neg split";

    // Test mirror pattern around center: lanes mirror each other's signs
    // Pattern: [-1, 1, -1, 1, -1, 1, -1, 1, 1, -1, 1, -1, 1, -1, 1, -1]
    // Expected: 0xAA55 (bits 0,2,4,6,9,11,13,15 set for negative values)
    int8_t mirror_pattern[16] = {-1, 1, -1, 1, -1, 1, -1, 1, 1, -1, 1, -1, 1, -1, 1, -1};
    ASSERT_TRUE(call_i8x16_bitmask(mirror_pattern, result))
        << "Failed to execute i8x16.bitmask with mirror alternating pattern";
    ASSERT_EQ(0xAA55U, result) << "i8x16.bitmask should return 0xAA55 for mirror alternating pattern";

    // Test quarter patterns: each 4-lane group has different sign pattern
    // Pattern: [+,+,+,+, -,-,-,-, +,+,+,+, -,-,-,-]
    // Expected: 0xF0F0 (bits 4-7 and 12-15 set)
    int8_t quarter_pattern[16] = {1, 2, 3, 4, -1, -2, -3, -4, 5, 6, 7, 8, -5, -6, -7, -8};
    ASSERT_TRUE(call_i8x16_bitmask(quarter_pattern, result))
        << "Failed to execute i8x16.bitmask with quarter-section pattern";
    ASSERT_EQ(0xF0F0U, result) << "i8x16.bitmask should return 0xF0F0 for quarter-section pattern";
}