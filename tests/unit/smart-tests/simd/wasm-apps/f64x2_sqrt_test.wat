(module
  ;; Import memory for test data
  (memory 1)

  ;; Export memory for external access
  (export "memory" (memory 0))

  ;; Helper function to convert f64 pair to v128 and apply sqrt
  (func $f64x2_sqrt_helper (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32) (result i32 i32 i32 i32)
    ;; Store f64 values in memory using i32 pairs
    (i32.const 0)
    local.get $val1_lo
    i32.store

    (i32.const 4)
    local.get $val1_hi
    i32.store

    (i32.const 8)
    local.get $val2_lo
    i32.store

    (i32.const 12)
    local.get $val2_hi
    i32.store

    ;; Load as v128, apply f64x2.sqrt, and store result
    (i32.const 16)  ;; destination address
    (i32.const 0)   ;; source address
    v128.load
    f64x2.sqrt
    v128.store

    ;; Return 4 i32 values
    (i32.const 16) i32.load  ;; result_lo_lane0
    (i32.const 20) i32.load  ;; result_hi_lane0
    (i32.const 24) i32.load  ;; result_lo_lane1
    (i32.const 28) i32.load  ;; result_hi_lane1
  )

  ;; Basic square root test function
  ;; Takes two f64 parameters as i32 pairs and returns v128 as 4 i32 values
  (func $test_basic_sqrt (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32) (result i32 i32 i32 i32)
    local.get $val1_lo
    local.get $val1_hi
    local.get $val2_lo
    local.get $val2_hi
    call $f64x2_sqrt_helper
  )
  (export "test_basic_sqrt" (func $test_basic_sqrt))

  ;; Boundary values test function
  ;; Tests DBL_MAX, DBL_MIN, and subnormal values
  (func $test_boundary_values (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32) (result i32 i32 i32 i32)
    local.get $val1_lo
    local.get $val1_hi
    local.get $val2_lo
    local.get $val2_hi
    call $f64x2_sqrt_helper
  )
  (export "test_boundary_values" (func $test_boundary_values))

  ;; Special values test function
  ;; Tests ±infinity, ±zero, NaN values
  (func $test_special_values (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32) (result i32 i32 i32 i32)
    local.get $val1_lo
    local.get $val1_hi
    local.get $val2_lo
    local.get $val2_hi
    call $f64x2_sqrt_helper
  )
  (export "test_special_values" (func $test_special_values))

  ;; Negative values test function
  ;; Tests negative values that should produce NaN results
  (func $test_negative_values (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32) (result i32 i32 i32 i32)
    local.get $val1_lo
    local.get $val1_hi
    local.get $val2_lo
    local.get $val2_hi
    call $f64x2_sqrt_helper
  )
  (export "test_negative_values" (func $test_negative_values))
)