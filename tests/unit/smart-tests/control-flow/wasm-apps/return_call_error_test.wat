(module
  ;; Function type definitions
  (type $i32_to_i32 (func (param i32) (result i32)))

  ;; Helper function that will trap
  (func $trap_function (type $i32_to_i32) (param $x i32) (result i32)
    ;; Force a trap by dividing by zero
    (i32.div_s (local.get $x) (i32.const 0))
  )

  ;; Valid function for comparison
  (func $valid_function (type $i32_to_i32) (param $x i32) (result i32)
    (local.get $x)
  )

  ;; Test function that will cause runtime trap via return_call
  (func $test_trap_in_tail_call (export "test_trap_in_tail_call")
    (type $i32_to_i32) (param $x i32) (result i32)
    ;; This return_call will cause a division by zero trap
    (return_call $trap_function (local.get $x))
  )

  ;; Test function for type safety (should work correctly)
  (func $test_valid_tail_call (export "test_valid_tail_call")
    (type $i32_to_i32) (param $x i32) (result i32)
    ;; This return_call should work correctly
    (return_call $valid_function (local.get $x))
  )
)