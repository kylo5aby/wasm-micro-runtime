(module
  ;; WebAssembly Text Format (WAT) module for f64.nearest opcode testing
  ;; This module provides comprehensive test functions for validating the IEEE 754
  ;; banker's rounding (round half to even) operation implementation in WAMR runtime environments.
  ;;
  ;; The f64.nearest instruction takes one f64 operand from the stack and returns
  ;; the rounded value to the nearest integer using banker's rounding rules:
  ;; - Round half-values (x.5) to the nearest even integer
  ;; - Preserve special IEEE 754 values (NaN, infinity, signed zeros)

  ;; Basic f64.nearest test function
  ;; Takes one f64 parameter and returns nearest(input) using banker's rounding
  (func (export "test_f64_nearest") (param $input f64) (result f64)
    local.get $input     ;; Load input operand onto stack
    f64.nearest         ;; Execute nearest: result = nearest(input)
  )

  ;; Banker's rounding validation functions
  ;; Test specific half-value cases to validate round-half-to-even behavior

  ;; Test 0.5 -> 0 (round to even)
  (func (export "test_half_to_zero") (result f64)
    f64.const 0.5       ;; Half-value that should round to 0 (even)
    f64.nearest
  )

  ;; Test 1.5 -> 2 (round to even)
  (func (export "test_half_to_two") (result f64)
    f64.const 1.5       ;; Half-value that should round to 2 (even)
    f64.nearest
  )

  ;; Test 2.5 -> 2 (round to even)
  (func (export "test_half_stays_two") (result f64)
    f64.const 2.5       ;; Half-value that should round to 2 (even)
    f64.nearest
  )

  ;; Test 3.5 -> 4 (round to even)
  (func (export "test_half_to_four") (result f64)
    f64.const 3.5       ;; Half-value that should round to 4 (even)
    f64.nearest
  )

  ;; Test 4.5 -> 4 (round to even)
  (func (export "test_half_stays_four") (result f64)
    f64.const 4.5       ;; Half-value that should round to 4 (even)
    f64.nearest
  )

  ;; Test 5.5 -> 6 (round to even)
  (func (export "test_half_to_six") (result f64)
    f64.const 5.5       ;; Half-value that should round to 6 (even)
    f64.nearest
  )

  ;; Negative banker's rounding tests
  ;; Test -2.5 -> -2 (round to even magnitude)
  (func (export "test_negative_half_to_negative_two") (result f64)
    f64.const -2.5      ;; Negative half-value that should round to -2 (even magnitude)
    f64.nearest
  )

  ;; Test -3.5 -> -4 (round to even magnitude)
  (func (export "test_negative_half_to_negative_four") (result f64)
    f64.const -3.5      ;; Negative half-value that should round to -4 (even magnitude)
    f64.nearest
  )

  ;; Test -4.5 -> -4 (round to even magnitude)
  (func (export "test_negative_half_stays_negative_four") (result f64)
    f64.const -4.5      ;; Negative half-value that should round to -4 (even magnitude)
    f64.nearest
  )

  ;; Test -5.5 -> -6 (round to even magnitude)
  (func (export "test_negative_half_to_negative_six") (result f64)
    f64.const -5.5      ;; Negative half-value that should round to -6 (even magnitude)
    f64.nearest
  )

  ;; Standard rounding test functions
  ;; Test values that are not half-values

  ;; Test 3.2 -> 3 (round down)
  (func (export "test_round_down_positive") (result f64)
    f64.const 3.2       ;; Should round down to 3
    f64.nearest
  )

  ;; Test 5.7 -> 6 (round up)
  (func (export "test_round_up_positive") (result f64)
    f64.const 5.7       ;; Should round up to 6
    f64.nearest
  )

  ;; Test -2.3 -> -2 (round toward zero from negative)
  (func (export "test_round_negative_toward_zero") (result f64)
    f64.const -2.3      ;; Should round to -2 (toward zero)
    f64.nearest
  )

  ;; Test -6.7 -> -7 (round away from zero from negative)
  (func (export "test_round_negative_away_zero") (result f64)
    f64.const -6.7      ;; Should round to -7 (away from zero)
    f64.nearest
  )

  ;; Special value test functions
  ;; IEEE 754 special values should be preserved

  ;; NaN preservation test
  (func (export "test_nan_preservation") (result f64)
    f64.const nan       ;; Not-a-Number input
    f64.nearest         ;; Should preserve NaN
  )

  ;; Positive infinity preservation test
  (func (export "test_positive_infinity_preservation") (result f64)
    f64.const inf       ;; Positive infinity input
    f64.nearest         ;; Should preserve +∞
  )

  ;; Negative infinity preservation test
  (func (export "test_negative_infinity_preservation") (result f64)
    f64.const -inf      ;; Negative infinity input
    f64.nearest         ;; Should preserve -∞
  )

  ;; Signed zero preservation tests
  (func (export "test_positive_zero_preservation") (result f64)
    f64.const 0.0       ;; Positive zero input
    f64.nearest         ;; Should preserve +0.0
  )

  (func (export "test_negative_zero_preservation") (result f64)
    f64.const -0.0      ;; Negative zero input
    f64.nearest         ;; Should preserve -0.0
  )

  ;; Integer identity operation tests
  ;; Integer values should remain unchanged

  (func (export "test_integer_identity_positive") (result f64)
    f64.const 42.0      ;; Integer input should remain unchanged
    f64.nearest
  )

  (func (export "test_integer_identity_negative") (result f64)
    f64.const -17.0     ;; Negative integer should remain unchanged
    f64.nearest
  )

  (func (export "test_integer_identity_zero") (result f64)
    f64.const 0.0       ;; Zero should remain unchanged
    f64.nearest
  )

  ;; Large value tests
  ;; Test values near the boundary of representable integers

  ;; Test large positive value near DBL_MAX
  (func (export "test_large_positive_value") (result f64)
    f64.const 1.7976931348623157e+308  ;; Large finite value
    f64.nearest
  )

  ;; Test small positive value near DBL_MIN
  (func (export "test_small_positive_value") (result f64)
    f64.const 2.2250738585072014e-308  ;; Small normalized value
    f64.nearest
  )

  ;; Test subnormal value
  (func (export "test_subnormal_value") (result f64)
    f64.const 4.9406564584124654e-324  ;; Smallest positive subnormal
    f64.nearest
  )

  ;; Precision boundary tests
  ;; Test values near the precision limits where rounding behavior matters

  (func (export "test_precision_boundary_near_one") (result f64)
    f64.const 1.0000000000000002       ;; 1.0 + epsilon
    f64.nearest
  )

  (func (export "test_precision_boundary_near_two") (result f64)
    f64.const 1.9999999999999998       ;; 2.0 - epsilon
    f64.nearest
  )
)