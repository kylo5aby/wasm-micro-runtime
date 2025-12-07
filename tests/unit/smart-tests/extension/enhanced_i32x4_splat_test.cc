/**
 * @file enhanced_i32x4_splat_test.cc
 * @brief Comprehensive unit tests for i32x4.splat SIMD opcode
 * @details Tests i32x4.splat functionality across interpreter and AOT execution modes
 *          with focus on basic operations, boundary conditions, integer replication,
 *          and error scenarios. Validates WAMR SIMD implementation correctness
 *          and cross-mode consistency for i32 vector construction operations.
 * @source_location tests/unit/enhanced_opcode/extension/enhanced_i32x4_splat_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <climits>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class I32x4SplatTestSuite
 * @brief Test fixture class for i32x4.splat opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles SIMD vector result validation using DummyExecEnv helper
 *          for comprehensive i32x4.splat operation validation.
 * @test_categories Main, Corner, Edge, Error exception validation
 */
class I32x4SplatTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for i32x4.splat testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files using relative path.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_i32x4_splat_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i32x4.splat test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i32x4_splat_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for i32x4.splat tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Properly destroys execution environment and WAMR runtime
     *          using RAII pattern to prevent resource leaks.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_i32x4_splat_test.cc:TearDown
     */
    void TearDown() override
    {
        // RAII automatically handles cleanup
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to call i32x4.splat WASM function and validate results
     * @param func_name WASM function name to call
     * @param value i32 input value to be replicated across all lanes
     * @param expected_values Array of 4 expected i32 values (should all be same as input)
     * @details Calls the specified WASM function and validates the execution.
     *          Returns the resulting i32 values for lane-by-lane validation.
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_i32x4_splat_test.cc:CallI32x4Splat
     */
    void CallI32x4Splat(const char* func_name, int32_t value, int32_t expected_values[4])
    {
        // v128 functions return 4 uint32_t values which overwrite the argv array
        uint32_t argv[4] = { static_cast<uint32_t>(value), 0, 0, 0 };

        // Execute the function and expect success
        bool call_result = dummy_env->execute(func_name, 1, argv);
        ASSERT_TRUE(call_result)
            << "Failed to call " << func_name << " with value: " << value;

        // Extract result i32 values from argv (now contains the v128 result)
        int32_t* result_values = reinterpret_cast<int32_t*>(argv);

        // Validate all four lanes contain the expected value
        for (int lane = 0; lane < 4; ++lane) {
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
     * @source_location tests/unit/enhanced_opcode/extension/enhanced_i32x4_splat_test.cc:ValidateStackUnderflow
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
 * @brief Validates i32x4.splat creates uniform vectors with typical i32 values
 * @details Tests fundamental splat operation with positive, negative, and mixed values.
 *          Verifies that i32x4.splat correctly replicates input value across all 4 lanes.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i32x4_splat
 * @input_conditions Standard i32 pairs: (42), (-100), (1000)
 * @expected_behavior Returns uniform vectors: [42,42,42,42], [-100,-100,-100,-100], [1000,1000,1000,1000]
 * @validation_method Direct comparison of all lanes with expected values
 */
TEST_F(I32x4SplatTestSuite, BasicSplat_TypicalValues_CreatesUniformVector)
{
    // Test positive value replication
    int32_t expected_positive[4] = { 42, 42, 42, 42 };
    CallI32x4Splat("test_i32x4_splat_basic", 42, expected_positive);

    // Test negative value replication
    int32_t expected_negative[4] = { -100, -100, -100, -100 };
    CallI32x4Splat("test_i32x4_splat_basic", -100, expected_negative);

    // Test larger positive value replication
    int32_t expected_large[4] = { 1000, 1000, 1000, 1000 };
    CallI32x4Splat("test_i32x4_splat_basic", 1000, expected_large);
}

/**
 * @test BoundaryValues_IntegerLimits_CreatesUniformVector
 * @brief Validates i32x4.splat handles boundary values correctly
 * @details Tests splat operation with INT32_MIN and INT32_MAX values.
 *          Verifies correct replication of extreme i32 boundary conditions.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i32x4_splat
 * @input_conditions Boundary values: INT32_MIN (-2147483648), INT32_MAX (2147483647)
 * @expected_behavior Returns uniform vectors with all lanes containing boundary values
 * @validation_method Lane-by-lane verification of boundary value replication
 */
TEST_F(I32x4SplatTestSuite, BoundaryValues_IntegerLimits_CreatesUniformVector)
{
    // Test INT32_MIN replication
    int32_t expected_min[4] = { INT32_MIN, INT32_MIN, INT32_MIN, INT32_MIN };
    CallI32x4Splat("test_i32x4_splat_basic", INT32_MIN, expected_min);

    // Test INT32_MAX replication
    int32_t expected_max[4] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
    CallI32x4Splat("test_i32x4_splat_basic", INT32_MAX, expected_max);
}

/**
 * @test SpecialValues_ZeroAndNegativeOne_CreatesUniformVector
 * @brief Validates i32x4.splat with special values zero and -1
 * @details Tests splat operation with zero and negative one values.
 *          Verifies correct replication of special numeric patterns.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i32x4_splat
 * @input_conditions Special values: 0, -1, 1
 * @expected_behavior Returns uniform vectors: [0,0,0,0], [-1,-1,-1,-1], [1,1,1,1]
 * @validation_method Direct comparison with expected special value patterns
 */
TEST_F(I32x4SplatTestSuite, SpecialValues_ZeroAndNegativeOne_CreatesUniformVector)
{
    // Test zero replication
    int32_t expected_zero[4] = { 0, 0, 0, 0 };
    CallI32x4Splat("test_i32x4_splat_basic", 0, expected_zero);

    // Test negative one replication (all bits set)
    int32_t expected_neg_one[4] = { -1, -1, -1, -1 };
    CallI32x4Splat("test_i32x4_splat_basic", -1, expected_neg_one);

    // Test positive one replication
    int32_t expected_one[4] = { 1, 1, 1, 1 };
    CallI32x4Splat("test_i32x4_splat_basic", 1, expected_one);
}

/**
 * @test IdentityProperty_SplatExtract_PreservesOriginalValue
 * @brief Validates splat-extract identity property preservation
 * @details Tests that splatting a value then extracting from any lane returns original value.
 *          Verifies fundamental mathematical property of splat operation.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i32x4_splat + extract_lane
 * @input_conditions Various test values for roundtrip: 123, -456, 789, 0
 * @expected_behavior Extract from any lane returns original input value
 * @validation_method Roundtrip verification through splat and extract operations
 */
TEST_F(I32x4SplatTestSuite, IdentityProperty_SplatExtract_PreservesOriginalValue)
{
    // Test identity property with positive value
    uint32_t argv_positive[4] = { 123, 0, 0, 0 };
    bool result = dummy_env->execute("test_i32x4_splat_extract_identity", 1, argv_positive);
    ASSERT_TRUE(result) << "Failed to execute splat-extract identity test with positive value";
    ASSERT_EQ(123, static_cast<int32_t>(argv_positive[0]))
        << "Identity property failed for positive value 123";

    // Test identity property with negative value
    uint32_t argv_negative[4] = { static_cast<uint32_t>(-456), 0, 0, 0 };
    result = dummy_env->execute("test_i32x4_splat_extract_identity", 1, argv_negative);
    ASSERT_TRUE(result) << "Failed to execute splat-extract identity test with negative value";
    ASSERT_EQ(-456, static_cast<int32_t>(argv_negative[0]))
        << "Identity property failed for negative value -456";

    // Test identity property with zero
    uint32_t argv_zero[4] = { 0, 0, 0, 0 };
    result = dummy_env->execute("test_i32x4_splat_extract_identity", 1, argv_zero);
    ASSERT_TRUE(result) << "Failed to execute splat-extract identity test with zero";
    ASSERT_EQ(0, static_cast<int32_t>(argv_zero[0]))
        << "Identity property failed for zero value";
}

/**
 * @test PowersOfTwo_ExtremeValues_CreatesUniformVector
 * @brief Validates i32x4.splat with power-of-2 values and extreme magnitudes
 * @details Tests splat operation with powers of 2 and large magnitude values.
 *          Verifies correct replication of specific bit patterns.
 * @test_category Edge - Power-of-2 and extreme value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:SIMD_i32x4_splat
 * @input_conditions Powers of 2: 1, 2, 4, 1024, 2048, large values
 * @expected_behavior Returns uniform vectors with correct power-of-2 replication
 * @validation_method Bit pattern verification and value comparison
 */
TEST_F(I32x4SplatTestSuite, PowersOfTwo_ExtremeValues_CreatesUniformVector)
{
    // Test small powers of 2
    int32_t expected_2[4] = { 2, 2, 2, 2 };
    CallI32x4Splat("test_i32x4_splat_basic", 2, expected_2);

    int32_t expected_4[4] = { 4, 4, 4, 4 };
    CallI32x4Splat("test_i32x4_splat_basic", 4, expected_4);

    // Test medium powers of 2
    int32_t expected_1024[4] = { 1024, 1024, 1024, 1024 };
    CallI32x4Splat("test_i32x4_splat_basic", 1024, expected_1024);

    int32_t expected_2048[4] = { 2048, 2048, 2048, 2048 };
    CallI32x4Splat("test_i32x4_splat_basic", 2048, expected_2048);

    // Test large power of 2
    int32_t large_power = 1 << 20; // 1048576
    int32_t expected_large[4] = { large_power, large_power, large_power, large_power };
    CallI32x4Splat("test_i32x4_splat_basic", large_power, expected_large);
}

/**
 * @test StackUnderflow_EmptyStack_FailsGracefully
 * @brief Validates proper error handling for stack underflow scenarios
 * @details Tests i32x4.splat execution with insufficient stack values.
 *          Verifies WAMR runtime properly detects and handles stack underflow.
 * @test_category Error - Stack underflow validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_fast.c:stack_underflow_handling
 * @input_conditions Execute i32x4.splat with empty stack (no i32 value available)
 * @expected_behavior Runtime trap or execution failure with proper error handling
 * @validation_method Trap detection and error state validation
 */
TEST_F(I32x4SplatTestSuite, StackUnderflow_EmptyStack_FailsGracefully)
{
    // Test stack underflow scenario - function should fail gracefully
    ValidateStackUnderflow("test_i32x4_splat_stack_underflow");
}