;; v128_and_test.wat - WebAssembly Text Format for v128.and opcode testing

(module
  ;; Function: test_v128_and
  ;; Description: Performs v128.and operation on two input v128 vectors
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4)
  ;; Returns: Four i32 values representing v128 result
  (func $test_v128_and (export "test_v128_and")
    (param $in1_0 i32) (param $in1_1 i32) (param $in1_2 i32) (param $in1_3 i32)
    (param $in2_0 i32) (param $in2_1 i32) (param $in2_2 i32) (param $in2_3 i32)
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Create first v128 vector from four i32 values
    (v128.const i32x4 0 0 0 0)
    (local.get $in1_0)
    (i32x4.replace_lane 0)
    (local.get $in1_1)
    (i32x4.replace_lane 1)
    (local.get $in1_2)
    (i32x4.replace_lane 2)
    (local.get $in1_3)
    (i32x4.replace_lane 3)
    (local.set $vector1)

    ;; Create second v128 vector from four i32 values
    (v128.const i32x4 0 0 0 0)
    (local.get $in2_0)
    (i32x4.replace_lane 0)
    (local.get $in2_1)
    (i32x4.replace_lane 1)
    (local.get $in2_2)
    (i32x4.replace_lane 2)
    (local.get $in2_3)
    (i32x4.replace_lane 3)
    (local.set $vector2)

    ;; Apply v128.and bitwise AND operation
    (local.get $vector1)
    (local.get $vector2)
    (v128.and)
    (local.set $result)

    ;; Extract and return four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function for testing mathematical properties (commutativity)
  (func $test_v128_and_commutative (export "test_v128_and_commutative")
    (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
    (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
    (result i32)
    (local $vector_a v128)
    (local $vector_b v128)
    (local $result_ab v128)
    (local $result_ba v128)

    ;; Create vector A
    (v128.const i32x4 0 0 0 0)
    (local.get $a0) (i32x4.replace_lane 0)
    (local.get $a1) (i32x4.replace_lane 1)
    (local.get $a2) (i32x4.replace_lane 2)
    (local.get $a3) (i32x4.replace_lane 3)
    (local.set $vector_a)

    ;; Create vector B
    (v128.const i32x4 0 0 0 0)
    (local.get $b0) (i32x4.replace_lane 0)
    (local.get $b1) (i32x4.replace_lane 1)
    (local.get $b2) (i32x4.replace_lane 2)
    (local.get $b3) (i32x4.replace_lane 3)
    (local.set $vector_b)

    ;; Compute A & B
    (local.get $vector_a)
    (local.get $vector_b)
    (v128.and)
    (local.set $result_ab)

    ;; Compute B & A
    (local.get $vector_b)
    (local.get $vector_a)
    (v128.and)
    (local.set $result_ba)

    ;; Compare results: return 1 if equal, 0 if different
    (local.get $result_ab)
    (local.get $result_ba)
    (i8x16.eq)
    (i8x16.all_true)
  )

  ;; Function for testing identity and annihilator properties
  (func $test_v128_and_identity (export "test_v128_and_identity")
    (param $test_type i32)
    (result i64)
    (local $test_vector v128)
    (local $result v128)

    ;; Set test vector (alternating bit pattern)
    (v128.const i64x2 0x5555555555555555 0xAAAAAAAAAAAAAAAA)
    (local.set $test_vector)

    ;; Select identity or annihilator test based on parameter
    (local.get $test_type)
    (if (result v128)
      (then
        ;; Test type 0: Identity test (AND with all ones)
        (local.get $test_vector)
        (v128.const i64x2 0xFFFFFFFFFFFFFFFF 0xFFFFFFFFFFFFFFFF)
        (v128.and)
      )
      (else
        ;; Test type 1: Annihilator test (AND with all zeros)
        (local.get $test_vector)
        (v128.const i64x2 0 0)
        (v128.and)
      )
    )

    ;; Extract first 64-bit lane as result
    (i64x2.extract_lane 0)
  )

  ;; Function for testing bit patterns across different lane interpretations
  (func $test_v128_and_lane_patterns (export "test_v128_and_lane_patterns")
    (param $pattern_type i32)
    (result i64 i64)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Select bit patterns based on pattern type
    (local.get $pattern_type)
    (if
      (then
        ;; Pattern 0: Checkerboard (0x5555... & 0x3333...)
        (v128.const i64x2 0x5555555555555555 0x5555555555555555)
        (local.set $vector1)
        (v128.const i64x2 0x3333333333333333 0x3333333333333333)
        (local.set $vector2)
      )
      (else
        ;; Pattern 1: Alternating nibbles (0x0F0F... & 0xF0F0...)
        (v128.const i64x2 0x0F0F0F0F0F0F0F0F 0x0F0F0F0F0F0F0F0F)
        (local.set $vector1)
        (v128.const i64x2 0xF0F0F0F0F0F0F0F0 0xF0F0F0F0F0F0F0F0)
        (local.set $vector2)
      )
    )

    ;; Perform AND operation
    (local.get $vector1)
    (local.get $vector2)
    (v128.and)
    (local.set $result)

    ;; Return both 64-bit lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )

  ;; Function for testing power-of-2 patterns across lanes
  (func $test_v128_and_power_patterns (export "test_v128_and_power_patterns")
    (result i64 i64)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Set power-of-2 patterns that should result in all zeros when ANDed
    (v128.const i64x2 0x0102040810204080 0x0102040810204080)
    (local.set $vector1)
    (v128.const i64x2 0x8040201008040201 0x8040201008040201)
    (local.set $vector2)

    ;; Perform AND operation
    (local.get $vector1)
    (local.get $vector2)
    (v128.and)
    (local.set $result)

    ;; Return both 64-bit lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )
)