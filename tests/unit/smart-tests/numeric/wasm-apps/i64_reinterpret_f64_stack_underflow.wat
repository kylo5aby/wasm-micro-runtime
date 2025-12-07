(module
  ;; This module is intentionally invalid to test stack underflow detection
  ;; It attempts to use i64.reinterpret_f64 with an empty stack
  (func $test_stack_underflow (export "test_stack_underflow")
    (result i64)
    ;; Intentionally call i64.reinterpret_f64 without pushing f64 value
    ;; This should cause a validation error during module loading
    i64.reinterpret_f64
  )

  ;; This function also has invalid stack usage
  (func $test_insufficient_stack (export "test_insufficient_stack")
    (result i64)
    ;; Push i32 but try to reinterpret as f64->i64 (type mismatch)
    i32.const 42
    i64.reinterpret_f64
  )
)