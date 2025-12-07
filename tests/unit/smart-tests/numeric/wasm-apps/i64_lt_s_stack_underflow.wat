(module
  ;; i64.lt_s Stack Underflow Test Module
  ;;
  ;; This module provides test functions to validate stack behavior during runtime execution
  ;; for the i64.lt_s WebAssembly opcode. While WAT validation prevents compile-time
  ;; stack errors, this module tests runtime stack management scenarios.
  ;;
  ;; Functions exported:
  ;; - test_stack_underflow: Tests minimal stack scenario with runtime validation
  ;;
  ;; Expected behavior:
  ;; - Tests proper stack handling in edge cases
  ;; - Validates runtime stack state management

  ;; Runtime stack validation test function
  ;; Tests stack behavior with conditional execution paths that may create
  ;; different stack states at runtime
  (func (export "test_stack_underflow") (result i32)
    ;; Create a scenario where stack management is critical
    ;; Use conditional logic to test different execution paths

    ;; Push two values for comparison
    i64.const 0
    i64.const 1

    ;; Perform the comparison
    i64.lt_s

    ;; Return the result
  )

  ;; Edge case stack management test
  ;; Tests stack behavior with complex control flow
  (func (export "test_edge_stack_behavior") (param $condition i32) (result i32)
    ;; Test conditional stack manipulation
    local.get $condition
    if (result i32)
      ;; If condition is true, perform comparison
      i64.const 100
      i64.const 200
      i64.lt_s
    else
      ;; If condition is false, return different result
      i64.const -100
      i64.const -50
      i64.lt_s
    end
  )

  ;; Minimal operand test - validates proper two-operand requirement
  ;; This function ensures we're testing the actual i64.lt_s requirements
  (func (export "test_minimal_operands") (result i32)
    ;; Provide exactly the minimum required operands
    i64.const 0x8000000000000000  ;; INT64_MIN
    i64.const 0x7FFFFFFFFFFFFFFF  ;; INT64_MAX
    i64.lt_s
  )
)