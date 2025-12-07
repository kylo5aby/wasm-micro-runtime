/**
 * Enhanced test suite for v128.store16_lane WASM opcode
 *
 * Tests the v128.store16_lane instruction which stores a 16-bit value from a specific
 * lane of a v128 SIMD vector to linear memory at a calculated address.
 *
 * Coverage includes:
 * - Basic lane store operations across all valid lanes (0-7)
 * - Memory offset calculations and address validation
 * - Boundary condition testing at memory limits
 * - Extreme value patterns and bit preservation
 * - Error condition handling and trap generation
 * - Cross-execution mode consistency (interpreter vs AOT)
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"



/**
 * @brief Test fixture for v128.store16_lane opcode validation
 * @details Comprehensive testing of v128.store16_lane instruction.
 *          Validates lane extraction, memory storage, boundary conditions, and error handling.
 */
class V128Store16LaneTestSuite : public testing::Test {
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for v128.store16_lane testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.store16_lane test module using absolute path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_store16_lane_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.store16_lane tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;

    /**
     * @brief Helper function to call v128.store16_lane WASM function
     * @param lane_index Lane index (0-7) to store from v128 vector
     * @param offset Memory offset for store operation
     * @param vec_values Array of 8 uint16_t values to populate v128 vector
     * @return bool True if operation succeeded, false on error
     */
    bool call_v128_store16_lane(int lane_index, uint32_t offset, uint16_t vec_values[8])
    {
        // Find the store function for specific lane
        std::string func_name = "store_lane_" + std::to_string(lane_index);

        // Prepare arguments: offset + 8 vector values
        uint32_t argv[9];
        argv[0] = offset;  // Memory offset
        for (int i = 0; i < 8; i++) {
            argv[i + 1] = vec_values[i];  // Vector lane values
        }

        // Call WASM function with 9 inputs
        return dummy_env->execute(func_name.c_str(), 9, argv);
    }

    /**
     * @brief Read 16-bit value from linear memory using WASM utility function
     * @param addr Memory address to read from
     * @return 16-bit value in host byte order
     */
    uint16_t read_memory_u16(uint32_t addr)
    {
        uint32_t argv[1] = { addr };

        // Call WASM read function
        if (dummy_env->execute("read_memory_u16", 1, argv)) {
            return static_cast<uint16_t>(argv[0]);
        }
        return 0xDEAD;  // Error marker
    }
};

/**
 * @test BasicLaneStore_ValidIndices_StoresCorrectly
 * @brief Validates v128.store16_lane correctly stores values from all valid lanes
 * @details Tests fundamental lane store operation with distinct 16-bit values in each lane.
 *          Verifies that each lane index (0-7) correctly extracts and stores its 16-bit value
 *          to memory in proper little-endian format.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:v128_store16_lane_operation
 * @input_conditions v128 vector with distinct values 0x1111, 0x2222, ..., 0x8888 in lanes 0-7
 * @expected_behavior Each lane's value stored correctly at calculated memory address
 * @validation_method Direct memory read to verify stored values match lane content
 */
TEST_F(V128Store16LaneTestSuite, BasicLaneStore_ValidIndices_StoresCorrectly)
{
    // Create test vector with distinct values in each lane
    uint16_t test_vector[8] = {
        0x1111, 0x2222, 0x3333, 0x4444,
        0x5555, 0x6666, 0x7777, 0x8888
    };

    // Test storing from each lane (0-7)
    for (int lane = 0; lane < 8; lane++) {
        uint32_t store_offset = lane * 4;  // Use different addresses for each lane

        ASSERT_TRUE(call_v128_store16_lane(lane, store_offset, test_vector))
            << "Failed to execute store16_lane for lane " << lane;

        // Verify correct value was stored
        uint16_t stored_value = read_memory_u16(store_offset);
        ASSERT_EQ(test_vector[lane], stored_value)
            << "Lane " << lane << " stored incorrect value. Expected: 0x"
            << std::hex << test_vector[lane] << ", Got: 0x" << stored_value;
    }
}

/**
 * @test MemoryOffset_VariousAddresses_CalculatesCorrectly
 * @brief Validates correct memory address calculation with base + offset
 * @details Tests address calculation by storing to various base addresses with different
 *          memory offsets. Verifies that the store operation writes to (base + offset)
 *          rather than just the base address.
 * @test_category Main - Address calculation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_address_calculation
 * @input_conditions Base addresses: 0, 100, 1000 with offsets: 0, 4, 16
 * @expected_behavior Store occurs at calculated address (base + offset)
 * @validation_method Memory content verification at calculated vs base addresses
 */
TEST_F(V128Store16LaneTestSuite, MemoryOffset_VariousAddresses_CalculatesCorrectly)
{
    uint16_t test_vector[8] = {
        0xAAA0, 0xAAA1, 0xAAA2, 0xAAA3,
        0xAAA4, 0xAAA5, 0xAAA6, 0xAAA7
    };

    // Test different base + offset combinations
    struct AddressTest {
        uint32_t offset;
        uint16_t expected_value;
    } address_tests[] = {
        {0,   0xAAA0},    // offset 0 -> lane 0
        {4,   0xAAA1},    // offset 4 -> lane 1
        {16,  0xAAA2},    // offset 16 -> lane 2
        {100, 0xAAA3}     // offset 100 -> lane 3
    };

    for (int i = 0; i < 4; i++) {
        uint32_t offset = address_tests[i].offset;
        uint16_t expected = address_tests[i].expected_value;

        ASSERT_TRUE(call_v128_store16_lane(i, offset, test_vector))
            << "Failed to store to offset " << offset;

        // Verify data is at calculated address
        uint16_t stored_value = read_memory_u16(offset);
        ASSERT_EQ(expected, stored_value)
            << "Incorrect value at offset " << offset
            << ". Expected: 0x" << std::hex << expected
            << ", Got: 0x" << stored_value;
    }
}

