(module
  ;; f32x4.trunc comprehensive test module
  ;; Tests IEEE 754 truncation operation on 4-lane f32 SIMD vectors

  ;; Test f32x4.trunc with basic mixed values
  ;; Input: [1.5f, -2.7f, 3.14f, -0.9f]
  ;; Expected result: [1.0f, -2.0f, 3.0f, -0.0f]
  (func (export "test_f32x4_trunc_basic") (result v128)
    ;; Create v128 with mixed positive/negative fractional values
    ;; [1.5f, -2.7f, 3.14f, -0.9f]
    v128.const f32x4 1.5 -2.7 3.14 -0.9
    ;; Apply f32x4.trunc
    f32x4.trunc
  )

  ;; Test f32x4.trunc with IEEE 754 special values
  ;; Input: [NaN, +inf, -inf, -0.0f]
  ;; Expected result: [NaN, +inf, -inf, -0.0f]
  (func (export "test_f32x4_trunc_special") (result v128)
    ;; Create v128 with special IEEE 754 values
    ;; NaN: 0x7FC00000, +inf: 0x7F800000, -inf: 0xFF800000, -0.0: 0x80000000
    v128.const i32x4 0x7FC00000 0x7F800000 0xFF800000 0x80000000
    ;; Apply f32x4.trunc
    f32x4.trunc
  )

  ;; Test f32x4.trunc with boundary values
  ;; Input: [FLT_MAX, -FLT_MAX, 0.999999f, -0.999999f]
  ;; Expected: [FLT_MAX, -FLT_MAX, 0.0f, -0.0f]
  (func (export "test_f32x4_trunc_boundary") (result v128)
    ;; Create v128 with boundary values
    ;; FLT_MAX ≈ 3.40282347e+38f, -FLT_MAX ≈ -3.40282347e+38f
    v128.const f32x4 3.40282347e+38 -3.40282347e+38 0.999999 -0.999999
    ;; Apply f32x4.trunc
    f32x4.trunc
  )

  ;; Test f32x4.trunc with zero values (positive and negative)
  ;; Input: [0.0f, -0.0f, 0.1f, -0.1f]
  ;; Expected: [0.0f, -0.0f, 0.0f, -0.0f]
  (func (export "test_f32x4_trunc_zeros") (result v128)
    ;; Create v128 with zero and near-zero values
    v128.const f32x4 0.0 -0.0 0.1 -0.1
    ;; Apply f32x4.trunc
    f32x4.trunc
  )

  ;; Test f32x4.trunc with perfect integer values
  ;; Input: [1.0f, -5.0f, 100.0f, -1000.0f]
  ;; Expected: [1.0f, -5.0f, 100.0f, -1000.0f] (unchanged)
  (func (export "test_f32x4_trunc_integers") (result v128)
    ;; Create v128 with integer values
    v128.const f32x4 1.0 -5.0 100.0 -1000.0
    ;; Apply f32x4.trunc
    f32x4.trunc
  )

  ;; Test f32x4.trunc with mathematical properties
  ;; Input: [2.7f, -3.8f, 0.5f, -0.3f]
  ;; Expected: [2.0f, -3.0f, 0.0f, -0.0f]
  (func (export "test_f32x4_trunc_properties") (result v128)
    ;; Create v128 for mathematical property testing
    v128.const f32x4 2.7 -3.8 0.5 -0.3
    ;; Apply f32x4.trunc
    f32x4.trunc
  )

  ;; Test f32x4.trunc with precision boundary values
  ;; Input: [16777216.5f, -16777216.5f, 8388607.9f, -8388607.1f]
  ;; Expected: Values near f32 precision limits
  (func (export "test_f32x4_trunc_precision") (result v128)
    ;; Create v128 with precision boundary values
    ;; 2^24 + 0.5, -2^24 - 0.5, 2^23 - 0.1, -2^23 + 0.9
    v128.const f32x4 16777216.5 -16777216.5 8388607.9 -8388607.1
    ;; Apply f32x4.trunc
    f32x4.trunc
  )

  ;; Test f32x4.trunc with edge cases
  ;; Input: [1e-38f, -1e-38f, 1.0000001f, -1.0000001f]
  ;; Expected: [0.0f, -0.0f, 1.0f, -1.0f]
  (func (export "test_f32x4_trunc_edge") (result v128)
    ;; Create v128 with edge case values
    v128.const f32x4 1e-38 -1e-38 1.0000001 -1.0000001
    ;; Apply f32x4.trunc
    f32x4.trunc
  )

  ;; Test f32x4.trunc comprehensive mixed scenario
  ;; Input: [NaN, 2.5f, +inf, -0.1f]
  ;; Expected: [NaN, 2.0f, +inf, -0.0f]
  (func (export "test_f32x4_trunc_mixed") (result v128)
    ;; Create v128 with mixed special/normal values
    ;; Use i32x4 const for exact NaN representation
    v128.const i32x4 0x7FC00000 0x40200000 0x7F800000 0xBDCCCCCD
    ;; Apply f32x4.trunc
    f32x4.trunc
  )
)