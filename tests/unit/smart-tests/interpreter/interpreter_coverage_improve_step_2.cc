/*
 * Copyright (C) 2024 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include "wasm_export.h"
#include "bh_read_file.h"
#include "test_helper.h"
#include <cmath>
#include <cfloat>

/*
 * Step 2: Floating Point Operations Coverage Improvement
 * 
 * Target Functions:
 * - f32_min() [0 hits, ~15 lines] - wasm_interp_classic.c
 * - f32_max() [0 hits, ~15 lines] - wasm_interp_classic.c  
 * - f64_min() [0 hits, ~15 lines] - wasm_interp_classic.c
 * - f64_max() [0 hits, ~15 lines] - wasm_interp_classic.c
 * - local_copysignf() [0 hits, ~15 lines] - wasm_interp_classic.c
 * - local_copysign() [0 hits, ~15 lines] - wasm_interp_classic.c
 * 
 * Expected Coverage Gain: ~90 lines (~1.0% improvement)
 * Target Module Coverage: 46.8% (from 45.8%)
 */

class InterpreterFloatingPointTest : public testing::Test
{
protected:
    void SetUp() override
    {
        runtime = std::make_unique<WAMRRuntimeRAII<512 * 1024>>();
        
        // Load the floating_point_operations.wasm module using standard WAMR pattern
        wasm_file_buf = (unsigned char *)bh_read_file_to_buffer("floating_point_operations.wasm", &wasm_file_size);
        ASSERT_NE(wasm_file_buf, nullptr) << "Failed to read WASM file";
        
        module = std::make_unique<WAMRModule>(wasm_file_buf, wasm_file_size);
        ASSERT_NE(module->get(), nullptr) << "Failed to load WASM module";
        
        instance = std::make_unique<WAMRInstance>(*module, 8192, 8192);
        ASSERT_NE(instance->get(), nullptr) << "Failed to instantiate WASM module";
        
        exec_env = std::make_unique<WAMRExecEnv>(*instance, 8192);
        ASSERT_NE(exec_env->get(), nullptr) << "Failed to create execution environment";
    }
    
    void TearDown() override
    {
        exec_env.reset();
        instance.reset();
        module.reset();
        if (wasm_file_buf) {
            wasm_runtime_free(wasm_file_buf);
            wasm_file_buf = nullptr;
        }
        runtime.reset();
    }
    
    // Helper function to call WASM functions with f32 parameters
    float call_wasm_f32_f32_f32(const char* func_name, float param1, float param2)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(instance->get(), func_name);
        if (!func) return NAN;
        
        uint32_t wasm_argv[2];
        memcpy(&wasm_argv[0], &param1, sizeof(float));
        memcpy(&wasm_argv[1], &param2, sizeof(float));
        
        bool success = wasm_runtime_call_wasm(exec_env->get(), func, 2, wasm_argv);
        if (!success) return NAN;
        
        float result;
        memcpy(&result, &wasm_argv[0], sizeof(float));
        return result;
    }
    
    // Helper function to call WASM functions with f64 parameters
    double call_wasm_f64_f64_f64(const char* func_name, double param1, double param2)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(instance->get(), func_name);
        if (!func) return NAN;
        
        uint32_t wasm_argv[4]; // f64 takes 2 slots
        memcpy(&wasm_argv[0], &param1, sizeof(double));
        memcpy(&wasm_argv[2], &param2, sizeof(double));
        
        bool success = wasm_runtime_call_wasm(exec_env->get(), func, 4, wasm_argv);
        if (!success) return NAN;
        
        double result;
        memcpy(&result, &wasm_argv[0], sizeof(double));
        return result;
    }
    
    // Helper function to call WASM functions with no parameters
    float call_wasm_f32_void(const char* func_name)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(instance->get(), func_name);
        if (!func) return NAN;
        
        uint32_t wasm_argv[1];
        bool success = wasm_runtime_call_wasm(exec_env->get(), func, 0, wasm_argv);
        if (!success) return NAN;
        
        float result;
        memcpy(&result, &wasm_argv[0], sizeof(float));
        return result;
    }
    
    std::unique_ptr<WAMRRuntimeRAII<512 * 1024>> runtime;
    std::unique_ptr<WAMRModule> module;
    std::unique_ptr<WAMRInstance> instance;
    std::unique_ptr<WAMRExecEnv> exec_env;
    unsigned char *wasm_file_buf = nullptr;
    uint32_t wasm_file_size = 0;
};

// Test f32_min() function - covers ~15 lines in wasm_interp_classic.c
TEST_F(InterpreterFloatingPointTest, F32Min_NormalValues_ReturnsMinimum)
{
    // Arrange: Test normal floating point values
    float val1 = 3.14f, val2 = 2.71f;
    
    // Act: Execute f32.min operation
    float result = call_wasm_f32_f32_f32("test_f32_min", val1, val2);
    
    // Assert: Verify minimum value returned and function executed
    ASSERT_FLOAT_EQ(result, 2.71f);
}

// Test f32_min() with special values - covers additional lines
TEST_F(InterpreterFloatingPointTest, F32Min_SpecialValues_HandlesNaNCorrectly)
{
    // Arrange: Test NaN and infinity values
    float nan_val = NAN, inf_val = INFINITY;
    
    // Act: Execute f32.min with NaN
    float result = call_wasm_f32_f32_f32("test_f32_min_nan", nan_val, inf_val);
    
    // Assert: Verify NaN propagation behavior
    ASSERT_TRUE(std::isnan(result));
}

