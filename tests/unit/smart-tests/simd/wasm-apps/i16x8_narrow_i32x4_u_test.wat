;; Test module for i16x8.narrow_i32x4_u SIMD instruction
;;
;; This module provides comprehensive test functions for validating the
;; i16x8.narrow_i32x4_u opcode which narrows two i32x4 vectors into
;; one i16x8 vector with unsigned saturation.
;;
;; Saturation behavior:
;; - Values < 0 saturate to 0
;; - Values > 65535 saturate to 65535
;; - Values in [0, 65535] convert directly
;;
;; Lane ordering: first vector → lanes 0-3, second vector → lanes 4-7

(module
  ;; Import memory for potential future extensions
  (memory 1)

  ;; Main test function: narrow_i32x4_u_test
  ;; Takes 8 i32 parameters representing two i32x4 vectors
  ;; Returns 8 i32 values representing the i16x8 result (extended to i32)
  ;;
  ;; Parameters:
  ;;   $v1_0, $v1_1, $v1_2, $v1_3: First i32x4 vector elements
  ;;   $v2_0, $v2_1, $v2_2, $v2_3: Second i32x4 vector elements
  ;;
  ;; Returns: 8 i32 values (i16 values zero-extended to i32)
  ;;   lanes 0-3 from first vector, lanes 4-7 from second vector
  (func (export "narrow_i32x4_u_test")
    (param $v1_0 i32) (param $v1_1 i32) (param $v1_2 i32) (param $v1_3 i32)
    (param $v2_0 i32) (param $v2_1 i32) (param $v2_2 i32) (param $v2_3 i32)
    (result i32) (result i32) (result i32) (result i32)
    (result i32) (result i32) (result i32) (result i32)

    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Build first i32x4 vector from parameters
    (local.set $vec1
      (i32x4.splat (local.get $v1_0)))
    (local.set $vec1
      (i32x4.replace_lane 1 (local.get $vec1) (local.get $v1_1)))
    (local.set $vec1
      (i32x4.replace_lane 2 (local.get $vec1) (local.get $v1_2)))
    (local.set $vec1
      (i32x4.replace_lane 3 (local.get $vec1) (local.get $v1_3)))

    ;; Build second i32x4 vector from parameters
    (local.set $vec2
      (i32x4.splat (local.get $v2_0)))
    (local.set $vec2
      (i32x4.replace_lane 1 (local.get $vec2) (local.get $v2_1)))
    (local.set $vec2
      (i32x4.replace_lane 2 (local.get $vec2) (local.get $v2_2)))
    (local.set $vec2
      (i32x4.replace_lane 3 (local.get $vec2) (local.get $v2_3)))

    ;; Perform i16x8.narrow_i32x4_u operation
    ;; Stack: vec1, vec2 → result_i16x8
    (local.set $result
      (i16x8.narrow_i32x4_u (local.get $vec1) (local.get $vec2)))

    ;; Extract i16 lanes and return as i32 (zero-extended)
    ;; Lane 0 (from vec1[0])
    (i32.const 0xFFFF)
    (i16x8.extract_lane_u 0 (local.get $result))
    i32.and

    ;; Lane 1 (from vec1[1])
    (i32.const 0xFFFF)
    (i16x8.extract_lane_u 1 (local.get $result))
    i32.and

    ;; Lane 2 (from vec1[2])
    (i32.const 0xFFFF)
    (i16x8.extract_lane_u 2 (local.get $result))
    i32.and

    ;; Lane 3 (from vec1[3])
    (i32.const 0xFFFF)
    (i16x8.extract_lane_u 3 (local.get $result))
    i32.and

    ;; Lane 4 (from vec2[0])
    (i32.const 0xFFFF)
    (i16x8.extract_lane_u 4 (local.get $result))
    i32.and

    ;; Lane 5 (from vec2[1])
    (i32.const 0xFFFF)
    (i16x8.extract_lane_u 5 (local.get $result))
    i32.and

    ;; Lane 6 (from vec2[2])
    (i32.const 0xFFFF)
    (i16x8.extract_lane_u 6 (local.get $result))
    i32.and

    ;; Lane 7 (from vec2[3])
    (i32.const 0xFFFF)
    (i16x8.extract_lane_u 7 (local.get $result))
    i32.and
  )

  ;; Helper function for testing specific saturation scenarios
  ;; Directly tests the narrow operation with predefined test vectors
  (func (export "test_saturation_limits")
    (result i32) (result i32) (result i32) (result i32)
    (result i32) (result i32) (result i32) (result i32)

    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Test vector 1: [0, 65535, 65536, -1]
    ;; Expected narrowed: [0, 65535, 65535, 0]
    (local.set $vec1
      (v128.const i32x4 0 65535 65536 -1))

    ;; Test vector 2: [100000, -50000, 1, 65535]
    ;; Expected narrowed: [65535, 0, 1, 65535]
    (local.set $vec2
      (v128.const i32x4 100000 -50000 1 65535))

    ;; Perform narrowing
    (local.set $result
      (i16x8.narrow_i32x4_u (local.get $vec1) (local.get $vec2)))

    ;; Return all 8 lanes as i32 values
    (i16x8.extract_lane_u 0 (local.get $result))
    (i16x8.extract_lane_u 1 (local.get $result))
    (i16x8.extract_lane_u 2 (local.get $result))
    (i16x8.extract_lane_u 3 (local.get $result))
    (i16x8.extract_lane_u 4 (local.get $result))
    (i16x8.extract_lane_u 5 (local.get $result))
    (i16x8.extract_lane_u 6 (local.get $result))
    (i16x8.extract_lane_u 7 (local.get $result))
  )

  ;; Function to test extreme value scenarios
  ;; Tests INT32_MIN, INT32_MAX, and other boundary cases
  (func (export "test_extreme_values")
    (result i32) (result i32) (result i32) (result i32)
    (result i32) (result i32) (result i32) (result i32)

    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Test vector 1: [INT32_MIN, 0, 65535, INT32_MAX]
    ;; Expected narrowed: [0, 0, 65535, 65535]
    (local.set $vec1
      (v128.const i32x4 0x80000000 0 65535 0x7FFFFFFF))

    ;; Test vector 2: [-1, 1, 65536, 2147483647]
    ;; Expected narrowed: [0, 1, 65535, 65535]
    (local.set $vec2
      (v128.const i32x4 0xFFFFFFFF 1 65536 2147483647))

    ;; Perform narrowing
    (local.set $result
      (i16x8.narrow_i32x4_u (local.get $vec1) (local.get $vec2)))

    ;; Return all 8 lanes
    (i16x8.extract_lane_u 0 (local.get $result))
    (i16x8.extract_lane_u 1 (local.get $result))
    (i16x8.extract_lane_u 2 (local.get $result))
    (i16x8.extract_lane_u 3 (local.get $result))
    (i16x8.extract_lane_u 4 (local.get $result))
    (i16x8.extract_lane_u 5 (local.get $result))
    (i16x8.extract_lane_u 6 (local.get $result))
    (i16x8.extract_lane_u 7 (local.get $result))
  )
)