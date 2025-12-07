(module
  ;; f32.sub test module for comprehensive opcode validation
  ;; This module provides test functions to validate f32.sub (32-bit floating-point subtraction)
  ;; across all test scenarios: basic arithmetic, boundary values, special IEEE 754 values,
  ;; and edge cases as defined in the test strategy.

  ;; Main test function: f32.sub operation
  ;; Parameters: f32 a (minuend), f32 b (subtrahend)
  ;; Returns: f32 result of a - b
  ;;
  ;; This function directly exercises the f32.sub WebAssembly opcode with the provided
  ;; operands and returns the IEEE 754 compliant subtraction result.
  (func $f32_sub_test (export "f32_sub_test") (param $a f32) (param $b f32) (result f32)
    (f32.sub (local.get $a) (local.get $b))
  )

  ;; Test function for basic arithmetic operations
  ;; Validates standard floating-point subtraction with typical values
  ;; Used by BasicSubtraction_ReturnsCorrectResult test case
  (func $test_basic_arithmetic (export "test_basic_arithmetic") (result f32)
    ;; Test: 5.5 - 2.3 = 3.2
    (f32.sub (f32.const 5.5) (f32.const 2.3))
  )

  ;; Test function for negative number operations
  ;; Validates subtraction of negative floating-point values
  (func $test_negative_arithmetic (export "test_negative_arithmetic") (result f32)
    ;; Test: (-10.7) - (-3.2) = -7.5
    (f32.sub (f32.const -10.7) (f32.const -3.2))
  )

  ;; Test function for mixed sign operations
  ;; Validates subtraction between positive and negative values
  (func $test_mixed_signs (export "test_mixed_signs") (result f32)
    ;; Test: 15.8 - (-4.9) = 20.7 (effectively addition)
    (f32.sub (f32.const 15.8) (f32.const -4.9))
  )

  ;; Test function for boundary value operations
  ;; Validates behavior with floating-point boundary values
  (func $test_boundary_values (export "test_boundary_values") (result f32)
    ;; Test maximum float value subtraction
    ;; FLT_MAX - 1.0 should equal FLT_MAX due to precision limits
    (f32.sub (f32.const 3.4028235e+38) (f32.const 1.0))
  )

  ;; Test function for overflow to infinity
  ;; Validates IEEE 754 overflow behavior
  (func $test_overflow_infinity (export "test_overflow_infinity") (result f32)
    ;; Test: FLT_MAX - (-FLT_MAX) should overflow to +INFINITY
    (f32.sub (f32.const 3.4028235e+38) (f32.const -3.4028235e+38))
  )

  ;; Test function for underflow to negative infinity
  ;; Validates IEEE 754 underflow behavior
  (func $test_underflow_infinity (export "test_underflow_infinity") (result f32)
    ;; Test: (-FLT_MAX) - FLT_MAX should underflow to -INFINITY
    (f32.sub (f32.const -3.4028235e+38) (f32.const 3.4028235e+38))
  )

  ;; Test function for NaN propagation
  ;; Validates IEEE 754 NaN handling in subtraction
  (func $test_nan_propagation (export "test_nan_propagation") (result f32)
    ;; Test: NaN - 5.0 should return NaN
    (f32.sub (f32.const nan) (f32.const 5.0))
  )

  ;; Test function for NaN with normal operand
  ;; Validates NaN propagation with normal value as first operand
  (func $test_normal_minus_nan (export "test_normal_minus_nan") (result f32)
    ;; Test: 5.0 - NaN should return NaN
    (f32.sub (f32.const 5.0) (f32.const nan))
  )

  ;; Test function for NaN - NaN operation
  ;; Validates NaN propagation with both operands as NaN
  (func $test_nan_minus_nan (export "test_nan_minus_nan") (result f32)
    ;; Test: NaN - NaN should return NaN
    (f32.sub (f32.const nan) (f32.const nan))
  )

  ;; Test function for positive infinity arithmetic
  ;; Validates IEEE 754 infinity arithmetic rules
  (func $test_pos_infinity_arithmetic (export "test_pos_infinity_arithmetic") (result f32)
    ;; Test: +INFINITY - 100.0 should equal +INFINITY
    (f32.sub (f32.const inf) (f32.const 100.0))
  )

  ;; Test function for negative infinity arithmetic
  ;; Validates negative infinity subtraction behavior
  (func $test_neg_infinity_arithmetic (export "test_neg_infinity_arithmetic") (result f32)
    ;; Test: -INFINITY - 100.0 should equal -INFINITY
    (f32.sub (f32.const -inf) (f32.const 100.0))
  )

  ;; Test function for finite minus positive infinity
  ;; Validates finite - (+INFINITY) = -INFINITY
  (func $test_finite_minus_pos_inf (export "test_finite_minus_pos_inf") (result f32)
    ;; Test: 100.0 - (+INFINITY) should equal -INFINITY
    (f32.sub (f32.const 100.0) (f32.const inf))
  )

  ;; Test function for finite minus negative infinity
  ;; Validates finite - (-INFINITY) = +INFINITY
  (func $test_finite_minus_neg_inf (export "test_finite_minus_neg_inf") (result f32)
    ;; Test: 100.0 - (-INFINITY) should equal +INFINITY
    (f32.sub (f32.const 100.0) (f32.const -inf))
  )

  ;; Test function for undefined infinity operations
  ;; Validates (+INFINITY) - (+INFINITY) = NaN
  (func $test_pos_inf_minus_pos_inf (export "test_pos_inf_minus_pos_inf") (result f32)
    ;; Test: (+INFINITY) - (+INFINITY) should return NaN (undefined)
    (f32.sub (f32.const inf) (f32.const inf))
  )

  ;; Test function for undefined negative infinity operations
  ;; Validates (-INFINITY) - (-INFINITY) = NaN
  (func $test_neg_inf_minus_neg_inf (export "test_neg_inf_minus_neg_inf") (result f32)
    ;; Test: (-INFINITY) - (-INFINITY) should return NaN (undefined)
    (f32.sub (f32.const -inf) (f32.const -inf))
  )

  ;; Test function for identity property validation
  ;; Validates x - 0 = x identity property
  (func $test_identity_property (export "test_identity_property") (result f32)
    ;; Test: 42.5 - 0.0 should equal 42.5
    (f32.sub (f32.const 42.5) (f32.const 0.0))
  )

  ;; Test function for negative identity property
  ;; Validates negative value identity: (-x) - 0 = -x
  (func $test_negative_identity (export "test_negative_identity") (result f32)
    ;; Test: (-17.3) - 0.0 should equal -17.3
    (f32.sub (f32.const -17.3) (f32.const 0.0))
  )

  ;; Test function for negation property validation
  ;; Validates 0 - x = -x negation property
  (func $test_negation_property (export "test_negation_property") (result f32)
    ;; Test: 0.0 - 25.7 should equal -25.7
    (f32.sub (f32.const 0.0) (f32.const 25.7))
  )

  ;; Test function for double negation property
  ;; Validates 0 - (-x) = x double negation
  (func $test_double_negation (export "test_double_negation") (result f32)
    ;; Test: 0.0 - (-33.1) should equal 33.1
    (f32.sub (f32.const 0.0) (f32.const -33.1))
  )

  ;; Test function for positive zero minus negative zero
  ;; Validates IEEE 754 signed zero arithmetic: (+0) - (-0) = +0
  (func $test_pos_zero_minus_neg_zero (export "test_pos_zero_minus_neg_zero") (result f32)
    ;; Test: (+0.0) - (-0.0) should result in +0.0
    (f32.sub (f32.const 0.0) (f32.const -0.0))
  )

  ;; Test function for negative zero minus positive zero
  ;; Validates IEEE 754 signed zero arithmetic: (-0) - (+0) = -0
  (func $test_neg_zero_minus_pos_zero (export "test_neg_zero_minus_pos_zero") (result f32)
    ;; Test: (-0.0) - (+0.0) should result in -0.0
    (f32.sub (f32.const -0.0) (f32.const 0.0))
  )

  ;; Test function for self-subtraction property
  ;; Validates x - x = 0 cancellation property
  (func $test_self_subtraction (export "test_self_subtraction") (result f32)
    ;; Test: 123.456 - 123.456 should equal 0.0
    (f32.sub (f32.const 123.456) (f32.const 123.456))
  )

  ;; Test function for negative self-subtraction
  ;; Validates (-x) - (-x) = 0 for negative values
  (func $test_negative_self_subtraction (export "test_negative_self_subtraction") (result f32)
    ;; Test: (-987.654) - (-987.654) should equal 0.0
    (f32.sub (f32.const -987.654) (f32.const -987.654))
  )

  ;; Test function for minimum positive value operations
  ;; Validates FLT_MIN - FLT_MIN = 0
  (func $test_min_value_subtraction (export "test_min_value_subtraction") (result f32)
    ;; Test: FLT_MIN - FLT_MIN should equal 0.0
    (f32.sub (f32.const 1.175494e-38) (f32.const 1.175494e-38))
  )

  ;; Test function for very small number precision
  ;; Validates precision handling with very small floating-point values
  (func $test_small_number_precision (export "test_small_number_precision") (result f32)
    ;; Test precision with small numbers that might result in denormalized values
    (f32.sub (f32.const 1.0e-38) (f32.const 5.0e-39))
  )

  ;; Test function for near-cancellation precision
  ;; Validates precision in scenarios where operands are very close in magnitude
  (func $test_near_cancellation (export "test_near_cancellation") (result f32)
    ;; Test precision when subtracting very close values
    (f32.sub (f32.const 1.0000001) (f32.const 1.0000000))
  )
)