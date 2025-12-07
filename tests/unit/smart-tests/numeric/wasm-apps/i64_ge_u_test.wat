(module
  ;; Function: i64_ge_u_test
  ;; Description: Tests i64.ge_u opcode with two i64 parameters
  ;; Parameters: $a (i64), $b (i64) - values to compare using unsigned semantics
  ;; Returns: i32 - result of unsigned comparison (1 if $a >= $b, 0 otherwise)
  ;; Stack behavior: consumes 2 i64 values, produces 1 i32 value
  (func $i64_ge_u_test (export "i64_ge_u_test") (param $a i64) (param $b i64) (result i32)
    ;; Load first parameter onto stack
    local.get $a
    ;; Load second parameter onto stack
    local.get $b
    ;; Execute i64.ge_u comparison (unsigned greater-than-or-equal)
    ;; Stack before: [a, b] (top)
    ;; Stack after: [result] where result = (a >= b ? 1 : 0) using unsigned semantics
    i64.ge_u
    ;; Return i32 result (1 for true, 0 for false)
  )

  ;; Function: i64_ge_u_boundary_test
  ;; Description: Tests i64.ge_u with unsigned boundary values (UINT64_MAX, 0)
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of UINT64_MAX >= 0 comparison
  (func $i64_ge_u_boundary_test (export "i64_ge_u_boundary_test") (result i32)
    ;; Push UINT64_MAX (0xFFFFFFFFFFFFFFFF)
    i64.const -1  ;; This represents UINT64_MAX in two's complement
    ;; Push 0 (minimum unsigned value)
    i64.const 0
    ;; Execute comparison: UINT64_MAX >= 0 (should be true)
    i64.ge_u
  )

  ;; Function: i64_ge_u_equal_boundary_test
  ;; Description: Tests i64.ge_u with equal boundary values
  ;; Parameters: None (uses hardcoded boundary values)
  ;; Returns: i32 - result of UINT64_MAX >= UINT64_MAX comparison
  (func $i64_ge_u_equal_boundary_test (export "i64_ge_u_equal_boundary_test") (result i32)
    ;; Push UINT64_MAX twice
    i64.const -1  ;; UINT64_MAX
    i64.const -1  ;; UINT64_MAX
    ;; Execute comparison: UINT64_MAX >= UINT64_MAX (should be true)
    i64.ge_u
  )

  ;; Function: i64_ge_u_zero_test
  ;; Description: Tests i64.ge_u with zero operands
  ;; Parameters: $val (i64) - value to compare against zero
  ;; Returns: i32 - result of $val >= 0 comparison (unsigned)
  (func $i64_ge_u_zero_test (export "i64_ge_u_zero_test") (param $val i64) (result i32)
    ;; Load parameter
    local.get $val
    ;; Push zero
    i64.const 0
    ;; Execute unsigned comparison: $val >= 0
    i64.ge_u
  )

  ;; Function: i64_ge_u_equal_test
  ;; Description: Tests i64.ge_u with equal values (reflexive property)
  ;; Parameters: $val (i64) - value to compare with itself
  ;; Returns: i32 - result of $val >= $val comparison (should always be 1)
  (func $i64_ge_u_equal_test (export "i64_ge_u_equal_test") (param $val i64) (result i32)
    ;; Load parameter twice
    local.get $val
    local.get $val
    ;; Execute comparison: $val >= $val (reflexive - should always be 1)
    i64.ge_u
  )

  ;; Function: i64_ge_u_unsigned_semantic_test
  ;; Description: Tests i64.ge_u demonstrating unsigned vs signed semantic differences
  ;; Parameters: None (uses hardcoded values that expose unsigned semantics)
  ;; Returns: i32 - result showing unsigned interpretation of bit patterns
  (func $i64_ge_u_unsigned_semantic_test (export "i64_ge_u_unsigned_semantic_test") (result i32)
    ;; Push 0xFFFFFFFFFFFFFFFF (would be -1 in signed, but UINT64_MAX in unsigned)
    i64.const -1
    ;; Push 1 (small positive value)
    i64.const 1
    ;; Execute comparison: 0xFFFFFFFFFFFFFFFF >= 1 (unsigned, should be true)
    i64.ge_u
  )

  ;; Function: i64_ge_u_high_bit_test
  ;; Description: Tests i64.ge_u with values that have high bit set
  ;; Parameters: None (uses hardcoded values with high bit set)
  ;; Returns: i32 - result of comparison between "negative" signed values treated as unsigned
  (func $i64_ge_u_high_bit_test (export "i64_ge_u_high_bit_test") (result i32)
    ;; Push 0x8000000000000000 (would be INT64_MIN in signed, large positive in unsigned)
    i64.const -9223372036854775808
    ;; Push 0x7FFFFFFFFFFFFFFF (INT64_MAX, largest signed positive)
    i64.const 9223372036854775807
    ;; Execute comparison: 0x8000000000000000 >= 0x7FFFFFFFFFFFFFFF (unsigned, should be true)
    i64.ge_u
  )

  ;; Function: i64_ge_u_transitivity_test
  ;; Description: Tests transitivity property in unsigned comparison: if a >= b and b >= c, then a >= c
  ;; Parameters: $a (i64), $b (i64), $c (i64) - values for transitivity test
  ;; Returns: i32 - result of a >= c comparison after establishing a >= b and b >= c
  (func $i64_ge_u_transitivity_test (export "i64_ge_u_transitivity_test") (param $a i64) (param $b i64) (param $c i64) (result i32)
    ;; This function assumes a >= b and b >= c are already true in unsigned comparison
    ;; We test that a >= c also holds (transitivity)
    local.get $a
    local.get $c
    ;; Execute final comparison: a >= c (unsigned)
    i64.ge_u
  )

  ;; Function: i64_ge_u_large_values_test
  ;; Description: Tests i64.ge_u with large unsigned values near UINT64_MAX
  ;; Parameters: None (uses hardcoded large unsigned values)
  ;; Returns: i32 - result of large unsigned value comparison
  (func $i64_ge_u_large_values_test (export "i64_ge_u_large_values_test") (result i32)
    ;; Push UINT64_MAX - 1 (0xFFFFFFFFFFFFFFFE)
    i64.const -2
    ;; Push UINT64_MAX - 2 (0xFFFFFFFFFFFFFFFD)
    i64.const -3
    ;; Execute comparison: (UINT64_MAX - 1) >= (UINT64_MAX - 2) (should be true)
    i64.ge_u
  )

  ;; Function: i64_ge_u_mixed_range_test
  ;; Description: Tests i64.ge_u with values from different unsigned ranges
  ;; Parameters: None (uses hardcoded values from different unsigned ranges)
  ;; Returns: i32 - result of mixed range unsigned comparison
  (func $i64_ge_u_mixed_range_test (export "i64_ge_u_mixed_range_test") (result i32)
    ;; Push large unsigned value in "negative" signed range
    i64.const -1000  ;; Large unsigned value
    ;; Push small unsigned value in "positive" signed range
    i64.const 1000   ;; Small unsigned value
    ;; Execute comparison: large_unsigned >= small_unsigned (should be true)
    i64.ge_u
  )

  ;; Function: i64_ge_u_near_max_test
  ;; Description: Tests i64.ge_u with values very close to UINT64_MAX
  ;; Parameters: None (uses values near unsigned maximum)
  ;; Returns: i32 - result of near-maximum unsigned comparison
  (func $i64_ge_u_near_max_test (export "i64_ge_u_near_max_test") (result i32)
    ;; Push UINT64_MAX (0xFFFFFFFFFFFFFFFF)
    i64.const -1
    ;; Push UINT64_MAX - 1000 (0xFFFFFFFFFFFFFC17)
    i64.const -1001
    ;; Execute comparison: UINT64_MAX >= (UINT64_MAX - 1000) (should be true)
    i64.ge_u
  )

  ;; Function: i64_ge_u_power_of_two_test
  ;; Description: Tests i64.ge_u with power-of-two values including high bit
  ;; Parameters: None (uses power-of-two test values)
  ;; Returns: i32 - result of power-of-two comparison
  (func $i64_ge_u_power_of_two_test (export "i64_ge_u_power_of_two_test") (result i32)
    ;; Push 2^63 (0x8000000000000000 - high bit set)
    i64.const -9223372036854775808
    ;; Push 2^62 (0x4000000000000000)
    i64.const 4611686018427387904
    ;; Execute comparison: 2^63 >= 2^62 (unsigned, should be true)
    i64.ge_u
  )
)