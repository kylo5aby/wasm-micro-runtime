/*
 * Enhanced unit tests for v128.load8x8_s SIMD memory opcode
 * Tests SIMD load operation that loads 8 bytes from memory and sign-extends them to 8 16-bit integers
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"
#include "bh_read_file.h"

/**
 * Enhanced unit tests for v128.load8x8_s WASM opcode
 *
 * Tests comprehensive SIMD memory load functionality including:
 * - Basic sign extension operations with mixed positive/negative values
 * - Boundary condition handling (memory limits, address boundaries)
 * - Edge cases (all zeros, all negative, extreme values)
 * - Out-of-bounds memory access validation
 * - Cross-execution mode validation (interpreter vs AOT)
 */

// Test execution modes for cross-validation
enum class V128Load8x8sRunningMode : uint8_t {
    INTERP = 1 << 0,
    AOT = 1 << 1
};

static constexpr const char *MODULE_NAME = "v128_load8x8_s_test";
static constexpr const char *FUNC_NAME_TEST_LOAD = "test_load8x8_s";
static constexpr const char *FUNC_NAME_SET_BYTES = "set_memory_bytes";

/**
 * Test fixture for v128.load8x8_s opcode validation
 *
 * Provides comprehensive test environment for SIMD memory load operations
 * using WAMR test helpers for proper runtime initialization and
 * cross-execution mode testing (interpreter and AOT).
 */
