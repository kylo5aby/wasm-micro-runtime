(module
  ;; Global variables for testing global.set opcode
  ;; All globals are mutable to allow global.set operations

  ;; i32 mutable global for basic integer testing
  (global $g_i32 (mut i32) (i32.const 0))

  ;; i64 mutable global for 64-bit integer testing
  (global $g_i64 (mut i64) (i64.const 0))

  ;; f32 mutable global for single-precision float testing
  (global $g_f32 (mut f32) (f32.const 0.0))

  ;; f64 mutable global for double-precision float testing
  (global $g_f64 (mut f64) (f64.const 0.0))

  ;; Additional globals for multiple global testing
  (global $g_test1 (mut i32) (i32.const 1))
  (global $g_test2 (mut i32) (i32.const 2))
  (global $g_test3 (mut i32) (i32.const 3))

  ;; Function to set i32 global and return its value for verification
  ;; Tests: global.set with i32 type, round-trip consistency
  (func $set_and_get_i32_global (param $value i32) (result i32)
    ;; Set the global with provided value
    local.get $value
    global.set $g_i32

    ;; Get the global value to verify it was set correctly
    global.get $g_i32
  )

  ;; Function to set i64 global and return its value for verification
  ;; Tests: global.set with i64 type, boundary values
  (func $set_and_get_i64_global (param $value i64) (result i64)
    ;; Set the global with provided value
    local.get $value
    global.set $g_i64

    ;; Get the global value to verify it was set correctly
    global.get $g_i64
  )

  ;; Function to set f32 global and return its value for verification
  ;; Tests: global.set with f32 type, special floating-point values
  (func $set_and_get_f32_global (param $value f32) (result f32)
    ;; Set the global with provided value
    local.get $value
    global.set $g_f32

    ;; Get the global value to verify it was set correctly
    global.get $g_f32
  )

  ;; Function to set f64 global and return its value for verification
  ;; Tests: global.set with f64 type, IEEE 754 compliance
  (func $set_and_get_f64_global (param $value f64) (result f64)
    ;; Set the global with provided value
    local.get $value
    global.set $g_f64

    ;; Get the global value to verify it was set correctly
    global.get $g_f64
  )

  ;; Function to test setting multiple globals independently
  ;; Tests: Multiple global.set operations, value independence
  (func $set_multiple_globals (result i32)
    ;; Set first global to 100
    i32.const 100
    global.set $g_test1

    ;; Set second global to 200
    i32.const 200
    global.set $g_test2

    ;; Set third global to 300
    i32.const 300
    global.set $g_test3

    ;; Verify all globals have correct values
    ;; Check first global
    global.get $g_test1
    i32.const 100
    i32.ne
    if
      ;; Return 0 if first global is wrong
      i32.const 0
      return
    end

    ;; Check second global
    global.get $g_test2
    i32.const 200
    i32.ne
    if
      ;; Return 0 if second global is wrong
      i32.const 0
      return
    end

    ;; Check third global
    global.get $g_test3
    i32.const 300
    i32.ne
    if
      ;; Return 0 if third global is wrong
      i32.const 0
      return
    end

    ;; All globals are correct, return success
    i32.const 1
  )

  ;; Function to test rapid succession setting of same global
  ;; Tests: Rapid global.set operations, last-set value persistence
  (func $test_rapid_setting (result i32)
    ;; Rapidly set the same global with different values
    i32.const 10
    global.set $g_i32

    i32.const 20
    global.set $g_i32

    i32.const 30
    global.set $g_i32

    i32.const 40
    global.set $g_i32

    i32.const 50
    global.set $g_i32

    ;; Verify the last set value persists
    global.get $g_i32
    i32.const 50
    i32.eq
  )

  ;; Function to test global state persistence across operations
  ;; Tests: Global value persistence, state management
  (func $test_persistence (param $initial i32) (result i32)
    (local $temp i32)
    ;; Set initial value
    local.get $initial
    global.set $g_i32

    ;; Perform some other operations (arithmetic, locals)
    i32.const 1000
    i32.const 500
    i32.add
    drop  ;; Drop the result

    ;; Set and use local variable
    i32.const 999
    local.set $temp
    local.get $temp
    drop  ;; Drop the local value

    ;; Verify global value is unchanged
    global.get $g_i32
  )

  ;; Function to test zero value handling
  ;; Tests: Setting globals to zero, signed zero for floats
  (func $test_zero_values (result i32)
    ;; Test i32 zero
    i32.const 0
    global.set $g_i32
    global.get $g_i32
    i32.const 0
    i32.ne
    if
      i32.const 0
      return
    end

    ;; Test i64 zero
    i64.const 0
    global.set $g_i64
    global.get $g_i64
    i64.const 0
    i64.ne
    if
      i32.const 0
      return
    end

    ;; Test f32 zero
    f32.const 0.0
    global.set $g_f32
    global.get $g_f32
    f32.const 0.0
    f32.ne
    if
      i32.const 0
      return
    end

    ;; Test f64 zero
    f64.const 0.0
    global.set $g_f64
    global.get $g_f64
    f64.const 0.0
    f64.ne
    if
      i32.const 0
      return
    end

    ;; All zero tests passed
    i32.const 1
  )

  ;; Export functions for external testing
  (export "set_and_get_i32_global" (func $set_and_get_i32_global))
  (export "set_and_get_i64_global" (func $set_and_get_i64_global))
  (export "set_and_get_f32_global" (func $set_and_get_f32_global))
  (export "set_and_get_f64_global" (func $set_and_get_f64_global))
  (export "set_multiple_globals" (func $set_multiple_globals))
  (export "test_rapid_setting" (func $test_rapid_setting))
  (export "test_persistence" (func $test_persistence))
  (export "test_zero_values" (func $test_zero_values))
)