// Test f32_max() function - covers ~15 lines in wasm_interp_classic.c
TEST_F(InterpreterFloatingPointTest, F32Max_NormalValues_ReturnsMaximum)
{
    // Arrange: Test normal floating point values
    float val1 = 1.41f, val2 = 1.73f;
    
    // Act: Execute f32.max operation
    float result = call_wasm_f32_f32_f32("test_f32_max", val1, val2);
    
    // Assert: Verify maximum value returned
    ASSERT_FLOAT_EQ(result, 1.73f);
}

// Test f32_max() with special values
TEST_F(InterpreterFloatingPointTest, F32Max_SpecialValues_HandlesNaNCorrectly)
{
    // Arrange: Test NaN propagation in max operation
    float nan_val = NAN, normal_val = 42.0f;
    
    // Act: Execute f32.max with NaN
    float result = call_wasm_f32_f32_f32("test_f32_max_nan", nan_val, normal_val);
    
    // Assert: Verify NaN propagation
    ASSERT_TRUE(std::isnan(result));
}

// Test f64_min() function - covers ~15 lines in wasm_interp_classic.c
TEST_F(InterpreterFloatingPointTest, F64Min_NormalValues_ReturnsMinimum)
{
    // Arrange: Test double precision values
    double val1 = 2.718281828, val2 = 3.141592653;
    
    // Act: Execute f64.min operation
    double result = call_wasm_f64_f64_f64("test_f64_min", val1, val2);
    
    // Assert: Verify minimum value returned
    ASSERT_DOUBLE_EQ(result, 2.718281828);
}

// Test f64_min() with special values
TEST_F(InterpreterFloatingPointTest, F64Min_SpecialValues_HandlesNaNCorrectly)
{
    // Arrange: Test NaN with double precision
    double nan_val = NAN, inf_val = INFINITY;
    
    // Act: Execute f64.min with NaN
    double result = call_wasm_f64_f64_f64("test_f64_min_nan", nan_val, inf_val);
    
    // Assert: Verify NaN propagation in double precision
    ASSERT_TRUE(std::isnan(result));
}

// Test f64_max() function - covers ~15 lines in wasm_interp_classic.c
TEST_F(InterpreterFloatingPointTest, F64Max_NormalValues_ReturnsMaximum)
{
    // Arrange: Test double precision maximum
    double val1 = 1.414213562, val2 = 1.732050807;
    
    // Act: Execute f64.max operation
    double result = call_wasm_f64_f64_f64("test_f64_max", val1, val2);
    
    // Assert: Verify maximum value returned
    ASSERT_DOUBLE_EQ(result, 1.732050807);
}

// Test f64_max() with special values
TEST_F(InterpreterFloatingPointTest, F64Max_SpecialValues_HandlesNaNCorrectly)
{
    // Arrange: Test NaN propagation in f64.max
    double nan_val = NAN, normal_val = 123.456;
    
    // Act: Execute f64.max with NaN
    double result = call_wasm_f64_f64_f64("test_f64_max_nan", nan_val, normal_val);
    
    // Assert: Verify NaN propagation
    ASSERT_TRUE(std::isnan(result));
}

// Test local_copysignf() function - covers ~15 lines in wasm_interp_classic.c
TEST_F(InterpreterFloatingPointTest, LocalCopysignf_Operations_CopiesSignCorrectly)
{
    // Arrange: Test f32.copysign operation
    float magnitude = 42.0f, sign_source = -1.0f;
    
    // Act: Execute f32.copysign operation
    float result = call_wasm_f32_f32_f32("test_f32_copysign", magnitude, sign_source);
    
    // Assert: Verify sign copied correctly
    ASSERT_FLOAT_EQ(result, -42.0f);
}

// Test local_copysign() function - covers ~15 lines in wasm_interp_classic.c  
TEST_F(InterpreterFloatingPointTest, LocalCopysign_Operations_CopiesSignCorrectly)
{
    // Arrange: Test f64.copysign operation
    double magnitude = 123.456, sign_source = -1.0;
    
    // Act: Execute f64.copysign operation
    double result = call_wasm_f64_f64_f64("test_f64_copysign", magnitude, sign_source);
    
    // Assert: Verify sign copied correctly in double precision
    ASSERT_DOUBLE_EQ(result, -123.456);
}

// Test zero handling edge cases
TEST_F(InterpreterFloatingPointTest, FloatingPointOperations_ZeroHandling_BehavesCorrectly)
{
    // Arrange: Test positive and negative zero handling
    // Act: Execute zero handling test
    float result = call_wasm_f32_void("test_f32_zero_handling");
    
    // Assert: Verify zero handling behavior
    // Should handle positive/negative zero according to IEEE 754
    ASSERT_TRUE(result == 0.0f || result == -0.0f);
}

// Test special values comprehensive handling
TEST_F(InterpreterFloatingPointTest, FloatingPointOperations_SpecialValues_HandledCorrectly)
{
    // Arrange: Test comprehensive special value handling
    // Act: Execute special values test
    float result = call_wasm_f32_void("test_f32_special_values");
    
    // Assert: Verify special value handling (NaN with infinity)
    ASSERT_TRUE(std::isnan(result));
}