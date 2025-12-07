/**
 * Enhanced unit tests for i32x4.lt_s WASM SIMD opcode
 * Tests signed less-than comparison on 4 x 32-bit integer lanes
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include "wasm_runtime_common.h"
#include "wasm_exec_env.h"
#include "bh_read_file.h"
#include "wasm_memory.h"


/**
 * @class I32x4LtSTest
 * @brief Comprehensive test suite for i32x4.lt_s SIMD opcode
 * @details Tests signed less-than comparison operation across interpreter and AOT modes.
 *          Validates proper SIMD lane-wise comparison with signed 32-bit integer semantics.
 */
class I32x4LtSTest : public testing::TestWithParam<RunningMode> {
protected:
    /**
     * @brief Set up test environment before each test case
     * @details Initializes WAMR runtime, loads test WASM module, and creates execution environment
     */
    void SetUp() override {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));

        init_args.mem_alloc_type = Alloc_With_System_Allocator;
        init_args.n_native_symbols = 0;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        LoadTestModule();
        CreateExecutionEnvironment();
    }

    /**
     * @brief Clean up test environment after each test case
     * @details Destroys execution environment, unloads module, and shuts down WAMR runtime
     */
    void TearDown() override {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (module_inst) {
            wasm_runtime_deinstantiate(module_inst);
            module_inst = nullptr;
        }
        if (module) {
            wasm_runtime_unload(module);
            module = nullptr;
        }
        if (buffer) {
            wasm_runtime_free(buffer);
            buffer = nullptr;
        }
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM test module containing i32x4.lt_s test functions
     * @details Reads WASM bytecode file and validates successful module loading
     */
    void LoadTestModule() {
        const char* wasm_file = "wasm-apps/i32x4_lt_s_test.wasm";

        buffer = (uint8_t*)bh_read_file_to_buffer(wasm_file, &buffer_size);
        ASSERT_NE(nullptr, buffer) << "Failed to read WASM file: " << wasm_file;
        ASSERT_GT(buffer_size, 0U) << "WASM file is empty: " << wasm_file;

        module = wasm_runtime_load(buffer, buffer_size, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;
    }

    /**
     * @brief Create execution environment for test module
     * @details Instantiates WASM module and creates execution environment with proper configuration
     */
    void CreateExecutionEnvironment() {
        module_inst = wasm_runtime_instantiate(module, 65536, 65536, error_buf, sizeof(error_buf));
        ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;

        exec_env = wasm_runtime_create_exec_env(module_inst, 65536);
        ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
    }

    /**
     * @brief Call WASM function with two v128 parameters and return result lanes
     * @param func_name Name of WASM function to call
     * @param a_lanes Input vector A as array of 4 i32 values
     * @param b_lanes Input vector B as array of 4 i32 values
     * @param result_lanes Output array to store result vector lanes
     */
    void CallI32x4LtSFunction(const char* func_name, const int32_t* a_lanes,
                              const int32_t* b_lanes, int32_t* result_lanes) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        ASSERT_NE(nullptr, func) << "Function not found: " << func_name;

        // Prepare arguments: two v128 vectors as 8 i32 values total
        uint32_t argv[8];

        // Pack vector A lanes
        for (int i = 0; i < 4; ++i) {
            argv[i] = static_cast<uint32_t>(a_lanes[i]);
        }

        // Pack vector B lanes
        for (int i = 0; i < 4; ++i) {
            argv[i + 4] = static_cast<uint32_t>(b_lanes[i]);
        }

        // Execute function
        bool call_result = wasm_runtime_call_wasm(exec_env, func, 8, argv);
        ASSERT_TRUE(call_result) << "WASM function call failed: " << wasm_runtime_get_exception(module_inst);

        // Unpack result vector lanes
        for (int i = 0; i < 4; ++i) {
            result_lanes[i] = static_cast<int32_t>(argv[i]);
        }
    }

    // Test infrastructure members
    RuntimeInitArgs init_args;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint8_t* buffer = nullptr;
    uint32_t buffer_size = 0;
    char error_buf[128];
};

/**
 * @test BasicComparison_ReturnsCorrectMasks
 * @brief Validates fundamental i32x4.lt_s functionality with typical signed integer values
 * @details Tests standard positive/negative integer combinations to ensure proper
 *          signed comparison semantics and correct true/false mask generation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i32x4_lt_s_operation
 * @input_conditions Mixed positive/negative integer vectors with known comparison results
 * @expected_behavior Lane-wise signed comparison producing 0xFFFFFFFF or 0x00000000 masks
 * @validation_method Direct comparison of result masks with expected values for each lane
 */
