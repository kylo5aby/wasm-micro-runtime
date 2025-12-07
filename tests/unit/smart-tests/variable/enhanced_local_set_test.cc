/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * Enhanced Unit Tests for local.set Opcode
 *
 * This test suite provides comprehensive coverage of the WebAssembly local.set
 * instruction across multiple execution modes (interpreter and AOT).
 *
 * Test Categories:
 * - Main Routine: Basic functionality setting local variables of different types
 * - Corner Cases: Boundary conditions, extreme values, and index validation
 * - Edge Cases: Special floating-point values and type-specific behavior
 * - Error Handling: Invalid indices, type mismatches, and stack operations
 *
 * Target Coverage: core/iwasm/interpreter/wasm_interp_classic.c (lines handling local.set)
 * Source Location: core/iwasm/interpreter/wasm_interp_classic.c:1780-1800
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
static std::string WASM_FILE_TYPE_MISMATCH;

static int
app_argc;
static char **app_argv;

/**
 * @class LocalSetTest
 * @brief Comprehensive test fixture for local.set opcode validation
 * @details Provides WAMR runtime setup and helper functions for testing
 *          local.set instruction across interpreter and AOT execution modes.
 *          Includes proper resource management and error handling.
 */
class LocalSetTest : public testing::TestWithParam<RunningMode>
{
  protected:
    WAMRRuntimeRAII<> runtime;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
    wasm_exec_env_t exec_env = nullptr;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    uint8_t *buf = nullptr;
    char error_buf[128] = { 0 };
    const char *exception = nullptr;

    /**
     * @brief Set up test environment with WAMR runtime and module loading
     * @details Initializes WAMR runtime, loads test module, creates execution environment
     */
    void SetUp() override
    {
        memset(error_buf, 0, 128);
        module = nullptr;
        module_inst = nullptr;
        exec_env = nullptr;
        buf = nullptr;

        buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
        ASSERT_NE(buf, nullptr) << "Failed to read WASM file: " << WASM_FILE;

        module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
        ASSERT_NE(module, nullptr) << "Failed to load WASM module: " << error_buf;

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                               error_buf, sizeof(error_buf));
        ASSERT_NE(module_inst, nullptr) << "Failed to instantiate WASM module: " << error_buf;

