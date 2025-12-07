(module
  ;; Test functions for f32x4.abs opcode validation

  ;; Function to test f32x4.abs with four input f32 values
  ;; Parameters: four f32 values to be packed into f32x4 vector
  ;; Returns: v128 result of f32x4.abs operation
  (func $test_f32x4_abs (export "test_f32x4_abs")
    (param $lane0 f32) (param $lane1 f32) (param $lane2 f32) (param $lane3 f32)
    (result v128)

    ;; Create f32x4 vector from input parameters
    local.get $lane3
    f32x4.splat
    local.get $lane0
    f32x4.replace_lane 0
    local.get $lane1
    f32x4.replace_lane 1
    local.get $lane2
    f32x4.replace_lane 2

    ;; Apply f32x4.abs operation
    f32x4.abs
  )

  ;; Test function for basic absolute value functionality
  ;; Tests mixed positive and negative values: [1.5, -2.7, 3.14, -0.5]
  ;; Expected result: [1.5, 2.7, 3.14, 0.5]
  (func $test_basic_abs (export "test_basic_abs") (result v128)
    f32.const -0.5
    f32x4.splat
    f32.const 1.5
    f32x4.replace_lane 0
    f32.const -2.7
    f32x4.replace_lane 1
    f32.const 3.14
    f32x4.replace_lane 2
    f32x4.abs
  )

  ;; Test function for special IEEE 754 values
  ;; Tests zero values: [+0.0, -0.0, +0.0, -0.0]
  ;; Expected result: [+0.0, +0.0, +0.0, +0.0]
  (func $test_zero_values (export "test_zero_values") (result v128)
    f32.const -0.0
    f32x4.splat
    f32.const 0.0
    f32x4.replace_lane 0
    f32.const -0.0
    f32x4.replace_lane 1
    f32.const 0.0
    f32x4.replace_lane 2
    f32x4.abs
  )

  ;; Test function for infinity values
  ;; Tests: [+inf, -inf, +inf, -inf]
  ;; Expected result: [+inf, +inf, +inf, +inf]
  (func $test_infinity_values (export "test_infinity_values") (result v128)
    f32.const -inf
    f32x4.splat
    f32.const inf
    f32x4.replace_lane 0
    f32.const -inf
    f32x4.replace_lane 1
    f32.const inf
    f32x4.replace_lane 2
    f32x4.abs
  )

  ;; Test function for NaN values
  ;; Tests: [nan, -nan, nan, -nan]
  ;; Expected result: [nan, nan, nan, nan] (NaN preservation)
  (func $test_nan_values (export "test_nan_values") (result v128)
    f32.const -nan
    f32x4.splat
    f32.const nan
    f32x4.replace_lane 0
    f32.const -nan
    f32x4.replace_lane 1
    f32.const nan
    f32x4.replace_lane 2
    f32x4.abs
  )

  ;; Test function for boundary values
  ;; Tests extreme floating-point values near limits
  ;; Uses large positive and negative values to test boundary handling
  (func $test_boundary_values (export "test_boundary_values") (result v128)
    f32.const -3.4028235e+38  ;; Near -FLT_MAX
    f32x4.splat
    f32.const 1.1754944e-38   ;; Near FLT_MIN
    f32x4.replace_lane 0
    f32.const -1.1754944e-38  ;; Near -FLT_MIN
    f32x4.replace_lane 1
    f32.const 3.4028235e+38   ;; Near FLT_MAX
    f32x4.replace_lane 2
    f32x4.abs
  )

  ;; Test function for denormal (subnormal) values
  ;; Tests very small floating-point values in denormal range
  ;; These values test proper handling of denormalized numbers
  (func $test_denormal_values (export "test_denormal_values") (result v128)
    f32.const -1.175e-38      ;; Near smallest normal (negative)
    f32x4.splat
    f32.const 1.4e-45         ;; Smallest positive denormal
    f32x4.replace_lane 0
    f32.const -1.4e-45        ;; Smallest negative denormal
    f32x4.replace_lane 1
    f32.const 1.175e-38       ;; Near smallest normal
    f32x4.replace_lane 2
    f32x4.abs
  )

  ;; Test function for various decimal values
  ;; Tests typical decimal numbers with different magnitudes
  (func $test_decimal_values (export "test_decimal_values") (result v128)
    f32.const -789.012
    f32x4.splat
    f32.const 0.1
    f32x4.replace_lane 0
    f32.const -0.1
    f32x4.replace_lane 1
    f32.const 123.456
    f32x4.replace_lane 2
    f32x4.abs
  )
)