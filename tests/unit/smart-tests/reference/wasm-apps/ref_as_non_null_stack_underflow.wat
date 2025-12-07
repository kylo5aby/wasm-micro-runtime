(module
  ;; Simple module to test stack underflow-like scenarios

  ;; Test function that simulates stack underflow behavior
  (func $test_stack_underflow (export "test_stack_underflow") (result i32)
    ;; This function will cause a runtime error by calling unreachable
    ;; to simulate the kind of trap that would occur with stack underflow
    unreachable
    ;; Should never reach here
    i32.const 1
  )
)