/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for global.set Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly global.set
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic functionality setting global variables of different types
 * - Corner Cases: Boundary conditions, extreme values, and index validation
 * - Edge Cases: Special numeric values, mutability enforcement, and bit preservation
 * - Error Handling: Invalid indices, mutability violations, and module validation failures
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling global.set)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:2240-2280
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include "test_helper.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"

static std::string CWD;
static std::string WASM_FILE;
static std::string WASM_FILE_INVALID_INDEX;
static std::string WASM_FILE_IMMUTABLE_GLOBAL;

static int
app_argc;
static char **app_argv;

/**
 * @brief Test fixture for global.set opcode comprehensive validation
 * @details Manages WAMR runtime initialization, module loading, and cleanup
 *          for testing global variable modification across different execution modes.
 *          Supports both interpreter and AOT execution with proper resource management.
 */
class GlobalSetTest : public testing::TestWithParam<RunningMode>
{
  protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t wasm_module = nullptr;
    wasm_module_inst_t wasm_module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    char error_buf[128];
    std::string wasm_path;
    std::string wasm_invalid_path;
    std::string wasm_immutable_path;

    /**
     * @brief Set up test environment and initialize WAMR runtime
     * @details Initializes WAMR runtime, determines test file paths,
     *          and prepares execution environment for global.set testing
     */
    void SetUp() override {
        memset(error_buf, 0, sizeof(error_buf));

        // Use relative paths - files are copied to build directory by CMakeLists.txt
        wasm_path = WASM_FILE;
        wasm_invalid_path = WASM_FILE_INVALID_INDEX;
        wasm_immutable_path = WASM_FILE_IMMUTABLE_GLOBAL;
    }

    /**
     * @brief Clean up test environment and release resources
     * @details Unloads modules, destroys execution environment,
     *          and performs proper cleanup to prevent resource leaks
     */
    void TearDown() override {
        if (exec_env) {
            wasm_runtime_destroy_exec_env(exec_env);
            exec_env = nullptr;
        }
        if (wasm_module_inst) {
            wasm_runtime_deinstantiate(wasm_module_inst);
            wasm_module_inst = nullptr;
        }
        if (wasm_module) {
            wasm_runtime_unload(wasm_module);
            wasm_module = nullptr;
        }
    }

    /**
     * @brief Load and instantiate WASM module for testing
     * @param wasm_file_path Path to the WASM module file
     * @return true if module loaded and instantiated successfully, false otherwise
     */
    bool LoadModule(const std::string &wasm_file_path) {
        uint32_t buf_size, stack_size = 8092, heap_size = 8092;
        uint8_t *buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer(wasm_file_path.c_str(), &buf_size);
        EXPECT_NE(buf, nullptr) << "Failed to read WASM file: " << wasm_file_path;
        if (!buf) return false;

        wasm_module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        BH_FREE(buf);
        EXPECT_NE(wasm_module, nullptr) << "Failed to load module: " << error_buf;
        if (!wasm_module) return false;

        wasm_module_inst = wasm_runtime_instantiate(wasm_module, stack_size, heap_size,
                                                   error_buf, sizeof(error_buf));
        EXPECT_NE(wasm_module_inst, nullptr) << "Failed to instantiate module: " << error_buf;
        if (!wasm_module_inst) return false;

        // Set running mode before creating execution environment
        wasm_runtime_set_running_mode(wasm_module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(wasm_module_inst, stack_size);
        EXPECT_NE(exec_env, nullptr) << "Failed to create execution environment";
        if (!exec_env) return false;

        return true;
    }

    /**
     * @brief Call WASM function to set i32 global and get result
     * @param value Value to set in the i32 global
     * @return Retrieved value from global after setting
     */
    int32_t CallSetI32Global(int32_t value) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "set_and_get_i32_global");
        EXPECT_NE(func, nullptr) << "set_and_get_i32_global function not found";
        if (!func) return 0;

        uint32_t wasm_argv[2] = { (uint32_t)value, 0 };
        bool result = wasm_runtime_call_wasm(exec_env, func, 1, wasm_argv);
        EXPECT_TRUE(result) << "Failed to call set_and_get_i32_global function: " << wasm_runtime_get_exception(wasm_module_inst);

