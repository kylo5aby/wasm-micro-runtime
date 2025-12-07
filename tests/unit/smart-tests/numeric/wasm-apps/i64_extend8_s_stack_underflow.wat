(module
  ;; i64.extend8_s Stack Underflow Test Module
  ;;
  ;; This module is designed to trigger stack underflow conditions for i64.extend8_s operation.
  ;; The WebAssembly specification requires that instructions have sufficient operands on the
  ;; execution stack. This module intentionally violates this requirement to test error handling.
  ;;
  ;; Expected behavior: Module loading should fail during validation phase due to stack underflow.

  ;; Function that attempts i64.extend8_s without providing an i64 operand
  ;; This should cause stack underflow during WebAssembly validation
  (func (export "test_stack_underflow") (result i64)
    ;; Attempt i64.extend8_s operation without pushing an i64 value first
    ;; This violates WebAssembly stack discipline and should be caught during module validation
    i64.extend8_s

    ;; This point should never be reached due to validation failure
  )

  ;; Alternative underflow test: consume more values than available
  (func (export "test_insufficient_operands") (result i64)
    ;; Push one i64 value
    i64.const 42

    ;; Consume the i64 value with extend8_s
    i64.extend8_s

    ;; Attempt another i64.extend8_s without sufficient operands
    ;; Stack is now empty, so this should cause underflow
    i64.extend8_s
  )
)