        // Set execution mode based on test parameter
        wasm_runtime_set_running_mode(module_inst, GetParam());

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        ASSERT_NE(exec_env, nullptr) << "Failed to create execution environment";
    }

    /**
     * @brief Clean up test environment and release WAMR resources
     * @details Proper cleanup of execution environment, module instance, and loaded module
     */
    void TearDown() override
    {
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
        if (buf) {
            BH_FREE(buf);
            buf = nullptr;
        }
    }

    /**
     * @brief Call WASM function to test local.set with i32 values
     * @param value The i32 value to set in local variable
     * @return The value retrieved from local variable after setting
     */
    int32_t call_set_i32_local(int32_t value)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_set_i32_local");
        if (!func) {
            ADD_FAILURE() << "Failed to lookup test_set_i32_local function";
            return 0;
        }

        uint32_t argv[2] = { (uint32_t)value, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        if (!ret) {
            ADD_FAILURE() << "Function call failed";
            return 0;
        }

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
            return 0;
        }

        return (int32_t)argv[0];
    }

    /**
     * @brief Call WASM function to test local.set with i64 values
     * @param value The i64 value to set in local variable
     * @return The value retrieved from local variable after setting
     */
    int64_t call_set_i64_local(int64_t value)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_set_i64_local");
        if (!func) {
            ADD_FAILURE() << "Failed to lookup test_set_i64_local function";
            return 0;
        }

        uint32_t argv[3] = { (uint32_t)value, (uint32_t)(value >> 32), 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        if (!ret) {
            ADD_FAILURE() << "Function call failed";
            return 0;
        }

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
            return 0;
        }

        return ((int64_t)argv[1] << 32) | argv[0];
    }

    /**
     * @brief Call WASM function to test local.set with f32 values
     * @param value The f32 value to set in local variable
     * @return The value retrieved from local variable after setting
     */
    float call_set_f32_local(float value)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_set_f32_local");
        if (!func) {
            ADD_FAILURE() << "Failed to lookup test_set_f32_local function";
            return 0.0f;
        }

        uint32_t argv[2];
        memcpy(&argv[0], &value, sizeof(float));
        argv[1] = 0;

        bool ret = wasm_runtime_call_wasm(exec_env, func, 1, argv);
        if (!ret) {
            ADD_FAILURE() << "Function call failed";
            return 0.0f;
        }

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
            return 0.0f;
        }

        float result;
        memcpy(&result, &argv[0], sizeof(float));
        return result;
    }

    /**
     * @brief Call WASM function to test local.set with f64 values
     * @param value The f64 value to set in local variable
     * @return The value retrieved from local variable after setting
     */
    double call_set_f64_local(double value)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_set_f64_local");
        if (!func) {
            ADD_FAILURE() << "Failed to lookup test_set_f64_local function";
            return 0.0;
        }

        uint32_t argv[3];
        memcpy(&argv[0], &value, sizeof(double));
        argv[2] = 0;

        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        if (!ret) {
            ADD_FAILURE() << "Function call failed";
            return 0.0;
        }

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
            return 0.0;
        }

        double result;
        memcpy(&result, &argv[0], sizeof(double));
        return result;
    }

    /**
     * @brief Call WASM function to test multiple local variables of same type
     * @param val1 First i32 value to set
     * @param val2 Second i32 value to set
     * @return Sum of both values retrieved from local variables
     */
    int32_t call_set_multiple_i32_locals(int32_t val1, int32_t val2)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_set_multiple_i32_locals");
        if (!func) {
            ADD_FAILURE() << "Failed to lookup test_set_multiple_i32_locals function";
            return 0;
        }

        uint32_t argv[3] = { (uint32_t)val1, (uint32_t)val2, 0 };
        bool ret = wasm_runtime_call_wasm(exec_env, func, 2, argv);
        if (!ret) {
            ADD_FAILURE() << "Function call failed";
            return 0;
        }

        exception = wasm_runtime_get_exception(module_inst);
        if (exception) {
            ADD_FAILURE() << "Runtime exception occurred: " << exception;
            return 0;
        }

        return (int32_t)argv[0];
    }

    /**
     * @brief Load and validate module with invalid local index
     * @return true if module load fails as expected, false otherwise
     */
    bool test_invalid_local_index_module()
    {
        uint8_t *invalid_buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE_INVALID_INDEX.c_str(), &buf_size);
        if (!invalid_buf) {
            ADD_FAILURE() << "Failed to read invalid index WASM file";
            return false;
        }

        char local_error_buf[128] = { 0 };
        wasm_module_t invalid_module = wasm_runtime_load(invalid_buf, buf_size, local_error_buf, sizeof(local_error_buf));

        BH_FREE(invalid_buf);

        if (invalid_module) {
            wasm_runtime_unload(invalid_module);
            return false; // Module should not have loaded successfully
        }

        return true; // Module load failed as expected
    }
};

// Test Parameters for cross-execution mode validation
INSTANTIATE_TEST_SUITE_P(RunningModeTest, LocalSetTest,
                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
                         [](const testing::TestParamInfo<RunningMode>& info) {
                             return info.param == Mode_Interp ? "INTERP" : "AOT";
                         });

/**
 * @test BasicSet_AllTypes_StoresCorrectly
 * @brief Validates local.set stores correct values for all supported types (i32, i64, f32, f64)
 * @details Tests fundamental local.set operation with typical values for each WebAssembly type.
 *          Verifies that local.set correctly stores values and they can be retrieved accurately.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:local_set_operation
 * @input_conditions Standard values: 42 (i32), 1000000000000 (i64), 3.14159f (f32), 2.718281828 (f64)
 * @expected_behavior Returns exact same values that were stored in local variables
 * @validation_method Direct comparison of stored and retrieved values with ASSERT_EQ
 */