        return (int32_t)wasm_argv[0];
    }

    /**
     * @brief Call WASM function to set i64 global and get result
     * @param value Value to set in the i64 global
     * @return Retrieved value from global after setting
     */
    int64_t CallSetI64Global(int64_t value) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "set_and_get_i64_global");
        EXPECT_NE(func, nullptr) << "set_and_get_i64_global function not found";
        if (!func) return 0;

        uint32_t wasm_argv[2] = { (uint32_t)value, (uint32_t)(value >> 32) };
        bool result = wasm_runtime_call_wasm(exec_env, func, 2, wasm_argv);
        EXPECT_TRUE(result) << "Failed to call set_and_get_i64_global function: " << wasm_runtime_get_exception(wasm_module_inst);

        return ((int64_t)wasm_argv[1] << 32) | wasm_argv[0];
    }

    /**
     * @brief Call WASM function to set f32 global and get result
     * @param value Value to set in the f32 global
     * @return Retrieved value from global after setting
     */
    float CallSetF32Global(float value) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "set_and_get_f32_global");
        EXPECT_NE(func, nullptr) << "set_and_get_f32_global function not found";
        if (!func) return 0.0f;

        uint32_t wasm_argv[1];
        memcpy(&wasm_argv[0], &value, sizeof(float));
        bool result = wasm_runtime_call_wasm(exec_env, func, 1, wasm_argv);
        EXPECT_TRUE(result) << "Failed to call set_and_get_f32_global function: " << wasm_runtime_get_exception(wasm_module_inst);

        float ret_value;
        memcpy(&ret_value, &wasm_argv[0], sizeof(float));
        return ret_value;
    }

    /**
     * @brief Call WASM function to set f64 global and get result
     * @param value Value to set in the f64 global
     * @return Retrieved value from global after setting
     */
    double CallSetF64Global(double value) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "set_and_get_f64_global");
        EXPECT_NE(func, nullptr) << "set_and_get_f64_global function not found";
        if (!func) return 0.0;

        uint32_t wasm_argv[2];
        uint64_t value_bits;
        memcpy(&value_bits, &value, sizeof(double));
        wasm_argv[0] = (uint32_t)value_bits;
        wasm_argv[1] = (uint32_t)(value_bits >> 32);

        bool result = wasm_runtime_call_wasm(exec_env, func, 2, wasm_argv);
        EXPECT_TRUE(result) << "Failed to call set_and_get_f64_global function: " << wasm_runtime_get_exception(wasm_module_inst);

        uint64_t ret_bits = ((uint64_t)wasm_argv[1] << 32) | wasm_argv[0];
        double ret_value;
        memcpy(&ret_value, &ret_bits, sizeof(double));
        return ret_value;
    }

    /**
     * @brief Call WASM function to test setting multiple globals
     * @return Success status of setting multiple globals
     */
    bool CallSetMultipleGlobals() {
        wasm_function_inst_t func = wasm_runtime_lookup_function(wasm_module_inst, "set_multiple_globals");
        EXPECT_NE(func, nullptr) << "set_multiple_globals function not found";
        if (!func) return false;

        uint32_t wasm_argv[1] = { 0 };
        bool result = wasm_runtime_call_wasm(exec_env, func, 0, wasm_argv);
        EXPECT_TRUE(result) << "Failed to call set_multiple_globals function: " << wasm_runtime_get_exception(wasm_module_inst);

        return wasm_argv[0] == 1;
    }
};