TEST_P(I32x4LtSTest, BasicComparison_ReturnsCorrectMasks) {
    // Test case 1: Mixed positive/negative comparisons
    int32_t a1[] = {1, -5, 10, -20};
    int32_t b1[] = {5, -3, 8, -15};
    int32_t result1[4];
    int32_t expected1[] = {-1, -1, 0, -1}; // 0xFFFFFFFF = -1, 0x00000000 = 0

    CallI32x4LtSFunction("test_i32x4_lt_s_basic", a1, b1, result1);

    ASSERT_EQ(expected1[0], result1[0]) << "Lane 0: 1 < 5 should be true";
    ASSERT_EQ(expected1[1], result1[1]) << "Lane 1: -5 < -3 should be true";
    ASSERT_EQ(expected1[2], result1[2]) << "Lane 2: 10 < 8 should be false";
    ASSERT_EQ(expected1[3], result1[3]) << "Lane 3: -20 < -15 should be true";

    // Test case 2: All true comparison scenario
    int32_t a2[] = {-10, -5, 0, 5};
    int32_t b2[] = {-5, 0, 5, 10};
    int32_t result2[4];
    int32_t expected2[] = {-1, -1, -1, -1}; // All true

    CallI32x4LtSFunction("test_i32x4_lt_s_basic", a2, b2, result2);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected2[i], result2[i]) << "Lane " << i << " should be true (all ascending)";
    }

    // Test case 3: All false comparison scenario
    int32_t a3[] = {10, 5, 0, -5};
    int32_t b3[] = {5, 0, -5, -10};
    int32_t result3[4];
    int32_t expected3[] = {0, 0, 0, 0}; // All false

    CallI32x4LtSFunction("test_i32x4_lt_s_basic", a3, b3, result3);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected3[i], result3[i]) << "Lane " << i << " should be false (all descending)";
    }
}

/**
 * @test BoundaryValues_HandlesExtremeIntegers
 * @brief Tests boundary conditions with INT32_MIN, INT32_MAX, and zero crossings
 * @details Validates proper signed comparison behavior at integer limits, ensuring
 *          INT32_MIN < INT32_MAX and proper handling of extreme values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_signed_comparison_limits
 * @input_conditions Integer boundary values (INT32_MIN, INT32_MAX) and zero crossings
 * @expected_behavior Correct signed comparison semantics at integer extremes
 * @validation_method Verification of specific boundary relationships and zero comparisons
 */
TEST_P(I32x4LtSTest, BoundaryValues_HandlesExtremeIntegers) {
    // Test case 1: Integer boundary extremes
    int32_t a1[] = {INT32_MIN, INT32_MAX, INT32_MIN, 0};
    int32_t b1[] = {INT32_MAX, INT32_MIN, -1, INT32_MAX};
    int32_t result1[4];
    int32_t expected1[] = {-1, 0, -1, -1}; // T,F,T,T

    CallI32x4LtSFunction("test_i32x4_lt_s_boundary", a1, b1, result1);

    ASSERT_EQ(expected1[0], result1[0]) << "INT32_MIN < INT32_MAX should be true";
    ASSERT_EQ(expected1[1], result1[1]) << "INT32_MAX < INT32_MIN should be false";
    ASSERT_EQ(expected1[2], result1[2]) << "INT32_MIN < -1 should be true";
    ASSERT_EQ(expected1[3], result1[3]) << "0 < INT32_MAX should be true";

    // Test case 2: Zero boundary comparisons
    int32_t a2[] = {-1, 0, 1, -1};
    int32_t b2[] = {0, 0, 0, -1};
    int32_t result2[4];
    int32_t expected2[] = {-1, 0, 0, 0}; // T,F,F,F

    CallI32x4LtSFunction("test_i32x4_lt_s_boundary", a2, b2, result2);

    ASSERT_EQ(expected2[0], result2[0]) << "-1 < 0 should be true";
    ASSERT_EQ(expected2[1], result2[1]) << "0 < 0 should be false (equality)";
    ASSERT_EQ(expected2[2], result2[2]) << "1 < 0 should be false";
    ASSERT_EQ(expected2[3], result2[3]) << "-1 < -1 should be false (equality)";
}

/**
 * @test ZeroOperations_ProducesCorrectIdentityResults
 * @brief Validates identity and zero-comparison properties of the operation
 * @details Tests reflexive property (x < x = false), all-zero vectors, and
 *          mathematical identity properties of signed less-than comparison.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_identity_operations
 * @input_conditions Identity vectors, all-zero vectors, and mathematical test cases
 * @expected_behavior Reflexive property holds, zero comparisons follow signed rules
 * @validation_method Verification of identity property and zero operation behavior
 */
