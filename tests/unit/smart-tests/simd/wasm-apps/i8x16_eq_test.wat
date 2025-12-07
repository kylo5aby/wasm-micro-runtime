;; i8x16_eq_test.wat - WebAssembly Text Format for i8x16.eq opcode testing

(module
  ;; Function: test_i8x16_eq
  ;; Description: Performs i8x16.eq operation on two input v128 vectors
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4 for each 128-bit vector)
  ;; Returns: Four i32 values representing v128 result with equality mask
  (func $test_i8x16_eq (export "test_i8x16_eq")
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

    ;; Apply i8x16.eq element-wise equality comparison
    (local.get $vector1)
    (local.get $vector2)
    (i8x16.eq)
    (local.set $result)

    ;; Extract and return four i32 values representing the result
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function for testing identity property (a == a should always be true)
  (func $test_i8x16_eq_identity (export "test_i8x16_eq_identity")
    (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
    (result i32)
    (local $vector_a v128)
    (local $result v128)

    ;; Create vector A
    (v128.const i32x4 0 0 0 0)
    (local.get $a0) (i32x4.replace_lane 0)
    (local.get $a1) (i32x4.replace_lane 1)
    (local.get $a2) (i32x4.replace_lane 2)
    (local.get $a3) (i32x4.replace_lane 3)
    (local.set $vector_a)

    ;; Compare vector A with itself using i8x16.eq
    (local.get $vector_a)
    (local.get $vector_a)
    (i8x16.eq)
    (local.set $result)

    ;; Verify all lanes are true (0xFF) using i8x16.all_true
    (local.get $result)
    (i8x16.all_true)
  )

  ;; Function for testing symmetry property (a == b should equal b == a)
  (func $test_i8x16_eq_symmetry (export "test_i8x16_eq_symmetry")
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

    ;; Compute A == B
    (local.get $vector_a)
    (local.get $vector_b)
    (i8x16.eq)
    (local.set $result_ab)

    ;; Compute B == A
    (local.get $vector_b)
    (local.get $vector_a)
    (i8x16.eq)
    (local.set $result_ba)

    ;; Compare results: return 1 if equal, 0 if different
    ;; Use bitwise XOR to check if results are identical
    (local.get $result_ab)
    (local.get $result_ba)
    (v128.xor)

    ;; Check if XOR result is all zeros (meaning original results were identical)
    (v128.const i32x4 0 0 0 0)
    (i8x16.eq)
    (i8x16.all_true)
  )

  ;; Function for testing boundary conditions with MIN/MAX values
  (func $test_i8x16_eq_boundary (export "test_i8x16_eq_boundary")
    (param $test_mode i32)  ;; 0: test MIN values, 1: test MAX values, 2: test mixed
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Branch based on test mode
    (local.get $test_mode)
    (i32.const 0)
    (i32.eq)
    (if
      (then
        ;; Test MIN values (-128 == 0x80)
        (v128.const i8x16 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128)
        (local.set $vector1)
        (v128.const i8x16 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128)
        (local.set $vector2)
      )
      (else
        (local.get $test_mode)
        (i32.const 1)
        (i32.eq)
        (if
          (then
            ;; Test MAX values (+127 == 0x7F)
            (v128.const i8x16 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127)
            (local.set $vector1)
            (v128.const i8x16 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127)
            (local.set $vector2)
          )
          (else
            ;; Test mixed MIN/MAX alternating pattern
            (v128.const i8x16 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127)
            (local.set $vector1)
            (v128.const i8x16 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128)
            (local.set $vector2)
          )
        )
      )
    )

    ;; Apply i8x16.eq comparison
    (local.get $vector1)
    (local.get $vector2)
    (i8x16.eq)
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

  ;; Function for testing zero vector comparisons
  (func $test_i8x16_eq_zero (export "test_i8x16_eq_zero")
    (param $test_zero_vs_zero i32)  ;; 1: test 0==0, 0: test 0==non-zero
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Always create zero vector for first operand
    (v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
    (local.set $vector1)

    ;; Create second vector based on test parameter
    (local.get $test_zero_vs_zero)
    (if
      (then
        ;; Test zero vs zero
        (v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
        (local.set $vector2)
      )
      (else
        ;; Test zero vs non-zero pattern
        (v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)
        (local.set $vector2)
      )
    )

    ;; Apply i8x16.eq comparison
    (local.get $vector1)
    (local.get $vector2)
    (i8x16.eq)
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
)