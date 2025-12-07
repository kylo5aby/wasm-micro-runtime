(module
  ;; Valid WASM module demonstrating proper i32.rotr stack usage
  ;; This module compiles successfully and tests normal operation

  ;; Standard two-parameter i32.rotr function
  (func (export "valid_rotr_test") (param $value i32) (param $count i32) (result i32)
    local.get $value
    local.get $count
    i32.rotr)

  ;; Test function that ensures proper stack management
  (func (export "stack_management_test") (result i32)
    i32.const 0x12345678  ;; Push value to rotate
    i32.const 4           ;; Push rotation count
    i32.rotr              ;; Should succeed with proper stack
    )

  ;; Function to validate stack state before and after rotation
  (func (export "stack_validation_test") (result i32)
    ;; Push multiple values to test stack state
    i32.const 1           ;; Extra value on stack
    i32.const 0xABCDEF00  ;; Value to rotate
    i32.const 8           ;; Rotation count
    i32.rotr              ;; Rotate (consumes 2, pushes 1)
    i32.add               ;; Add with the extra value (should work)
    )

  ;; Test edge case with zero rotation
  (func (export "zero_rotation_test") (result i32)
    i32.const 0x87654321
    i32.const 0           ;; Zero rotation count
    i32.rotr              ;; Should return original value
    )

  ;; Test with maximum rotation count (31)
  (func (export "max_rotation_test") (result i32)
    i32.const 0x80000001
    i32.const 31
    i32.rotr
    )

  ;; Test modulo behavior with large count
  (func (export "modulo_rotation_test") (result i32)
    i32.const 0x12345678
    i32.const 36          ;; 36 % 32 = 4
    i32.rotr
    )
)