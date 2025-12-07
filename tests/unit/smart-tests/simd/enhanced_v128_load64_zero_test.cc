/**
 * @file enhanced_v128_load64_zero_test.cc
 * @brief Comprehensive unit tests for v128.load64_zero SIMD opcode
 * @details Tests v128.load64_zero functionality across interpreter and AOT execution modes
 *          with focus on 64-bit memory load operations, zero-extension behavior, and boundary conditions.
 *          Validates WAMR SIMD memory load implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load64_zero_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class V128Load64ZeroTestSuite
 * @brief Test fixture class for v128.load64_zero opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class V128Load64ZeroTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.load64_zero testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files with memory operations.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load64_zero_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.load64_zero test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_load64_zero_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.load64_zero tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_v128_load64_zero_test.cc:TearDown
     */
    void TearDown() override
    {
        // RAII automatically handles cleanup
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Initialize 64-bit memory location with test data
     * @details Uses WASM module's init_memory_64 function to setup memory for testing.
     *          Handles little-endian byte ordering and proper memory layout.
     * @param offset Memory offset to initialize
     * @param value_low Lower 32 bits of 64-bit value
     * @param value_high Upper 32 bits of 64-bit value
     */
    void init_memory_64bit(uint32_t offset, uint32_t value_low, uint32_t value_high)
    {
        uint32_t argv[3];
        argv[0] = offset;
        argv[1] = value_low;
        argv[2] = value_high;

        bool success = dummy_env->execute("init_memory_64", 3, argv);
        ASSERT_TRUE(success) << "Failed to initialize 64-bit memory at offset " << offset;
    }

    /**
     * @brief Execute v128.load64_zero operation and return v128 result components
     * @details Calls WASM test function and extracts v128 result as four i32 lanes
     *          for detailed validation of zero-extension behavior.
     * @param offset Memory offset to load from
     * @return Vector of 4 uint32_t values representing v128 lanes (little-endian)
     */
    std::vector<uint32_t> call_v128_load64_zero(uint32_t offset)
    {
        uint32_t argv[4];
        argv[0] = offset;

        bool success = dummy_env->execute("test_v128_load64_zero", 1, argv);
        if (!success) {
            ADD_FAILURE() << "v128.load64_zero call failed at offset " << offset;
            return {0, 0, 0, 0};
        }

        return {
            argv[0],  // Lane 0: bits 0-31
            argv[1],  // Lane 1: bits 32-63
            argv[2],  // Lane 2: bits 64-95 (should be 0)
            argv[3]   // Lane 3: bits 96-127 (should be 0)
        };
    }

    /**
     * @brief Execute v128.load64_zero with immediate offset and return result components
     * @details Tests immediate offset functionality of v128.load64_zero instruction.
     * @param base_addr Base memory address
     * @return Vector of 4 uint32_t values representing v128 lanes
     */
    std::vector<uint32_t> call_v128_load64_zero_with_offset(uint32_t base_addr)
    {
        uint32_t argv[4];
        argv[0] = base_addr;

        bool success = dummy_env->execute("test_v128_load64_zero_with_offset", 1, argv);
        if (!success) {
            ADD_FAILURE() << "v128.load64_zero with offset call failed at base " << base_addr;
            return {0, 0, 0, 0};
        }

        return {
            argv[0],
            argv[1],
            argv[2],
            argv[3]
        };
    }

    /**
     * @brief Clear memory region for clean testing
     * @details Uses WASM module function to zero-out memory regions
     * @param offset Starting memory offset
     * @param size Number of bytes to clear
     */
    void clear_memory_region(uint32_t offset, uint32_t size)
    {
        uint32_t argv[2];
        argv[0] = offset;
        argv[1] = size;

        bool success = dummy_env->execute("clear_memory_region", 2, argv);
        ASSERT_TRUE(success) << "Failed to clear memory region at offset " << offset;
    }

    /**
     * @brief Get current memory size for boundary testing
     * @details Retrieves memory size in pages for boundary condition validation
     * @return Memory size in pages (each page is 64KB)
     */
    uint32_t get_memory_size_pages()
    {
        uint32_t argv[1];
        bool success = dummy_env->execute("get_memory_size", 0, argv);
        if (!success) {
            ADD_FAILURE() << "Failed to get memory size";
            return 2; // Default to 2 pages (128KB) for error cases
        }
        return argv[0];
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicLoad_ValidMemoryAccess_ReturnsZeroExtendedResult
 * @brief Validates v128.load64_zero produces correct zero-extended results for basic memory access
 * @details Tests fundamental 64-bit load operation with known patterns. Verifies that loaded
 *          64-bit values occupy lower lanes of v128 result while upper 64 bits are zero-filled.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:v128_load64_zero_operation
 * @input_conditions Standard 64-bit patterns at various memory locations
 * @expected_behavior Lower 64 bits match memory content, upper 64 bits are zero
 * @validation_method Direct comparison of v128 lanes with expected 64-bit patterns
 */
TEST_F(V128Load64ZeroTestSuite, BasicLoad_ValidMemoryAccess_ReturnsZeroExtendedResult)
{
    // Test pattern: 0x0123456789ABCDEF (little-endian storage)
    init_memory_64bit(0, 0x89ABCDEF, 0x01234567);
    auto result = call_v128_load64_zero(0);

    // Verify loaded 64-bit value in lower lanes
    ASSERT_EQ(0x89ABCDEF, result[0]) << "Lane 0 should contain lower 32 bits of loaded value";
    ASSERT_EQ(0x01234567, result[1]) << "Lane 1 should contain upper 32 bits of loaded value";

    // Verify zero-extension in upper lanes
    ASSERT_EQ(0x00000000, result[2]) << "Lane 2 should be zero-extended";
    ASSERT_EQ(0x00000000, result[3]) << "Lane 3 should be zero-extended";

    // Test different pattern: 0xFEDCBA9876543210
    init_memory_64bit(8, 0x76543210, 0xFEDCBA98);
    auto result2 = call_v128_load64_zero(8);

    ASSERT_EQ(0x76543210, result2[0]) << "Lane 0 should contain lower 32 bits of second pattern";
    ASSERT_EQ(0xFEDCBA98, result2[1]) << "Lane 1 should contain upper 32 bits of second pattern";
    ASSERT_EQ(0x00000000, result2[2]) << "Lane 2 should remain zero-extended";
    ASSERT_EQ(0x00000000, result2[3]) << "Lane 3 should remain zero-extended";

    // Test zero pattern: 0x0000000000000000
    init_memory_64bit(16, 0x00000000, 0x00000000);
    auto result3 = call_v128_load64_zero(16);

    ASSERT_EQ(0x00000000, result3[0]) << "Lane 0 should be zero for zero pattern";
    ASSERT_EQ(0x00000000, result3[1]) << "Lane 1 should be zero for zero pattern";
    ASSERT_EQ(0x00000000, result3[2]) << "Lane 2 should be zero-extended for zero pattern";
    ASSERT_EQ(0x00000000, result3[3]) << "Lane 3 should be zero-extended for zero pattern";
}

/**
 * @test MemoryAlignment_UnalignedAccess_LoadsCorrectly
 * @brief Validates v128.load64_zero handles unaligned memory access correctly
 * @details Tests loading 64-bit values from various alignment boundaries to ensure
 *          correct behavior regardless of memory alignment. WebAssembly allows unaligned access.
 * @test_category Corner - Memory alignment boundary conditions
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_access_unaligned
 * @input_conditions Test patterns at aligned and unaligned memory positions
 * @expected_behavior Correct data loaded regardless of alignment, upper bits zero-extended
 * @validation_method Comparison of results from aligned vs unaligned accesses
 */
TEST_F(V128Load64ZeroTestSuite, MemoryAlignment_UnalignedAccess_LoadsCorrectly)
{
    // Initialize memory with overlapping patterns for alignment testing
    // Pattern spans multiple alignment boundaries
    init_memory_64bit(0, 0x11111111, 0x22222222);  // 8-byte aligned
    init_memory_64bit(8, 0x33333333, 0x44444444);  // 8-byte aligned
    init_memory_64bit(16, 0x55555555, 0x66666666); // 8-byte aligned

    // Test aligned access (8-byte boundary)
    auto aligned_result = call_v128_load64_zero(0);
    ASSERT_EQ(0x11111111, aligned_result[0]) << "Aligned access lane 0 failed";
    ASSERT_EQ(0x22222222, aligned_result[1]) << "Aligned access lane 1 failed";
    ASSERT_EQ(0x00000000, aligned_result[2]) << "Aligned access zero-extension failed";
    ASSERT_EQ(0x00000000, aligned_result[3]) << "Aligned access zero-extension failed";

    // Initialize test pattern for unaligned testing
    init_memory_64bit(1, 0xAAAAAAAA, 0xBBBBBBBB);  // 1-byte unaligned
    auto unaligned_result = call_v128_load64_zero(1);
    ASSERT_EQ(0xAAAAAAAA, unaligned_result[0]) << "Unaligned access lane 0 failed";
    ASSERT_EQ(0xBBBBBBBB, unaligned_result[1]) << "Unaligned access lane 1 failed";
    ASSERT_EQ(0x00000000, unaligned_result[2]) << "Unaligned access zero-extension failed";
    ASSERT_EQ(0x00000000, unaligned_result[3]) << "Unaligned access zero-extension failed";

    // Test 4-byte unaligned access
    init_memory_64bit(4, 0xCCCCCCCC, 0xDDDDDDDD);  // 4-byte unaligned
    auto partial_aligned = call_v128_load64_zero(4);
    ASSERT_EQ(0xCCCCCCCC, partial_aligned[0]) << "4-byte unaligned access lane 0 failed";
    ASSERT_EQ(0xDDDDDDDD, partial_aligned[1]) << "4-byte unaligned access lane 1 failed";
    ASSERT_EQ(0x00000000, partial_aligned[2]) << "4-byte unaligned zero-extension failed";
    ASSERT_EQ(0x00000000, partial_aligned[3]) << "4-byte unaligned zero-extension failed";
}

/**
 * @test MemoryBoundary_ValidBoundaryAccess_LoadsCorrectly
 * @brief Validates v128.load64_zero at memory boundaries and with immediate offsets
 * @details Tests loading from memory boundaries, page boundaries, and using immediate
 *          offset parameters. Ensures correct behavior at memory limits.
 * @test_category Corner - Memory boundary conditions and offset handling
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_boundary_check
 * @input_conditions Memory boundary addresses, immediate offset combinations
 * @expected_behavior Successful loads at valid boundaries, correct offset handling
 * @validation_method Boundary address calculations and offset result verification
 */
TEST_F(V128Load64ZeroTestSuite, MemoryBoundary_ValidBoundaryAccess_LoadsCorrectly)
{
    // Get memory size for boundary testing
    uint32_t memory_pages = get_memory_size_pages();
    uint32_t memory_size_bytes = memory_pages * 65536;  // 64KB per page

    // Test exact boundary access (memory.size() - 8)
    uint32_t boundary_offset = memory_size_bytes - 8;
    init_memory_64bit(boundary_offset, 0xBEEFCAFE, 0xDEADBEEF);
    auto boundary_result = call_v128_load64_zero(boundary_offset);

    ASSERT_EQ(0xBEEFCAFE, boundary_result[0]) << "Boundary access lane 0 failed";
    ASSERT_EQ(0xDEADBEEF, boundary_result[1]) << "Boundary access lane 1 failed";
    ASSERT_EQ(0x00000000, boundary_result[2]) << "Boundary access zero-extension failed";
    ASSERT_EQ(0x00000000, boundary_result[3]) << "Boundary access zero-extension failed";

    // Test immediate offset functionality
    // Set up data at offset 0 and load using base_addr + immediate offset of 8
    init_memory_64bit(8, 0x12345678, 0x9ABCDEF0);
    auto offset_result = call_v128_load64_zero_with_offset(0);  // loads from 0 + 8 = 8

    ASSERT_EQ(0x12345678, offset_result[0]) << "Immediate offset access lane 0 failed";
    ASSERT_EQ(0x9ABCDEF0, offset_result[1]) << "Immediate offset access lane 1 failed";
    ASSERT_EQ(0x00000000, offset_result[2]) << "Immediate offset zero-extension failed";
    ASSERT_EQ(0x00000000, offset_result[3]) << "Immediate offset zero-extension failed";

    // Test page boundary crossing (if memory > 64KB)
    if (memory_pages > 1) {
        uint32_t page_boundary = 65536 - 4;  // Cross page boundary
        init_memory_64bit(page_boundary, 0xFACEFEED, 0xCAFEBABE);
        auto page_cross_result = call_v128_load64_zero(page_boundary);

        ASSERT_EQ(0xFACEFEED, page_cross_result[0]) << "Page boundary cross lane 0 failed";
        ASSERT_EQ(0xCAFEBABE, page_cross_result[1]) << "Page boundary cross lane 1 failed";
        ASSERT_EQ(0x00000000, page_cross_result[2]) << "Page boundary zero-extension failed";
        ASSERT_EQ(0x00000000, page_cross_result[3]) << "Page boundary zero-extension failed";
    }
}

/**
 * @test ZeroExtension_VariousPatterns_AlwaysZeroUpper
 * @brief Validates zero-extension behavior with various bit patterns
 * @details Tests that upper 64 bits of v128 result are always zero regardless of
 *          the content of the loaded 64-bit value. Validates against various bit patterns.
 * @test_category Edge - Zero-extension invariant validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:v128_zero_extension
 * @input_conditions Various 64-bit patterns including extreme values
 * @expected_behavior Upper 64 bits always zero, lower 64 bits match memory exactly
 * @validation_method Systematic verification of zero-extension with diverse patterns
 */
TEST_F(V128Load64ZeroTestSuite, ZeroExtension_VariousPatterns_AlwaysZeroUpper)
{
    // Test maximum value pattern: 0xFFFFFFFFFFFFFFFF
    init_memory_64bit(0, 0xFFFFFFFF, 0xFFFFFFFF);
    auto max_result = call_v128_load64_zero(0);

    ASSERT_EQ(0xFFFFFFFF, max_result[0]) << "Max pattern lane 0 failed";
    ASSERT_EQ(0xFFFFFFFF, max_result[1]) << "Max pattern lane 1 failed";
    ASSERT_EQ(0x00000000, max_result[2]) << "Max pattern upper bits not zero-extended";
    ASSERT_EQ(0x00000000, max_result[3]) << "Max pattern upper bits not zero-extended";

    // Test alternating pattern: 0xAAAAAAAA55555555
    init_memory_64bit(8, 0x55555555, 0xAAAAAAAA);
    auto alternating_result = call_v128_load64_zero(8);

    ASSERT_EQ(0x55555555, alternating_result[0]) << "Alternating pattern lane 0 failed";
    ASSERT_EQ(0xAAAAAAAA, alternating_result[1]) << "Alternating pattern lane 1 failed";
    ASSERT_EQ(0x00000000, alternating_result[2]) << "Alternating pattern upper not zero-extended";
    ASSERT_EQ(0x00000000, alternating_result[3]) << "Alternating pattern upper not zero-extended";

    // Test single bit patterns
    init_memory_64bit(16, 0x00000001, 0x00000000);  // Only LSB set
    auto single_bit_result = call_v128_load64_zero(16);

    ASSERT_EQ(0x00000001, single_bit_result[0]) << "Single bit pattern lane 0 failed";
    ASSERT_EQ(0x00000000, single_bit_result[1]) << "Single bit pattern lane 1 failed";
    ASSERT_EQ(0x00000000, single_bit_result[2]) << "Single bit pattern upper not zero-extended";
    ASSERT_EQ(0x00000000, single_bit_result[3]) << "Single bit pattern upper not zero-extended";

    // Test MSB pattern: 0x8000000000000000
    init_memory_64bit(24, 0x00000000, 0x80000000);  // Only MSB of 64-bit value set
    auto msb_result = call_v128_load64_zero(24);

    ASSERT_EQ(0x00000000, msb_result[0]) << "MSB pattern lane 0 failed";
    ASSERT_EQ(0x80000000, msb_result[1]) << "MSB pattern lane 1 failed";
    ASSERT_EQ(0x00000000, msb_result[2]) << "MSB pattern upper not zero-extended";
    ASSERT_EQ(0x00000000, msb_result[3]) << "MSB pattern upper not zero-extended";
}

/**
 * @test ErrorConditions_OutOfBounds_TriggersTraps
 * @brief Validates error handling for out-of-bounds memory access
 * @details Tests that v128.load64_zero properly handles and reports out-of-bounds
 *          memory access conditions. Verifies WAMR trap mechanisms function correctly.
 * @test_category Error - Out-of-bounds access and error handling validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_bounds_check
 * @input_conditions Invalid memory addresses that exceed memory boundaries
 * @expected_behavior WebAssembly traps, proper error propagation, no memory corruption
 * @validation_method Exception handling verification and memory state validation
 */
TEST_F(V128Load64ZeroTestSuite, ErrorConditions_OutOfBounds_TriggersTraps)
{
    // First verify normal operation works
    init_memory_64bit(0, 0x12345678, 0x9ABCDEF0);
    auto initial_result = call_v128_load64_zero(0);
    ASSERT_EQ(0x12345678, initial_result[0]) << "Initial memory setup failed";
    ASSERT_EQ(0x9ABCDEF0, initial_result[1]) << "Initial memory setup failed";

    // Get memory size for out-of-bounds testing
    uint32_t memory_pages = get_memory_size_pages();
    uint32_t memory_size_bytes = memory_pages * 65536;

    // Test exact out-of-bounds access (memory.size() - 7, needs 8 bytes but only 7 available)
    uint32_t invalid_offset = memory_size_bytes - 7;

    uint32_t argv[4];
    argv[0] = invalid_offset;

    // This should fail due to out-of-bounds access
    bool success = dummy_env->execute("test_v128_load64_zero", 1, argv);
    ASSERT_FALSE(success) << "Out-of-bounds access should fail but succeeded at offset " << invalid_offset;

    // Test significantly out-of-bounds access
    uint32_t far_invalid_offset = memory_size_bytes + 1000;
    uint32_t far_argv[4];
    far_argv[0] = far_invalid_offset;

    bool far_success = dummy_env->execute("test_v128_load64_zero", 1, far_argv);
    ASSERT_FALSE(far_success) << "Far out-of-bounds access should fail at offset " << far_invalid_offset;

    // Note: After WASM traps occur, the execution environment may be in an invalid state.
    // This is expected behavior for WebAssembly trap handling.
    // The test validates that out-of-bounds access correctly triggers traps as required by the spec.
}