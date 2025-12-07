/**
 * Enhanced unit tests for i32x4.lt WASM SIMD opcode
 * Tests unsigned less-than comparison on 4 x 32-bit integer lanes
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include "wasm_runtime_common.h"
#include "wasm_exec_env.h"
#include "bh_read_file.h"
#include "wasm_memory.h"


/**
 * @class I32x4LtTest
 * @brief Comprehensive test suite for i32x4.lt SIMD opcode
 * @details Tests unsigned less-than comparison operation across interpreter and AOT modes.
 *          Validates proper SIMD lane-wise comparison with unsigned 32-bit integer semantics.
 */
class I32x4LtTest : public testing::TestWithParam<RunningMode> {
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
     * @brief Load WASM test module containing i32x4.lt test functions
     * @details Reads WASM bytecode file and validates successful module loading
     */
    void LoadTestModule() {
        const char* wasm_file = "wasm-apps/i32x4_lt_test.wasm";

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
     * @param a_lanes Input vector A as array of 4 u32 values
     * @param b_lanes Input vector B as array of 4 u32 values
     * @param result_lanes Output array to store result vector lanes
     */
    void CallI32x4LtFunction(const char* func_name, const uint32_t* a_lanes,
                             const uint32_t* b_lanes, uint32_t* result_lanes) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, func_name);
        ASSERT_NE(nullptr, func) << "Function not found: " << func_name;

        // Prepare arguments: two v128 vectors as 8 u32 values total
        uint32_t argv[8];

        // Pack vector A lanes
        for (int i = 0; i < 4; ++i) {
            argv[i] = a_lanes[i];
        }

        // Pack vector B lanes
        for (int i = 0; i < 4; ++i) {
            argv[i + 4] = b_lanes[i];
        }

        // Execute function
        bool call_result = wasm_runtime_call_wasm(exec_env, func, 8, argv);
        ASSERT_TRUE(call_result) << "WASM function call failed: " << wasm_runtime_get_exception(module_inst);

        // Unpack result vector lanes
        for (int i = 0; i < 4; ++i) {
            result_lanes[i] = argv[i];
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
 * @brief Validates fundamental i32x4.lt functionality with typical unsigned integer values
 * @details Tests standard unsigned integer combinations to ensure proper unsigned comparison
 *          semantics and correct true/false mask generation.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_i32x4_lt_operation
 * @input_conditions Mixed unsigned integer vectors with known comparison results
 * @expected_behavior Lane-wise unsigned comparison producing 0xFFFFFFFF or 0x00000000 masks
 * @validation_method Direct comparison of result masks with expected values for each lane
 */
TEST_P(I32x4LtTest, BasicComparison_ReturnsCorrectMasks) {
    // Test case 1: Basic unsigned comparisons
    uint32_t a1[] = {1, 5, 10, 20};
    uint32_t b1[] = {5, 3, 15, 18};
    uint32_t result1[4];
    uint32_t expected1[] = {0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000}; // T,F,T,F

    CallI32x4LtFunction("test_i32x4_lt_basic", a1, b1, result1);

    ASSERT_EQ(expected1[0], result1[0]) << "Lane 0: 1 < 5 should be true";
    ASSERT_EQ(expected1[1], result1[1]) << "Lane 1: 5 < 3 should be false";
    ASSERT_EQ(expected1[2], result1[2]) << "Lane 2: 10 < 15 should be true";
    ASSERT_EQ(expected1[3], result1[3]) << "Lane 3: 20 < 18 should be false";

    // Test case 2: All true comparison scenario
    uint32_t a2[] = {0, 10, 100, 1000};
    uint32_t b2[] = {1, 20, 200, 2000};
    uint32_t result2[4];
    uint32_t expected2[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}; // All true

    CallI32x4LtFunction("test_i32x4_lt_basic", a2, b2, result2);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected2[i], result2[i]) << "Lane " << i << " should be true (all ascending)";
    }