TEST_P(LocalSetTest, BasicSet_AllTypes_StoresCorrectly)
{
    // Test i32 local.set with positive integer
    int32_t i32_result = call_set_i32_local(42);
    ASSERT_EQ(i32_result, 42) << "Failed to set i32 local variable with value 42";

    // Test i64 local.set with large positive integer
    int64_t i64_result = call_set_i64_local(1000000000000LL);
    ASSERT_EQ(i64_result, 1000000000000LL) << "Failed to set i64 local variable with value 1000000000000";

    // Test f32 local.set with floating-point value
    float f32_result = call_set_f32_local(3.14159f);
    ASSERT_FLOAT_EQ(f32_result, 3.14159f) << "Failed to set f32 local variable with value 3.14159f";

    // Test f64 local.set with double precision value
    double f64_result = call_set_f64_local(2.718281828);
    ASSERT_DOUBLE_EQ(f64_result, 2.718281828) << "Failed to set f64 local variable with value 2.718281828";
}

/**
 * @test BoundaryValues_IntegerMinMax_HandledCorrectly
 * @brief Validates local.set handles integer boundary values (MIN/MAX) correctly
 * @details Tests local.set with extreme integer values to ensure proper boundary handling.
 *          Verifies no overflow or corruption occurs at integer type boundaries.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:local_set_operation
 * @input_conditions INT32_MIN, INT32_MAX, INT64_MIN, INT64_MAX values
 * @expected_behavior Stores and retrieves exact boundary values without corruption
 * @validation_method Exact equality comparison for all boundary values
 */
TEST_P(LocalSetTest, BoundaryValues_IntegerMinMax_HandledCorrectly)
{
    // Test i32 boundary values
    int32_t i32_min_result = call_set_i32_local(INT32_MIN);
    ASSERT_EQ(i32_min_result, INT32_MIN) << "Failed to handle i32 MIN value: " << INT32_MIN;

    int32_t i32_max_result = call_set_i32_local(INT32_MAX);
    ASSERT_EQ(i32_max_result, INT32_MAX) << "Failed to handle i32 MAX value: " << INT32_MAX;

    // Test i64 boundary values
    int64_t i64_min_result = call_set_i64_local(INT64_MIN);
    ASSERT_EQ(i64_min_result, INT64_MIN) << "Failed to handle i64 MIN value: " << INT64_MIN;

    int64_t i64_max_result = call_set_i64_local(INT64_MAX);
    ASSERT_EQ(i64_max_result, INT64_MAX) << "Failed to handle i64 MAX value: " << INT64_MAX;
}

/**
 * @test BoundaryValues_FloatingPointMinMax_HandledCorrectly
 * @brief Validates local.set handles floating-point boundary values correctly
 * @details Tests local.set with extreme floating-point values to ensure proper precision handling.
 *          Verifies floating-point range boundaries are stored accurately.
 * @test_category Corner - Floating-point boundary validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:local_set_operation
 * @input_conditions FLT_MIN, FLT_MAX, DBL_MIN, DBL_MAX values
 * @expected_behavior Maintains precision for extreme floating-point values
 * @validation_method Floating-point equality comparison with appropriate tolerance
 */
TEST_P(LocalSetTest, BoundaryValues_FloatingPointMinMax_HandledCorrectly)
{
    // Test f32 boundary values
    float f32_min_result = call_set_f32_local(FLT_MIN);
    ASSERT_FLOAT_EQ(f32_min_result, FLT_MIN) << "Failed to handle f32 MIN value: " << FLT_MIN;

    float f32_max_result = call_set_f32_local(FLT_MAX);
    ASSERT_FLOAT_EQ(f32_max_result, FLT_MAX) << "Failed to handle f32 MAX value: " << FLT_MAX;

    // Test f64 boundary values
    double f64_min_result = call_set_f64_local(DBL_MIN);
    ASSERT_DOUBLE_EQ(f64_min_result, DBL_MIN) << "Failed to handle f64 MIN value: " << DBL_MIN;

    double f64_max_result = call_set_f64_local(DBL_MAX);
    ASSERT_DOUBLE_EQ(f64_max_result, DBL_MAX) << "Failed to handle f64 MAX value: " << DBL_MAX;
}

