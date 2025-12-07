(module
  ;; f32x4.nearest comprehensive test module
  ;; Tests IEEE 754 "round half to even" (banker's rounding) operation on 4-lane f32 SIMD vectors

  ;; Test f32x4.nearest with parameters and individual results
  ;; Input: four f32 parameters
  ;; Expected result: four f32 results after nearest rounding
  (func (export "test_f32x4_nearest") (param f32 f32 f32 f32) (result f32 f32 f32 f32)
    (local $vec v128)
    ;; Build v128 from individual f32 parameters
    v128.const f32x4 0.0 0.0 0.0 0.0
    local.get 0
    f32x4.replace_lane 0
    local.get 1
    f32x4.replace_lane 1
    local.get 2
    f32x4.replace_lane 2
    local.get 3
    f32x4.replace_lane 3
    ;; Apply f32x4.nearest
    f32x4.nearest
    ;; Store result in local variable
    local.tee $vec
    ;; Extract lanes to return individual f32 values
    f32x4.extract_lane 0
    local.get $vec
    f32x4.extract_lane 1
    local.get $vec
    f32x4.extract_lane 2
    local.get $vec
    f32x4.extract_lane 3
  )

  ;; Test f32x4.nearest with basic rounding cases
  ;; Input: [1.2f, 2.7f, -3.1f, -4.9f]
  ;; Expected result: [1.0f, 3.0f, -3.0f, -5.0f]
  (func (export "test_f32x4_nearest_basic") (result v128)
    ;; Create v128 with mixed positive/negative fractional values
    v128.const f32x4 1.2 2.7 -3.1 -4.9
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with banker's rounding (round half to even)
  ;; Input: [2.5f, 3.5f, -2.5f, -3.5f]
  ;; Expected result: [2.0f, 4.0f, -2.0f, -4.0f] (round half to even)
  (func (export "test_f32x4_nearest_bankers") (result v128)
    ;; Create v128 with half values for banker's rounding test
    v128.const f32x4 2.5 3.5 -2.5 -3.5
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with additional banker's rounding cases
  ;; Input: [0.5f, 1.5f, -0.5f, -1.5f]
  ;; Expected result: [0.0f, 2.0f, -0.0f, -2.0f] (round half to even)
  (func (export "test_f32x4_nearest_bankers_extended") (result v128)
    ;; Create v128 with additional half values for comprehensive banker's rounding test
    v128.const f32x4 0.5 1.5 -0.5 -1.5
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with higher magnitude banker's rounding
  ;; Input: [10.5f, 11.5f, -10.5f, -11.5f]
  ;; Expected result: [10.0f, 12.0f, -10.0f, -12.0f] (round half to even)
  (func (export "test_f32x4_nearest_bankers_large") (result v128)
    ;; Create v128 with larger magnitude half values
    v128.const f32x4 10.5 11.5 -10.5 -11.5
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with small fractional values near zero
  ;; Input: [0.1f, 0.4f, 0.6f, 0.9f]
  ;; Expected result: [0.0f, 0.0f, 1.0f, 1.0f]
  (func (export "test_f32x4_nearest_small_fractions") (result v128)
    ;; Create v128 with small fractional values
    v128.const f32x4 0.1 0.4 0.6 0.9
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with already integer values (identity operation)
  ;; Input: [1.0f, -5.0f, 42.0f, -17.0f]
  ;; Expected result: [1.0f, -5.0f, 42.0f, -17.0f] (unchanged)
  (func (export "test_f32x4_nearest_integers") (result v128)
    ;; Create v128 with integer values (should remain unchanged)
    v128.const f32x4 1.0 -5.0 42.0 -17.0
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with IEEE 754 special values
  ;; Input: [NaN, +inf, -inf, NaN]
  ;; Expected result: [NaN, +inf, -inf, NaN] (preserved unchanged)
  (func (export "test_f32x4_nearest_special") (result v128)
    ;; Create v128 with special IEEE 754 values
    ;; NaN: 0x7FC00000, +inf: 0x7F800000, -inf: 0xFF800000, NaN: 0x7FC00000
    v128.const i32x4 0x7FC00000 0x7F800000 0xFF800000 0x7FC00000
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with signed zero values
  ;; Input: [+0.0f, -0.0f, +0.0f, -0.0f]
  ;; Expected result: [+0.0f, -0.0f, +0.0f, -0.0f] (sign preserved)
  (func (export "test_f32x4_nearest_zeros") (result v128)
    ;; Create v128 with signed zeros
    ;; +0.0: 0x00000000, -0.0: 0x80000000
    v128.const i32x4 0x00000000 0x80000000 0x00000000 0x80000000
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with mixed special and normal values
  ;; Input: [1.5f, NaN, -inf, 2.5f]
  ;; Expected result: [2.0f, NaN, -inf, 2.0f]
  (func (export "test_f32x4_nearest_mixed_special") (result v128)
    ;; Create v128 mixing normal and special values
    v128.const f32x4 0.0 0.0 0.0 0.0
    f32.const 1.5
    f32x4.replace_lane 0  ;; Replace lane 0 with 1.5f
    ;; Convert i32 to f32 for NaN insertion
    i32.const 0x7FC00000
    f32.reinterpret_i32
    f32x4.replace_lane 1  ;; Replace lane 1 with NaN
    ;; Convert i32 to f32 for -inf insertion
    i32.const 0xFF800000
    f32.reinterpret_i32
    f32x4.replace_lane 2  ;; Replace lane 2 with -inf
    f32.const 2.5
    f32x4.replace_lane 3  ;; Replace lane 3 with 2.5f
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest at f32 precision boundary (2^23)
  ;; Input: [8388608.0f, 8388608.5f, 8388609.0f, 16777216.0f]
  ;; Expected result: [8388608.0f, 8388608.0f, 8388609.0f, 16777216.0f]
  (func (export "test_f32x4_nearest_precision_boundary") (result v128)
    ;; Create v128 with values at f32 precision boundary
    ;; 8388608.0f = 2^23 (exact), 8388608.5f should round down to even
    ;; 8388609.0f = 2^23 + 1 (exact), 16777216.0f = 2^24 (exact)
    v128.const f32x4 8388608.0 8388608.5 8388609.0 16777216.0
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with large integer values
  ;; Input: [1e10f, -1e10f, 1e20f, -1e20f]
  ;; Expected result: [1e10f, -1e10f, 1e20f, -1e20f] (unchanged - already integers)
  (func (export "test_f32x4_nearest_large_integers") (result v128)
    ;; Create v128 with large integer values
    v128.const f32x4 1e10 -1e10 1e20 -1e20
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest with very small values (subnormal handling)
  ;; Input: [1e-10f, -1e-10f, 1e-20f, -1e-20f]
  ;; Expected result: [0.0f, -0.0f, 0.0f, -0.0f] (round to zero with sign preservation)
  (func (export "test_f32x4_nearest_subnormal") (result v128)
    ;; Create v128 with very small values that should round to zero
    v128.const f32x4 1e-10 -1e-10 1e-20 -1e-20
    ;; Apply f32x4.nearest
    f32x4.nearest
  )

  ;; Test f32x4.nearest boundary values near FLT_MAX
  ;; Input: [FLT_MAX, -FLT_MAX, largest_integer_f32, -largest_integer_f32]
  ;; Expected: [FLT_MAX, -FLT_MAX, largest_integer_f32, -largest_integer_f32] (unchanged)
  (func (export "test_f32x4_nearest_boundary") (result v128)
    ;; Create v128 with boundary values
    ;; FLT_MAX: 0x7F7FFFFF, -FLT_MAX: 0xFF7FFFFF
    ;; Largest representable integer in f32: 16777216.0f = 2^24
    v128.const i32x4 0x7F7FFFFF 0xFF7FFFFF 0x4B800000 0xCB800000
    ;; Apply f32x4.nearest
    f32x4.nearest
  )
)