;; Copyright (C) 2019 Intel Corporation. All rights reserved.
;; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

;; WebAssembly Text Format module for comprehensive i32.rem_s opcode testing
;; Provides test functions for signed 32-bit integer remainder (modulo) validation across
;; various scenarios including basic arithmetic, boundary conditions, sign handling,
;; identity operations, and exception handling.

(module
  ;; Main i32.rem_s test function for all remainder scenarios
  ;; Parameters: dividend (i32), divisor (i32)
  ;; Returns: remainder (i32)
  ;; Note: Division by zero will trap as per WebAssembly spec
  ;; The remainder has the same sign as the dividend in signed operations
  (func $i32_rem_s (export "i32_rem_s") (param $dividend i32) (param $divisor i32) (result i32)
    local.get $dividend  ;; Push dividend onto stack
    local.get $divisor   ;; Push divisor onto stack
    i32.rem_s           ;; Perform signed 32-bit integer remainder: dividend % divisor
  )

  ;; Basic remainder test functions for different sign combinations

  ;; Positive dividend, positive divisor: 17 % 5 = 2
  (func $test_positive_positive (export "test_positive_positive") (result i32)
    i32.const 17
    i32.const 5
    i32.rem_s
  )

  ;; Negative dividend, positive divisor: -17 % 5 = -2 (same sign as dividend)
  (func $test_negative_positive (export "test_negative_positive") (result i32)
    i32.const -17
    i32.const 5
    i32.rem_s
  )

  ;; Positive dividend, negative divisor: 17 % -5 = 2 (same sign as dividend)
  (func $test_positive_negative (export "test_positive_negative") (result i32)
    i32.const 17
    i32.const -5
    i32.rem_s
  )

  ;; Negative dividend, negative divisor: -17 % -5 = -2 (same sign as dividend)
  (func $test_negative_negative (export "test_negative_negative") (result i32)
    i32.const -17
    i32.const -5
    i32.rem_s
  )

  ;; Boundary value test functions

  ;; INT32_MAX % small divisor: 2147483647 % 3 = 1
  (func $test_max_int_remainder (export "test_max_int_remainder") (result i32)
    i32.const 2147483647  ;; INT32_MAX
    i32.const 3
    i32.rem_s
  )

  ;; INT32_MIN % small divisor: -2147483648 % 3 = -2
  (func $test_min_int_remainder (export "test_min_int_remainder") (result i32)
    i32.const -2147483648  ;; INT32_MIN
    i32.const 3
    i32.rem_s
  )

  ;; Small dividend % large divisor: 5 % INT32_MAX = 5
  (func $test_small_large_remainder (export "test_small_large_remainder") (result i32)
    i32.const 5
    i32.const 2147483647  ;; INT32_MAX
    i32.rem_s
  )

  ;; Zero operand test functions

  ;; Zero dividend: 0 % 5 = 0
  (func $test_zero_dividend (export "test_zero_dividend") (result i32)
    i32.const 0
    i32.const 5
    i32.rem_s
  )

  ;; Zero dividend with negative divisor: 0 % -3 = 0
  (func $test_zero_dividend_negative (export "test_zero_dividend_negative") (result i32)
    i32.const 0
    i32.const -3
    i32.rem_s
  )

  ;; Identity operation test functions

  ;; x % x = 0: 15 % 15 = 0
  (func $test_same_operands (export "test_same_operands") (result i32)
    i32.const 15
    i32.const 15
    i32.rem_s
  )

  ;; x % 1 = 0: 42 % 1 = 0
  (func $test_modulo_one (export "test_modulo_one") (result i32)
    i32.const 42
    i32.const 1
    i32.rem_s
  )

  ;; x % -1 = 0: 42 % -1 = 0
  (func $test_modulo_neg_one (export "test_modulo_neg_one") (result i32)
    i32.const 42
    i32.const -1
    i32.rem_s
  )

  ;; Special case test functions

  ;; INT32_MIN % -1 = 0 (special case - no overflow in remainder)
  (func $test_min_int_mod_neg_one (export "test_min_int_mod_neg_one") (result i32)
    i32.const -2147483648  ;; INT32_MIN
    i32.const -1
    i32.rem_s
  )

  ;; Boundary around zero: -1 % 1 = 0
  (func $test_neg_one_mod_one (export "test_neg_one_mod_one") (result i32)
    i32.const -1
    i32.const 1
    i32.rem_s
  )

  ;; Division by zero test functions (these will trap when executed)

  ;; Positive % 0 - will trap
  (func $test_positive_div_zero (export "test_positive_div_zero") (result i32)
    i32.const 5
    i32.const 0
    i32.rem_s
  )

  ;; Negative % 0 - will trap
  (func $test_negative_div_zero (export "test_negative_div_zero") (result i32)
    i32.const -5
    i32.const 0
    i32.rem_s
  )

  ;; Zero % 0 - will trap
  (func $test_zero_div_zero (export "test_zero_div_zero") (result i32)
    i32.const 0
    i32.const 0
    i32.rem_s
  )
)