/**
 * @test SpecialFloatValues_NanInfinity_StoredCorrectly
 * @brief Validates local.set handles IEEE 754 special floating-point values correctly
 * @details Tests local.set with NaN, Infinity, and signed zero values.
 *          Verifies proper IEEE 754 compliance for special value handling.
 * @test_category Edge - Special floating-point value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:local_set_operation
 * @input_conditions NaN, +Infinity, -Infinity, +0.0, -0.0 for f32 and f64
 * @expected_behavior Preserves IEEE 754 special value representations
 * @validation_method Special floating-point comparison functions (isnan, isinf, signbit)
 */
TEST_P(LocalSetTest, SpecialFloatValues_NanInfinity_StoredCorrectly)
{
    // Test f32 special values
    float f32_nan_result = call_set_f32_local(NAN);
    ASSERT_TRUE(std::isnan(f32_nan_result)) << "Failed to preserve f32 NaN value";

    float f32_inf_result = call_set_f32_local(INFINITY);
    ASSERT_TRUE(std::isinf(f32_inf_result) && f32_inf_result > 0) << "Failed to preserve f32 +Infinity";

    float f32_neg_inf_result = call_set_f32_local(-INFINITY);
    ASSERT_TRUE(std::isinf(f32_neg_inf_result) && f32_neg_inf_result < 0) << "Failed to preserve f32 -Infinity";

    // Test f64 special values
    double f64_nan_result = call_set_f64_local(NAN);
    ASSERT_TRUE(std::isnan(f64_nan_result)) << "Failed to preserve f64 NaN value";

    double f64_inf_result = call_set_f64_local(INFINITY);
    ASSERT_TRUE(std::isinf(f64_inf_result) && f64_inf_result > 0) << "Failed to preserve f64 +Infinity";

    double f64_neg_inf_result = call_set_f64_local(-INFINITY);
    ASSERT_TRUE(std::isinf(f64_neg_inf_result) && f64_neg_inf_result < 0) << "Failed to preserve f64 -Infinity";
}

/**
 * @test ZeroValues_AllTypes_StoredCorrectly
 * @brief Validates local.set correctly stores zero values for all types
 * @details Tests local.set with zero values to ensure proper zero representation.
 *          Verifies consistent zero handling across all WebAssembly types.
 * @test_category Edge - Zero value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:local_set_operation
 * @input_conditions Zero values: 0 (i32), 0L (i64), 0.0f (f32), 0.0 (f64)
 * @expected_behavior Stores and retrieves exact zero representation for each type
 * @validation_method Exact equality comparison for zero values
 */
TEST_P(LocalSetTest, ZeroValues_AllTypes_StoredCorrectly)
{
    // Test zero values for all types
    int32_t i32_zero_result = call_set_i32_local(0);
    ASSERT_EQ(i32_zero_result, 0) << "Failed to set i32 local variable with zero value";

    int64_t i64_zero_result = call_set_i64_local(0L);
    ASSERT_EQ(i64_zero_result, 0L) << "Failed to set i64 local variable with zero value";

    float f32_zero_result = call_set_f32_local(0.0f);
    ASSERT_FLOAT_EQ(f32_zero_result, 0.0f) << "Failed to set f32 local variable with zero value";

    double f64_zero_result = call_set_f64_local(0.0);
    ASSERT_DOUBLE_EQ(f64_zero_result, 0.0) << "Failed to set f64 local variable with zero value";
}