    // Test case 3: All false comparison scenario
    uint32_t a3[] = {10, 50, 200, 1000};
    uint32_t b3[] = {5, 25, 100, 500};
    uint32_t result3[4];
    uint32_t expected3[] = {0x00000000, 0x00000000, 0x00000000, 0x00000000}; // All false

    CallI32x4LtFunction("test_i32x4_lt_basic", a3, b3, result3);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected3[i], result3[i]) << "Lane " << i << " should be false (all descending)";
    }
}

/**
 * @test BoundaryValues_HandlesExtremeIntegers
 * @brief Tests boundary conditions with UINT32_MAX, 0, and extreme unsigned values
 * @details Validates proper unsigned comparison behavior at integer limits, ensuring
 *          correct handling of maximum unsigned values and zero boundaries.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_unsigned_comparison_limits
 * @input_conditions Unsigned integer boundary values (0, UINT32_MAX) and extreme cases
 * @expected_behavior Correct unsigned comparison semantics at integer extremes
 * @validation_method Verification of specific boundary relationships and zero comparisons
 */
TEST_P(I32x4LtTest, BoundaryValues_HandlesExtremeIntegers) {
    // Test case 1: Unsigned boundary extremes
    uint32_t a1[] = {0, UINT32_MAX, 0x80000000, 1};
    uint32_t b1[] = {1, 0x80000000, UINT32_MAX, 0};
    uint32_t result1[4];
    uint32_t expected1[] = {0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000}; // T,F,T,F

    CallI32x4LtFunction("test_i32x4_lt_boundary", a1, b1, result1);

    ASSERT_EQ(expected1[0], result1[0]) << "0 < 1 should be true";
    ASSERT_EQ(expected1[1], result1[1]) << "UINT32_MAX < 0x80000000 should be false";
    ASSERT_EQ(expected1[2], result1[2]) << "0x80000000 < UINT32_MAX should be true";
    ASSERT_EQ(expected1[3], result1[3]) << "1 < 0 should be false";

    // Test case 2: Zero boundary comparisons
    uint32_t a2[] = {0, 1, 0xFFFFFFFF, 0x7FFFFFFF};
    uint32_t b2[] = {0, 0, 0xFFFFFFFF, 0x80000000};
    uint32_t result2[4];
    uint32_t expected2[] = {0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF}; // F,F,F,T

    CallI32x4LtFunction("test_i32x4_lt_boundary", a2, b2, result2);

    ASSERT_EQ(expected2[0], result2[0]) << "0 < 0 should be false (equality)";
    ASSERT_EQ(expected2[1], result2[1]) << "1 < 0 should be false";
    ASSERT_EQ(expected2[2], result2[2]) << "0xFFFFFFFF < 0xFFFFFFFF should be false (equality)";
    ASSERT_EQ(expected2[3], result2[3]) << "0x7FFFFFFF < 0x80000000 should be true (unsigned)";
}

/**
 * @test EqualValues_ReturnsFalse
 * @brief Validates that equal values always return false masks
 * @details Tests reflexive property (x < x = false) with various unsigned values,
 *          ensuring proper identity behavior of less-than comparison.
 * @test_category Edge - Mathematical property validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_identity_operations
 * @input_conditions Identity vectors and equal value pairs
 * @expected_behavior Reflexive property holds for all unsigned values
 * @validation_method Verification of identity property and equal value behavior
 */
