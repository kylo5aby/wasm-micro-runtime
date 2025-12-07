(module
  ;; Test function that causes runtime trap using unreachable
  (func $test_stack_underflow (export "test_stack_underflow") (result externref)
    ;; Force a runtime trap to simulate error conditions
    unreachable
  )

  ;; Test function that demonstrates proper externref handling
  (func $test_proper_stack (export "test_proper_stack") (result externref)
    (ref.null extern)
  )

  ;; Test function for runtime error simulation
  (func $test_runtime_error (export "test_runtime_error") (result i32)
    ;; This will cause a runtime trap
    unreachable
  )

  ;; Test function that properly handles stack operations
  (func $test_valid_operations (export "test_valid_operations") (result i32)
    (i32.const 1)
    (i32.const 2)
    i32.add
  )
)