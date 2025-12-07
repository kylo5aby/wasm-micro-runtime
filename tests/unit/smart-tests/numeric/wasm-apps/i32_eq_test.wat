;; WebAssembly Text Format (.wat) for i32.eq Opcode Testing
;; Comprehensive test module for 32-bit integer equality comparison operation
;;
;; This module provides multiple test functions to validate i32.eq behavior
;; across various scenarios including boundary values, bit patterns, and edge cases.

(module
  ;; Primary test function for i32.eq opcode
  ;; Takes two i32 parameters and returns 1 if equal, 0 if not equal
  (func (export "test_i32_eq") (param $operand1 i32) (param $operand2 i32) (result i32)
    ;; Load first operand onto stack
    local.get $operand1
    ;; Load second operand onto stack
    local.get $operand2
    ;; Execute i32.eq opcode
    ;; Stack before: [..., operand1, operand2]
    ;; Stack after:  [..., result] (1 if operand1 == operand2, 0 otherwise)
    i32.eq
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for zero equality verification
  ;; Validates that zero equals zero (identity property)
  (func (export "test_zero_equality") (result i32)
    i32.const 0
    i32.const 0
    i32.eq  ;; Should return 1
  )

  ;; Test function for boundary value equality (INT32_MAX)
  ;; Tests equality comparison at maximum 32-bit signed integer
  (func (export "test_max_boundary") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.eq                ;; Should return 1
  )

  ;; Test function for boundary value equality (INT32_MIN)
  ;; Tests equality comparison at minimum 32-bit signed integer
  (func (export "test_min_boundary") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.eq                ;; Should return 1
  )

  ;; Test function for bit pattern equality (all ones)
  ;; Validates equality with maximum unsigned value (0xFFFFFFFF)
  (func (export "test_all_ones") (result i32)
    i32.const 0xFFFFFFFF  ;; All bits set (-1 in signed, UINT32_MAX in unsigned)
    i32.const 0xFFFFFFFF  ;; All bits set
    i32.eq                ;; Should return 1
  )

  ;; Test function for alternating bit pattern equality
  ;; Tests equality with alternating bit patterns
  (func (export "test_alternating_bits") (result i32)
    i32.const 0xAAAAAAAA  ;; Alternating pattern (10101010...)
    i32.const 0xAAAAAAAA  ;; Same alternating pattern
    i32.eq                ;; Should return 1
  )

  ;; Test function for inequality verification
  ;; Validates that different values produce false result
  (func (export "test_inequality") (result i32)
    i32.const 42
    i32.const 24
    i32.eq  ;; Should return 0 (false)
  )

  ;; Test function for signed/unsigned boundary equality
  ;; Tests that same bit patterns compare equal regardless of interpretation
  (func (export "test_sign_boundary") (result i32)
    i32.const -1          ;; -1 in signed representation
    i32.const 0xFFFFFFFF  ;; Same bit pattern as unsigned
    i32.eq                ;; Should return 1 (same bit pattern)
  )

  ;; Test function for reflexive property validation
  ;; Verifies that any value equals itself (mathematical property)
  (func (export "test_reflexive_property") (param $value i32) (result i32)
    local.get $value
    local.get $value
    i32.eq  ;; Should always return 1 for any input
  )

  ;; Test function for mixed sign equality (should be false)
  ;; Tests that same magnitude with different signs are not equal
  (func (export "test_mixed_signs") (result i32)
    i32.const 100
    i32.const -100
    i32.eq  ;; Should return 0 (different values)
  )

  ;; Test function for single bit difference
  ;; Validates that values differing by one bit are not equal
  (func (export "test_single_bit_diff") (result i32)
    i32.const 0x00000001  ;; Only LSB set
    i32.const 0x80000000  ;; Only MSB set (sign bit)
    i32.eq                ;; Should return 0 (different values)
  )
)