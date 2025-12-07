;; f64x2_eq_test.wat - WebAssembly Text Format for f64x2.eq opcode testing

(module
  ;; Function: f64x2_eq_basic
  ;; Description: Performs f64x2.eq operation on two input v128 vectors containing f64 values
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4 for each 128-bit vector)
  ;; Returns: Four i32 values representing v128 result with equality mask
  (func $f64x2_eq_basic (export "f64x2_eq_basic")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector from four i32 values
    ;; Each f64 value consists of two i32 parts (low and high 32 bits)
    (v128.const i32x4 0 0 0 0)
    (local.get $in1_lo_0)    ;; Low 32 bits of first f64 (lane 0)
    (i32x4.replace_lane 0)
    (local.get $in1_lo_1)    ;; High 32 bits of first f64 (lane 0)
    (i32x4.replace_lane 1)
    (local.get $in1_hi_0)    ;; Low 32 bits of second f64 (lane 1)
    (i32x4.replace_lane 2)
    (local.get $in1_hi_1)    ;; High 32 bits of second f64 (lane 1)
    (i32x4.replace_lane 3)
    (local.set $vector1)

    ;; Create second v128 vector from four i32 values
    (v128.const i32x4 0 0 0 0)
    (local.get $in2_lo_0)    ;; Low 32 bits of first f64 (lane 0)
    (i32x4.replace_lane 0)
    (local.get $in2_lo_1)    ;; High 32 bits of first f64 (lane 0)
    (i32x4.replace_lane 1)
    (local.get $in2_hi_0)    ;; Low 32 bits of second f64 (lane 1)
    (i32x4.replace_lane 2)
    (local.get $in2_hi_1)    ;; High 32 bits of second f64 (lane 1)
    (i32x4.replace_lane 3)
    (local.set $vector2)

    ;; Apply f64x2.eq element-wise equality comparison
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.eq)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    ;; Result is a v128 with two 64-bit lanes (each containing 0xFFFFFFFFFFFFFFFF or 0x0000000000000000)
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Low 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; High 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Low 32 bits of lane 1 result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; High 32 bits of lane 1 result
  )

  ;; Function: f64x2_eq_special_values
  ;; Description: Tests f64x2.eq with special IEEE 754 values (NaN, infinity, zero)
  ;; Parameters: Eight i32 values representing two v128 inputs with special f64 values
  ;; Returns: Four i32 values representing v128 result
  (func $f64x2_eq_special_values (export "f64x2_eq_special_values")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector (may contain NaN, infinity, etc.)
    (v128.const i32x4 0 0 0 0)
    (local.get $in1_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in1_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in1_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in1_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector1)

    ;; Create second v128 vector (may contain NaN, infinity, etc.)
    (v128.const i32x4 0 0 0 0)
    (local.get $in2_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in2_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in2_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in2_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector2)

    ;; Apply f64x2.eq with special values (NaN behavior: NaN == anything is false, including NaN == NaN)
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.eq)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Low 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; High 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Low 32 bits of lane 1 result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; High 32 bits of lane 1 result
  )

  ;; Function: f64x2_eq_boundary_test
  ;; Description: Tests f64x2.eq with boundary values (DBL_MAX, DBL_MIN, denormal)
  ;; Parameters: Eight i32 values representing two v128 inputs with boundary f64 values
  ;; Returns: Four i32 values representing v128 result
  (func $f64x2_eq_boundary_test (export "f64x2_eq_boundary_test")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector with boundary values
    (v128.const i32x4 0 0 0 0)
    (local.get $in1_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in1_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in1_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in1_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector1)

    ;; Create second v128 vector with boundary values
    (v128.const i32x4 0 0 0 0)
    (local.get $in2_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in2_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in2_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in2_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector2)

    ;; Apply f64x2.eq with boundary values (DBL_MAX, DBL_MIN, denormal)
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.eq)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Low 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; High 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Low 32 bits of lane 1 result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; High 32 bits of lane 1 result
  )

  ;; Function: f64x2_eq_identity_test
  ;; Description: Tests f64x2.eq identity operations (same values should equal)
  ;; Parameters: Eight i32 values representing two identical v128 inputs
  ;; Returns: Four i32 values representing v128 result (should be all 0xFFFFFFFF)
  (func $f64x2_eq_identity_test (export "f64x2_eq_identity_test")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector
    (v128.const i32x4 0 0 0 0)
    (local.get $in1_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in1_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in1_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in1_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector1)

    ;; Create second v128 vector (potentially identical to first)
    (v128.const i32x4 0 0 0 0)
    (local.get $in2_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in2_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in2_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in2_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector2)

    ;; Apply f64x2.eq for identity testing (A == A should be true except for NaN)
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.eq)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Low 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; High 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Low 32 bits of lane 1 result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; High 32 bits of lane 1 result
  )

  ;; Function: f64x2_eq_zero_test
  ;; Description: Tests f64x2.eq with positive and negative zero values
  ;; Parameters: Eight i32 values representing zero variants (+0.0, -0.0)
  ;; Returns: Four i32 values representing v128 result (+0.0 == -0.0 should be true per IEEE 754)
  (func $f64x2_eq_zero_test (export "f64x2_eq_zero_test")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector with zero values
    (v128.const i32x4 0 0 0 0)
    (local.get $in1_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in1_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in1_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in1_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector1)

    ;; Create second v128 vector with zero values (may be different sign)
    (v128.const i32x4 0 0 0 0)
    (local.get $in2_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in2_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in2_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in2_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector2)

    ;; Apply f64x2.eq with zero values (+0.0 == -0.0 per IEEE 754)
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.eq)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Low 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; High 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Low 32 bits of lane 1 result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; High 32 bits of lane 1 result
  )
)