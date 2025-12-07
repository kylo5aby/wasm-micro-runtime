(module
  ;; i64.extend32_s Stack Underflow Test Module
  ;;
  ;; This module contains intentionally invalid WebAssembly code to test
  ;; stack underflow detection during i64.extend32_s operation.
  ;; The function attempts to execute i64.extend32_s without providing
  ;; the required i64 operand on the stack.
  ;;
  ;; Expected behavior: WAMR should detect this as a validation error
  ;; during module loading or instantiation phase.

  ;; Invalid function that attempts i64.extend32_s without operand
  ;; This should cause a stack underflow validation error
  (func (export "test_stack_underflow") (result i64)
    ;; Attempt to execute i64.extend32_s without pushing an i64 value first
    ;; This creates a stack underflow condition:
    ;; - Stack state: empty []
    ;; - i64.extend32_s requires: [i64] -> [i64]
    ;; - Actual stack provides: [] (underflow!)
    i64.extend32_s

    ;; This line should never be reached due to validation failure
  )
)