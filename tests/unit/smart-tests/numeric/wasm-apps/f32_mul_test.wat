(module
  ;; f32.mul test module for comprehensive opcode validation
  ;; This module provides test functions to validate f32.mul (32-bit floating-point multiplication)
  ;; across all test scenarios: basic arithmetic, boundary values, special IEEE 754 values,
  ;; overflow/underflow conditions, and mathematical properties as defined in the test strategy.

  ;; Main test function: f32.mul operation
  ;; Parameters: f32 a (multiplicand), f32 b (multiplier)
  ;; Returns: f32 result of a * b
  ;;
  ;; This function directly exercises the f32.mul WebAssembly opcode with the provided
  ;; operands and returns the IEEE 754 compliant multiplication result.
  (func $f32_mul_test (export "f32_mul_test") (param $a f32) (param $b f32) (result f32)
    (f32.mul (local.get $a) (local.get $b))
  )

  ;; Test function for basic positive number multiplication
  ;; Validates standard floating-point multiplication with positive values
  ;; Used by BasicMultiplication_ReturnsCorrectResults test case
  (func $test_positive_multiplication (export "test_positive_multiplication") (result f32)
    ;; Test: 2.5 * 3.0 = 7.5
    (f32.mul (f32.const 2.5) (f32.const 3.0))
  )

  ;; Test function for negative number multiplication
  ;; Validates multiplication of negative floating-point values (negative * negative = positive)
  (func $test_negative_multiplication (export "test_negative_multiplication") (result f32)
    ;; Test: (-4.0) * (-2.5) = 10.0
    (f32.mul (f32.const -4.0) (f32.const -2.5))
  )

  ;; Test function for mixed sign multiplication
  ;; Validates multiplication between positive and negative values (result negative)
  (func $test_mixed_signs (export "test_mixed_signs") (result f32)
    ;; Test: 6.0 * (-1.5) = -9.0
    (f32.mul (f32.const 6.0) (f32.const -1.5))
  )

  ;; Test function for boundary value operations with FLT_MAX
  ;; Validates behavior with floating-point maximum boundary values
  (func $test_boundary_max (export "test_boundary_max") (result f32)
    ;; Test maximum float value multiplication with identity
    ;; FLT_MAX * 1.0 should equal FLT_MAX
    (f32.mul (f32.const 3.4028235e+38) (f32.const 1.0))
  )

  ;; Test function for boundary value operations with FLT_MIN
  ;; Validates behavior with floating-point minimum positive boundary values
  (func $test_boundary_min (export "test_boundary_min") (result f32)
    ;; Test minimum positive float value multiplication with identity
    ;; FLT_MIN * 1.0 should equal FLT_MIN
    (f32.mul (f32.const 1.175494e-38) (f32.const 1.0))
  )

  ;; Test function for overflow to positive infinity
  ;; Validates IEEE 754 overflow behavior for positive results
  (func $test_overflow_positive (export "test_overflow_positive") (result f32)
    ;; Test: FLT_MAX * 2.0 should overflow to +INFINITY
    (f32.mul (f32.const 3.4028235e+38) (f32.const 2.0))
  )

  ;; Test function for overflow to negative infinity
  ;; Validates IEEE 754 overflow behavior for negative results
  (func $test_overflow_negative (export "test_overflow_negative") (result f32)
    ;; Test: (-FLT_MAX) * 2.0 should overflow to -INFINITY
    (f32.mul (f32.const -3.4028235e+38) (f32.const 2.0))
  )

  ;; Test function for underflow to zero/denormal
  ;; Validates IEEE 754 underflow behavior
  (func $test_underflow (export "test_underflow") (result f32)
    ;; Test: FLT_MIN * 0.5 should underflow to zero or denormal
    (f32.mul (f32.const 1.175494e-38) (f32.const 0.5))
  )

  ;; Test function for NaN propagation
  ;; Validates that NaN values propagate through multiplication operations
  (func $test_nan_propagation (export "test_nan_propagation") (result f32)
    ;; Test: NaN * 5.0 should return NaN
    (f32.mul (f32.const nan) (f32.const 5.0))
  )

  ;; Test function for infinity multiplication with finite values
  ;; Validates infinity arithmetic rules per IEEE 754
  (func $test_infinity_finite (export "test_infinity_finite") (result f32)
    ;; Test: +INFINITY * 2.5 should return +INFINITY
    (f32.mul (f32.const inf) (f32.const 2.5))
  )

  ;; Test function for infinity multiplication with negative finite values
  ;; Validates sign handling in infinity arithmetic
  (func $test_infinity_negative (export "test_infinity_negative") (result f32)
    ;; Test: +INFINITY * (-1.8) should return -INFINITY
    (f32.mul (f32.const inf) (f32.const -1.8))
  )

  ;; Test function for infinity multiplication with zero (invalid operation)
  ;; Validates that infinity * zero produces NaN per IEEE 754
  (func $test_infinity_zero (export "test_infinity_zero") (result f32)
    ;; Test: +INFINITY * 0.0 should return NaN (invalid operation)
    (f32.mul (f32.const inf) (f32.const 0.0))
  )

  ;; Test function for signed zero handling
  ;; Validates proper sign handling with zero operands
  (func $test_signed_zero (export "test_signed_zero") (result f32)
    ;; Test: 0.0 * (-2.0) should return -0.0 or 0.0 (implementation-dependent)
    (f32.mul (f32.const 0.0) (f32.const -2.0))
  )

  ;; Test function for multiplicative identity
  ;; Validates that multiplication by 1.0 preserves the operand
  (func $test_identity (export "test_identity") (result f32)
    ;; Test: 7.25 * 1.0 should equal 7.25 (multiplicative identity)
    (f32.mul (f32.const 7.25) (f32.const 1.0))
  )

  ;; Test function for negation through multiplication
  ;; Validates multiplication by -1.0 for sign inversion
  (func $test_negation (export "test_negation") (result f32)
    ;; Test: 7.25 * (-1.0) should equal -7.25 (negation through multiplication)
    (f32.mul (f32.const 7.25) (f32.const -1.0))
  )

  ;; Test function for commutativity validation (first order)
  ;; Validates a * b for commutativity property testing
  (func $test_commute_ab (export "test_commute_ab") (result f32)
    ;; Test: 3.14 * 2.71 for commutativity validation
    (f32.mul (f32.const 3.14) (f32.const 2.71))
  )

  ;; Test function for commutativity validation (reverse order)
  ;; Validates b * a for commutativity property testing
  (func $test_commute_ba (export "test_commute_ba") (result f32)
    ;; Test: 2.71 * 3.14 for commutativity validation
    (f32.mul (f32.const 2.71) (f32.const 3.14))
  )

  ;; Test function for commutativity with mixed signs (first order)
  ;; Validates commutativity with negative operands
  (func $test_commute_mixed_ab (export "test_commute_mixed_ab") (result f32)
    ;; Test: (-5.5) * 4.2 for mixed sign commutativity validation
    (f32.mul (f32.const -5.5) (f32.const 4.2))
  )

  ;; Test function for commutativity with mixed signs (reverse order)
  ;; Validates commutativity with negative operands in reverse order
  (func $test_commute_mixed_ba (export "test_commute_mixed_ba") (result f32)
    ;; Test: 4.2 * (-5.5) for mixed sign commutativity validation
    (f32.mul (f32.const 4.2) (f32.const -5.5))
  )

  ;; Test function for commutativity with zero (first order)
  ;; Validates commutativity with zero operand
  (func $test_commute_zero_ab (export "test_commute_zero_ab") (result f32)
    ;; Test: 0.0 * 8.9 for zero commutativity validation
    (f32.mul (f32.const 0.0) (f32.const 8.9))
  )

  ;; Test function for commutativity with zero (reverse order)
  ;; Validates commutativity with zero operand in reverse order
  (func $test_commute_zero_ba (export "test_commute_zero_ba") (result f32)
    ;; Test: 8.9 * 0.0 for zero commutativity validation
    (f32.mul (f32.const 8.9) (f32.const 0.0))
  )

  ;; Test function for commutativity with infinity (first order)
  ;; Validates commutativity with infinity operand
  (func $test_commute_inf_ab (export "test_commute_inf_ab") (result f32)
    ;; Test: +INFINITY * 3.0 for infinity commutativity validation
    (f32.mul (f32.const inf) (f32.const 3.0))
  )

  ;; Test function for commutativity with infinity (reverse order)
  ;; Validates commutativity with infinity operand in reverse order
  (func $test_commute_inf_ba (export "test_commute_inf_ba") (result f32)
    ;; Test: 3.0 * +INFINITY for infinity commutativity validation
    (f32.mul (f32.const 3.0) (f32.const inf))
  )
)