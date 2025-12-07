;; f64x2_pmax_test.wat - WebAssembly Text Format for f64x2.pmax opcode testing

(module
  ;; Function: test_f64x2_pmax_basic
  ;; Description: Performs f64x2.pmax operation on two input v128 vectors containing f64 values
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4 for each 128-bit vector)
  ;; Returns: Four i32 values representing v128 result with element-wise pseudo-maximum values
  (func $test_f64x2_pmax_basic (export "test_f64x2_pmax_basic")
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

    ;; Apply f64x2.pmax element-wise pseudo-maximum operation
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmax)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    ;; Result contains two 64-bit f64 pseudo-maximums (lane 0 and lane 1)
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Low 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; High 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Low 32 bits of lane 1 result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; High 32 bits of lane 1 result
  )

  ;; Function: test_f64x2_pmax_boundary
  ;; Description: Tests f64x2.pmax with boundary values (DBL_MAX, infinity, etc.)
  ;; Parameters: Eight i32 values for boundary value testing
  ;; Returns: Four i32 values representing v128 result with boundary pseudo-maximum values
  (func $test_f64x2_pmax_boundary (export "test_f64x2_pmax_boundary")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector for boundary testing
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

    ;; Create second v128 vector for boundary testing
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

    ;; Apply f64x2.pmax for boundary value testing
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmax)
    (local.set $result)

    ;; Extract and return boundary test results
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: test_f64x2_pmax_nan
  ;; Description: Tests f64x2.pmax NaN handling (pseudo-maximum behavior: pmax(NaN,x)=x, pmax(x,NaN)=x)
  ;; Parameters: Eight i32 values for NaN testing scenarios
  ;; Returns: Four i32 values representing v128 result with NaN pseudo-maximum handling
  (func $test_f64x2_pmax_nan (export "test_f64x2_pmax_nan")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector for NaN testing
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

    ;; Create second v128 vector for NaN testing
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

    ;; Apply f64x2.pmax for NaN behavior testing
    ;; Expected: pmax(NaN, x) = x, pmax(x, NaN) = x (pseudo-maximum semantics)
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmax)
    (local.set $result)

    ;; Extract and return NaN test results
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: test_f64x2_pmax_zeros
  ;; Description: Tests f64x2.pmax signed zero handling (+0.0 vs -0.0 distinctions)
  ;; Parameters: Eight i32 values for signed zero testing scenarios
  ;; Returns: Four i32 values representing v128 result with signed zero pseudo-maximum handling
  (func $test_f64x2_pmax_zeros (export "test_f64x2_pmax_zeros")
    (param $in1_lo_0 i32) (param $in1_lo_1 i32) (param $in1_hi_0 i32) (param $in1_hi_1 i32)
    (param $in2_lo_0 i32) (param $in2_lo_1 i32) (param $in2_hi_0 i32) (param $in2_hi_1 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector for signed zero testing
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

    ;; Create second v128 vector for signed zero testing
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

    ;; Apply f64x2.pmax for signed zero behavior testing
    ;; Expected: IEEE 754 signed zero handling in pseudo-maximum operation
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.pmax)
    (local.set $result)

    ;; Extract and return signed zero test results
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