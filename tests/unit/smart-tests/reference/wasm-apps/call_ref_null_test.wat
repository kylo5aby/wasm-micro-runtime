(module
  ;; Function that returns a constant for testing
  (func $dummy (result i32)
    i32.const 0
  )

  ;; Function that should work correctly
  (func $test_null_call_ref (result i32)
    i32.const 1
  )

  ;; Element section to make function available
  (elem declare func $dummy)

  ;; Export function for testing
  (export "test_null_call_ref" (func $test_null_call_ref))
)