(module
  ;; Test function for i64.extend_i32_s opcode
  ;; This function takes an i32 parameter and returns the sign-extended i64 value
  (func $extend_i32_s_test (export "extend_i32_s_test") (param $input i32) (result i64)
    ;; Load the i32 parameter onto the stack
    local.get $input

    ;; Perform i64.extend_i32_s operation
    ;; This pops the i32 from stack and pushes the sign-extended i64
    i64.extend_i32_s

    ;; Return the i64 result
  )

  ;; Test function for boundary values - INT32_MAX
  (func $extend_max_test (export "extend_max_test") (result i64)
    ;; Load INT32_MAX (0x7FFFFFFF) onto stack
    i32.const 0x7FFFFFFF

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0x000000007FFFFFFF
  )

  ;; Test function for boundary values - INT32_MIN
  (func $extend_min_test (export "extend_min_test") (result i64)
    ;; Load INT32_MIN (0x80000000) onto stack
    i32.const 0x80000000

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0xFFFFFFFF80000000
  )

  ;; Test function for zero value
  (func $extend_zero_test (export "extend_zero_test") (result i64)
    ;; Load zero onto stack
    i32.const 0

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0x0000000000000000
  )

  ;; Test function for positive one
  (func $extend_one_test (export "extend_one_test") (result i64)
    ;; Load positive one onto stack
    i32.const 1

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0x0000000000000001
  )

  ;; Test function for negative one
  (func $extend_neg_one_test (export "extend_neg_one_test") (result i64)
    ;; Load negative one onto stack
    i32.const -1

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0xFFFFFFFFFFFFFFFF
  )

  ;; Test function for positive typical value
  (func $extend_positive_test (export "extend_positive_test") (result i64)
    ;; Load positive value 100 onto stack
    i32.const 100

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0x0000000000000064
  )

  ;; Test function for negative typical value
  (func $extend_negative_test (export "extend_negative_test") (result i64)
    ;; Load negative value -100 onto stack
    i32.const -100

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0xFFFFFFFFFFFFFF9C
  )

  ;; Test function for value near INT32_MAX
  (func $extend_near_max_test (export "extend_near_max_test") (result i64)
    ;; Load 0x7FFFFFFE (INT32_MAX - 1) onto stack
    i32.const 0x7FFFFFFE

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0x000000007FFFFFFE
  )

  ;; Test function for value near INT32_MIN
  (func $extend_near_min_test (export "extend_near_min_test") (result i64)
    ;; Load 0x80000001 (INT32_MIN + 1) onto stack
    i32.const 0x80000001

    ;; Perform sign extension
    i64.extend_i32_s

    ;; Should result in 0xFFFFFFFF80000001
  )
)