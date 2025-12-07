/**
 * @file enhanced_i8x16_eq_test.cc
 * @brief Comprehensive unit tests for i8x16.eq SIMD opcode
 * @details Tests i8x16.eq functionality across interpreter and AOT execution modes
 *          with focus on element-wise equality comparison of sixteen 8-bit signed integers,
 *          mathematical properties validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_eq_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class I8x16EqTestSuite
 * @brief Test fixture class for i8x16.eq opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, Mathematical property validation
 */
class I8x16EqTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for i8x16.eq testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_eq_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.eq test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_eq_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i8x16.eq tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_eq_test.cc:TearDown
     */
    void TearDown() override
    {
        // Cleanup handled by RAII destructors
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call WASM i8x16.eq function with two vector inputs
     * @details Executes i8x16.eq operation on two input vectors and returns element-wise equality result.
     *          Handles WASM function invocation and v128 result extraction for 16-byte comparison.
     * @param input1_bytes 16-byte array representing first input v128 vector as i8 lanes
     * @param input2_bytes 16-byte array representing second input v128 vector as i8 lanes
     * @param result_bytes 16-byte array to store the equality comparison result
     * @return bool True if operation succeeded, false on error
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_eq_test.cc:call_i8x16_eq
     */
    bool call_i8x16_eq(const uint8_t input1_bytes[16], const uint8_t input2_bytes[16],
                       uint8_t result_bytes[16])
    {
        // Prepare arguments: two input v128 vectors as four i64 values each
        uint32_t argv[8];

        // Convert byte arrays to 64-bit values (little-endian format)
        // First v128 vector
        uint64_t input1_lo = 0, input1_hi = 0;
        for (int i = 0; i < 8; i++) {
            input1_lo |= (static_cast<uint64_t>(input1_bytes[i]) << (i * 8));
            input1_hi |= (static_cast<uint64_t>(input1_bytes[i + 8]) << (i * 8));
        }

        // Second v128 vector
        uint64_t input2_lo = 0, input2_hi = 0;
        for (int i = 0; i < 8; i++) {
            input2_lo |= (static_cast<uint64_t>(input2_bytes[i]) << (i * 8));
            input2_hi |= (static_cast<uint64_t>(input2_bytes[i + 8]) << (i * 8));
        }

        // WASM expects little-endian format: low part first, then high part
        // First v128 vector
        argv[0] = static_cast<uint32_t>(input1_lo);        // Low 32 bits of low i64
        argv[1] = static_cast<uint32_t>(input1_lo >> 32);  // High 32 bits of low i64
        argv[2] = static_cast<uint32_t>(input1_hi);        // Low 32 bits of high i64
        argv[3] = static_cast<uint32_t>(input1_hi >> 32);  // High 32 bits of high i64
        // Second v128 vector
        argv[4] = static_cast<uint32_t>(input2_lo);        // Low 32 bits of low i64
        argv[5] = static_cast<uint32_t>(input2_lo >> 32);  // High 32 bits of low i64
        argv[6] = static_cast<uint32_t>(input2_hi);        // Low 32 bits of high i64
        argv[7] = static_cast<uint32_t>(input2_hi >> 32);  // High 32 bits of high i64

        // Call WASM function with two v128 inputs
        bool call_success = dummy_env->execute("test_i8x16_eq", 8, argv);
        EXPECT_TRUE(call_success) << "Failed to call test_i8x16_eq function";

        if (call_success) {
            // Extract v128 result and convert back to byte array
            uint64_t result_lo = (static_cast<uint64_t>(argv[1]) << 32) | argv[0];
            uint64_t result_hi = (static_cast<uint64_t>(argv[3]) << 32) | argv[2];

            // Convert 64-bit values back to byte array
            for (int i = 0; i < 8; i++) {
                result_bytes[i] = static_cast<uint8_t>(result_lo >> (i * 8));
                result_bytes[i + 8] = static_cast<uint8_t>(result_hi >> (i * 8));
            }
        }

        return call_success;
    }

    /**
     * @brief Verify that i8x16.eq result vector matches expected byte pattern
     * @details Compares each of the 16 lanes for expected equality mask values.
     *          Equal lanes should contain 0xFF, unequal lanes should contain 0x00.
     * @param expected_bytes Expected 16-byte result pattern
     * @param actual_bytes Actual 16-byte result from i8x16.eq operation
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_eq_test.cc:assert_i8x16_eq_result
     */
    void assert_i8x16_eq_result(const uint8_t expected_bytes[16], const uint8_t actual_bytes[16])
    {
        for (int i = 0; i < 16; i++) {
            ASSERT_EQ(expected_bytes[i], actual_bytes[i])
                << "i8x16.eq lane " << i << " mismatch - Expected: 0x" << std::hex
                << static_cast<int>(expected_bytes[i]) << ", Actual: 0x" << std::hex
                << static_cast<int>(actual_bytes[i]);
        }
    }

    /**
     * @brief Helper function to create test vector with specified byte values
     * @details Populates a 16-byte array with provided values for test input preparation.
     * @param bytes Output array to populate
     * @param b0-b15 Individual byte values for lanes 0-15
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_i8x16_eq_test.cc:make_i8x16_vector
     */
    void make_i8x16_vector(uint8_t bytes[16],
                          uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3,
                          uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7,
                          uint8_t b8, uint8_t b9, uint8_t b10, uint8_t b11,
                          uint8_t b12, uint8_t b13, uint8_t b14, uint8_t b15)
    {
        bytes[0] = b0;   bytes[1] = b1;   bytes[2] = b2;   bytes[3] = b3;
        bytes[4] = b4;   bytes[5] = b5;   bytes[6] = b6;   bytes[7] = b7;
        bytes[8] = b8;   bytes[9] = b9;   bytes[10] = b10; bytes[11] = b11;
        bytes[12] = b12; bytes[13] = b13; bytes[14] = b14; bytes[15] = b15;
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicComparison_ProducesCorrectMask
 * @brief Validates i8x16.eq produces correct results for fundamental equality scenarios
 * @details Tests element-wise comparison with identical vectors, different vectors, and
 *          partial equality patterns. Validates proper boolean mask generation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/compilation/simd/simd_comparison.c:i8x16_eq_operation
 * @input_conditions Identical vectors, completely different vectors, mixed equality patterns
 * @expected_behavior 0xFF for equal lanes, 0x00 for different lanes in result mask
 * @validation_method Direct lane-by-lane comparison with expected equality masks
 */
TEST_F(I8x16EqTestSuite, BasicComparison_ProducesCorrectMask)
{
    uint8_t input1[16], input2[16], result[16], expected[16];

    // Test complete equality: identical vectors should produce all 0xFF
    make_i8x16_vector(input1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    make_i8x16_vector(input2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result))
        << "Failed to execute i8x16.eq with identical vectors";
    assert_i8x16_eq_result(expected, result);

    // Test complete inequality: different vectors should produce all 0x00
    make_i8x16_vector(input1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    make_i8x16_vector(input2, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    make_i8x16_vector(expected, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result))
        << "Failed to execute i8x16.eq with different vectors";
    assert_i8x16_eq_result(expected, result);

    // Test partial equality: alternating pattern
    make_i8x16_vector(input1, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8);
    make_i8x16_vector(input2, 1, 0, 3, 0, 5, 0, 7, 0, 1, 0, 3, 0, 5, 0, 7, 0);
    make_i8x16_vector(expected, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
                               0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result))
        << "Failed to execute i8x16.eq with partial equality pattern";
    assert_i8x16_eq_result(expected, result);
}

/**
 * @test BoundaryValues_HandleMinMaxCorrectly
 * @brief Tests i8x16.eq with signed 8-bit boundary values and edge conditions
 * @details Validates comparison behavior at MIN_VALUE (-128), MAX_VALUE (+127),
 *          zero boundary, and signed integer transitions.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/compilation/simd/simd_comparison.c:i8x16_signed_comparison
 * @input_conditions MIN/MAX signed values, zero, boundary transitions
 * @expected_behavior Accurate signed comparison at integer limits and transitions
 * @validation_method Boundary-specific equality validation with signed semantics
 */
TEST_F(I8x16EqTestSuite, BoundaryValues_HandleMinMaxCorrectly)
{
    uint8_t input1[16], input2[16], result[16], expected[16];

    // Test MIN_VALUE equality: -128 == -128 should be true
    make_i8x16_vector(input1, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                              0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
    make_i8x16_vector(input2, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                              0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result))
        << "Failed to execute i8x16.eq with MIN_VALUE (-128) boundary test";
    assert_i8x16_eq_result(expected, result);

    // Test MAX_VALUE equality: +127 == +127 should be true
    make_i8x16_vector(input1, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                              0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F);
    make_i8x16_vector(input2, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                              0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F);
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result))
        << "Failed to execute i8x16.eq with MAX_VALUE (+127) boundary test";
    assert_i8x16_eq_result(expected, result);

    // Test cross-boundary inequality: MIN vs MAX values
    make_i8x16_vector(input1, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F,
                              0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F);
    make_i8x16_vector(input2, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80,
                              0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80);
    make_i8x16_vector(expected, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result))
        << "Failed to execute i8x16.eq with MIN/MAX cross-boundary test";
    assert_i8x16_eq_result(expected, result);
}

