(module
  ;; Import memory for SIMD operations
  (memory 1)

  ;; Test function: Basic f32x4.ne operation
  ;; Parameters: Two v128 vectors (8 i32 values representing 4 f32 values each)
  ;; Returns: v128 result of element-wise not-equal comparison
  (func (export "f32x4_ne_basic")
    (param $v1_lane0 i32) (param $v1_lane1 i32) (param $v1_lane2 i32) (param $v1_lane3 i32)
    (param $v2_lane0 i32) (param $v2_lane1 i32) (param $v2_lane2 i32) (param $v2_lane3 i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Create first v128 vector from 4 f32 lanes
    (v128.const i32x4 0 0 0 0)
    (local.get $v1_lane0)
    (i32x4.replace_lane 0)
    (local.get $v1_lane1)
    (i32x4.replace_lane 1)
    (local.get $v1_lane2)
    (i32x4.replace_lane 2)
    (local.get $v1_lane3)
    (i32x4.replace_lane 3)

    ;; Create second v128 vector from 4 f32 lanes
    (v128.const i32x4 0 0 0 0)
    (local.get $v2_lane0)
    (i32x4.replace_lane 0)
    (local.get $v2_lane1)
    (i32x4.replace_lane 1)
    (local.get $v2_lane2)
    (i32x4.replace_lane 2)
    (local.get $v2_lane3)
    (i32x4.replace_lane 3)

    ;; Perform f32x4.ne operation
    (f32x4.ne)
    (local.set $result)

    ;; Extract result lanes as i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Test function: f32x4.ne with immediate values
  ;; Tests specific combinations of equal and not-equal values
  (func (export "f32x4_ne_immediate_test") (result i32 i32 i32 i32)
    (local $result v128)
    ;; Create first vector: [1.0, 2.0, 3.0, 4.0]
    (v128.const f32x4 1.0 2.0 3.0 4.0)

    ;; Create second vector: [1.0, 9.0, 3.0, 10.0]
    (v128.const f32x4 1.0 9.0 3.0 10.0)

    ;; Perform f32x4.ne - should return [false, true, false, true]
    (f32x4.ne)
    (local.set $result)

    ;; Extract result lanes
    (local.get $result)
    (i32x4.extract_lane 0)  ;; Should be 0x00000000 (1.0 == 1.0)
    (local.get $result)
    (i32x4.extract_lane 1)  ;; Should be 0xFFFFFFFF (2.0 != 9.0)
    (local.get $result)
    (i32x4.extract_lane 2)  ;; Should be 0x00000000 (3.0 == 3.0)
    (local.get $result)
    (i32x4.extract_lane 3)  ;; Should be 0xFFFFFFFF (4.0 != 10.0)
  )

  ;; Test function: f32x4.ne with special values (NaN, infinity, zero)
  (func (export "f32x4_ne_special_values") (result i32 i32 i32 i32)
    (local $result v128)
    ;; Create first vector with special values
    ;; NaN, +inf, +0.0, -0.0
    (v128.const f32x4 nan inf 0.0 -0.0)

    ;; Create second vector for comparison
    ;; NaN, +inf, -0.0, +0.0
    (v128.const f32x4 nan inf -0.0 0.0)

    ;; Perform f32x4.ne
    ;; Expected: [true, false, false, false]
    ;; NaN != NaN → true, +inf == +inf → false, +0.0 == -0.0 → false, -0.0 == +0.0 → false
    (f32x4.ne)
    (local.set $result)

    ;; Extract result lanes
    (local.get $result)
    (i32x4.extract_lane 0)  ;; NaN != NaN should be true (0xFFFFFFFF)
    (local.get $result)
    (i32x4.extract_lane 1)  ;; +inf == +inf should be false (0x00000000)
    (local.get $result)
    (i32x4.extract_lane 2)  ;; +0.0 == -0.0 should be false (0x00000000)
    (local.get $result)
    (i32x4.extract_lane 3)  ;; -0.0 == +0.0 should be false (0x00000000)
  )

  ;; Test function: f32x4.ne with boundary values
  (func (export "f32x4_ne_boundary_values") (result i32 i32 i32 i32)
    (local $result v128)
    ;; Create first vector with boundary values
    ;; FLT_MAX, FLT_MIN, very small positive, very small negative
    (v128.const f32x4 3.4028235e+38 1.1754944e-38 1e-38 -1e-38)

    ;; Create second vector for comparison
    ;; FLT_MAX, FLT_MIN*2, very small negative, very small positive
    (v128.const f32x4 3.4028235e+38 2.3509887e-38 -1e-38 1e-38)

    ;; Perform f32x4.ne
    ;; Expected: [false, true, false, true]
    (f32x4.ne)
    (local.set $result)

    ;; Extract result lanes
    (local.get $result)
    (i32x4.extract_lane 0)  ;; FLT_MAX == FLT_MAX should be false (0x00000000)
    (local.get $result)
    (i32x4.extract_lane 1)  ;; FLT_MIN != FLT_MIN*2 should be true (0xFFFFFFFF)
    (local.get $result)
    (i32x4.extract_lane 2)  ;; 1e-38 == -1e-38 should be false (0x00000000)
    (local.get $result)
    (i32x4.extract_lane 3)  ;; -1e-38 != 1e-38 should be true (0xFFFFFFFF)
  )

  ;; Test function: All different values
  (func (export "f32x4_ne_all_different") (result i32 i32 i32 i32)
    (local $result v128)
    ;; Create first vector: [1.5, 2.5, 3.5, 4.5]
    (v128.const f32x4 1.5 2.5 3.5 4.5)

    ;; Create second vector: [5.5, 6.5, 7.5, 8.5]
    (v128.const f32x4 5.5 6.5 7.5 8.5)

    ;; Perform f32x4.ne - all should be not-equal
    (f32x4.ne)
    (local.set $result)

    ;; Extract result lanes - all should be 0xFFFFFFFF
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Test function: All equal values
  (func (export "f32x4_ne_all_equal") (result i32 i32 i32 i32)
    (local $result v128)
    ;; Create first vector: [7.25, 7.25, 7.25, 7.25]
    (v128.const f32x4 7.25 7.25 7.25 7.25)

    ;; Create second vector: [7.25, 7.25, 7.25, 7.25]
    (v128.const f32x4 7.25 7.25 7.25 7.25)

    ;; Perform f32x4.ne - all should be equal
    (f32x4.ne)
    (local.set $result)

    ;; Extract result lanes - all should be 0x00000000
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Test function: NaN vs various values
  (func (export "f32x4_ne_nan_comparisons") (result i32 i32 i32 i32)
    (local $result v128)
    ;; Create first vector: all NaN
    (v128.const f32x4 nan nan nan nan)

    ;; Create second vector: [NaN, 1.0, +inf, 0.0]
    (v128.const f32x4 nan 1.0 inf 0.0)

    ;; Perform f32x4.ne
    ;; All comparisons should be true since NaN != anything (including NaN)
    (f32x4.ne)
    (local.set $result)

    ;; Extract result lanes - all should be 0xFFFFFFFF
    (local.get $result)
    (i32x4.extract_lane 0)  ;; NaN != NaN → true
    (local.get $result)
    (i32x4.extract_lane 1)  ;; NaN != 1.0 → true
    (local.get $result)
    (i32x4.extract_lane 2)  ;; NaN != +inf → true
    (local.get $result)
    (i32x4.extract_lane 3)  ;; NaN != 0.0 → true
  )

  ;; Test function: Negative and positive comparisons
  (func (export "f32x4_ne_negative_positive") (result i32 i32 i32 i32)
    (local $result v128)
    ;; Create first vector: [-1.0, -2.0, 3.0, -4.0]
    (v128.const f32x4 -1.0 -2.0 3.0 -4.0)

    ;; Create second vector: [-1.0, 2.0, -3.0, -4.0]
    (v128.const f32x4 -1.0 2.0 -3.0 -4.0)

    ;; Perform f32x4.ne
    ;; Expected: [false, true, true, false]
    (f32x4.ne)
    (local.set $result)

    ;; Extract result lanes
    (local.get $result)
    (i32x4.extract_lane 0)  ;; -1.0 == -1.0 should be false (0x00000000)
    (local.get $result)
    (i32x4.extract_lane 1)  ;; -2.0 != 2.0 should be true (0xFFFFFFFF)
    (local.get $result)
    (i32x4.extract_lane 2)  ;; 3.0 != -3.0 should be true (0xFFFFFFFF)
    (local.get $result)
    (i32x4.extract_lane 3)  ;; -4.0 == -4.0 should be false (0x00000000)
  )
)