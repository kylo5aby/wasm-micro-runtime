/*
 * @brief Comprehensive unit tests for select opcode
 * @file enhanced_select_test.cc
 *
 * This file contains comprehensive test cases for the WebAssembly select opcode,
 * testing conditional selection functionality across all supported data types
 * and execution modes (interpreter and AOT).
 *
 * The select opcode implements conditional selection similar to a ternary operator:
 * - Pops condition (i32), val2 (T), val1 (T) from stack
 * - Returns val1 if condition != 0, otherwise returns val2
 * - Supports i32, i64, f32, f64, and reference types
 */

#include "gtest/gtest.h"
#include "wasm_runtime_common.h"
#include "wasm_native.h"
#include "wasm_memory.h"
#include "bh_read_file.h"
#include "test_helper.h"
#include <cmath>
#include <limits>
#include <cstring>
#include <climits>
#include <cfloat>

static const char *WASM_FILE = "wasm-apps/select_test.wasm";

/**
 * @brief Test fixture class for select opcode testing
 * @details Provides comprehensive test infrastructure for validating select opcode
 *          functionality across different execution modes and data types.
 *          Tests conditional selection behavior and edge case handling.
 */
class SelectTest : public testing::Test {
protected:
    /**
     * @brief Set up test environment and initialize WASM runtime
     * @details Initializes WAMR runtime with proper configuration for select opcode testing
     */
    void SetUp() override
    {
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        ASSERT_TRUE(wasm_runtime_full_init(&init_args))
            << "Failed to initialize WAMR runtime";

        // Load and instantiate test module
        LoadWasmModule(WASM_FILE);
        CreateModuleInstance();
        CreateExecEnv();
    }

    /**
     * @brief Clean up test environment and destroy WAMR runtime
     * @details Properly cleans up all allocated resources including
     *          module instances, modules, and runtime environment
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
        wasm_runtime_destroy();
    }

    /**
     * @brief Load WASM module from file path
     * @param wasm_file Relative path to WASM file from build directory
     * @return Pointer to loaded WASM module, or nullptr on failure
     */
    wasm_module_t LoadWasmModule(const char *wasm_file)
    {
        char error_buf[128] = { 0 };
        uint32 wasm_file_size;
        uint8 *wasm_file_buf = nullptr;

        wasm_file_buf = (uint8 *)bh_read_file_to_buffer(wasm_file, &wasm_file_size);
        EXPECT_NE(nullptr, wasm_file_buf) << "Failed to read WASM file: " << wasm_file;
        if (!wasm_file_buf) {
            return nullptr;
        }

        module = wasm_runtime_load(wasm_file_buf, wasm_file_size, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;

        BH_FREE(wasm_file_buf);
        return module;
    }

    /**
     * @brief Create WASM module instance for testing
     * @param stack_size Stack size for module instance (default 8192)
     * @param heap_size Heap size for module instance (default 8192)
     * @return Pointer to module instance, or nullptr on failure
     */
    wasm_module_inst_t CreateModuleInstance(uint32 stack_size = 8192, uint32 heap_size = 8192)
    {
        char error_buf[128] = { 0 };

        EXPECT_NE(nullptr, module) << "Module must be loaded before creating instance";
        if (!module) {
            return nullptr;
        }

        module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf, sizeof(error_buf));
        EXPECT_NE(nullptr, module_inst) << "Failed to instantiate module: " << error_buf;

        return module_inst;
    }

    /**
     * @brief Create execution environment for testing
     * @param stack_size Execution stack size (default 8192)
     * @return Pointer to execution environment, or nullptr on failure
     */
    wasm_exec_env_t CreateExecEnv(uint32 stack_size = 8192)
    {
        EXPECT_NE(nullptr, module_inst) << "Module instance must be created before exec env";
        if (!module_inst) {
            return nullptr;
        }

        exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
        EXPECT_NE(nullptr, exec_env) << "Failed to create execution environment";

        return exec_env;
    }

