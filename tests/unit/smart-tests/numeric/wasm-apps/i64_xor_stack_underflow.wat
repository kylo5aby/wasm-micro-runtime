;; WebAssembly Text Format (.wat) for i64.xor Stack Underflow Testing
;; Test module for stack underflow scenarios with i64.xor operation
;;
;; This module provides functions that will be called with insufficient arguments
;; to test WAMR runtime error handling capabilities.

(module
  ;; Test function that expects two operands but will be called with only one
  ;; The underflow will be detected at the C++ test level
  (func (export "test_i64_xor_underflow") (param $operand i64) (result i64)
    ;; This function is correctly formed but will be tested by calling it
    ;; with insufficient arguments from the C++ test framework
    local.get $operand
    i64.const 0
    i64.xor
  )

  ;; Valid function for testing normal operation
  (func (export "test_i64_xor_valid") (param $a i64) (param $b i64) (result i64)
    local.get $a
    local.get $b
    i64.xor
  )
)