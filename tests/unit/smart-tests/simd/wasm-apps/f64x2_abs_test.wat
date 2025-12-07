(module
  ;; Import memory for test data if needed
  (memory 1)

  ;; Export memory for external access
  (export "memory" (memory 0))

  ;; Basic absolute value test function
  ;; Takes two f64 parameters and returns v128 result of f64x2.abs
  (func $test_basic_abs (param $val1 f64) (param $val2 f64) (result v128)
    ;; Create v128 vector from two f64 values using v128.const
    ;; We need to store values in memory and load as v128
    (i32.const 0)  ;; Memory offset
    local.get $val1
    f64.store      ;; Store first f64 at offset 0

    (i32.const 8)  ;; Memory offset for second f64
    local.get $val2
    f64.store      ;; Store second f64 at offset 8

    ;; Load as v128 and apply f64x2.abs
    (i32.const 0)
    v128.load
    f64x2.abs
  )
  (export "test_basic_abs" (func $test_basic_abs))

  ;; Boundary values test function
  ;; Tests DBL_MAX, DBL_MIN, and subnormal values
  (func $test_boundary_values (param $val1 f64) (param $val2 f64) (result v128)
    ;; Store values in memory and load as v128
    (i32.const 16)  ;; Different memory offset
    local.get $val1
    f64.store

    (i32.const 24)
    local.get $val2
    f64.store

    (i32.const 16)
    v128.load
    f64x2.abs
  )
  (export "test_boundary_values" (func $test_boundary_values))

  ;; Special values test function
  ;; Tests ±infinity, ±zero, NaN values
  (func $test_special_values (param $val1 f64) (param $val2 f64) (result v128)
    ;; Store values in memory and load as v128
    (i32.const 32)  ;; Different memory offset
    local.get $val1
    f64.store

    (i32.const 40)
    local.get $val2
    f64.store

    (i32.const 32)
    v128.load
    f64x2.abs
  )
  (export "test_special_values" (func $test_special_values))

  ;; Mixed scenarios test function
  ;; Tests combinations of different value types
  (func $test_mixed_scenarios (param $val1 f64) (param $val2 f64) (result v128)
    ;; Store values in memory and load as v128
    (i32.const 48)  ;; Different memory offset
    local.get $val1
    f64.store

    (i32.const 56)
    local.get $val2
    f64.store

    (i32.const 48)
    v128.load
    f64x2.abs
  )
  (export "test_mixed_scenarios" (func $test_mixed_scenarios))

  ;; Direct constant test functions for specific scenarios

  ;; Test with positive constant values
  (func $f64x2_abs_positive_const (result v128)
    ;; Store constants in memory
    (i32.const 64)
    (f64.const 1.5)
    f64.store

    (i32.const 72)
    (f64.const 2.5)
    f64.store

    (i32.const 64)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_positive_const" (func $f64x2_abs_positive_const))

  ;; Test with negative constant values
  (func $f64x2_abs_negative_const (result v128)
    (i32.const 80)
    (f64.const -1.5)
    f64.store

    (i32.const 88)
    (f64.const -2.5)
    f64.store

    (i32.const 80)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_negative_const" (func $f64x2_abs_negative_const))

  ;; Test with mixed sign constant values
  (func $f64x2_abs_mixed_const (result v128)
    (i32.const 96)
    (f64.const 3.14159)
    f64.store

    (i32.const 104)
    (f64.const -2.71828)
    f64.store

    (i32.const 96)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_mixed_const" (func $f64x2_abs_mixed_const))

  ;; Test with positive zero
  (func $f64x2_abs_pos_zero (result v128)
    (i32.const 112)
    (f64.const 0.0)
    f64.store

    (i32.const 120)
    (f64.const 0.0)
    f64.store

    (i32.const 112)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_pos_zero" (func $f64x2_abs_pos_zero))

  ;; Test with negative zero
  (func $f64x2_abs_neg_zero (result v128)
    (i32.const 128)
    (f64.const -0.0)
    f64.store

    (i32.const 136)
    (f64.const -0.0)
    f64.store

    (i32.const 128)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_neg_zero" (func $f64x2_abs_neg_zero))

  ;; Test with positive infinity
  (func $f64x2_abs_pos_inf (result v128)
    (i32.const 144)
    (f64.const inf)
    f64.store

    (i32.const 152)
    (f64.const inf)
    f64.store

    (i32.const 144)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_pos_inf" (func $f64x2_abs_pos_inf))

  ;; Test with negative infinity
  (func $f64x2_abs_neg_inf (result v128)
    (i32.const 160)
    (f64.const -inf)
    f64.store

    (i32.const 168)
    (f64.const -inf)
    f64.store

    (i32.const 160)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_neg_inf" (func $f64x2_abs_neg_inf))

  ;; Test with NaN (not a number)
  (func $f64x2_abs_nan (result v128)
    (i32.const 176)
    (f64.const nan)
    f64.store

    (i32.const 184)
    (f64.const nan)
    f64.store

    (i32.const 176)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_nan" (func $f64x2_abs_nan))

  ;; Test with DBL_MAX equivalent
  (func $f64x2_abs_max_positive (result v128)
    (i32.const 192)
    (f64.const 1.7976931348623157e+308)
    f64.store

    (i32.const 200)
    (f64.const 1.7976931348623157e+308)
    f64.store

    (i32.const 192)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_max_positive" (func $f64x2_abs_max_positive))

  ;; Test with -DBL_MAX
  (func $f64x2_abs_max_negative (result v128)
    (i32.const 208)
    (f64.const -1.7976931348623157e+308)
    f64.store

    (i32.const 216)
    (f64.const -1.7976931348623157e+308)
    f64.store

    (i32.const 208)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_max_negative" (func $f64x2_abs_max_negative))

  ;; Test with DBL_MIN equivalent (smallest normal positive)
  (func $f64x2_abs_min_positive (result v128)
    (i32.const 224)
    (f64.const 2.2250738585072014e-308)
    f64.store

    (i32.const 232)
    (f64.const 2.2250738585072014e-308)
    f64.store

    (i32.const 224)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_min_positive" (func $f64x2_abs_min_positive))

  ;; Test with -DBL_MIN
  (func $f64x2_abs_min_negative (result v128)
    (i32.const 240)
    (f64.const -2.2250738585072014e-308)
    f64.store

    (i32.const 248)
    (f64.const -2.2250738585072014e-308)
    f64.store

    (i32.const 240)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_min_negative" (func $f64x2_abs_min_negative))

  ;; Test with very small subnormal value
  (func $f64x2_abs_subnormal (result v128)
    (i32.const 256)
    (f64.const 5.0e-324)
    f64.store

    (i32.const 264)
    (f64.const -5.0e-324)
    f64.store

    (i32.const 256)
    v128.load
    f64x2.abs
  )
  (export "f64x2_abs_subnormal" (func $f64x2_abs_subnormal))

  ;; Test idempotent property: abs(abs(x)) = abs(x)
  (func $f64x2_abs_idempotent (param $val1 f64) (param $val2 f64) (result v128)
    ;; Store values in memory and load as v128
    (i32.const 272)
    local.get $val1
    f64.store

    (i32.const 280)
    local.get $val2
    f64.store

    ;; Load, apply abs twice
    (i32.const 272)
    v128.load
    f64x2.abs
    f64x2.abs  ;; Apply abs twice
  )
  (export "f64x2_abs_idempotent" (func $f64x2_abs_idempotent))
)