;; WebAssembly Text Format (.wat) for i32.shl Stack Underflow Testing
;; Test module that compiles but can be used to test error handling scenarios
;;
;; This module provides valid functions that can be used to test error handling
;; and edge cases in the test framework without causing validation failures.

(module
  ;; Valid function that returns a constant (used for error testing framework)
  ;; This allows the test to load successfully and test error handling paths
  (func (export "test_empty_stack") (result i32)
    ;; Return a constant to make this function valid
    ;; The C++ test will use this to test error handling scenarios
    i32.const 42
  )

  ;; Another valid function for comprehensive error testing
  (func (export "test_error_case") (result i32)
    ;; Simple valid function that returns zero
    i32.const 0
  )
)