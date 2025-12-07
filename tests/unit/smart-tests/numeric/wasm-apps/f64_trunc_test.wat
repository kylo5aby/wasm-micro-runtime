(module
  ;; f64.trunc test module for comprehensive opcode validation
  ;; This module provides test functions for the f64.trunc WebAssembly instruction
  ;; covering all categories: basic functionality, boundary conditions, special values, and edge cases

  ;; Main test function: f64_trunc_test
  ;; Takes an f64 parameter and returns the result of f64.trunc operation
  (func $f64_trunc_test (export "f64_trunc_test") (param f64) (result f64)
    local.get 0    ;; Load the input parameter
    f64.trunc      ;; Apply f64.trunc operation
  )

  ;; Additional utility functions for comprehensive testing

  ;; Test function for verifying f64.trunc with constant values
  ;; Useful for testing compiler optimizations and constant folding
  (func $f64_trunc_const_positive (export "f64_trunc_const_positive") (result f64)
    f64.const 5.7  ;; Constant positive fractional value
    f64.trunc      ;; Should return 5.0
  )

  (func $f64_trunc_const_negative (export "f64_trunc_const_negative") (result f64)
    f64.const -5.7 ;; Constant negative fractional value
    f64.trunc      ;; Should return -5.0
  )

  ;; Test function for NaN handling
  (func $f64_trunc_nan (export "f64_trunc_nan") (result f64)
    f64.const nan  ;; NaN constant
    f64.trunc      ;; Should preserve NaN
  )

  ;; Test function for infinity handling
  (func $f64_trunc_infinity (export "f64_trunc_infinity") (result f64)
    f64.const inf  ;; Positive infinity
    f64.trunc      ;; Should preserve +infinity
  )

  (func $f64_trunc_neg_infinity (export "f64_trunc_neg_infinity") (result f64)
    f64.const -inf ;; Negative infinity
    f64.trunc      ;; Should preserve -infinity
  )

  ;; Test function for zero handling
  (func $f64_trunc_zero (export "f64_trunc_zero") (result f64)
    f64.const 0.0  ;; Positive zero
    f64.trunc      ;; Should preserve +0.0
  )

  (func $f64_trunc_neg_zero (export "f64_trunc_neg_zero") (result f64)
    f64.const -0.0 ;; Negative zero
    f64.trunc      ;; Should preserve -0.0
  )

  ;; Test function for boundary values
  (func $f64_trunc_max (export "f64_trunc_max") (result f64)
    f64.const 1.7976931348623157e+308  ;; Maximum f64 value
    f64.trunc                          ;; Should remain unchanged
  )

  (func $f64_trunc_min (export "f64_trunc_min") (result f64)
    f64.const -1.7976931348623157e+308 ;; Minimum f64 value
    f64.trunc                          ;; Should remain unchanged
  )

  ;; Test function for precision boundary (2^53)
  (func $f64_trunc_precision_max (export "f64_trunc_precision_max") (result f64)
    f64.const 9007199254740992.0  ;; Largest exactly representable integer
    f64.trunc                     ;; Should remain unchanged
  )

  ;; Test function for subnormal values
  (func $f64_trunc_subnormal (export "f64_trunc_subnormal") (result f64)
    f64.const 4.9406564584124654e-324  ;; Smallest positive subnormal
    f64.trunc                          ;; Should truncate to 0.0
  )

  ;; Test function for values near integer boundaries
  (func $f64_trunc_near_one (export "f64_trunc_near_one") (result f64)
    f64.const 0.9999999999999999  ;; Just below 1.0
    f64.trunc                     ;; Should truncate to 0.0
  )

  (func $f64_trunc_near_neg_one (export "f64_trunc_near_neg_one") (result f64)
    f64.const -0.9999999999999999 ;; Just above -1.0
    f64.trunc                     ;; Should truncate to -0.0
  )

  ;; Test function for mathematical property validation
  ;; Demonstrates towards-zero truncation behavior
  (func $f64_trunc_towards_zero_pos (export "f64_trunc_towards_zero_pos") (result f64)
    f64.const 3.7  ;; Positive fractional
    f64.trunc      ;; Should return 3.0 (towards zero)
  )

  (func $f64_trunc_towards_zero_neg (export "f64_trunc_towards_zero_neg") (result f64)
    f64.const -3.7 ;; Negative fractional
    f64.trunc      ;; Should return -3.0 (towards zero)
  )

  ;; Test function for idempotent property validation
  ;; trunc(trunc(x)) should equal trunc(x)
  (func $f64_trunc_idempotent (export "f64_trunc_idempotent") (param f64) (result f64)
    local.get 0    ;; Load parameter
    f64.trunc      ;; First truncation
    f64.trunc      ;; Second truncation (should be identical)
  )

  ;; Test function for multiple operations in sequence
  ;; Validates that f64.trunc doesn't affect subsequent operations
  (func $f64_trunc_sequence (export "f64_trunc_sequence") (param f64) (result f64)
    local.get 0    ;; Load parameter
    f64.trunc      ;; Truncate
    f64.const 1.0  ;; Add constant
    f64.add        ;; Add operation should work normally
    f64.const 1.0  ;; Subtract constant
    f64.sub        ;; Result should be trunc(param)
  )
)