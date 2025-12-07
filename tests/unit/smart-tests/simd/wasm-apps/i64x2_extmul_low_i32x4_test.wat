(module
  ;; Test module for i64x2.extmul_low_i32x4 opcode validation
  ;; This module provides comprehensive test functions for extended multiplication
  ;; of low i32 lanes from i32x4 vectors, producing i64x2 results

  ;; Function: test_i64x2_extmul_low_i32x4_basic
  ;; Purpose: Basic functionality test with typical positive values
  ;; Parameters: Two i32x4 vectors (8 i32 values total)
  ;; Returns: i64x2 result (2 i64 values)
  (func $test_i64x2_extmul_low_i32x4_basic (export "test_i64x2_extmul_low_i32x4_basic")
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (result i64 i64)
    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Create first i32x4 vector from individual lane parameters
    (local.set $vec1
      (i32x4.splat (local.get $vec1_lane0)))
    (local.set $vec1
      (i32x4.replace_lane 1 (local.get $vec1) (local.get $vec1_lane1)))
    (local.set $vec1
      (i32x4.replace_lane 2 (local.get $vec1) (local.get $vec1_lane2)))
    (local.set $vec1
      (i32x4.replace_lane 3 (local.get $vec1) (local.get $vec1_lane3)))

    ;; Create second i32x4 vector from individual lane parameters
    (local.set $vec2
      (i32x4.splat (local.get $vec2_lane0)))
    (local.set $vec2
      (i32x4.replace_lane 1 (local.get $vec2) (local.get $vec2_lane1)))
    (local.set $vec2
      (i32x4.replace_lane 2 (local.get $vec2) (local.get $vec2_lane2)))
    (local.set $vec2
      (i32x4.replace_lane 3 (local.get $vec2) (local.get $vec2_lane3)))

    ;; Perform i64x2.extmul_low_i32x4 operation
    ;; This extracts lanes 0 and 1 from both vectors, sign-extends to i64, and multiplies
    (local.set $result
      (i64x2.extmul_low_i32x4_s (local.get $vec1) (local.get $vec2)))

    ;; Extract and return both i64 lanes
    (i64x2.extract_lane 0 (local.get $result))
    (i64x2.extract_lane 1 (local.get $result))
  )

  ;; Function: test_i64x2_extmul_low_i32x4_boundary
  ;; Purpose: Boundary values test with INT32_MIN/MAX
  ;; Parameters: Two i32x4 vectors with extreme values in low lanes
  ;; Returns: i64x2 result with proper boundary handling
  (func $test_i64x2_extmul_low_i32x4_boundary (export "test_i64x2_extmul_low_i32x4_boundary")
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (result i64 i64)
    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Build first vector with boundary values
    (local.set $vec1
      (i32x4.splat (local.get $vec1_lane0)))
    (local.set $vec1
      (i32x4.replace_lane 1 (local.get $vec1) (local.get $vec1_lane1)))
    (local.set $vec1
      (i32x4.replace_lane 2 (local.get $vec1) (local.get $vec1_lane2)))
    (local.set $vec1
      (i32x4.replace_lane 3 (local.get $vec1) (local.get $vec1_lane3)))

    ;; Build second vector with boundary test multipliers
    (local.set $vec2
      (i32x4.splat (local.get $vec2_lane0)))
    (local.set $vec2
      (i32x4.replace_lane 1 (local.get $vec2) (local.get $vec2_lane1)))
    (local.set $vec2
      (i32x4.replace_lane 2 (local.get $vec2) (local.get $vec2_lane2)))
    (local.set $vec2
      (i32x4.replace_lane 3 (local.get $vec2) (local.get $vec2_lane3)))

    ;; Execute extended multiplication with sign extension
    (local.set $result
      (i64x2.extmul_low_i32x4_s (local.get $vec1) (local.get $vec2)))

    ;; Return both result lanes for boundary validation
    (i64x2.extract_lane 0 (local.get $result))
    (i64x2.extract_lane 1 (local.get $result))
  )

  ;; Function: test_i64x2_extmul_low_i32x4_sign
  ;; Purpose: Sign extension test with mixed positive/negative values
  ;; Parameters: Two i32x4 vectors with mixed sign values in low lanes
  ;; Returns: i64x2 result demonstrating proper sign preservation
  (func $test_i64x2_extmul_low_i32x4_sign (export "test_i64x2_extmul_low_i32x4_sign")
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (result i64 i64)
    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Construct first vector with negative values in low lanes
    (local.set $vec1
      (i32x4.splat (local.get $vec1_lane0)))
    (local.set $vec1
      (i32x4.replace_lane 1 (local.get $vec1) (local.get $vec1_lane1)))
    (local.set $vec1
      (i32x4.replace_lane 2 (local.get $vec1) (local.get $vec1_lane2)))
    (local.set $vec1
      (i32x4.replace_lane 3 (local.get $vec1) (local.get $vec1_lane3)))

    ;; Construct second vector with mixed sign multipliers
    (local.set $vec2
      (i32x4.splat (local.get $vec2_lane0)))
    (local.set $vec2
      (i32x4.replace_lane 1 (local.get $vec2) (local.get $vec2_lane1)))
    (local.set $vec2
      (i32x4.replace_lane 2 (local.get $vec2) (local.get $vec2_lane2)))
    (local.set $vec2
      (i32x4.replace_lane 3 (local.get $vec2) (local.get $vec2_lane3)))

    ;; Perform sign-preserving extended multiplication
    (local.set $result
      (i64x2.extmul_low_i32x4_s (local.get $vec1) (local.get $vec2)))

    ;; Extract results for sign validation
    (i64x2.extract_lane 0 (local.get $result))
    (i64x2.extract_lane 1 (local.get $result))
  )

  ;; Function: test_i64x2_extmul_low_i32x4_zero
  ;; Purpose: Zero handling test to validate zero multiplication property
  ;; Parameters: Two i32x4 vectors with zero values in low lanes
  ;; Returns: i64x2 result demonstrating zero multiplication behavior
  (func $test_i64x2_extmul_low_i32x4_zero (export "test_i64x2_extmul_low_i32x4_zero")
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (result i64 i64)
    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Create first vector with zero in lane 0
    (local.set $vec1
      (i32x4.splat (local.get $vec1_lane0)))
    (local.set $vec1
      (i32x4.replace_lane 1 (local.get $vec1) (local.get $vec1_lane1)))
    (local.set $vec1
      (i32x4.replace_lane 2 (local.get $vec1) (local.get $vec1_lane2)))
    (local.set $vec1
      (i32x4.replace_lane 3 (local.get $vec1) (local.get $vec1_lane3)))

    ;; Create second vector with zero in lane 1
    (local.set $vec2
      (i32x4.splat (local.get $vec2_lane0)))
    (local.set $vec2
      (i32x4.replace_lane 1 (local.get $vec2) (local.get $vec2_lane1)))
    (local.set $vec2
      (i32x4.replace_lane 2 (local.get $vec2) (local.get $vec2_lane2)))
    (local.set $vec2
      (i32x4.replace_lane 3 (local.get $vec2) (local.get $vec2_lane3)))

    ;; Execute extended multiplication with zero values
    (local.set $result
      (i64x2.extmul_low_i32x4_s (local.get $vec1) (local.get $vec2)))

    ;; Return results to validate zero multiplication property
    (i64x2.extract_lane 0 (local.get $result))
    (i64x2.extract_lane 1 (local.get $result))
  )
)