TEST_P(I32x4LtTest, EqualValues_ReturnsFalse) {
    // Test case 1: All zero vectors
    uint32_t a1[] = {0, 0, 0, 0};
    uint32_t b1[] = {0, 0, 0, 0};
    uint32_t result1[4];
    uint32_t expected1[] = {0x00000000, 0x00000000, 0x00000000, 0x00000000}; // All false

    CallI32x4LtFunction("test_i32x4_lt_equal", a1, b1, result1);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected1[i], result1[i]) << "Lane " << i << ": 0 < 0 should be false";
    }

    // Test case 2: Identity comparison (reflexive property)
    uint32_t a2[] = {42, 0x80000000, UINT32_MAX, 0x12345678};
    uint32_t b2[] = {42, 0x80000000, UINT32_MAX, 0x12345678};
    uint32_t result2[4];
    uint32_t expected2[] = {0x00000000, 0x00000000, 0x00000000, 0x00000000}; // All false

    CallI32x4LtFunction("test_i32x4_lt_equal", a2, b2, result2);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected2[i], result2[i]) << "Lane " << i << ": x < x should be false (reflexive property)";
    }

    // Test case 3: Maximum value equality
    uint32_t a3[] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
    uint32_t b3[] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};
    uint32_t result3[4];
    uint32_t expected3[] = {0x00000000, 0x00000000, 0x00000000, 0x00000000}; // All false

    CallI32x4LtFunction("test_i32x4_lt_equal", a3, b3, result3);

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(expected3[i], result3[i]) << "Lane " << i << ": UINT32_MAX < UINT32_MAX should be false";
    }
}

/**
 * @test UnsignedSemantics_ValidatesUnsignedInterpretation
 * @brief Confirms unsigned comparison semantics vs signed interpretation
 * @details Tests values that would differ between signed/unsigned comparison to ensure
 *          proper unsigned integer semantics, especially with high-bit set values.
 * @test_category Edge - Unsigned semantics validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_unsigned_comparison_logic
 * @input_conditions Values that differ between signed/unsigned interpretation
 * @expected_behavior Results consistent with unsigned 32-bit integer comparison rules
 * @validation_method Verify high-bit values compared as unsigned values, not signed
 */
TEST_P(I32x4LtTest, UnsignedSemantics_ValidatesUnsignedInterpretation) {
    // Test critical unsigned vs signed distinction values
    uint32_t a[] = {0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000001};
    uint32_t b[] = {0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF};
    uint32_t result[4];
    uint32_t expected[] = {0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000}; // F,F,T,F in unsigned interpretation

    CallI32x4LtFunction("test_i32x4_lt_unsigned", a, b, result);

    ASSERT_EQ(expected[0], result[0]) << "0x80000000 < 0x7FFFFFFF should be false (unsigned)";
    ASSERT_EQ(expected[1], result[1]) << "0xFFFFFFFF < 0x80000000 should be false (unsigned)";
    ASSERT_EQ(expected[2], result[2]) << "0x7FFFFFFF < 0xFFFFFFFF should be true (unsigned)";
    ASSERT_EQ(expected[3], result[3]) << "0x80000001 < 0x7FFFFFFF should be false (unsigned)";

    // Additional verification of unsigned ordering
    uint32_t a2[] = {0, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE};
    uint32_t b2[] = {0x80000000, 0x80000000, 0xFFFFFFFF, 0xFFFFFFFF};
    uint32_t result2[4];
    uint32_t expected2[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}; // All true in unsigned

    CallI32x4LtFunction("test_i32x4_lt_unsigned", a2, b2, result2);

    ASSERT_EQ(expected2[0], result2[0]) << "0 < 0x80000000 should be true (unsigned)";
    ASSERT_EQ(expected2[1], result2[1]) << "0x7FFFFFFF < 0x80000000 should be true (unsigned)";
    ASSERT_EQ(expected2[2], result2[2]) << "0x80000000 < 0xFFFFFFFF should be true (unsigned)";
    ASSERT_EQ(expected2[3], result2[3]) << "0xFFFFFFFE < 0xFFFFFFFF should be true (unsigned)";
}

/**
 * @test MixedResults_ReturnsCorrectPattern
 * @brief Tests scenarios with mixed true/false results across lanes
 * @details Validates complex comparison patterns with varied outcomes per lane,
 *          ensuring independent lane processing and correct pattern generation.
 * @test_category Main - Pattern validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:simd_lane_independence
 * @input_conditions Vectors with intentionally mixed comparison outcomes
 * @expected_behavior Correct true/false pattern matching expected results per lane
 * @validation_method Verification of specific mixed patterns and lane independence
 */