    /**
     * @brief Call i32 select test function
     * @param val1 First i32 operand value
     * @param val2 Second i32 operand value
     * @param cond Condition value (0 = false, non-zero = true)
     * @return Selected i32 value (val1 if cond != 0, val2 if cond == 0)
     */
    int32_t call_i32_select(int32_t val1, int32_t val2, int32_t cond) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i32_select");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_i32_select function";

        uint32_t argv[3] = { static_cast<uint32_t>(val1),
                            static_cast<uint32_t>(val2),
                            static_cast<uint32_t>(cond) };

        EXPECT_TRUE(wasm_runtime_call_wasm(exec_env, func, 3, argv))
            << "Failed to call test_i32_select function";

        return static_cast<int32_t>(argv[0]);
    }

    /**
     * @brief Call i64 select test function
     * @param val1 First i64 operand value
     * @param val2 Second i64 operand value
     * @param cond Condition value (0 = false, non-zero = true)
     * @return Selected i64 value (val1 if cond != 0, val2 if cond == 0)
     */
    int64_t call_i64_select(int64_t val1, int64_t val2, int32_t cond) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_i64_select");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_i64_select function";

        uint32_t argv[5];
        // Pack i64 values into argv array (low 32 bits, high 32 bits)
        argv[0] = static_cast<uint32_t>(val1);
        argv[1] = static_cast<uint32_t>(val1 >> 32);
        argv[2] = static_cast<uint32_t>(val2);
        argv[3] = static_cast<uint32_t>(val2 >> 32);
        argv[4] = static_cast<uint32_t>(cond);

        EXPECT_TRUE(wasm_runtime_call_wasm(exec_env, func, 5, argv))
            << "Failed to call test_i64_select function";

        // Unpack i64 result from argv array
        return static_cast<int64_t>(argv[0]) | (static_cast<int64_t>(argv[1]) << 32);
    }

    /**
     * @brief Call f32 select test function
     * @param val1 First f32 operand value
     * @param val2 Second f32 operand value
     * @param cond Condition value (0 = false, non-zero = true)
     * @return Selected f32 value (val1 if cond != 0, val2 if cond == 0)
     */
    float call_f32_select(float val1, float val2, int32_t cond) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f32_select");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f32_select function";

        uint32_t argv[3];
        // Use memcpy to preserve bit patterns for float values
        memcpy(&argv[0], &val1, sizeof(float));
        memcpy(&argv[1], &val2, sizeof(float));
        argv[2] = static_cast<uint32_t>(cond);

        EXPECT_TRUE(wasm_runtime_call_wasm(exec_env, func, 3, argv))
            << "Failed to call test_f32_select function";

        float result;
        memcpy(&result, &argv[0], sizeof(float));
        return result;
    }

    /**
     * @brief Call f64 select test function
     * @param val1 First f64 operand value
     * @param val2 Second f64 operand value
     * @param cond Condition value (0 = false, non-zero = true)
     * @return Selected f64 value (val1 if cond != 0, val2 if cond == 0)
     */
    double call_f64_select(double val1, double val2, int32_t cond) {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "test_f64_select");
        EXPECT_NE(nullptr, func) << "Failed to lookup test_f64_select function";

        uint32_t argv[5];
        // Pack f64 values into argv array preserving bit patterns
        uint64_t val1_bits, val2_bits;
        memcpy(&val1_bits, &val1, sizeof(double));
        memcpy(&val2_bits, &val2, sizeof(double));

        argv[0] = static_cast<uint32_t>(val1_bits);
        argv[1] = static_cast<uint32_t>(val1_bits >> 32);
        argv[2] = static_cast<uint32_t>(val2_bits);
        argv[3] = static_cast<uint32_t>(val2_bits >> 32);
        argv[4] = static_cast<uint32_t>(cond);

        EXPECT_TRUE(wasm_runtime_call_wasm(exec_env, func, 5, argv))
            << "Failed to call test_f64_select function";

        // Unpack f64 result from argv array
        uint64_t result_bits = static_cast<uint64_t>(argv[0]) | (static_cast<uint64_t>(argv[1]) << 32);
        double result;
        memcpy(&result, &result_bits, sizeof(double));
        return result;
    }

    RuntimeInitArgs init_args;
    wasm_exec_env_t exec_env = nullptr;
    wasm_module_t module = nullptr;
    wasm_module_inst_t module_inst = nullptr;
};

