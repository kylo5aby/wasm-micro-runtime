;; WebAssembly Text Format (.wat) for i32.ne Opcode Testing
;; Comprehensive test module for 32-bit integer inequality comparison operation
;;
;; This module provides multiple test functions to validate i32.ne behavior
;; across various scenarios including boundary values, bit patterns, and edge cases.

(module
  ;; Primary test function for i32.ne opcode
  ;; Takes two i32 parameters and returns 1 if not equal, 0 if equal
  (func (export "test_i32_ne") (param $operand1 i32) (param $operand2 i32) (result i32)
    ;; Load first operand onto stack
    local.get $operand1
    ;; Load second operand onto stack
    local.get $operand2
    ;; Execute i32.ne opcode
    ;; Stack before: [..., operand1, operand2]
    ;; Stack after:  [..., result] (1 if operand1 != operand2, 0 otherwise)
    i32.ne
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for zero inequality verification
  ;; Validates that zero equals zero (identity property returns false for ne)
  (func (export "test_zero_inequality") (result i32)
    i32.const 0
    i32.const 0
    i32.ne  ;; Should return 0 (zero equals zero)
  )

  ;; Test function for boundary value inequality (INT32_MAX)
  ;; Tests inequality comparison at maximum 32-bit signed integer
  (func (export "test_max_boundary") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.ne                ;; Should return 0 (same values)
  )

  ;; Test function for boundary value inequality (INT32_MIN)
  ;; Tests inequality comparison at minimum 32-bit signed integer
  (func (export "test_min_boundary") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.ne                ;; Should return 0 (same values)
  )

  ;; Test function for bit pattern inequality (all ones)
  ;; Validates inequality with maximum unsigned value (0xFFFFFFFF)
  (func (export "test_all_ones") (result i32)
    i32.const 0xFFFFFFFF  ;; All bits set (-1 in signed, UINT32_MAX in unsigned)
    i32.const 0xFFFFFFFF  ;; All bits set
    i32.ne                ;; Should return 0 (same bit patterns)
  )

  ;; Test function for alternating bit pattern inequality
  ;; Tests inequality with alternating bit patterns
  (func (export "test_alternating_bits") (result i32)
    i32.const 0xAAAAAAAA  ;; Alternating pattern (10101010...)
    i32.const 0xAAAAAAAA  ;; Same alternating pattern
    i32.ne                ;; Should return 0 (same patterns)
  )

  ;; Test function for inequality verification
  ;; Validates that different values produce true result
  (func (export "test_inequality") (result i32)
    i32.const 42
    i32.const 24
    i32.ne  ;; Should return 1 (different values)
  )

  ;; Test function for signed/unsigned boundary inequality
  ;; Tests that same bit patterns compare equal (ne returns false)
  (func (export "test_sign_boundary") (result i32)
    i32.const -1          ;; -1 in signed representation
    i32.const 0xFFFFFFFF  ;; Same bit pattern as unsigned
    i32.ne                ;; Should return 0 (same bit pattern)
  )

  ;; Test function for reflexive property validation
  ;; Verifies that any value equals itself (ne should return false)
  (func (export "test_reflexive_property") (param $value i32) (result i32)
    local.get $value
    local.get $value
    i32.ne  ;; Should always return 0 for any input (value == value)
  )

  ;; Test function for mixed sign inequality (should be true)
  ;; Tests that same magnitude with different signs are not equal
  (func (export "test_mixed_signs") (result i32)
    i32.const 100
    i32.const -100
    i32.ne  ;; Should return 1 (different values)
  )

  ;; Test function for single bit difference
  ;; Validates that values differing by one bit are not equal
  (func (export "test_single_bit_diff") (result i32)
    i32.const 0x00000001  ;; Only LSB set
    i32.const 0x80000000  ;; Only MSB set (sign bit)
    i32.ne                ;; Should return 1 (different values)
  )

  ;; Test function for boundary difference (MAX vs MIN)
  ;; Tests inequality between extreme boundary values
  (func (export "test_boundary_difference") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i32.const 0x80000000  ;; INT32_MIN
    i32.ne                ;; Should return 1 (different boundary values)
  )

  ;; Test function for zero vs non-zero
  ;; Tests that zero is not equal to any non-zero value
  (func (export "test_zero_vs_nonzero") (result i32)
    i32.const 0
    i32.const 1
    i32.ne  ;; Should return 1 (zero != one)
  )

  ;; Test function for different bit patterns
  ;; Tests inequality between alternating patterns
  (func (export "test_different_patterns") (result i32)
    i32.const 0xAAAAAAAA  ;; Alternating pattern A
    i32.const 0x55555555  ;; Alternating pattern 5
    i32.ne                ;; Should return 1 (different patterns)
  )

  ;; Test function for commutative property validation
  ;; Verifies that a != b equals b != a (mathematical property)
  (func (export "test_commutative_property") (param $a i32) (param $b i32) (result i32)
    ;; Calculate a != b
    local.get $a
    local.get $b
    i32.ne
    ;; Calculate b != a
    local.get $b
    local.get $a
    i32.ne
    ;; Both results should be equal, so (a != b) == (b != a) should be true
    i32.eq  ;; Should return 1 (commutative property holds)
  )
)