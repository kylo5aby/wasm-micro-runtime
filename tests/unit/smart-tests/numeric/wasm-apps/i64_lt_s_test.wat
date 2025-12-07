(module
  ;; i64.lt_s Test Module
  ;;
  ;; This module provides comprehensive test functions for the i64.lt_s WebAssembly opcode.
  ;; The i64.lt_s instruction performs signed 64-bit integer less-than comparison,
  ;; returning 1 (i32) if the first operand is less than the second operand, 0 otherwise.
  ;;
  ;; Functions exported:
  ;; - test_i64_lt_s: Basic signed less-than comparison operation
  ;; - test_i64_lt_s_multiple: Multiple comparisons in sequence
  ;;
  ;; Comparison logic:
  ;; - Pops two i64 values from stack (second operand first, then first operand)
  ;; - Performs signed comparison: first_operand < second_operand
  ;; - Pushes i32 result: 1 if true, 0 if false
  ;; - Handles signed arithmetic with proper two's complement interpretation

  ;; Basic i64.lt_s comparison test function
  ;; Takes two i64 inputs, applies i64.lt_s, and returns the result as i32
  ;; Tests the fundamental signed less-than comparison behavior
  (func (export "test_i64_lt_s") (param $left i64) (param $right i64) (result i32)
    ;; Load first operand (left side of comparison)
    local.get $left

    ;; Load second operand (right side of comparison)
    local.get $right

    ;; Apply i64.lt_s operation
    ;; This instruction:
    ;; 1. Pops $right from stack (second operand)
    ;; 2. Pops $left from stack (first operand)
    ;; 3. Performs signed comparison: $left < $right
    ;; 4. Pushes i32 result: 1 if $left < $right, 0 otherwise
    ;; 5. Uses signed two's complement arithmetic for comparison
    i64.lt_s

    ;; Return the comparison result as i32
  )

  ;; Multiple comparison test function for complex scenarios
  ;; Performs multiple i64.lt_s operations to test stack behavior and chaining
  ;; Tests: (a < b) AND (c < d) using logical operations
  (func (export "test_i64_lt_s_multiple") (param $a i64) (param $b i64) (param $c i64) (param $d i64) (result i32)
    ;; First comparison: a < b
    local.get $a
    local.get $b
    i64.lt_s

    ;; Second comparison: c < d
    local.get $c
    local.get $d
    i64.lt_s

    ;; Logical AND of both results
    ;; Both comparisons must be true (1) for final result to be true (1)
    i32.and

    ;; Return combined result
  )

  ;; Boundary value comparison test function
  ;; Tests comparison with extreme values and edge cases
  ;; Validates behavior at INT64_MAX and INT64_MIN boundaries
  (func (export "test_i64_lt_s_boundary") (param $input i64) (result i32)
    ;; Compare input with INT64_MAX (0x7FFFFFFFFFFFFFFF)
    local.get $input
    i64.const 0x7FFFFFFFFFFFFFFF
    i64.lt_s

    ;; If input < INT64_MAX, the result will be 1
    ;; Only INT64_MAX itself will return 0
  )

  ;; Zero comparison test function
  ;; Tests various comparisons involving zero value
  ;; Validates positive, negative, and zero boundary behavior
  (func (export "test_i64_lt_s_zero") (param $input i64) (result i32)
    ;; Compare input with zero
    local.get $input
    i64.const 0
    i64.lt_s

    ;; Returns 1 if input < 0 (negative), 0 if input >= 0
  )

  ;; Mathematical property test function
  ;; Tests anti-reflexive property: x < x should always be false
  ;; Validates that any value compared with itself returns 0
  (func (export "test_i64_lt_s_identity") (param $input i64) (result i32)
    ;; Compare input with itself
    local.get $input
    local.get $input
    i64.lt_s

    ;; Should always return 0 (false) for any input value
    ;; This validates the anti-reflexive property of less-than
  )
)