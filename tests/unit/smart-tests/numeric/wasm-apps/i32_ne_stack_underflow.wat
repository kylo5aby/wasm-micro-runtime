;; WebAssembly Text Format (.wat) for i32.ne Stack Validation Testing
;; Minimal stack usage test module for validating proper stack management
;;
;; This module provides test functions that use minimal stack operations
;; to validate WAMR's stack management capabilities with i32.ne operations.

(module
  ;; Test function for minimal stack usage
  ;; Uses constant values to minimize stack operations while testing i32.ne
  (func (export "test_minimal_stack") (result i32)
    ;; Load two identical constants to test identity case
    i32.const 42
    i32.const 42
    ;; Execute i32.ne - should return 0 since 42 != 42 is false
    i32.ne
    ;; Result should be 0 (42 equals 42, so ne returns false)
  )

  ;; Test function for boundary stack operations
  ;; Tests stack operations near boundaries to validate proper management
  (func (export "test_boundary_stack") (result i32)
    ;; Test with different values to ensure proper stack handling
    i32.const 100
    i32.const 200
    ;; Execute i32.ne - should return 1 since 100 != 200 is true
    i32.ne
    ;; Result should be 1 (100 does not equal 200)
  )

  ;; Test function for stack validation with zero values
  ;; Minimal operations with zero to test stack edge cases
  (func (export "test_zero_stack") (result i32)
    i32.const 0
    i32.const 0
    ;; Execute i32.ne - should return 0 since 0 != 0 is false
    i32.ne
    ;; Result should be 0 (zero equals zero, so ne returns false)
  )

  ;; Test function for single parameter stack operations
  ;; Uses parameter to test stack management with input values
  (func (export "test_param_stack") (param $input i32) (result i32)
    local.get $input
    i32.const 42
    ;; Execute i32.ne - returns 1 if input != 42, 0 if input == 42
    i32.ne
    ;; Result depends on input value
  )
)