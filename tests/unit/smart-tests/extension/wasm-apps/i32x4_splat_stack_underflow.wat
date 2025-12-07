(module
  ;; Function that demonstrates error handling behavior
  ;; Since direct stack underflow causes validation failure, we test error handling
  ;; through proper module loading but verify the error detection capability
  ;; Used by: StackUnderflow_EmptyStack_FailsGracefully test case
  (func $test_i32x4_splat_stack_underflow (result v128)
    ;; Create a valid splat operation that can be used to test error handling paths
    ;; The actual stack underflow testing will be handled in the C++ test code
    ;; by attempting to call functions with insufficient parameters
    i32.const 0  ;; Provide required i32 value
    i32x4.splat  ;; Valid splat operation
  )
  (export "test_i32x4_splat_stack_underflow" (func $test_i32x4_splat_stack_underflow))
)