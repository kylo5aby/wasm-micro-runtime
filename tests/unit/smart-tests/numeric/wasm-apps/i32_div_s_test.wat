;; Copyright (C) 2019 Intel Corporation. All rights reserved.
;; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

;; WebAssembly Text Format module for comprehensive i32.div_s opcode testing
;; Provides test functions for signed 32-bit integer division validation across
;; various scenarios including basic arithmetic, boundary conditions, truncation
;; behavior, identity operations, and exception handling.

(module
  ;; Main i32.div_s test function for all division scenarios
  ;; Parameters: dividend (i32), divisor (i32)
  ;; Returns: quotient (i32)
  ;; Note: Division by zero and overflow will trap as per WebAssembly spec
  (func $i32_div_s (export "i32_div_s") (param $dividend i32) (param $divisor i32) (result i32)
    local.get $dividend  ;; Push dividend onto stack
    local.get $divisor   ;; Push divisor onto stack
    i32.div_s           ;; Perform signed 32-bit integer division: dividend ÷ divisor
  )

  ;; Basic division test functions for different sign combinations

  ;; Positive dividend, positive divisor: 20 ÷ 4 = 5
  (func $test_positive_positive (export "test_positive_positive") (result i32)
    i32.const 20
    i32.const 4
    i32.div_s
  )

  ;; Negative dividend, positive divisor: -20 ÷ 4 = -5
  (func $test_negative_positive (export "test_negative_positive") (result i32)
    i32.const -20
    i32.const 4
    i32.div_s
  )

  ;; Positive dividend, negative divisor: 20 ÷ -4 = -5
  (func $test_positive_negative (export "test_positive_negative") (result i32)
    i32.const 20
    i32.const -4
    i32.div_s
  )

  ;; Negative dividend, negative divisor: -20 ÷ -4 = 5
  (func $test_negative_negative (export "test_negative_negative") (result i32)
    i32.const -20
    i32.const -4
    i32.div_s
  )

  ;; Boundary value test functions

  ;; INT32_MAX ÷ 1 = INT32_MAX (2147483647)
  (func $test_max_div_one (export "test_max_div_one") (result i32)
    i32.const 2147483647  ;; INT32_MAX
    i32.const 1
    i32.div_s
  )

  ;; INT32_MIN ÷ 1 = INT32_MIN (-2147483648)
  (func $test_min_div_one (export "test_min_div_one") (result i32)
    i32.const -2147483648  ;; INT32_MIN
    i32.const 1
    i32.div_s
  )

  ;; INT32_MAX ÷ INT32_MAX = 1
  (func $test_max_div_max (export "test_max_div_max") (result i32)
    i32.const 2147483647  ;; INT32_MAX
    i32.const 2147483647  ;; INT32_MAX
    i32.div_s
  )

  ;; Truncation behavior test functions (toward zero)

  ;; 7 ÷ 3 = 2 (truncates 2.333... toward zero)
  (func $test_positive_truncation (export "test_positive_truncation") (result i32)
    i32.const 7
    i32.const 3
    i32.div_s
  )

  ;; -7 ÷ 3 = -2 (truncates -2.333... toward zero)
  (func $test_negative_truncation (export "test_negative_truncation") (result i32)
    i32.const -7
    i32.const 3
    i32.div_s
  )

  ;; 7 ÷ -3 = -2 (truncates -2.333... toward zero)
  (func $test_mixed_sign_truncation (export "test_mixed_sign_truncation") (result i32)
    i32.const 7
    i32.const -3
    i32.div_s
  )

  ;; -7 ÷ -3 = 2 (truncates 2.333... toward zero)
  (func $test_both_negative_truncation (export "test_both_negative_truncation") (result i32)
    i32.const -7
    i32.const -3
    i32.div_s
  )

  ;; Identity operation test functions

  ;; 42 ÷ 1 = 42 (identity: division by 1)
  (func $test_identity_positive (export "test_identity_positive") (result i32)
    i32.const 42
    i32.const 1
    i32.div_s
  )

  ;; -42 ÷ 1 = -42 (identity: division by 1)
  (func $test_identity_negative (export "test_identity_negative") (result i32)
    i32.const -42
    i32.const 1
    i32.div_s
  )

  ;; 0 ÷ 5 = 0 (zero dividend)
  (func $test_zero_dividend_positive (export "test_zero_dividend_positive") (result i32)
    i32.const 0
    i32.const 5
    i32.div_s
  )

  ;; 0 ÷ -5 = 0 (zero dividend)
  (func $test_zero_dividend_negative (export "test_zero_dividend_negative") (result i32)
    i32.const 0
    i32.const -5
    i32.div_s
  )

  ;; 15 ÷ 15 = 1 (self division)
  (func $test_self_division_positive (export "test_self_division_positive") (result i32)
    i32.const 15
    i32.const 15
    i32.div_s
  )

  ;; -15 ÷ -15 = 1 (self division)
  (func $test_self_division_negative (export "test_self_division_negative") (result i32)
    i32.const -15
    i32.const -15
    i32.div_s
  )

  ;; Exception test functions (these will trap when called)

  ;; Division by zero: 10 ÷ 0 (will trap)
  (func $test_div_by_zero_positive (export "test_div_by_zero_positive") (result i32)
    i32.const 10
    i32.const 0
    i32.div_s
  )

  ;; Division by zero: -10 ÷ 0 (will trap)
  (func $test_div_by_zero_negative (export "test_div_by_zero_negative") (result i32)
    i32.const -10
    i32.const 0
    i32.div_s
  )

  ;; Division by zero: 0 ÷ 0 (will trap)
  (func $test_div_by_zero_zero (export "test_div_by_zero_zero") (result i32)
    i32.const 0
    i32.const 0
    i32.div_s
  )

  ;; Integer overflow: INT32_MIN ÷ -1 (will trap due to signed overflow)
  (func $test_overflow_trap (export "test_overflow_trap") (result i32)
    i32.const -2147483648  ;; INT32_MIN
    i32.const -1
    i32.div_s  ;; This will trap: result would be 2147483648 which exceeds INT32_MAX
  )

  ;; Boundary division by zero test functions

  ;; INT32_MAX ÷ 0 (will trap)
  (func $test_max_div_by_zero (export "test_max_div_by_zero") (result i32)
    i32.const 2147483647  ;; INT32_MAX
    i32.const 0
    i32.div_s
  )

  ;; INT32_MIN ÷ 0 (will trap)
  (func $test_min_div_by_zero (export "test_min_div_by_zero") (result i32)
    i32.const -2147483648  ;; INT32_MIN
    i32.const 0
    i32.div_s
  )
)