;; WebAssembly Text Format (.wat) for i32.eqz Stack Validation Testing
;; Test module for validating proper stack management in boundary conditions
;;
;; This module provides minimal stack usage tests to ensure WAMR properly
;; handles stack operations near boundary conditions for i32.eqz operation.

(module
  ;; Test function with minimal stack usage for i32.eqz
  ;; Uses constant value to test zero equality with minimal stack operations
  (func (export "test_minimal_stack") (result i32)
    ;; Test with constant zero - minimal stack usage
    i32.const 0
    i32.eqz  ;; Should return 1 (0 == 0 is true)
  )

  ;; Test function for boundary stack validation
  ;; Tests that valid stack operations work correctly
  (func (export "test_valid_stack") (result i32)
    ;; Test with non-zero constant
    i32.const 42
    i32.eqz  ;; Should return 0 (42 != 0)
  )

  ;; Test function for multiple sequential operations
  ;; Validates stack management across multiple i32.eqz operations
  (func (export "test_sequential_ops") (result i32)
    ;; First operation: test zero
    i32.const 0
    i32.eqz  ;; Returns 1

    ;; Second operation: test the result (1) against zero
    i32.eqz  ;; Returns 0 (1 != 0)
  )

  ;; Test function for stack depth validation
  ;; Ensures proper stack management with deeper operations
  (func (export "test_stack_depth") (result i32)
    ;; Build up some stack depth
    i32.const 0
    i32.const 1
    i32.add      ;; Result: 1
    i32.eqz      ;; Test if result equals zero: 0
    i32.eqz      ;; Test if 0 equals zero: 1
  )
)