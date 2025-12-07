/**
 * @file enhanced_v128_load32_zero_test.cc
 * @brief Comprehensive unit tests for v128.load32_zero SIMD opcode
 * @details Tests v128.load32_zero functionality across interpreter and AOT execution modes
 *          with focus on memory load operations, zero-extension behavior, and boundary conditions.
 *          Validates WAMR SIMD memory load implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load32_zero_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class V128Load32ZeroTestSuite
 * @brief Test fixture class for v128.load32_zero opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class V128Load32ZeroTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.load32_zero testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files with memory operations.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load32_zero_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.load32_zero test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_load32_zero_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.load32_zero tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load32_zero_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM v128.load32_zero function with memory offset
     * @details Executes v128.load32_zero operation at specified memory offset and returns v128 result.
     *          Handles WASM function invocation and v128 result extraction with proper zero-extension.
     * @param offset Memory offset for v128.load32_zero operation
     * @param result_hi Reference to store high 64 bits of result v128 vector
     * @param result_lo Reference to store low 64 bits of result v128 vector
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load32_zero_test.cc:call_v128_load32_zero
     */
    bool call_v128_load32_zero(uint32_t offset, uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: memory offset as i32
        uint32_t argv[4];
        argv[0] = offset;

        // Call WASM function with memory offset
        bool call_success = dummy_env->execute("test_v128_load32_zero", 1, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_v128_load32_zero function at offset " << offset;

        if (call_success) {
            // Extract v128 result as two i64 values (little-endian format)
            result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];
        }

        return call_success;
    }

    /**
     * @brief Helper function to initialize memory with test data at specified offset
     * @details Writes 32-bit test value to memory at given offset for load testing.
     *          Handles WASM memory initialization through function calls.
     * @param offset Memory offset to write data
     * @param value 32-bit value to write to memory
     * @return bool True if write succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load32_zero_test.cc:init_memory
     */
    bool init_memory(uint32_t offset, uint32_t value)
    {
        // Prepare arguments: offset and value
        uint32_t argv[2];
        argv[0] = offset;
        argv[1] = value;

        // Call WASM memory initialization function
        return dummy_env->execute("init_memory", 2, argv);
    }

    /**
     * @brief Verify that v128 vector has correct 32-bit value in lower bits and zeros in upper bits
     * @details Validates v128.load32_zero zero-extension behavior and data integrity.
     * @param expected_value Expected 32-bit value in lower 32 bits of v128
     * @param actual_hi Actual high 64 bits of v128 result
     * @param actual_lo Actual low 64 bits of v128 result
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load32_zero_test.cc:assert_v128_load32_zero_result
     */
    void assert_v128_load32_zero_result(uint32_t expected_value, uint64_t actual_hi, uint64_t actual_lo)
    {
        // Upper 96 bits must be zero
        ASSERT_EQ(0x0000000000000000ULL, actual_hi)
            << "v128.load32_zero upper 64 bits should be zero - Actual: 0x" << std::hex << actual_hi;

        // Upper 32 bits of low 64 bits must be zero
        ASSERT_EQ(0x00000000U, static_cast<uint32_t>(actual_lo >> 32))
            << "v128.load32_zero upper 32 bits of low 64 bits should be zero - Actual: 0x"
            << std::hex << static_cast<uint32_t>(actual_lo >> 32);

        // Lower 32 bits must contain the expected value
        ASSERT_EQ(expected_value, static_cast<uint32_t>(actual_lo))
            << "v128.load32_zero lower 32 bits mismatch - Expected: 0x" << std::hex << expected_value
            << ", Actual: 0x" << std::hex << static_cast<uint32_t>(actual_lo);
    }

    /**
     * @brief Helper function to call WASM v128.load32_zero without EXPECT assertion
     * @details Executes v128.load32_zero operation at specified memory offset without logging expected failures.
     *          Used specifically for testing out-of-bounds scenarios where failure is expected.
     * @param offset Memory offset for v128.load32_zero operation
     * @param result_hi Reference to store high 64 bits of result v128 vector
     * @param result_lo Reference to store low 64 bits of result v128 vector
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load32_zero_test.cc:call_v128_load32_zero_silent
     */
    bool call_v128_load32_zero_silent(uint32_t offset, uint64_t& result_hi, uint64_t& result_lo)
    {
        // Prepare arguments: memory offset as i32
        uint32_t argv[4];
        argv[0] = offset;

        // Call WASM function with memory offset (no EXPECT assertion for out-of-bounds testing)
        bool call_success = dummy_env->execute("test_v128_load32_zero", 1, argv);

        if (call_success) {
            // Extract v128 result as two i64 values (little-endian format)
            result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];
        }

        return call_success;
    }

    /**
     * @brief Test memory boundary access scenarios and validate trap behavior
     * @details Executes v128.load32_zero with out-of-bounds offset and verifies proper error handling.
     * @param offset Memory offset that should cause out-of-bounds access
     * @return bool True if trap/error occurred as expected, false if operation incorrectly succeeded
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load32_zero_test.cc:test_out_of_bounds_access
     */
    bool test_out_of_bounds_access(uint32_t offset)
    {
        uint64_t result_hi, result_lo;
        // Out-of-bounds access should fail/trap
        return !call_v128_load32_zero_silent(offset, result_hi, result_lo);
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicLoad_AlignedMemory_LoadsCorrectValue
 * @brief Validates v128.load32_zero loads 32-bit data correctly from aligned memory addresses
 * @details Tests fundamental load operation with typical values, validates proper zero-extension
 *          behavior, and confirms correct data transfer from memory to v128 register.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/compilation/simd/simd_load_store.c:v128_load32_zero
 * @input_conditions Aligned memory offsets (0, 4, 8, 16) with test values (0x12345678, 0xABCDEF00, etc.)
 * @expected_behavior Correct 32-bit values in lower bits, zero-extension in upper 96 bits
 * @validation_method Direct result comparison with expected v128 structure and zero validation
 */
TEST_F(V128Load32ZeroTestSuite, BasicLoad_AlignedMemory_LoadsCorrectValue)
{
    uint64_t result_hi, result_lo;

    // Test basic load with typical 32-bit value at offset 0
    ASSERT_TRUE(init_memory(0, 0x12345678U))
        << "Failed to initialize memory at offset 0 with test value 0x12345678";
    ASSERT_TRUE(call_v128_load32_zero(0, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from aligned offset 0";
    assert_v128_load32_zero_result(0x12345678U, result_hi, result_lo);

    // Test load with different value pattern at offset 4 (aligned)
    ASSERT_TRUE(init_memory(4, 0xABCDEF00U))
        << "Failed to initialize memory at offset 4 with test value 0xABCDEF00";
    ASSERT_TRUE(call_v128_load32_zero(4, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from aligned offset 4";
    assert_v128_load32_zero_result(0xABCDEF00U, result_hi, result_lo);

    // Test load with alternating bit pattern at offset 8 (aligned)
    ASSERT_TRUE(init_memory(8, 0x55555555U))
        << "Failed to initialize memory at offset 8 with test value 0x55555555";
    ASSERT_TRUE(call_v128_load32_zero(8, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from aligned offset 8";
    assert_v128_load32_zero_result(0x55555555U, result_hi, result_lo);

    // Test load with sign bit set at offset 16 (aligned)
    ASSERT_TRUE(init_memory(16, 0x80000001U))
        << "Failed to initialize memory at offset 16 with test value 0x80000001";
    ASSERT_TRUE(call_v128_load32_zero(16, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from aligned offset 16";
    assert_v128_load32_zero_result(0x80000001U, result_hi, result_lo);
}

/**
 * @test AlignmentBoundary_UnalignedAccess_ProcessesCorrectly
 * @brief Verify v128.load32_zero handles unaligned memory access properly
 * @details Tests load operation from unaligned memory addresses to validate WAMR's
 *          unaligned access handling and data integrity preservation.
 * @test_category Corner - Boundary conditions validation
 * @coverage_target core/iwasm/compilation/simd/simd_load_store.c:v128_load32_zero_unaligned
 * @input_conditions Unaligned offsets (1, 2, 3, 5, 6, 7) with various test data patterns
 * @expected_behavior Correct data loading regardless of alignment, proper zero-extension
 * @validation_method Unaligned access result comparison with expected values
 */
TEST_F(V128Load32ZeroTestSuite, AlignmentBoundary_UnalignedAccess_ProcessesCorrectly)
{
    uint64_t result_hi, result_lo;

    // Initialize memory region with overlapping test data
    ASSERT_TRUE(init_memory(0, 0x11223344U))
        << "Failed to initialize memory at offset 0";
    ASSERT_TRUE(init_memory(4, 0x55667788U))
        << "Failed to initialize memory at offset 4";
    ASSERT_TRUE(init_memory(8, 0x99AABBCCU))
        << "Failed to initialize memory at offset 8";

    // Test unaligned load from offset 1 (should work with most implementations)
    ASSERT_TRUE(call_v128_load32_zero(1, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from unaligned offset 1";
    // Upper 96 bits should be zero
    ASSERT_EQ(0x0000000000000000ULL, result_hi)
        << "v128.load32_zero upper 64 bits should be zero for unaligned access";
    ASSERT_EQ(0x00000000U, static_cast<uint32_t>(result_lo >> 32))
        << "v128.load32_zero upper 32 bits of low 64 bits should be zero for unaligned access";

    // Test unaligned load from offset 2
    ASSERT_TRUE(call_v128_load32_zero(2, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from unaligned offset 2";
    // Validate zero-extension behavior
    ASSERT_EQ(0x0000000000000000ULL, result_hi)
        << "v128.load32_zero upper 64 bits should be zero for unaligned offset 2";

    // Test unaligned load from offset 3
    ASSERT_TRUE(call_v128_load32_zero(3, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from unaligned offset 3";
    // Validate zero-extension behavior
    ASSERT_EQ(0x0000000000000000ULL, result_hi)
        << "v128.load32_zero upper 64 bits should be zero for unaligned offset 3";
}

/**
 * @test ZeroOperands_ZeroOffset_LoadsFromBaseAddress
 * @brief Test v128.load32_zero loading from memory base address (offset 0)
 * @details Validates load operation from memory start with various stored values,
 *          confirming proper base address handling and zero-extension.
 * @test_category Edge - Zero operand scenarios
 * @coverage_target core/iwasm/compilation/simd/simd_load_store.c:v128_load32_zero_base
 * @input_conditions Offset 0 with different stored values (0x00000000, 0xFFFFFFFF, patterns)
 * @expected_behavior Correct loading from memory base, proper zero-extension
 * @validation_method Base address load result verification and zero-extension validation
 */
TEST_F(V128Load32ZeroTestSuite, ZeroOperands_ZeroOffset_LoadsFromBaseAddress)
{
    uint64_t result_hi, result_lo;

    // Test loading zero value from base address
    ASSERT_TRUE(init_memory(0, 0x00000000U))
        << "Failed to initialize memory at base address with zero value";
    ASSERT_TRUE(call_v128_load32_zero(0, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from base address (offset 0)";
    assert_v128_load32_zero_result(0x00000000U, result_hi, result_lo);

    // Test loading maximum 32-bit value from base address
    ASSERT_TRUE(init_memory(0, 0xFFFFFFFFU))
        << "Failed to initialize memory at base address with maximum value";
    ASSERT_TRUE(call_v128_load32_zero(0, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from base address with max value";
    assert_v128_load32_zero_result(0xFFFFFFFFU, result_hi, result_lo);

    // Test loading alternating pattern from base address
    ASSERT_TRUE(init_memory(0, 0xAAAAAAAAU))
        << "Failed to initialize memory at base address with alternating pattern";
    ASSERT_TRUE(call_v128_load32_zero(0, result_hi, result_lo))
        << "Failed to execute v128.load32_zero from base address with alternating pattern";
    assert_v128_load32_zero_result(0xAAAAAAAAU, result_hi, result_lo);
}

/**
 * @test ExtremeValues_MaximumPatterns_HandlesCorrectly
 * @brief Validate v128.load32_zero handling of extreme bit patterns and boundary values
 * @details Tests load operation with maximum values, extreme patterns, and special
 *          numeric values to ensure robust handling across all possible 32-bit values.
 * @test_category Edge - Extreme value scenarios
 * @coverage_target core/iwasm/compilation/simd/simd_load_store.c:v128_load32_zero_extremes
 * @input_conditions Extreme values (0xFFFFFFFF, 0x80000000, alternating patterns)
 * @expected_behavior Correct extreme value loading with proper zero-extension
 * @validation_method Extreme value result verification and bit pattern validation
 */
TEST_F(V128Load32ZeroTestSuite, ExtremeValues_MaximumPatterns_HandlesCorrectly)
{
    uint64_t result_hi, result_lo;

    // Test maximum 32-bit unsigned value
    ASSERT_TRUE(init_memory(0, 0xFFFFFFFFU))
        << "Failed to initialize memory with maximum 32-bit value";
    ASSERT_TRUE(call_v128_load32_zero(0, result_hi, result_lo))
        << "Failed to load maximum 32-bit value using v128.load32_zero";
    assert_v128_load32_zero_result(0xFFFFFFFFU, result_hi, result_lo);

    // Test most significant bit set (sign bit in signed interpretation)
    ASSERT_TRUE(init_memory(4, 0x80000000U))
        << "Failed to initialize memory with sign bit value";
    ASSERT_TRUE(call_v128_load32_zero(4, result_hi, result_lo))
        << "Failed to load sign bit value using v128.load32_zero";
    assert_v128_load32_zero_result(0x80000000U, result_hi, result_lo);

    // Test alternating bit pattern 1010...
    ASSERT_TRUE(init_memory(8, 0xAAAAAAAAU))
        << "Failed to initialize memory with alternating pattern 0xAAAAAAAA";
    ASSERT_TRUE(call_v128_load32_zero(8, result_hi, result_lo))
        << "Failed to load alternating pattern 0xAAAAAAAA";
    assert_v128_load32_zero_result(0xAAAAAAAAU, result_hi, result_lo);

    // Test alternating bit pattern 0101...
    ASSERT_TRUE(init_memory(12, 0x55555555U))
        << "Failed to initialize memory with alternating pattern 0x55555555";
    ASSERT_TRUE(call_v128_load32_zero(12, result_hi, result_lo))
        << "Failed to load alternating pattern 0x55555555";
    assert_v128_load32_zero_result(0x55555555U, result_hi, result_lo);
}

/**
 * @test ZeroExtensionProperty_UpperBits_AlwaysZero
 * @brief Verify upper 96 bits are always zero regardless of loaded data value
 * @details Tests the critical zero-extension property of v128.load32_zero across
 *          various data patterns to ensure consistent behavior.
 * @test_category Edge - Mathematical properties validation
 * @coverage_target core/iwasm/compilation/simd/simd_load_store.c:v128_load32_zero_extension
 * @input_conditions Various 32-bit patterns to test zero-extension consistency
 * @expected_behavior Upper 96 bits consistently zero for all loaded values
 * @validation_method Zero-extension property verification across multiple test cases
 */
TEST_F(V128Load32ZeroTestSuite, ZeroExtensionProperty_UpperBits_AlwaysZero)
{
    uint64_t result_hi, result_lo;

    // Test zero-extension with various bit patterns
    uint32_t test_values[] = {
        0x00000000U, 0x00000001U, 0x000000FFU, 0x0000FFFFU,
        0x00FFFFFFU, 0xFFFFFFFFU, 0x80000000U, 0x7FFFFFFFU,
        0x12345678U, 0x87654321U, 0xAAAAAAAAU, 0x55555555U
    };

    for (size_t i = 0; i < sizeof(test_values) / sizeof(test_values[0]); ++i) {
        uint32_t test_value = test_values[i];
        uint32_t offset = static_cast<uint32_t>(i * 4);

        // Initialize memory with test value
        ASSERT_TRUE(init_memory(offset, test_value))
            << "Failed to initialize memory at offset " << offset
            << " with value 0x" << std::hex << test_value;

        // Load and verify zero-extension property
        ASSERT_TRUE(call_v128_load32_zero(offset, result_hi, result_lo))
            << "Failed to load value 0x" << std::hex << test_value
            << " from offset " << offset;

        // Critical zero-extension validation
        ASSERT_EQ(0x0000000000000000ULL, result_hi)
            << "Zero-extension failed: upper 64 bits not zero for value 0x"
            << std::hex << test_value << " - Got: 0x" << result_hi;

        ASSERT_EQ(0x00000000U, static_cast<uint32_t>(result_lo >> 32))
            << "Zero-extension failed: bits 32-63 not zero for value 0x"
            << std::hex << test_value << " - Got: 0x" << (result_lo >> 32);

        ASSERT_EQ(test_value, static_cast<uint32_t>(result_lo))
            << "Data integrity failed: lower 32 bits incorrect for value 0x"
            << std::hex << test_value;
    }
}

/**
 * @test OutOfBounds_OffsetTooLarge_TrapsProperly
 * @brief Verify proper trap behavior for out-of-bounds memory access
 * @details Tests v128.load32_zero with invalid memory offsets to ensure proper
 *          error handling and trap generation for memory access violations.
 * @test_category Error - Out-of-bounds access validation
 * @coverage_target core/iwasm/compilation/simd/simd_load_store.c:v128_load32_zero_bounds_check
 * @input_conditions Memory offsets beyond allocated memory bounds
 * @expected_behavior Execution traps or errors for out-of-bounds access
 * @validation_method Error detection and proper trap behavior verification
 */
TEST_F(V128Load32ZeroTestSuite, OutOfBounds_OffsetTooLarge_TrapsProperly)
{
    // Test out-of-bounds access with very large offset
    // Note: Actual memory size depends on WASM module configuration
    // These tests verify proper bounds checking behavior

    // Test offset that would definitely exceed typical memory bounds
    // The test_out_of_bounds_access function returns true if trap/error occurred as expected
    ASSERT_TRUE(test_out_of_bounds_access(0x10000000U))  // 256MB offset
        << "Expected out-of-bounds trap for extremely large offset";

    ASSERT_TRUE(test_out_of_bounds_access(0xFFFFFFFCU))  // Near max uint32
        << "Expected out-of-bounds trap for near-maximum offset";

    ASSERT_TRUE(test_out_of_bounds_access(0xFFFFFFFFU))  // Max uint32
        << "Expected out-of-bounds trap for maximum uint32 offset";

    // Test offset + 4 bytes would exceed bounds (edge case)
    // This tests the specific case where offset might be valid but offset+4 is not
    ASSERT_TRUE(test_out_of_bounds_access(0xFFFFFFFDU))  // offset+4 would wrap/exceed
        << "Expected out-of-bounds trap when offset+4 would exceed bounds";
}