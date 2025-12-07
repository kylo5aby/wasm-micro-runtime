(module
  ;; Basic floor test function
  ;; Takes four i32 parameters representing two f64 values and returns four i32 representing v128 result
  (func $test_basic_floor (export "test_basic_floor")
    (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32)
    (result i32 i32 i32 i32)
    (local $vector v128)
    (local $result v128)

    ;; Create v128 vector from two f64 values passed as i32 pairs
    (v128.const i32x4 0 0 0 0)
    (local.get $val1_lo)
    (i32x4.replace_lane 0)
    (local.get $val1_hi)
    (i32x4.replace_lane 1)
    (local.get $val2_lo)
    (i32x4.replace_lane 2)
    (local.get $val2_hi)
    (i32x4.replace_lane 3)
    (local.set $vector)

    ;; Apply f64x2.floor
    (local.get $vector)
    f64x2.floor
    (local.set $result)

    ;; Extract result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Boundary values test function
  ;; Tests large magnitude values, exact integers, and precision boundaries
  (func $test_boundary_values (export "test_boundary_values")
    (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32)
    (result i32 i32 i32 i32)
    (local $vector v128)
    (local $result v128)

    ;; Create v128 vector from two f64 values passed as i32 pairs
    (v128.const i32x4 0 0 0 0)
    (local.get $val1_lo)
    (i32x4.replace_lane 0)
    (local.get $val1_hi)
    (i32x4.replace_lane 1)
    (local.get $val2_lo)
    (i32x4.replace_lane 2)
    (local.get $val2_hi)
    (i32x4.replace_lane 3)
    (local.set $vector)

    ;; Apply f64x2.floor
    (local.get $vector)
    f64x2.floor
    (local.set $result)

    ;; Extract result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Special values test function
  ;; Tests ±infinity, ±zero, NaN values
  (func $test_special_values (export "test_special_values")
    (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32)
    (result i32 i32 i32 i32)
    (local $vector v128)
    (local $result v128)

    ;; Create v128 vector from two f64 values passed as i32 pairs
    (v128.const i32x4 0 0 0 0)
    (local.get $val1_lo)
    (i32x4.replace_lane 0)
    (local.get $val1_hi)
    (i32x4.replace_lane 1)
    (local.get $val2_lo)
    (i32x4.replace_lane 2)
    (local.get $val2_hi)
    (i32x4.replace_lane 3)
    (local.set $vector)

    ;; Apply f64x2.floor
    (local.get $vector)
    f64x2.floor
    (local.set $result)

    ;; Extract result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Zero values test function
  ;; Tests ±zero, near-zero values, and sign preservation
  (func $test_zero_values (export "test_zero_values")
    (param $val1_lo i32) (param $val1_hi i32) (param $val2_lo i32) (param $val2_hi i32)
    (result i32 i32 i32 i32)
    (local $vector v128)
    (local $result v128)

    ;; Create v128 vector from two f64 values passed as i32 pairs
    (v128.const i32x4 0 0 0 0)
    (local.get $val1_lo)
    (i32x4.replace_lane 0)
    (local.get $val1_hi)
    (i32x4.replace_lane 1)
    (local.get $val2_lo)
    (i32x4.replace_lane 2)
    (local.get $val2_hi)
    (i32x4.replace_lane 3)
    (local.set $vector)

    ;; Apply f64x2.floor
    (local.get $vector)
    f64x2.floor
    (local.set $result)

    ;; Extract result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)
    (local.get $result)
    (i32x4.extract_lane 1)
    (local.get $result)
    (i32x4.extract_lane 2)
    (local.get $result)
    (i32x4.extract_lane 3)
  )
)