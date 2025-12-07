(module
  ;; Test function for basic i64.reinterpret_f64 operations
  (func $test_i64_reinterpret_f64 (export "test_i64_reinterpret_f64")
    (param $f64_val f64) (result i64)
    ;; Reinterpret the f64 parameter as i64 bit pattern
    local.get $f64_val
    i64.reinterpret_f64
  )

  ;; Placeholder for stack underflow test (moved to separate module)
  ;; Stack underflow test is in i64_reinterpret_f64_stack_underflow.wasm
  (func $test_valid_operation (export "test_valid_operation")
    (result i64)
    ;; Valid operation to ensure main module loads successfully
    f64.const 1.0
    i64.reinterpret_f64
  )

  ;; Helper function for special values testing
  (func $test_special_values (export "test_special_values")
    (param $input f64) (result i64)
    ;; Direct reinterpretation for special IEEE 754 values
    local.get $input
    i64.reinterpret_f64
  )

  ;; Test boundary values like DBL_MIN, DBL_MAX
  (func $test_boundary_values (export "test_boundary_values")
    (param $val f64) (result i64)
    ;; Test boundary condition reinterpretations
    local.get $val
    i64.reinterpret_f64
  )

  ;; Batch testing function for multiple values
  (func $test_batch_reinterpret (export "test_batch_reinterpret")
    (param $val1 f64) (param $val2 f64) (param $val3 f64)
    (result i64 i64 i64)
    ;; Test multiple reinterpretations in sequence
    local.get $val1
    i64.reinterpret_f64

    local.get $val2
    i64.reinterpret_f64

    local.get $val3
    i64.reinterpret_f64
  )

  ;; Identity test: f64 -> i64 -> f64 roundtrip
  (func $test_roundtrip (export "test_roundtrip")
    (param $input f64) (result f64)
    ;; Test that f64 -> i64 -> f64 preserves original value
    local.get $input
    i64.reinterpret_f64
    f64.reinterpret_i64
  )

  ;; Test various NaN bit patterns
  (func $test_nan_patterns (export "test_nan_patterns")
    (param $nan_bits i64) (result i64)
    ;; Convert i64 bit pattern to f64, then reinterpret back to i64
    local.get $nan_bits
    f64.reinterpret_i64
    i64.reinterpret_f64
  )

  ;; Test zero bit patterns (positive and negative zero)
  (func $test_zero_patterns (export "test_zero_patterns")
    (param $pos_zero f64) (param $neg_zero f64) (result i64 i64)
    ;; Test both positive and negative zero bit patterns
    local.get $pos_zero
    i64.reinterpret_f64

    local.get $neg_zero
    i64.reinterpret_f64
  )

  ;; Test infinity bit patterns
  (func $test_infinity_patterns (export "test_infinity_patterns")
    (param $pos_inf f64) (param $neg_inf f64) (result i64 i64)
    ;; Test positive and negative infinity bit patterns
    local.get $pos_inf
    i64.reinterpret_f64

    local.get $neg_inf
    i64.reinterpret_f64
  )

  ;; Test subnormal values
  (func $test_subnormal_values (export "test_subnormal_values")
    (param $subnormal f64) (result i64)
    ;; Test subnormal value reinterpretation
    local.get $subnormal
    i64.reinterpret_f64
  )

  ;; Test specific bit patterns for accuracy validation
  (func $test_bit_accuracy (export "test_bit_accuracy")
    (param $pattern f64) (result i64)
    ;; Test specific bit pattern preservation
    local.get $pattern
    i64.reinterpret_f64
  )

  ;; Main entry point for general testing
  (func $main (export "main")
    (result i64)
    ;; Test basic functionality with 1.0
    f64.const 1.0
    call $test_i64_reinterpret_f64
    drop

    ;; Return 0 to indicate success
    i64.const 0
  )
)