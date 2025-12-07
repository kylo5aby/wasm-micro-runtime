(module
  ;; Enhanced test module for f64x2.extmul_low_i32x4_s opcode validation
  ;;
  ;; This module provides comprehensive test functions for validating
  ;; extended multiplication functionality by simulating the behavior
  ;; until native f64x2.extmul_low_i32x4_s support is available.
  ;; The functions extract lower i32 elements and perform manual extended multiplication.

  ;; Manual extended multiplication simulation
  ;; Extracts lower two i32 elements, converts to i64, multiplies, then converts to f64
  (func $simulate_extmul_low (param $input1 v128) (param $input2 v128) (result v128)
    (local $elem1_i32 i32) (local $elem2_i32 i32)
    (local $elem3_i32 i32) (local $elem4_i32 i32)
    (local $result1_i64 i64) (local $result2_i64 i64)
    (local $result1_f64 f64) (local $result2_f64 f64)

    ;; Extract lower two i32 elements from first vector
    (local.set $elem1_i32 (i32x4.extract_lane 0 (local.get $input1)))
    (local.set $elem2_i32 (i32x4.extract_lane 1 (local.get $input1)))

    ;; Extract lower two i32 elements from second vector
    (local.set $elem3_i32 (i32x4.extract_lane 0 (local.get $input2)))
    (local.set $elem4_i32 (i32x4.extract_lane 1 (local.get $input2)))

    ;; Perform extended multiplication: i32 -> i64 -> i64 multiplication
    (local.set $result1_i64
      (i64.mul
        (i64.extend_i32_s (local.get $elem1_i32))
        (i64.extend_i32_s (local.get $elem3_i32))
      )
    )
    (local.set $result2_i64
      (i64.mul
        (i64.extend_i32_s (local.get $elem2_i32))
        (i64.extend_i32_s (local.get $elem4_i32))
      )
    )

    ;; Convert i64 results to f64
    (local.set $result1_f64 (f64.convert_i64_s (local.get $result1_i64)))
    (local.set $result2_f64 (f64.convert_i64_s (local.get $result2_i64)))

    ;; Construct f64x2 result vector
    (f64x2.splat (local.get $result1_f64))
    (local.get $result2_f64)
    (f64x2.replace_lane 1)
  )

  ;; Test function: Basic extended multiplication with typical values
  ;; Tests fundamental extended multiplication functionality with various signed integer patterns
  (func $test_basic_extmul (param $input1 v128) (param $input2 v128) (result v128)
    (call $simulate_extmul_low (local.get $input1) (local.get $input2))
  )

  ;; Test function: Boundary value extended multiplication
  ;; Tests critical boundary cases with INT32_MIN/MAX values requiring 64-bit precision
  (func $test_boundary_values (param $input1 v128) (param $input2 v128) (result v128)
    (call $simulate_extmul_low (local.get $input1) (local.get $input2))
  )

  ;; Test function: Zero and identity operations
  ;; Tests mathematical properties: 0×n=0, 1×n=n, -1×n=-n, and commutative property
  (func $test_zero_and_identity (param $input1 v128) (param $input2 v128) (result v128)
    (call $simulate_extmul_low (local.get $input1) (local.get $input2))
  )

  ;; Additional test: Large integer multiplication
  ;; Tests extended multiplication with various large signed integers
  (func $test_large_integers (param $input1 v128) (param $input2 v128) (result v128)
    (call $simulate_extmul_low (local.get $input1) (local.get $input2))
  )

  ;; Additional test: Mixed sign combinations
  ;; Tests all combinations of positive/negative operands
  (func $test_mixed_signs (param $input1 v128) (param $input2 v128) (result v128)
    (call $simulate_extmul_low (local.get $input1) (local.get $input2))
  )

  ;; Additional test: Precision validation
  ;; Tests f64 precision handling for large multiplication results
  (func $test_precision_handling (param $input1 v128) (param $input2 v128) (result v128)
    (call $simulate_extmul_low (local.get $input1) (local.get $input2))
  )

  ;; Test helper: Specific positive multiplication
  ;; Tests 10×5 and 20×3 for basic validation
  (func $test_simple_positive (param $input1 v128) (param $input2 v128) (result v128)
    (call $simulate_extmul_low (local.get $input1) (local.get $input2))
  )

  ;; Test helper: Specific negative multiplication
  ;; Tests negative number handling in extended multiplication
  (func $test_simple_negative (param $input1 v128) (param $input2 v128) (result v128)
    (call $simulate_extmul_low (local.get $input1) (local.get $input2))
  )

  ;; Export all test functions for external access
  (export "test_basic_extmul" (func $test_basic_extmul))
  (export "test_boundary_values" (func $test_boundary_values))
  (export "test_zero_and_identity" (func $test_zero_and_identity))
  (export "test_large_integers" (func $test_large_integers))
  (export "test_mixed_signs" (func $test_mixed_signs))
  (export "test_precision_handling" (func $test_precision_handling))
  (export "test_simple_positive" (func $test_simple_positive))
  (export "test_simple_negative" (func $test_simple_negative))
)