(module
  ;; Test function that demonstrates error handling capabilities
  ;; This creates a valid module but tests error conditions at runtime
  (func $stack_underflow_test (export "stack_underflow_test") (result i64)
    ;; This is a placeholder for error testing - valid WAT but may be used
    ;; to test error handling in the C++ test through controlled scenarios
    i32.const 0
    i64.extend_i32_s
  )
)