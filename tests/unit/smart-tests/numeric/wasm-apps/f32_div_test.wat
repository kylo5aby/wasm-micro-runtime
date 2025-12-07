(module
  ;; WebAssembly Text Format (WAT) module for f32.div opcode testing
  ;; This module provides comprehensive test functions for validating the IEEE 754
  ;; single-precision division operation implementation in WAMR runtime environments.
  ;;
  ;; The f32.div instruction takes two f32 operands from the stack:
  ;; - First operand (dividend): the value to be divided
  ;; - Second operand (divisor): the value to divide by
  ;; Returns an f32 with the quotient following IEEE 754 division rules

  ;; Basic f32.div test function
  ;; Takes two f32 parameters and returns dividend / divisor
  (func (export "f32_div_test") (param $dividend f32) (param $divisor f32) (result f32)
    local.get $dividend   ;; Load dividend operand onto stack
    local.get $divisor    ;; Load divisor operand onto stack
    f32.div              ;; Execute division: result = dividend / divisor
  )

  ;; Standard division test cases
  (func (export "test_standard_division") (result f32)
    f32.const 6.0        ;; Dividend: 6.0
    f32.const 2.0        ;; Divisor: 2.0
    f32.div              ;; Result: 3.0
  )

  ;; Negative division test
  (func (export "test_negative_division") (result f32)
    f32.const -8.0       ;; Dividend: -8.0
    f32.const 4.0        ;; Divisor: 4.0
    f32.div              ;; Result: -2.0
  )

  ;; Mixed sign division test
  (func (export "test_mixed_sign_division") (result f32)
    f32.const 15.0       ;; Dividend: 15.0
    f32.const -3.0       ;; Divisor: -3.0
    f32.div              ;; Result: -5.0
  )

  ;; Fractional division test
  (func (export "test_fractional_division") (result f32)
    f32.const 1.0        ;; Dividend: 1.0
    f32.const 3.0        ;; Divisor: 3.0
    f32.div              ;; Result: 0.33333334 (single-precision approximation)
  )

  ;; Division by positive zero test (should produce +infinity)
  (func (export "test_division_by_positive_zero") (result f32)
    f32.const 5.0        ;; Dividend: 5.0
    f32.const 0.0        ;; Divisor: +0.0
    f32.div              ;; Result: +infinity
  )

  ;; Division by negative zero test (should produce -infinity)
  (func (export "test_division_by_negative_zero") (result f32)
    f32.const 5.0        ;; Dividend: 5.0
    f32.const -0.0       ;; Divisor: -0.0
    f32.div              ;; Result: -infinity
  )

  ;; Zero divided by zero test (should produce NaN)
  (func (export "test_zero_divided_by_zero") (result f32)
    f32.const 0.0        ;; Dividend: 0.0
    f32.const 0.0        ;; Divisor: 0.0
    f32.div              ;; Result: NaN
  )

  ;; Infinity division tests
  (func (export "test_infinity_by_finite") (result f32)
    f32.const inf        ;; Dividend: +infinity
    f32.const 2.0        ;; Divisor: 2.0
    f32.div              ;; Result: +infinity
  )

  (func (export "test_finite_by_infinity") (result f32)
    f32.const 2.0        ;; Dividend: 2.0
    f32.const inf        ;; Divisor: +infinity
    f32.div              ;; Result: +0.0
  )

  (func (export "test_infinity_by_infinity") (result f32)
    f32.const inf        ;; Dividend: +infinity
    f32.const inf        ;; Divisor: +infinity
    f32.div              ;; Result: NaN
  )

  ;; NaN propagation tests
  (func (export "test_nan_dividend") (result f32)
    f32.const nan        ;; Dividend: NaN
    f32.const 5.0        ;; Divisor: 5.0
    f32.div              ;; Result: NaN
  )

  (func (export "test_nan_divisor") (result f32)
    f32.const 5.0        ;; Dividend: 5.0
    f32.const nan        ;; Divisor: NaN
    f32.div              ;; Result: NaN
  )

  ;; Identity operation tests
  (func (export "test_identity_division") (result f32)
    f32.const 42.5       ;; Dividend: 42.5
    f32.const 1.0        ;; Divisor: 1.0
    f32.div              ;; Result: 42.5 (identity)
  )

  (func (export "test_self_division") (result f32)
    f32.const 5.5        ;; Dividend: 5.5
    f32.const 5.5        ;; Divisor: 5.5
    f32.div              ;; Result: 1.0
  )

  (func (export "test_negation_division") (result f32)
    f32.const 3.14       ;; Dividend: 3.14
    f32.const -1.0       ;; Divisor: -1.0
    f32.div              ;; Result: -3.14 (negation)
  )

  ;; Extreme value tests
  (func (export "test_max_by_min") (result f32)
    f32.const 3.4028235e+38  ;; FLT_MAX
    f32.const 1.1754944e-38  ;; FLT_MIN (smallest positive normal)
    f32.div                  ;; Should overflow to infinity
  )

  (func (export "test_min_by_max") (result f32)
    f32.const 1.1754944e-38  ;; FLT_MIN
    f32.const 3.4028235e+38  ;; FLT_MAX
    f32.div                  ;; Should underflow toward zero
  )

  ;; Large/small value division tests
  (func (export "test_large_by_small") (result f32)
    f32.const 1.0e30     ;; Large value
    f32.const 1.0e-30    ;; Small value
    f32.div              ;; Should overflow to infinity
  )

  (func (export "test_small_by_large") (result f32)
    f32.const 1.0e-30    ;; Small value
    f32.const 1.0e30     ;; Large value
    f32.div              ;; Should underflow to zero
  )

  ;; Sign combination tests
  (func (export "test_positive_by_positive") (result f32)
    f32.const 8.0        ;; Positive dividend
    f32.const 2.0        ;; Positive divisor
    f32.div              ;; Result: +4.0
  )

  (func (export "test_positive_by_negative") (result f32)
    f32.const 8.0        ;; Positive dividend
    f32.const -2.0       ;; Negative divisor
    f32.div              ;; Result: -4.0
  )

  (func (export "test_negative_by_positive") (result f32)
    f32.const -8.0       ;; Negative dividend
    f32.const 2.0        ;; Positive divisor
    f32.div              ;; Result: -4.0
  )

  (func (export "test_negative_by_negative") (result f32)
    f32.const -8.0       ;; Negative dividend
    f32.const -2.0       ;; Negative divisor
    f32.div              ;; Result: +4.0
  )

  ;; Precision testing with repeating decimal results
  (func (export "test_precision_one_third") (result f32)
    f32.const 1.0
    f32.const 3.0
    f32.div              ;; ~0.33333334 in single precision
  )

  (func (export "test_precision_two_thirds") (result f32)
    f32.const 2.0
    f32.const 3.0
    f32.div              ;; ~0.6666667 in single precision
  )

  ;; Special IEEE 754 zero tests
  (func (export "test_positive_zero_result") (result f32)
    f32.const 0.0        ;; +0.0
    f32.const 5.0        ;; Positive divisor
    f32.div              ;; Result: +0.0
  )

  (func (export "test_negative_zero_result") (result f32)
    f32.const -0.0       ;; -0.0
    f32.const 5.0        ;; Positive divisor
    f32.div              ;; Result: -0.0
  )
)