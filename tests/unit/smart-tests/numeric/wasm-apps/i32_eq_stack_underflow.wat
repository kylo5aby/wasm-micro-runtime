;; WebAssembly Text Format (.wat) for i32.eq Stack Validation Testing
;; Valid module that tests edge cases near stack underflow conditions
;;
;; This module contains functions that test boundary conditions for stack
;; validation, demonstrating proper error handling in edge cases.

(module
  ;; Function that tests minimum valid stack usage for i32.eq
  ;; This tests the boundary between valid and invalid stack states
  (func (export "test_minimal_stack") (result i32)
    i32.const 42  ;; Push operand 1
    i32.const 42  ;; Push operand 2 (now we have exactly the required operands)
    i32.eq        ;; Valid: exactly 2 operands available - should return 1
  )

  ;; Function that tests stack with extra operands (valid but unusual)
  ;; This tests that i32.eq works correctly with more than minimum stack depth
  (func (export "test_extra_stack") (result i32)
    i32.const 100  ;; Extra value (will remain on stack)
    i32.const 5    ;; Operand 1 for i32.eq
    i32.const 5    ;; Operand 2 for i32.eq
    i32.eq         ;; Valid: 2+ operands available - should return 1
    ;; Stack now contains: [100, 1] - the extra value and the result
    drop           ;; Remove the i32.eq result
    ;; Return the extra value that was left on stack
  )

  ;; Function that demonstrates proper stack management before i32.eq
  ;; This shows correct usage patterns and validates clean execution
  (func (export "test_proper_usage") (param $a i32) (param $b i32) (result i32)
    local.get $a   ;; Get first parameter
    local.get $b   ;; Get second parameter
    i32.eq         ;; Compare the parameters
    ;; Result is automatically returned
  )

  ;; Function that tests consecutive i32.eq operations
  ;; This validates that stack management works correctly across multiple operations
  (func (export "test_consecutive_ops") (result i32)
    ;; First comparison: 10 == 10
    i32.const 10
    i32.const 10
    i32.eq         ;; Result: 1 (true)

    ;; Second comparison: 5 == 3
    i32.const 5
    i32.const 3
    i32.eq         ;; Result: 0 (false)

    ;; Compare the two results: 1 == 0
    i32.eq         ;; Final result: 0 (false, since 1 != 0)
  )

  ;; Function that tests stack underflow is properly prevented by validation
  ;; This demonstrates that the module loader prevents invalid operations
  (func (export "test_validation_success") (result i32)
    ;; This function succeeds because it has valid stack usage
    i32.const 123
    i32.const 123
    i32.eq    ;; Returns 1 - validation ensures this is safe
  )
)