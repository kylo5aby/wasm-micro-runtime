;; i64_shr_u_no_operands.wat - Stack underflow test (no operands)
;; This module should fail to validate due to stack underflow

(module
  ;; Test function with stack underflow - no operands for i64.shr_u
  (func (export "test_no_operands") (result i64)
    i64.shr_u      ;; This should cause validation error - no operands on stack
    i64.const 0    ;; Unreachable due to validation error
  )
)