;; v128_xor_test.wat - WebAssembly Text Format for v128.xor opcode testing

(module
  ;; Function: test_v128_xor
  ;; Description: Performs v128.xor operation on two input v128 vectors
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4)
  ;; Returns: Four i32 values representing v128 result
  (func $test_v128_xor (export "test_v128_xor")
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

    ;; Apply v128.xor bitwise XOR operation
    (local.get $vector1)
    (local.get $vector2)
    (v128.xor)
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
  (func $test_v128_xor_commutative (export "test_v128_xor_commutative")
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

    ;; Compute A ^ B
    (local.get $vector_a)
    (local.get $vector_b)
    (v128.xor)
    (local.set $result_ab)

    ;; Compute B ^ A
    (local.get $vector_b)
    (local.get $vector_a)
    (v128.xor)
    (local.set $result_ba)

    ;; Compare results: return 1 if equal, 0 if different
    (local.get $result_ab)
    (local.get $result_ba)
    (i8x16.eq)
    (i8x16.all_true)
  )

  ;; Function for testing identity and self-inverse properties
  (func $test_v128_xor_identity (export "test_v128_xor_identity")
    (param $test_type i32)
    (result i64)
    (local $test_vector v128)
    (local $result v128)

    ;; Set test vector (alternating bit pattern)
    (v128.const i64x2 0x5555555555555555 0xAAAAAAAAAAAAAAAA)
    (local.set $test_vector)

    ;; Select identity or self-inverse test based on parameter
    (local.get $test_type)
    (if (result v128)
      (then
        ;; Test type 0: Identity test (XOR with all zeros)
        (local.get $test_vector)
        (v128.const i64x2 0 0)
        (v128.xor)
      )
      (else
        ;; Test type 1: Self-inverse test (XOR with self)
        (local.get $test_vector)
        (local.get $test_vector)
        (v128.xor)
      )
    )

    ;; Extract first 64-bit lane as result
    (i64x2.extract_lane 0)
  )

  ;; Function for testing bit patterns across different lane interpretations
  (func $test_v128_xor_lane_patterns (export "test_v128_xor_lane_patterns")
    (param $pattern_type i32)
    (result i64 i64)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Select bit patterns based on pattern type
    (local.get $pattern_type)
    (if
      (then
        ;; Pattern 0: Checkerboard (0x5555... ^ 0x3333... = 0x6666...)
        (v128.const i64x2 0x5555555555555555 0x5555555555555555)
        (local.set $vector1)
        (v128.const i64x2 0x3333333333333333 0x3333333333333333)
        (local.set $vector2)
      )
      (else
        ;; Pattern 1: Alternating nibbles (0x0F0F... ^ 0xF0F0... = 0xFFFF...)
        (v128.const i64x2 0x0F0F0F0F0F0F0F0F 0x0F0F0F0F0F0F0F0F)
        (local.set $vector1)
        (v128.const i64x2 0xF0F0F0F0F0F0F0F0 0xF0F0F0F0F0F0F0F0)
        (local.set $vector2)
      )
    )

    ;; Perform XOR operation
    (local.get $vector1)
    (local.get $vector2)
    (v128.xor)
    (local.set $result)

    ;; Return both 64-bit lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )

  ;; Function for testing power-of-2 patterns across lanes
  (func $test_v128_xor_power_patterns (export "test_v128_xor_power_patterns")
    (result i64 i64)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Set power-of-2 patterns that create combined result when XORed
    (v128.const i64x2 0x0102040810204080 0x0102040810204080)
    (local.set $vector1)
    (v128.const i64x2 0x8040201008040201 0x8040201008040201)
    (local.set $vector2)

    ;; Perform XOR operation
    (local.get $vector1)
    (local.get $vector2)
    (v128.xor)
    (local.set $result)

    ;; Return both 64-bit lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (local.get $result)
    (i64x2.extract_lane 1)
  )

  ;; Function for testing associativity properties
  (func $test_v128_xor_associativity (export "test_v128_xor_associativity")
    (param $a_lo i64) (param $a_hi i64)
    (param $b_lo i64) (param $b_hi i64)
    (param $c_lo i64) (param $c_hi i64)
    (result i64 i64 i64 i64)
    (local $vector_a v128)
    (local $vector_b v128)
    (local $vector_c v128)
    (local $temp_result v128)
    (local $final_result_1 v128)
    (local $final_result_2 v128)

    ;; Create vectors from input parameters
    (v128.const i64x2 0 0)
    (local.get $a_lo) (i64x2.replace_lane 0)
    (local.get $a_hi) (i64x2.replace_lane 1)
    (local.set $vector_a)

    (v128.const i64x2 0 0)
    (local.get $b_lo) (i64x2.replace_lane 0)
    (local.get $b_hi) (i64x2.replace_lane 1)
    (local.set $vector_b)

    (v128.const i64x2 0 0)
    (local.get $c_lo) (i64x2.replace_lane 0)
    (local.get $c_hi) (i64x2.replace_lane 1)
    (local.set $vector_c)

    ;; Compute (a ^ b) ^ c
    (local.get $vector_a)
    (local.get $vector_b)
    (v128.xor)
    (local.get $vector_c)
    (v128.xor)
    (local.set $final_result_1)

    ;; Compute a ^ (b ^ c)
    (local.get $vector_b)
    (local.get $vector_c)
    (v128.xor)
    (local.get $vector_a)
    (i32.const 0)  ;; swap operands: move a to second position for a ^ (b ^ c)
    (drop)
    (v128.xor)
    (local.set $final_result_2)

    ;; Return both results as four i64 values for comparison
    (local.get $final_result_1) (i64x2.extract_lane 0)
    (local.get $final_result_1) (i64x2.extract_lane 1)
    (local.get $final_result_2) (i64x2.extract_lane 0)
    (local.get $final_result_2) (i64x2.extract_lane 1)
  )

  ;; Function for testing self-inverse property (a ^ a = 0)
  (func $test_v128_xor_self_inverse (export "test_v128_xor_self_inverse")
    (param $input_lo i64) (param $input_hi i64)
    (result i64 i64)
    (local $vector v128)
    (local $result v128)

    ;; Create vector from input
    (v128.const i64x2 0 0)
    (local.get $input_lo) (i64x2.replace_lane 0)
    (local.get $input_hi) (i64x2.replace_lane 1)
    (local.set $vector)

    ;; Perform a ^ a
    (local.get $vector)
    (local.get $vector)
    (v128.xor)
    (local.set $result)

    ;; Return result as two i64 values
    (local.get $result) (i64x2.extract_lane 0)
    (local.get $result) (i64x2.extract_lane 1)
  )

  ;; Function for testing complement property (a ^ ~a = 0xFFFFFFFF...)
  (func $test_v128_xor_complement (export "test_v128_xor_complement")
    (param $input_lo i64) (param $input_hi i64)
    (result i64 i64)
    (local $vector v128)
    (local $complement v128)
    (local $result v128)

    ;; Create vector from input
    (v128.const i64x2 0 0)
    (local.get $input_lo) (i64x2.replace_lane 0)
    (local.get $input_hi) (i64x2.replace_lane 1)
    (local.set $vector)

    ;; Create complement vector (~a)
    (local.get $vector)
    (v128.not)
    (local.set $complement)

    ;; Perform a ^ ~a
    (local.get $vector)
    (local.get $complement)
    (v128.xor)
    (local.set $result)

    ;; Return result as two i64 values
    (local.get $result) (i64x2.extract_lane 0)
    (local.get $result) (i64x2.extract_lane 1)
  )
)