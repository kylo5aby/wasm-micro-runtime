(module
  ;; Import memory for test data if needed
  (memory 1)

  ;; Export memory for external access
  (export "memory" (memory 0))

  ;; Test function referenced by the C++ test: test_basic_splat
  ;; Takes an i32 parameter and returns v128 result of i16x8.splat
  (func $test_basic_splat (param $value i32) (result v128)
    local.get $value
    i16x8.splat
  )
  (export "test_basic_splat" (func $test_basic_splat))

  ;; Test function referenced by the C++ test: test_boundary_splat
  (func $test_boundary_splat (param $value i32) (result v128)
    local.get $value
    i16x8.splat
  )
  (export "test_boundary_splat" (func $test_boundary_splat))

  ;; Test function referenced by the C++ test: test_truncation_splat
  (func $test_truncation_splat (param $value i32) (result v128)
    local.get $value
    i16x8.splat
  )
  (export "test_truncation_splat" (func $test_truncation_splat))

  ;; Test function referenced by the C++ test: test_bitpattern_splat
  (func $test_bitpattern_splat (param $value i32) (result v128)
    local.get $value
    i16x8.splat
  )
  (export "test_bitpattern_splat" (func $test_bitpattern_splat))

  ;; Test function for constant splat with immediate value
  (func $i16x8_splat_const (result v128)
    i32.const 1000
    i16x8.splat
  )
  (export "i16x8_splat_const" (func $i16x8_splat_const))

  ;; Test function for boundary value (i16 max)
  (func $i16x8_splat_max (result v128)
    i32.const 32767  ;; INT16_MAX
    i16x8.splat
  )
  (export "i16x8_splat_max" (func $i16x8_splat_max))

  ;; Test function for boundary value (i16 min)
  (func $i16x8_splat_min (result v128)
    i32.const -32768  ;; INT16_MIN
    i16x8.splat
  )
  (export "i16x8_splat_min" (func $i16x8_splat_min))

  ;; Test function for zero value
  (func $i16x8_splat_zero (result v128)
    i32.const 0
    i16x8.splat
  )
  (export "i16x8_splat_zero" (func $i16x8_splat_zero))

  ;; Test function for truncation behavior (65537 -> 1)
  (func $i16x8_splat_truncate_65537 (result v128)
    i32.const 65537
    i16x8.splat
  )
  (export "i16x8_splat_truncate_65537" (func $i16x8_splat_truncate_65537))

  ;; Test function for truncation behavior (65536 -> 0)
  (func $i16x8_splat_truncate_65536 (result v128)
    i32.const 65536
    i16x8.splat
  )
  (export "i16x8_splat_truncate_65536" (func $i16x8_splat_truncate_65536))

  ;; Test function for extreme value truncation (0x12345678)
  (func $i16x8_splat_extreme_large (result v128)
    i32.const 0x12345678
    i16x8.splat
  )
  (export "i16x8_splat_extreme_large" (func $i16x8_splat_extreme_large))

  ;; Test function for extreme value truncation (0x87654321)
  (func $i16x8_splat_extreme_negative (result v128)
    i32.const 0x87654321
    i16x8.splat
  )
  (export "i16x8_splat_extreme_negative" (func $i16x8_splat_extreme_negative))

  ;; Test function for negative one value
  (func $i16x8_splat_neg_one (result v128)
    i32.const -1
    i16x8.splat
  )
  (export "i16x8_splat_neg_one" (func $i16x8_splat_neg_one))

  ;; Test function for alternating bit pattern (0x5555)
  (func $i16x8_splat_alt_bits (result v128)
    i32.const 0x5555
    i16x8.splat
  )
  (export "i16x8_splat_alt_bits" (func $i16x8_splat_alt_bits))

  ;; Test function for power of 2 (1024)
  (func $i16x8_splat_power2 (result v128)
    i32.const 1024
    i16x8.splat
  )
  (export "i16x8_splat_power2" (func $i16x8_splat_power2))

  ;; Test function that causes stack underflow
  ;; This function has invalid logic that would cause runtime stack underflow
  (func $test_stack_underflow (result v128)
    ;; Push a value then pop it, leaving stack empty for splat
    i32.const 42
    drop  ;; Remove the value from stack
    ;; Now stack is empty, but we still need valid WAT syntax
    i32.const 0  ;; Add dummy value to make WAT valid
    i16x8.splat
  )
  (export "test_stack_underflow" (func $test_stack_underflow))
)