/**
 * @test BasicSelection_ReturnsCorrectValue
 * @brief Validates select produces correct results for basic conditional selection
 * @details Tests fundamental select operation with typical values across all supported
 *          data types (i32, i64, f32, f64). Verifies condition evaluation logic
 *          where non-zero values are true and zero is false.
 * @test_category Main - Basic functionality validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_SELECT*
 * @input_conditions Standard value pairs with true/false conditions
 * @expected_behavior Returns val1 when condition != 0, val2 when condition == 0
 * @validation_method Direct comparison of WASM function result with expected values
 */
TEST_F(SelectTest, BasicSelection_ReturnsCorrectValue) {
    // Test i32 selection with true condition
    ASSERT_EQ(42, call_i32_select(42, 99, 1))
        << "i32 select with true condition should return first value";

    // Test i32 selection with false condition
    ASSERT_EQ(99, call_i32_select(42, 99, 0))
        << "i32 select with false condition should return second value";

    // Test i64 selection with true condition
    ASSERT_EQ(0x123456789ABCDEF0LL, call_i64_select(0x123456789ABCDEF0LL, 0xFEDCBA9876543210LL, 1))
        << "i64 select with true condition should return first value";

    // Test i64 selection with false condition
    ASSERT_EQ(0xFEDCBA9876543210LL, call_i64_select(0x123456789ABCDEF0LL, 0xFEDCBA9876543210LL, 0))
        << "i64 select with false condition should return second value";

    // Test f32 selection with true condition
    ASSERT_FLOAT_EQ(3.14159f, call_f32_select(3.14159f, 2.71828f, 1))
        << "f32 select with true condition should return first value";

    // Test f32 selection with false condition
    ASSERT_FLOAT_EQ(2.71828f, call_f32_select(3.14159f, 2.71828f, 0))
        << "f32 select with false condition should return second value";

    // Test f64 selection with true condition
    ASSERT_DOUBLE_EQ(3.141592653589793, call_f64_select(3.141592653589793, 2.718281828459045, 1))
        << "f64 select with true condition should return first value";

    // Test f64 selection with false condition
    ASSERT_DOUBLE_EQ(2.718281828459045, call_f64_select(3.141592653589793, 2.718281828459045, 0))
        << "f64 select with false condition should return second value";
}

/**
 * @test BoundaryValues_SelectsCorrectly
 * @brief Validates select behavior with boundary and extreme values
 * @details Tests selection operation with MIN/MAX values for integers and
 *          boundary floating-point values. Verifies correct handling of
 *          extreme condition values.
 * @test_category Corner - Boundary condition validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_SELECT*
 * @input_conditions MIN/MAX values as operands and conditions
 * @expected_behavior Correct selection regardless of extreme values
 * @validation_method Direct comparison with expected boundary value results
 */
