(module
  ;; f32x4.neg comprehensive test module
  ;; Tests IEEE 754 compliant negation operation on 4-lane f32 SIMD vectors

  ;; Test f32x4.neg with basic mixed values
  ;; Input: [1.0f, -2.5f, 3.14159f, -100.0f]
  ;; Expected result: [-1.0f, 2.5f, -3.14159f, 100.0f]
  (func (export "test_f32x4_neg_basic") (result v128)
    ;; Create v128 with mixed positive/negative values
    v128.const f32x4 1.0 -2.5 3.14159 -100.0
    ;; Apply f32x4.neg
    f32x4.neg
  )

  ;; Test f32x4.neg with IEEE 754 special values
  ;; Input: [+0.0f, -0.0f, +inf, -inf]
  ;; Expected result: [-0.0f, +0.0f, -inf, +inf]
  (func (export "test_f32x4_neg_special") (result v128)
    ;; Create v128 with special IEEE 754 values using bit patterns
    ;; +0.0: 0x00000000, -0.0: 0x80000000, +inf: 0x7F800000, -inf: 0xFF800000
    v128.const i32x4 0x00000000 0x80000000 0x7F800000 0xFF800000
    ;; Apply f32x4.neg
    f32x4.neg
  )

  ;; Test f32x4.neg with NaN values (should preserve payload, flip sign)
  ;; Input: [qNaN, sNaN, custom_payload_NaN, normal_value]
  ;; Expected: sign bit flipped, payload preserved for NaNs
  (func (export "test_f32x4_neg_nan") (result v128)
    ;; Create v128 with various NaN values and one normal value
    ;; Quiet NaN: 0x7FC00000, Signaling NaN: 0x7F800001
    ;; Custom payload NaN: 0x7FC12345, Normal: 42.0f (0x42280000)
    v128.const i32x4 0x7FC00000 0x7F800001 0x7FC12345 0x42280000
    ;; Apply f32x4.neg - should flip sign bit only
    f32x4.neg
  )

  ;; Test f32x4.neg with boundary values
  ;; Input: [FLT_MAX, -FLT_MAX, FLT_MIN, subnormal]
  ;; Expected: sign flipped, mantissa/exponent preserved
  (func (export "test_f32x4_neg_boundary") (result v128)
    ;; Create v128 with boundary values
    ;; FLT_MAX: 0x7F7FFFFF, -FLT_MAX: 0xFF7FFFFF
    ;; FLT_MIN (smallest normalized): 0x00800000
    ;; Smallest positive subnormal: 0x00000001
    v128.const i32x4 0x7F7FFFFF 0xFF7FFFFF 0x00800000 0x00000001
    ;; Apply f32x4.neg
    f32x4.neg
  )

  ;; Test f32x4.neg lane independence
  ;; Input: [normal, infinity, NaN, zero] - different types per lane
  ;; Expected: each lane negated independently
  (func (export "test_f32x4_neg_independence") (result v128)
    ;; Create v128 with different value types in each lane
    ;; Lane 0: Normal value 123.45f (0x42F6E666)
    ;; Lane 1: Positive infinity (0x7F800000)
    ;; Lane 2: Quiet NaN (0x7FC00000)
    ;; Lane 3: Positive zero (0x00000000)
    v128.const i32x4 0x42F6E666 0x7F800000 0x7FC00000 0x00000000
    ;; Apply f32x4.neg - each lane processed independently
    f32x4.neg
  )

  ;; Test f32x4.neg double negation property: -(-x) = x
  ;; Input: [1.0f, -1.0f, 3.14f, -3.14f]
  ;; Expected: original values after double negation
  (func (export "test_f32x4_neg_double") (result v128)
    ;; Create v128 with alternating signs
    v128.const f32x4 1.0 -1.0 3.14159 -3.14159
    ;; Apply double negation
    f32x4.neg
    f32x4.neg
    ;; Result should match original input
  )

  ;; Test f32x4.neg with precision preservation
  ;; Input: values with maximum mantissa precision
  ;; Expected: exact negation without precision loss
  (func (export "test_f32x4_neg_precision") (result v128)
    ;; Create v128 with high-precision values
    ;; Use bit patterns for exact precision control
    ;; High precision positive: 0x3F7FFFFF (0.999999940395355f)
    ;; High precision negative: 0xBF7FFFFF (-0.999999940395355f)
    ;; Max mantissa: 0x407FFFFF (3.9999997615814209f)
    ;; Min mantissa: 0xC07FFFFF (-3.9999997615814209f)
    v128.const i32x4 0x3F7FFFFF 0xBF7FFFFF 0x407FFFFF 0xC07FFFFF
    ;; Apply f32x4.neg - should preserve exact precision
    f32x4.neg
  )
)