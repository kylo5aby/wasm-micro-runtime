(module
  ;; Import memory for test data if needed
  (memory 1)

  ;; Export memory for external access
  (export "memory" (memory 0))

  ;; Main test function for f64x2.splat operation
  ;; Takes an f64 parameter and returns v128 result of f64x2.splat
  (func $test_f64x2_splat (param $value f64) (result v128)
    local.get $value
    f64x2.splat
  )
  (export "test_f64x2_splat" (func $test_f64x2_splat))

  ;; Test function for positive constant value
  (func $f64x2_splat_positive_const (result v128)
    f64.const 1.0
    f64x2.splat
  )
  (export "f64x2_splat_positive_const" (func $f64x2_splat_positive_const))

  ;; Test function for negative constant value
  (func $f64x2_splat_negative_const (result v128)
    f64.const -1.0
    f64x2.splat
  )
  (export "f64x2_splat_negative_const" (func $f64x2_splat_negative_const))

  ;; Test function for pi constant
  (func $f64x2_splat_pi (result v128)
    f64.const 3.14159265358979323846
    f64x2.splat
  )
  (export "f64x2_splat_pi" (func $f64x2_splat_pi))

  ;; Test function for positive zero
  (func $f64x2_splat_pos_zero (result v128)
    f64.const 0.0
    f64x2.splat
  )
  (export "f64x2_splat_pos_zero" (func $f64x2_splat_pos_zero))

  ;; Test function for negative zero
  (func $f64x2_splat_neg_zero (result v128)
    f64.const -0.0
    f64x2.splat
  )
  (export "f64x2_splat_neg_zero" (func $f64x2_splat_neg_zero))

  ;; Test function for positive infinity
  (func $f64x2_splat_pos_inf (result v128)
    f64.const inf
    f64x2.splat
  )
  (export "f64x2_splat_pos_inf" (func $f64x2_splat_pos_inf))

  ;; Test function for negative infinity
  (func $f64x2_splat_neg_inf (result v128)
    f64.const -inf
    f64x2.splat
  )
  (export "f64x2_splat_neg_inf" (func $f64x2_splat_neg_inf))

  ;; Test function for NaN (not a number)
  (func $f64x2_splat_nan (result v128)
    f64.const nan
    f64x2.splat
  )
  (export "f64x2_splat_nan" (func $f64x2_splat_nan))

  ;; Test function for very small positive value (DBL_MIN equivalent)
  (func $f64x2_splat_tiny_positive (result v128)
    f64.const 2.2250738585072014e-308
    f64x2.splat
  )
  (export "f64x2_splat_tiny_positive" (func $f64x2_splat_tiny_positive))

  ;; Test function for very large positive value (DBL_MAX equivalent)
  (func $f64x2_splat_huge_positive (result v128)
    f64.const 1.7976931348623157e+308
    f64x2.splat
  )
  (export "f64x2_splat_huge_positive" (func $f64x2_splat_huge_positive))

  ;; Test function for fractional value
  (func $f64x2_splat_fraction (result v128)
    f64.const 0.5
    f64x2.splat
  )
  (export "f64x2_splat_fraction" (func $f64x2_splat_fraction))

  ;; Test function for mathematical constant e
  (func $f64x2_splat_e_const (result v128)
    f64.const 2.7182818284590452354
    f64x2.splat
  )
  (export "f64x2_splat_e_const" (func $f64x2_splat_e_const))
)