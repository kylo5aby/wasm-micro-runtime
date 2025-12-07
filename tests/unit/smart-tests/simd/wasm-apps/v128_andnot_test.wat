;; v128_andnot_test.wat - WebAssembly Text Format for v128.andnot opcode testing

(module
  ;; Function: test_v128_andnot
  ;; Description: Performs v128.andnot operation (a & ~b) on two input v128 vectors
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4)
  ;; Returns: Four i32 values representing v128 result
  (func $test_v128_andnot (export "test_v128_andnot")
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

    ;; Apply v128.andnot bitwise AND-NOT operation (vector1 & ~vector2)
    (local.get $vector1)
    (local.get $vector2)
    (v128.andnot)
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

  ;; Function for testing non-commutative property (a & ~b ≠ b & ~a)
  (func $test_v128_andnot_non_commutative (export "test_v128_andnot_non_commutative")
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

    ;; Compute A & ~B
    (local.get $vector_a)
    (local.get $vector_b)
    (v128.andnot)
    (local.set $result_ab)

    ;; Compute B & ~A
    (local.get $vector_b)
    (local.get $vector_a)
    (v128.andnot)
    (local.set $result_ba)

    ;; Compare results: return 1 if different (non-commutative), 0 if same
    (local.get $result_ab)
    (local.get $result_ba)
    (i8x16.ne)
    (v128.any_true)
  )

  ;; Function for testing identity and annihilator properties
  (func $test_v128_andnot_identity (export "test_v128_andnot_identity")
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
        ;; Test type 0: Identity test (AND-NOT with all zeros = original)
        (local.get $test_vector)
        (v128.const i64x2 0 0)
        (v128.andnot)
      )
      (else
        ;; Test type 1: Annihilator test (AND-NOT with all ones = zero)
        (local.get $test_vector)
        (v128.const i64x2 0xFFFFFFFFFFFFFFFF 0xFFFFFFFFFFFFFFFF)
        (v128.andnot)
      )
    )

    ;; Extract first 64-bit lane as result
    (i64x2.extract_lane 0)
  )

  ;; Function for testing self-complement property (a & ~a = 0)
  (func $test_v128_andnot_self_complement (export "test_v128_andnot_self_complement")
    (param $pattern_hi i64) (param $pattern_lo i64)
    (result i64 i64)
    (local $test_vector v128)
    (local $result v128)

    ;; Create test vector from parameters
    (v128.const i64x2 0 0)
    (local.get $pattern_lo)
    (i64x2.replace_lane 0)
    (local.get $pattern_hi)
    (i64x2.replace_lane 1)
    (local.set $test_vector)

    ;; Perform self AND-NOT: vector & ~vector = 0
    (local.get $test_vector)
    (local.get $test_vector)
    (v128.andnot)
    (local.set $result)

    ;; Return both 64-bit lanes (should be all zeros)
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )

  ;; Function for testing bit patterns across different lane interpretations
  (func $test_v128_andnot_lane_patterns (export "test_v128_andnot_lane_patterns")
    (param $pattern_type i32)
    (result i64 i64)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Select bit patterns based on pattern type
    (local.get $pattern_type)
    (if
      (then
        ;; Pattern 0: Alternating bits (0x5555... & ~0xAAAA...)
        (v128.const i64x2 0x5555555555555555 0x5555555555555555)
        (local.set $vector1)
        (v128.const i64x2 0xAAAAAAAAAAAAAAAA 0xAAAAAAAAAAAAAAAA)
        (local.set $vector2)
      )
      (else
        ;; Pattern 1: Nibble alternation (0x0F0F... & ~0xF0F0...)
        (v128.const i64x2 0x0F0F0F0F0F0F0F0F 0x0F0F0F0F0F0F0F0F)
        (local.set $vector1)
        (v128.const i64x2 0xF0F0F0F0F0F0F0F0 0xF0F0F0F0F0F0F0F0)
        (local.set $vector2)
      )
    )

    ;; Perform AND-NOT operation
    (local.get $vector1)
    (local.get $vector2)
    (v128.andnot)
    (local.set $result)

    ;; Return both 64-bit lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )

  ;; Function for testing power-of-2 patterns
  (func $test_v128_andnot_power_patterns (export "test_v128_andnot_power_patterns")
    (result i64 i64)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Set power-of-2 patterns that should preserve first operand when no overlap
    (v128.const i64x2 0x0102040810204080 0x0102040810204080)
    (local.set $vector1)
    (v128.const i64x2 0x8040201008040201 0x8040201008040201)
    (local.set $vector2)

    ;; Perform AND-NOT operation: should preserve vector1 since no bit overlap
    (local.get $vector1)
    (local.get $vector2)
    (v128.andnot)
    (local.set $result)

    ;; Return both 64-bit lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )

  ;; Function for testing bit clearing operations (practical use case)
  (func $test_v128_andnot_bit_clearing (export "test_v128_andnot_bit_clearing")
    (param $data_hi i64) (param $data_lo i64)
    (param $mask_hi i64) (param $mask_lo i64)
    (result i64 i64)
    (local $data_vector v128)
    (local $mask_vector v128)
    (local $result v128)

    ;; Create data vector
    (v128.const i64x2 0 0)
    (local.get $data_lo)
    (i64x2.replace_lane 0)
    (local.get $data_hi)
    (i64x2.replace_lane 1)
    (local.set $data_vector)

    ;; Create mask vector
    (v128.const i64x2 0 0)
    (local.get $mask_lo)
    (i64x2.replace_lane 0)
    (local.get $mask_hi)
    (i64x2.replace_lane 1)
    (local.set $mask_vector)

    ;; Clear bits where mask is 1: data & ~mask
    (local.get $data_vector)
    (local.get $mask_vector)
    (v128.andnot)
    (local.set $result)

    ;; Return both 64-bit lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )

  ;; Function for testing mathematical relationship validation
  (func $test_v128_andnot_math_relationship (export "test_v128_andnot_math_relationship")
    (param $a_hi i64) (param $a_lo i64)
    (param $b_hi i64) (param $b_lo i64)
    (result i64 i64)
    (local $vector_a v128)
    (local $vector_b v128)
    (local $result v128)

    ;; Create vectors from parameters
    (v128.const i64x2 0 0)
    (local.get $a_lo)
    (i64x2.replace_lane 0)
    (local.get $a_hi)
    (i64x2.replace_lane 1)
    (local.set $vector_a)

    (v128.const i64x2 0 0)
    (local.get $b_lo)
    (i64x2.replace_lane 0)
    (local.get $b_hi)
    (i64x2.replace_lane 1)
    (local.set $vector_b)

    ;; Compute a & ~b
    (local.get $vector_a)
    (local.get $vector_b)
    (v128.andnot)
    (local.set $result)

    ;; Return both 64-bit lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )
)