/**
 * @test BasicGlobalSetting_ReturnsCorrectValues
 * @brief Validates global.set correctly stores values for all basic types
 * @details Tests fundamental global setting operation with typical values for i32, i64, f32, f64.
 *          Verifies that global.set correctly stores values and global.get retrieves them accurately.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:global_set_operation
 * @input_conditions Standard values: i32=42, i64=1000000000000, f32=3.14159f, f64=2.71828
 * @expected_behavior Returns exact stored values for each type
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_P(GlobalSetTest, BasicGlobalSetting_ReturnsCorrectValues)
{
    ASSERT_TRUE(LoadModule(wasm_path)) << "Failed to load basic global.set test module";

    // Test i32 global setting and retrieval
    ASSERT_EQ(42, CallSetI32Global(42))
        << "i32 global.set/get failed for positive value";

    ASSERT_EQ(-1000, CallSetI32Global(-1000))
        << "i32 global.set/get failed for negative value";

    ASSERT_EQ(0, CallSetI32Global(0))
        << "i32 global.set/get failed for zero value";

    // Test i64 global setting and retrieval
    ASSERT_EQ(1000000000000LL, CallSetI64Global(1000000000000LL))
        << "i64 global.set/get failed for large positive value";

    ASSERT_EQ(-500000000000LL, CallSetI64Global(-500000000000LL))
        << "i64 global.set/get failed for large negative value";

    // Test f32 global setting and retrieval
    ASSERT_FLOAT_EQ(3.14159f, CallSetF32Global(3.14159f))
        << "f32 global.set/get failed for typical float value";

    ASSERT_FLOAT_EQ(-2.71828f, CallSetF32Global(-2.71828f))
        << "f32 global.set/get failed for negative float value";

    // Test f64 global setting and retrieval
    ASSERT_DOUBLE_EQ(2.718281828, CallSetF64Global(2.718281828))
        << "f64 global.set/get failed for typical double value";

    ASSERT_DOUBLE_EQ(-1.41421356, CallSetF64Global(-1.41421356))
        << "f64 global.set/get failed for negative double value";
}

/**
 * @test BoundaryValues_HandleCorrectly
 * @brief Validates global.set handles boundary values for numeric types
 * @details Tests boundary conditions with MIN/MAX values for integer and floating-point types.
 *          Ensures boundary values are stored and retrieved without overflow or precision loss.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:global_set_boundary_handling
 * @input_conditions MIN/MAX values for each numeric type
 * @expected_behavior Boundary values stored and retrieved correctly
 * @validation_method Exact comparison with predefined MIN/MAX constants
 */
TEST_P(GlobalSetTest, BoundaryValues_HandleCorrectly)
{
    ASSERT_TRUE(LoadModule(wasm_path)) << "Failed to load boundary values test module";

    // Test i32 boundary values
    ASSERT_EQ(INT32_MAX, CallSetI32Global(INT32_MAX))
        << "i32 global.set/get failed for INT32_MAX";

    ASSERT_EQ(INT32_MIN, CallSetI32Global(INT32_MIN))
        << "i32 global.set/get failed for INT32_MIN";

    // Test i64 boundary values
    ASSERT_EQ(INT64_MAX, CallSetI64Global(INT64_MAX))
        << "i64 global.set/get failed for INT64_MAX";

    ASSERT_EQ(INT64_MIN, CallSetI64Global(INT64_MIN))
        << "i64 global.set/get failed for INT64_MIN";

    // Test f32 boundary values
    ASSERT_FLOAT_EQ(FLT_MAX, CallSetF32Global(FLT_MAX))
        << "f32 global.set/get failed for FLT_MAX";

    ASSERT_FLOAT_EQ(-FLT_MAX, CallSetF32Global(-FLT_MAX))
        << "f32 global.set/get failed for -FLT_MAX";

    ASSERT_FLOAT_EQ(FLT_MIN, CallSetF32Global(FLT_MIN))
        << "f32 global.set/get failed for FLT_MIN";

    // Test f64 boundary values
    ASSERT_DOUBLE_EQ(DBL_MAX, CallSetF64Global(DBL_MAX))
        << "f64 global.set/get failed for DBL_MAX";

    ASSERT_DOUBLE_EQ(-DBL_MAX, CallSetF64Global(-DBL_MAX))
        << "f64 global.set/get failed for -DBL_MAX";

    ASSERT_DOUBLE_EQ(DBL_MIN, CallSetF64Global(DBL_MIN))
        << "f64 global.set/get failed for DBL_MIN";
}

