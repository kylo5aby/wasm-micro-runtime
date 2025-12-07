(module
  ;;
  ;; Test functions for f64x2.extract_lane opcode validation
  ;; Each function creates an f64x2 vector from 2 f64 inputs and extracts a specific lane
  ;;

  ;;
  ;; Core extraction functions - used by C++ test harness
  ;;
  (func (export "extract_f64x2_lane_0")
    (param $v0 f64) (param $v1 f64)
    (result f64)
    ;; Build f64x2 vector from input parameters
    (f64x2.splat (local.get $v0))
    (local.get $v1)
    (f64x2.replace_lane 1)
    ;; Extract lane 0 from the vector
    (f64x2.extract_lane 0)
  )

  (func (export "extract_f64x2_lane_1")
    (param $v0 f64) (param $v1 f64)
    (result f64)
    ;; Build f64x2 vector from input parameters
    (f64x2.splat (local.get $v0))
    (local.get $v1)
    (f64x2.replace_lane 1)
    ;; Extract lane 1 from the vector
    (f64x2.extract_lane 1)
  )

  ;;
  ;; Comprehensive test functions for various value categories
  ;;

  ;;
  ;; Test function: Basic values extraction - typical double precision values
  ;;
  (func (export "test_basic_values")
    (result f64)
    ;; Create f64x2 vector with typical values (3.14159, -2.71828)
    (f64x2.splat (f64.const 3.14159265358979323846))
    (f64.const -2.718281828459045)
    (f64x2.replace_lane 1)
    ;; Extract lane 0 (should return 3.14159265358979323846)
    (f64x2.extract_lane 0)
  )

  ;;
  ;; Test function: Maximum boundary values - DBL_MAX
  ;;
  (func (export "test_boundary_dbl_max_extract")
    (result f64)
    ;; Create f64x2 vector with DBL_MAX in lane 0
    (f64x2.splat (f64.const 1.7976931348623157e+308))  ;; DBL_MAX
    (f64.const 1.0)
    (f64x2.replace_lane 1)
    ;; Extract DBL_MAX from lane 0
    (f64x2.extract_lane 0)
  )

  ;;
  ;; Test function: Minimum normal values - DBL_MIN
  ;;
  (func (export "test_boundary_dbl_min_extract")
    (result f64)
    ;; Create f64x2 vector with DBL_MIN in lane 1
    (f64x2.splat (f64.const 42.0))
    (f64.const 2.2250738585072014e-308)  ;; DBL_MIN
    (f64x2.replace_lane 1)
    ;; Extract DBL_MIN from lane 1
    (f64x2.extract_lane 1)
  )

  ;;
  ;; Test function: Signed zero extraction - positive zero
  ;;
  (func (export "test_signed_zero_positive")
    (result f64)
    ;; Create f64x2 vector with +0.0 in lane 0
    (f64x2.splat (f64.const 0.0))
    (f64.const -1.0)
    (f64x2.replace_lane 1)
    ;; Extract +0.0 from lane 0
    (f64x2.extract_lane 0)
  )

  ;;
  ;; Test function: Signed zero extraction - negative zero
  ;;
  (func (export "test_signed_zero_negative")
    (result f64)
    ;; Create f64x2 vector with -0.0 in lane 1
    (f64x2.splat (f64.const 1.0))
    (f64.const -0.0)
    (f64x2.replace_lane 1)
    ;; Extract -0.0 from lane 1
    (f64x2.extract_lane 1)
  )

  ;;
  ;; Test function: Infinity extraction - positive infinity
  ;;
  (func (export "test_infinity_positive")
    (result f64)
    ;; Create f64x2 vector with +infinity in lane 0
    (f64x2.splat (f64.const inf))
    (f64.const 123.456)
    (f64x2.replace_lane 1)
    ;; Extract +infinity from lane 0
    (f64x2.extract_lane 0)
  )

  ;;
  ;; Test function: Infinity extraction - negative infinity
  ;;
  (func (export "test_infinity_negative")
    (result f64)
    ;; Create f64x2 vector with -infinity in lane 1
    (f64x2.splat (f64.const 789.123))
    (f64.const -inf)
    (f64x2.replace_lane 1)
    ;; Extract -infinity from lane 1
    (f64x2.extract_lane 1)
  )

  ;;
  ;; Test function: NaN extraction - quiet NaN
  ;;
  (func (export "test_nan_quiet")
    (result f64)
    ;; Create f64x2 vector with quiet NaN in lane 0
    (f64x2.splat (f64.const nan))
    (f64.const 42.0)
    (f64x2.replace_lane 1)
    ;; Extract NaN from lane 0
    (f64x2.extract_lane 0)
  )

  ;;
  ;; Test function: Power-of-two values extraction
  ;;
  (func (export "test_power_of_two")
    (result f64)
    ;; Create f64x2 vector with power-of-two values (1.0, 0.5)
    (f64x2.splat (f64.const 1.0))
    (f64.const 0.5)
    (f64x2.replace_lane 1)
    ;; Extract 0.5 from lane 1
    (f64x2.extract_lane 1)
  )

  ;;
  ;; Test function: Scientific notation values
  ;;
  (func (export "test_scientific_notation")
    (result f64)
    ;; Create f64x2 vector with scientific notation values
    (f64x2.splat (f64.const 1.23456789e+100))
    (f64.const -4.56789123e-200)
    (f64x2.replace_lane 1)
    ;; Extract scientific value from lane 0
    (f64x2.extract_lane 0)
  )

  ;;
  ;; Test function: Cross-lane independence verification
  ;;
  (func (export "test_cross_lane_independence")
    (result f64)
    ;; Create f64x2 vector with very different magnitudes
    (f64x2.splat (f64.const 1e-100))  ;; Very small positive
    (f64.const 1e100)                 ;; Very large positive
    (f64x2.replace_lane 1)
    ;; Extract small value from lane 0
    (f64x2.extract_lane 0)
  )

  ;;
  ;; Test function: Identical values in both lanes
  ;;
  (func (export "test_identical_lanes")
    (result f64)
    ;; Create f64x2 vector with identical values
    (f64x2.splat (f64.const 987.654321))
    ;; No need to replace lane 1 since splat creates identical values
    ;; Extract from lane 1 to verify it's the same
    (f64x2.extract_lane 1)
  )

  ;;
  ;; Test function: Subnormal number extraction
  ;;
  (func (export "test_subnormal_extraction")
    (result f64)
    ;; Create f64x2 vector with a very small subnormal value
    ;; Using minimum positive subnormal (approximately 5e-324)
    (f64x2.splat (f64.const 4.9406564584124654e-324))
    (f64.const 1.0)
    (f64x2.replace_lane 1)
    ;; Extract subnormal from lane 0
    (f64x2.extract_lane 0)
  )

  ;;
  ;; Test function: Mixed special and normal values
  ;;
  (func (export "test_mixed_special_normal")
    (result f64)
    ;; Create f64x2 vector with NaN in lane 0, normal value in lane 1
    (f64x2.splat (f64.const nan))
    (f64.const 42.123456789)
    (f64x2.replace_lane 1)
    ;; Extract normal value from lane 1
    (f64x2.extract_lane 1)
  )

  ;;
  ;; Test function: Opposite sign values
  ;;
  (func (export "test_opposite_signs")
    (result f64)
    ;; Create f64x2 vector with opposite signs
    (f64x2.splat (f64.const 123.456789))
    (f64.const -123.456789)
    (f64x2.replace_lane 1)
    ;; Extract negative value from lane 1
    (f64x2.extract_lane 1)
  )

  ;;
  ;; Test function: High precision values near boundaries
  ;;
  (func (export "test_high_precision_boundaries")
    (result f64)
    ;; Create f64x2 vector with high precision value near maximum
    (f64x2.splat (f64.const 1.7976931348623155e+308))  ;; Near DBL_MAX but not quite
    (f64.const 2.2250738585072016e-308)                ;; Near DBL_MIN but slightly larger
    (f64x2.replace_lane 1)
    ;; Extract near-boundary value from lane 0
    (f64x2.extract_lane 0)
  )
)