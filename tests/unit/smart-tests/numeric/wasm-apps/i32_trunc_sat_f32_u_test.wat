(module
  ;; Test module for i32.trunc_sat_f32_u opcode validation
  ;; Provides comprehensive test function for saturating truncation from f32 to unsigned i32

  ;; Memory section (required for WAMR module loading)
  (memory 1)

  ;; Export memory for host access
  (export "memory" (memory 0))

  ;; Main test function: test_i32_trunc_sat_f32_u
  ;; Input: f32 value on stack
  ;; Output: i32 result after saturating truncation (representing unsigned integer)
  ;; Note: This function exercises the WASM_OP_I32_TRUNC_SAT_U_F32 opcode
  (func $test_i32_trunc_sat_f32_u (param $f f32) (result i32)
    local.get $f
    ;; The actual opcode being tested - converts f32 to unsigned i32 with saturation
    ;; This is a miscellaneous prefix instruction: 0xFC 0x01
    ;; - NaN values saturate to 0
    ;; - Values > UINT32_MAX saturate to UINT32_MAX (4294967295)
    ;; - Values < 0 saturate to 0
    ;; - Normal positive values truncate toward zero (fractional parts discarded)
    i32.trunc_sat_f32_u
  )

  ;; Export the test function for host environment access
  (export "test_i32_trunc_sat_f32_u" (func $test_i32_trunc_sat_f32_u))

  ;; Additional test functions to validate specific boundary conditions

  ;; Test function for positive overflow saturation
  (func $test_positive_overflow (result i32)
    ;; Test with value that definitely exceeds UINT32_MAX
    f32.const 5e9  ;; 5 billion, exceeds UINT32_MAX (4.29 billion)
    i32.trunc_sat_f32_u
  )
  (export "test_positive_overflow" (func $test_positive_overflow))

  ;; Test function for negative value saturation
  (func $test_negative_saturation (result i32)
    ;; Test with negative value that should saturate to 0
    f32.const -100.0
    i32.trunc_sat_f32_u
  )
  (export "test_negative_saturation" (func $test_negative_saturation))

  ;; Test function for NaN handling
  (func $test_nan_saturation (result i32)
    ;; Create NaN value and test saturation to 0
    f32.const nan
    i32.trunc_sat_f32_u
  )
  (export "test_nan_saturation" (func $test_nan_saturation))

  ;; Test function for positive infinity saturation
  (func $test_positive_infinity (result i32)
    f32.const inf
    i32.trunc_sat_f32_u
  )
  (export "test_positive_infinity" (func $test_positive_infinity))

  ;; Test function for negative infinity saturation
  (func $test_negative_infinity (result i32)
    f32.const -inf
    i32.trunc_sat_f32_u
  )
  (export "test_negative_infinity" (func $test_negative_infinity))

  ;; Test function for zero values (positive and negative zero)
  (func $test_zero_values (param $f f32) (result i32)
    local.get $f
    i32.trunc_sat_f32_u
  )
  (export "test_zero_values" (func $test_zero_values))

  ;; Test function for fractional truncation
  (func $test_fractional_truncation (param $f f32) (result i32)
    local.get $f
    i32.trunc_sat_f32_u
  )
  (export "test_fractional_truncation" (func $test_fractional_truncation))

  ;; Test function for boundary values near UINT32 limits
  (func $test_boundary_values (param $f f32) (result i32)
    local.get $f
    i32.trunc_sat_f32_u
  )
  (export "test_boundary_values" (func $test_boundary_values))

  ;; Test function for exact UINT32_MAX boundary
  (func $test_uint32_max_boundary (result i32)
    ;; Test with f32 representation of UINT32_MAX
    f32.const 4294967295.0  ;; Exact UINT32_MAX as float
    i32.trunc_sat_f32_u
  )
  (export "test_uint32_max_boundary" (func $test_uint32_max_boundary))

  ;; Test function for small negative values
  (func $test_small_negative (result i32)
    ;; Test with small negative value (should saturate to 0)
    f32.const -0.1
    i32.trunc_sat_f32_u
  )
  (export "test_small_negative" (func $test_small_negative))

  ;; Test function for very large positive values
  (func $test_very_large_positive (result i32)
    ;; Test with extremely large positive value
    f32.const 1e20  ;; Much larger than UINT32_MAX
    i32.trunc_sat_f32_u
  )
  (export "test_very_large_positive" (func $test_very_large_positive))

  ;; Test function to validate consistent behavior across multiple calls
  (func $test_consistency (param $f f32) (result i32)
    ;; Call the opcode multiple times to ensure consistent results
    local.get $f
    i32.trunc_sat_f32_u
    drop

    local.get $f
    i32.trunc_sat_f32_u
    drop

    ;; Return final result
    local.get $f
    i32.trunc_sat_f32_u
  )
  (export "test_consistency" (func $test_consistency))

  ;; Test function for typical conversion range
  (func $test_typical_range (param $f f32) (result i32)
    ;; Test typical positive values in normal unsigned integer range
    local.get $f
    i32.trunc_sat_f32_u
  )
  (export "test_typical_range" (func $test_typical_range))

  ;; Test function for subnormal numbers
  (func $test_subnormal_handling (result i32)
    ;; Test with very small positive subnormal number
    f32.const 1e-38  ;; Very small positive value
    i32.trunc_sat_f32_u
  )
  (export "test_subnormal_handling" (func $test_subnormal_handling))
)