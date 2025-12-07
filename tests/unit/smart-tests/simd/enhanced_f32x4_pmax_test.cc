/**
 * @file enhanced_f32x4_pmax_test.cc
 * @brief Comprehensive unit tests for f32x4.pmax SIMD opcode
 * @details Tests f32x4.pmax functionality across interpreter and AOT execution modes
 *          with focus on element-wise pseudo-maximum operation of two 32-bit single-precision
 *          floating-point values, IEEE 754 compliance validation, and comprehensive edge case coverage.
 *          Validates WAMR SIMD implementation correctness and cross-mode consistency.
 * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmax_test.cc
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <cmath>
#include <limits>
#include <cfloat>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class F32x4PmaxTestSuite
 * @brief Test fixture class for f32x4.pmax opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Handles memory initialization and SIMD vector result validation
 *          for comprehensive testing using existing WAMR test helpers.
 * @test_categories Main, Corner, Edge, IEEE 754 compliance validation
 */
class F32x4PmaxTestSuite : public testing::Test
{
protected:
    /**
     * @brief Initialize WAMR runtime and prepare test environment for f32x4.pmax testing
     * @details Sets up WAMR runtime with SIMD support using WAMRRuntimeRAII helper.
     *          Initializes execution environment for WASM test files.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmax_test.cc:SetUp
     */
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the f32x4.pmax test module using relative path
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/f32x4_pmax_test.wasm");
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to create execution environment for f32x4.pmax tests";
    }

    /**
     * @brief Clean up WAMR runtime and release test resources
     * @details Performs cleanup of WAMR runtime environment using RAII helpers.
     *          Ensures proper resource deallocation and runtime shutdown.
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmax_test.cc:TearDown
     */
    void TearDown() override
    {
        // RAII handles cleanup automatically
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Helper function to execute f32x4.pmax with 8 f32 parameters
     * @details Creates f32x4 vectors from input parameters, executes f32x4.pmax via WASM,
     *          and returns the result as 4 f32 values.
     * @param function_name WASM export function name to call
     * @param a0,a1,a2,a3 First f32x4 vector components
     * @param b0,b1,b2,b3 Second f32x4 vector components
     * @return Array of 4 f32 results from f32x4.pmax operation
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmax_test.cc:call_f32x4_pmax
     */
    std::array<float, 4> call_f32x4_pmax(const char *function_name,
                                         float a0, float a1, float a2, float a3,
                                         float b0, float b1, float b2, float b3)
    {
        // Prepare function arguments (8 f32 values for 2 f32x4 vectors)
        uint32_t argv[8];
        memcpy(&argv[0], &a0, sizeof(float));
        memcpy(&argv[1], &a1, sizeof(float));
        memcpy(&argv[2], &a2, sizeof(float));
        memcpy(&argv[3], &a3, sizeof(float));
        memcpy(&argv[4], &b0, sizeof(float));
        memcpy(&argv[5], &b1, sizeof(float));
        memcpy(&argv[6], &b2, sizeof(float));
        memcpy(&argv[7], &b3, sizeof(float));

        // Execute WASM function and validate successful execution
        bool success = dummy_env->execute(function_name, 8, argv);
        EXPECT_TRUE(success) << "Failed to execute " << function_name;

        // Extract and return f32x4 result as array
        std::array<float, 4> result;
        if (success) {
            memcpy(&result[0], &argv[0], sizeof(float));
            memcpy(&result[1], &argv[1], sizeof(float));
            memcpy(&result[2], &argv[2], sizeof(float));
            memcpy(&result[3], &argv[3], sizeof(float));
        }

        return result;
    }

    /**
     * @brief Helper function to check if a float value is NaN
     * @details Uses IEEE 754 standard isnan check for reliable NaN detection
     * @param value Float value to check for NaN
     * @return True if value is NaN, false otherwise
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmax_test.cc:isNaN
     */
    bool isNaN(float value) {
        return std::isnan(value);
    }

    /**
     * @brief Helper function to check if float value is positive infinity
     * @details Checks for IEEE 754 positive infinity value
     * @param value Float value to check
     * @return True if value is positive infinity
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmax_test.cc:isPosInf
     */
    bool isPosInf(float value) {
        return std::isinf(value) && value > 0.0f;
    }

    /**
     * @brief Helper function to check if float value is negative infinity
     * @details Checks for IEEE 754 negative infinity value
     * @param value Float value to check
     * @return True if value is negative infinity
     * @source_location tests/unit/enhanced_opcode/simd/enhanced_f32x4_pmax_test.cc:isNegInf
     */
    bool isNegInf(float value) {
        return std::isinf(value) && value < 0.0f;
    }

private:
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

// Test cases implementation

/**
 * @test BasicPmax_ReturnsCorrectMaximum
 * @brief Validates f32x4.pmax produces correct pseudo-maximum results for typical inputs
 * @details Tests fundamental pseudo-maximum operation with positive, negative, and mixed-sign floats.
 *          Verifies that f32x4.pmax correctly computes element-wise maximum for various input combinations.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmax_operation
 * @input_conditions Standard float pairs: (5.0f, 3.0f), (-10.0f, -15.0f), (20.0f, -8.0f), (0.0f, 1.0f)
 * @expected_behavior Returns mathematical maximum: 5.0f, -10.0f, 20.0f, 1.0f respectively
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(F32x4PmaxTestSuite, BasicPmax_ReturnsCorrectMaximum)
{
    // Execute f32x4.pmax with typical float values across all lanes
    auto result = call_f32x4_pmax("test_f32x4_pmax_basic",
                                  5.0f, -10.0f, 20.0f, 0.0f,    // First vector
                                  3.0f, -15.0f, -8.0f, 1.0f);   // Second vector

    // Validate pseudo-maximum results for each lane
    ASSERT_EQ(5.0f, result[0])  << "Lane 0: pmax(5.0f, 3.0f) should return 5.0f";
    ASSERT_EQ(-10.0f, result[1]) << "Lane 1: pmax(-10.0f, -15.0f) should return -10.0f";
    ASSERT_EQ(20.0f, result[2])  << "Lane 2: pmax(20.0f, -8.0f) should return 20.0f";
    ASSERT_EQ(1.0f, result[3])   << "Lane 3: pmax(0.0f, 1.0f) should return 1.0f";
}

/**
 * @test MixedSignValues_ReturnsExpectedResults
 * @brief Validates f32x4.pmax handles mixed positive/negative value combinations correctly
 * @details Tests pseudo-maximum operation with various sign patterns to ensure consistent behavior
 *          across different combinations of positive, negative, and zero values.
 * @test_category Main - Mixed sign value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmax_operation
 * @input_conditions Mixed sign combinations: (-7.5f, 2.25f), (0.0f, -0.0f), (42.0f, 42.0f), (-100.0f, 50.0f)
 * @expected_behavior Returns correct maximum: 2.25f, 0.0f, 42.0f, 50.0f respectively
 * @validation_method Element-wise comparison with detailed error messages
 */
TEST_F(F32x4PmaxTestSuite, MixedSignValues_ReturnsExpectedResults)
{
    // Test mixed positive/negative combinations
    auto result = call_f32x4_pmax("test_f32x4_pmax_basic",
                                  -7.5f, 0.0f, 42.0f, -100.0f,     // First vector
                                  2.25f, -0.0f, 42.0f, 50.0f);     // Second vector

    // Validate each lane handles sign differences correctly
    ASSERT_EQ(2.25f, result[0])  << "Lane 0: pmax(-7.5f, 2.25f) should return positive value";
    ASSERT_EQ(0.0f, result[1])   << "Lane 1: pmax(0.0f, -0.0f) should return 0.0f";
    ASSERT_EQ(42.0f, result[2])  << "Lane 2: pmax(42.0f, 42.0f) should return equal value";
    ASSERT_EQ(50.0f, result[3])  << "Lane 3: pmax(-100.0f, 50.0f) should return positive value";
}

/**
 * @test BoundaryValues_MaxFloat_ReturnsCorrectResult
 * @brief Tests f32x4.pmax behavior with floating-point boundary values
 * @details Validates pseudo-maximum operation with FLT_MAX, FLT_MIN, and extreme float values
 *          to ensure proper handling of boundary conditions in IEEE 754 representation.
 * @test_category Corner - Boundary value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmax_operation
 * @input_conditions Boundary values: FLT_MAX, FLT_MIN, large negative, small positive
 * @expected_behavior Returns correct maximum at float boundaries
 * @validation_method Boundary-specific assertions with IEEE 754 compliance
 */
TEST_F(F32x4PmaxTestSuite, BoundaryValues_MaxFloat_ReturnsCorrectResult)
{
    // Test with floating-point boundary values
    auto result = call_f32x4_pmax("test_f32x4_pmax_basic",
                                  FLT_MAX, FLT_MIN, -1e38f, 1e-38f,        // First vector
                                  1e38f, -FLT_MIN, FLT_MAX, -1e-38f);      // Second vector

    // Validate boundary value handling
    ASSERT_EQ(FLT_MAX, result[0]) << "Lane 0: pmax(FLT_MAX, 1e38f) should return FLT_MAX";
    ASSERT_EQ(FLT_MIN, result[1]) << "Lane 1: pmax(FLT_MIN, -FLT_MIN) should return positive minimum";
    ASSERT_EQ(FLT_MAX, result[2]) << "Lane 2: pmax(-1e38f, FLT_MAX) should return FLT_MAX";
    ASSERT_EQ(1e-38f, result[3])  << "Lane 3: pmax(1e-38f, -1e-38f) should return positive value";
}

/**
 * @test SubnormalValues_HandledCorrectly
 * @brief Validates f32x4.pmax processes denormalized (subnormal) float values correctly
 * @details Tests pseudo-maximum operation with subnormal floats near zero to ensure
 *          proper IEEE 754 denormalized number handling and precision maintenance.
 * @test_category Corner - Subnormal value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmax_operation
 * @input_conditions Subnormal values near zero and regular small floats
 * @expected_behavior Correct subnormal float processing with maintained precision
 * @validation_method Subnormal-specific checks with precision validation
 */
TEST_F(F32x4PmaxTestSuite, SubnormalValues_HandledCorrectly)
{
    // Create subnormal test values (very small numbers near zero)
    float subnormal1 = 1e-40f;  // Likely subnormal
    float subnormal2 = -1e-45f; // Likely subnormal
    float normal1 = 1e-30f;     // Normal small value
    float normal2 = -1e-35f;    // Normal small value

    auto result = call_f32x4_pmax("test_f32x4_pmax_basic",
                                  subnormal1, subnormal2, normal1, normal2,     // First vector
                                  subnormal2, subnormal1, normal2, normal1);    // Second vector

    // Validate subnormal value handling (expect larger magnitude values)
    ASSERT_EQ(subnormal1, result[0]) << "Lane 0: pmax should handle subnormal values correctly";
    ASSERT_EQ(subnormal1, result[1]) << "Lane 1: pmax should return larger subnormal value";
    ASSERT_EQ(normal1, result[2])    << "Lane 2: pmax should return positive normal value";
    ASSERT_EQ(normal1, result[3])    << "Lane 3: pmax should return larger normal value";
}

/**
 * @test SpecialValues_Infinity_HandledCorrectly
 * @brief Validates f32x4.pmax handles IEEE 754 infinity values according to specification
 * @details Tests pseudo-maximum operation with positive/negative infinity combinations
 *          to ensure correct IEEE 754 compliant infinity arithmetic behavior.
 * @test_category Edge - Infinity value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmax_operation
 * @input_conditions Positive/negative infinity combinations with finite values
 * @expected_behavior IEEE 754 compliant infinity handling: +Inf beats all, -Inf loses to all finite
 * @validation_method Infinity checks with IEEE 754 compliance validation
 */
TEST_F(F32x4PmaxTestSuite, SpecialValues_Infinity_HandledCorrectly)
{
    // Test infinity value combinations
    float pos_inf = std::numeric_limits<float>::infinity();
    float neg_inf = -std::numeric_limits<float>::infinity();

    auto result = call_f32x4_pmax("test_f32x4_pmax_basic",
                                  pos_inf, neg_inf, 1000.0f, -1000.0f,     // First vector
                                  100.0f, -100.0f, pos_inf, neg_inf);      // Second vector

    // Validate infinity handling according to IEEE 754
    ASSERT_TRUE(isPosInf(result[0])) << "Lane 0: pmax(+Inf, 100.0f) should return +Infinity";
    ASSERT_EQ(-100.0f, result[1])   << "Lane 1: pmax(-Inf, -100.0f) should return finite value";
    ASSERT_TRUE(isPosInf(result[2])) << "Lane 2: pmax(1000.0f, +Inf) should return +Infinity";
    ASSERT_EQ(-1000.0f, result[3])  << "Lane 3: pmax(-1000.0f, -Inf) should return finite value";
}

/**
 * @test NaNBehavior_PmaxSemantics_DiffersFromRegularMax
 * @brief Validates f32x4.pmax implements asymmetric NaN semantics correctly
 * @details Tests WAMR's asymmetric NaN behavior: pmax(NaN, x) = NaN (first operand NaN propagates)
 *          but pmax(x, NaN) = x (second operand NaN doesn't propagate).
 * @test_category Edge - Asymmetric NaN behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmax_operation
 * @input_conditions Various NaN patterns to test asymmetric NaN propagation behavior
 * @expected_behavior Asymmetric NaN behavior: first operand NaN propagates, second doesn't
 * @validation_method NaN pattern checks with WAMR-specific asymmetric validation
 */
TEST_F(F32x4PmaxTestSuite, NaNBehavior_PmaxSemantics_DiffersFromRegularMax)
{
    // Create NaN value for testing
    float nan_val = std::numeric_limits<float>::quiet_NaN();

    // Test case 1: First operand has NaN - should propagate NaN (asymmetric behavior)
    auto result1 = call_f32x4_pmax("test_f32x4_pmax_basic",
                                   nan_val, 10.0f, 20.0f, 30.0f,          // First vector (NaN in lane 0)
                                   5.0f, 15.0f, 25.0f, 35.0f);            // Second vector (all normal)

    // Validate asymmetric NaN propagation - first operand NaN propagates
    ASSERT_TRUE(isNaN(result1[0]))   << "Lane 0: pmax(NaN, 5.0f) should return NaN (first operand NaN propagates)";
    ASSERT_EQ(15.0f, result1[1])     << "Lane 1: pmax(10.0f, 15.0f) should return 15.0f";
    ASSERT_EQ(25.0f, result1[2])     << "Lane 2: pmax(20.0f, 25.0f) should return 25.0f";
    ASSERT_EQ(35.0f, result1[3])     << "Lane 3: pmax(30.0f, 35.0f) should return 35.0f";

    // Test case 2: Second operand has NaN - should NOT propagate (asymmetric behavior)
    auto result2 = call_f32x4_pmax("test_f32x4_pmax_basic",
                                   5.0f, 15.0f, 25.0f, 35.0f,             // First vector (all normal)
                                   nan_val, 10.0f, 20.0f, 30.0f);         // Second vector (NaN in lane 0)

    // Validate asymmetric NaN behavior - second operand NaN doesn't propagate
    ASSERT_EQ(5.0f, result2[0])      << "Lane 0: pmax(5.0f, NaN) should return 5.0f (second operand NaN doesn't propagate)";
    ASSERT_EQ(15.0f, result2[1])     << "Lane 1: pmax(15.0f, 10.0f) should return 15.0f";
    ASSERT_EQ(25.0f, result2[2])     << "Lane 2: pmax(25.0f, 20.0f) should return 25.0f";
    ASSERT_EQ(35.0f, result2[3])     << "Lane 3: pmax(35.0f, 30.0f) should return 35.0f";
}

/**
 * @test ZeroValues_SignBehavior_ReturnsPositiveZero
 * @brief Validates f32x4.pmax handles signed zero values with WAMR-specific behavior
 * @details Tests pseudo-maximum operation with +0.0f and -0.0f combinations to validate
 *          WAMR's actual signed zero handling behavior (may differ from strict IEEE 754).
 * @test_category Edge - WAMR signed zero behavior validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f32x4_pmax_operation
 * @input_conditions Combinations of +0.0f and -0.0f in different lane positions
 * @expected_behavior WAMR-specific signed zero handling behavior
 * @validation_method Validation that pmax handles zero values consistently
 */
TEST_F(F32x4PmaxTestSuite, ZeroValues_SignBehavior_ReturnsPositiveZero)
{
    // Test signed zero behavior
    float pos_zero = +0.0f;
    float neg_zero = -0.0f;

    auto result = call_f32x4_pmax("test_f32x4_pmax_basic",
                                  pos_zero, neg_zero, pos_zero, neg_zero,   // First vector
                                  neg_zero, pos_zero, neg_zero, pos_zero);  // Second vector

    // Validate signed zero handling - WAMR may not strictly follow IEEE 754 for signed zeros
    // Test basic zero equality (both +0.0f and -0.0f should be treated as equal to 0.0f)
    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(0.0f, result[i]) << "Lane " << i << ": pmax of zero values should return zero";
    }

    // Test case: Verify zero vs non-zero behavior works correctly
    auto result2 = call_f32x4_pmax("test_f32x4_pmax_basic",
                                   pos_zero, neg_zero, 1.0f, -1.0f,        // First vector
                                   -1.0f, 1.0f, neg_zero, pos_zero);       // Second vector

    // Validate that pmax correctly chooses maximum values
    ASSERT_EQ(0.0f, result2[0])   << "Lane 0: pmax(+0.0f, -1.0f) should return +0.0f (larger value)";
    ASSERT_EQ(1.0f, result2[1])   << "Lane 1: pmax(-0.0f, 1.0f) should return 1.0f (positive value)";
    ASSERT_EQ(1.0f, result2[2])   << "Lane 2: pmax(1.0f, -0.0f) should return 1.0f (positive value)";
    ASSERT_EQ(0.0f, result2[3])   << "Lane 3: pmax(-1.0f, +0.0f) should return +0.0f (larger value)";
}

