// /*
//  * Copyright (C) 2019 Intel Corporation. All rights reserved.
//  * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//  */

// #include <gtest/gtest.h>
// #include <cmath>
// #include <limits>
// #include "wasm_export.h"
// #include "bh_read_file.h"

// static std::string CWD;
// static std::string WASM_FILE;
// static wasm_module_t module = nullptr;
// static wasm_module_inst_t module_inst = nullptr;
// static wasm_exec_env_t exec_env = nullptr;
// static uint32_t buf_size, stack_size = 8092, heap_size = 8092;
// static uint8_t *buf = nullptr;

// /**
//  * @brief Test fixture for f64.gt opcode validation
//  * @details Comprehensive test suite for WebAssembly f64.gt (0x64) instruction.
//  *          Tests floating-point greater-than comparison across interpreter and AOT modes.
//  */
// class F64GtTest : public testing::TestWithParam<RunningMode> {
// protected:
//     void SetUp() override {
//         memset(&init_args, 0, sizeof(RuntimeInitArgs));
//         init_args.mem_alloc_type = Alloc_With_System_Allocator;
        
//         // Initialize WAMR runtime
//         ASSERT_TRUE(wasm_runtime_full_init(&init_args))
//             << "Failed to initialize WAMR runtime";
        
//         // Load WASM module
//         CWD = get_current_dir();
//         WASM_FILE = CWD + "/wasm-apps/f64_gt_test.wasm";
//         buf = (uint8_t *)bh_read_file_to_buffer(WASM_FILE.c_str(), &buf_size);
//         ASSERT_NE(nullptr, buf) << "Failed to read WASM file: " << WASM_FILE;
        
//         // Load and instantiate module
//         module = wasm_runtime_load(buf, buf_size, error_buf, sizeof(error_buf));
//         ASSERT_NE(nullptr, module) << "Failed to load WASM module: " << error_buf;
        
//         module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
//                                                error_buf, sizeof(error_buf));
//         ASSERT_NE(nullptr, module_inst) << "Failed to instantiate WASM module: " << error_buf;
        
//         exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
//         ASSERT_NE(nullptr, exec_env) << "Failed to create execution environment";
//     }
    
//     void TearDown() override {
//         // Cleanup execution environment
//         if (exec_env) {
//             wasm_runtime_destroy_exec_env(exec_env);
//             exec_env = nullptr;
//         }
        
//         // Cleanup module instance
//         if (module_inst) {
//             wasm_runtime_deinstantiate(module_inst);
//             module_inst = nullptr;
//         }
        
//         // Cleanup module
//         if (module) {
//             wasm_runtime_unload(module);
//             module = nullptr;
//         }
        
//         // Cleanup buffer
//         if (buf) {
//             BH_FREE(buf);
//             buf = nullptr;
//         }
        
//         // Destroy WAMR runtime
//         wasm_runtime_destroy();
//     }
    
//     /**
//      * @brief Helper function to call f64.gt WASM function
//      * @param a First f64 operand
//      * @param b Second f64 operand
//      * @return i32 result (1 if a > b, 0 otherwise)
//      */
//     int32_t call_f64_gt(double a, double b) {
//         wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "f64_gt_test");
//         EXPECT_NE(nullptr, func) << "Failed to lookup f64_gt_test function";
        
//         uint32_t argv[4];
//         // Pack f64 values into uint32_t array (little-endian)
//         memcpy(&argv[0], &a, sizeof(double));
//         memcpy(&argv[2], &b, sizeof(double));
        
//         bool ret = wasm_runtime_call_wasm(exec_env, func, 4, argv);
//         EXPECT_TRUE(ret) << "Failed to call f64_gt_test function: " 
//                         << wasm_runtime_get_exception(module_inst);
        
//         return (int32_t)argv[0];
//     }
    
//     /**
//      * @brief Get current working directory
//      * @return Current directory path as string
//      */
//     std::string get_current_dir() {
//         char cwd[1024];
//         if (getcwd(cwd, sizeof(cwd)) != nullptr) {
//             return std::string(cwd);
//         }
//         return "";
//     }

// private:
//     RuntimeInitArgs init_args;
//     char error_buf[128];
// };

