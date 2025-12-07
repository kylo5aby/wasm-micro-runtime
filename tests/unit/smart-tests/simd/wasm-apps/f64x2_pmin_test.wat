;; f64x2_pmin_test.wat - WebAssembly Text Format for f64x2.pmin opcode testing

(module
  ;; Function: f64x2_pmin_basic
  ;; Description: Performs f64x2.pmin operation on two input v128 vectors containing f64 values
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4 for each 128-bit vector)
  ;; Returns: Four i32 values representing v128 result with element-wise pseudo-minimum values
  (func $f64x2_pmin_basic (export "f64x2_pmin_basic")
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

    ;; Apply f64x2.pmin element-wise pseudo-minimum operation
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmin)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    ;; Result contains two 64-bit f64 pseudo-minimums (lane 0 and lane 1)
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Low 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; High 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Low 32 bits of lane 1 result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; High 32 bits of lane 1 result
  )

  ;; Function: f64x2_pmin_nan
  ;; Description: Tests f64x2.pmin with NaN values to validate pseudo-minimum NaN propagation rules
  ;; Parameters: Eight i32 values representing two v128 inputs with potential NaN values
  ;; Returns: Four i32 values representing v128 result with pseudo-minimum NaN handling
  (func $f64x2_pmin_nan (export "f64x2_pmin_nan")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector from four i32 values
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

    ;; Create second v128 vector from four i32 values
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

    ;; Apply f64x2.pmin with NaN handling
    ;; pmin(NaN, x) = x, pmin(x, NaN) = x, pmin(NaN, NaN) = NaN (second operand)
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmin)
    (local.set $result)

    ;; Extract result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: f64x2_pmin_special
  ;; Description: Tests f64x2.pmin with special IEEE 754 values (infinities, signed zeros)
  ;; Parameters: Eight i32 values representing two v128 inputs with special values
  ;; Returns: Four i32 values representing v128 result with special value handling
  (func $f64x2_pmin_special (export "f64x2_pmin_special")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector from four i32 values
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

    ;; Create second v128 vector from four i32 values
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

    ;; Apply f64x2.pmin with special value handling
    ;; Handles +inf, -inf, +0.0, -0.0 according to IEEE 754 pseudo-minimum rules
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmin)
    (local.set $result)

    ;; Extract result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: f64x2_pmin_boundary
  ;; Description: Tests f64x2.pmin with boundary and extreme values (DBL_MAX, DBL_MIN)
  ;; Parameters: Eight i32 values representing two v128 inputs with boundary values
  ;; Returns: Four i32 values representing v128 result with boundary value handling
  (func $f64x2_pmin_boundary (export "f64x2_pmin_boundary")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector from four i32 values
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

    ;; Create second v128 vector from four i32 values
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

    ;; Apply f64x2.pmin with boundary value handling
    ;; Tests extreme values like DBL_MAX, DBL_MIN, large magnitude differences
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmin)
    (local.set $result)

    ;; Extract result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: f64x2_pmin_independence
  ;; Description: Tests f64x2.pmin lane independence with mixed scenarios per lane
  ;; Parameters: Eight i32 values representing two v128 inputs with different scenarios per lane
  ;; Returns: Four i32 values representing v128 result validating lane independence
  (func $f64x2_pmin_independence (export "f64x2_pmin_independence")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector from four i32 values
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

    ;; Create second v128 vector from four i32 values
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

    ;; Apply f64x2.pmin with cross-lane independence validation
    ;; Each lane should process independently without interference
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmin)
    (local.set $result)

    ;; Extract result as four i32 values
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