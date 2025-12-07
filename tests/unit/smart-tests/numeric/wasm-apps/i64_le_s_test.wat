(module
  ;; Function: i64_le_s_test
  ;; Description: Tests i64.le_s opcode with two i64 parameters
  ;; Parameters: $a (i64), $b (i64) - values to compare
  ;; Returns: i32 - result of signed comparison (1 if $a <= $b, 0 otherwise)
  ;; Stack behavior: consumes 2 i64 values, produces 1 i32 value
  (func $i64_le_s_test (export "i64_le_s_test") (param $a i64) (param $b i64) (result i32)
    ;; Load first parameter onto stack
    local.get $a
    ;; Load second parameter onto stack
    local.get $b
    ;; Execute i64.le_s comparison
    ;; Stack before: [a, b] (top)
    ;; Stack after: [result] where result = (a <= b ? 1 : 0)
    i64.le_s
    ;; Return i32 result (1 for true, 0 for false)
  )

  ;; Function: i64_le_s_boundary_test
  ;; Description: Tests i64.le_s with boundary values (INT64_MAX, INT64_MIN)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of INT64_MIN <= INT64_MAX comparison (should be true)
  (func $i64_le_s_boundary_test (export "i64_le_s_boundary_test") (result i32)
    ;; Push INT64_MIN (-9223372036854775808)
    i64.const -9223372036854775808
    ;; Push INT64_MAX (9223372036854775807)
    i64.const 9223372036854775807
    ;; Execute comparison: INT64_MIN <= INT64_MAX (should be true)
    i64.le_s
  )

  ;; Function: i64_le_s_equal_boundary_test
  ;; Description: Tests i64.le_s with equal boundary values (reflexive property)
  ;; Parameters: None (uses hardcoded equal boundary values)
  ;; Returns: i32 - result of INT64_MAX <= INT64_MAX comparison (should be true)
  (func $i64_le_s_equal_boundary_test (export "i64_le_s_equal_boundary_test") (result i32)
    ;; Push INT64_MAX
    i64.const 9223372036854775807
    ;; Push INT64_MAX again
    i64.const 9223372036854775807
    ;; Execute comparison: INT64_MAX <= INT64_MAX (should be true - reflexive)
    i64.le_s
  )

  ;; Function: i64_le_s_zero_test
  ;; Description: Tests i64.le_s with zero operands
  ;; Parameters: $val (i64) - value to compare against zero
  ;; Returns: i32 - result of $val <= 0 comparison
  (func $i64_le_s_zero_test (export "i64_le_s_zero_test") (param $val i64) (result i32)
    ;; Load parameter
    local.get $val
    ;; Push zero
    i64.const 0
    ;; Execute comparison: $val <= 0
    i64.le_s
  )

  ;; Function: i64_le_s_zero_reflexive_test
  ;; Description: Tests i64.le_s with zero <= zero (reflexive property)
  ;; Parameters: None (uses hardcoded zero values)
  ;; Returns: i32 - result of 0 <= 0 comparison (should be true)
  (func $i64_le_s_zero_reflexive_test (export "i64_le_s_zero_reflexive_test") (result i32)
    ;; Push zero
    i64.const 0
    ;; Push zero again
    i64.const 0
    ;; Execute comparison: 0 <= 0 (should be true - reflexive)
    i64.le_s
  )

  ;; Function: i64_le_s_equal_test
  ;; Description: Tests i64.le_s with equal values (reflexive property)
  ;; Parameters: $val (i64) - value to compare with itself
  ;; Returns: i32 - result of $val <= $val comparison (should always be 1)
  (func $i64_le_s_equal_test (export "i64_le_s_equal_test") (param $val i64) (result i32)
    ;; Load parameter twice
    local.get $val
    local.get $val
    ;; Execute comparison: $val <= $val (always true - reflexive property)
    i64.le_s
  )

  ;; Function: i64_le_s_negative_test
  ;; Description: Tests i64.le_s with negative numbers
  ;; Parameters: None (uses hardcoded negative values)
  ;; Returns: i32 - result of -20 <= -5 comparison (should be true)
  (func $i64_le_s_negative_test (export "i64_le_s_negative_test") (result i32)
    ;; Push -20
    i64.const -20
    ;; Push -5
    i64.const -5
    ;; Execute comparison: -20 <= -5 (should be true in signed arithmetic)
    i64.le_s
  )

  ;; Function: i64_le_s_mixed_sign_true_test
  ;; Description: Tests i64.le_s with negative and positive values (true case)
  ;; Parameters: None (uses hardcoded mixed sign values)
  ;; Returns: i32 - result of -5 <= 7 comparison (should be true)
  (func $i64_le_s_mixed_sign_true_test (export "i64_le_s_mixed_sign_true_test") (result i32)
    ;; Push negative value
    i64.const -5
    ;; Push positive value
    i64.const 7
    ;; Execute comparison: -5 <= 7 (should be true)
    i64.le_s
  )

  ;; Function: i64_le_s_mixed_sign_false_test
  ;; Description: Tests i64.le_s with positive and negative values (false case)
  ;; Parameters: None (uses hardcoded mixed sign values)
  ;; Returns: i32 - result of 12 <= -2 comparison (should be false)
  (func $i64_le_s_mixed_sign_false_test (export "i64_le_s_mixed_sign_false_test") (result i32)
    ;; Push positive value
    i64.const 12
    ;; Push negative value
    i64.const -2
    ;; Execute comparison: 12 <= -2 (should be false)
    i64.le_s
  )

  ;; Function: i64_le_s_sign_boundary_test
  ;; Description: Tests i64.le_s across sign boundary (-1 vs 0)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of -1 <= 0 comparison (should be true)
  (func $i64_le_s_sign_boundary_test (export "i64_le_s_sign_boundary_test") (result i32)
    ;; Push -1
    i64.const -1
    ;; Push 0
    i64.const 0
    ;; Execute comparison: -1 <= 0 (should be true in signed arithmetic)
    i64.le_s
  )

  ;; Function: i64_le_s_positive_boundary_test
  ;; Description: Tests i64.le_s across positive boundary (0 vs 1)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of 0 <= 1 comparison (should be true)
  (func $i64_le_s_positive_boundary_test (export "i64_le_s_positive_boundary_test") (result i32)
    ;; Push 0
    i64.const 0
    ;; Push 1
    i64.const 1
    ;; Execute comparison: 0 <= 1 (should be true)
    i64.le_s
  )

  ;; Function: i64_le_s_adjacent_boundary_test
  ;; Description: Tests i64.le_s with adjacent boundary values
  ;; Parameters: None (uses hardcoded adjacent values)
  ;; Returns: i32 - result of (INT64_MAX-1) <= INT64_MAX comparison (should be true)
  (func $i64_le_s_adjacent_boundary_test (export "i64_le_s_adjacent_boundary_test") (result i32)
    ;; Push INT64_MAX - 1
    i64.const 9223372036854775806
    ;; Push INT64_MAX
    i64.const 9223372036854775807
    ;; Execute comparison: (INT64_MAX-1) <= INT64_MAX (should be true)
    i64.le_s
  )

  ;; Function: i64_le_s_min_adjacent_test
  ;; Description: Tests i64.le_s with minimum adjacent boundary values
  ;; Parameters: None (uses hardcoded adjacent minimum values)
  ;; Returns: i32 - result of INT64_MIN <= (INT64_MIN+1) comparison (should be true)
  (func $i64_le_s_min_adjacent_test (export "i64_le_s_min_adjacent_test") (result i32)
    ;; Push INT64_MIN
    i64.const -9223372036854775808
    ;; Push INT64_MIN + 1
    i64.const -9223372036854775807
    ;; Execute comparison: INT64_MIN <= (INT64_MIN+1) (should be true)
    i64.le_s
  )

  ;; Function: i64_le_s_typical_vs_max_test
  ;; Description: Tests i64.le_s with typical value against maximum value
  ;; Parameters: None (uses hardcoded values)
  ;; Returns: i32 - result of 1000 <= INT64_MAX comparison (should be true)
  (func $i64_le_s_typical_vs_max_test (export "i64_le_s_typical_vs_max_test") (result i32)
    ;; Push typical positive value
    i64.const 1000
    ;; Push INT64_MAX
    i64.const 9223372036854775807
    ;; Execute comparison: 1000 <= INT64_MAX (should be true)
    i64.le_s
  )

  ;; Function: i64_le_s_max_vs_min_false_test
  ;; Description: Tests i64.le_s with maximum vs minimum (false case)
  ;; Parameters: None (uses hardcoded extreme values)
  ;; Returns: i32 - result of INT64_MAX <= INT64_MIN comparison (should be false)
  (func $i64_le_s_max_vs_min_false_test (export "i64_le_s_max_vs_min_false_test") (result i32)
    ;; Push INT64_MAX
    i64.const 9223372036854775807
    ;; Push INT64_MIN
    i64.const -9223372036854775808
    ;; Execute comparison: INT64_MAX <= INT64_MIN (should be false)
    i64.le_s
  )
)