// /**
//  * @test BasicComparison_ReturnsCorrectResult
//  * @brief Validates f64.gt produces correct comparison results for typical inputs
//  * @details Tests fundamental greater-than operation with positive, negative, and mixed-sign doubles.
//  *          Verifies that f64.gt correctly computes (a > b) for various input combinations.
//  * @test_category Main - Basic functionality validation
//  * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_gt_operation
//  * @input_conditions Standard double pairs: various comparison scenarios
//  * @expected_behavior Returns 1 when first operand > second, 0 otherwise
//  * @validation_method Direct comparison of WASM function result with expected values
//  */
// TEST_P(F64GtTest, BasicComparison_ReturnsCorrectResult) {
//     // Test positive numbers: 5.5 > 3.3 should return 1
//     ASSERT_EQ(1, call_f64_gt(5.5, 3.3)) 
//         << "f64.gt failed for positive numbers: 5.5 > 3.3";
    
//     // Test negative numbers: -2.1 > -5.7 should return 1
//     ASSERT_EQ(1, call_f64_gt(-2.1, -5.7))
//         << "f64.gt failed for negative numbers: -2.1 > -5.7";
    
//     // Test mixed signs: 4.2 > -1.8 should return 1
//     ASSERT_EQ(1, call_f64_gt(4.2, -1.8))
//         << "f64.gt failed for mixed signs: 4.2 > -1.8";
    
//     // Test equal values: 2.5 > 2.5 should return 0
//     ASSERT_EQ(0, call_f64_gt(2.5, 2.5))
//         << "f64.gt failed for equal values: 2.5 > 2.5";
    
//     // Test reverse comparison: 1.1 > 7.7 should return 0
//     ASSERT_EQ(0, call_f64_gt(1.1, 7.7))
//         << "f64.gt failed for reverse comparison: 1.1 > 7.7";
// }

// /**
//  * @test ZeroComparison_HandlesZeroCorrectly
//  * @brief Validates f64.gt handles zero values correctly
//  * @details Tests comparison operations involving positive zero, negative zero, and regular values.
//  *          Verifies IEEE 754 compliance for zero comparisons.
//  * @test_category Main - Zero value validation
//  * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_gt_operation
//  * @input_conditions Zero values: +0.0, -0.0, and comparisons with non-zero
//  * @expected_behavior Correct IEEE 754 zero comparison behavior
//  * @validation_method Verification of zero comparison semantics
//  */
// TEST_P(F64GtTest, ZeroComparison_HandlesZeroCorrectly) {
//     // Test positive zero vs negative zero: +0.0 > -0.0 should return 0 (equal)
//     ASSERT_EQ(0, call_f64_gt(0.0, -0.0))
//         << "f64.gt failed for +0.0 > -0.0 comparison";
    
//     // Test negative zero vs positive zero: -0.0 > +0.0 should return 0 (equal)
//     ASSERT_EQ(0, call_f64_gt(-0.0, 0.0))
//         << "f64.gt failed for -0.0 > +0.0 comparison";
    
//     // Test positive number vs zero: 1.0 > 0.0 should return 1
//     ASSERT_EQ(1, call_f64_gt(1.0, 0.0))
//         << "f64.gt failed for 1.0 > 0.0 comparison";
    
//     // Test zero vs positive number: 0.0 > 1.0 should return 0
//     ASSERT_EQ(0, call_f64_gt(0.0, 1.0))
//         << "f64.gt failed for 0.0 > 1.0 comparison";
    
//     // Test negative number vs zero: -1.0 > 0.0 should return 0
//     ASSERT_EQ(0, call_f64_gt(-1.0, 0.0))
//         << "f64.gt failed for -1.0 > 0.0 comparison";
// }

// /**
//  * @test InfinityComparison_HandlesInfinityCorrectly
//  * @brief Validates f64.gt handles infinity values correctly
//  * @details Tests comparison operations involving positive infinity, negative infinity, and finite values.
//  *          Verifies IEEE 754 compliance for infinity comparisons.
//  * @test_category Edge - Infinity value validation
//  * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_gt_operation
//  * @input_conditions Infinity values: +inf, -inf, and comparisons with finite values
//  * @expected_behavior Correct IEEE 754 infinity comparison behavior
//  * @validation_method Verification of infinity comparison semantics
//  */
// TEST_P(F64GtTest, InfinityComparison_HandlesInfinityCorrectly) {
//     double pos_inf = std::numeric_limits<double>::infinity();
//     double neg_inf = -std::numeric_limits<double>::infinity();
    
//     // Test positive infinity vs finite: +inf > 1000.0 should return 1
//     ASSERT_EQ(1, call_f64_gt(pos_inf, 1000.0))
//         << "f64.gt failed for +inf > 1000.0 comparison";
    
//     // Test finite vs positive infinity: 1000.0 > +inf should return 0
//     ASSERT_EQ(0, call_f64_gt(1000.0, pos_inf))
//         << "f64.gt failed for 1000.0 > +inf comparison";
    