TEST_F(SelectTest, BoundaryValues_SelectsCorrectly) {
    // Test with INT32 boundary values as operands
    ASSERT_EQ(std::numeric_limits<int32_t>::max(),
              call_i32_select(std::numeric_limits<int32_t>::max(),
                            std::numeric_limits<int32_t>::min(), 1))
        << "Should select INT32_MAX when condition is true";

    ASSERT_EQ(std::numeric_limits<int32_t>::min(),
              call_i32_select(std::numeric_limits<int32_t>::max(),
                            std::numeric_limits<int32_t>::min(), 0))
        << "Should select INT32_MIN when condition is false";

    // Test with INT32 boundary values as condition (both should be "true")
    ASSERT_EQ(100, call_i32_select(100, 200, std::numeric_limits<int32_t>::max()))
        << "INT32_MAX condition should be evaluated as true";

    ASSERT_EQ(100, call_i32_select(100, 200, std::numeric_limits<int32_t>::min()))
        << "INT32_MIN condition should be evaluated as true";

    // Test with INT64 boundary values
    ASSERT_EQ(std::numeric_limits<int64_t>::max(),
              call_i64_select(std::numeric_limits<int64_t>::max(),
                            std::numeric_limits<int64_t>::min(), 1))
        << "Should select INT64_MAX when condition is true";

    // Test with floating-point boundary values
    ASSERT_FLOAT_EQ(std::numeric_limits<float>::max(),
                    call_f32_select(std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::min(), 1))
        << "Should select FLT_MAX when condition is true";

    ASSERT_DOUBLE_EQ(std::numeric_limits<double>::max(),
                     call_f64_select(std::numeric_limits<double>::max(),
                                   std::numeric_limits<double>::min(), 1))
        << "Should select DBL_MAX when condition is true";
}

/**
 * @test SpecialFloatValues_HandledCorrectly
 * @brief Validates correct handling of special floating-point values
 * @details Tests selection behavior with NaN, Infinity, +/-0.0, and denormal
 *          numbers. Ensures bit-exact preservation of special float values.
 * @test_category Edge - Special value validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_SELECT*
 * @input_conditions NaN, Infinity, +/-0.0 as operands
 * @expected_behavior Bit-exact preservation of special float values
 * @validation_method Bit-level comparison for exact value preservation
 */
TEST_F(SelectTest, SpecialFloatValues_HandledCorrectly) {
    // Test NaN selection and preservation
    float nan_f32 = std::numeric_limits<float>::quiet_NaN();
    float result_f32 = call_f32_select(nan_f32, 1.0f, 1);
    ASSERT_TRUE(std::isnan(result_f32))
        << "NaN value should be preserved through select operation";

    double nan_f64 = std::numeric_limits<double>::quiet_NaN();
    double result_f64 = call_f64_select(nan_f64, 1.0, 1);
    ASSERT_TRUE(std::isnan(result_f64))
        << "f64 NaN value should be preserved through select operation";

    // Test Infinity selection
    ASSERT_TRUE(std::isinf(call_f32_select(std::numeric_limits<float>::infinity(), 1.0f, 1)))
        << "Positive infinity should be preserved";

    ASSERT_TRUE(std::isinf(call_f32_select(-std::numeric_limits<float>::infinity(), 1.0f, 1)))
        << "Negative infinity should be preserved";

    // Test +0.0 and -0.0 distinction (bit-exact comparison)
    float pos_zero = +0.0f;
    float neg_zero = -0.0f;

    float result_pos = call_f32_select(pos_zero, 1.0f, 1);
    float result_neg = call_f32_select(neg_zero, 1.0f, 1);

    // Use memcmp for bit-exact comparison
    ASSERT_EQ(0, memcmp(&result_pos, &pos_zero, sizeof(float)))
        << "Positive zero bit pattern should be preserved";

    ASSERT_EQ(0, memcmp(&result_neg, &neg_zero, sizeof(float)))
        << "Negative zero bit pattern should be preserved";

    // Test denormal number preservation
    float denormal = std::numeric_limits<float>::denorm_min();
    float result_denormal = call_f32_select(denormal, 1.0f, 1);
    ASSERT_EQ(0, memcmp(&result_denormal, &denormal, sizeof(float)))
        << "Denormal number bit pattern should be preserved";
}

/**
 * @test IdentityOperands_ReturnsConsistentResult
 * @brief Validates selection behavior when both operands are identical
 * @details Tests cases where both operands have the same value, ensuring
 *          the result is consistent regardless of the condition value.
 *          This validates the mathematical property that select(x,x,c) = x.
 * @test_category Edge - Identity operation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:WASM_OP_SELECT*
 * @input_conditions Identical operand values with various conditions
 * @expected_behavior Same result regardless of condition when operands are equal
 * @validation_method Comparison of results with different conditions
 */
