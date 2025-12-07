(module
  ;; Test function for basic f64.sqrt operations
  ;; Used to validate perfect squares, non-perfect squares, and fractional values
  (func $test_sqrt_basic (export "test_sqrt_basic") (param $input f64) (result f64)
    (f64.sqrt (local.get $input))
  )

  ;; Test function for boundary value f64.sqrt operations
  ;; Used to validate DBL_MAX, DBL_MIN, and near-zero boundary cases
  (func $test_sqrt_boundaries (export "test_sqrt_boundaries") (param $input f64) (result f64)
    (f64.sqrt (local.get $input))
  )

  ;; Test function for special IEEE 754 value f64.sqrt operations
  ;; Used to validate signed zeros, infinities, and NaN propagation
  (func $test_sqrt_special (export "test_sqrt_special") (param $input f64) (result f64)
    (f64.sqrt (local.get $input))
  )

  ;; Test function for negative input f64.sqrt operations
  ;; Used to validate IEEE 754 compliant NaN return for negative finite numbers
  (func $test_sqrt_negative (export "test_sqrt_negative") (param $input f64) (result f64)
    (f64.sqrt (local.get $input))
  )

  ;; Additional test functions for comprehensive validation scenarios

  ;; Test perfect squares with known exact results
  (func $test_perfect_squares (export "test_perfect_squares") (result f64)
    ;; Test sqrt(16.0) = 4.0
    (f64.sqrt (f64.const 16.0))
  )

  ;; Test fractional perfect squares
  (func $test_fractional_squares (export "test_fractional_squares") (result f64)
    ;; Test sqrt(0.0625) = 0.25 (1/4)^2 = 1/16 -> sqrt = 1/4
    (f64.sqrt (f64.const 0.0625))
  )

  ;; Test very large values approaching DBL_MAX
  (func $test_large_values (export "test_large_values") (result f64)
    ;; Test sqrt(1e+100) should be 1e+50
    (f64.sqrt (f64.const 1e+100))
  )

  ;; Test very small positive values approaching zero
  (func $test_small_values (export "test_small_values") (result f64)
    ;; Test sqrt(1e-100) should be 1e-50
    (f64.sqrt (f64.const 1e-100))
  )

  ;; Test identity case where sqrt(1) = 1
  (func $test_identity (export "test_identity") (result f64)
    (f64.sqrt (f64.const 1.0))
  )

  ;; Test positive zero preservation
  (func $test_positive_zero (export "test_positive_zero") (result f64)
    (f64.sqrt (f64.const 0.0))
  )

  ;; Test negative zero preservation (should return -0.0)
  (func $test_negative_zero (export "test_negative_zero") (result f64)
    (f64.sqrt (f64.const -0.0))
  )

  ;; Test positive infinity handling
  (func $test_positive_infinity (export "test_positive_infinity") (result f64)
    (f64.sqrt (f64.const inf))
  )

  ;; Test negative infinity handling (should return NaN)
  (func $test_negative_infinity (export "test_negative_infinity") (result f64)
    (f64.sqrt (f64.const -inf))
  )

  ;; Test NaN propagation
  (func $test_nan_propagation (export "test_nan_propagation") (result f64)
    (f64.sqrt (f64.const nan))
  )

  ;; Test common mathematical values
  (func $test_sqrt_2 (export "test_sqrt_2") (result f64)
    ;; sqrt(2) ≈ 1.414213562373095
    (f64.sqrt (f64.const 2.0))
  )

  (func $test_sqrt_3 (export "test_sqrt_3") (result f64)
    ;; sqrt(3) ≈ 1.7320508075688772
    (f64.sqrt (f64.const 3.0))
  )

  (func $test_sqrt_10 (export "test_sqrt_10") (result f64)
    ;; sqrt(10) ≈ 3.1622776601683795
    (f64.sqrt (f64.const 10.0))
  )

  ;; Test negative finite numbers (should return NaN)
  (func $test_negative_one (export "test_negative_one") (result f64)
    (f64.sqrt (f64.const -1.0))
  )

  (func $test_negative_large (export "test_negative_large") (result f64)
    (f64.sqrt (f64.const -1000.0))
  )

  (func $test_negative_small (export "test_negative_small") (result f64)
    (f64.sqrt (f64.const -0.001))
  )
)