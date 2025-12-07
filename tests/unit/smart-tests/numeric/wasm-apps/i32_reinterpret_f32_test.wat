(module
  ;; Test function for basic i32.reinterpret_f32 operations
  (func $test_i32_reinterpret_f32 (export "test_i32_reinterpret_f32")
    (param $f32_val f32) (result i32)
    ;; Reinterpret the f32 parameter as i32 bit pattern
    local.get $f32_val
    i32.reinterpret_f32
  )

  ;; Placeholder for stack underflow test (moved to separate module)
  ;; Stack underflow test is in i32_reinterpret_f32_stack_underflow.wasm
  (func $test_valid_operation (export "test_valid_operation")
    (result i32)
    ;; Valid operation to ensure main module loads successfully
    f32.const 1.0
    i32.reinterpret_f32
  )

  ;; Helper function for special values testing
  (func $test_special_values (export "test_special_values")
    (param $input f32) (result i32)
    ;; Direct reinterpretation for special IEEE 754 values
    local.get $input
    i32.reinterpret_f32
  )

  ;; Batch testing function for multiple values
  (func $test_batch_reinterpret (export "test_batch_reinterpret")
    (param $val1 f32) (param $val2 f32) (param $val3 f32)
    (result i32 i32 i32)
    ;; Test multiple reinterpretations in sequence
    local.get $val1
    i32.reinterpret_f32

    local.get $val2
    i32.reinterpret_f32

    local.get $val3
    i32.reinterpret_f32
  )

  ;; Identity test: f32 -> i32 -> f32 roundtrip
  (func $test_roundtrip (export "test_roundtrip")
    (param $input f32) (result f32)
    ;; Test that f32 -> i32 -> f32 preserves original value
    local.get $input
    i32.reinterpret_f32
    f32.reinterpret_i32
  )

  ;; Test various NaN bit patterns
  (func $test_nan_patterns (export "test_nan_patterns")
    (param $nan_bits i32) (result i32)
    ;; Convert i32 bit pattern to f32, then reinterpret back to i32
    local.get $nan_bits
    f32.reinterpret_i32
    i32.reinterpret_f32
  )

  ;; Main entry point for general testing
  (func $main (export "main")
    (result i32)
    ;; Test basic functionality with 1.0f
    f32.const 1.0
    call $test_i32_reinterpret_f32
    drop

    ;; Return 0 to indicate success
    i32.const 0
  )
)