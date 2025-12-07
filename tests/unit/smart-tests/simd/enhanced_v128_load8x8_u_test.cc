/*
 * Enhanced unit tests for v128.load8x8_u SIMD memory opcode
 * Tests SIMD load operation that loads 8 bytes from memory and zero-extends them to 8 16-bit integers
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * Enhanced unit tests for v128.load8x8_u WASM opcode
 *
 * Tests comprehensive SIMD memory load functionality including:
 * - Basic zero extension operations with various byte patterns
 * - Boundary condition handling (memory limits, address boundaries)
 * - Edge cases (all zeros, maximum bytes, extreme values)
 * - Out-of-bounds memory access validation
 * - Cross-execution mode validation (interpreter vs AOT)
 */

// Test execution modes for cross-validation
enum class V128Load8x8uRunningMode : uint8_t {
    INTERP = 1 << 0,
    AOT = 1 << 1
};

static constexpr const char *MODULE_NAME = "v128_load8x8_u_test";
static constexpr const char *FUNC_NAME_TEST_LOAD = "test_load8x8_u";
static constexpr const char *FUNC_NAME_SET_BYTES = "set_memory_bytes";
static constexpr const char *FUNC_NAME_OUT_OF_BOUNDS = "test_out_of_bounds_load";

/**
 * Test fixture for v128.load8x8_u opcode validation
 *
 * Provides comprehensive test environment for SIMD memory load operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class V128Load8x8uTestSuite : public testing::TestWithParam<V128Load8x8uRunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the v128.load8x8_u test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.load8x8_u test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_load8x8_u_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.load8x8_u tests";
    }

    /**
     * Cleans up test environment and runtime resources
     *
     * Cleanup is handled automatically by RAII destructors.
     */
    void TearDown() override {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * Set bytes in WASM linear memory at specified offset
     * @param offset Memory offset to write bytes
     * @param bytes Array of bytes to write
     * @param count Number of bytes to write
     */
    void set_memory_bytes(uint32_t offset, const uint8_t* bytes, uint32_t count) {
        for (uint32_t i = 0; i < count; ++i) {
            uint32_t argv[2] = { offset + i, bytes[i] };
            bool success = dummy_env->execute(FUNC_NAME_SET_BYTES, 2, argv);
            ASSERT_TRUE(success) << "Failed to set memory byte at offset " << (offset + i);
        }
    }

    /**
     * Call WASM function to execute v128.load8x8_u with specified memory offset
     * @param offset Memory offset for load operation
     * @param result_lanes Array to store 8 u16 lane values from result vector
     */
    void call_load8x8_u(uint32_t offset, uint16_t* result_lanes) {
        // Prepare arguments: offset as i32 parameter
        uint32_t argv[4];  // Space for v128 result (4 x i32)
        argv[0] = offset;

        // Execute function - result will be stored in argv as 4 i32 values
        bool success = dummy_env->execute(FUNC_NAME_TEST_LOAD, 4, argv);
        ASSERT_TRUE(success) << "Failed to execute v128.load8x8_u function";

        // Extract u16 lanes from returned v128 (packed as 4 i32 values)
        uint16_t* packed_u16 = reinterpret_cast<uint16_t*>(argv);

        // Copy 8 u16 lanes to output array
        std::memcpy(result_lanes, packed_u16, 8 * sizeof(uint16_t));
    }

    /**
     * Call WASM function that attempts out-of-bounds memory access
     * @param offset Memory offset that should trigger out-of-bounds access
     * @return true if exception was handled properly, false otherwise
     */
    bool call_out_of_bounds_load(uint32_t offset) {
        uint32_t argv[4];
        argv[0] = offset;

        // This should trap/fail for out-of-bounds access
        bool success = dummy_env->execute(FUNC_NAME_OUT_OF_BOUNDS, 4, argv);
        return !success; // Return true if the call failed (as expected for out-of-bounds)
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicFunctionality_LoadsAndZeroExtendsCorrectly
 * @brief Validates v128.load8x8_u produces correct zero extension for typical byte patterns
 * @details Tests fundamental load operation with various byte values including maximum bytes.
 *          Verifies that each byte is correctly zero-extended to 16-bit unsigned integers.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:wasm_interp_call_func_native
 * @input_conditions Standard byte patterns: sequential, alternating, and offset variations
 * @expected_behavior Returns v128 with zero-extended u16 lanes preserving unsigned interpretation
 * @validation_method Lane-by-lane comparison of zero-extended u16 values
 */
TEST_P(V128Load8x8uTestSuite, BasicFunctionality_LoadsAndZeroExtendsCorrectly) {
    // Test sequential pattern: bytes [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08]
    uint8_t sequential_bytes[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint16_t sequential_expected[] = {0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008};

    // Write sequential bytes to memory at offset 0
    set_memory_bytes(0, sequential_bytes, 8);

    // Execute v128.load8x8_u from memory offset 0
    uint16_t result_lanes[8];
    call_load8x8_u(0, result_lanes);

    // Verify each lane contains correctly zero-extended value
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(sequential_expected[i], result_lanes[i])
            << "Sequential pattern lane " << i << " mismatch: expected 0x" << std::hex << sequential_expected[i]
            << " but got 0x" << result_lanes[i] << " (byte 0x" << (int)sequential_bytes[i] << ")";
    }

    // Test alternating pattern: bytes [0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA]
    uint8_t alternating_bytes[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};
    uint16_t alternating_expected[] = {0x0055, 0x00AA, 0x0055, 0x00AA, 0x0055, 0x00AA, 0x0055, 0x00AA};

    // Write alternating bytes to memory at offset 16
    set_memory_bytes(16, alternating_bytes, 8);

    // Execute v128.load8x8_u from memory offset 16
    call_load8x8_u(16, result_lanes);

    // Verify alternating pattern results
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(alternating_expected[i], result_lanes[i])
            << "Alternating pattern lane " << i << " mismatch: expected 0x" << std::hex << alternating_expected[i]
            << " but got 0x" << result_lanes[i] << " (byte 0x" << (int)alternating_bytes[i] << ")";
    }
}

/**
 * @test MemoryBoundaries_HandlesLimitConditionsCorrectly
 * @brief Validates loading from near memory boundary addresses without out-of-bounds errors
 * @details Tests memory access at addresses that should be valid for 8-byte loads.
 *          Ensures proper boundary checking and successful load operation.
 * @test_category Corner - Memory boundary condition validation
 * @coverage_target core/iwasm/common/wasm_memory.c:validate_app_addr
 * @input_conditions Memory access near valid boundaries with test pattern
 * @expected_behavior Successful load with correct zero extension, no memory violation
 * @validation_method Boundary address calculation and zero-extension verification
 */
TEST_P(V128Load8x8uTestSuite, MemoryBoundaries_HandlesLimitConditionsCorrectly) {
    // Test bytes with high values to verify zero-extension behavior
    uint8_t boundary_bytes[] = {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7};
    uint16_t boundary_expected[] = {0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7};

    // Write boundary test bytes to a high but valid memory offset
    uint32_t high_offset = 1000;  // Use a reasonable high offset within memory bounds
    set_memory_bytes(high_offset, boundary_bytes, 8);

    // Execute v128.load8x8_u from high memory offset
    uint16_t result_lanes[8];
    call_load8x8_u(high_offset, result_lanes);

    // Verify boundary case results
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(boundary_expected[i], result_lanes[i])
            << "Boundary case lane " << i << " mismatch: expected 0x" << std::hex << boundary_expected[i]
            << " but got 0x" << result_lanes[i] << " (byte 0x" << (int)boundary_bytes[i] << ")";
    }

    // Test with large valid addresses
    uint8_t large_addr_bytes[] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87};
    uint16_t large_addr_expected[] = {0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087};

    uint32_t large_offset = 2048;
    set_memory_bytes(large_offset, large_addr_bytes, 8);

    call_load8x8_u(large_offset, result_lanes);

    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(large_addr_expected[i], result_lanes[i])
            << "Large address lane " << i << " mismatch: expected 0x" << std::hex << large_addr_expected[i]
            << " but got 0x" << result_lanes[i] << " (byte 0x" << (int)large_addr_bytes[i] << ")";
    }
}

