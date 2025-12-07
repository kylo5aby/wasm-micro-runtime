(module
  ;; Function to test i64.rem_s opcode
  ;; Takes two i64 parameters (dividend, divisor) and returns i64 remainder
  (func (export "i64_rem_s_test") (param $dividend i64) (param $divisor i64) (result i64)
    ;; Push dividend and divisor onto stack
    local.get $dividend
    local.get $divisor

    ;; Perform i64.rem_s operation
    ;; Computes signed remainder: dividend % divisor
    ;; Result sign follows dividend sign (C-style remainder)
    i64.rem_s
  )

  ;; Additional test functions for comprehensive validation

  ;; Test function for basic positive remainder
  (func (export "test_positive_remainder") (result i64)
    i64.const 10
    i64.const 3
    i64.rem_s
  )

  ;; Test function for negative dividend remainder
  (func (export "test_negative_dividend") (result i64)
    i64.const -10
    i64.const 3
    i64.rem_s
  )

  ;; Test function for negative divisor remainder
  (func (export "test_negative_divisor") (result i64)
    i64.const 10
    i64.const -3
    i64.rem_s
  )

  ;; Test function for both negative remainder
  (func (export "test_both_negative") (result i64)
    i64.const -10
    i64.const -3
    i64.rem_s
  )

  ;; Test function for zero dividend
  (func (export "test_zero_dividend") (result i64)
    i64.const 0
    i64.const 5
    i64.rem_s
  )

  ;; Test function for unit divisor positive
  (func (export "test_unit_divisor_pos") (result i64)
    i64.const 42
    i64.const 1
    i64.rem_s
  )

  ;; Test function for unit divisor negative
  (func (export "test_unit_divisor_neg") (result i64)
    i64.const 42
    i64.const -1
    i64.rem_s
  )

  ;; Test function for identity operation
  (func (export "test_identity") (result i64)
    i64.const 5
    i64.const 5
    i64.rem_s
  )

  ;; Test function for INT64_MAX boundary
  (func (export "test_int64_max") (result i64)
    i64.const 9223372036854775807  ;; INT64_MAX
    i64.const 2
    i64.rem_s
  )

  ;; Test function for INT64_MIN boundary
  (func (export "test_int64_min") (result i64)
    i64.const -9223372036854775808  ;; INT64_MIN
    i64.const 2
    i64.rem_s
  )

  ;; Test function for special overflow case: INT64_MIN % -1
  (func (export "test_overflow_case") (result i64)
    i64.const -9223372036854775808  ;; INT64_MIN
    i64.const -1
    i64.rem_s
  )

  ;; Test function for power of 2 divisor
  (func (export "test_power_of_two") (result i64)
    i64.const 15
    i64.const 8
    i64.rem_s
  )

  ;; Test function for large number remainder
  (func (export "test_large_numbers") (result i64)
    i64.const 1000000007
    i64.const 1000
    i64.rem_s
  )

  ;; Test function that should trap: division by zero
  ;; This function will cause a runtime trap when called
  (func (export "test_div_by_zero") (result i64)
    i64.const 42
    i64.const 0
    i64.rem_s
  )
)