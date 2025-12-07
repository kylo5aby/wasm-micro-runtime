(module
  ;; Enhanced test module for i16x8.abs opcode validation
  ;;
  ;; This module provides comprehensive test functions for validating
  ;; the i16x8.abs SIMD instruction across various scenarios:
  ;; - Basic functionality with mixed positive/negative values
  ;; - Boundary conditions (INT16_MIN/MAX handling)
  ;; - Edge cases (all zeros, all positive)
  ;; - Mathematical property validation

  ;; Test function: Basic absolute value with mixed values
  ;; Tests fundamental i16x8.abs functionality with typical input patterns
  ;; Expected behavior: abs(negative) = positive, abs(positive) = positive, abs(0) = 0
  (func $test_basic_abs (param $input v128) (result v128)
    (local.get $input)
    (i16x8.abs)
  )

  ;; Test function: Boundary value handling
  ;; Tests critical boundary cases including INT16_MIN overflow scenario
  ;; Expected behavior: abs(-32768) may be -32768 (overflow), abs(32767) = 32767, others normal
  (func $test_boundary_values (param $input v128) (result v128)
    (local.get $input)
    (i16x8.abs)
  )

  ;; Test function: Zero vector identity
  ;; Tests mathematical identity property: abs(0) = 0
  ;; Expected behavior: All zero lanes remain zero
  (func $test_zero_vector (param $input v128) (result v128)
    (local.get $input)
    (i16x8.abs)
  )

  ;; Test function: Positive values remain unchanged
  ;; Tests mathematical property: abs(x) = x for x >= 0
  ;; Expected behavior: All positive values remain identical
  (func $test_positive_values (param $input v128) (result v128)
    (local.get $input)
    (i16x8.abs)
  )

  ;; Additional test: Alternating pattern validation
  ;; Tests lane independence with alternating positive/negative pattern
  (func $test_alternating_pattern (param $input v128) (result v128)
    (local.get $input)
    (i16x8.abs)
  )

  ;; Additional test: Extreme value combinations
  ;; Tests mixed extreme values: INT16_MIN, INT16_MAX, and intermediate values
  (func $test_extreme_combinations (param $input v128) (result v128)
    (local.get $input)
    (i16x8.abs)
  )

  ;; Additional test: Mathematical property validation
  ;; Tests idempotent property: abs(abs(x)) = abs(x)
  (func $test_idempotent_property (param $input v128) (result v128)
    (local.get $input)
    (i16x8.abs)
    (i16x8.abs)  ;; Apply abs twice to verify idempotent property
  )

  ;; Additional test: Large negative values
  ;; Tests behavior with various large negative 16-bit values
  (func $test_large_negatives (param $input v128) (result v128)
    (local.get $input)
    (i16x8.abs)
  )

  ;; Export all test functions for C++ test harness access
  (export "test_basic_abs" (func $test_basic_abs))
  (export "test_boundary_values" (func $test_boundary_values))
  (export "test_zero_vector" (func $test_zero_vector))
  (export "test_positive_values" (func $test_positive_values))
  (export "test_alternating_pattern" (func $test_alternating_pattern))
  (export "test_extreme_combinations" (func $test_extreme_combinations))
  (export "test_idempotent_property" (func $test_idempotent_property))
  (export "test_large_negatives" (func $test_large_negatives))
)