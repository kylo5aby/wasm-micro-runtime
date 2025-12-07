;; f32x4_eq_test.wat - WebAssembly Text Format for f32x4.eq opcode testing

(module
  ;; Function: f32x4_eq_basic
  ;; Description: Performs f32x4.eq operation on two input v128 vectors containing f32 values
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4 for each 128-bit vector)
  ;; Returns: Four i32 values representing v128 result with equality mask
  (func $f32x4_eq_basic (export "f32x4_eq_basic")
    (param $in1_lane0 i32) (param $in1_lane1 i32) (param $in1_lane2 i32) (param $in1_lane3 i32)
    (param $in2_lane0 i32) (param $in2_lane1 i32) (param $in2_lane2 i32) (param $in2_lane3 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector from four i32 values representing f32 bit patterns
    (v128.const i32x4 0 0 0 0)
    (local.get $in1_lane0)    ;; f32 value for lane 0 (as i32 bit pattern)
    (i32x4.replace_lane 0)
    (local.get $in1_lane1)    ;; f32 value for lane 1 (as i32 bit pattern)
    (i32x4.replace_lane 1)
    (local.get $in1_lane2)    ;; f32 value for lane 2 (as i32 bit pattern)
    (i32x4.replace_lane 2)
    (local.get $in1_lane3)    ;; f32 value for lane 3 (as i32 bit pattern)
    (i32x4.replace_lane 3)
    (local.set $vector1)

    ;; Create second v128 vector from four i32 values representing f32 bit patterns
    (v128.const i32x4 0 0 0 0)
    (local.get $in2_lane0)    ;; f32 value for lane 0 (as i32 bit pattern)
    (i32x4.replace_lane 0)
    (local.get $in2_lane1)    ;; f32 value for lane 1 (as i32 bit pattern)
    (i32x4.replace_lane 1)
    (local.get $in2_lane2)    ;; f32 value for lane 2 (as i32 bit pattern)
    (i32x4.replace_lane 2)
    (local.get $in2_lane3)    ;; f32 value for lane 3 (as i32 bit pattern)
    (i32x4.replace_lane 3)
    (local.set $vector2)

    ;; Apply f32x4.eq element-wise equality comparison
    (local.get $vector1)
    (local.get $vector2)
    (f32x4.eq)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    ;; Result is a v128 with four 32-bit lanes (each containing 0xFFFFFFFF or 0x00000000)
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Lane 0 equality result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; Lane 1 equality result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Lane 2 equality result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; Lane 3 equality result
  )

  ;; Function: f32x4_eq_boundary
  ;; Description: Tests f32x4.eq with boundary values (FLT_MAX, FLT_MIN, etc.)
  ;; Returns: Four i32 values representing boundary test results
  (func $f32x4_eq_boundary (export "f32x4_eq_boundary")
    (result i32 i32 i32 i32)
    (local $boundary_vec1 v128)
    (local $boundary_vec2 v128)
    (local $result v128)

    ;; Create boundary test vectors
    ;; Lane 0: FLT_MAX (0x7F7FFFFF) == FLT_MAX (should be true)
    ;; Lane 1: FLT_MIN (0x00800000) == FLT_MIN (should be true)
    ;; Lane 2: -FLT_MAX (0xFF7FFFFF) == -FLT_MAX (should be true)
    ;; Lane 3: Smallest subnormal (0x00000001) == Smallest subnormal (should be true)
    (v128.const i32x4 0x7F7FFFFF 0x00800000 0xFF7FFFFF 0x00000001)
    (local.set $boundary_vec1)

    ;; Create identical boundary vector for comparison
    (v128.const i32x4 0x7F7FFFFF 0x00800000 0xFF7FFFFF 0x00000001)
    (local.set $boundary_vec2)

    ;; Apply f32x4.eq boundary comparison
    (local.get $boundary_vec1)
    (local.get $boundary_vec2)
    (f32x4.eq)
    (local.set $result)

    ;; Extract and return results
    (local.get $result)
    (i32x4.extract_lane 0)    ;; FLT_MAX equality result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; FLT_MIN equality result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; -FLT_MAX equality result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; Subnormal equality result
  )

  ;; Function: f32x4_eq_special
  ;; Description: Tests f32x4.eq with special IEEE 754 values (NaN, infinity, signed zero)
  ;; Returns: Four i32 values representing special values test results
  (func $f32x4_eq_special (export "f32x4_eq_special")
    (result i32 i32 i32 i32)
    (local $special_vec1 v128)
    (local $special_vec2 v128)
    (local $result v128)

    ;; Create first special values vector
    ;; Lane 0: Quiet NaN (0x7FC00000)
    ;; Lane 1: Positive Infinity (0x7F800000)
    ;; Lane 2: Positive Zero (0x00000000)
    ;; Lane 3: Negative Infinity (0xFF800000)
    (v128.const i32x4 0x7FC00000 0x7F800000 0x00000000 0xFF800000)
    (local.set $special_vec1)

    ;; Create second special values vector for comparison
    ;; Lane 0: Same Quiet NaN (0x7FC00000) - should be false per IEEE 754
    ;; Lane 1: Same Positive Infinity (0x7F800000) - should be true
    ;; Lane 2: Negative Zero (0x80000000) - should be true (+0.0 == -0.0)
    ;; Lane 3: Same Negative Infinity (0xFF800000) - should be true
    (v128.const i32x4 0x7FC00000 0x7F800000 0x80000000 0xFF800000)
    (local.set $special_vec2)

    ;; Apply f32x4.eq special values comparison
    (local.get $special_vec1)
    (local.get $special_vec2)
    (f32x4.eq)
    (local.set $result)

    ;; Extract and return results
    (local.get $result)
    (i32x4.extract_lane 0)    ;; NaN comparison result (should be false)
    (local.get $result)
    (i32x4.extract_lane 1)    ;; +Infinity comparison result (should be true)
    (local.get $result)
    (i32x4.extract_lane 2)    ;; +0.0 vs -0.0 comparison result (should be true)
    (local.get $result)
    (i32x4.extract_lane 3)    ;; -Infinity comparison result (should be true)
  )

  ;; Function: f32x4_eq_mixed_cases
  ;; Description: Additional comprehensive test cases for various f32x4.eq scenarios
  ;; Parameters: No parameters - uses predefined test vectors
  ;; Returns: Four i32 values representing mixed test case results
  (func $f32x4_eq_mixed_cases (export "f32x4_eq_mixed_cases")
    (result i32 i32 i32 i32)
    (local $mixed_vec1 v128)
    (local $mixed_vec2 v128)
    (local $result v128)

    ;; Create first mixed test vector
    ;; Lane 0: 1.5f (0x3FC00000)
    ;; Lane 1: -2.75f (0xC0300000)
    ;; Lane 2: 0.0f (0x00000000)
    ;; Lane 3: 42.0f (0x42280000)
    (v128.const i32x4 0x3FC00000 0xC0300000 0x00000000 0x42280000)
    (local.set $mixed_vec1)

    ;; Create second mixed test vector
    ;; Lane 0: 1.5f (0x3FC00000) - same as lane 0 (should be true)
    ;; Lane 1: 3.25f (0x40500000) - different from lane 1 (should be false)
    ;; Lane 2: -0.0f (0x80000000) - different sign zero from lane 2 (should be true per IEEE 754)
    ;; Lane 3: 42.0f (0x42280000) - same as lane 3 (should be true)
    (v128.const i32x4 0x3FC00000 0x40500000 0x80000000 0x42280000)
    (local.set $mixed_vec2)

    ;; Apply f32x4.eq mixed cases comparison
    (local.get $mixed_vec1)
    (local.get $mixed_vec2)
    (f32x4.eq)
    (local.set $result)

    ;; Extract and return results
    (local.get $result)
    (i32x4.extract_lane 0)    ;; 1.5f equality result (should be true)
    (local.get $result)
    (i32x4.extract_lane 1)    ;; -2.75f vs 3.25f result (should be false)
    (local.get $result)
    (i32x4.extract_lane 2)    ;; +0.0f vs -0.0f result (should be true)
    (local.get $result)
    (i32x4.extract_lane 3)    ;; 42.0f equality result (should be true)
  )

  ;; Function: f32x4_eq_edge_nan_variations
  ;; Description: Tests various NaN representations to ensure consistent IEEE 754 behavior
  ;; Returns: Four i32 values representing NaN variation test results
  (func $f32x4_eq_edge_nan_variations (export "f32x4_eq_edge_nan_variations")
    (result i32 i32 i32 i32)
    (local $nan_vec1 v128)
    (local $nan_vec2 v128)
    (local $result v128)

    ;; Create first NaN variations vector
    ;; Lane 0: Quiet NaN with payload 0 (0x7FC00000)
    ;; Lane 1: Quiet NaN with payload 1 (0x7FC00001)
    ;; Lane 2: Signaling NaN (0x7F800001)
    ;; Lane 3: Negative Quiet NaN (0xFFC00000)
    (v128.const i32x4 0x7FC00000 0x7FC00001 0x7F800001 0xFFC00000)
    (local.set $nan_vec1)

    ;; Create second NaN variations vector
    ;; All should result in false comparisons per IEEE 754 standard
    ;; Lane 0: Same Quiet NaN (0x7FC00000) - NaN != NaN
    ;; Lane 1: Different Quiet NaN (0x7FC00002) - NaN != NaN
    ;; Lane 2: Regular value (1.0f = 0x3F800000) - NaN != value
    ;; Lane 3: Same Negative Quiet NaN (0xFFC00000) - NaN != NaN
    (v128.const i32x4 0x7FC00000 0x7FC00002 0x3F800000 0xFFC00000)
    (local.set $nan_vec2)

    ;; Apply f32x4.eq NaN variations comparison
    (local.get $nan_vec1)
    (local.get $nan_vec2)
    (f32x4.eq)
    (local.set $result)

    ;; Extract and return results (all should be false per IEEE 754)
    (local.get $result)
    (i32x4.extract_lane 0)    ;; QNaN == QNaN result (should be false)
    (local.get $result)
    (i32x4.extract_lane 1)    ;; QNaN1 == QNaN2 result (should be false)
    (local.get $result)
    (i32x4.extract_lane 2)    ;; SNaN == 1.0f result (should be false)
    (local.get $result)
    (i32x4.extract_lane 3)    ;; -QNaN == -QNaN result (should be false)
  )
)