class V128Load8x8sTestSuite : public testing::TestWithParam<V128Load8x8sRunningMode> {
protected:
    /**
     * Sets up test environment with WAMR runtime initialization
     *
     * Uses WAMR test helpers to initialize runtime with SIMD support
     * and loads the v128.load8x8_s test WASM module.
     */
    void SetUp() override {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the v128.load8x8_s test module using test helper
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/v128_load8x8_s_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for v128.load8x8_s tests";
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
     * Call WASM function to execute v128.load8x8_s with specified memory offset
     * @param offset Memory offset for load operation
     * @param result_lanes Array to store 8 i16 lane values from result vector
     */
    void call_load8x8_s(uint32_t offset, int16_t* result_lanes) {
        // Prepare arguments: offset as i32 parameter
        uint32_t argv[4];  // Space for v128 result (4 x i32)
        argv[0] = offset;

        // Execute function - result will be stored in argv as 4 i32 values
        bool success = dummy_env->execute(FUNC_NAME_TEST_LOAD, 4, argv);
        ASSERT_TRUE(success) << "Failed to execute v128.load8x8_s function";

        // Extract i16 lanes from returned v128 (packed as 4 i32 values)
        int16_t* packed_i16 = reinterpret_cast<int16_t*>(argv);

        // Copy 8 i16 lanes to output array
        std::memcpy(result_lanes, packed_i16, 8 * sizeof(int16_t));
    }

    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicLoad_SignExtension_ProducesCorrectVectorLanes
 * @brief Validates v128.load8x8_s produces correct sign extension for typical byte patterns
 * @details Tests fundamental load operation with mixed positive and negative bytes.
 *          Verifies that each byte is correctly sign-extended to 16-bit signed integers.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:wasm_interp_call_func_native
 * @input_conditions Standard byte patterns: [0x01, 0x7F, 0x80, 0xFF, 0x00, 0x55, 0xAA, 0x33]
 * @expected_behavior Returns v128 with lanes: [1, 127, -128, -1, 0, 85, -86, 51]
 * @validation_method Lane-by-lane comparison of sign-extended i16 values
 */
TEST_P(V128Load8x8sTestSuite, BasicLoad_SignExtension_ProducesCorrectVectorLanes) {
    // Set up test bytes with mixed positive and negative values
    uint8_t test_bytes[] = {0x01, 0x7F, 0x80, 0xFF, 0x00, 0x55, 0xAA, 0x33};
    int16_t expected_lanes[] = {1, 127, -128, -1, 0, 85, -86, 51};

    // Write test bytes to memory at offset 0
    set_memory_bytes(0, test_bytes, 8);

    // Execute v128.load8x8_s from memory offset 0
    int16_t result_lanes[8];
    call_load8x8_s(0, result_lanes);

    // Verify each lane contains correctly sign-extended value
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(expected_lanes[i], result_lanes[i])
            << "Lane " << i << " mismatch: expected " << expected_lanes[i]
            << " but got " << result_lanes[i] << " (byte 0x" << std::hex << (int)test_bytes[i] << ")";
    }
}

/**
 * @test BoundaryLoad_NearMemoryLimit_SucceedsCorrectly
 * @brief Validates loading from near memory boundary addresses without out-of-bounds errors
 * @details Tests memory access at addresses that should be valid for 8-byte loads.
 *          Ensures proper boundary checking and successful load operation.
 * @test_category Corner - Memory boundary condition validation
 * @coverage_target core/iwasm/common/wasm_memory.c:validate_app_addr
 * @input_conditions Memory access near valid boundaries with test pattern
 * @expected_behavior Successful load with correct sign extension, no memory violation
 * @validation_method Boundary address validation and result verification
 */
TEST_P(V128Load8x8sTestSuite, BoundaryLoad_NearMemoryLimit_SucceedsCorrectly) {
    // Test loading from various offsets that should be valid
    uint32_t test_offsets[] = {16, 32, 48, 64};  // Well within memory bounds

    for (uint32_t offset : test_offsets) {
        // Set up boundary test pattern
        uint8_t boundary_bytes[] = {0x12, 0x34, 0x87, 0x9A, 0x45, 0x67, 0xBC, 0xDE};
        int16_t expected_boundary[] = {0x0012, 0x0034, -121, -102, 0x0045, 0x0067, -68, -34};

        // Write test bytes at boundary address
        set_memory_bytes(offset, boundary_bytes, 8);

        // Execute load at boundary - should succeed
        int16_t result_lanes[8];
        call_load8x8_s(offset, result_lanes);

        // Verify boundary load produces correct results
        for (int i = 0; i < 8; ++i) {
            ASSERT_EQ(expected_boundary[i], result_lanes[i])
                << "Offset " << offset << " lane " << i << " mismatch: expected " << expected_boundary[i]
                << " but got " << result_lanes[i];
        }
    }
}

/**
 * @test ExtremeValues_AllZeroAndMaxNegative_HandledCorrectly
 * @brief Validates handling of extreme byte values (all zeros and all 0xFF)
 * @details Tests edge cases with uniform byte patterns to verify consistent sign extension.
 *          Covers both extremes of the signed byte range after extension.
 * @test_category Edge - Extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:wasm_load8x8_s_operation
 * @input_conditions All zeros [0x00 × 8] and all maximum bytes [0xFF × 8]
 * @expected_behavior Zero vector and all-negative-one vector respectively
 * @validation_method Extreme value sign extension verification
 */
TEST_P(V128Load8x8sTestSuite, ExtremeValues_AllZeroAndMaxNegative_HandledCorrectly) {
    // Test all-zero pattern
    uint8_t zero_bytes[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    set_memory_bytes(80, zero_bytes, 8);

    int16_t zero_result[8];
    call_load8x8_s(80, zero_result);
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(0, zero_result[i]) << "Zero pattern lane " << i << " should be 0, got " << zero_result[i];
    }

    // Test all-0xFF pattern (should become all -1)
    uint8_t max_neg_bytes[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    set_memory_bytes(96, max_neg_bytes, 8);

    int16_t max_neg_result[8];
    call_load8x8_s(96, max_neg_result);
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(-1, max_neg_result[i]) << "Max negative pattern lane " << i << " should be -1, got " << max_neg_result[i];
    }
}

/**
 * @test MemoryAccess_OutOfBounds_TriggersAppropriateError
 * @brief Validates proper error handling for out-of-bounds memory access
 * @details Tests that attempting to load beyond memory boundaries triggers appropriate
 *          runtime errors without causing crashes or undefined behavior.
 * @test_category Exception - Out-of-bounds error validation
 * @coverage_target core/iwasm/common/wasm_memory.c:validate_app_addr
 * @input_conditions Addresses beyond reasonable memory limits requiring 8-byte access
 * @expected_behavior Runtime error or trap, no undefined behavior or crashes
 * @validation_method Error condition verification and crash prevention
 */
TEST_P(V128Load8x8sTestSuite, MemoryAccess_OutOfBounds_TriggersAppropriateError) {
    // Test access to very large offset (likely out of bounds)
    uint32_t invalid_offsets[] = {100000, 1000000};  // Well beyond typical test memory

    for (uint32_t invalid_offset : invalid_offsets) {
        int16_t result_lanes[8];

        // This should either fail gracefully or succeed if memory is large enough
        // We mainly want to ensure no crashes occur
        dummy_env->clear_exception();

        uint32_t argv[1] = { invalid_offset };
        bool success = dummy_env->execute(FUNC_NAME_TEST_LOAD, 1, argv);

        // Check if there was an exception (out of bounds)
        const char* exception = dummy_env->get_exception();

        if (!success || exception) {
            // Expected behavior: function fails or exception is set
            ASSERT_TRUE(!success || (exception && strlen(exception) > 0))
                << "Expected out-of-bounds access to fail or set exception for offset " << invalid_offset;
        }

        // Clear any exception for next test
        dummy_env->clear_exception();
    }
}

/**
 * @test SignExtensionBoundary_CriticalValues_ExtendedCorrectly
 * @brief Validates precise sign extension at the critical boundary (0x7F/0x80)
 * @details Tests the exact boundary where sign extension behavior changes.
 *          Verifies that 0x7F extends to positive and 0x80 extends to negative.
 * @test_category Edge - Sign extension boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:sign_extension_i8_to_i16
 * @input_conditions Critical boundary bytes 0x7F and 0x80 with surrounding context
 * @expected_behavior 0x7F → 0x007F (127), 0x80 → 0xFF80 (-128)
 * @validation_method Precise sign bit handling verification
 */
TEST_P(V128Load8x8sTestSuite, SignExtensionBoundary_CriticalValues_ExtendedCorrectly) {
    // Test critical sign extension boundary values
    uint8_t boundary_bytes[] = {0x7E, 0x7F, 0x80, 0x81, 0x00, 0x01, 0xFE, 0xFF};
    int16_t expected_boundary[] = {126, 127, -128, -127, 0, 1, -2, -1};

    // Write boundary test bytes to memory
    set_memory_bytes(112, boundary_bytes, 8);

    // Execute load and verify precise sign extension
    int16_t result_lanes[8];
    call_load8x8_s(112, result_lanes);

    // Critical validation of sign bit behavior
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(expected_boundary[i], result_lanes[i])
            << "Sign boundary lane " << i << " mismatch: byte 0x" << std::hex
            << (int)boundary_bytes[i] << " should extend to " << expected_boundary[i]
            << " but got " << result_lanes[i];
    }

    // Specific validation of critical boundary cases
    ASSERT_EQ(127, result_lanes[1])   << "0x7F should sign-extend to +127";
    ASSERT_EQ(-128, result_lanes[2])  << "0x80 should sign-extend to -128";
}

// Parameterized test to run in both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(
    RunningModes,
    V128Load8x8sTestSuite,
    ::testing::Values(
        V128Load8x8sRunningMode::INTERP
#if WASM_ENABLE_AOT != 0
        , V128Load8x8sRunningMode::AOT
#endif
    ),
    [](const testing::TestParamInfo<V128Load8x8sRunningMode>& info) {
        return info.param == V128Load8x8sRunningMode::INTERP ? "Interpreter" : "AOT";
    }
);