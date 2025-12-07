(module
  ;; Test function for i32.rem_u opcode
  ;; Performs unsigned remainder operation on two i32 values
  (func (export "test_rem_u") (param $dividend i32) (param $divisor i32) (result i32)
    local.get $dividend  ;; Push dividend onto stack
    local.get $divisor   ;; Push divisor onto stack
    i32.rem_u           ;; Perform unsigned remainder operation
  )

  ;; Additional test functions for comprehensive coverage

  ;; Test with specific boundary values
  (func (export "test_max_values") (param $dividend i32) (param $divisor i32) (result i32)
    local.get $dividend
    local.get $divisor
    i32.rem_u
  )

  ;; Test zero dividend scenarios
  (func (export "test_zero_dividend") (param $divisor i32) (result i32)
    i32.const 0          ;; Zero dividend
    local.get $divisor
    i32.rem_u
  )

  ;; Test identity operations
  (func (export "test_identity") (param $value i32) (result i32)
    local.get $value
    i32.const 1          ;; Modulo 1 should always return 0
    i32.rem_u
  )

  ;; Test self-modulo operations
  (func (export "test_self_modulo") (param $value i32) (result i32)
    local.get $value
    local.get $value     ;; value % value should return 0 (when value != 0)
    i32.rem_u
  )

  ;; Test large number operations
  (func (export "test_large_numbers") (param $dividend i32) (param $divisor i32) (result i32)
    local.get $dividend
    local.get $divisor
    i32.rem_u
  )
)