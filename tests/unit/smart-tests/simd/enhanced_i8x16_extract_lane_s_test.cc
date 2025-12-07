/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "wasm_runtime_common.h"
#include "wasm_export.h"
#include "test_helper.h"

/**
 * @class I8x16ExtractLaneSTestSuite
 * @brief Test fixture class for i8x16.extract_lane_s opcode testing
 * @details Provides setup/teardown for WAMR runtime and module loading.
 *          Tests signed lane extraction from v128 vectors with comprehensive validation.
 */
class I8x16ExtractLaneSTestSuite : public testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize WAMR runtime using RAII helper
        runtime_raii = std::make_unique<WAMRRuntimeRAII<>>();

        // Load the i8x16.extract_lane_s test module
        dummy_env = std::make_unique<DummyExecEnv>("wasm-apps/i8x16_extract_lane_s_test.wasm");
        ASSERT_NE(nullptr, dummy_env)
            << "Failed to create DummyExecEnv for i8x16.extract_lane_s tests";
        ASSERT_NE(nullptr, dummy_env->get())
            << "Failed to get valid execution environment from DummyExecEnv";
    }

    void TearDown() override
    {
        dummy_env.reset();
        runtime_raii.reset();
    }

    /**
     * @brief Execute WASM function and return i32 result with error handling
     */
    int32_t call_wasm_function(const char* func_name)
    {
        if (!dummy_env || !dummy_env->get()) {
            ADD_FAILURE() << "Invalid execution environment for function: " << func_name;
            return 0;
        }

        uint32_t argv[1] = {0};
        bool call_result = dummy_env->execute(func_name, 0, argv);
        if (!call_result) {
            ADD_FAILURE() << "Function execution failed: " << func_name
                         << ", exception: " << (dummy_env->get_exception() ? dummy_env->get_exception() : "No exception info");
            return 0;
        }
        return static_cast<int32_t>(argv[0]);
    }

    // Test environment resources
    std::unique_ptr<WAMRRuntimeRAII<>> runtime_raii;
    std::unique_ptr<DummyExecEnv> dummy_env;
};

/**
 * @test BasicExtraction_PositiveValue_ReturnsCorrectResult
 * @brief Validates basic i8x16.extract_lane_s functionality with positive value
 */
TEST_F(I8x16ExtractLaneSTestSuite, BasicExtraction_PositiveValue_ReturnsCorrectResult)
{
    ASSERT_EQ(42, call_wasm_function("test_extract_positive_lane0"))
        << "Failed to extract positive value (42) from lane 0";
}

/**
 * @test BoundaryValues_MaxPositive_ReturnsCorrectSignExtension
 * @brief Validates extraction of maximum positive signed 8-bit value (+127)
 */
TEST_F(I8x16ExtractLaneSTestSuite, BoundaryValues_MaxPositive_ReturnsCorrectSignExtension)
{
    ASSERT_EQ(127, call_wasm_function("test_extract_max_positive"))
        << "Failed to extract maximum positive signed value (+127)";
}

/**
 * @test BoundaryValues_MaxNegative_ReturnsCorrectSignExtension
 * @brief Validates extraction of minimum negative signed 8-bit value (-128)
 */
TEST_F(I8x16ExtractLaneSTestSuite, BoundaryValues_MaxNegative_ReturnsCorrectSignExtension)
{
    ASSERT_EQ(-128, call_wasm_function("test_extract_max_negative"))
        << "Failed to extract minimum negative signed value (-128) with proper sign extension";
}

/**
 * @test ModuleLoading_ValidWASM_LoadsSuccessfully
 * @brief Validates successful loading of WASM module containing i8x16.extract_lane_s
 */
TEST_F(I8x16ExtractLaneSTestSuite, ModuleLoading_ValidWASM_LoadsSuccessfully)
{
    // Verify we can successfully execute a test function (validates module loading)
    ASSERT_EQ(42, call_wasm_function("test_extract_positive_lane0"))
        << "Test function should execute successfully, validating proper module loading";
}