(module
  ;; i32.le_u test module for unsigned 32-bit integer less-than-or-equal comparison
  ;; This module provides test functions for the i32.le_u WebAssembly opcode
  ;; which performs unsigned less-than-or-equal comparison between two i32 values

  ;; Test function for basic i32.le_u opcode functionality
  ;; Parameters: two i32 values (a, b)
  ;; Returns: i32 result (1 if a <= b unsigned, 0 otherwise)
  (func $test_i32_le_u_basic (param $a i32) (param $b i32) (result i32)
    ;; Load first parameter (a) onto stack
    local.get $a
    ;; Load second parameter (b) onto stack
    local.get $b
    ;; Perform unsigned less-than-or-equal comparison: a <= b (unsigned)
    ;; Stack effect: [i32, i32] -> [i32]
    ;; Returns 1 if a <= b (treating both as unsigned 32-bit integers), 0 otherwise
    i32.le_u
  )

  ;; Export the basic test function to make it accessible from C++
  (export "test_i32_le_u_basic" (func $test_i32_le_u_basic))

  ;; Test function for boundary value comparisons (0, UINT32_MAX)
  ;; Handles extreme values at the limits of 32-bit unsigned integer range
  (func $test_i32_le_u_boundary (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_u
  )

  ;; Export boundary test function
  (export "test_i32_le_u_boundary" (func $test_i32_le_u_boundary))

  ;; Test function for signed vs unsigned semantics validation
  ;; Critical for testing cases where negative numbers become large unsigned values
  (func $test_i32_le_u_signed_semantics (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_u
  )

  ;; Export signed semantics validation function
  (export "test_i32_le_u_signed_semantics" (func $test_i32_le_u_signed_semantics))

  ;; Test function for zero operand comparisons
  ;; Handles all scenarios involving zero as minimum unsigned value
  (func $test_i32_le_u_zero (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_u
  )

  ;; Export zero test function
  (export "test_i32_le_u_zero" (func $test_i32_le_u_zero))

  ;; Test function for identity/reflexivity property validation (a <= a)
  ;; Validates mathematical property that any value is <= itself
  (func $test_i32_le_u_identity (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_u
  )

  ;; Export identity test function
  (export "test_i32_le_u_identity" (func $test_i32_le_u_identity))

  ;; Demonstration function showing i32.le_u in a conditional context
  ;; Shows practical usage of unsigned less-than-or-equal comparison in control flow
  (func $conditional_le_u_test (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.le_u
    if (result i32)
      ;; If a <= b (unsigned), return 100
      i32.const 100
    else
      ;; If a > b (unsigned), return 200
      i32.const 200
    end
  )

  ;; Export conditional test function
  (export "conditional_le_u_test" (func $conditional_le_u_test))

  ;; Comprehensive test function demonstrating unsigned comparison chaining
  ;; Shows transitivity: if (a <= b) AND (b <= c) then (a <= c) in unsigned context
  (func $complex_le_u_test (param $x i32) (param $y i32) (param $z i32) (result i32)
    (local $temp i32)

    ;; First comparison: x <= y (unsigned)
    local.get $x
    local.get $y
    i32.le_u
    local.set $temp

    ;; Second comparison: y <= z (unsigned)
    local.get $y
    local.get $z
    i32.le_u

    ;; Combine results with AND operation (both must be true)
    local.get $temp
    i32.and

    ;; Returns 1 if both (x <= y) AND (y <= z) in unsigned context, 0 otherwise
  )

  ;; Export complex test function
  (export "complex_le_u_test" (func $complex_le_u_test))

  ;; Edge case function for reflexivity testing (a <= a should always be true)
  (func $reflexivity_test (param $value i32) (result i32)
    local.get $value
    local.get $value
    i32.le_u
    ;; Should always return 1 for any value (reflexivity property)
  )

  ;; Export reflexivity test function
  (export "reflexivity_test" (func $reflexivity_test))

  ;; Specialized function for testing sign bit interpretation differences
  ;; Demonstrates how negative signed values become large unsigned values
  (func $sign_bit_interpretation_test (param $signed_neg i32) (param $positive i32) (result i32)
    ;; Compare negative number (large unsigned) with positive number
    local.get $signed_neg  ;; e.g., -1 becomes 0xFFFFFFFF = 4294967295
    local.get $positive    ;; e.g., 1000
    i32.le_u
    ;; Should return 0 because 4294967295 > 1000 in unsigned comparison
  )

  ;; Export sign bit interpretation test
  (export "sign_bit_interpretation_test" (func $sign_bit_interpretation_test))

  ;; Function to test adjacent value comparisons near boundaries
  ;; Tests behavior at critical unsigned boundary transitions
  (func $adjacent_value_test (param $base i32) (result i32)
    (local $incremented i32)

    ;; Calculate base + 1 (with potential wrap-around at UINT32_MAX)
    local.get $base
    i32.const 1
    i32.add
    local.set $incremented

    ;; Test base <= (base + 1), handling wrap-around case
    local.get $base
    local.get $incremented
    i32.le_u
    ;; Returns 1 except when base = UINT32_MAX (wrap-around case)
  )

  ;; Export adjacent value test function
  (export "adjacent_value_test" (func $adjacent_value_test))
)