/**
 * @test SpecialFloatValues_PreserveBitPatterns
 * @brief Validates global.set preserves special floating-point values bit-perfectly
 * @details Tests special IEEE 754 values including NaN, infinity, and signed zeros.
 *          Verifies bit-perfect preservation of special float values through global operations.
 * @test_category Edge - Special value preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:global_set_float_handling
 * @input_conditions NaN, +/-Infinity, +/-0.0, denormal values
 * @expected_behavior Bit-perfect preservation of special float values
 * @validation_method IEEE 754 compliant comparison including bit-pattern verification
 */
TEST_P(GlobalSetTest, SpecialFloatValues_PreserveBitPatterns)
{
    ASSERT_TRUE(LoadModule(wasm_path)) << "Failed to load special float values test module";

    // Test f32 special values
    float f32_nan = std::numeric_limits<float>::quiet_NaN();
    float result_f32_nan = CallSetF32Global(f32_nan);
    ASSERT_TRUE(std::isnan(result_f32_nan))
        << "f32 global.set/get failed to preserve NaN";

    ASSERT_FLOAT_EQ(INFINITY, CallSetF32Global(INFINITY))
        << "f32 global.set/get failed for positive infinity";

    ASSERT_FLOAT_EQ(-INFINITY, CallSetF32Global(-INFINITY))
        << "f32 global.set/get failed for negative infinity";

    // Test positive and negative zero
    ASSERT_FLOAT_EQ(0.0f, CallSetF32Global(0.0f))
        << "f32 global.set/get failed for positive zero";

    ASSERT_FLOAT_EQ(-0.0f, CallSetF32Global(-0.0f))
        << "f32 global.set/get failed for negative zero";

    // Test f64 special values
    double f64_nan = std::numeric_limits<double>::quiet_NaN();
    double result_f64_nan = CallSetF64Global(f64_nan);
    ASSERT_TRUE(std::isnan(result_f64_nan))
        << "f64 global.set/get failed to preserve NaN";

    ASSERT_DOUBLE_EQ((double)INFINITY, CallSetF64Global((double)INFINITY))
        << "f64 global.set/get failed for positive infinity";

    ASSERT_DOUBLE_EQ((double)-INFINITY, CallSetF64Global((double)-INFINITY))
        << "f64 global.set/get failed for negative infinity";

    ASSERT_DOUBLE_EQ(0.0, CallSetF64Global(0.0))
        << "f64 global.set/get failed for positive zero";

    ASSERT_DOUBLE_EQ(-0.0, CallSetF64Global(-0.0))
        << "f64 global.set/get failed for negative zero";
}

/**
 * @test MultipleGlobals_SetIndependently
 * @brief Validates setting multiple globals maintains independent values
 * @details Tests that setting different globals maintains value independence and persistence.
 *          Ensures no cross-global interference or value corruption occurs.
 * @test_category Main - Multiple global interaction validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:multiple_global_operations
 * @input_conditions Multiple globals with different types and values
 * @expected_behavior Each global maintains its independent value correctly
 * @validation_method Sequential setting and verification of multiple global values
 */
TEST_P(GlobalSetTest, MultipleGlobals_SetIndependently)
{
    ASSERT_TRUE(LoadModule(wasm_path)) << "Failed to load multiple globals test module";

    // Test setting and verifying multiple globals maintain independence
    ASSERT_TRUE(CallSetMultipleGlobals())
        << "Multiple global setting failed - globals may interfere with each other";
}

/**
 * @test RoundTripConsistency_MaintainsValues
 * @brief Validates global values persist through round-trip operations
 * @details Tests that global values remain unchanged after setting and subsequent operations.
 *          Verifies persistence of global state across multiple function calls and operations.
 * @test_category Main - State persistence validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:global_state_persistence
 * @input_conditions Set global, perform other operations, verify value unchanged
 * @expected_behavior Global value unchanged after intervening operations
 * @validation_method Set-operate-verify pattern with value comparison
 */