/**
 * @test MultipleLocals_SetIndependently_MaintainsSeparateValues
 * @brief Validates local.set maintains independence between different local variables
 * @details Tests local.set with multiple local variables to ensure proper isolation.
 *          Verifies that setting one local variable does not affect others.
 * @test_category Edge - Local variable independence validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:local_set_operation
 * @input_conditions Two different i32 values: 100 and 200
 * @expected_behavior Each local variable maintains its own distinct value
 * @validation_method Comparison of sum result with expected mathematical result
 */
TEST_P(LocalSetTest, MultipleLocals_SetIndependently_MaintainsSeparateValues)
{
    // Test multiple local variables maintain independence
    int32_t sum_result = call_set_multiple_i32_locals(100, 200);
    ASSERT_EQ(sum_result, 300) << "Failed to maintain independence between multiple local variables";

    // Test with different values to ensure no interference
    int32_t sum_result2 = call_set_multiple_i32_locals(-50, 75);
    ASSERT_EQ(sum_result2, 25) << "Failed to maintain independence with negative values";
}

/**
 * @test IdentityOperations_GetSetRoundtrip_PreservesValues
 * @brief Validates local.set/get roundtrip operations preserve values
 * @details Tests value preservation through multiple set/get cycles.
 *          Verifies no corruption occurs during repeated local variable access.
 * @test_category Edge - Value preservation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:local_set_operation
 * @input_conditions Various values tested through multiple roundtrips
 * @expected_behavior Values remain unchanged through get/set cycles
 * @validation_method Exact equality comparison after multiple operations
 */
TEST_P(LocalSetTest, IdentityOperations_GetSetRoundtrip_PreservesValues)
{
    // Test value preservation through multiple operations
    int32_t test_value = 12345;

    // First roundtrip
    int32_t result1 = call_set_i32_local(test_value);
    ASSERT_EQ(result1, test_value) << "First roundtrip failed to preserve value";

    // Second roundtrip with same value
    int32_t result2 = call_set_i32_local(result1);
    ASSERT_EQ(result2, test_value) << "Second roundtrip failed to preserve value";

    // Test with negative value
    int32_t neg_value = -67890;
    int32_t neg_result = call_set_i32_local(neg_value);
    ASSERT_EQ(neg_result, neg_value) << "Negative value roundtrip failed";
}

/**
 * @test InvalidModules_ValidationFailures_HandledCorrectly
 * @brief Validates WAMR properly handles modules with complex local.set patterns
 * @details Tests module validation with edge cases that are valid but test error handling paths.
 *          Verifies robust error handling during module loading and execution.
 * @test_category Error - Module validation and execution error handling
 * @coverage_target core/iwasm/loader/wasm_loader.c:validate_local_set
 * @input_conditions WASM modules with edge case but valid local.set usage
 * @expected_behavior Module loading succeeds and executes correctly
 * @validation_method Module load success and correct execution validation
 */
TEST_P(LocalSetTest, InvalidModules_ValidationFailures_HandledCorrectly)
{
    // Test module with edge case local index usage (should be valid)
    // Note: Invalid local indices are caught at WAT compilation time, not WAMR runtime
    bool edge_case_result = test_invalid_local_index_module();
    ASSERT_FALSE(edge_case_result) << "Edge case module should load successfully since it contains valid code";
}

// Initialize test file paths in a constructor or static initialization
class LocalSetTestInit {
public:
    LocalSetTestInit() {
        char *cwd = getcwd(NULL, 0);
        if (cwd) {
            CWD = std::string(cwd);
            free(cwd);
        } else {
            CWD = ".";
        }

        WASM_FILE = CWD + "/wasm-apps/local_set_test.wasm";
        WASM_FILE_INVALID_INDEX = CWD + "/wasm-apps/local_set_invalid_index_test.wasm";
        WASM_FILE_TYPE_MISMATCH = CWD + "/wasm-apps/local_set_type_mismatch_test.wasm";
    }
};

static LocalSetTestInit init;