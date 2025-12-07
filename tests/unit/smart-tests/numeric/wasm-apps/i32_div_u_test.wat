(module
  ;; Test function for i32.div_u opcode validation
  ;; Performs unsigned 32-bit integer division: dividend ÷ divisor
  ;; Parameters: i32 dividend, i32 divisor
  ;; Returns: i32 quotient (unsigned division result)
  ;; Exception: Traps on division by zero with "integer divide by zero"

  ;; Main test function: i32_div_u
  ;; Implements i32.div_u operation for comprehensive testing
  ;; Covers all test scenarios from basic arithmetic to boundary conditions
  (func $i32_div_u (export "i32_div_u") (param $dividend i32) (param $divisor i32) (result i32)
    ;; Push dividend and divisor onto stack
    local.get $dividend
    local.get $divisor

    ;; Perform unsigned 32-bit division
    ;; This will trap if divisor is 0 with "integer divide by zero"
    i32.div_u

    ;; Return quotient result
  )

  ;; Basic arithmetic test cases
  ;; Validates fundamental unsigned division operations

  (func $basic_division_10_5 (export "basic_division_10_5") (result i32)
    i32.const 10
    i32.const 5
    i32.div_u
  )

  (func $basic_division_20_4 (export "basic_division_20_4") (result i32)
    i32.const 20
    i32.const 4
    i32.div_u
  )

  (func $basic_division_100_10 (export "basic_division_100_10") (result i32)
    i32.const 100
    i32.const 10
    i32.div_u
  )

  ;; Boundary value test cases
  ;; Tests extreme values and edge conditions

  (func $boundary_max_div_max (export "boundary_max_div_max") (result i32)
    i32.const 4294967295  ;; UINT32_MAX
    i32.const 4294967295  ;; UINT32_MAX
    i32.div_u             ;; Should equal 1
  )

  (func $boundary_max_div_2 (export "boundary_max_div_2") (result i32)
    i32.const 4294967295  ;; UINT32_MAX
    i32.const 2
    i32.div_u             ;; Should equal 2147483647
  )

  (func $boundary_small_div_max (export "boundary_small_div_max") (result i32)
    i32.const 5
    i32.const 4294967295  ;; UINT32_MAX
    i32.div_u             ;; Should equal 0
  )

  ;; Power of 2 division tests (efficient operations)

  (func $power_of_2_div_8_4 (export "power_of_2_div_8_4") (result i32)
    i32.const 8
    i32.const 4
    i32.div_u             ;; Should equal 2
  )

  (func $power_of_2_div_16_8 (export "power_of_2_div_16_8") (result i32)
    i32.const 16
    i32.const 8
    i32.div_u             ;; Should equal 2
  )

  ;; Identity operation test cases
  ;; Validates mathematical identity properties

  (func $identity_div_by_1 (export "identity_div_by_1") (result i32)
    i32.const 42
    i32.const 1
    i32.div_u             ;; Should equal 42
  )

  (func $identity_max_div_by_1 (export "identity_max_div_by_1") (result i32)
    i32.const 4294967295  ;; UINT32_MAX
    i32.const 1
    i32.div_u             ;; Should equal UINT32_MAX
  )

  (func $identity_self_div_15 (export "identity_self_div_15") (result i32)
    i32.const 15
    i32.const 15
    i32.div_u             ;; Should equal 1
  )

  (func $identity_self_div_1000 (export "identity_self_div_1000") (result i32)
    i32.const 1000
    i32.const 1000
    i32.div_u             ;; Should equal 1
  )

  (func $identity_self_div_large (export "identity_self_div_large") (result i32)
    i32.const 3000000000
    i32.const 3000000000
    i32.div_u             ;; Should equal 1
  )

  ;; Zero dividend test cases
  ;; Validates zero divided by non-zero equals zero

  (func $zero_dividend_div_1 (export "zero_dividend_div_1") (result i32)
    i32.const 0
    i32.const 1
    i32.div_u             ;; Should equal 0
  )

  (func $zero_dividend_div_5 (export "zero_dividend_div_5") (result i32)
    i32.const 0
    i32.const 5
    i32.div_u             ;; Should equal 0
  )

  (func $zero_dividend_div_max (export "zero_dividend_div_max") (result i32)
    i32.const 0
    i32.const 4294967295  ;; UINT32_MAX
    i32.div_u             ;; Should equal 0
  )

  ;; Truncation behavior test cases
  ;; Validates floor division (truncation toward zero)

  (func $truncation_7_div_3 (export "truncation_7_div_3") (result i32)
    i32.const 7
    i32.const 3
    i32.div_u             ;; Should equal 2 (7/3 = 2.33... → 2)
  )

  (func $truncation_10_div_3 (export "truncation_10_div_3") (result i32)
    i32.const 10
    i32.const 3
    i32.div_u             ;; Should equal 3 (10/3 = 3.33... → 3)
  )

  (func $truncation_100_div_7 (export "truncation_100_div_7") (result i32)
    i32.const 100
    i32.const 7
    i32.div_u             ;; Should equal 14 (100/7 = 14.28... → 14)
  )

  (func $truncation_large_div_3 (export "truncation_large_div_3") (result i32)
    i32.const 4000000000
    i32.const 3
    i32.div_u             ;; Should equal 1333333333
  )

  ;; Division by zero test cases
  ;; These functions will trap with "integer divide by zero"
  ;; Used for exception testing in C++ test suite

  (func $div_by_zero_10 (export "div_by_zero_10") (result i32)
    i32.const 10
    i32.const 0
    i32.div_u             ;; Will trap: "integer divide by zero"
  )

  (func $div_by_zero_0 (export "div_by_zero_0") (result i32)
    i32.const 0
    i32.const 0
    i32.div_u             ;; Will trap: "integer divide by zero"
  )

  (func $div_by_zero_max (export "div_by_zero_max") (result i32)
    i32.const 4294967295  ;; UINT32_MAX
    i32.const 0
    i32.div_u             ;; Will trap: "integer divide by zero"
  )

  (func $div_by_zero_large (export "div_by_zero_large") (result i32)
    i32.const 3000000000
    i32.const 0
    i32.div_u             ;; Will trap: "integer divide by zero"
  )
)