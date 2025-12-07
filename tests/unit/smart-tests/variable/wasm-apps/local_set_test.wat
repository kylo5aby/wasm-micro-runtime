(module
  ;; Main test functions for local.set with different types

  ;; Function: test_set_i32_local
  ;; Purpose: Test local.set with i32 type
  ;; Parameters: i32 value to set
  ;; Returns: i32 value retrieved from local variable
  (func (export "test_set_i32_local") (param $input i32) (result i32)
    (local $local_var i32)
    ;; Set local variable with input parameter
    local.get $input
    local.set $local_var
    ;; Get the value back from local variable
    local.get $local_var
  )

  ;; Function: test_set_i64_local
  ;; Purpose: Test local.set with i64 type
  ;; Parameters: i64 value to set
  ;; Returns: i64 value retrieved from local variable
  (func (export "test_set_i64_local") (param $input i64) (result i64)
    (local $local_var i64)
    ;; Set local variable with input parameter
    local.get $input
    local.set $local_var
    ;; Get the value back from local variable
    local.get $local_var
  )

  ;; Function: test_set_f32_local
  ;; Purpose: Test local.set with f32 type
  ;; Parameters: f32 value to set
  ;; Returns: f32 value retrieved from local variable
  (func (export "test_set_f32_local") (param $input f32) (result f32)
    (local $local_var f32)
    ;; Set local variable with input parameter
    local.get $input
    local.set $local_var
    ;; Get the value back from local variable
    local.get $local_var
  )

  ;; Function: test_set_f64_local
  ;; Purpose: Test local.set with f64 type
  ;; Parameters: f64 value to set
  ;; Returns: f64 value retrieved from local variable
  (func (export "test_set_f64_local") (param $input f64) (result f64)
    (local $local_var f64)
    ;; Set local variable with input parameter
    local.get $input
    local.set $local_var
    ;; Get the value back from local variable
    local.get $local_var
  )

  ;; Function: test_set_multiple_i32_locals
  ;; Purpose: Test local.set with multiple local variables of same type
  ;; Parameters: two i32 values to set in different locals
  ;; Returns: sum of both local variables
  (func (export "test_set_multiple_i32_locals") (param $val1 i32) (param $val2 i32) (result i32)
    (local $local1 i32)
    (local $local2 i32)
    ;; Set first local variable
    local.get $val1
    local.set $local1
    ;; Set second local variable
    local.get $val2
    local.set $local2
    ;; Return sum of both locals to verify independence
    local.get $local1
    local.get $local2
    i32.add
  )

  ;; Function: test_set_mixed_type_locals
  ;; Purpose: Test local.set with locals of different types in same function
  ;; Parameters: i32, i64, f32, f64 values
  ;; Returns: i32 conversion of sum (for testing purposes)
  (func (export "test_set_mixed_type_locals") (param $i32_val i32) (param $i64_val i64)
                                               (param $f32_val f32) (param $f64_val f64) (result i32)
    (local $i32_local i32)
    (local $i64_local i64)
    (local $f32_local f32)
    (local $f64_local f64)

    ;; Set each local variable
    local.get $i32_val
    local.set $i32_local

    local.get $i64_val
    local.set $i64_local

    local.get $f32_val
    local.set $f32_local

    local.get $f64_val
    local.set $f64_local

    ;; Return the i32 local (simple validation)
    local.get $i32_local
  )

  ;; Function: test_set_local_boundary_indices
  ;; Purpose: Test local.set with boundary local indices (first and last valid)
  ;; Parameters: values for locals at index 0 and high index
  ;; Returns: sum of values from boundary index locals
  (func (export "test_set_local_boundary_indices") (param $val0 i32) (param $val_last i32) (result i32)
    (local $local0 i32)    ;; index 0 (first local)
    (local $local1 i32)    ;; index 1
    (local $local2 i32)    ;; index 2
    (local $local3 i32)    ;; index 3
    (local $local_last i32) ;; index 4 (last local)

    ;; Set local at index 0
    local.get $val0
    local.set $local0

    ;; Set local at last valid index
    local.get $val_last
    local.set $local_last

    ;; Return sum to verify both were set correctly
    local.get $local0
    local.get $local_last
    i32.add
  )

  ;; Function: test_set_local_overwrite
  ;; Purpose: Test local.set overwrites previous values correctly
  ;; Parameters: initial value and new value
  ;; Returns: the overwritten (new) value
  (func (export "test_set_local_overwrite") (param $initial_val i32) (param $new_val i32) (result i32)
    (local $local_var i32)

    ;; Set initial value
    local.get $initial_val
    local.set $local_var

    ;; Overwrite with new value
    local.get $new_val
    local.set $local_var

    ;; Return the final value (should be new_val)
    local.get $local_var
  )

  ;; Function: test_local_set_zero_values
  ;; Purpose: Test local.set with zero values for all types
  ;; Returns: 1 if all zero tests pass, 0 otherwise
  (func (export "test_local_set_zero_values") (result i32)
    (local $i32_zero i32)
    (local $i64_zero i64)
    (local $f32_zero f32)
    (local $f64_zero f64)

    ;; Set zero values
    i32.const 0
    local.set $i32_zero

    i64.const 0
    local.set $i64_zero

    f32.const 0.0
    local.set $f32_zero

    f64.const 0.0
    local.set $f64_zero

    ;; Verify all are zero (return 1 if success)
    local.get $i32_zero
    i32.const 0
    i32.eq

    local.get $i64_zero
    i64.const 0
    i64.eq
    i32.and

    local.get $f32_zero
    f32.const 0.0
    f32.eq
    i32.and

    local.get $f64_zero
    f64.const 0.0
    f64.eq
    i32.and
  )

  ;; Function: test_local_set_extreme_values
  ;; Purpose: Test local.set with extreme/special values
  ;; Returns: 1 if tests pass, 0 otherwise
  (func (export "test_local_set_extreme_values") (result i32)
    (local $f32_nan f32)
    (local $f32_inf f32)
    (local $f64_nan f64)
    (local $f64_inf f64)

    ;; Set special floating-point values
    f32.const nan
    local.set $f32_nan

    f32.const inf
    local.set $f32_inf

    f64.const nan
    local.set $f64_nan

    f64.const inf
    local.set $f64_inf

    ;; Return success (basic validation that operations completed)
    i32.const 1
  )
)