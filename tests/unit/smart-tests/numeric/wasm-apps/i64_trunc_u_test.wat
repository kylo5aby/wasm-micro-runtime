(module
  ;; Enhanced test module for i64.trunc_u operations
  ;; Tests unsigned truncation from f32/f64 to i64

  ;; Core test functions matching the C++ test expectations

  ;; Function: test_i64_trunc_u_f32
  ;; Converts f32 to unsigned i64 using truncation
  ;; Input: f32 floating-point value
  ;; Output: i64 unsigned integer (truncated toward zero)
  (func $test_i64_trunc_u_f32 (export "test_i64_trunc_u_f32") (param $input f32) (result i64)
    local.get $input
    i64.trunc_f32_u
  )

  ;; Function: test_i64_trunc_u_f64
  ;; Converts f64 to unsigned i64 using truncation
  ;; Input: f64 floating-point value
  ;; Output: i64 unsigned integer (truncated toward zero)
  (func $test_i64_trunc_u_f64 (export "test_i64_trunc_u_f64") (param $input f64) (result i64)
    local.get $input
    i64.trunc_f64_u
  )

  ;; Basic functionality test functions

  ;; Test basic f32 truncation with typical positive values
  (func $test_basic_f32_typical (export "test_basic_f32_typical") (result i64)
    f32.const 42.7
    i64.trunc_f32_u
  )

  ;; Test basic f64 truncation with typical positive values
  (func $test_basic_f64_typical (export "test_basic_f64_typical") (result i64)
    f64.const 1234.99
    i64.trunc_f64_u
  )

  ;; Test small decimal values that truncate to zero
  (func $test_small_f32_decimal (export "test_small_f32_decimal") (result i64)
    f32.const 0.9
    i64.trunc_f32_u
  )

  (func $test_small_f64_decimal (export "test_small_f64_decimal") (result i64)
    f64.const 0.1
    i64.trunc_f64_u
  )

  ;; Boundary value test functions

  ;; Test f64 value approaching UINT64_MAX
  (func $test_f64_near_max (export "test_f64_near_max") (result i64)
    f64.const 18446744073709550000.0
    i64.trunc_f64_u
  )

  ;; Test values just above zero
  (func $test_f32_one (export "test_f32_one") (result i64)
    f32.const 1.0
    i64.trunc_f32_u
  )

  (func $test_f64_one (export "test_f64_one") (result i64)
    f64.const 1.0
    i64.trunc_f64_u
  )

  ;; Test precision boundary for f32
  (func $test_f32_precision_boundary (export "test_f32_precision_boundary") (result i64)
    f32.const 16777216.0
    i64.trunc_f32_u
  )

  ;; Special value test functions

  ;; Test positive zero
  (func $test_f32_positive_zero (export "test_f32_positive_zero") (result i64)
    f32.const 0.0
    i64.trunc_f32_u
  )

  (func $test_f64_positive_zero (export "test_f64_positive_zero") (result i64)
    f64.const 0.0
    i64.trunc_f64_u
  )

  ;; Test negative zero (should also convert to 0)
  (func $test_f32_negative_zero (export "test_f32_negative_zero") (result i64)
    f32.const -0.0
    i64.trunc_f32_u
  )

  (func $test_f64_negative_zero (export "test_f64_negative_zero") (result i64)
    f64.const -0.0
    i64.trunc_f64_u
  )

  ;; Test identity operations (integers as floats)
  (func $test_f32_identity_42 (export "test_f32_identity_42") (result i64)
    f32.const 42.0
    i64.trunc_f32_u
  )

  (func $test_f64_identity_42 (export "test_f64_identity_42") (result i64)
    f64.const 42.0
    i64.trunc_f64_u
  )

  (func $test_f32_identity_100 (export "test_f32_identity_100") (result i64)
    f32.const 100.0
    i64.trunc_f32_u
  )

  (func $test_f64_identity_100 (export "test_f64_identity_100") (result i64)
    f64.const 100.0
    i64.trunc_f64_u
  )

  ;; Invalid value test functions (these will cause traps)

  ;; Test NaN (should trap)
  (func $test_f32_nan (export "test_f32_nan") (result i64)
    f32.const nan
    i64.trunc_f32_u
  )

  (func $test_f64_nan (export "test_f64_nan") (result i64)
    f64.const nan
    i64.trunc_f64_u
  )

  ;; Test positive infinity (should trap)
  (func $test_f32_positive_inf (export "test_f32_positive_inf") (result i64)
    f32.const inf
    i64.trunc_f32_u
  )

  (func $test_f64_positive_inf (export "test_f64_positive_inf") (result i64)
    f64.const inf
    i64.trunc_f64_u
  )

  ;; Test negative infinity (should trap)
  (func $test_f32_negative_inf (export "test_f32_negative_inf") (result i64)
    f32.const -inf
    i64.trunc_f32_u
  )

  (func $test_f64_negative_inf (export "test_f64_negative_inf") (result i64)
    f64.const -inf
    i64.trunc_f64_u
  )

  ;; Test negative values (should trap for unsigned conversion)
  (func $test_f32_negative_one (export "test_f32_negative_one") (result i64)
    f32.const -1.0
    i64.trunc_f32_u
  )

  (func $test_f64_negative_one (export "test_f64_negative_one") (result i64)
    f64.const -1.0
    i64.trunc_f64_u
  )

  (func $test_f32_negative_fractional (export "test_f32_negative_fractional") (result i64)
    f32.const -42.5
    i64.trunc_f32_u
  )

  (func $test_f64_negative_fractional (export "test_f64_negative_fractional") (result i64)
    f64.const -42.5
    i64.trunc_f64_u
  )

  ;; Test overflow values (≥ 2^64 should trap)
  (func $test_f64_overflow (export "test_f64_overflow") (result i64)
    f64.const 18446744073709551616.0
    i64.trunc_f64_u
  )

  (func $test_f64_large_overflow (export "test_f64_large_overflow") (result i64)
    f64.const 1e20
    i64.trunc_f64_u
  )

  (func $test_f32_overflow (export "test_f32_overflow") (result i64)
    f32.const 18446744073709551616.0
    i64.trunc_f32_u
  )
)