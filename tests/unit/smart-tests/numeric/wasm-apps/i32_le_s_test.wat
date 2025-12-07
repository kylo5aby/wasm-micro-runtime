(module
  ;; i32.le_s test module for signed 32-bit integer less-than-or-equal comparison
  ;; This module provides test functions for the i32.le_s WebAssembly opcode
  ;; which performs signed less-than-or-equal comparison between two i32 values

  ;; Test function for basic i32.le_s opcode functionality
  ;; Parameters: two i32 values (a, b)
  ;; Returns: i32 result (1 if a <= b signed, 0 otherwise)
  (func $test_i32_le_s_basic (param $a i32) (param $b i32) (result i32)
    ;; Load first parameter (a) onto stack
    local.get $a
    ;; Load second parameter (b) onto stack
    local.get $b
    ;; Perform signed less-than-or-equal comparison: a <= b (signed)
    ;; Stack effect: [i32, i32] -> [i32]
    ;; Returns 1 if a <= b (treating both as signed 32-bit integers), 0 otherwise
    i32.le_s
  )

  ;; Export the basic test function to make it accessible from C++
  (export "test_i32_le_s_basic" (func $test_i32_le_s_basic))

  ;; Test function for boundary value comparisons (INT32_MIN, INT32_MAX)
  ;; Handles extreme values at the limits of 32-bit signed integer range
  (func $test_i32_le_s_boundary (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_s
  )

  ;; Export boundary test function
  (export "test_i32_le_s_boundary" (func $test_i32_le_s_boundary))

  ;; Test function for zero comparisons
  ;; Handles all scenarios involving zero: 0<=0, 0<=positive, 0<=negative, etc.
  (func $test_i32_le_s_zero (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_s
  )

  ;; Export zero test function
  (export "test_i32_le_s_zero" (func $test_i32_le_s_zero))

  ;; Test function for signed semantics validation
  ;; Ensures signed interpretation vs unsigned bit-pattern comparison
  (func $test_i32_le_s_signed_semantics (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_s
  )

  ;; Export signed semantics validation function
  (export "test_i32_le_s_signed_semantics" (func $test_i32_le_s_signed_semantics))

  ;; Demonstration function showing i32.le_s in a conditional context
  ;; Shows practical usage of less-than-or-equal comparison in control flow
  (func $conditional_le_s_test (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_s
    if (result i32)
      ;; If a <= b, return 100
      i32.const 100
    else
      ;; If a > b, return 200
      i32.const 200
    end
  )

  ;; Export conditional test function
  (export "conditional_le_s_test" (func $conditional_le_s_test))

  ;; Comprehensive test function combining multiple i32.le_s operations
  ;; Demonstrates complex usage patterns and multiple comparisons
  (func $complex_le_s_test (param $x i32) (param $y i32) (param $z i32) (result i32)
    (local $temp i32)

    ;; First comparison: x <= y
    local.get $x
    local.get $y
    i32.le_s
    local.set $temp

    ;; Second comparison: y <= z
    local.get $y
    local.get $z
    i32.le_s

    ;; Combine results with AND operation (both must be true)
    local.get $temp
    i32.and

    ;; Returns 1 if both (x <= y) AND (y <= z), 0 otherwise
  )

  ;; Export complex test function
  (export "complex_le_s_test" (func $complex_le_s_test))

  ;; Edge case function for reflexivity testing (a <= a should always be true)
  (func $reflexivity_test (param $value i32) (result i32)
    local.get $value
    local.get $value
    i32.le_s
    ;; Should always return 1 for any value (reflexivity property)
  )

  ;; Export reflexivity test function
  (export "reflexivity_test" (func $reflexivity_test))
)