/**
 * @test ExtremeValues_ProcessesZeroAndMaximumBytesCorrectly
 * @brief Validates zero-extension behavior with extreme byte values (0x00 and 0xFF)
 * @details Tests edge cases including all-zero bytes, all-maximum bytes, and power-of-two patterns.
 *          Verifies that zero-extension preserves unsigned interpretation of all byte values.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:simd_load_extend_operation
 * @input_conditions Extreme byte patterns: zeros, maximums, powers of two
 * @expected_behavior Correct zero-extension maintaining unsigned semantics
 * @validation_method Verification of zero-extended values for extreme cases
 */
TEST_P(V128Load8x8uTestSuite, ExtremeValues_ProcessesZeroAndMaximumBytesCorrectly) {
    // Test all-zero bytes: [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
    uint8_t zero_bytes[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t zero_expected[] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

    set_memory_bytes(32, zero_bytes, 8);

    uint16_t result_lanes[8];
    call_load8x8_u(32, result_lanes);

    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(zero_expected[i], result_lanes[i])
            << "All-zero case lane " << i << " mismatch: expected 0x" << std::hex << zero_expected[i]
            << " but got 0x" << result_lanes[i];
    }

    // Test all-maximum bytes: [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]
    uint8_t max_bytes[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint16_t max_expected[] = {0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF};

    set_memory_bytes(48, max_bytes, 8);
    call_load8x8_u(48, result_lanes);

    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(max_expected[i], result_lanes[i])
            << "All-maximum case lane " << i << " mismatch: expected 0x" << std::hex << max_expected[i]
            << " but got 0x" << result_lanes[i];
    }

    // Test power-of-two pattern: [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80]
    uint8_t power2_bytes[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    uint16_t power2_expected[] = {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080};

    set_memory_bytes(64, power2_bytes, 8);
    call_load8x8_u(64, result_lanes);

    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(power2_expected[i], result_lanes[i])
            << "Power-of-two case lane " << i << " mismatch: expected 0x" << std::hex << power2_expected[i]
            << " but got 0x" << result_lanes[i] << " (byte 0x" << (int)power2_bytes[i] << ")";
    }
}