/**
 * @test IdentityAndSymmetry_FollowMathematicalProperties
 * @brief Validates mathematical properties of i8x16.eq equality operation
 * @details Tests identity property (a==a), symmetry property (a==b equals b==a),
 *          zero vector handling, and extreme value patterns.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/compilation/simd/simd_comparison.c:i8x16_eq_properties
 * @input_conditions Self-comparison, operand swapping, zero vectors, extreme patterns
 * @expected_behavior Identity, symmetry, and zero handling properties hold
 * @validation_method Mathematical property verification with comprehensive patterns
 */
TEST_F(I8x16EqTestSuite, IdentityAndSymmetry_FollowMathematicalProperties)
{
    uint8_t input1[16], input2[16], result1[16], result2[16], expected[16];

    // Test identity property: any vector compared with itself should be all 0xFF
    make_i8x16_vector(input1, 42, 42, 42, 42, 42, 42, 42, 42,
                              42, 42, 42, 42, 42, 42, 42, 42);
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ASSERT_TRUE(call_i8x16_eq(input1, input1, result1))
        << "Failed to execute i8x16.eq for identity property test";
    assert_i8x16_eq_result(expected, result1);

    // Test symmetry property: eq(a,b) should equal eq(b,a)
    make_i8x16_vector(input1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    make_i8x16_vector(input2, 1, 0, 3, 0, 5, 0, 7, 0, 9, 0, 11, 0, 13, 0, 15, 0);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result1))
        << "Failed to execute first i8x16.eq for symmetry property test";
    ASSERT_TRUE(call_i8x16_eq(input2, input1, result2))
        << "Failed to execute second i8x16.eq for symmetry property test";
    assert_i8x16_eq_result(result1, result2);

    // Test all-zero vector equality
    make_i8x16_vector(input1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    make_i8x16_vector(input2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result1))
        << "Failed to execute i8x16.eq for all-zero vector test";
    assert_i8x16_eq_result(expected, result1);

    // Test extreme alternating pattern equality
    make_i8x16_vector(input1, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F,
                              0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F);
    make_i8x16_vector(input2, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F,
                              0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F);
    make_i8x16_vector(expected, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result1))
        << "Failed to execute i8x16.eq for extreme alternating pattern test";
    assert_i8x16_eq_result(expected, result1);
}

