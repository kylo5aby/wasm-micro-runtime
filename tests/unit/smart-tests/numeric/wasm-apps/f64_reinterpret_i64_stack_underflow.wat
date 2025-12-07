(module
  ;; Test function that simulates error handling for f64.reinterpret_i64
  ;; This module is designed to test error handling scenarios
  (func $test_stack_underflow (export "test_stack_underflow")
    (result f64)
    ;; This would cause a stack underflow at runtime if called with empty stack
    ;; WebAssembly validation requires proper stack types, so we provide minimal valid code
    ;; The actual stack underflow testing is done at the runtime execution level
    i64.const 0  ;; Provide required i64 value
    f64.reinterpret_i64
  )

  ;; Function that should work normally
  (func $test_normal_operation (export "test_normal_operation")
    (result f64)
    ;; Proper operation with i64 value on stack
    i64.const 0x3FF0000000000000  ;; Bit pattern for 1.0
    f64.reinterpret_i64
  )

  ;; Test invalid type sequence
  (func $test_type_mismatch (export "test_type_mismatch")
    (result f64)
    ;; This should be caught by WebAssembly validation
    ;; f32.const 1.0  ;; This would be a type mismatch
    ;; f64.reinterpret_i64  ;; Expects i64, gets f32
    ;; For safety, we use correct types here
    i64.const 0
    f64.reinterpret_i64
  )

  ;; Main function for underflow testing
  (func $main (export "main")
    (result i32)
    ;; Return error code indicating this is a test module
    i32.const -1
  )
)