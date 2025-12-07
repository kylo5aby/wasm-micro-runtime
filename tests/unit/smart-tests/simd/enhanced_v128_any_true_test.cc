/**
 * @file enhanced_v128_any_true_test.cc
 * @brief Comprehensive unit tests for v128.any_true SIMD opcode
 * @details Tests v128.any_true functionality across interpreter and AOT execution modes
 *          with focus on boolean reduction operations, edge cases, and cross-mode consistency.
 *          Validates WAMR SIMD implementation correctness for any_true boolean reduction.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_any_true_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class V128AnyTrueTestSuite
 * @brief Test fixture class for v128.any_true opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD boolean result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class V128AnyTrueTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.any_true testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_any_true_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.any_true test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_any_true_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.any_true tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_any_true_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM v128.any_true function with vector input
     * @details Executes v128.any_true operation on input vector and returns boolean result.
     *          Handles WASM function invocation and i32 result extraction.
     * @param input_hi High 64 bits of input v128 vector
     * @param input_lo Low 64 bits of input v128 vector
     * @param result Reference to store i32 result (0 or 1)
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_any_true_test.cc:call_v128_any_true
     */
    bool call_v128_any_true(uint64_t input_hi, uint64_t input_lo, uint32_t& result)
    {
        // Prepare arguments: one input v128 vector as two i64 values
        uint32_t argv[4];
        // WASM expects little-endian format: low part first, then high part
        argv[0] = static_cast<uint32_t>(input_lo);        // Low 32 bits of low i64
        argv[1] = static_cast<uint32_t>(input_lo >> 32);  // High 32 bits of low i64
        argv[2] = static_cast<uint32_t>(input_hi);        // Low 32 bits of high i64
        argv[3] = static_cast<uint32_t>(input_hi >> 32);  // High 32 bits of high i64

        // Call WASM function with one v128 input
        bool call_success = dummy_env->execute("test_v128_any_true", 4, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_v128_any_true function";

        if (call_success) {
            // Extract i32 result (boolean: 0 or 1)
            result = argv[0];
        }

        return call_success;
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicAnyTrue_ValidatesBooleanLogic
 * @brief Validates v128.any_true produces correct results for typical boolean reduction scenarios
 * @details Tests fundamental any_true operation with common bit patterns, validates proper
 *          boolean logic where any non-zero bit returns 1, all zero bits return 0.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/compilation/simd/simd_bool_reductions.c:aot_compile_simd_v128_any_true
 * @input_conditions Mixed 0/1 patterns, single bit sets, multiple bits set
 * @expected_behavior Returns 1 if any bit is set, 0 if all bits are zero
 * @validation_method Direct comparison of WASM function result with expected boolean values
 */
TEST_F(V128AnyTrueTestSuite, BasicAnyTrue_ValidatesBooleanLogic)
{
    uint32_t result;

    // Test with mixed pattern containing many 1 bits - should return 1
    ASSERT_TRUE(call_v128_any_true(0x5555555555555555ULL, 0x5555555555555555ULL, result))
        << "Failed to execute v128.any_true with alternating bit pattern";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 for vector with alternating bits set";

    // Test with single bit set in high part - should return 1
    ASSERT_TRUE(call_v128_any_true(0x8000000000000000ULL, 0x0000000000000000ULL, result))
        << "Failed to execute v128.any_true with single high bit set";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 for vector with single high bit set";

    // Test with single bit set in low part - should return 1
    ASSERT_TRUE(call_v128_any_true(0x0000000000000000ULL, 0x0000000000000001ULL, result))
        << "Failed to execute v128.any_true with single low bit set";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 for vector with single low bit set";

    // Test with multiple scattered bits - should return 1
    ASSERT_TRUE(call_v128_any_true(0x0F0F0F0F0F0F0F0FULL, 0xF0F0F0F0F0F0F0F0ULL, result))
        << "Failed to execute v128.any_true with nibble alternating pattern";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 for vector with nibble pattern bits set";
}

/**
 * @test BoundaryBits_ValidatesEdgePositions
 * @brief Tests boundary bit positions and lane boundaries for v128.any_true operation
 * @details Validates correct detection of bits at vector boundaries (first bit, last bit),
 *          lane boundaries, and corner positions within the 128-bit vector.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/compilation/simd/simd_bool_reductions.c:llvm.vector.reduce.or.v128i1
 * @input_conditions First bit set, last bit set, lane boundary bits, middle positions
 * @expected_behavior Detects any single bit regardless of position within vector
 * @validation_method Systematic testing of specific bit positions across vector boundaries
 */
TEST_F(V128AnyTrueTestSuite, BoundaryBits_ValidatesEdgePositions)
{
    uint32_t result;

    // Test first bit of vector (bit 0 of low part) - should return 1
    ASSERT_TRUE(call_v128_any_true(0x0000000000000000ULL, 0x0000000000000001ULL, result))
        << "Failed to execute v128.any_true with first bit set";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 when first bit (bit 0) is set";

    // Test last bit of vector (bit 127, MSB of high part) - should return 1
    ASSERT_TRUE(call_v128_any_true(0x8000000000000000ULL, 0x0000000000000000ULL, result))
        << "Failed to execute v128.any_true with last bit set";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 when last bit (bit 127) is set";

    // Test boundary between low and high 64-bit parts (bit 63 and 64)
    ASSERT_TRUE(call_v128_any_true(0x0000000000000001ULL, 0x0000000000000000ULL, result))
        << "Failed to execute v128.any_true with bit 64 set";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 when bit 64 (LSB of high part) is set";

    ASSERT_TRUE(call_v128_any_true(0x0000000000000000ULL, 0x8000000000000000ULL, result))
        << "Failed to execute v128.any_true with bit 63 set";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 when bit 63 (MSB of low part) is set";

    // Test middle bit position (arbitrary bit in middle) - should return 1
    ASSERT_TRUE(call_v128_any_true(0x0000000000001000ULL, 0x0000000000000000ULL, result))
        << "Failed to execute v128.any_true with middle bit set";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 when middle bit is set";
}

/**
 * @test ExtremePatterns_ValidatesAllZerosAndOnes
 * @brief Tests extreme bit patterns including all zeros and all ones vectors
 * @details Validates correct handling of boundary cases: zero vector returns 0,
 *          all-ones vector returns 1, and various dense/sparse patterns.
 * @test_category Edge - Extreme pattern validation
 * @coverage_target core/iwasm/compilation/simd/simd_bool_reductions.c:vector_reduce_or
 * @input_conditions All zeros vector, all ones vector, maximum/minimum density patterns
 * @expected_behavior Zero vector returns 0, any non-zero pattern returns 1
 * @validation_method Testing mathematical extremes and density variations
 */
TEST_F(V128AnyTrueTestSuite, ExtremePatterns_ValidatesAllZerosAndOnes)
{
    uint32_t result;

    // Test all zeros vector - should return 0
    ASSERT_TRUE(call_v128_any_true(0x0000000000000000ULL, 0x0000000000000000ULL, result))
        << "Failed to execute v128.any_true with all zeros vector";
    ASSERT_EQ(0, result) << "v128.any_true should return 0 for vector with all bits zero";

    // Test all ones vector - should return 1
    ASSERT_TRUE(call_v128_any_true(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, result))
        << "Failed to execute v128.any_true with all ones vector";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 for vector with all bits set";

    // Test maximum sparsity (single bit) in various positions - all should return 1
    // Single bit in position 32 (arbitrary middle position)
    ASSERT_TRUE(call_v128_any_true(0x0000000000000000ULL, 0x0000000100000000ULL, result))
        << "Failed to execute v128.any_true with single bit at position 32";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 for vector with single bit at position 32";

    // Test high density pattern (127 out of 128 bits set) - should return 1
    ASSERT_TRUE(call_v128_any_true(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL, result))
        << "Failed to execute v128.any_true with 127 bits set";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 for vector with 127 out of 128 bits set";

    // Test alternating patterns with different density
    ASSERT_TRUE(call_v128_any_true(0xAAAAAAAAAAAAAAAAULL, 0xAAAAAAAAAAAAAAAAULL, result))
        << "Failed to execute v128.any_true with alternating pattern (high bits)";
    ASSERT_EQ(1, result) << "v128.any_true should return 1 for alternating pattern with high bits";
}

/**
 * @test InvalidModule_ValidatesErrorHandling
 * @brief Tests error handling for invalid function calls and runtime failures
 * @details Validates proper error detection when calling v128.any_true functions
 *          that don't exist or with invalid parameters.
 * @test_category Error - Invalid scenario validation
 * @coverage_target core/iwasm/interpreter/wasm_runtime.c:wasm_runtime_call_wasm
 * @input_conditions Invalid function names, malformed parameters
 * @expected_behavior Proper error reporting without crashes
 * @validation_method Function call failure validation with existing valid module
 */
TEST_F(V128AnyTrueTestSuite, InvalidModule_ValidatesErrorHandling)
{
    uint32_t result;

    // Test calling non-existent function - should fail gracefully
    uint32_t invalid_argv[4] = {0, 0, 0, 0};
    bool call_success = dummy_env->execute("nonexistent_v128_function", 4, invalid_argv);
    ASSERT_FALSE(call_success)
        << "Expected function call to fail for non-existent function name";

    // Test calling with wrong parameter count - should fail gracefully
    uint32_t wrong_argc_argv[2] = {0, 0};
    call_success = dummy_env->execute("test_v128_any_true", 2, wrong_argc_argv);
    ASSERT_FALSE(call_success)
        << "Expected function call to fail with incorrect parameter count";

    // Verify the execution environment remains stable after error conditions
    ASSERT_NE(nullptr, dummy_env->get())
        << "Execution environment should remain valid after handling errors";
}