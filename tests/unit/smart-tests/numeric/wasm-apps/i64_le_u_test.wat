(module
  ;; Function: i64_le_u_test
  ;; Description: Tests i64.le_u opcode with two i64 parameters
  ;; Parameters: $a (i64), $b (i64) - values to compare using unsigned semantics
  ;; Returns: i32 - result of unsigned comparison (1 if $a <= $b, 0 otherwise)
  ;; Stack behavior: consumes 2 i64 values, produces 1 i32 value
  (func $i64_le_u_test (export "i64_le_u_test") (param $a i64) (param $b i64) (result i32)
    ;; Load first parameter onto stack
    local.get $a
    ;; Load second parameter onto stack
    local.get $b
    ;; Execute i64.le_u comparison - unsigned less-than-or-equal
    ;; Stack before: [a, b] (top)
    ;; Stack after: [result] where result = (a <= b ? 1 : 0) using unsigned comparison
    i64.le_u
    ;; Return i32 result (1 for true, 0 for false)
  )

  ;; Function: i64_le_u_boundary_test
  ;; Description: Tests i64.le_u with unsigned boundary values (0 vs UINT64_MAX)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of 0 <= UINT64_MAX comparison (should be true)
  (func $i64_le_u_boundary_test (export "i64_le_u_boundary_test") (result i32)
    ;; Push 0 (minimum unsigned value)
    i64.const 0
    ;; Push UINT64_MAX (0xFFFFFFFFFFFFFFFF = -1 in signed, max unsigned value)
    i64.const -1
    ;; Execute comparison: 0 <= UINT64_MAX (should be true in unsigned)
    i64.le_u
  )

  ;; Function: i64_le_u_max_vs_zero_test
  ;; Description: Tests i64.le_u with UINT64_MAX vs 0 (false case)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of UINT64_MAX <= 0 comparison (should be false)
  (func $i64_le_u_max_vs_zero_test (export "i64_le_u_max_vs_zero_test") (result i32)
    ;; Push UINT64_MAX
    i64.const -1
    ;; Push 0
    i64.const 0
    ;; Execute comparison: UINT64_MAX <= 0 (should be false - maximum vs minimum)
    i64.le_u
  )

  ;; Function: i64_le_u_equal_boundary_test
  ;; Description: Tests i64.le_u with equal boundary values (reflexive property)
  ;; Parameters: None (uses hardcoded equal boundary values)
  ;; Returns: i32 - result of UINT64_MAX <= UINT64_MAX comparison (should be true)
  (func $i64_le_u_equal_boundary_test (export "i64_le_u_equal_boundary_test") (result i32)
    ;; Push UINT64_MAX
    i64.const -1
    ;; Push UINT64_MAX again
    i64.const -1
    ;; Execute comparison: UINT64_MAX <= UINT64_MAX (should be true - reflexive)
    i64.le_u
  )

  ;; Function: i64_le_u_zero_test
  ;; Description: Tests i64.le_u with zero operands
  ;; Parameters: $val (i64) - value to compare against zero
  ;; Returns: i32 - result of $val <= 0 comparison
  (func $i64_le_u_zero_test (export "i64_le_u_zero_test") (param $val i64) (result i32)
    ;; Load parameter
    local.get $val
    ;; Push zero
    i64.const 0
    ;; Execute comparison: $val <= 0 (only true if $val is 0)
    i64.le_u
  )

  ;; Function: i64_le_u_zero_reflexive_test
  ;; Description: Tests i64.le_u with zero <= zero (reflexive property)
  ;; Parameters: None (uses hardcoded zero values)
  ;; Returns: i32 - result of 0 <= 0 comparison (should be true)
  (func $i64_le_u_zero_reflexive_test (export "i64_le_u_zero_reflexive_test") (result i32)
    ;; Push zero
    i64.const 0
    ;; Push zero again
    i64.const 0
    ;; Execute comparison: 0 <= 0 (should be true - reflexive)
    i64.le_u
  )

  ;; Function: i64_le_u_equal_test
  ;; Description: Tests i64.le_u with equal values (reflexive property)
  ;; Parameters: $val (i64) - value to compare with itself
  ;; Returns: i32 - result of $val <= $val comparison (should always be 1)
  (func $i64_le_u_equal_test (export "i64_le_u_equal_test") (param $val i64) (result i32)
    ;; Load parameter twice
    local.get $val
    local.get $val
    ;; Execute comparison: $val <= $val (always true - reflexive property)
    i64.le_u
  )

  ;; Function: i64_le_u_unsigned_vs_signed_test
  ;; Description: Tests i64.le_u with values that differ in signed vs unsigned interpretation
  ;; Parameters: None (uses hardcoded values that show unsigned vs signed difference)
  ;; Returns: i32 - result of UINT64_MAX <= INT64_MAX comparison (false in unsigned, true in signed)
  (func $i64_le_u_unsigned_vs_signed_test (export "i64_le_u_unsigned_vs_signed_test") (result i32)
    ;; Push UINT64_MAX (0xFFFFFFFFFFFFFFFF = -1 signed, max unsigned)
    i64.const -1
    ;; Push INT64_MAX (0x7FFFFFFFFFFFFFFF = max positive signed)
    i64.const 9223372036854775807
    ;; Execute comparison: UINT64_MAX <= INT64_MAX
    ;; In unsigned: 18446744073709551615 <= 9223372036854775807 -> false
    ;; In signed: -1 <= 9223372036854775807 -> true
    i64.le_u
  )

  ;; Function: i64_le_u_sign_bit_test
  ;; Description: Tests i64.le_u with sign bit boundary (INT64_MAX vs first "negative" unsigned)
  ;; Parameters: None (uses hardcoded sign bit boundary values)
  ;; Returns: i32 - result of INT64_MAX <= 0x8000000000000000 comparison (true in unsigned)
  (func $i64_le_u_sign_bit_test (export "i64_le_u_sign_bit_test") (result i32)
    ;; Push INT64_MAX (0x7FFFFFFFFFFFFFFF)
    i64.const 9223372036854775807
    ;; Push 0x8000000000000000 (INT64_MIN in signed, but large positive in unsigned)
    i64.const -9223372036854775808
    ;; Execute comparison: INT64_MAX <= 0x8000000000000000
    ;; In unsigned: 9223372036854775807 <= 9223372036854775808 -> true
    i64.le_u
  )

  ;; Function: i64_le_u_sign_bit_reverse_test
  ;; Description: Tests i64.le_u with reverse sign bit comparison (should be false)
  ;; Parameters: None (uses hardcoded sign bit boundary values)
  ;; Returns: i32 - result of 0x8000000000000000 <= INT64_MAX comparison (false in unsigned)
  (func $i64_le_u_sign_bit_reverse_test (export "i64_le_u_sign_bit_reverse_test") (result i32)
    ;; Push 0x8000000000000000
    i64.const -9223372036854775808
    ;; Push INT64_MAX
    i64.const 9223372036854775807
    ;; Execute comparison: 0x8000000000000000 <= INT64_MAX
    ;; In unsigned: 9223372036854775808 <= 9223372036854775807 -> false
    i64.le_u
  )

  ;; Function: i64_le_u_small_values_test
  ;; Description: Tests i64.le_u with small positive values
  ;; Parameters: None (uses hardcoded small values)
  ;; Returns: i32 - result of 5 <= 10 comparison (should be true)
  (func $i64_le_u_small_values_test (export "i64_le_u_small_values_test") (result i32)
    ;; Push 5
    i64.const 5
    ;; Push 10
    i64.const 10
    ;; Execute comparison: 5 <= 10 (should be true)
    i64.le_u
  )

  ;; Function: i64_le_u_small_reverse_test
  ;; Description: Tests i64.le_u with small values in reverse (false case)
  ;; Parameters: None (uses hardcoded small values)
  ;; Returns: i32 - result of 15 <= 8 comparison (should be false)
  (func $i64_le_u_small_reverse_test (export "i64_le_u_small_reverse_test") (result i32)
    ;; Push 15
    i64.const 15
    ;; Push 8
    i64.const 8
    ;; Execute comparison: 15 <= 8 (should be false)
    i64.le_u
  )

  ;; Function: i64_le_u_adjacent_boundary_test
  ;; Description: Tests i64.le_u with adjacent boundary values
  ;; Parameters: None (uses hardcoded adjacent values)
  ;; Returns: i32 - result of (UINT64_MAX-1) <= UINT64_MAX comparison (should be true)
  (func $i64_le_u_adjacent_boundary_test (export "i64_le_u_adjacent_boundary_test") (result i32)
    ;; Push UINT64_MAX - 1 (0xFFFFFFFFFFFFFFFE)
    i64.const -2
    ;; Push UINT64_MAX (0xFFFFFFFFFFFFFFFF)
    i64.const -1
    ;; Execute comparison: (UINT64_MAX-1) <= UINT64_MAX (should be true)
    i64.le_u
  )

  ;; Function: i64_le_u_min_adjacent_test
  ;; Description: Tests i64.le_u with minimum adjacent boundary values
  ;; Parameters: None (uses hardcoded adjacent minimum values)
  ;; Returns: i32 - result of 0 <= 1 comparison (should be true)
  (func $i64_le_u_min_adjacent_test (export "i64_le_u_min_adjacent_test") (result i32)
    ;; Push 0 (minimum unsigned value)
    i64.const 0
    ;; Push 1 (next minimum value)
    i64.const 1
    ;; Execute comparison: 0 <= 1 (should be true)
    i64.le_u
  )

  ;; Function: i64_le_u_typical_vs_max_test
  ;; Description: Tests i64.le_u with typical value against maximum value
  ;; Parameters: None (uses hardcoded values)
  ;; Returns: i32 - result of 1000 <= UINT64_MAX comparison (should be true)
  (func $i64_le_u_typical_vs_max_test (export "i64_le_u_typical_vs_max_test") (result i32)
    ;; Push typical positive value
    i64.const 1000
    ;; Push UINT64_MAX
    i64.const -1
    ;; Execute comparison: 1000 <= UINT64_MAX (should be true)
    i64.le_u
  )

  ;; Function: i64_le_u_large_vs_small_test
  ;; Description: Tests i64.le_u with large unsigned value vs small value
  ;; Parameters: None (uses hardcoded values)
  ;; Returns: i32 - result of UINT64_MAX <= 1000 comparison (should be false)
  (func $i64_le_u_large_vs_small_test (export "i64_le_u_large_vs_small_test") (result i32)
    ;; Push UINT64_MAX
    i64.const -1
    ;; Push small value
    i64.const 1000
    ;; Execute comparison: UINT64_MAX <= 1000 (should be false)
    i64.le_u
  )

  ;; Function: i64_le_u_mid_range_test
  ;; Description: Tests i64.le_u with mid-range values
  ;; Parameters: None (uses hardcoded mid-range values)
  ;; Returns: i32 - result of 1000000 <= 2000000 comparison (should be true)
  (func $i64_le_u_mid_range_test (export "i64_le_u_mid_range_test") (result i32)
    ;; Push 1000000
    i64.const 1000000
    ;; Push 2000000
    i64.const 2000000
    ;; Execute comparison: 1000000 <= 2000000 (should be true)
    i64.le_u
  )

  ;; Function: i64_le_u_sign_bit_equal_test
  ;; Description: Tests i64.le_u with equal values at sign bit boundary
  ;; Parameters: None (uses hardcoded equal sign bit values)
  ;; Returns: i32 - result of 0x8000000000000000 <= 0x8000000000000000 comparison (should be true)
  (func $i64_le_u_sign_bit_equal_test (export "i64_le_u_sign_bit_equal_test") (result i32)
    ;; Push 0x8000000000000000
    i64.const -9223372036854775808
    ;; Push 0x8000000000000000 again
    i64.const -9223372036854775808
    ;; Execute comparison: 0x8000000000000000 <= 0x8000000000000000 (should be true - reflexive)
    i64.le_u
  )

  ;; Function: i64_le_u_high_bit_vs_max_test
  ;; Description: Tests i64.le_u with high bit set value vs maximum
  ;; Parameters: None (uses hardcoded values)
  ;; Returns: i32 - result of 0x8000000000000000 <= UINT64_MAX comparison (should be true)
  (func $i64_le_u_high_bit_vs_max_test (export "i64_le_u_high_bit_vs_max_test") (result i32)
    ;; Push 0x8000000000000000 (first value with high bit set)
    i64.const -9223372036854775808
    ;; Push UINT64_MAX
    i64.const -1
    ;; Execute comparison: 0x8000000000000000 <= UINT64_MAX (should be true)
    i64.le_u
  )
)