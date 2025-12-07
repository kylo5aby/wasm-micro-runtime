(module
  ;; f32x4.floor comprehensive test module
  ;; Tests IEEE 754 floor operation on 4-lane f32 SIMD vectors

  ;; Test f32x4.floor with basic mixed values
  ;; Input: [1.2f, -2.7f, 3.0f, -4.9f]
  ;; Expected result: [1.0f, -3.0f, 3.0f, -5.0f]
  (func (export "test_f32x4_floor_basic") (result v128)
    ;; Create v128 with mixed positive/negative/integer values
    ;; [1.2f, -2.7f, 3.0f, -4.9f]
    v128.const f32x4 1.2 -2.7 3.0 -4.9
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test f32x4.floor with IEEE 754 special values
  ;; Input: [NaN, +inf, -inf, +0.0f]
  ;; Expected result: [NaN, +inf, -inf, +0.0f]
  (func (export "test_f32x4_floor_special") (result v128)
    ;; Create v128 with special IEEE 754 values
    ;; NaN: 0x7FC00000, +inf: 0x7F800000, -inf: 0xFF800000, +0.0: 0x00000000
    v128.const i32x4 0x7FC00000 0x7F800000 0xFF800000 0x00000000
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test f32x4.floor with boundary values
  ;; Input: [FLT_MAX, -FLT_MAX, smallest_positive_subnormal, smallest_negative_subnormal]
  ;; Expected: [FLT_MAX, -FLT_MAX, 0.0f, -1.0f]
  (func (export "test_f32x4_floor_boundary") (result v128)
    ;; Create v128 with boundary values
    ;; FLT_MAX: 0x7F7FFFFF, -FLT_MAX: 0xFF7FFFFF
    ;; Smallest positive subnormal: 0x00000001 (1.4e-45f)
    ;; Smallest negative subnormal: 0x80000001 (-1.4e-45f)
    v128.const i32x4 0x7F7FFFFF 0xFF7FFFFF 0x00000001 0x80000001
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test f32x4.floor idempotent property (first call)
  ;; Input: [1.5f, -2.3f, 3.7f, -4.1f]
  ;; Expected result: [1.0f, -3.0f, 3.0f, -5.0f]
  (func (export "test_f32x4_floor_idempotent") (result v128)
    ;; Create v128 with fractional values for idempotent test
    v128.const f32x4 1.5 -2.3 3.7 -4.1
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test f32x4.floor idempotent property (second call - floor of floor)
  ;; Input: floor results [1.0f, -3.0f, 3.0f, -5.0f]
  ;; Expected result: [1.0f, -3.0f, 3.0f, -5.0f] (same as first call)
  (func (export "test_f32x4_floor_idempotent_double") (result v128)
    ;; Create v128 with already floor-ed values (should be unchanged)
    v128.const f32x4 1.0 -3.0 3.0 -5.0
    ;; Apply f32x4.floor again (idempotent test)
    f32x4.floor
  )

  ;; Test f32x4.floor with precision boundary values
  ;; Input: [16777216.0f, 16777215.5f, -16777216.0f, -16777215.5f]
  ;; Expected: [16777216.0f, 16777215.0f, -16777216.0f, -16777216.0f]
  (func (export "test_f32x4_floor_precision") (result v128)
    ;; Create v128 with values at f32 precision boundaries
    ;; Large integer (exactly representable): 16777216.0f
    ;; Fractional near precision limit: 16777215.5f
    v128.const f32x4 16777216.0 16777215.5 -16777216.0 -16777215.5
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test f32x4.floor with small fractional values near zero
  ;; Input: [0.1f, -0.1f, 0.9f, -0.9f]
  ;; Expected result: [0.0f, -1.0f, 0.0f, -1.0f]
  (func (export "test_f32x4_floor_small") (result v128)
    ;; Create v128 with small fractional values
    v128.const f32x4 0.1 -0.1 0.9 -0.9
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test mathematical property: floor(x) <= x for all finite x
  ;; This test provides input values that should demonstrate the floor property
  ;; Input: [1.1f, 2.9f, -1.1f, -2.9f]
  ;; Expected result: [1.0f, 2.0f, -2.0f, -3.0f]
  (func (export "test_f32x4_floor_property") (result v128)
    ;; Create v128 with values to validate floor(x) <= x property
    v128.const f32x4 1.1 2.9 -1.1 -2.9
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Additional test functions for comprehensive coverage

  ;; Test with signed zero values
  ;; Input: [+0.0f, -0.0f, +0.0f, -0.0f]
  ;; Expected result: [+0.0f, -0.0f, +0.0f, -0.0f]
  (func (export "test_f32x4_floor_zeros") (result v128)
    ;; Create v128 with signed zeros
    ;; +0.0: 0x00000000, -0.0: 0x80000000
    v128.const i32x4 0x00000000 0x80000000 0x00000000 0x80000000
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test with large integer values (should remain unchanged)
  ;; Input: [1000000.0f, -1000000.0f, 16777216.0f, -16777216.0f]
  ;; Expected result: [1000000.0f, -1000000.0f, 16777216.0f, -16777216.0f]
  (func (export "test_f32x4_floor_large_integers") (result v128)
    ;; Create v128 with large integer values
    v128.const f32x4 1000000.0 -1000000.0 16777216.0 -16777216.0
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test with mixed extreme values
  ;; Input: [0.0000001f, -0.0000001f, 999999.9f, -999999.9f]
  ;; Expected result: [0.0f, -1.0f, 999999.0f, -1000000.0f]
  (func (export "test_f32x4_floor_mixed_extreme") (result v128)
    ;; Create v128 with mixed extreme values
    v128.const f32x4 0.0000001 -0.0000001 999999.9 -999999.9
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test with denormalized (subnormal) numbers
  ;; Input: Various subnormal values that should floor to 0.0 or -1.0
  (func (export "test_f32x4_floor_subnormal") (result v128)
    ;; Use bit patterns for subnormal numbers
    ;; Positive subnormal: 0x00000010, Negative subnormal: 0x80000010
    ;; Larger positive subnormal: 0x007FFFFF, Larger negative subnormal: 0x807FFFFF
    v128.const i32x4 0x00000010 0x80000010 0x007FFFFF 0x807FFFFF
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test with half values (x.5)
  ;; Input: [1.5f, -1.5f, 2.5f, -2.5f]
  ;; Expected result: [1.0f, -2.0f, 2.0f, -3.0f]
  (func (export "test_f32x4_floor_half_values") (result v128)
    ;; Create v128 with half values to test floor behavior
    v128.const f32x4 1.5 -1.5 2.5 -2.5
    ;; Apply f32x4.floor
    f32x4.floor
  )

  ;; Test edge case: values very close to integers
  ;; Input: [0.9999999f, 1.0000001f, -0.9999999f, -1.0000001f]
  ;; Expected result: [0.0f, 1.0f, -1.0f, -2.0f]
  (func (export "test_f32x4_floor_near_integers") (result v128)
    ;; Create v128 with values very close to integers
    v128.const f32x4 0.9999999 1.0000001 -0.9999999 -1.0000001
    ;; Apply f32x4.floor
    f32x4.floor
  )
)