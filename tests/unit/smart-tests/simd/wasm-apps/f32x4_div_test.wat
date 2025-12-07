(module
  ;; Test functions for f32x4.div opcode validation

  ;; Function to test f32x4.div with eight input f32 values (two vectors)
  ;; Parameters: four f32 values for dividend vector, four f32 values for divisor vector
  ;; Returns: v128 result of f32x4.div operation
  (func $test_f32x4_div (export "test_f32x4_div")
    (param $dividend0 f32) (param $dividend1 f32) (param $dividend2 f32) (param $dividend3 f32)
    (param $divisor0 f32) (param $divisor1 f32) (param $divisor2 f32) (param $divisor3 f32)
    (result v128)

    ;; Create dividend f32x4 vector from first four parameters
    local.get $dividend3
    f32x4.splat
    local.get $dividend0
    f32x4.replace_lane 0
    local.get $dividend1
    f32x4.replace_lane 1
    local.get $dividend2
    f32x4.replace_lane 2

    ;; Create divisor f32x4 vector from last four parameters
    local.get $divisor3
    f32x4.splat
    local.get $divisor0
    f32x4.replace_lane 0
    local.get $divisor1
    f32x4.replace_lane 1
    local.get $divisor2
    f32x4.replace_lane 2

    ;; Apply f32x4.div operation (dividend / divisor)
    f32x4.div
  )

  ;; Test function for basic division functionality
  ;; Tests: [8.0, 4.0, 6.0, 12.0] / [2.0, 2.0, 3.0, 4.0]
  ;; Expected result: [4.0, 2.0, 2.0, 3.0]
  (func $test_basic_division (export "test_basic_division") (result v128)
    ;; Create dividend vector [8.0, 4.0, 6.0, 12.0]
    f32.const 12.0
    f32x4.splat
    f32.const 8.0
    f32x4.replace_lane 0
    f32.const 4.0
    f32x4.replace_lane 1
    f32.const 6.0
    f32x4.replace_lane 2

    ;; Create divisor vector [2.0, 2.0, 3.0, 4.0]
    f32.const 4.0
    f32x4.splat
    f32.const 2.0
    f32x4.replace_lane 0
    f32.const 2.0
    f32x4.replace_lane 1
    f32.const 3.0
    f32x4.replace_lane 2

    f32x4.div
  )

  ;; Test function for division by one (identity property)
  ;; Tests: [5.0, -3.0, 7.0, -9.0] / [1.0, 1.0, 1.0, 1.0]
  ;; Expected result: [5.0, -3.0, 7.0, -9.0]
  (func $test_division_by_one (export "test_division_by_one") (result v128)
    ;; Create dividend vector [5.0, -3.0, 7.0, -9.0]
    f32.const -9.0
    f32x4.splat
    f32.const 5.0
    f32x4.replace_lane 0
    f32.const -3.0
    f32x4.replace_lane 1
    f32.const 7.0
    f32x4.replace_lane 2

    ;; Create divisor vector [1.0, 1.0, 1.0, 1.0]
    f32.const 1.0
    f32x4.splat

    f32x4.div
  )

  ;; Test function for division by negative one (negation property)
  ;; Tests: [2.0, -4.0, 6.0, -8.0] / [-1.0, -1.0, -1.0, -1.0]
  ;; Expected result: [-2.0, 4.0, -6.0, 8.0]
  (func $test_division_by_negative_one (export "test_division_by_negative_one") (result v128)
    ;; Create dividend vector [2.0, -4.0, 6.0, -8.0]
    f32.const -8.0
    f32x4.splat
    f32.const 2.0
    f32x4.replace_lane 0
    f32.const -4.0
    f32x4.replace_lane 1
    f32.const 6.0
    f32x4.replace_lane 2

    ;; Create divisor vector [-1.0, -1.0, -1.0, -1.0]
    f32.const -1.0
    f32x4.splat

    f32x4.div
  )

  ;; Test function for division by zero (should produce infinity)
  ;; Tests: [1.0, -1.0, 2.0, -2.0] / [0.0, 0.0, 0.0, 0.0]
  ;; Expected result: [+inf, -inf, +inf, -inf]
  (func $test_divide_by_zero (export "test_divide_by_zero") (result v128)
    ;; Create dividend vector [1.0, -1.0, 2.0, -2.0]
    f32.const -2.0
    f32x4.splat
    f32.const 1.0
    f32x4.replace_lane 0
    f32.const -1.0
    f32x4.replace_lane 1
    f32.const 2.0
    f32x4.replace_lane 2

    ;; Create divisor vector [0.0, 0.0, 0.0, 0.0]
    f32.const 0.0
    f32x4.splat

    f32x4.div
  )

  ;; Test function for infinity divided by infinity (should produce NaN)
  ;; Tests: [+inf, -inf, +inf, -inf] / [+inf, +inf, -inf, -inf]
  ;; Expected result: [NaN, NaN, NaN, NaN]
  (func $test_infinity_div_infinity (export "test_infinity_div_infinity") (result v128)
    ;; Create dividend vector [+inf, -inf, +inf, -inf]
    f32.const -inf
    f32x4.splat
    f32.const inf
    f32x4.replace_lane 0
    f32.const -inf
    f32x4.replace_lane 1
    f32.const inf
    f32x4.replace_lane 2

    ;; Create divisor vector [+inf, +inf, -inf, -inf]
    f32.const -inf
    f32x4.splat
    f32.const inf
    f32x4.replace_lane 0
    f32.const inf
    f32x4.replace_lane 1
    f32.const -inf
    f32x4.replace_lane 2

    f32x4.div
  )

  ;; Test function for zero divided by zero (should produce NaN)
  ;; Tests: [0.0, -0.0, 0.0, -0.0] / [0.0, 0.0, -0.0, -0.0]
  ;; Expected result: [NaN, NaN, NaN, NaN]
  (func $test_zero_div_zero (export "test_zero_div_zero") (result v128)
    ;; Create dividend vector [0.0, -0.0, 0.0, -0.0]
    f32.const -0.0
    f32x4.splat
    f32.const 0.0
    f32x4.replace_lane 0
    f32.const -0.0
    f32x4.replace_lane 1
    f32.const 0.0
    f32x4.replace_lane 2

    ;; Create divisor vector [0.0, 0.0, -0.0, -0.0]
    f32.const -0.0
    f32x4.splat
    f32.const 0.0
    f32x4.replace_lane 0
    f32.const 0.0
    f32x4.replace_lane 1
    f32.const -0.0
    f32x4.replace_lane 2

    f32x4.div
  )

  ;; Test function for finite divided by infinity (should produce zero)
  ;; Tests: [42.0, -42.0, 100.0, -100.0] / [+inf, +inf, -inf, -inf]
  ;; Expected result: [0.0, 0.0, 0.0, 0.0]
  (func $test_finite_div_infinity (export "test_finite_div_infinity") (result v128)
    ;; Create dividend vector [42.0, -42.0, 100.0, -100.0]
    f32.const -100.0
    f32x4.splat
    f32.const 42.0
    f32x4.replace_lane 0
    f32.const -42.0
    f32x4.replace_lane 1
    f32.const 100.0
    f32x4.replace_lane 2

    ;; Create divisor vector [+inf, +inf, -inf, -inf]
    f32.const -inf
    f32x4.splat
    f32.const inf
    f32x4.replace_lane 0
    f32.const inf
    f32x4.replace_lane 1
    f32.const -inf
    f32x4.replace_lane 2

    f32x4.div
  )

  ;; Test function for NaN propagation in division
  ;; Tests various NaN scenarios: [NaN/finite, finite/NaN, NaN/NaN, -NaN/finite]
  ;; Expected result: [NaN, NaN, NaN, NaN]
  (func $test_nan_propagation (export "test_nan_propagation") (result v128)
    ;; Create dividend vector [NaN, 42.0, NaN, -NaN]
    f32.const -nan
    f32x4.splat
    f32.const nan
    f32x4.replace_lane 0
    f32.const 42.0
    f32x4.replace_lane 1
    f32.const nan
    f32x4.replace_lane 2

    ;; Create divisor vector [5.0, NaN, NaN, 7.0]
    f32.const 7.0
    f32x4.splat
    f32.const 5.0
    f32x4.replace_lane 0
    f32.const nan
    f32x4.replace_lane 1
    f32.const nan
    f32x4.replace_lane 2

    f32x4.div
  )

  ;; Test function for small fractional divisions
  ;; Tests: [0.5, 1.5, 2.5, 3.5] / [0.25, 0.75, 1.25, 1.75]
  ;; Expected result: [2.0, 2.0, 2.0, 2.0]
  (func $test_fractional_division (export "test_fractional_division") (result v128)
    ;; Create dividend vector [0.5, 1.5, 2.5, 3.5]
    f32.const 3.5
    f32x4.splat
    f32.const 0.5
    f32x4.replace_lane 0
    f32.const 1.5
    f32x4.replace_lane 1
    f32.const 2.5
    f32x4.replace_lane 2

    ;; Create divisor vector [0.25, 0.75, 1.25, 1.75]
    f32.const 1.75
    f32x4.splat
    f32.const 0.25
    f32x4.replace_lane 0
    f32.const 0.75
    f32x4.replace_lane 1
    f32.const 1.25
    f32x4.replace_lane 2

    f32x4.div
  )

  ;; Test function for boundary values division
  ;; Tests near floating-point limits
  ;; Uses large and small values to test boundary handling
  (func $test_boundary_division (export "test_boundary_division") (result v128)
    ;; Create dividend vector with large values
    f32.const 1.0e38
    f32x4.splat
    f32.const 3.4e38
    f32x4.replace_lane 0
    f32.const -3.4e38
    f32x4.replace_lane 1
    f32.const 1.0e38
    f32x4.replace_lane 2

    ;; Create divisor vector with smaller values
    f32.const 1.0e19
    f32x4.splat
    f32.const 1.0e19
    f32x4.replace_lane 0
    f32.const -1.0e19
    f32x4.replace_lane 1
    f32.const 2.0e19
    f32x4.replace_lane 2

    f32x4.div
  )

  ;; Test function for denormal number divisions
  ;; Tests very small floating-point values in denormal range
  (func $test_denormal_division (export "test_denormal_division") (result v128)
    ;; Create dividend vector with denormal values
    f32.const 2.8e-45
    f32x4.splat
    f32.const 1.4e-45     ;; Smallest positive denormal
    f32x4.replace_lane 0
    f32.const -1.4e-45    ;; Smallest negative denormal
    f32x4.replace_lane 1
    f32.const 4.2e-45
    f32x4.replace_lane 2

    ;; Create divisor vector
    f32.const 1.4e-45
    f32x4.splat
    f32.const 1.4e-45
    f32x4.replace_lane 0
    f32.const -1.4e-45
    f32x4.replace_lane 1
    f32.const 2.1e-45
    f32x4.replace_lane 2

    f32x4.div
  )
)