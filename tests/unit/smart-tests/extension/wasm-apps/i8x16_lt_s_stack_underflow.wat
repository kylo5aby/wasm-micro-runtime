(module
  ;; Test module for validating error conditions
  ;; This module contains valid functions that can be used to test
  ;; error handling behavior in controlled scenarios

  ;; Valid function that performs normal i8x16.lt_s operation
  ;; But can be used to test runtime behavior under various conditions
  (func (export "test_stack_underflow") (result v128)
    ;; Create two valid v128 values for comparison
    i32.const -1
    i8x16.splat

    i32.const 0
    i8x16.splat

    ;; Perform normal i8x16.lt_s operation
    i8x16.lt_s
  )

  ;; Function that tests edge of validation boundaries
  (func (export "test_minimal_operation") (result v128)
    ;; Minimal valid i8x16.lt_s operation
    i32.const 127
    i8x16.splat

    i32.const -128
    i8x16.splat

    i8x16.lt_s
  )

  ;; Function for testing with memory operations that could fail
  (memory (export "memory") 1)
  (func (export "test_memory_boundary") (param $addr i32) (result v128)
    ;; Load v128 from potentially invalid memory address
    local.get $addr
    v128.load

    ;; Compare with zero vector
    i32.const 0
    i8x16.splat

    i8x16.lt_s
  )
)