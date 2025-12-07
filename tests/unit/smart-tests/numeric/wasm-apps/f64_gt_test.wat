(module
  ;; Test module for f64.gt opcode (0x64)
  ;; Validates floating-point 64-bit greater-than comparison
  
  ;; Export f64.gt test function
  (func $f64_gt_test (export "f64_gt_test") (param $a f64) (param $b f64) (result i32)
    ;; Load first operand (a)
    local.get $a
    ;; Load second operand (b)  
    local.get $b
    ;; Execute f64.gt comparison: returns 1 if a > b, 0 otherwise
    f64.gt
  )
  
  ;; Additional test functions for comprehensive validation
  
  ;; Test function for zero comparisons
  (func $f64_gt_zero_test (export "f64_gt_zero_test") (param $a f64) (result i32)
    local.get $a
    f64.const 0.0
    f64.gt
  )
  
  ;; Test function for infinity comparisons
  (func $f64_gt_inf_test (export "f64_gt_inf_test") (param $a f64) (result i32)
    local.get $a
    f64.const inf
    f64.gt
  )
  
  ;; Test function for negative infinity comparisons
  (func $f64_gt_neg_inf_test (export "f64_gt_neg_inf_test") (param $a f64) (result i32)
    local.get $a
    f64.const -inf
    f64.gt
  )
  
  ;; Test function for NaN comparisons
  (func $f64_gt_nan_test (export "f64_gt_nan_test") (param $a f64) (result i32)
    local.get $a
    f64.const nan
    f64.gt
  )
  
  ;; Test function for constant vs constant comparison
  (func $f64_gt_const_test (export "f64_gt_const_test") (result i32)
    f64.const 5.5
    f64.const 3.3
    f64.gt
  )
  
  ;; Test function for negative number comparisons
  (func $f64_gt_negative_test (export "f64_gt_negative_test") (result i32)
    f64.const -2.1
    f64.const -5.7
    f64.gt
  )
  
  ;; Test function for mixed sign comparisons
  (func $f64_gt_mixed_test (export "f64_gt_mixed_test") (result i32)
    f64.const 4.2
    f64.const -1.8
    f64.gt
  )
  
  ;; Test function for equal values
  (func $f64_gt_equal_test (export "f64_gt_equal_test") (result i32)
    f64.const 2.5
    f64.const 2.5
    f64.gt
  )
  
  ;; Test function for very small numbers (subnormal range)
  (func $f64_gt_subnormal_test (export "f64_gt_subnormal_test") (result i32)
    ;; Use very small positive number
    f64.const 4.9406564584124654e-324  ;; Smallest positive subnormal
    f64.const 0.0
    f64.gt
  )
  
  ;; Test function for maximum finite values
  (func $f64_gt_max_test (export "f64_gt_max_test") (result i32)
    f64.const 1.7976931348623157e+308  ;; Maximum finite f64
    f64.const 1.0
    f64.gt
  )
  
  ;; Test function for minimum finite values
  (func $f64_gt_min_test (export "f64_gt_min_test") (result i32)
    f64.const 1.0
    f64.const 2.2250738585072014e-308  ;; Minimum positive normal f64
    f64.gt
  )
)