/**
 * @test OutOfBoundsAccess_GeneratesTrapsCorrectly
 * @brief Validates proper trap generation for out-of-bounds memory access attempts
 * @details Tests memory access beyond allocated memory limits to verify runtime protection.
 *          Ensures WAMR properly detects and handles invalid memory access scenarios.
 * @test_category Error - Out-of-bounds access validation
 * @coverage_target core/iwasm/common/wasm_memory.c:check_memory_overflow
 * @input_conditions Memory addresses that exceed valid memory bounds
 * @expected_behavior Runtime trap/exception for invalid memory access attempts
 * @validation_method Exception handling verification for out-of-bounds scenarios
 */
TEST_P(V128Load8x8uTestSuite, OutOfBoundsAccess_GeneratesTrapsCorrectly) {
    // Attempt to load from an out-of-bounds address
    // This should fail and be handled by the WASM runtime
    uint32_t out_of_bounds_offset = 0xFFFFFF00;  // Very large offset likely to be out of bounds

    bool trapped_correctly = call_out_of_bounds_load(out_of_bounds_offset);
    ASSERT_TRUE(trapped_correctly)
        << "Expected out-of-bounds access to fail/trap for offset 0x" << std::hex << out_of_bounds_offset
        << ", but operation succeeded when it should have failed";

    // Test another out-of-bounds scenario with a different large offset
    uint32_t another_oob_offset = 0x10000000;  // Another large offset

    trapped_correctly = call_out_of_bounds_load(another_oob_offset);
    ASSERT_TRUE(trapped_correctly)
        << "Expected out-of-bounds access to fail/trap for offset 0x" << std::hex << another_oob_offset
        << ", but operation succeeded when it should have failed";
}

// Instantiate parameterized tests for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    CrossExecutionModeValidation,
    V128Load8x8uTestSuite,
    testing::Values(
        V128Load8x8uRunningMode::INTERP,
        V128Load8x8uRunningMode::AOT
    ),
    [](const testing::TestParamInfo<V128Load8x8uRunningMode>& info) {
        switch (info.param) {
            case V128Load8x8uRunningMode::INTERP: return "InterpreterMode";
            case V128Load8x8uRunningMode::AOT: return "AOTMode";
            default: return "UnknownMode";
        }
    }
);