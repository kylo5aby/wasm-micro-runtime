;; WebAssembly Text Format (.wat) for i32.add Opcode Testing
;; Comprehensive test module for 32-bit integer addition operation
;;
;; This module provides multiple test functions to validate i32.add behavior
;; across various scenarios including overflow, underflow, and edge cases.

(module
  ;; Primary test function for i32.add opcode
  ;; Takes two i32 parameters and returns their sum
  (func (export "test_i32_add") (param $operand1 i32) (param $operand2 i32) (result i32)
    ;; Load first operand onto stack
    local.get $operand1
    ;; Load second operand onto stack
    local.get $operand2
    ;; Execute i32.add opcode
    ;; Stack before: [..., operand1, operand2]
    ;; Stack after:  [..., result]
    i32.add
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for zero addition (identity property)
  (func (export "test_zero_add") (param $x i32) (result i32)
    local.get $x
    i32.const 0
    i32.add
  )

  ;; Test function for overflow behavior (INT32_MAX + 1)
  (func (export "test_overflow") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.const 1
    i32.add               ;; Should wrap to INT32_MIN (-2147483648)
  )

  ;; Test function for underflow behavior (INT32_MIN + (-1))
  (func (export "test_underflow") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.const -1
    i32.add               ;; Should wrap to INT32_MAX (2147483647)
  )

  ;; Test function for large positive addition (INT32_MAX + INT32_MAX)
  (func (export "test_large_positive") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i32.add               ;; Should wrap to -2
  )

  ;; Test function for large negative addition (INT32_MIN + INT32_MIN)
  (func (export "test_large_negative") (result i32)
    i32.const 0x80000000  ;; INT32_MIN
    i32.const 0x80000000  ;; INT32_MIN
    i32.add               ;; Should wrap to 0
  )

  ;; Test function for boundary mix (INT32_MAX + INT32_MIN)
  (func (export "test_boundary_mix") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i32.const 0x80000000  ;; INT32_MIN
    i32.add               ;; Should equal -1
  )
)