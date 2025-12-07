(module
  ;; Test module for i32x4.trunc_sat_f64x2_u_zero opcode validation
  ;; This module provides comprehensive test functions for saturating truncation
  ;; from f64x2 to i32x4 (unsigned) with zero padding in upper lanes

  ;; Import memory for storing and loading v128 data
  (memory (export "memory") 1)

  ;; Main test function: i32x4.trunc_sat_f64x2_u_zero
  ;; @param f64 First f64 value for lane 0
  ;; @param f64 Second f64 value for lane 1
  ;; @return v128 i32x4 result with converted values in lanes 0,1 and zeros in lanes 2,3
  (func (export "test_i32x4_trunc_sat_f64x2_u_zero") (param $f64_0 f64) (param $f64_1 f64) (result v128)
    ;; Store f64 values in memory to create f64x2 vector
    (i32.const 0)       ;; Memory offset for first f64
    local.get $f64_0
    f64.store

    (i32.const 8)       ;; Memory offset for second f64
    local.get $f64_1
    f64.store

    ;; Load as v128 f64x2 vector and apply unsigned truncation
    (i32.const 0)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero

    ;; Return the resulting i32x4 vector
  )

  ;; Alternative test function for specific edge case scenarios
  ;; Tests with predefined f64x2 constants for consistent unsigned edge case validation
  (func (export "test_trunc_sat_constants") (result v128)
    ;; Store constant values: 42.7 and 100.99 (positive values only for unsigned)
    (i32.const 16)
    f64.const 42.7
    f64.store

    (i32.const 24)
    f64.const 100.99
    f64.store

    ;; Load and apply unsigned truncation opcode
    (i32.const 16)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero
  )

  ;; Test function for boundary saturation validation
  ;; Tests UINT32_MAX boundary conditions and negative saturation to zero
  (func (export "test_saturation_boundaries") (result v128)
    ;; Store boundary values: 4294967296.0 and -1000.5
    (i32.const 32)
    f64.const 4294967296.0    ;; Over UINT32_MAX, should saturate to UINT32_MAX
    f64.store

    (i32.const 40)
    f64.const -1000.5         ;; Negative value, should saturate to 0
    f64.store

    ;; Load and apply unsigned truncation opcode
    (i32.const 32)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero
  )

  ;; Test function for special floating-point values
  ;; Tests NaN and positive infinity handling for unsigned conversion
  (func (export "test_special_values_nan_inf") (result v128)
    ;; Store NaN and positive infinity
    (i32.const 48)
    f64.const nan           ;; NaN should convert to 0
    f64.store

    (i32.const 56)
    f64.const inf           ;; Positive infinity should saturate to UINT32_MAX
    f64.store

    ;; Load and apply unsigned truncation opcode
    (i32.const 48)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero
  )

  ;; Test function for zero values (positive and negative)
  (func (export "test_zero_values") (result v128)
    ;; Store positive and negative zero
    (i32.const 64)
    f64.const 0.0          ;; Positive zero
    f64.store

    (i32.const 72)
    f64.const -0.0         ;; Negative zero
    f64.store

    ;; Load and apply unsigned truncation opcode
    (i32.const 64)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero
  )

  ;; Test function for negative infinity and negative values
  (func (export "test_negative_values") (result v128)
    ;; Store negative infinity and large negative value
    (i32.const 80)
    f64.const -inf         ;; Negative infinity should saturate to 0
    f64.store

    (i32.const 88)
    f64.const -1e20        ;; Large negative should also saturate to 0
    f64.store

    ;; Load and apply unsigned truncation opcode
    (i32.const 80)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero
  )

  ;; Test function for very small values (subnormal behavior)
  (func (export "test_small_values") (result v128)
    ;; Store very small values that should truncate to zero
    (i32.const 96)
    f64.const 1e-100       ;; Very small positive
    f64.store

    (i32.const 104)
    f64.const -1e-100      ;; Very small negative (should saturate to 0)
    f64.store

    ;; Load and apply unsigned truncation opcode
    (i32.const 96)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero
  )

  ;; Test function for exact boundary values
  (func (export "test_exact_boundaries") (result v128)
    ;; Store exact UINT32_MAX and test zero boundary
    (i32.const 112)
    f64.const 4294967295.0    ;; Exact UINT32_MAX
    f64.store

    (i32.const 120)
    f64.const -0.1            ;; Small negative, should saturate to 0
    f64.store

    ;; Load and apply unsigned truncation opcode
    (i32.const 112)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero
  )

  ;; Test function for large positive values that exceed UINT32_MAX
  (func (export "test_large_positive_values") (result v128)
    ;; Store very large positive values
    (i32.const 128)
    f64.const 1e15            ;; Very large positive
    f64.store

    (i32.const 136)
    f64.const 1e20            ;; Extremely large positive
    f64.store

    ;; Load and apply unsigned truncation opcode
    (i32.const 128)
    v128.load
    i32x4.trunc_sat_f64x2_u_zero
  )

  ;; Utility functions to extract specific i32 lanes from v128 result
  ;; Returns lane 0 (first converted value)
  (func (export "extract_lane_0") (param $vec v128) (result i32)
    local.get $vec
    i32x4.extract_lane 0
  )

  ;; Returns lane 1 (second converted value)
  (func (export "extract_lane_1") (param $vec v128) (result i32)
    local.get $vec
    i32x4.extract_lane 1
  )

  ;; Returns lane 2 (should be zero)
  (func (export "extract_lane_2") (param $vec v128) (result i32)
    local.get $vec
    i32x4.extract_lane 2
  )

  ;; Returns lane 3 (should be zero)
  (func (export "extract_lane_3") (param $vec v128) (result i32)
    local.get $vec
    i32x4.extract_lane 3
  )
)