(module
  ;; Test module for f64.promote_f32 opcode validation
  ;; This module contains functions to test various aspects of f32 to f64 promotion

  ;; Basic promote_f32 function for testing standard f64.promote_f32 operation
  ;; @param $0 f32 - Input value to be promoted
  ;; @return f64 - Promoted double-precision value
  (func $promote_f32 (param $0 f32) (result f64)
    local.get $0    ;; Push f32 input onto stack
    f64.promote_f32 ;; Promote f32 to f64 with precision extension
  )

  ;; Test function for boundary values promotion
  ;; @param $0 f32 - Boundary value (FLT_MAX, FLT_MIN, etc.)
  ;; @return f64 - Promoted boundary value
  (func $promote_boundary (param $0 f32) (result f64)
    local.get $0    ;; Push f32 boundary value onto stack
    f64.promote_f32 ;; Promote to f64 maintaining boundary properties
  )

  ;; Test function for special IEEE 754 values
  ;; @param $0 f32 - Special value (±∞, NaN, ±0.0, subnormal)
  ;; @return f64 - Promoted special value with preserved properties
  (func $promote_special (param $0 f32) (result f64)
    local.get $0    ;; Push f32 special value onto stack
    f64.promote_f32 ;; Promote to f64 preserving IEEE 754 properties
  )

  ;; Test function for precision validation
  ;; Tests that f32 precision is exactly preserved in f64 format
  ;; @param $0 f32 - Input value for precision testing
  ;; @return f64 - Promoted value for precision comparison
  (func $promote_precision_test (param $0 f32) (result f64)
    local.get $0    ;; Push f32 input onto stack
    f64.promote_f32 ;; Promote with exact precision preservation
  )

  ;; Test function for monotonicity validation
  ;; Ensures that if a < b in f32, then promote(a) < promote(b) in f64
  ;; @param $0 f32 - First value
  ;; @param $1 f32 - Second value
  ;; @return f64 - Difference between promoted values
  (func $promote_monotonic_test (param $0 f32) (param $1 f32) (result f64)
    local.get $1    ;; Push second f32 value
    f64.promote_f32 ;; Promote second value to f64
    local.get $0    ;; Push first f32 value
    f64.promote_f32 ;; Promote first value to f64
    f64.sub         ;; Compute difference: promote(b) - promote(a)
  )

  ;; Test function for signed zero preservation
  ;; Validates that signed zeros are correctly preserved during promotion
  ;; @param $0 f32 - Signed zero value (±0.0f)
  ;; @return f64 - Promoted signed zero
  (func $promote_signed_zero (param $0 f32) (result f64)
    local.get $0    ;; Push signed zero onto stack
    f64.promote_f32 ;; Promote preserving sign bit
  )

  ;; Export all test functions for C++ test access
  (export "promote_f32" (func $promote_f32))
  (export "promote_boundary" (func $promote_boundary))
  (export "promote_special" (func $promote_special))
  (export "promote_precision_test" (func $promote_precision_test))
  (export "promote_monotonic_test" (func $promote_monotonic_test))
  (export "promote_signed_zero" (func $promote_signed_zero))
)