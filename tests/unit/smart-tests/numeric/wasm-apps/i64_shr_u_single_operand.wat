;; i64_shr_u_single_operand.wat - Stack underflow test (single operand)
;; This module should fail to validate due to insufficient operands

(module
  ;; Test function with stack underflow - only one operand for i64.shr_u
  (func (export "test_single_operand") (result i64)
    i64.const 42   ;; Push single operand
    i64.shr_u      ;; This should cause validation error - needs 2 operands
    i64.const 0    ;; Unreachable due to validation error
  )
)