/*
 * Copyright (C) 2024 Amazon Inc.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef V128_LOAD32_LANE_COMMON_H
#define V128_LOAD32_LANE_COMMON_H

#include <cstdint>
#include <vector>
#include <fstream>

/**
 * @file v128_load32_lane_common.h
 * @brief Common definitions and utilities for v128.load32_lane opcode testing
 * @details Provides shared constants, test data patterns, and utility functions
 *          for comprehensive v128.load32_lane SIMD instruction validation.
 */

// Test constants for v128.load32_lane operations
namespace V128Load32LaneTestConstants {
    // Memory test patterns for validation
    static constexpr uint32_t PATTERN_ZEROS = 0x00000000;
    static constexpr uint32_t PATTERN_ONES = 0xFFFFFFFF;
    static constexpr uint32_t PATTERN_ALTERNATING_1 = 0xAAAAAAAA;
    static constexpr uint32_t PATTERN_ALTERNATING_2 = 0x55555555;
    static constexpr uint32_t PATTERN_TEST_DATA = 0xDEADBEEF;
    static constexpr uint32_t PATTERN_ENDIAN_TEST = 0x12345678;

    // 32-bit boundary values for comprehensive testing
    static constexpr uint32_t UINT32_MIN = 0x00000000;
    static constexpr uint32_t UINT32_MAX = 0xFFFFFFFF;
    static constexpr uint32_t INT32_MAX = 0x7FFFFFFF;
    static constexpr uint32_t INT32_MIN = 0x80000000;

    // Special Float32 bit patterns (as 32-bit integers)
    static constexpr uint32_t FLOAT32_POSITIVE_ZERO = 0x00000000;
    static constexpr uint32_t FLOAT32_NEGATIVE_ZERO = 0x80000000;
    static constexpr uint32_t FLOAT32_POSITIVE_INFINITY = 0x7F800000;
    static constexpr uint32_t FLOAT32_NEGATIVE_INFINITY = 0xFF800000;
    static constexpr uint32_t FLOAT32_CANONICAL_NAN = 0x7FC00000;
    static constexpr uint32_t FLOAT32_SIGNALING_NAN = 0x7F800001;

    // Memory layout constants
    static constexpr uint32_t DEFAULT_MEMORY_SIZE = 65536; // 64KB default WASM memory
    static constexpr uint32_t MEMORY_PAGE_SIZE = 65536;
    static constexpr uint32_t LAST_VALID_32BIT_ADDRESS = DEFAULT_MEMORY_SIZE - 4;

    // Lane index constants for v128 (4 lanes for 32-bit values)
    static constexpr int LANE_COUNT_32BIT = 4;
    static constexpr int LANE_INDEX_MIN = 0;
    static constexpr int LANE_INDEX_MAX = 3;

    // Test data arrays for systematic testing
    static constexpr uint32_t BOUNDARY_TEST_VALUES[] = {
        UINT32_MIN, UINT32_MAX, INT32_MAX, INT32_MIN
    };

    static constexpr uint32_t SPECIAL_FLOAT_VALUES[] = {
        FLOAT32_POSITIVE_ZERO, FLOAT32_NEGATIVE_ZERO,
        FLOAT32_POSITIVE_INFINITY, FLOAT32_NEGATIVE_INFINITY,
        FLOAT32_CANONICAL_NAN, FLOAT32_SIGNALING_NAN
    };

    static constexpr uint32_t PATTERN_TEST_VALUES[] = {
        PATTERN_ZEROS, PATTERN_ONES,
        PATTERN_ALTERNATING_1, PATTERN_ALTERNATING_2,
        PATTERN_TEST_DATA, PATTERN_ENDIAN_TEST
    };
}

/**
 * @brief Utility functions for v128.load32_lane testing
 */
namespace V128Load32LaneTestUtils {
    /**
     * @brief Load WASM binary file into memory buffer
     * @param filename Path to WASM file
     * @return Vector containing WASM binary data
     */
    inline std::vector<uint8_t> load_wasm_file(const char* filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {};
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            return {};
        }

        return buffer;
    }

    /**
     * @brief Verify lane preservation in v128 result
     * @param original Original v128 values as 4 uint32_t array
     * @param result Result v128 values as 4 uint32_t array
     * @param modified_lane Lane index that should be modified (0-3)
     * @param expected_value Expected value for the modified lane
     * @return True if verification passes, false otherwise
     */
    inline bool verify_lane_preservation(const uint32_t original[4],
                                        const uint32_t result[4],
                                        int modified_lane,
                                        uint32_t expected_value) {
        if (modified_lane < 0 || modified_lane >= 4) {
            return false;
        }

        // Check that modified lane has expected value
        if (result[modified_lane] != expected_value) {
            return false;
        }

        // Check that all other lanes are preserved
        for (int i = 0; i < 4; i++) {
            if (i != modified_lane && result[i] != original[i]) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Generate test vector with specified pattern
     * @param pattern Pattern to use for all lanes
     * @param vec Output array for v128 vector (4 uint32_t values)
     */
    inline void create_test_vector(uint32_t pattern, uint32_t vec[4]) {
        for (int i = 0; i < 4; i++) {
            vec[i] = pattern;
        }
    }

    /**
     * @brief Create diverse test vector with different values per lane
     * @param vec Output array for v128 vector (4 uint32_t values)
     */
    inline void create_diverse_test_vector(uint32_t vec[4]) {
        vec[0] = 0x11111111;
        vec[1] = 0x22222222;
        vec[2] = 0x33333333;
        vec[3] = 0x44444444;
    }

    /**
     * @brief Validate memory address alignment
     * @param addr Memory address to check
     * @param alignment Required alignment (typically 4 for 32-bit)
     * @return True if address is properly aligned
     */
    inline bool is_aligned(uint32_t addr, uint32_t alignment) {
        return (addr % alignment) == 0;
    }
}

// Execution mode enumeration for parameterized testing
enum class RunningMode {
    INTERPRETER_MODE,
    AOT_MODE
};

#endif // V128_LOAD32_LANE_COMMON_H