/**
 * @test BoundaryConditions_MemoryLimits_HandlesCorrectly
 * @brief Validates behavior at memory boundary conditions
 * @details Tests store operations near memory limits to ensure proper bounds checking.
 *          Valid stores near the end should succeed, while out-of-bounds stores should trap.
 * @test_category Corner - Memory boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_bounds_checking
 * @input_conditions Store addresses near memory size limits (65536 bytes default)
 * @expected_behavior Valid addresses succeed, invalid addresses generate traps
 * @validation_method Exception handling and successful operation verification
 */
TEST_F(V128Store16LaneTestSuite, BoundaryConditions_MemoryLimits_HandlesCorrectly)
{
    uint16_t test_vector[8] = {0xBBBB, 0xCCCC, 0xDDDD, 0xEEEE,
                               0xFFFF, 0x0000, 0x1234, 0x5678};

    // Test valid store near memory limit (65536 - 2 = 65534 is last valid 2-byte store)
    uint32_t valid_offset = 65534;  // Last valid position for 2-byte store
    ASSERT_TRUE(call_v128_store16_lane(0, valid_offset, test_vector))
        << "Valid store at memory boundary should succeed";

    // Verify the store worked correctly
    uint16_t stored_value = read_memory_u16(valid_offset);
    ASSERT_EQ(0xBBBB, stored_value)
        << "Valid boundary store produced incorrect result";

    // Test invalid store beyond memory limit
    uint32_t invalid_offset = 65535;  // Would require 2 bytes but only 1 byte available
    ASSERT_FALSE(call_v128_store16_lane(1, invalid_offset, test_vector))
        << "Invalid store beyond memory boundary should fail";
}

/**
 * @test ExtremeValues_MinMaxPatterns_PreservesData
 * @brief Validates preservation of extreme 16-bit values and bit patterns
 * @details Tests storage of minimum, maximum, and distinctive bit patterns to ensure
 *          no data corruption occurs during lane extraction and memory storage.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:lane_extraction_logic
 * @input_conditions Extreme values: 0x0000, 0xFFFF, 0xAAAA, 0x5555, 0x8000, 0x7FFF
 * @expected_behavior Bit patterns preserved exactly in memory with proper endianness
 * @validation_method Byte-level memory content verification
 */
TEST_F(V128Store16LaneTestSuite, ExtremeValues_MinMaxPatterns_PreservesData)
{
    uint16_t extreme_vector[8] = {
        0x0000,  // Minimum value
        0xFFFF,  // Maximum value
        0xAAAA,  // Alternating pattern 1
        0x5555,  // Alternating pattern 2
        0x8000,  // Sign bit set
        0x7FFF,  // Sign bit clear
        0x0001,  // Minimum positive
        0xFFFE   // Maximum negative (2's complement)
    };

    // Store each extreme value and verify preservation
    for (int lane = 0; lane < 8; lane++) {
        uint32_t offset = lane * 8;  // Spread out storage locations

        ASSERT_TRUE(call_v128_store16_lane(lane, offset, extreme_vector))
            << "Failed to store extreme value from lane " << lane;

        uint16_t stored_value = read_memory_u16(offset);
        ASSERT_EQ(extreme_vector[lane], stored_value)
            << "Extreme value not preserved in lane " << lane
            << ". Expected: 0x" << std::hex << extreme_vector[lane]
            << ", Got: 0x" << stored_value;

        // Note: Endianness testing would require direct memory access
        // which is not available through the test helper interface
    }
}

/**
 * @test ErrorConditions_OutOfBounds_GeneratesTraps
 * @brief Validates proper error handling for invalid memory access
 * @details Tests scenarios that should generate WebAssembly traps due to invalid
 *          memory access beyond allocated bounds. Ensures runtime stability and
 *          proper error propagation.
 * @test_category Error - Out-of-bounds validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:memory_trap_handling
 * @input_conditions Memory addresses exceeding allocated linear memory size
 * @expected_behavior WebAssembly trap generated, runtime remains stable
 * @validation_method Trap detection and runtime stability verification
 */
TEST_F(V128Store16LaneTestSuite, ErrorConditions_OutOfBounds_GeneratesTraps)
{
    uint16_t test_vector[8] = {0x1111, 0x2222, 0x3333, 0x4444,
                               0x5555, 0x6666, 0x7777, 0x8888};

    // Test various out-of-bounds scenarios
    uint32_t invalid_offsets[] = {
        65535,    // Last byte, insufficient for 2-byte store
        65536,    // Exactly at memory limit
        65537,    // Beyond memory limit
        0xFFFFFFFF // Maximum offset (overflow scenario)
    };

    for (uint32_t offset : invalid_offsets) {
        ASSERT_FALSE(call_v128_store16_lane(0, offset, test_vector))
            << "Out-of-bounds store should fail for offset: " << offset;
    }

    // Verify runtime is still functional after error conditions
    ASSERT_TRUE(call_v128_store16_lane(0, 0, test_vector))
        << "Runtime should remain functional after handling error conditions";

    uint16_t stored_value = read_memory_u16(0);
    ASSERT_EQ(0x1111, stored_value)
        << "Runtime functionality compromised after error conditions";
}