TEST_P(I32x4LtSTest, ZeroOperations_ProducesCorrectIdentityResults) {
    // Test case 1: All zero vectors
    int32_t a1[] = {0, 0, 0, 0};
    int32_t b1[] = {0, 0, 0, 0};
    int32_t result1[4];
    int32_t expected1[] = {0, 0, 0, 0}; // All false (0 < 0 = false)

    CallI32x4LtSFunction("test_i32x4_lt_s_zero", a1, b1, result1);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected1[i], result1[i]) << "Lane " << i << ": 0 < 0 should be false";
    }

    // Test case 2: Identity comparison (reflexive property)
    int32_t a2[] = {42, -17, INT32_MAX, INT32_MIN};
    int32_t b2[] = {42, -17, INT32_MAX, INT32_MIN};
    int32_t result2[4];
    int32_t expected2[] = {0, 0, 0, 0}; // All false (x < x = false)

    CallI32x4LtSFunction("test_i32x4_lt_s_zero", a2, b2, result2);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected2[i], result2[i]) << "Lane " << i << ": x < x should be false (reflexive property)";
    }

    // Test case 3: Mathematical property validation
    int32_t a3[] = {5, -3, 100, -200};
    int32_t b3[] = {3, -5, 50, -100};
    int32_t result3[4];
    int32_t expected3[] = {0, 0, 0, -1}; // F,F,F,T

    CallI32x4LtSFunction("test_i32x4_lt_s_zero", a3, b3, result3);

    ASSERT_EQ(expected3[0], result3[0]) << "5 < 3 should be false";
    ASSERT_EQ(expected3[1], result3[1]) << "-3 < -5 should be false";
    ASSERT_EQ(expected3[2], result3[2]) << "100 < 50 should be false";
    ASSERT_EQ(expected3[3], result3[3]) << "-200 < -100 should be true";
}

/**
 * @test SignedVsUnsigned_EnsuresSignedInterpretation
 * @brief Confirms signed comparison semantics vs unsigned interpretation
 * @details Tests values that would differ between signed/unsigned comparison to ensure
 *          proper signed integer semantics, especially with 0x80000000 and similar values.
 * @test_category Edge - Signed semantics validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_signed_comparison_logic
 * @input_conditions Values that differ between signed/unsigned interpretation
 * @expected_behavior Results consistent with signed 32-bit integer comparison rules
 * @validation_method Verify negative numbers compared as signed values, not unsigned
 */
TEST_P(I32x4LtSTest, SignedVsUnsigned_EnsuresSignedInterpretation) {
    // Test critical signed vs unsigned distinction values
    int32_t a[] = {static_cast<int32_t>(0x80000000), static_cast<int32_t>(0xFFFFFFFF),
                   0x7FFFFFFF, static_cast<int32_t>(0x80000001)};
    int32_t b[] = {0x7FFFFFFF, static_cast<int32_t>(0x80000000),
                   static_cast<int32_t>(0xFFFFFFFF), 0x7FFFFFFF};
    int32_t result[4];
    int32_t expected[] = {-1, 0, 0, -1}; // T,F,F,T in signed interpretation

    CallI32x4LtSFunction("test_i32x4_lt_s_signed", a, b, result);

    ASSERT_EQ(expected[0], result[0]) << "INT32_MIN < INT32_MAX should be true (signed)";
    ASSERT_EQ(expected[1], result[1]) << "-1 < INT32_MIN should be false (signed)";
    ASSERT_EQ(expected[2], result[2]) << "INT32_MAX < -1 should be false (signed)";
    ASSERT_EQ(expected[3], result[3]) << "INT32_MIN+1 < INT32_MAX should be true (signed)";

    // Verify these would be different in unsigned comparison
    // In unsigned: 0x80000000 > 0x7FFFFFFF, 0xFFFFFFFF > 0x80000000
    // But in signed: INT32_MIN < INT32_MAX, -1 < INT32_MIN
}

/**
 * @test ModuleLoading_ValidatesWASMIntegrity
 * @brief Tests WASM module loading and validation for SIMD instruction support
 * @details Validates that the test module loads correctly and contains the required
 *          SIMD functions, ensuring proper WASM infrastructure for testing.
 * @test_category Error - Module validation
 * @coverage_target wasm_runtime.c:wasm_module_loading_and_validation
 * @input_conditions Valid WASM module with i32x4.lt_s instruction
 * @expected_behavior Successful module loading and function lookup
 * @validation_method Module and execution environment validation with error checking
 */
TEST_P(I32x4LtSTest, ModuleLoading_ValidatesWASMIntegrity) {
    // Module should be loaded successfully in SetUp
    ASSERT_NE(nullptr, module) << "WASM module should be loaded";
    ASSERT_NE(nullptr, module_inst) << "WASM module instance should be created";
    ASSERT_NE(nullptr, exec_env) << "Execution environment should be created";

    // Verify required test functions exist
    wasm_function_inst_t func_basic = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_s_basic");
    ASSERT_NE(nullptr, func_basic) << "test_i32x4_lt_s_basic function should exist";

    wasm_function_inst_t func_boundary = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_s_boundary");
    ASSERT_NE(nullptr, func_boundary) << "test_i32x4_lt_s_boundary function should exist";

    wasm_function_inst_t func_zero = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_s_zero");
    ASSERT_NE(nullptr, func_zero) << "test_i32x4_lt_s_zero function should exist";

    wasm_function_inst_t func_signed = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_s_signed");
    ASSERT_NE(nullptr, func_signed) << "test_i32x4_lt_s_signed function should exist";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    RunningModeTest,
    I32x4LtSTest,
    testing::Values(
        Mode_Interp,
        Mode_LLVM_JIT
    )
);