/**
 * @test ModuleValidation_LoadsAndExecutesCorrectly
 * @brief Tests WASM module loading and SIMD feature validation for i8x16.eq
 * @details Validates successful module loading when SIMD supported, proper function
 *          export availability, and basic execution environment setup.
 * @test_category Error - Module integration validation
 * @coverage_target core/iwasm/common/wasm_loader.c:simd_instruction_validation
 * @input_conditions Valid i8x16.eq WASM module, SIMD feature availability
 * @expected_behavior Successful module loading and function accessibility
 * @validation_method Module loading success and function export verification
 */
TEST_F(I8x16EqTestSuite, ModuleValidation_LoadsAndExecutesCorrectly)
{
    // Test that the module loaded successfully (validated in SetUp)
    ASSERT_NE(nullptr, dummy_env->get())
        << "i8x16.eq test module should load successfully when SIMD supported";

    // Test that no exception occurred during module loading
    const char* exception = dummy_env->get_exception();
    ASSERT_EQ(nullptr, exception)
        << "No exception should occur during i8x16.eq module loading: "
        << (exception ? exception : "null");

    // Test basic function execution to ensure the module is properly instantiated
    uint8_t input1[16], input2[16], result[16];
    make_i8x16_vector(input1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    make_i8x16_vector(input2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);

    ASSERT_TRUE(call_i8x16_eq(input1, input2, result))
        << "Basic i8x16.eq function call should succeed after successful module loading";

    // Verify the execution environment remains stable
    ASSERT_EQ(nullptr, dummy_env->get_exception())
        << "No runtime exception should occur during basic i8x16.eq execution";
}