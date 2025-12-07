;; WebAssembly Text Format (.wat) for i32.eqz Opcode Testing
;; Comprehensive test module for 32-bit integer "equal to zero" test operation
;;
;; This module provides multiple test functions to validate i32.eqz behavior
;; across various scenarios including boundary values, bit patterns, and edge cases.

(module
  ;; Primary test function for i32.eqz opcode
  ;; Takes one i32 parameter and returns 1 if zero, 0 if non-zero
  (func (export "test_i32_eqz") (param $operand i32) (result i32)
    ;; Load operand onto stack
    local.get $operand
    ;; Execute i32.eqz opcode
    ;; Stack before: [..., operand]
    ;; Stack after:  [..., result] (1 if operand == 0, 0 otherwise)
    i32.eqz
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for zero value verification
  ;; Validates that zero returns true (1)
  (func (export "test_zero_value") (result i32)
    i32.const 0
    i32.eqz  ;; Should return 1
  )

  ;; Test function for positive non-zero value
  ;; Validates that positive values return false (0)
  (func (export "test_positive_nonzero") (result i32)
    i32.const 42
    i32.eqz  ;; Should return 0
  )

  ;; Test function for negative non-zero value
  ;; Validates that negative values return false (0)
  (func (export "test_negative_nonzero") (result i32)
    i32.const -42
    i32.eqz  ;; Should return 0
  )

  ;; Test function for boundary value (INT32_MAX)
  ;; Tests that maximum 32-bit signed integer returns false
  (func (export "test_max_boundary") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.eqz               ;; Should return 0
  )

  ;; Test function for boundary value (INT32_MIN)
  ;; Tests that minimum 32-bit signed integer returns false
  (func (export "test_min_boundary") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.eqz               ;; Should return 0
  )

  ;; Test function for all ones bit pattern
  ;; Validates that 0xFFFFFFFF (-1) returns false
  (func (export "test_all_ones") (result i32)
    i32.const 0xFFFFFFFF  ;; All bits set (-1 in signed)
    i32.eqz               ;; Should return 0
  )

  ;; Test function for alternating bit pattern
  ;; Tests that alternating bit patterns return false
  (func (export "test_alternating_bits") (result i32)
    i32.const 0xAAAAAAAA  ;; Alternating pattern (10101010...)
    i32.eqz               ;; Should return 0
  )

  ;; Test function for single bit set (LSB)
  ;; Validates that minimal non-zero value returns false
  (func (export "test_single_bit_lsb") (result i32)
    i32.const 0x00000001  ;; Only LSB set
    i32.eqz               ;; Should return 0
  )

  ;; Test function for single bit set (MSB/sign bit)
  ;; Tests that sign bit only returns false
  (func (export "test_single_bit_msb") (result i32)
    i32.const 0x80000000  ;; Only MSB set (sign bit)
    i32.eqz               ;; Should return 0
  )

  ;; Test function for small positive values
  ;; Tests edge case around zero boundary
  (func (export "test_small_positive") (result i32)
    i32.const 1
    i32.eqz  ;; Should return 0
  )

  ;; Test function for small negative values
  ;; Tests edge case around zero boundary (negative side)
  (func (export "test_small_negative") (result i32)
    i32.const -1
    i32.eqz  ;; Should return 0
  )

  ;; Test function for power of two values
  ;; Tests various powers of two (common bit patterns)
  (func (export "test_power_of_two") (result i32)
    i32.const 1024  ;; 2^10
    i32.eqz         ;; Should return 0
  )

  ;; Test function for large positive value
  ;; Tests behavior with large positive integers
  (func (export "test_large_positive") (result i32)
    i32.const 1000000
    i32.eqz  ;; Should return 0
  )

  ;; Test function for large negative value
  ;; Tests behavior with large negative integers
  (func (export "test_large_negative") (result i32)
    i32.const -1000000
    i32.eqz  ;; Should return 0
  )

  ;; Test function for boundary near zero (positive)
  ;; Tests values just above zero
  (func (export "test_near_zero_positive") (result i32)
    i32.const 2
    i32.eqz  ;; Should return 0
  )

  ;; Test function for boundary near zero (negative)
  ;; Tests values just below zero
  (func (export "test_near_zero_negative") (result i32)
    i32.const -2
    i32.eqz  ;; Should return 0
  )
)