(module
  ;; Enhanced test module for i32x4.abs opcode validation
  ;;
  ;; This module provides comprehensive test functions for validating
  ;; the i32x4.abs SIMD instruction across various scenarios:
  ;; - Basic functionality with mixed positive/negative values
  ;; - Boundary conditions (INT32_MIN/MAX handling)
  ;; - Edge cases (all zeros, all positive, all negative)
  ;; - Mathematical property validation

  ;; Test function: Basic absolute value with mixed values
  ;; Tests fundamental i32x4.abs functionality with typical input patterns
  ;; Expected behavior: abs(negative) = positive, abs(positive) = positive, abs(0) = 0
  (func $test_basic_abs (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
  )

  ;; Test function: Boundary value handling
  ;; Tests critical boundary cases including INT32_MIN overflow scenario
  ;; Expected behavior: abs(INT32_MIN) = INT32_MIN (overflow), abs(INT32_MAX) = INT32_MAX, others normal
  (func $test_boundary_values (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
  )

  ;; Test function: Zero vector identity
  ;; Tests mathematical identity property: abs(0) = 0
  ;; Expected behavior: All zero lanes remain zero
  (func $test_zero_vector (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
  )

  ;; Test function: Positive values remain unchanged
  ;; Tests mathematical property: abs(x) = x for x >= 0
  ;; Expected behavior: All positive values remain identical
  (func $test_positive_values (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
  )

  ;; Test function: Negative values become positive
  ;; Tests mathematical property: abs(-x) = x for x > 0
  ;; Expected behavior: All negative values become their positive equivalents
  (func $test_negative_values (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
  )

  ;; Test function: INT32_MIN overflow handling
  ;; Tests specific overflow scenario where abs(INT32_MIN) cannot be represented
  ;; Expected behavior: All INT32_MIN values wrap to INT32_MIN due to overflow
  (func $test_int32_min_overflow (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
  )

  ;; Additional test: Alternating pattern validation
  ;; Tests lane independence with alternating positive/negative pattern
  (func $test_alternating_pattern (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
  )

  ;; Additional test: Extreme value combinations
  ;; Tests mixed extreme values: INT32_MIN, INT32_MAX, and intermediate values
  (func $test_extreme_combinations (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
  )

  ;; Additional test: Mathematical property validation
  ;; Tests idempotent property: abs(abs(x)) = abs(x)
  (func $test_idempotent_property (param $input v128) (result v128)
    (local.get $input)
    (i32x4.abs)
    (i32x4.abs)  ;; Apply abs twice to verify idempotent property
  )

  ;; Export all test functions for C++ test harness access
  (export "test_basic_abs" (func $test_basic_abs))
  (export "test_boundary_values" (func $test_boundary_values))
  (export "test_zero_vector" (func $test_zero_vector))
  (export "test_positive_values" (func $test_positive_values))
  (export "test_negative_values" (func $test_negative_values))
  (export "test_int32_min_overflow" (func $test_int32_min_overflow))
  (export "test_alternating_pattern" (func $test_alternating_pattern))
  (export "test_extreme_combinations" (func $test_extreme_combinations))
  (export "test_idempotent_property" (func $test_idempotent_property))
)