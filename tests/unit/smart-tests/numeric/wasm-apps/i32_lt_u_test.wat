;; WebAssembly Text Format (.wat) for i32.lt_u Opcode Testing
;; Comprehensive test module for 32-bit unsigned integer less-than comparison operation
;;
;; This module provides multiple test functions to validate i32.lt_u behavior
;; across various scenarios including unsigned boundaries, signed/unsigned differences,
;; and mathematical properties of unsigned comparison.

(module
  ;; Primary test function for i32.lt_u opcode
  ;; Takes two i32 parameters and returns 1 if first < second (unsigned), 0 otherwise
  (func (export "test_i32_lt_u") (param $operand1 i32) (param $operand2 i32) (result i32)
    ;; Load first operand onto stack
    local.get $operand1
    ;; Load second operand onto stack
    local.get $operand2
    ;; Execute i32.lt_u opcode - unsigned less-than comparison
    ;; Stack before: [..., operand1, operand2]
    ;; Stack after:  [..., result] (1 if operand1 < operand2 unsigned, 0 otherwise)
    i32.lt_u
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for zero boundary verification
  ;; Validates that 0 < 1 in unsigned comparison returns true
  (func (export "test_zero_less_than_one") (result i32)
    i32.const 0
    i32.const 1
    i32.lt_u  ;; Should return 1 (true)
  )

  ;; Test function for maximum unsigned value boundary
  ;; Validates behavior with MAX_UINT32 (0xFFFFFFFF)
  (func (export "test_max_uint32_boundary") (result i32)
    i32.const 0xFFFFFFFE  ;; MAX_UINT32 - 1
    i32.const 0xFFFFFFFF  ;; MAX_UINT32
    i32.lt_u  ;; Should return 1 (true)
  )

  ;; Test function demonstrating signed vs unsigned difference
  ;; Shows that -1 (0xFFFFFFFF) is largest unsigned value
  (func (export "test_negative_as_unsigned") (result i32)
    i32.const 100         ;; Positive value
    i32.const -1          ;; -1 as unsigned is 0xFFFFFFFF (largest unsigned)
    i32.lt_u  ;; Should return 1 (true): 100 < 0xFFFFFFFF
  )

  ;; Test function for identity comparison
  ;; Validates that x < x always returns false (anti-reflexivity)
  (func (export "test_identity_comparison") (result i32)
    i32.const 42
    i32.const 42
    i32.lt_u  ;; Should return 0 (false)
  )

  ;; Test function for signed/unsigned boundary comparison
  ;; INT32_MAX vs INT32_MIN in unsigned interpretation
  (func (export "test_signed_boundary_as_unsigned") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i32.const 0x80000000  ;; INT32_MIN (as unsigned: 2147483648)
    i32.lt_u  ;; Should return 1 (true): 0x7FFFFFFF < 0x80000000 unsigned
  )

  ;; Test function for transitivity property validation
  ;; If a < b and b < c, then a < c should hold
  (func (export "test_transitivity") (result i32)
    ;; Test: 10 < 20 < 30
    i32.const 10
    i32.const 30
    i32.lt_u  ;; Should return 1 (true)
  )

  ;; Test function for negative values comparison in unsigned context
  ;; Demonstrates that -2 < -1 in unsigned interpretation
  (func (export "test_negative_values_unsigned") (result i32)
    i32.const -2          ;; 0xFFFFFFFE as unsigned
    i32.const -1          ;; 0xFFFFFFFF as unsigned
    i32.lt_u  ;; Should return 1 (true): 0xFFFFFFFE < 0xFFFFFFFF
  )

  ;; Test function for large positive vs negative in unsigned context
  ;; Shows negative values are larger than positive when viewed as unsigned
  (func (export "test_positive_vs_negative_unsigned") (result i32)
    i32.const 0x70000000  ;; Large positive value
    i32.const -100        ;; Negative value (0xFFFFFF9C as unsigned)
    i32.lt_u  ;; Should return 1 (true): positive < negative as unsigned
  )
)