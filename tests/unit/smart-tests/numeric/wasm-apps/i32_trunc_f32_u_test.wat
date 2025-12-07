(module
  ;; Test module for i32.trunc_f32_u opcode validation
  ;; Provides comprehensive test functions for floating-point to unsigned integer truncation

  ;; Basic conversion test function
  ;; @param f32 - input floating-point value to truncate
  ;; @return i32 - truncated unsigned integer result (interpreted as i32)
  (func $test_basic_conversion (export "test_basic_conversion") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Truncation behavior test function
  ;; @param f32 - input floating-point value with fractional part
  ;; @return i32 - integer part with fractional portion truncated towards zero
  (func $test_truncation_behavior (export "test_truncation_behavior") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Boundary value test function
  ;; @param f32 - input floating-point value near unsigned boundaries
  ;; @return i32 - truncated result within unsigned 32-bit range
  (func $test_boundary_values (export "test_boundary_values") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Overflow condition test function
  ;; @param f32 - values exceeding UINT32_MAX that should cause traps
  ;; @return i32 - should not return for overflow values (trap expected)
  (func $test_overflow_conditions (export "test_overflow_conditions") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Special value test function
  ;; @param f32 - special IEEE 754 values (NaN, infinity)
  ;; @return i32 - should trap for NaN and infinity values
  (func $test_special_values (export "test_special_values") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Zero handling test function
  ;; @param f32 - positive and negative zero values
  ;; @return i32 - should return 0 for both +0.0 and -0.0
  (func $test_zero_handling (export "test_zero_handling") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Negative value test function
  ;; @param f32 - negative floating-point values
  ;; @return i32 - should trap for any negative input (unsigned cannot represent)
  (func $test_negative_values (export "test_negative_values") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Stack underflow test function
  ;; Tests i32.trunc_f32_u with insufficient stack preparation
  ;; @return i32 - should trap due to invalid operation (simulated with negative value)
  (func $test_stack_underflow (export "test_stack_underflow") (result i32)
    ;; Use negative value to simulate error condition since WAT validation
    ;; prevents actual stack underflow at compile time
    f32.const -1.0
    i32.trunc_f32_u
  )

  ;; Comprehensive test function with multiple scenarios
  ;; @param f32 - test input value
  ;; @return i32 - truncation result for valid unsigned inputs
  (func $test_comprehensive (export "test_comprehensive") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Test function for exact integer values
  ;; @param f32 - floating-point values that are exact integers
  ;; @return i32 - should match input without fractional part
  (func $test_exact_integers (export "test_exact_integers") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Test function for fractional values near zero
  ;; @param f32 - small positive fractional values
  ;; @return i32 - should truncate to zero for values in [0, 1)
  (func $test_near_zero (export "test_near_zero") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Test function for large magnitude values
  ;; @param f32 - large positive values within unsigned range
  ;; @return i32 - converted result or trap if exceeding UINT32_MAX
  (func $test_large_values (export "test_large_values") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Test function for IEEE 754 edge cases
  ;; @param f32 - edge case values (subnormal, precision limits)
  ;; @return i32 - appropriate conversion or trap for invalid values
  (func $test_ieee754_edge_cases (export "test_ieee754_edge_cases") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Test function for precision boundary cases
  ;; @param f32 - values where f32 precision affects integer conversion
  ;; @return i32 - converted result with proper precision handling
  (func $test_precision_boundaries (export "test_precision_boundaries") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )

  ;; Test function for maximum representable unsigned values
  ;; @param f32 - values near the maximum unsigned 32-bit integer
  ;; @return i32 - valid conversions within UINT32_MAX range
  (func $test_max_unsigned (export "test_max_unsigned") (param $input f32) (result i32)
    local.get $input
    i32.trunc_f32_u
  )
)