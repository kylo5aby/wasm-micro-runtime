;; f64x2_max_test.wat - WebAssembly Text Format for f64x2.max opcode testing

(module
  ;; Function: f64x2_max_basic
  ;; Description: Performs f64x2.max operation on two input v128 vectors containing f64 values
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4 for each 128-bit vector)
  ;; Returns: Four i32 values representing v128 result with element-wise maximum values
  (func $f64x2_max_basic (export "f64x2_max_basic")
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

    ;; Apply f64x2.max element-wise maximum operation
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.max)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    ;; Result contains two 64-bit f64 maximums (lane 0 and lane 1)
    (local.get $result)
    (i32x4.extract_lane 0)    ;; Low 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 1)    ;; High 32 bits of lane 0 result
    (local.get $result)
    (i32x4.extract_lane 2)    ;; Low 32 bits of lane 1 result
    (local.get $result)
    (i32x4.extract_lane 3)    ;; High 32 bits of lane 1 result
  )

  ;; Function: f64x2_max_special_values
  ;; Description: Tests f64x2.max with special IEEE 754 values (NaN, infinity, signed zeros)
  ;; Parameters: Eight i32 values representing two v128 inputs with special f64 values
  ;; Returns: Four i32 values representing v128 result
  (func $f64x2_max_special_values (export "f64x2_max_special_values")
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

    ;; Apply f64x2.max with special values
    ;; IEEE 754 behavior: NaN propagation, max(+0.0, -0.0) = +0.0, infinity handling
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.max)
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

  ;; Function: f64x2_max_boundary_test
  ;; Description: Tests f64x2.max with boundary values (DBL_MAX, DBL_MIN, denormal)
  ;; Parameters: Eight i32 values representing two v128 inputs with boundary f64 values
  ;; Returns: Four i32 values representing v128 result
  (func $f64x2_max_boundary_test (export "f64x2_max_boundary_test")
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

    ;; Apply f64x2.max with boundary values
    ;; Tests precision at limits (DBL_MAX, DBL_MIN, denormal numbers)
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.max)
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

  ;; Function: f64x2_max_commutative_test
  ;; Description: Tests f64x2.max commutative property (max(a,b) = max(b,a))
  ;; Parameters: Eight i32 values representing two v128 inputs for commutativity testing
  ;; Returns: Four i32 values representing v128 result (should be same regardless of order)
  (func $f64x2_max_commutative_test (export "f64x2_max_commutative_test")
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

    ;; Create second v128 vector
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

    ;; Apply f64x2.max for commutativity testing
    ;; Validates max(A, B) mathematical property
    (local.get $vector1)
    (local.get $vector2)
    (f64x2.max)
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