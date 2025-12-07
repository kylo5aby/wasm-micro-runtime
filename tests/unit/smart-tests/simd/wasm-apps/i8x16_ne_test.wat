;; i8x16_ne_test.wat - WebAssembly Text Format for i8x16.ne opcode testing

(module
  ;; Function: test_i8x16_ne
  ;; Description: Performs i8x16.ne operation on two input v128 vectors
  ;; Parameters: Eight i32 values representing two v128 inputs (4+4 for each 128-bit vector)
  ;; Returns: Four i32 values representing v128 result with inequality mask
  (func $test_i8x16_ne (export "test_i8x16_ne")
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

    ;; Apply i8x16.ne element-wise not-equal comparison
    (local.get $vector1)
    (local.get $vector2)
    (i8x16.ne)
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

  ;; Function for testing identity property (a != a should always be false)
  (func $test_i8x16_ne_identity (export "test_i8x16_ne_identity")
    (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
    (result i32)
    (local $vector_a v128)
    (local $result v128)

    ;; Create vector A from input parameters
    (v128.const i32x4 0 0 0 0)
    (local.get $a0)
    (i32x4.replace_lane 0)
    (local.get $a1)
    (i32x4.replace_lane 1)
    (local.get $a2)
    (i32x4.replace_lane 2)
    (local.get $a3)
    (i32x4.replace_lane 3)
    (local.set $vector_a)

    ;; Compare vector A with itself using i8x16.ne
    (local.get $vector_a)
    (local.get $vector_a)
    (i8x16.ne)
    (local.set $result)

    ;; Check if result is all zeros (meaning a != a is always false)
    (local.get $result)
    (v128.const i32x4 0 0 0 0)
    (i8x16.eq)
    (i8x16.all_true)
  )

  ;; Function for testing antisymmetric property (a != b should be opposite of a == b)
  (func $test_i8x16_ne_antisymmetric (export "test_i8x16_ne_antisymmetric")
    (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
    (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
    (result i32)
    (local $vector_a v128)
    (local $vector_b v128)
    (local $result_ne v128)
    (local $result_eq v128)

    ;; Create vector A from input parameters
    (v128.const i32x4 0 0 0 0)
    (local.get $a0)
    (i32x4.replace_lane 0)
    (local.get $a1)
    (i32x4.replace_lane 1)
    (local.get $a2)
    (i32x4.replace_lane 2)
    (local.get $a3)
    (i32x4.replace_lane 3)
    (local.set $vector_a)

    ;; Create vector B from input parameters
    (v128.const i32x4 0 0 0 0)
    (local.get $b0)
    (i32x4.replace_lane 0)
    (local.get $b1)
    (i32x4.replace_lane 1)
    (local.get $b2)
    (i32x4.replace_lane 2)
    (local.get $b3)
    (i32x4.replace_lane 3)
    (local.set $vector_b)

    ;; Compute a != b
    (local.get $vector_a)
    (local.get $vector_b)
    (i8x16.ne)
    (local.set $result_ne)

    ;; Compute a == b
    (local.get $vector_a)
    (local.get $vector_b)
    (i8x16.eq)
    (local.set $result_eq)

    ;; Check if (a != b) == NOT(a == b)
    (local.get $result_ne)
    (local.get $result_eq)
    (v128.not)
    (i8x16.eq)
    (i8x16.all_true)
  )

  ;; Function for testing boundary conditions with MIN/MAX values
  (func $test_i8x16_ne_boundary (export "test_i8x16_ne_boundary")
    (param $test_mode i32)  ;; 0: test MIN values, 1: test MAX values, 2: test mixed
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Test different boundary condition modes
    (local.get $test_mode)
    (i32.const 0)
    (i32.eq)
    (if
      (then
        ;; Test MIN_VALUE (-128) equality
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
            ;; Test MAX_VALUE (+127) equality
            (v128.const i8x16 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127)
            (local.set $vector1)
            (v128.const i8x16 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127)
            (local.set $vector2)
          )
          (else
            ;; Test mixed MIN/MAX inequality
            (v128.const i8x16 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127)
            (local.set $vector1)
            (v128.const i8x16 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128)
            (local.set $vector2)
          )
        )
      )
    )

    ;; Apply i8x16.ne comparison
    (local.get $vector1)
    (local.get $vector2)
    (i8x16.ne)
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

  ;; Function for testing zero vector comparisons
  (func $test_i8x16_ne_zero (export "test_i8x16_ne_zero")
    (param $test_zero_vs_zero i32)  ;; 1: test 0!=0, 0: test 0!=non-zero
    (result i32 i32 i32 i32)
    (local $vector1 v128)
    (local $vector2 v128)
    (local $result v128)

    ;; Set up test vectors based on test mode
    (local.get $test_zero_vs_zero)
    (if
      (then
        ;; Test zero vs zero (should be all false for i8x16.ne)
        (v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
        (local.set $vector1)
        (v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
        (local.set $vector2)
      )
      (else
        ;; Test zero vs non-zero (should be all true for i8x16.ne)
        (v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
        (local.set $vector1)
        (v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)
        (local.set $vector2)
      )
    )

    ;; Apply i8x16.ne comparison
    (local.get $vector1)
    (local.get $vector2)
    (i8x16.ne)
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
)