(module
  ;; Test globals for comprehensive global.get testing

  ;; Basic global variables for main routine tests
  (global $g_i32 i32 (i32.const 42))
  (global $g_i64 i64 (i64.const 1000))
  (global $g_f32 f32 (f32.const 3.14))
  (global $g_f64 f64 (f64.const 2.718281828459045))

  ;; Boundary value globals for corner case tests
  (global $g_i32_min i32 (i32.const -2147483648))  ;; INT32_MIN
  (global $g_i32_max i32 (i32.const 2147483647))   ;; INT32_MAX
  (global $g_i64_min i64 (i64.const -9223372036854775808))  ;; INT64_MIN
  (global $g_i64_max i64 (i64.const 9223372036854775807))   ;; INT64_MAX

  ;; Extreme floating-point boundary values
  (global $g_f32_max f32 (f32.const 0x1.fffffep+127))  ;; FLT_MAX
  (global $g_f64_max f64 (f64.const 0x1.fffffffffffffp+1023))  ;; DBL_MAX

  ;; Special IEEE 754 floating-point values for edge case tests
  (global $g_f32_nan f32 (f32.const nan))
  (global $g_f32_inf f32 (f32.const inf))
  (global $g_f32_neg_inf f32 (f32.const -inf))
  (global $g_f32_pos_zero f32 (f32.const 0.0))
  (global $g_f32_neg_zero f32 (f32.const -0.0))

  (global $g_f64_nan f64 (f64.const nan))
  (global $g_f64_inf f64 (f64.const inf))
  (global $g_f64_neg_inf f64 (f64.const -inf))

  ;; Mutability test globals
  (global $g_const i32 (i32.const 100))        ;; Immutable global
  (global $g_mut (mut i32) (i32.const 200))    ;; Mutable global

  ;; Sequential indexing test globals
  (global $g_index_0 i32 (i32.const 1))
  (global $g_index_1 i32 (i32.const 2))
  (global $g_index_2 i32 (i32.const 3))

  ;; Basic global access functions
  (func $get_i32_global (result i32)
    global.get $g_i32
  )

  (func $get_i64_global (result i64)
    global.get $g_i64
  )

  (func $get_f32_global (result f32)
    global.get $g_f32
  )

  (func $get_f64_global (result f64)
    global.get $g_f64
  )

  ;; Boundary value access functions
  (func $get_i32_min_global (result i32)
    global.get $g_i32_min
  )

  (func $get_i32_max_global (result i32)
    global.get $g_i32_max
  )

  (func $get_i64_min_global (result i64)
    global.get $g_i64_min
  )

  (func $get_i64_max_global (result i64)
    global.get $g_i64_max
  )

  (func $get_f32_max_global (result f32)
    global.get $g_f32_max
  )

  (func $get_f64_max_global (result f64)
    global.get $g_f64_max
  )

  ;; Special floating-point value access functions
  (func $get_f32_nan_global (result f32)
    global.get $g_f32_nan
  )

  (func $get_f32_inf_global (result f32)
    global.get $g_f32_inf
  )

  (func $get_f32_neg_inf_global (result f32)
    global.get $g_f32_neg_inf
  )

  (func $get_f32_pos_zero_global (result f32)
    global.get $g_f32_pos_zero
  )

  (func $get_f32_neg_zero_global (result f32)
    global.get $g_f32_neg_zero
  )

  (func $get_f64_nan_global (result f64)
    global.get $g_f64_nan
  )

  (func $get_f64_inf_global (result f64)
    global.get $g_f64_inf
  )

  (func $get_f64_neg_inf_global (result f64)
    global.get $g_f64_neg_inf
  )

  ;; Mutability test functions
  (func $get_const_global (result i32)
    global.get $g_const
  )

  (func $get_mut_global (result i32)
    global.get $g_mut
  )

  ;; Sequential indexing test functions
  (func $get_global_index_0 (result i32)
    global.get $g_index_0
  )

  (func $get_global_index_1 (result i32)
    global.get $g_index_1
  )

  (func $get_global_index_2 (result i32)
    global.get $g_index_2
  )

  ;; Export all test functions
  (export "get_i32_global" (func $get_i32_global))
  (export "get_i64_global" (func $get_i64_global))
  (export "get_f32_global" (func $get_f32_global))
  (export "get_f64_global" (func $get_f64_global))

  (export "get_i32_min_global" (func $get_i32_min_global))
  (export "get_i32_max_global" (func $get_i32_max_global))
  (export "get_i64_min_global" (func $get_i64_min_global))
  (export "get_i64_max_global" (func $get_i64_max_global))
  (export "get_f32_max_global" (func $get_f32_max_global))
  (export "get_f64_max_global" (func $get_f64_max_global))

  (export "get_f32_nan_global" (func $get_f32_nan_global))
  (export "get_f32_inf_global" (func $get_f32_inf_global))
  (export "get_f32_neg_inf_global" (func $get_f32_neg_inf_global))
  (export "get_f32_pos_zero_global" (func $get_f32_pos_zero_global))
  (export "get_f32_neg_zero_global" (func $get_f32_neg_zero_global))

  (export "get_f64_nan_global" (func $get_f64_nan_global))
  (export "get_f64_inf_global" (func $get_f64_inf_global))
  (export "get_f64_neg_inf_global" (func $get_f64_neg_inf_global))

  (export "get_const_global" (func $get_const_global))
  (export "get_mut_global" (func $get_mut_global))

  (export "get_global_index_0" (func $get_global_index_0))
  (export "get_global_index_1" (func $get_global_index_1))
  (export "get_global_index_2" (func $get_global_index_2))
)