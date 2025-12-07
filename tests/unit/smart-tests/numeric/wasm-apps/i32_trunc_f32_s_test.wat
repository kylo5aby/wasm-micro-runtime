(module
  ;; Test module for i32.trunc_f32_s opcode validation
  ;; Provides comprehensive test functions for floating-point to signed integer truncation

  ;; Basic truncation test function
  ;; @param f32 - input floating-point value to truncate
  ;; @return i32 - truncated signed integer result
  (func $test_basic_trunc (export "test_basic_trunc") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )

  ;; Boundary value test function
  ;; @param f32 - input floating-point value near boundaries
  ;; @return i32 - truncated result or trap for overflow
  (func $test_boundary_values (export "test_boundary_values") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )

  ;; Special value test function
  ;; @param f32 - special IEEE 754 values (zero, NaN, infinity)
  ;; @return i32 - converted result or trap for invalid values
  (func $test_special_values (export "test_special_values") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )

  ;; Error condition test function
  ;; @param f32 - values that should cause traps
  ;; @return i32 - should not return for invalid inputs (trap expected)
  (func $test_error_conditions (export "test_error_conditions") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )

  ;; Stack underflow test function
  ;; Tests i32.trunc_f32_s with insufficient stack preparation
  ;; @return i32 - should trap due to stack underflow (simulated with NaN)
  (func $test_stack_underflow (export "test_stack_underflow") (result i32)
    ;; Use NaN to simulate stack underflow condition since WAT validation
    ;; prevents actual stack underflow at compile time
    f32.const nan
    i32.trunc_f32_s
  )

  ;; Comprehensive test function with multiple scenarios
  ;; @param f32 - test input value
  ;; @return i32 - truncation result for valid inputs
  (func $test_comprehensive (export "test_comprehensive") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )

  ;; Test function for exact integer values
  ;; @param f32 - floating-point values that are exact integers
  ;; @return i32 - should match input without fractional part
  (func $test_exact_integers (export "test_exact_integers") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )

  ;; Test function for fractional values near zero
  ;; @param f32 - small fractional values
  ;; @return i32 - should truncate to zero for values in (-1, 1)
  (func $test_near_zero (export "test_near_zero") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )

  ;; Test function for large magnitude values
  ;; @param f32 - large positive and negative values
  ;; @return i32 - converted result or trap if out of range
  (func $test_large_values (export "test_large_values") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )

  ;; Test function for IEEE 754 edge cases
  ;; @param f32 - edge case values (subnormal, etc.)
  ;; @return i32 - appropriate conversion or trap
  (func $test_ieee754_edge_cases (export "test_ieee754_edge_cases") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_s
  )
)