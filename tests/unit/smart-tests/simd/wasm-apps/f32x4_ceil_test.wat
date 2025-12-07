(module
  ;; f32x4.ceil comprehensive test module
  ;; Tests IEEE 754 ceiling operation on 4-lane f32 SIMD vectors

  ;; Test f32x4.ceil with basic mixed values
  ;; Input: [1.2f, -2.7f, 3.0f, -4.9f]
  ;; Expected result: [2.0f, -2.0f, 3.0f, -4.0f]
  (func (export "test_f32x4_ceil_basic") (result v128)
    ;; Create v128 with mixed positive/negative/integer values
    ;; [1.2f, -2.7f, 3.0f, -4.9f]
    v128.const f32x4 1.2 -2.7 3.0 -4.9
    ;; Apply f32x4.ceil
    f32x4.ceil
  )

  ;; Test f32x4.ceil with IEEE 754 special values
  ;; Input: [NaN, +inf, -inf, +0.0f]
  ;; Expected result: [NaN, +inf, -inf, +0.0f]
  (func (export "test_f32x4_ceil_special") (result v128)
    ;; Create v128 with special IEEE 754 values
    ;; NaN: 0x7FC00000, +inf: 0x7F800000, -inf: 0xFF800000, +0.0: 0x00000000
    v128.const i32x4 0x7FC00000 0x7F800000 0xFF800000 0x00000000
    ;; Apply f32x4.ceil
    f32x4.ceil
  )

  ;; Test f32x4.ceil with boundary values
  ;; Input: [FLT_MAX, -FLT_MAX, smallest_positive_subnormal, smallest_negative_subnormal]
  ;; Expected: [FLT_MAX, -FLT_MAX, 1.0f, -0.0f]
  (func (export "test_f32x4_ceil_boundary") (result v128)
    ;; Create v128 with boundary values
    ;; FLT_MAX: 0x7F7FFFFF, -FLT_MAX: 0xFF7FFFFF
    ;; Smallest positive subnormal: 0x00000001 (1.4e-45f)
    ;; Smallest negative subnormal: 0x80000001 (-1.4e-45f)
    v128.const i32x4 0x7F7FFFFF 0xFF7FFFFF 0x00000001 0x80000001
    ;; Apply f32x4.ceil
    f32x4.ceil
  )

  ;; Test f32x4.ceil with signed zero values
  ;; Input: [+0.0f, -0.0f, +0.0f, -0.0f]
  ;; Expected result: [+0.0f, -0.0f, +0.0f, -0.0f]
  (func (export "test_f32x4_ceil_zeros") (result v128)
    ;; Create v128 with signed zeros
    ;; +0.0: 0x00000000, -0.0: 0x80000000
    v128.const i32x4 0x00000000 0x80000000 0x00000000 0x80000000
    ;; Apply f32x4.ceil
    f32x4.ceil
  )

  ;; Test f32x4.ceil idempotent property (first call)
  ;; Input: [1.5f, -2.3f, 3.7f, -4.1f]
  ;; Expected result: [2.0f, -2.0f, 4.0f, -4.0f]
  (func (export "test_f32x4_ceil_idempotent") (result v128)
    ;; Create v128 with fractional values for idempotent test
    v128.const f32x4 1.5 -2.3 3.7 -4.1
    ;; Apply f32x4.ceil
    f32x4.ceil
  )

  ;; Test f32x4.ceil idempotent property (second call - ceiling of ceiling)
  ;; Input: ceiling results [2.0f, -2.0f, 4.0f, -4.0f]
  ;; Expected result: [2.0f, -2.0f, 4.0f, -4.0f] (same as first call)
  (func (export "test_f32x4_ceil_idempotent_double") (result v128)
    ;; Create v128 with already ceiling-ed values (should be unchanged)
    v128.const f32x4 2.0 -2.0 4.0 -4.0
    ;; Apply f32x4.ceil again (idempotent test)
    f32x4.ceil
  )

  ;; Additional test functions for comprehensive coverage

  ;; Test with small fractional values near zero
  ;; Input: [0.1f, -0.1f, 0.9f, -0.9f]
  ;; Expected result: [1.0f, -0.0f, 1.0f, -0.0f]
  (func (export "test_f32x4_ceil_small_fractions") (result v128)
    v128.const f32x4 0.1 -0.1 0.9 -0.9
    f32x4.ceil
  )

  ;; Test with large integer values (should remain unchanged)
  ;; Input: [1000000.0f, -1000000.0f, 16777216.0f, -16777216.0f]
  ;; Expected result: [1000000.0f, -1000000.0f, 16777216.0f, -16777216.0f]
  (func (export "test_f32x4_ceil_large_integers") (result v128)
    v128.const f32x4 1000000.0 -1000000.0 16777216.0 -16777216.0
    f32x4.ceil
  )

  ;; Test with values near the precision limit of f32
  ;; Input: [16777216.5f, 16777217.5f, -16777216.5f, -16777217.5f]
  ;; Expected: Ceiling behavior at precision boundary
  (func (export "test_f32x4_ceil_precision_limit") (result v128)
    v128.const f32x4 16777216.5 16777217.5 -16777216.5 -16777217.5
    f32x4.ceil
  )

  ;; Test with mixed extreme values
  ;; Input: [0.0000001f, -0.0000001f, 999999.9f, -999999.9f]
  ;; Expected result: [1.0f, -0.0f, 1000000.0f, -999999.0f]
  (func (export "test_f32x4_ceil_mixed_extreme") (result v128)
    v128.const f32x4 0.0000001 -0.0000001 999999.9 -999999.9
    f32x4.ceil
  )

  ;; Test mathematical property: ceil(x) >= x for all finite x
  ;; This test provides input values that should demonstrate the ceiling property
  ;; Input: [1.1f, 2.9f, -1.1f, -2.9f]
  ;; Expected result: [2.0f, 3.0f, -1.0f, -2.0f]
  (func (export "test_f32x4_ceil_property") (result v128)
    v128.const f32x4 1.1 2.9 -1.1 -2.9
    f32x4.ceil
  )

  ;; Test with denormalized (subnormal) numbers
  ;; Input: Various subnormal values that should ceiling to 0.0 or 1.0
  (func (export "test_f32x4_ceil_subnormal") (result v128)
    ;; Use bit patterns for subnormal numbers
    ;; Positive subnormal: 0x00000010, Negative subnormal: 0x80000010
    ;; Larger positive subnormal: 0x007FFFFF, Larger negative subnormal: 0x807FFFFF
    v128.const i32x4 0x00000010 0x80000010 0x007FFFFF 0x807FFFFF
    f32x4.ceil
  )
)