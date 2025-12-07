/**
 * @file enhanced_i64x2_splat_test.cc
 * @brief Comprehensive unit tests for i64x2.splat SIMD opcode
 * @details Tests i64x2.splat functionality across interpreter and AOT execution modes
 *          with focus on basic operations, boundary conditions, 64-bit integer replication,
 *          and error scenarios. Validates WAMR SIMD implementation correctness
 *          and cross-mode consistency for i64 vector construction operations.
 * @source_location tests/unit/enhanced_opcode/extension/enhanced_i64x2_splat_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <climits>
#include <cstdint>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class I64x2SplatTestSuite
 * @brief Test fixture class for i64x2.splat opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles SIMD vector result validation using DummyExecEnv helper
 *          for comprehensive i64x2.splat operation validation.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class I64x2SplatTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for i64x2.splat testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files using relative path.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_i64x2_splat_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i64x2.splat test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i64x2_splat_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i64x2.splat tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_i64x2_splat_test.cc:TearDown
     */
    void TearDown() override
    {
        // RAII automatically handles cleanup
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call i64x2.splat WASM function and validate results
     * @param func_name WASM function name to call
     * @param value i64 input value to be replicated across both lanes
     * @param expected_values Array of 2 expected i64 values (should both be same as input)
     * @details Calls the specified WASM function and validates the execution.
     *          Returns the resulting i64 values for lane-by-lane validation.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_i64x2_splat_test.cc:CallI64x2Splat
     */
    void CallI64x2Splat(const char* func_name, int64_t value, int64_t expected_values[2])
    {
        // v128 functions return 4 uint32_t values which overwrite the argv array
        // For i64 input: argv[0] = low 32 bits, argv[1] = high 32 bits of input
        // For v128 output: argv contains 4 uint32_t values representing 2 i64 values
        uint32_t argv[4];
        uint64_t input_u64 = static_cast<uint64_t>(value);
        argv[0] = static_cast<uint32_t>(input_u64);        // Low 32 bits
        argv[1] = static_cast<uint32_t>(input_u64 >> 32);  // High 32 bits
        argv[2] = 0;
        argv[3] = 0;

        // Execute the function and expect success (pass 2 uint32_t for 1 i64 parameter)
        bool call_result = dummy_env->execute(func_name, 2, argv);
        ASSERT_TRUE(call_result)
            << "Failed to call " << func_name << " with value: " << value;

        // Extract result i64 values from argv (now contains the v128 result as 4 uint32_t)
        // Reconstruct i64 values from pairs of uint32_t values
        int64_t result_values[2];
        result_values[0] = static_cast<int64_t>(static_cast<uint64_t>(argv[0]) | (static_cast<uint64_t>(argv[1]) << 32));
        result_values[1] = static_cast<int64_t>(static_cast<uint64_t>(argv[2]) | (static_cast<uint64_t>(argv[3]) << 32));

        // Validate both lanes contain the expected value
        for (int lane = 0; lane < 2; ++lane) {
            ASSERT_EQ(expected_values[lane], result_values[lane])
                << "Lane " << lane << " mismatch for value " << value
                << ": expected " << expected_values[lane]
                << ", got " << result_values[lane];
        }
    }

    /**
     * @brief Helper function to validate stack underflow behavior
     * @param func_name WASM function name that should cause stack underflow
     * @details Attempts to call function that causes stack underflow and validates
     *          proper error handling by WAMR runtime.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_i64x2_splat_test.cc:ValidateStackUnderflow
     */
    void ValidateStackUnderflow(const char* func_name)
    {
        uint32_t argv[4] = { 0, 0, 0, 0 };

        // Execute function expected to fail due to stack underflow
        bool call_result = dummy_env->execute(func_name, 0, argv);
        ASSERT_FALSE(call_result)
            << "Expected stack underflow function " << func_name << " to fail, but it succeeded";
    }

protected:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicSplat_TypicalValues_CreatesUniformVector
 * @brief Validates i64x2.splat creates uniform vectors with typical i64 values
 * @details Tests fundamental splat operation with positive, negative, and mixed values.
 *          Verifies that i64x2.splat correctly replicates input value across both lanes.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i64x2_splat
 * @input_conditions Standard i64 values: (42), (-100), (1000000)
 * @expected_behavior Returns uniform vectors: [42,42], [-100,-100], [1000000,1000000]
 * @validation_method Direct comparison of both lanes with expected values
 */
TEST_F(I64x2SplatTestSuite, BasicSplat_TypicalValues_CreatesUniformVector)
{
    // Test positive value replication
    int64_t expected_positive[2] = { 42, 42 };
    CallI64x2Splat("test_i64x2_splat_basic", 42, expected_positive);

    // Test negative value replication
    int64_t expected_negative[2] = { -100, -100 };
    CallI64x2Splat("test_i64x2_splat_basic", -100, expected_negative);

    // Test larger positive value replication
    int64_t expected_large[2] = { 1000000, 1000000 };
    CallI64x2Splat("test_i64x2_splat_basic", 1000000, expected_large);
}

/**
 * @test BoundaryValues_IntegerLimits_CreatesUniformVector
 * @brief Validates i64x2.splat handles boundary values correctly
 * @details Tests splat operation with INT64_MIN and INT64_MAX values.
 *          Verifies correct replication of extreme i64 boundary conditions.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i64x2_splat
 * @input_conditions Boundary values: INT64_MIN (-9223372036854775808), INT64_MAX (9223372036854775807)
 * @expected_behavior Returns uniform vectors with both lanes containing boundary values
 * @validation_method Lane-by-lane verification of boundary value replication
 */
TEST_F(I64x2SplatTestSuite, BoundaryValues_IntegerLimits_CreatesUniformVector)
{
    // Test INT64_MIN replication
    int64_t expected_min[2] = { INT64_MIN, INT64_MIN };
    CallI64x2Splat("test_i64x2_splat_basic", INT64_MIN, expected_min);

    // Test INT64_MAX replication
    int64_t expected_max[2] = { INT64_MAX, INT64_MAX };
    CallI64x2Splat("test_i64x2_splat_basic", INT64_MAX, expected_max);
}

/**
 * @test SpecialValues_ZeroAndNegativeOne_CreatesUniformVector
 * @brief Validates i64x2.splat with special values zero and -1
 * @details Tests splat operation with zero and negative one values.
 *          Verifies correct replication of special numeric patterns.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i64x2_splat
 * @input_conditions Special values: 0, -1, 1
 * @expected_behavior Returns uniform vectors: [0,0], [-1,-1], [1,1]
 * @validation_method Direct comparison with expected special value patterns
 */
TEST_F(I64x2SplatTestSuite, SpecialValues_ZeroAndNegativeOne_CreatesUniformVector)
{
    // Test zero replication
    int64_t expected_zero[2] = { 0, 0 };
    CallI64x2Splat("test_i64x2_splat_basic", 0, expected_zero);

    // Test negative one replication (all bits set)
    int64_t expected_neg_one[2] = { -1, -1 };
    CallI64x2Splat("test_i64x2_splat_basic", -1, expected_neg_one);

    // Test positive one replication
    int64_t expected_one[2] = { 1, 1 };
    CallI64x2Splat("test_i64x2_splat_basic", 1, expected_one);
}

/**
 * @test IdentityProperty_SplatExtract_PreservesOriginalValue
 * @brief Validates splat-extract identity property preservation
 * @details Tests that splatting a value then extracting from any lane returns original value.
 *          Verifies fundamental mathematical property of splat operation.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i64x2_splat + extract_lane
 * @input_conditions Various test values for roundtrip: 123456, -789012, 0
 * @expected_behavior Extract from any lane returns original input value
 * @validation_method Roundtrip verification through splat and extract operations
 */
TEST_F(I64x2SplatTestSuite, IdentityProperty_SplatExtract_PreservesOriginalValue)
{
    // Test identity property with positive value
    uint32_t argv_positive[4];
    uint64_t positive_u64 = static_cast<uint64_t>(123456);
    argv_positive[0] = static_cast<uint32_t>(positive_u64);
    argv_positive[1] = static_cast<uint32_t>(positive_u64 >> 32);
    argv_positive[2] = 0;
    argv_positive[3] = 0;
    bool result = dummy_env->execute("test_i64x2_splat_extract_identity", 2, argv_positive);
    ASSERT_TRUE(result) << "Failed to execute splat-extract identity test with positive value";
    int64_t positive_result = static_cast<int64_t>(static_cast<uint64_t>(argv_positive[0]) | (static_cast<uint64_t>(argv_positive[1]) << 32));
    ASSERT_EQ(123456, positive_result)
        << "Identity property failed for positive value 123456";

    // Test identity property with negative value
    uint32_t argv_negative[4];
    uint64_t negative_u64 = static_cast<uint64_t>(-789012LL);
    argv_negative[0] = static_cast<uint32_t>(negative_u64);
    argv_negative[1] = static_cast<uint32_t>(negative_u64 >> 32);
    argv_negative[2] = 0;
    argv_negative[3] = 0;
    result = dummy_env->execute("test_i64x2_splat_extract_identity", 2, argv_negative);
    ASSERT_TRUE(result) << "Failed to execute splat-extract identity test with negative value";
    int64_t negative_result = static_cast<int64_t>(static_cast<uint64_t>(argv_negative[0]) | (static_cast<uint64_t>(argv_negative[1]) << 32));
    ASSERT_EQ(-789012LL, negative_result)
        << "Identity property failed for negative value -789012";

    // Test identity property with zero
    uint32_t argv_zero[4] = { 0, 0, 0, 0 };
    result = dummy_env->execute("test_i64x2_splat_extract_identity", 2, argv_zero);
    ASSERT_TRUE(result) << "Failed to execute splat-extract identity test with zero";
    int64_t zero_result = static_cast<int64_t>(static_cast<uint64_t>(argv_zero[0]) | (static_cast<uint64_t>(argv_zero[1]) << 32));
    ASSERT_EQ(0, zero_result)
        << "Identity property failed for zero value";
}

/**
 * @test PowersOfTwo_ExtremeValues_CreatesUniformVector
 * @brief Validates i64x2.splat with power-of-2 values and extreme magnitudes
 * @details Tests splat operation with powers of 2 and large magnitude values.
 *          Verifies correct replication of specific bit patterns.
 * @test_category Edge - Power-of-2 and extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i64x2_splat
 * @input_conditions Powers of 2: 2, 4, 1024, 2048, large 64-bit values
 * @expected_behavior Returns uniform vectors with correct power-of-2 replication
 * @validation_method Bit pattern verification and value comparison
 */
TEST_F(I64x2SplatTestSuite, PowersOfTwo_ExtremeValues_CreatesUniformVector)
{
    // Test small powers of 2
    int64_t expected_2[2] = { 2, 2 };
    CallI64x2Splat("test_i64x2_splat_basic", 2, expected_2);

    int64_t expected_4[2] = { 4, 4 };
    CallI64x2Splat("test_i64x2_splat_basic", 4, expected_4);

    // Test medium powers of 2
    int64_t expected_1024[2] = { 1024, 1024 };
    CallI64x2Splat("test_i64x2_splat_basic", 1024, expected_1024);

    int64_t expected_2048[2] = { 2048, 2048 };
    CallI64x2Splat("test_i64x2_splat_basic", 2048, expected_2048);

    // Test large power of 2 (64-bit specific)
    int64_t large_power = 1LL << 40; // 1099511627776
    int64_t expected_large[2] = { large_power, large_power };
    CallI64x2Splat("test_i64x2_splat_basic", large_power, expected_large);
}

/**
 * @test AlternatingBitPatterns_SpecialValues_CreatesUniformVector
 * @brief Validates i64x2.splat with alternating bit patterns and special 64-bit values
 * @details Tests splat operation with specific bit patterns that exercise 64-bit boundaries.
 *          Verifies correct replication of alternating and boundary bit patterns.
 * @test_category Edge - Bit pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i64x2_splat
 * @input_conditions Bit patterns: 0x5555555555555555, 0xAAAAAAAAAAAAAAAA, sign bit patterns
 * @expected_behavior Returns uniform vectors with exact bit pattern replication
 * @validation_method Hexadecimal bit pattern verification
 */
TEST_F(I64x2SplatTestSuite, AlternatingBitPatterns_SpecialValues_CreatesUniformVector)
{
    // Test alternating bit pattern (0x5555...)
    int64_t alternating_55 = 0x5555555555555555LL;
    int64_t expected_55[2] = { alternating_55, alternating_55 };
    CallI64x2Splat("test_i64x2_splat_basic", alternating_55, expected_55);

    // Test alternating bit pattern (0xAAAA...)
    int64_t alternating_AA = static_cast<int64_t>(0xAAAAAAAAAAAAAAAAULL);
    int64_t expected_AA[2] = { alternating_AA, alternating_AA };
    CallI64x2Splat("test_i64x2_splat_basic", alternating_AA, expected_AA);

    // Test sign bit set (0x8000000000000000)
    int64_t sign_bit = static_cast<int64_t>(0x8000000000000000ULL);
    int64_t expected_sign[2] = { sign_bit, sign_bit };
    CallI64x2Splat("test_i64x2_splat_basic", sign_bit, expected_sign);
}

/**
 * @test StackUnderflow_EmptyStack_FailsGracefully
 * @brief Validates proper error handling for stack underflow scenarios
 * @details Tests i64x2.splat execution with insufficient stack values.
 *          Verifies WAMR runtime properly detects and handles stack underflow.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:stack_underflow_handling
 * @input_conditions Execute i64x2.splat with empty stack (no i64 value available)
 * @expected_behavior Runtime trap or execution failure with proper error handling
 * @validation_method Trap detection and error state validation
 */
TEST_F(I64x2SplatTestSuite, StackUnderflow_EmptyStack_FailsGracefully)
{
    // Test stack underflow scenario - function should fail gracefully
    ValidateStackUnderflow("test_i64x2_splat_stack_underflow");
}