(module
  ;; Test module for stack underflow simulation
  ;; This module contains functions that can be used to test error conditions

  ;; Function that simulates stack underflow scenarios
  (func $test_stack_underflow (export "test_stack_underflow") (result i32)
    ;; Simulate a stack underflow scenario by returning an error indicator
    ;; In actual testing, the C++ code will call ref.eq with insufficient arguments
    i32.const 0  ;; Error indicator - stack underflow detected
  )

  ;; Function for testing invalid reference operations
  (func $test_malformed_ref_op (export "test_malformed_ref_op") (result i32)
    ;; Return error indicator for malformed operations
    i32.const -1
  )
)