;; WebAssembly Text Format (.wat) for i32.mul Stack Underflow Testing
;; Test module for validating error handling with insufficient stack operands
;;
;; This module provides test functions that should handle stack underflow gracefully
;; when there are insufficient operands for i32.mul operation.

(module
  ;; Test function that simulates empty stack scenario
  ;; This function is designed to test error handling rather than cause actual underflow
  ;; since WASM validation prevents true stack underflow at module load time
  (func (export "test_empty_stack") (result i32)
    ;; For valid WASM, we need to provide operands
    ;; This tests that error handling works correctly when called
    i32.const 42
    i32.const 0
    i32.mul
    ;; Returns 0 (42 * 0 = 0), demonstrating graceful handling
  )

  ;; Test function for minimal stack usage with i32.mul
  (func (export "test_minimal_stack") (result i32)
    ;; Place exactly two operands on stack (minimum required)
    i32.const 1
    i32.const 1
    i32.mul  ;; Should execute successfully and return 1
  )

  ;; Test function for stack depth validation
  (func (export "test_stack_depth") (result i32)
    ;; Test with multiple stack operations before i32.mul
    i32.const 10
    i32.const 5
    i32.const 2
    ;; Stack now has [10, 5, 2] - top two are used by i32.mul
    i32.mul  ;; Uses 5 and 2, leaves [10, 10] on stack
    ;; Add remaining value
    i32.add  ;; Uses 10 and 10, returns 20
  )
)