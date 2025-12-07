(module
  ;; F32.GT Test Module
  ;; This module provides test functions for the f32.gt (greater than) opcode
  ;; testing IEEE 754 compliant floating-point comparison operations

  ;; Export f32.gt test function for C++ test integration
  (func $f32_gt_test (export "f32_gt_test") (param $a f32) (param $b f32) (result i32)
    ;; Load both f32 parameters onto stack
    local.get $a    ;; Push first operand (a)
    local.get $b    ;; Push second operand (b)

    ;; Execute f32.gt comparison: returns 1 if a > b, 0 otherwise
    f32.gt          ;; Perform f32.gt operation: stack [a, b] -> [i32 result]
  )

  ;; Additional utility functions for comprehensive testing scenarios

  ;; Test function for positive infinity comparison
  (func $test_pos_inf (export "test_pos_inf") (param $val f32) (result i32)
    f32.const inf   ;; Load positive infinity
    local.get $val  ;; Load test value
    f32.gt          ;; Test: +∞ > val
  )

  ;; Test function for negative infinity comparison
  (func $test_neg_inf (export "test_neg_inf") (param $val f32) (result i32)
    f32.const -inf  ;; Load negative infinity
    local.get $val  ;; Load test value
    f32.gt          ;; Test: -∞ > val
  )

  ;; Test function for NaN comparison
  (func $test_nan (export "test_nan") (param $val f32) (result i32)
    f32.const nan   ;; Load NaN (quiet NaN)
    local.get $val  ;; Load test value
    f32.gt          ;; Test: NaN > val (should always be 0)
  )

  ;; Test function for zero comparisons (+0.0 vs -0.0)
  (func $test_zero_comparison (export "test_zero_comparison") (result i32)
    f32.const 0.0   ;; Load +0.0
    f32.const -0.0  ;; Load -0.0
    f32.gt          ;; Test: +0.0 > -0.0 (should be 0, IEEE 754: +0 == -0)
  )

  ;; Test function for boundary value FLT_MAX
  (func $test_flt_max (export "test_flt_max") (param $val f32) (result i32)
    f32.const 0x1.fffffep+127  ;; FLT_MAX = 3.40282347e+38
    local.get $val             ;; Load test value
    f32.gt                     ;; Test: FLT_MAX > val
  )

  ;; Test function for boundary value FLT_MIN (smallest positive normal)
  (func $test_flt_min (export "test_flt_min") (param $val f32) (result i32)
    f32.const 0x1p-126  ;; FLT_MIN = 1.17549435e-38 (smallest positive normal)
    local.get $val      ;; Load test value
    f32.gt              ;; Test: FLT_MIN > val
  )
)