//     // Test positive infinity vs negative infinity: +inf > -inf should return 1
//     ASSERT_EQ(1, call_f64_gt(pos_inf, neg_inf))
//         << "f64.gt failed for +inf > -inf comparison";
    
//     // Test negative infinity vs finite: -inf > -1000.0 should return 0
//     ASSERT_EQ(0, call_f64_gt(neg_inf, -1000.0))
//         << "f64.gt failed for -inf > -1000.0 comparison";
    
//     // Test positive infinity vs positive infinity: +inf > +inf should return 0
//     ASSERT_EQ(0, call_f64_gt(pos_inf, pos_inf))
//         << "f64.gt failed for +inf > +inf comparison";
// }

// /**
//  * @test NaNComparison_HandlesNaNCorrectly
//  * @brief Validates f64.gt handles NaN values correctly
//  * @details Tests comparison operations involving NaN (Not a Number) values.
//  *          Verifies IEEE 754 compliance: any comparison with NaN returns false.
//  * @test_category Edge - NaN value validation
//  * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_gt_operation
//  * @input_conditions NaN values and comparisons with finite/infinite values
//  * @expected_behavior All NaN comparisons return 0 (false) per IEEE 754
//  * @validation_method Verification of NaN comparison semantics
//  */
// TEST_P(F64GtTest, NaNComparison_HandlesNaNCorrectly) {
//     double nan_val = std::numeric_limits<double>::quiet_NaN();
    
//     // Test NaN vs finite: NaN > 5.0 should return 0
//     ASSERT_EQ(0, call_f64_gt(nan_val, 5.0))
//         << "f64.gt failed for NaN > 5.0 comparison";
    
//     // Test finite vs NaN: 5.0 > NaN should return 0
//     ASSERT_EQ(0, call_f64_gt(5.0, nan_val))
//         << "f64.gt failed for 5.0 > NaN comparison";
    
//     // Test NaN vs NaN: NaN > NaN should return 0
//     ASSERT_EQ(0, call_f64_gt(nan_val, nan_val))
//         << "f64.gt failed for NaN > NaN comparison";
    
//     // Test NaN vs infinity: NaN > +inf should return 0
//     ASSERT_EQ(0, call_f64_gt(nan_val, std::numeric_limits<double>::infinity()))
//         << "f64.gt failed for NaN > +inf comparison";
    
//     // Test NaN vs zero: NaN > 0.0 should return 0
//     ASSERT_EQ(0, call_f64_gt(nan_val, 0.0))
//         << "f64.gt failed for NaN > 0.0 comparison";
// }

// /**
//  * @test SubnormalComparison_HandlesSubnormalCorrectly
//  * @brief Validates f64.gt handles subnormal values correctly
//  * @details Tests comparison operations involving subnormal (denormalized) floating-point values.
//  *          Verifies correct handling of very small magnitude numbers.
//  * @test_category Edge - Subnormal value validation
//  * @coverage_target core/iwasm/interpreter/wasm_interp_classic.c:f64_gt_operation
//  * @input_conditions Subnormal values and comparisons with normal values
//  * @expected_behavior Correct comparison behavior for subnormal numbers
//  * @validation_method Verification of subnormal comparison semantics
//  */
// TEST_P(F64GtTest, SubnormalComparison_HandlesSubnormalCorrectly) {
//     double min_normal = std::numeric_limits<double>::min();
//     double subnormal = min_normal / 2.0;  // Create subnormal value
    
//     // Test subnormal vs zero: subnormal > 0.0 should return 1
//     ASSERT_EQ(1, call_f64_gt(subnormal, 0.0))
//         << "f64.gt failed for subnormal > 0.0 comparison";
    
//     // Test zero vs subnormal: 0.0 > subnormal should return 0
//     ASSERT_EQ(0, call_f64_gt(0.0, subnormal))
//         << "f64.gt failed for 0.0 > subnormal comparison";
    
//     // Test normal vs subnormal: min_normal > subnormal should return 1
//     ASSERT_EQ(1, call_f64_gt(min_normal, subnormal))
//         << "f64.gt failed for min_normal > subnormal comparison";
    
//     // Test negative subnormal: -subnormal > 0.0 should return 0
//     ASSERT_EQ(0, call_f64_gt(-subnormal, 0.0))
//         << "f64.gt failed for -subnormal > 0.0 comparison";
// }

// // Parameterized test instantiation for different execution modes
// INSTANTIATE_TEST_SUITE_P(RunningMode, F64GtTest,
//                         testing::Values(Mode_Interp, Mode_LLVM_JIT),
//                         [](const testing::TestParamInfo<F64GtTest::ParamType> &info) {
//                             return info.param == Mode_Interp ? "INTERP" : "AOT";
//                         });