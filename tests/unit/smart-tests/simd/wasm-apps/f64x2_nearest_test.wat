(module
  ;; Import memory for test data storage
  (memory 1)

  ;; Export memory for external access
  (export "memory" (memory 0))

  ;; Basic nearest rounding test function
  ;; Takes v128 parameter and returns v128 result of f64x2.nearest
  (func $test_basic_nearest (param $input v128) (result v128)
    ;; Apply f64x2.nearest directly to input v128
    local.get $input
    f64x2.nearest
  )
  (export "test_basic_nearest" (func $test_basic_nearest))

  ;; Banker's rounding test function
  ;; Tests "round half to even" behavior with exact half values
  (func $test_bankers_rounding (param $input v128) (result v128)
    ;; Apply f64x2.nearest directly to input v128
    local.get $input
    f64x2.nearest
  )
  (export "test_bankers_rounding" (func $test_bankers_rounding))

  ;; Boundary values test function
  ;; Tests DBL_MAX, DBL_MIN, and subnormal values
  (func $test_boundary_values (param $input v128) (result v128)
    ;; Apply f64x2.nearest directly to input v128
    local.get $input
    f64x2.nearest
  )
  (export "test_boundary_values" (func $test_boundary_values))

  ;; Special values test function
  ;; Tests ±infinity, ±zero, NaN values
  (func $test_special_values (param $input v128) (result v128)
    ;; Apply f64x2.nearest directly to input v128
    local.get $input
    f64x2.nearest
  )
  (export "test_special_values" (func $test_special_values))

  ;; Direct constant test functions for specific banker's rounding scenarios

  ;; Test exact half values - positive
  (func $f64x2_nearest_positive_half (result v128)
    ;; Store half values that should round to even
    (i32.const 64)
    (f64.const 0.5)  ;; Should round to 0.0 (even)
    f64.store

    (i32.const 72)
    (f64.const 1.5)  ;; Should round to 2.0 (even)
    f64.store

    (i32.const 64)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_positive_half" (func $f64x2_nearest_positive_half))

  ;; Test exact half values - more positive cases
  (func $f64x2_nearest_higher_positive_half (result v128)
    (i32.const 80)
    (f64.const 2.5)  ;; Should round to 2.0 (even)
    f64.store

    (i32.const 88)
    (f64.const 3.5)  ;; Should round to 4.0 (even)
    f64.store

    (i32.const 80)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_higher_positive_half" (func $f64x2_nearest_higher_positive_half))

  ;; Test exact half values - negative
  (func $f64x2_nearest_negative_half (result v128)
    (i32.const 96)
    (f64.const -0.5)  ;; Should round to 0.0 (even)
    f64.store

    (i32.const 104)
    (f64.const -1.5)  ;; Should round to -2.0 (even)
    f64.store

    (i32.const 96)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_negative_half" (func $f64x2_nearest_negative_half))

  ;; Test exact half values - more negative cases
  (func $f64x2_nearest_higher_negative_half (result v128)
    (i32.const 112)
    (f64.const -2.5)  ;; Should round to -2.0 (even)
    f64.store

    (i32.const 120)
    (f64.const -3.5)  ;; Should round to -4.0 (even)
    f64.store

    (i32.const 112)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_higher_negative_half" (func $f64x2_nearest_higher_negative_half))

  ;; Test basic fractional values
  (func $f64x2_nearest_basic_fractions (result v128)
    (i32.const 128)
    (f64.const 1.4)  ;; Should round to 1.0
    f64.store

    (i32.const 136)
    (f64.const 1.6)  ;; Should round to 2.0
    f64.store

    (i32.const 128)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_basic_fractions" (func $f64x2_nearest_basic_fractions))

  ;; Test negative fractional values
  (func $f64x2_nearest_negative_fractions (result v128)
    (i32.const 144)
    (f64.const -1.4)  ;; Should round to -1.0
    f64.store

    (i32.const 152)
    (f64.const -1.6)  ;; Should round to -2.0
    f64.store

    (i32.const 144)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_negative_fractions" (func $f64x2_nearest_negative_fractions))

  ;; Test positive zero
  (func $f64x2_nearest_pos_zero (result v128)
    (i32.const 160)
    (f64.const 0.0)
    f64.store

    (i32.const 168)
    (f64.const 0.0)
    f64.store

    (i32.const 160)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_pos_zero" (func $f64x2_nearest_pos_zero))

  ;; Test negative zero
  (func $f64x2_nearest_neg_zero (result v128)
    (i32.const 176)
    (f64.const -0.0)
    f64.store

    (i32.const 184)
    (f64.const -0.0)
    f64.store

    (i32.const 176)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_neg_zero" (func $f64x2_nearest_neg_zero))

  ;; Test positive infinity
  (func $f64x2_nearest_pos_inf (result v128)
    (i32.const 192)
    (f64.const inf)
    f64.store

    (i32.const 200)
    (f64.const inf)
    f64.store

    (i32.const 192)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_pos_inf" (func $f64x2_nearest_pos_inf))

  ;; Test negative infinity
  (func $f64x2_nearest_neg_inf (result v128)
    (i32.const 208)
    (f64.const -inf)
    f64.store

    (i32.const 216)
    (f64.const -inf)
    f64.store

    (i32.const 208)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_neg_inf" (func $f64x2_nearest_neg_inf))

  ;; Test NaN (not a number)
  (func $f64x2_nearest_nan (result v128)
    (i32.const 224)
    (f64.const nan)
    f64.store

    (i32.const 232)
    (f64.const nan)
    f64.store

    (i32.const 224)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_nan" (func $f64x2_nearest_nan))

  ;; Test DBL_MAX equivalent (largest finite f64)
  (func $f64x2_nearest_max_positive (result v128)
    (i32.const 240)
    (f64.const 1.7976931348623157e+308)
    f64.store

    (i32.const 248)
    (f64.const 1.7976931348623157e+308)
    f64.store

    (i32.const 240)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_max_positive" (func $f64x2_nearest_max_positive))

  ;; Test -DBL_MAX
  (func $f64x2_nearest_max_negative (result v128)
    (i32.const 256)
    (f64.const -1.7976931348623157e+308)
    f64.store

    (i32.const 264)
    (f64.const -1.7976931348623157e+308)
    f64.store

    (i32.const 256)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_max_negative" (func $f64x2_nearest_max_negative))

  ;; Test DBL_MIN equivalent (smallest normal positive)
  (func $f64x2_nearest_min_positive (result v128)
    (i32.const 272)
    (f64.const 2.2250738585072014e-308)
    f64.store

    (i32.const 280)
    (f64.const 2.2250738585072014e-308)
    f64.store

    (i32.const 272)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_min_positive" (func $f64x2_nearest_min_positive))

  ;; Test -DBL_MIN
  (func $f64x2_nearest_min_negative (result v128)
    (i32.const 288)
    (f64.const -2.2250738585072014e-308)
    f64.store

    (i32.const 296)
    (f64.const -2.2250738585072014e-308)
    f64.store

    (i32.const 288)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_min_negative" (func $f64x2_nearest_min_negative))

  ;; Test very small subnormal values
  (func $f64x2_nearest_subnormal (result v128)
    (i32.const 304)
    (f64.const 5.0e-324)  ;; Smallest positive subnormal
    f64.store

    (i32.const 312)
    (f64.const -5.0e-324)  ;; Smallest negative subnormal
    f64.store

    (i32.const 304)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_subnormal" (func $f64x2_nearest_subnormal))

  ;; Test large integer values (2^53 precision limit)
  (func $f64x2_nearest_large_integers (result v128)
    (i32.const 320)
    (f64.const 9007199254740992.0)  ;; 2^53 - exactly representable
    f64.store

    (i32.const 328)
    (f64.const -9007199254740992.0)  ;; -2^53
    f64.store

    (i32.const 320)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_large_integers" (func $f64x2_nearest_large_integers))

  ;; Test integer identity (values already integers)
  (func $f64x2_nearest_integer_identity (result v128)
    (i32.const 336)
    (f64.const 42.0)  ;; Should remain 42.0
    f64.store

    (i32.const 344)
    (f64.const -17.0)  ;; Should remain -17.0
    f64.store

    (i32.const 336)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_integer_identity" (func $f64x2_nearest_integer_identity))

  ;; Test mixed scenarios (one half value, one fraction)
  (func $f64x2_nearest_mixed_scenarios (result v128)
    (i32.const 352)
    (f64.const 4.5)  ;; Should round to 4.0 (even)
    f64.store

    (i32.const 360)
    (f64.const 3.7)  ;; Should round to 4.0
    f64.store

    (i32.const 352)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_mixed_scenarios" (func $f64x2_nearest_mixed_scenarios))

  ;; Test edge cases around precision limit
  (func $f64x2_nearest_precision_edge (result v128)
    (i32.const 368)
    ;; Value slightly less than 2^53 + 0.5
    (f64.const 9007199254740991.5)  ;; Should round to 9007199254740992.0
    f64.store

    (i32.const 376)
    ;; Value exactly 2^53 + 1
    (f64.const 9007199254740993.0)  ;; Should remain as is
    f64.store

    (i32.const 368)
    v128.load
    f64x2.nearest
  )
  (export "f64x2_nearest_precision_edge" (func $f64x2_nearest_precision_edge))
)