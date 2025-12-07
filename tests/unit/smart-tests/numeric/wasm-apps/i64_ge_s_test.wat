(module
  ;; Function: i64_ge_s_test
  ;; Description: Tests i64.ge_s opcode with two i64 parameters
  ;; Parameters: $a (i64), $b (i64) - values to compare
  ;; Returns: i32 - result of signed comparison (1 if $a >= $b, 0 otherwise)
  ;; Stack behavior: consumes 2 i64 values, produces 1 i32 value
  (func $i64_ge_s_test (export "i64_ge_s_test") (param $a i64) (param $b i64) (result i32)
    ;; Load first parameter onto stack
    local.get $a
    ;; Load second parameter onto stack
    local.get $b
    ;; Execute i64.ge_s comparison
    ;; Stack before: [a, b] (top)
    ;; Stack after: [result] where result = (a >= b ? 1 : 0)
    i64.ge_s
    ;; Return i32 result (1 for true, 0 for false)
  )

  ;; Function: i64_ge_s_boundary_test
  ;; Description: Tests i64.ge_s with boundary values (INT64_MAX, INT64_MIN)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of INT64_MAX >= INT64_MIN comparison
  (func $i64_ge_s_boundary_test (export "i64_ge_s_boundary_test") (result i32)
    ;; Push INT64_MAX (9223372036854775807)
    i64.const 9223372036854775807
    ;; Push INT64_MIN (-9223372036854775808)
    i64.const -9223372036854775808
    ;; Execute comparison: INT64_MAX >= INT64_MIN (should be true)
    i64.ge_s
  )

  ;; Function: i64_ge_s_equal_boundary_test
  ;; Description: Tests i64.ge_s with equal boundary values
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of INT64_MAX >= INT64_MAX comparison
  (func $i64_ge_s_equal_boundary_test (export "i64_ge_s_equal_boundary_test") (result i32)
    ;; Push INT64_MAX twice
    i64.const 9223372036854775807
    i64.const 9223372036854775807
    ;; Execute comparison: INT64_MAX >= INT64_MAX (should be true)
    i64.ge_s
  )

  ;; Function: i64_ge_s_zero_test
  ;; Description: Tests i64.ge_s with zero operands
  ;; Parameters: $val (i64) - value to compare against zero
  ;; Returns: i32 - result of $val >= 0 comparison
  (func $i64_ge_s_zero_test (export "i64_ge_s_zero_test") (param $val i64) (result i32)
    ;; Load parameter
    local.get $val
    ;; Push zero
    i64.const 0
    ;; Execute comparison: $val >= 0
    i64.ge_s
  )

  ;; Function: i64_ge_s_equal_test
  ;; Description: Tests i64.ge_s with equal values (reflexive property)
  ;; Parameters: $val (i64) - value to compare with itself
  ;; Returns: i32 - result of $val >= $val comparison (should always be 1)
  (func $i64_ge_s_equal_test (export "i64_ge_s_equal_test") (param $val i64) (result i32)
    ;; Load parameter twice
    local.get $val
    local.get $val
    ;; Execute comparison: $val >= $val (reflexive - should always be 1)
    i64.ge_s
  )

  ;; Function: i64_ge_s_negative_test
  ;; Description: Tests i64.ge_s with negative numbers
  ;; Parameters: $a (i64), $b (i64) - negative values to compare
  ;; Returns: i32 - result of signed comparison with negative operands
  (func $i64_ge_s_negative_test (export "i64_ge_s_negative_test") (param $a i64) (param $b i64) (result i32)
    ;; Load first negative parameter
    local.get $a
    ;; Load second negative parameter
    local.get $b
    ;; Execute comparison with negative values
    i64.ge_s
  )

  ;; Function: i64_ge_s_mixed_sign_test
  ;; Description: Tests i64.ge_s with mixed positive/negative values
  ;; Parameters: $positive (i64), $negative (i64) - mixed sign values
  ;; Returns: i32 - result of positive >= negative comparison
  (func $i64_ge_s_mixed_sign_test (export "i64_ge_s_mixed_sign_test") (param $positive i64) (param $negative i64) (result i32)
    ;; Load positive parameter
    local.get $positive
    ;; Load negative parameter
    local.get $negative
    ;; Execute comparison: positive >= negative (should typically be 1)
    i64.ge_s
  )

  ;; Function: i64_ge_s_transitivity_test
  ;; Description: Tests transitivity property: if a >= b and b >= c, then a >= c
  ;; Parameters: $a (i64), $b (i64), $c (i64) - values for transitivity test
  ;; Returns: i32 - result of a >= c comparison after establishing a >= b and b >= c
  (func $i64_ge_s_transitivity_test (export "i64_ge_s_transitivity_test") (param $a i64) (param $b i64) (param $c i64) (result i32)
    ;; This function assumes a >= b and b >= c are already true
    ;; We test that a >= c also holds (transitivity)
    local.get $a
    local.get $c
    ;; Execute final comparison: a >= c
    i64.ge_s
  )

  ;; Function: i64_ge_s_large_values_test
  ;; Description: Tests i64.ge_s with large values near boundaries
  ;; Parameters: None (uses hardcoded large values)
  ;; Returns: i32 - result of large value comparison
  (func $i64_ge_s_large_values_test (export "i64_ge_s_large_values_test") (result i32)
    ;; Push large positive value (near INT64_MAX)
    i64.const 9223372036854775806  ;; INT64_MAX - 1
    ;; Push smaller large positive value
    i64.const 9223372036854775000  ;; Much smaller than INT64_MAX
    ;; Execute comparison
    i64.ge_s
  )

  ;; Function: i64_ge_s_large_negative_test
  ;; Description: Tests i64.ge_s with large negative values near INT64_MIN
  ;; Parameters: None (uses hardcoded large negative values)
  ;; Returns: i32 - result of large negative value comparison
  (func $i64_ge_s_large_negative_test (export "i64_ge_s_large_negative_test") (result i32)
    ;; Push less negative value (closer to zero)
    i64.const -9223372036854775000  ;; Much greater than INT64_MIN
    ;; Push more negative value (near INT64_MIN)
    i64.const -9223372036854775807  ;; INT64_MIN + 1
    ;; Execute comparison: -9223372036854775000 >= -9223372036854775807 (should be true)
    i64.ge_s
  )
)