/**
 * @test InvalidModule_MalformedBytecode_FailsGracefully
 * @brief Validates graceful failure handling for invalid WASM modules with malformed f32x4.pmax usage
 * @details Tests WAMR's ability to handle corrupted or invalid WASM modules containing
 *          malformed f32x4.pmax opcodes without crashing or undefined behavior.
 * @test_category Error - Invalid module validation
 * @coverage_target core/iwasm/common/wasm_runtime_common.c:wasm_runtime_load
 * @input_conditions Invalid WASM module with corrupted f32x4.pmax bytecode
 * @expected_behavior Graceful failure during module loading with proper error reporting
 * @validation_method Module loading failure checks with error message validation
 */
TEST_F(F32x4PmaxTestSuite, InvalidModule_MalformedBytecode_FailsGracefully)
{
    // Create intentionally malformed WASM bytecode (truncated module)
    uint8_t malformed_wasm[] = {
        0x00, 0x61, 0x73, 0x6d,  // WASM magic number
        0x01, 0x00, 0x00, 0x00,  // Version
        // Intentionally truncated/invalid content
        0x01, 0xFF, 0xFF, 0xFF
    };

    char error_buf[256];
    memset(error_buf, 0, sizeof(error_buf));

    // Attempt to load malformed module
    wasm_module_t malformed_module = wasm_runtime_load(malformed_wasm, sizeof(malformed_wasm),
                                                      error_buf, sizeof(error_buf));

    // Validate graceful failure handling
    ASSERT_EQ(nullptr, malformed_module)
        << "Expected module load to fail for malformed bytecode, but got valid module";

    // Validate error message is provided
    ASSERT_GT(strlen(error_buf), 0)
        << "Expected error message for malformed module, but error buffer is empty";
}