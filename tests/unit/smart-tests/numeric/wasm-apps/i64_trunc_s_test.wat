(module
  ;; Basic truncation functions for i64.trunc_s operations

  ;; Function: i64_trunc_s_f32
  ;; Converts f32 to i64 using signed truncation
  ;; Input: f32 floating-point value
  ;; Output: i64 signed integer (truncated toward zero)
  (func $i64_trunc_s_f32 (export "i64_trunc_s_f32") (param $input f32) (result i64)
    local.get $input
    i64.trunc_f32_s
  )

  ;; Function: i64_trunc_s_f64
  ;; Converts f64 to i64 using signed truncation
  ;; Input: f64 floating-point value
  ;; Output: i64 signed integer (truncated toward zero)
  (func $i64_trunc_s_f64 (export "i64_trunc_s_f64") (param $input f64) (result i64)
    local.get $input
    i64.trunc_f64_s
  )

  ;; Basic functionality test functions

  ;; Test basic f32 truncation with positive values
  (func $test_basic_f32_positive (export "test_basic_f32_positive") (result i64)
    f32.const 42.7
    i64.trunc_f32_s
  )

  ;; Test basic f32 truncation with negative values
  (func $test_basic_f32_negative (export "test_basic_f32_negative") (result i64)
    f32.const -25.8
    i64.trunc_f32_s
  )

  ;; Test basic f64 truncation with positive values
  (func $test_basic_f64_positive (export "test_basic_f64_positive") (result i64)
    f64.const 1000.9
    i64.trunc_f64_s
  )

  ;; Test basic f64 truncation with negative values
  (func $test_basic_f64_negative (export "test_basic_f64_negative") (result i64)
    f64.const -500.3
    i64.trunc_f64_s
  )

  ;; Boundary value test functions

  ;; Test f32 near i64 upper boundary (within range)
  (func $test_f32_boundary_upper (export "test_f32_boundary_upper") (result i64)
    f32.const 9223372036854775000.0
    i64.trunc_f32_s
  )

  ;; Test f32 near i64 lower boundary (within range)
  (func $test_f32_boundary_lower (export "test_f32_boundary_lower") (result i64)
    f32.const -9223372036854775000.0
    i64.trunc_f32_s
  )

  ;; Test f64 near i64 upper boundary (within range)
  (func $test_f64_boundary_upper (export "test_f64_boundary_upper") (result i64)
    f64.const 9223372036854775000.0
    i64.trunc_f64_s
  )

  ;; Test f64 near i64 lower boundary (within range)
  (func $test_f64_boundary_lower (export "test_f64_boundary_lower") (result i64)
    f64.const -9223372036854775000.0
    i64.trunc_f64_s
  )

  ;; Precision boundary test functions

  ;; Test f32 precision boundary (2^24)
  (func $test_f32_precision_boundary (export "test_f32_precision_boundary") (result i64)
    f32.const 16777216.0
    i64.trunc_f32_s
  )

  ;; Test f32 negative precision boundary
  (func $test_f32_precision_boundary_neg (export "test_f32_precision_boundary_neg") (result i64)
    f32.const -16777216.0
    i64.trunc_f32_s
  )

  ;; Test f64 precision boundary (2^53)
  (func $test_f64_precision_boundary (export "test_f64_precision_boundary") (result i64)
    f64.const 9007199254740992.0
    i64.trunc_f64_s
  )

  ;; Test f64 negative precision boundary
  (func $test_f64_precision_boundary_neg (export "test_f64_precision_boundary_neg") (result i64)
    f64.const -9007199254740992.0
    i64.trunc_f64_s
  )

  ;; Zero value test functions

  ;; Test f32 positive zero
  (func $test_f32_positive_zero (export "test_f32_positive_zero") (result i64)
    f32.const 0.0
    i64.trunc_f32_s
  )

  ;; Test f32 negative zero
  (func $test_f32_negative_zero (export "test_f32_negative_zero") (result i64)
    f32.const -0.0
    i64.trunc_f32_s
  )

  ;; Test f64 positive zero
  (func $test_f64_positive_zero (export "test_f64_positive_zero") (result i64)
    f64.const 0.0
    i64.trunc_f64_s
  )

  ;; Test f64 negative zero
  (func $test_f64_negative_zero (export "test_f64_negative_zero") (result i64)
    f64.const -0.0
    i64.trunc_f64_s
  )

  ;; Test values close to zero (should truncate to 0)
  (func $test_f32_small_positive (export "test_f32_small_positive") (result i64)
    f32.const 0.9
    i64.trunc_f32_s
  )

  (func $test_f32_small_negative (export "test_f32_small_negative") (result i64)
    f32.const -0.9
    i64.trunc_f32_s
  )

  ;; Integer value test functions

  ;; Test f32 integer values (exact representation)
  (func $test_f32_integer_value (export "test_f32_integer_value") (result i64)
    f32.const 42.0
    i64.trunc_f32_s
  )

  ;; Test f32 negative integer values
  (func $test_f32_negative_integer_value (export "test_f32_negative_integer_value") (result i64)
    f32.const -25.0
    i64.trunc_f32_s
  )

  ;; Test f64 power of 2 values
  (func $test_f64_power_of_two_1024 (export "test_f64_power_of_two_1024") (result i64)
    f64.const 1024.0
    i64.trunc_f64_s
  )

  (func $test_f64_power_of_two_2048 (export "test_f64_power_of_two_2048") (result i64)
    f64.const 2048.0
    i64.trunc_f64_s
  )

  ;; NaN test functions (should trap)

  ;; Test f32 NaN (should trap)
  (func $test_f32_nan (export "test_f32_nan") (result i64)
    f32.const nan
    i64.trunc_f32_s
  )

  ;; Test f64 NaN (should trap)
  (func $test_f64_nan (export "test_f64_nan") (result i64)
    f64.const nan
    i64.trunc_f64_s
  )

  ;; Infinity test functions (should trap)

  ;; Test f32 positive infinity (should trap)
  (func $test_f32_pos_inf (export "test_f32_pos_inf") (result i64)
    f32.const inf
    i64.trunc_f32_s
  )

  ;; Test f32 negative infinity (should trap)
  (func $test_f32_neg_inf (export "test_f32_neg_inf") (result i64)
    f32.const -inf
    i64.trunc_f32_s
  )

  ;; Test f64 positive infinity (should trap)
  (func $test_f64_pos_inf (export "test_f64_pos_inf") (result i64)
    f64.const inf
    i64.trunc_f64_s
  )

  ;; Test f64 negative infinity (should trap)
  (func $test_f64_neg_inf (export "test_f64_neg_inf") (result i64)
    f64.const -inf
    i64.trunc_f64_s
  )

  ;; Overflow test functions (should trap)

  ;; Test f32 overflow (value too large for i64, should trap)
  (func $test_f32_overflow (export "test_f32_overflow") (result i64)
    f32.const 1e20
    i64.trunc_f32_s
  )

  ;; Test f32 underflow (value too small for i64, should trap)
  (func $test_f32_underflow (export "test_f32_underflow") (result i64)
    f32.const -1e20
    i64.trunc_f32_s
  )

  ;; Test f64 overflow (value too large for i64, should trap)
  (func $test_f64_overflow (export "test_f64_overflow") (result i64)
    f64.const 1e20
    i64.trunc_f64_s
  )

  ;; Test f64 underflow (value too small for i64, should trap)
  (func $test_f64_underflow (export "test_f64_underflow") (result i64)
    f64.const -1e20
    i64.trunc_f64_s
  )

  ;; Additional comprehensive test functions

  ;; Test mixed precision scenarios
  (func $test_mixed_precision (export "test_mixed_precision") (result i64)
    f64.const 123.456789012345
    i64.trunc_f64_s
  )

  ;; Test large values within valid range
  (func $test_large_valid_f32 (export "test_large_valid_f32") (result i64)
    f32.const 1000000.7
    i64.trunc_f32_s
  )

  (func $test_large_valid_f64 (export "test_large_valid_f64") (result i64)
    f64.const 1000000.9
    i64.trunc_f64_s
  )

  ;; Test negative large values within valid range
  (func $test_negative_large_valid_f32 (export "test_negative_large_valid_f32") (result i64)
    f32.const -1000000.7
    i64.trunc_f32_s
  )

  (func $test_negative_large_valid_f64 (export "test_negative_large_valid_f64") (result i64)
    f64.const -1000000.9
    i64.trunc_f64_s
  )
)