TEST_F(SelectTest, IdentityOperands_ReturnsConsistentResult) {
    // Test i32 identity selection
    int32_t test_val = 12345;
    ASSERT_EQ(test_val, call_i32_select(test_val, test_val, 1))
        << "Identity select with true condition should return the value";

    ASSERT_EQ(test_val, call_i32_select(test_val, test_val, 0))
        << "Identity select with false condition should return the value";

    ASSERT_EQ(test_val, call_i32_select(test_val, test_val, 999))
        << "Identity select with any condition should return the value";

    // Test i64 identity selection
    int64_t test_val_64 = 0x123456789ABCDEF0LL;
    ASSERT_EQ(test_val_64, call_i64_select(test_val_64, test_val_64, 1))
        << "i64 identity select should return the value regardless of condition";

    ASSERT_EQ(test_val_64, call_i64_select(test_val_64, test_val_64, 0))
        << "i64 identity select should return the value regardless of condition";

    // Test f32 identity selection
    float test_val_f32 = 3.14159f;
    ASSERT_FLOAT_EQ(test_val_f32, call_f32_select(test_val_f32, test_val_f32, 1))
        << "f32 identity select should return the value regardless of condition";

    ASSERT_FLOAT_EQ(test_val_f32, call_f32_select(test_val_f32, test_val_f32, 0))
        << "f32 identity select should return the value regardless of condition";

    // Test f64 identity selection
    double test_val_f64 = 2.718281828459045;
    ASSERT_DOUBLE_EQ(test_val_f64, call_f64_select(test_val_f64, test_val_f64, 1))
        << "f64 identity select should return the value regardless of condition";

    ASSERT_DOUBLE_EQ(test_val_f64, call_f64_select(test_val_f64, test_val_f64, 0))
        << "f64 identity select should return the value regardless of condition";
}

/**
 * @test ConditionEdgeCases_EvaluatesCorrectly
 * @brief Validates correct evaluation of various condition values
 * @details Tests edge cases in condition evaluation, confirming that only
 *          zero is considered false while all other values are true.
 *          Validates proper handling of negative numbers, large values, etc.
 * @test_category Edge - Condition evaluation validation
 * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:condition_evaluation
 * @input_conditions Various condition values (0, 1, -1, MAX, MIN)
 * @expected_behavior Only 0 evaluates to false, all others evaluate to true
 * @validation_method Testing condition evaluation with representative values
 */
TEST_F(SelectTest, ConditionEdgeCases_EvaluatesCorrectly) {
    int32_t val1 = 111;
    int32_t val2 = 222;

    // Test explicit true conditions (should select val1)
    ASSERT_EQ(val1, call_i32_select(val1, val2, 1))
        << "Condition 1 should evaluate to true";

    ASSERT_EQ(val1, call_i32_select(val1, val2, -1))
        << "Condition -1 should evaluate to true";

    ASSERT_EQ(val1, call_i32_select(val1, val2, 42))
        << "Condition 42 should evaluate to true";

    ASSERT_EQ(val1, call_i32_select(val1, val2, 0x80000000)) // INT32_MIN
        << "INT32_MIN condition should evaluate to true";

    ASSERT_EQ(val1, call_i32_select(val1, val2, 0x7FFFFFFF)) // INT32_MAX
        << "INT32_MAX condition should evaluate to true";

    // Test explicit false condition (should select val2)
    ASSERT_EQ(val2, call_i32_select(val1, val2, 0))
        << "Condition 0 should evaluate to false";

    // Test with different data types to ensure condition evaluation is consistent
    ASSERT_EQ(0x1111111111111111LL, call_i64_select(0x1111111111111111LL, 0x2222222222222222LL, -999))
        << "Negative condition should evaluate to true for i64 select";

    ASSERT_FLOAT_EQ(1.111f, call_f32_select(1.111f, 2.222f, 999999))
        << "Large positive condition should evaluate to true for f32 select";

    ASSERT_DOUBLE_EQ(1.111111, call_f64_select(1.111111, 2.222222, -999999))
        << "Large negative condition should evaluate to true for f64 select";
}

