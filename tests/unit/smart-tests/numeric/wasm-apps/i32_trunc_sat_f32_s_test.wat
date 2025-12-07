(module
  ;; Test module for i32.trunc_sat_f32_s opcode validation
  ;; Provides comprehensive test function for saturating truncation from f32 to signed i32

  ;; Memory section (required for WAMR module loading)
  (memory 1)

  ;; Export memory for host access
  (export "memory" (memory 0))

  ;; Main test function: test_i32_trunc_sat_f32_s
  ;; Input: f32 value on stack
  ;; Output: i32 result after saturating truncation
  ;; Note: This function exercises the WASM_OP_I32_TRUNC_SAT_S_F32 opcode
  (func $test_i32_trunc_sat_f32_s (param $f f32) (result i32)
    local.get $f
    ;; The actual opcode being tested - converts f32 to signed i32 with saturation
    ;; This is a miscellaneous prefix instruction: 0xFC 0x00
    ;; - NaN values saturate to 0
    ;; - Values > INT32_MAX saturate to INT32_MAX (2147483647)
    ;; - Values < INT32_MIN saturate to INT32_MIN (-2147483648)
    ;; - Normal values truncate toward zero (fractional parts discarded)
    i32.trunc_sat_f32_s
  )

  ;; Export the test function for host environment access
  (export "test_i32_trunc_sat_f32_s" (func $test_i32_trunc_sat_f32_s))

  ;; Additional test functions to validate specific boundary conditions

  ;; Test function for positive overflow saturation
  (func $test_positive_overflow (result i32)
    ;; Test with value that definitely exceeds INT32_MAX
    f32.const 3e9  ;; 3 billion, exceeds INT32_MAX (2.147 billion)
    i32.trunc_sat_f32_s
  )
  (export "test_positive_overflow" (func $test_positive_overflow))

  ;; Test function for negative underflow saturation
  (func $test_negative_underflow (result i32)
    ;; Test with value that definitely exceeds INT32_MIN
    f32.const -3e9  ;; -3 billion, exceeds INT32_MIN magnitude
    i32.trunc_sat_f32_s
  )
  (export "test_negative_underflow" (func $test_negative_underflow))

  ;; Test function for NaN handling
  (func $test_nan_saturation (result i32)
    ;; Create NaN value and test saturation to 0
    f32.const nan
    i32.trunc_sat_f32_s
  )
  (export "test_nan_saturation" (func $test_nan_saturation))

  ;; Test function for positive infinity saturation
  (func $test_positive_infinity (result i32)
    f32.const inf
    i32.trunc_sat_f32_s
  )
  (export "test_positive_infinity" (func $test_positive_infinity))

  ;; Test function for negative infinity saturation
  (func $test_negative_infinity (result i32)
    f32.const -inf
    i32.trunc_sat_f32_s
  )
  (export "test_negative_infinity" (func $test_negative_infinity))

  ;; Test function for zero values
  (func $test_zero_values (param $f f32) (result i32)
    local.get $f
    i32.trunc_sat_f32_s
  )
  (export "test_zero_values" (func $test_zero_values))

  ;; Test function for fractional truncation
  (func $test_fractional_truncation (param $f f32) (result i32)
    local.get $f
    i32.trunc_sat_f32_s
  )
  (export "test_fractional_truncation" (func $test_fractional_truncation))

  ;; Test function for boundary values near INT32 limits
  (func $test_boundary_values (param $f f32) (result i32)
    local.get $f
    i32.trunc_sat_f32_s
  )
  (export "test_boundary_values" (func $test_boundary_values))

  ;; Test function to validate consistent behavior across multiple calls
  (func $test_consistency (param $f f32) (result i32)
    ;; Call the opcode multiple times to ensure consistent results
    local.get $f
    i32.trunc_sat_f32_s
    drop

    local.get $f
    i32.trunc_sat_f32_s
    drop

    ;; Return final result
    local.get $f
    i32.trunc_sat_f32_s
  )
  (export "test_consistency" (func $test_consistency))
)