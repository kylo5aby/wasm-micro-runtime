(module
  ;; Import memory for test data if needed
  (memory 1)

  ;; Export memory for external access
  (export "memory" (memory 0))

  ;; Main test function for i8x16.splat operation
  ;; Takes an i32 parameter and returns v128 result of i8x16.splat
  (func $i8x16_splat_test (param $value i32) (result v128)
    local.get $value
    i8x16.splat
  )
  (export "i8x16_splat_test" (func $i8x16_splat_test))

  ;; Test function for basic splat with immediate value
  (func $i8x16_splat_const (result v128)
    i32.const 42
    i8x16.splat
  )
  (export "i8x16_splat_const" (func $i8x16_splat_const))

  ;; Test function for boundary value (i8 max)
  (func $i8x16_splat_max (result v128)
    i32.const 127  ;; INT8_MAX
    i8x16.splat
  )
  (export "i8x16_splat_max" (func $i8x16_splat_max))

  ;; Test function for boundary value (i8 min)
  (func $i8x16_splat_min (result v128)
    i32.const -128  ;; INT8_MIN
    i8x16.splat
  )
  (export "i8x16_splat_min" (func $i8x16_splat_min))

  ;; Test function for zero value
  (func $i8x16_splat_zero (result v128)
    i32.const 0
    i8x16.splat
  )
  (export "i8x16_splat_zero" (func $i8x16_splat_zero))

  ;; Test function for truncation behavior (255 -> -1)
  (func $i8x16_splat_truncate_255 (result v128)
    i32.const 255
    i8x16.splat
  )
  (export "i8x16_splat_truncate_255" (func $i8x16_splat_truncate_255))

  ;; Test function for truncation behavior (256 -> 0)
  (func $i8x16_splat_truncate_256 (result v128)
    i32.const 256
    i8x16.splat
  )
  (export "i8x16_splat_truncate_256" (func $i8x16_splat_truncate_256))

  ;; Test function for extreme value truncation (INT32_MAX)
  (func $i8x16_splat_extreme_max (result v128)
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i8x16.splat
  )
  (export "i8x16_splat_extreme_max" (func $i8x16_splat_extreme_max))

  ;; Test function for extreme value truncation (INT32_MIN)
  (func $i8x16_splat_extreme_min (result v128)
    i32.const 0x80000000  ;; INT32_MIN
    i8x16.splat
  )
  (export "i8x16_splat_extreme_min" (func $i8x16_splat_extreme_min))

  ;; Test function for negative one value
  (func $i8x16_splat_neg_one (result v128)
    i32.const -1
    i8x16.splat
  )
  (export "i8x16_splat_neg_one" (func $i8x16_splat_neg_one))
)