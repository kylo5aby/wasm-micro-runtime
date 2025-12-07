;; WebAssembly Text Format (.wat) for Stack Underflow Testing
;; Test module to validate module loading with invalid i32.add usage
;;
;; This module will be used to test that WAMR properly rejects
;; modules with invalid stack operations during validation.

(module
  ;; Valid function that will be used for testing
  ;; The test will demonstrate that invalid modules are rejected
  (func (export "test_empty_stack") (result i32)
    ;; This function returns a constant to make the module valid
    ;; The actual underflow testing is done by attempting to load
    ;; an invalid module in the C++ test code
    i32.const 0
  )

  ;; Another valid function
  (func (export "test_single_operand") (result i32)
    i32.const 1
  )
)