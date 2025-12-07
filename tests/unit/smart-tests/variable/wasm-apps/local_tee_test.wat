(module
  ;; local.tee comprehensive test module
  ;; Tests the dual functionality of local.tee: store to local AND return on stack

  ;; i32 local variable for testing
  (global $g_i32_local (mut i32) (i32.const 0))

  ;; Test function for i32 local.tee operation
  ;; Demonstrates: local.tee stores value in local 0 AND returns it on stack
  (func $test_i32_tee (export "test_i32_tee") (param $input i32) (result i32)
    (local $local_var i32)

    ;; Push input value and tee it to local variable
    ;; local.tee: pops value from stack, stores in local, pushes same value back
    local.get $input
    local.tee $local_var

    ;; At this point:
    ;; - local_var contains the input value
    ;; - stack top contains the same input value
    ;; Return the value from stack (demonstrating tee kept it on stack)
  )

  ;; Test function for i64 local.tee operation
  (func $test_i64_tee (export "test_i64_tee") (param $input i64) (result i64)
    (local $local_var i64)

    local.get $input
    local.tee $local_var
  )

  ;; Test function for f32 local.tee operation
  (func $test_f32_tee (export "test_f32_tee") (param $input f32) (result f32)
    (local $local_var f32)

    local.get $input
    local.tee $local_var
  )

  ;; Test function for f64 local.tee operation
  (func $test_f64_tee (export "test_f64_tee") (param $input f64) (result f64)
    (local $local_var f64)

    local.get $input
    local.tee $local_var
  )

  ;; Test function that both tees a value AND returns the local variable content
  ;; This demonstrates the dual functionality of local.tee and allows verification
  (func $test_i32_tee_and_get (export "test_i32_tee_and_get") (param $input i32) (result i32)
    (local $local_var i32)

    ;; Tee the input to local variable (this stores AND returns the value)
    local.get $input
    local.tee $local_var
    drop  ;; Remove the returned value from stack

    ;; Return the stored local value to verify storage worked
    local.get $local_var
  )

  ;; Test function for repeated tee operations
  ;; Validates that multiple consecutive local.tee operations maintain consistency
  (func $test_repeated_tee (export "test_repeated_tee") (param $input i32) (result i32)
    (local $local_var i32)

    ;; Perform multiple tee operations on the same local variable
    local.get $input
    local.tee $local_var    ;; First tee: store input, keep on stack
    local.tee $local_var    ;; Second tee: store same value again, keep on stack
    local.tee $local_var    ;; Third tee: store same value again, keep on stack

    ;; Verify the local variable still contains the original value
    local.get $local_var
    ;; Stack now has: [input_value, stored_value]
    i32.eq                  ;; Compare them

    ;; If they're equal, return the original input value
    (if (result i32)
      (then
        local.get $input
      )
      (else
        i32.const -1        ;; Error indicator
      )
    )
  )

  ;; Test function for tee with zero values
  ;; Specifically tests zero handling for different types
  (func $test_zero_tee_i32 (export "test_zero_tee_i32") (result i32)
    (local $zero_local i32)

    i32.const 0
    local.tee $zero_local

    ;; Verify zero was stored correctly
    local.get $zero_local
    i32.add                 ;; 0 + 0 = 0
  )

  ;; Test function for tee with special f32 values
  (func $test_special_f32_tee (export "test_special_f32_tee") (param $input f32) (result i32)
    (local $special_local f32)

    local.get $input
    local.tee $special_local

    ;; Check if the teed value equals the stored value
    local.get $special_local
    f32.eq

    ;; Return 1 if equal, 0 if not equal (for NaN cases)
    (if (result i32)
      (then i32.const 1)
      (else i32.const 0)
    )
  )

  ;; Test function for tee with special f64 values
  (func $test_special_f64_tee (export "test_special_f64_tee") (param $input f64) (result i32)
    (local $special_local f64)

    local.get $input
    local.tee $special_local

    ;; Check if the teed value equals the stored value
    local.get $special_local
    f64.eq

    ;; Return 1 if equal, 0 if not equal (for NaN cases)
    (if (result i32)
      (then i32.const 1)
      (else i32.const 0)
    )
  )

  ;; Test boundary values for i32
  (func $test_i32_boundary_values (export "test_i32_boundary_values") (result i32)
    (local $min_local i32)
    (local $max_local i32)

    ;; Test INT32_MIN
    i32.const -2147483648
    local.tee $min_local
    local.get $min_local
    i32.eq

    ;; Test INT32_MAX
    i32.const 2147483647
    local.tee $max_local
    local.get $max_local
    i32.eq

    ;; Both should be equal (1 and 1)
    i32.and
  )

  ;; Test boundary values for i64
  (func $test_i64_boundary_values (export "test_i64_boundary_values") (result i32)
    (local $min_local i64)
    (local $max_local i64)

    ;; Test INT64_MIN
    i64.const -9223372036854775808
    local.tee $min_local
    local.get $min_local
    i64.eq

    ;; Test INT64_MAX
    i64.const 9223372036854775807
    local.tee $max_local
    local.get $max_local
    i64.eq

    ;; Both should be equal (convert i64 results to i32)
    i32.and
  )

  ;; Test that demonstrates stack preservation property of local.tee
  ;; This is the key difference from local.set (which consumes stack value)
  (func $test_stack_preservation (export "test_stack_preservation") (param $input i32) (result i32)
    (local $temp_local i32)

    ;; Push input value
    local.get $input

    ;; Duplicate it by teeing to a local variable
    local.tee $temp_local

    ;; Now we have the original value on stack AND stored in local
    ;; Add them together to prove both exist
    local.get $temp_local
    i32.add                 ;; input + input = 2 * input
  )

  ;; Test multiple local variables with tee operations
  (func $test_multiple_locals (export "test_multiple_locals") (param $val1 i32) (param $val2 i32) (result i32)
    (local $local1 i32)
    (local $local2 i32)

    ;; Tee first value to local1
    local.get $val1
    local.tee $local1

    ;; Tee second value to local2
    local.get $val2
    local.tee $local2

    ;; Add the teed values with the stored locals
    i32.add                 ;; val2 + val1 (from tee operations)
    local.get $local1
    local.get $local2
    i32.add                 ;; val1 + val2 (from stored locals)
    i32.add                 ;; Total: 2 * (val1 + val2)
  )
)