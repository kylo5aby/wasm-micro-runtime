(module
  ;; Function: i64_le_s_stack_underflow_test
  ;; Description: Tests i64.le_s with insufficient stack operands (should fail validation)
  ;; This function intentionally has invalid bytecode to test error handling
  ;; Parameters: None
  ;; Returns: i32 (but should fail during module validation)
  (func $i64_le_s_stack_underflow_test (export "i64_le_s_stack_underflow_test") (result i32)
    ;; Push only one i64 value (need 2 for i64.le_s)
    i64.const 42
    ;; Execute i64.le_s with insufficient operands - this should cause validation error
    i64.le_s
    ;; This should never be reached due to validation failure
  )

  ;; Function: i64_le_s_empty_stack_test
  ;; Description: Tests i64.le_s with completely empty stack (should fail validation)
  ;; Parameters: None
  ;; Returns: i32 (but should fail during module validation)
  (func $i64_le_s_empty_stack_test (export "i64_le_s_empty_stack_test") (result i32)
    ;; No operands pushed to stack
    ;; Execute i64.le_s with empty stack - this should cause validation error
    i64.le_s
    ;; This should never be reached due to validation failure
  )
)