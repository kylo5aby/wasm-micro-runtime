(module
  ;; Main test function for f64.reinterpret_i64 operations
  (func $f64_reinterpret_i64 (export "f64_reinterpret_i64")
    (param $i64_val i64) (result f64)
    ;; Reinterpret the i64 parameter as f64 bit pattern
    local.get $i64_val
    f64.reinterpret_i64
  )

  ;; Test function for basic f64.reinterpret_i64 operations
  (func $test_f64_reinterpret_i64 (export "test_f64_reinterpret_i64")
    (param $i64_val i64) (result f64)
    ;; Reinterpret the i64 parameter as f64 bit pattern
    local.get $i64_val
    f64.reinterpret_i64
  )

  ;; Placeholder for stack underflow test (moved to separate module)
  ;; Stack underflow test is in f64_reinterpret_i64_stack_underflow.wasm
  (func $test_valid_operation (export "test_valid_operation")
    (result f64)
    ;; Valid operation to ensure main module loads successfully
    i64.const 0x3FF0000000000000  ;; Bit pattern for 1.0
    f64.reinterpret_i64
  )

  ;; Helper function for special values testing
  (func $test_special_values (export "test_special_values")
    (param $input i64) (result f64)
    ;; Direct reinterpretation for special IEEE 754 values
    local.get $input
    f64.reinterpret_i64
  )

  ;; Test boundary values like INT64_MIN, INT64_MAX
  (func $test_boundary_values (export "test_boundary_values")
    (param $val i64) (result f64)
    ;; Test boundary condition reinterpretations
    local.get $val
    f64.reinterpret_i64
  )

  ;; Batch testing function for multiple values
  (func $test_batch_reinterpret (export "test_batch_reinterpret")
    (param $val1 i64) (param $val2 i64) (param $val3 i64)
    (result f64 f64 f64)
    ;; Test multiple reinterpretations in sequence
    local.get $val1
    f64.reinterpret_i64

    local.get $val2
    f64.reinterpret_i64

    local.get $val3
    f64.reinterpret_i64
  )

  ;; Identity test: i64 -> f64 -> i64 roundtrip
  (func $test_roundtrip (export "test_roundtrip")
    (param $input i64) (result i64)
    ;; Test that i64 -> f64 -> i64 preserves original value
    local.get $input
    f64.reinterpret_i64
    i64.reinterpret_f64
  )

  ;; Test various NaN bit patterns
  (func $test_nan_patterns (export "test_nan_patterns")
    (param $nan_bits i64) (result f64)
    ;; Convert i64 bit pattern to f64 (should produce NaN for NaN patterns)
    local.get $nan_bits
    f64.reinterpret_i64
  )

  ;; Test zero bit patterns (positive and negative zero)
  (func $test_zero_patterns (export "test_zero_patterns")
    (param $pos_zero i64) (param $neg_zero i64) (result f64 f64)
    ;; Test both positive and negative zero bit patterns
    local.get $pos_zero
    f64.reinterpret_i64

    local.get $neg_zero
    f64.reinterpret_i64
  )

  ;; Test infinity bit patterns
  (func $test_infinity_patterns (export "test_infinity_patterns")
    (param $pos_inf i64) (param $neg_inf i64) (result f64 f64)
    ;; Test positive and negative infinity bit patterns
    local.get $pos_inf
    f64.reinterpret_i64

    local.get $neg_inf
    f64.reinterpret_i64
  )

  ;; Test subnormal values
  (func $test_subnormal_values (export "test_subnormal_values")
    (param $subnormal i64) (result f64)
    ;; Test subnormal value reinterpretation
    local.get $subnormal
    f64.reinterpret_i64
  )

  ;; Test specific bit patterns for accuracy validation
  (func $test_bit_accuracy (export "test_bit_accuracy")
    (param $pattern i64) (result f64)
    ;; Test specific bit pattern preservation
    local.get $pattern
    f64.reinterpret_i64
  )

  ;; Test with typical IEEE 754 bit patterns
  (func $test_ieee754_patterns (export "test_ieee754_patterns")
    (result f64 f64 f64 f64 f64 f64)
    ;; Test common IEEE 754 bit patterns
    ;; 0x0000000000000000 -> +0.0
    i64.const 0x0000000000000000
    f64.reinterpret_i64

    ;; 0x8000000000000000 -> -0.0
    i64.const 0x8000000000000000
    f64.reinterpret_i64

    ;; 0x3FF0000000000000 -> 1.0
    i64.const 0x3FF0000000000000
    f64.reinterpret_i64

    ;; 0xBFF0000000000000 -> -1.0
    i64.const 0xBFF0000000000000
    f64.reinterpret_i64

    ;; 0x4000000000000000 -> 2.0
    i64.const 0x4000000000000000
    f64.reinterpret_i64

    ;; 0x7FF0000000000000 -> +infinity
    i64.const 0x7FF0000000000000
    f64.reinterpret_i64
  )

  ;; Test with special IEEE 754 values
  (func $test_special_ieee754 (export "test_special_ieee754")
    (result f64 f64 f64 f64)
    ;; 0xFFF0000000000000 -> -infinity
    i64.const 0xFFF0000000000000
    f64.reinterpret_i64

    ;; 0x7FF8000000000000 -> quiet NaN
    i64.const 0x7FF8000000000000
    f64.reinterpret_i64

    ;; 0x7FF0000000000001 -> signaling NaN
    i64.const 0x7FF0000000000001
    f64.reinterpret_i64

    ;; 0x0000000000000001 -> smallest positive subnormal
    i64.const 0x0000000000000001
    f64.reinterpret_i64
  )

  ;; Main entry point for general testing
  (func $main (export "main")
    (result f64)
    ;; Test basic functionality with bit pattern for 1.0
    i64.const 0x3FF0000000000000
    call $test_f64_reinterpret_i64
    drop

    ;; Return 0.0 to indicate success
    f64.const 0.0
  )
)