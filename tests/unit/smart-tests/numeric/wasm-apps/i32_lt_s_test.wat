(module
  ;; i32.lt_s test module for signed 32-bit integer less-than comparison
  ;; This module provides test functions for the i32.lt_s WebAssembly opcode
  ;; which performs signed comparison between two i32 values

  ;; Test function for i32.lt_s opcode
  ;; Parameters: two i32 values (a, b)
  ;; Returns: i32 result (1 if a < b signed, 0 otherwise)
  (func $test_i32_lt_s (param $a i32) (param $b i32) (result i32)
    ;; Load first parameter (a) onto stack
    local.get $a
    ;; Load second parameter (b) onto stack
    local.get $b
    ;; Perform signed less-than comparison: a < b (signed)
    ;; Stack effect: [i32, i32] -> [i32]
    ;; Returns 1 if a < b (treating both as signed 32-bit integers), 0 otherwise
    i32.lt_s
  )

  ;; Export the test function to make it accessible from C++
  (export "test_i32_lt_s" (func $test_i32_lt_s))

  ;; Additional helper functions for comprehensive testing

  ;; Test function for boundary value comparisons
  (func $test_boundary_comparison (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.lt_s
  )

  ;; Export boundary test function
  (export "test_boundary_comparison" (func $test_boundary_comparison))

  ;; Test function for zero comparisons
  (func $test_zero_comparison (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.lt_s
  )

  ;; Export zero test function
  (export "test_zero_comparison" (func $test_zero_comparison))

  ;; Test function for mathematical property validation
  (func $test_property_validation (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.lt_s
  )

  ;; Export property validation function
  (export "test_property_validation" (func $test_property_validation))

  ;; Demonstration function showing i32.lt_s in a conditional context
  (func $conditional_test (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.lt_s
    if (result i32)
      ;; If a < b, return 42
      i32.const 42
    else
      ;; If a >= b, return 24
      i32.const 24
    end
  )

  ;; Export conditional test function
  (export "conditional_test" (func $conditional_test))
)