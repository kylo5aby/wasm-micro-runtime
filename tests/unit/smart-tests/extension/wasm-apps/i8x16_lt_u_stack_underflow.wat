(module
  ;; Import memory for test data
  (memory (export "memory") 1)

  ;; Test function that demonstrates proper stack usage (no underflow)
  (func (export "test_stack_underflow")
    ;; Create two valid v128 vectors on stack
    i32.const 100
    i8x16.splat

    i32.const 200
    i8x16.splat

    ;; Perform i8x16.lt_u comparison with proper operands
    i8x16.lt_u

    ;; Drop result to clean stack
    drop
  )

  ;; Helper function for stack validation testing
  (func (export "validate_stack_state")
    ;; Push multiple vectors for comprehensive stack testing
    i32.const 50
    i8x16.splat

    i32.const 150
    i8x16.splat

    ;; Perform comparison
    i8x16.lt_u

    ;; Store result validation
    drop
  )

  ;; Test function for proper SIMD stack management
  (func (export "simd_stack_management")
    ;; Create test vectors
    i32.const 0
    i8x16.splat

    i32.const 255
    i8x16.splat

    ;; Test the comparison operation
    i8x16.lt_u

    ;; Clean up stack
    drop
  )

  ;; Function to test multiple sequential operations
  (func (export "sequential_operations_test")
    ;; First comparison
    i32.const 10
    i8x16.splat
    i32.const 20
    i8x16.splat
    i8x16.lt_u
    drop

    ;; Second comparison
    i32.const 30
    i8x16.splat
    i32.const 40
    i8x16.splat
    i8x16.lt_u
    drop

    ;; Third comparison
    i32.const 50
    i8x16.splat
    i32.const 60
    i8x16.splat
    i8x16.lt_u
    drop
  )
)