;; f64x2_ne_test.wat - WebAssembly Text Format for f64x2.ne opcode testing

(module
  ;; Function: f64x2_ne_basic
  ;; Description: Performs f64x2.ne operation on two input v128 vectors containing f64 values
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4 for each 128-bit vector)
  ;; Returns: Four i32 values representing v128 result with inequality mask
  (func $f64x2_ne_basic (export "f64x2_ne_basic")
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

    ;; Apply f64x2.ne element-wise not-equal comparison
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.ne)
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

  ;; Function: f64x2_ne_special_values
  ;; Description: Tests f64x2.ne with special IEEE 754 values (NaN, infinity, zero)
  ;; Parameters: Eight i32 values representing two v128 inputs with special f64 values
  ;; Returns: Four i32 values representing v128 result
  (func $f64x2_ne_special_values (export "f64x2_ne_special_values")
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

    ;; Apply f64x2.ne with special values (NaN behavior: NaN != anything including NaN)
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.ne)
    (local.set $result)

    ;; Return result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: f64x2_ne_boundary_values
  ;; Description: Tests f64x2.ne with boundary and extreme values (DBL_MAX, DBL_MIN)
  ;; Parameters: Eight i32 values representing two v128 inputs with boundary f64 values
  ;; Returns: Four i32 values representing v128 result
  (func $f64x2_ne_boundary_values (export "f64x2_ne_boundary_values")
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

    ;; Apply f64x2.ne with boundary values
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.ne)
    (local.set $result)

    ;; Return result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: f64x2_ne_subnormal_values
  ;; Description: Tests f64x2.ne with subnormal/denormal values
  ;; Parameters: Eight i32 values representing two v128 inputs with subnormal f64 values
  ;; Returns: Four i32 values representing v128 result
  (func $f64x2_ne_subnormal_values (export "f64x2_ne_subnormal_values")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector with subnormal values
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

    ;; Create second v128 vector with subnormal values
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

    ;; Apply f64x2.ne with subnormal values
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.ne)
    (local.set $result)

    ;; Return result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: f64x2_ne_identity_test
  ;; Description: Tests f64x2.ne identity property (A != A should be false except for NaN)
  ;; Parameters: Eight i32 values representing identical v128 inputs
  ;; Returns: Four i32 values representing v128 result (should be all zeros except for NaN inputs)
  (func $f64x2_ne_identity_test (export "f64x2_ne_identity_test")
    (param $in_lo_0 i32) (param $in_lo_1 i32) (param $in_hi_0 i32) (param $in_hi_1 i32)
    (param $dummy_0 i32) (param $dummy_1 i32) (param $dummy_2 i32) (param $dummy_3 i32)
    (result i32 i32 i32 i32)
    (local $vector v128)
    (local $result v128)

    ;; Create identical v128 vectors for self-comparison
    (v128.const i32x4 0 0 0 0)
    (local.get $in_lo_0)
    (i32x4.replace_lane 0)
    (local.get $in_lo_1)
    (i32x4.replace_lane 1)
    (local.get $in_hi_0)
    (i32x4.replace_lane 2)
    (local.get $in_hi_1)
    (i32x4.replace_lane 3)
    (local.set $vector)

    ;; Apply f64x2.ne to identical vectors (identity test)
    (local.get $vector)
    (local.get $vector)
    (f64x2.ne)
    (local.set $result)

    ;; Return result - should be all zeros except for NaN values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: f64x2_ne_mixed_scenarios
  ;; Description: Tests f64x2.ne with mixed equal/unequal lanes
  ;; Parameters: Eight i32 values representing two v128 inputs with mixed scenarios
  ;; Returns: Four i32 values representing v128 result
  (func $f64x2_ne_mixed_scenarios (export "f64x2_ne_mixed_scenarios")
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

    ;; Create second v128 vector (designed for mixed equal/unequal results)
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

    ;; Apply f64x2.ne for mixed scenarios
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.ne)
    (local.set $result)

    ;; Return result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )
)