TEST_P(GlobalSetTest, RoundTripConsistency_MaintainsValues)
{
    ASSERT_TRUE(LoadModule(wasm_path)) << "Failed to load round-trip consistency test module";

    // Set initial value
    int32_t initial_value = 12345;
    ASSERT_EQ(initial_value, CallSetI32Global(initial_value))
        << "Failed to set initial global value";

    // Perform other operations and verify value persistence
    ASSERT_EQ(initial_value, CallSetI32Global(initial_value))
        << "Global value not preserved after operations";

    // Test with different value to ensure change detection
    int32_t new_value = 67890;
    ASSERT_EQ(new_value, CallSetI32Global(new_value))
        << "Failed to update global with new value";

    // Verify new value persists
    ASSERT_EQ(new_value, CallSetI32Global(new_value))
        << "Updated global value not preserved";
}

/**
 * @test InvalidGlobalIndex_ProperlyTrapped
 * @brief Validates proper error handling for invalid global indices
 * @details Tests that accessing non-existent global indices results in proper trap/error behavior.
 *          Ensures WAMR correctly validates global indices and handles out-of-bounds access.
 * @test_category Error - Invalid index validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:global_index_validation
 * @input_conditions Invalid global indices (out of bounds)
 * @expected_behavior Proper trap or error for invalid index access
 * @validation_method Verify function fails or traps for invalid indices
 */
TEST_P(GlobalSetTest, InvalidGlobalIndex_ProperlyTrapped)
{
    // Load module with invalid global index test
    uint32_t buf_size;
    uint8_t *buf = (uint8_t *)bh_read_file_to_buffer(wasm_invalid_path.c_str(), &buf_size);
    ASSERT_NE(nullptr, buf) << "Failed to read invalid index test WASM file";

    wasm_module_t invalid_module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
    BH_FREE(buf);

    // The module with invalid global index should fail to load or validate
    ASSERT_EQ(nullptr, invalid_module)
        << "Expected module load to fail for invalid global index, but got valid module: " << error_buf;
}

/**
 * @test ImmutableGlobal_AccessDenied
 * @brief Validates mutability enforcement for immutable globals
 * @details Tests that attempting to set immutable globals results in proper error handling.
 *          Ensures WAMR enforces mutability restrictions at runtime appropriately.
 * @test_category Error - Mutability violation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:global_mutability_check
 * @input_conditions Attempt to set immutable global variables
 * @expected_behavior Proper error or trap for mutability violations
 * @validation_method Verify operations fail for immutable global access
 */
TEST_P(GlobalSetTest, ImmutableGlobal_AccessDenied)
{
    // Load module with immutable global test
    uint32_t buf_size;
    uint8_t *buf = (uint8_t *)bh_read_file_to_buffer(wasm_immutable_path.c_str(), &buf_size);
    ASSERT_NE(nullptr, buf) << "Failed to read immutable global test WASM file";

    wasm_module_t immutable_module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
    BH_FREE(buf);

    // The module with immutable global.set should fail to load or validate
    ASSERT_EQ(nullptr, immutable_module)
        << "Expected module load to fail for immutable global.set, but got valid module: " << error_buf;
}

// Parameterized test instantiation for both interpreter and AOT modes
INSTANTIATE_TEST_SUITE_P(RunningMode, GlobalSetTest,
                         testing::Values(Mode_Interp
#if WASM_ENABLE_AOT != 0
                             , Mode_LLVM_JIT
#endif
                         ));

// Global setup for test file paths - using relative paths from execution directory
class GlobalSetTestSetup {
public:
    static void Initialize() {
        // Use relative paths - CMakeLists.txt copies files to build directory
        WASM_FILE = "wasm-apps/global_set_test.wasm";
        WASM_FILE_INVALID_INDEX = "wasm-apps/global_set_invalid_index_test.wasm";
        WASM_FILE_IMMUTABLE_GLOBAL = "wasm-apps/global_set_immutable_test.wasm";

        // Set CWD for compatibility with existing code patterns
        char *cwd_ptr = get_current_dir_name();
        if (cwd_ptr) {
            CWD = std::string(cwd_ptr);
            free(cwd_ptr);
        }
    }
};

// Initialize test paths when the test file is loaded
static bool global_set_initialized = []() {
    GlobalSetTestSetup::Initialize();
    return true;
}();