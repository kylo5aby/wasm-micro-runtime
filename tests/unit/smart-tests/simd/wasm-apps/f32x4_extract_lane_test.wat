(module
  ;;
  ;; Test functions for f32x4.extract_lane opcode validation
  ;; Each function creates an f32x4 vector from 4 f32 inputs and extracts a specific lane
  ;;

  ;;
  ;; Test function: extract lane 0 from f32x4 vector
  ;;
  (func (export "test_extract_lane_0")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result f32)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Extract lane 0 from the vector
    (f32x4.extract_lane 0)
  )

  ;;
  ;; Test function: extract lane 1 from f32x4 vector
  ;;
  (func (export "test_extract_lane_1")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result f32)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Extract lane 1 from the vector
    (f32x4.extract_lane 1)
  )

  ;;
  ;; Test function: extract lane 2 from f32x4 vector
  ;;
  (func (export "test_extract_lane_2")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result f32)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Extract lane 2 from the vector
    (f32x4.extract_lane 2)
  )

  ;;
  ;; Test function: extract lane 3 from f32x4 vector
  ;;
  (func (export "test_extract_lane_3")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result f32)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Extract lane 3 from the vector
    (f32x4.extract_lane 3)
  )

  ;;
  ;; Test function: boundary values test - extract from vector with FLT_MAX
  ;;
  (func (export "test_boundary_flt_max_extract")
    (result f32)
    ;; Create f32x4 vector with FLT_MAX in lane 0
    (f32x4.splat (f32.const 3.4028235e+38))  ;; FLT_MAX approximation
    (f32.const 1.0)
    (f32x4.replace_lane 1)
    (f32.const 2.0)
    (f32x4.replace_lane 2)
    (f32.const 3.0)
    (f32x4.replace_lane 3)
    ;; Extract FLT_MAX from lane 0
    (f32x4.extract_lane 0)
  )

  ;;
  ;; Test function: boundary values test - extract from vector with FLT_MIN
  ;;
  (func (export "test_boundary_flt_min_extract")
    (result f32)
    ;; Create f32x4 vector with FLT_MIN in lane 1
    (f32x4.splat (f32.const 1.0))
    (f32.const 1.1754944e-38)  ;; FLT_MIN approximation
    (f32x4.replace_lane 1)
    (f32.const 2.0)
    (f32x4.replace_lane 2)
    (f32.const 3.0)
    (f32x4.replace_lane 3)
    ;; Extract FLT_MIN from lane 1
    (f32x4.extract_lane 1)
  )

  ;;
  ;; Test function: special values test - extract NaN
  ;;
  (func (export "test_special_nan_extract")
    (result f32)
    ;; Create f32x4 vector with NaN in lane 0
    (f32x4.splat (f32.const nan))
    (f32.const 1.0)
    (f32x4.replace_lane 1)
    (f32.const 2.0)
    (f32x4.replace_lane 2)
    (f32.const 3.0)
    (f32x4.replace_lane 3)
    ;; Extract NaN from lane 0
    (f32x4.extract_lane 0)
  )

  ;;
  ;; Test function: special values test - extract positive infinity
  ;;
  (func (export "test_special_pos_inf_extract")
    (result f32)
    ;; Create f32x4 vector with +infinity in lane 1
    (f32x4.splat (f32.const 1.0))
    (f32.const inf)
    (f32x4.replace_lane 1)
    (f32.const 2.0)
    (f32x4.replace_lane 2)
    (f32.const 3.0)
    (f32x4.replace_lane 3)
    ;; Extract +infinity from lane 1
    (f32x4.extract_lane 1)
  )

  ;;
  ;; Test function: special values test - extract negative infinity
  ;;
  (func (export "test_special_neg_inf_extract")
    (result f32)
    ;; Create f32x4 vector with -infinity in lane 2
    (f32x4.splat (f32.const 1.0))
    (f32.const 2.0)
    (f32x4.replace_lane 1)
    (f32.const -inf)
    (f32x4.replace_lane 2)
    (f32.const 3.0)
    (f32x4.replace_lane 3)
    ;; Extract -infinity from lane 2
    (f32x4.extract_lane 2)
  )

  ;;
  ;; Test function: special values test - extract negative zero
  ;;
  (func (export "test_special_neg_zero_extract")
    (result f32)
    ;; Create f32x4 vector with -0.0 in lane 3
    (f32x4.splat (f32.const 1.0))
    (f32.const 2.0)
    (f32x4.replace_lane 1)
    (f32.const 3.0)
    (f32x4.replace_lane 2)
    (f32.const -0.0)
    (f32x4.replace_lane 3)
    ;; Extract -0.0 from lane 3
    (f32x4.extract_lane 3)
  )

  ;;
  ;; Test function: all zeros vector - extract positive zero
  ;;
  (func (export "test_all_zeros_extract")
    (result f32)
    ;; Create f32x4 vector with all zeros [0.0, 0.0, 0.0, 0.0]
    (f32x4.splat (f32.const 0.0))
    ;; Extract 0.0 from lane 0
    (f32x4.extract_lane 0)
  )

  ;;
  ;; Test function: mixed zeros vector - extract negative zero from lane 1
  ;;
  (func (export "test_mixed_zeros_extract")
    (result f32)
    ;; Create f32x4 vector with mixed signed zeros [0.0, -0.0, 0.0, -0.0]
    (f32x4.splat (f32.const 0.0))
    (f32.const -0.0)
    (f32x4.replace_lane 1)
    (f32.const 0.0)
    (f32x4.replace_lane 2)
    (f32.const -0.0)
    (f32x4.replace_lane 3)
    ;; Extract -0.0 from lane 1
    (f32x4.extract_lane 1)
  )

  ;;
  ;; Test function: subnormal values test - smallest positive subnormal
  ;;
  (func (export "test_subnormal_smallest_extract")
    (result f32)
    ;; Create f32x4 vector with smallest positive subnormal in lane 0
    ;; Using hex representation for exact bit pattern: 0x00000001
    (v128.const i32x4 0x00000001 0x3f800000 0x40000000 0x40400000)  ;; [smallest_subnormal, 1.0, 2.0, 3.0]
    ;; Extract smallest subnormal from lane 0
    (f32x4.extract_lane 0)
  )

  ;;
  ;; Test function: subnormal values test - largest positive subnormal
  ;;
  (func (export "test_subnormal_largest_extract")
    (result f32)
    ;; Create f32x4 vector with largest positive subnormal in lane 1
    ;; Using hex representation for exact bit pattern: 0x007FFFFF
    (v128.const i32x4 0x3f800000 0x007FFFFF 0x40000000 0x40400000)  ;; [1.0, largest_subnormal, 2.0, 3.0]
    ;; Extract largest subnormal from lane 1
    (f32x4.extract_lane 1)
  )

  ;;
  ;; Test function: subnormal values test - smallest negative subnormal
  ;;
  (func (export "test_subnormal_neg_smallest_extract")
    (result f32)
    ;; Create f32x4 vector with smallest negative subnormal in lane 2
    ;; Using hex representation for exact bit pattern: 0x80000001
    (v128.const i32x4 0x3f800000 0x40000000 0x80000001 0x40400000)  ;; [1.0, 2.0, smallest_neg_subnormal, 3.0]
    ;; Extract smallest negative subnormal from lane 2
    (f32x4.extract_lane 2)
  )

  ;;
  ;; Test function: subnormal values test - largest negative subnormal
  ;;
  (func (export "test_subnormal_neg_largest_extract")
    (result f32)
    ;; Create f32x4 vector with largest negative subnormal in lane 3
    ;; Using hex representation for exact bit pattern: 0x807FFFFF
    (v128.const i32x4 0x3f800000 0x40000000 0x40400000 0x807FFFFF)  ;; [1.0, 2.0, 3.0, largest_neg_subnormal]
    ;; Extract largest negative subnormal from lane 3
    (f32x4.extract_lane 3)
  )

  ;;
  ;; Test function: comprehensive extraction test - extract distinct values from each lane
  ;;
  (func (export "test_comprehensive_extraction")
    (result f32)
    ;; Create f32x4 vector with distinct values [1.5, -2.3, 42.7, 100.0]
    (f32x4.splat (f32.const 1.5))
    (f32.const -2.3)
    (f32x4.replace_lane 1)
    (f32.const 42.7)
    (f32x4.replace_lane 2)
    (f32.const 100.0)
    (f32x4.replace_lane 3)
    ;; Extract value from lane 2 (should be 42.7)
    (f32x4.extract_lane 2)
  )

  ;;
  ;; Test function: zero mixed with non-zero values extraction
  ;;
  (func (export "test_zero_mixed_extract")
    (result f32)
    ;; Create f32x4 vector [1.5, 0.0, -2.3, -0.0]
    (f32x4.splat (f32.const 1.5))
    (f32.const 0.0)
    (f32x4.replace_lane 1)
    (f32.const -2.3)
    (f32x4.replace_lane 2)
    (f32.const -0.0)
    (f32x4.replace_lane 3)
    ;; Extract 0.0 from lane 1
    (f32x4.extract_lane 1)
  )
)