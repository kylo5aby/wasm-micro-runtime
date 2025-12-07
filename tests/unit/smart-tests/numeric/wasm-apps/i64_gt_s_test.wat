(module
  ;; Function: i64_gt_s_test
  ;; Description: Tests i64.gt_s opcode with two i64 parameters
  ;; Parameters: $a (i64), $b (i64) - values to compare
  ;; Returns: i32 - result of signed comparison (1 if $a > $b, 0 otherwise)
  ;; Stack behavior: consumes 2 i64 values, produces 1 i32 value
  (func $i64_gt_s_test (export "i64_gt_s_test") (param $a i64) (param $b i64) (result i32)
    ;; Load first parameter onto stack
    local.get $a
    ;; Load second parameter onto stack
    local.get $b
    ;; Execute i64.gt_s comparison
    ;; Stack before: [a, b] (top)
    ;; Stack after: [result] where result = (a > b ? 1 : 0)
    i64.gt_s
    ;; Return i32 result (1 for true, 0 for false)
  )

  ;; Function: i64_gt_s_boundary_test
  ;; Description: Tests i64.gt_s with boundary values (INT64_MAX, INT64_MIN)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of INT64_MAX > INT64_MIN comparison
  (func $i64_gt_s_boundary_test (export "i64_gt_s_boundary_test") (result i32)
    ;; Push INT64_MAX (9223372036854775807)
    i64.const 9223372036854775807
    ;; Push INT64_MIN (-9223372036854775808)
    i64.const -9223372036854775808
    ;; Execute comparison: INT64_MAX > INT64_MIN (should be true)
    i64.gt_s
  )

  ;; Function: i64_gt_s_zero_test
  ;; Description: Tests i64.gt_s with zero operands
  ;; Parameters: $val (i64) - value to compare against zero
  ;; Returns: i32 - result of $val > 0 comparison
  (func $i64_gt_s_zero_test (export "i64_gt_s_zero_test") (param $val i64) (result i32)
    ;; Load parameter
    local.get $val
    ;; Push zero
    i64.const 0
    ;; Execute comparison: $val > 0
    i64.gt_s
  )

  ;; Function: i64_gt_s_equal_test
  ;; Description: Tests i64.gt_s with equal values (anti-reflexive property)
  ;; Parameters: $val (i64) - value to compare with itself
  ;; Returns: i32 - result of $val > $val comparison (should always be 0)
  (func $i64_gt_s_equal_test (export "i64_gt_s_equal_test") (param $val i64) (result i32)
    ;; Load parameter twice
    local.get $val
    local.get $val
    ;; Execute comparison: $val > $val (always false)
    i64.gt_s
  )

  ;; Function: i64_gt_s_negative_test
  ;; Description: Tests i64.gt_s with negative numbers
  ;; Parameters: None (uses hardcoded negative values)
  ;; Returns: i32 - result of -10 > -20 comparison (should be true)
  (func $i64_gt_s_negative_test (export "i64_gt_s_negative_test") (result i32)
    ;; Push -10
    i64.const -10
    ;; Push -20
    i64.const -20
    ;; Execute comparison: -10 > -20 (should be true in signed arithmetic)
    i64.gt_s
  )

  ;; Function: i64_gt_s_mixed_sign_test
  ;; Description: Tests i64.gt_s with positive and negative values
  ;; Parameters: None (uses hardcoded mixed sign values)
  ;; Returns: i32 - result of 42 > -15 comparison (should be true)
  (func $i64_gt_s_mixed_sign_test (export "i64_gt_s_mixed_sign_test") (result i32)
    ;; Push positive value
    i64.const 42
    ;; Push negative value
    i64.const -15
    ;; Execute comparison: 42 > -15 (should be true)
    i64.gt_s
  )

  ;; Function: i64_gt_s_sign_boundary_test
  ;; Description: Tests i64.gt_s across sign boundary (-1 vs 1)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of -1 > 1 comparison (should be false)
  (func $i64_gt_s_sign_boundary_test (export "i64_gt_s_sign_boundary_test") (result i32)
    ;; Push -1
    i64.const -1
    ;; Push 1
    i64.const 1
    ;; Execute comparison: -1 > 1 (should be false in signed arithmetic)
    i64.gt_s
  )

  ;; Function: i64_gt_s_max_vs_typical_test
  ;; Description: Tests i64.gt_s with maximum value against typical value
  ;; Parameters: None (uses hardcoded values)
  ;; Returns: i32 - result of INT64_MAX > 1000 comparison (should be true)
  (func $i64_gt_s_max_vs_typical_test (export "i64_gt_s_max_vs_typical_test") (result i32)
    ;; Push INT64_MAX
    i64.const 9223372036854775807
    ;; Push typical positive value
    i64.const 1000
    ;; Execute comparison: INT64_MAX > 1000 (should be true)
    i64.gt_s
  )

  ;; Function: i64_gt_s_min_vs_typical_test
  ;; Description: Tests i64.gt_s with minimum value against typical negative
  ;; Parameters: None (uses hardcoded values)
  ;; Returns: i32 - result of INT64_MIN > -1000 comparison (should be false)
  (func $i64_gt_s_min_vs_typical_test (export "i64_gt_s_min_vs_typical_test") (result i32)
    ;; Push INT64_MIN
    i64.const -9223372036854775808
    ;; Push typical negative value
    i64.const -1000
    ;; Execute comparison: INT64_MIN > -1000 (should be false)
    i64.gt_s
  )

  ;; Function: i64_gt_s_adjacent_boundary_test
  ;; Description: Tests i64.gt_s with adjacent boundary values
  ;; Parameters: None (uses hardcoded adjacent values)
  ;; Returns: i32 - result of (INT64_MAX-1) > INT64_MAX comparison (should be false)
  (func $i64_gt_s_adjacent_boundary_test (export "i64_gt_s_adjacent_boundary_test") (result i32)
    ;; Push INT64_MAX - 1
    i64.const 9223372036854775806
    ;; Push INT64_MAX
    i64.const 9223372036854775807
    ;; Execute comparison: (INT64_MAX-1) > INT64_MAX (should be false)
    i64.gt_s
  )
)