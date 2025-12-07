;; v128_not_test.wat - WebAssembly Text Format for v128.not opcode testing

(module
  ;; Function: test_v128_not
  ;; Description: Performs v128.not operation on input v128 vector
  ;; Parameters: Four i32 values representing v128 input
  ;; Returns: Four i32 values representing v128 result
  (func $test_v128_not (export "test_v128_not")
    (param $in0 i32) (param $in1 i32) (param $in2 i32) (param $in3 i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Create v128 vector from four i32 values
    (v128.const i32x4 0 0 0 0)
    (local.get $in0)
    (i32x4.replace_lane 0)
    (local.get $in1)
    (i32x4.replace_lane 1)
    (local.get $in2)
    (i32x4.replace_lane 2)
    (local.get $in3)
    (i32x4.replace_lane 3)

    ;; Apply v128.not bitwise NOT operation
    (v128.not)
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

  ;; Simple function for pattern testing
  (func $test_v128_not_simple (export "test_v128_not_simple")
    (param $pattern i32)
    (result i64)

    ;; Select test pattern and apply NOT
    (local.get $pattern)
    (if (result v128)
      (then
        ;; Pattern 0: All zeros
        (v128.const i64x2 0 0)
      )
      (else
        ;; Pattern 1: All ones
        (v128.const i64x2 0xFFFFFFFFFFFFFFFF 0xFFFFFFFFFFFFFFFF)
      )
    )

    ;; Apply NOT and extract result
    (v128.not)
    (i64x2.extract_lane 0)
  )
)