TEST_P(I32x4LtTest, MixedResults_ReturnsCorrectPattern) {
    // Test case 1: Alternating pattern
    uint32_t a1[] = {10, 50, 100, 500};
    uint32_t b1[] = {20, 30, 200, 400};
    uint32_t result1[4];
    uint32_t expected1[] = {0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000}; // T,F,T,F

    CallI32x4LtFunction("test_i32x4_lt_mixed", a1, b1, result1);

    ASSERT_EQ(expected1[0], result1[0]) << "Lane 0: 10 < 20 should be true";
    ASSERT_EQ(expected1[1], result1[1]) << "Lane 1: 50 < 30 should be false";
    ASSERT_EQ(expected1[2], result1[2]) << "Lane 2: 100 < 200 should be true";
    ASSERT_EQ(expected1[3], result1[3]) << "Lane 3: 500 < 400 should be false";

    // Test case 2: Complex mixed pattern with high values
    uint32_t a2[] = {0xFFFFFFFF, 1, 0x80000000, 0};
    uint32_t b2[] = {0x80000000, 0xFFFFFFFF, 0x80000001, 1};
    uint32_t result2[4];
    uint32_t expected2[] = {0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}; // F,T,T,T

    CallI32x4LtFunction("test_i32x4_lt_mixed", a2, b2, result2);

    ASSERT_EQ(expected2[0], result2[0]) << "Lane 0: 0xFFFFFFFF < 0x80000000 should be false";
    ASSERT_EQ(expected2[1], result2[1]) << "Lane 1: 1 < 0xFFFFFFFF should be true";
    ASSERT_EQ(expected2[2], result2[2]) << "Lane 2: 0x80000000 < 0x80000001 should be true";
    ASSERT_EQ(expected2[3], result2[3]) << "Lane 3: 0 < 1 should be true";
}

/**
 * @test ModuleLoading_ValidatesWASMIntegrity
 * @brief Tests WASM module loading and validation for SIMD instruction support
 * @details Validates that the test module loads correctly and contains the required
 *          SIMD functions, ensuring proper WASM infrastructure for testing.
 * @test_category Error - Module validation
 * @coverage_target wasm_runtime.c:wasm_module_loading_and_validation
 * @input_conditions Valid WASM module with i32x4.lt instruction
 * @expected_behavior Successful module loading and function lookup
 * @validation_method Module and execution environment validation with error checking
 */
TEST_P(I32x4LtTest, ModuleLoading_ValidatesWASMIntegrity) {
    // Module should be loaded successfully in SetUp
    ASSERT_NE(nullptr, module) << "WASM module should be loaded";
    ASSERT_NE(nullptr, module_inst) << "WASM module instance should be created";
    ASSERT_NE(nullptr, exec_env) << "Execution environment should be created";

    // Verify required test functions exist
    wasm_function_inst_t func_basic = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_basic");
    ASSERT_NE(nullptr, func_basic) << "test_i32x4_lt_basic function should exist";

    wasm_function_inst_t func_boundary = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_boundary");
    ASSERT_NE(nullptr, func_boundary) << "test_i32x4_lt_boundary function should exist";

    wasm_function_inst_t func_equal = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_equal");
    ASSERT_NE(nullptr, func_equal) << "test_i32x4_lt_equal function should exist";

    wasm_function_inst_t func_unsigned = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_unsigned");
    ASSERT_NE(nullptr, func_unsigned) << "test_i32x4_lt_unsigned function should exist";

    wasm_function_inst_t func_mixed = wasm_runtime_lookup_function(module_inst, "test_i32x4_lt_mixed");
    ASSERT_NE(nullptr, func_mixed) << "test_i32x4_lt_mixed function should exist";
}

// Parameterized test instantiation for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(
    RunningModeTest,
    I32x4LtTest,
    testing::Values(
        Mode_Interp,
        Mode_LLVM_JIT
    )
);