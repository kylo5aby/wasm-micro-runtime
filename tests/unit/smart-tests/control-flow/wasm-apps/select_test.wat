;; select_test.wat - WebAssembly Text Format for select opcode testing
;;
;; This module provides comprehensive test functions for the WebAssembly select opcode.
;; Tests cover all supported data types (i32, i64, f32, f64) and various selection scenarios
;; including basic conditional selection, boundary values, special float values, and edge cases.

(module
  ;; Test function for i32 select operation
  ;; Parameters: val1 (i32), val2 (i32), cond (i32)
  ;; Returns: selected i32 value (val1 if cond != 0, val2 if cond == 0)
  (func (export "test_i32_select") (param $val1 i32) (param $val2 i32) (param $cond i32) (result i32)
    local.get $val1
    local.get $val2
    local.get $cond
    select
  )

  ;; Test function for i64 select operation
  ;; Parameters: val1 (i64), val2 (i64), cond (i32)
  ;; Returns: selected i64 value (val1 if cond != 0, val2 if cond == 0)
  (func (export "test_i64_select") (param $val1 i64) (param $val2 i64) (param $cond i32) (result i64)
    local.get $val1
    local.get $val2
    local.get $cond
    select
  )

  ;; Test function for f32 select operation
  ;; Parameters: val1 (f32), val2 (f32), cond (i32)
  ;; Returns: selected f32 value (val1 if cond != 0, val2 if cond == 0)
  (func (export "test_f32_select") (param $val1 f32) (param $val2 f32) (param $cond i32) (result f32)
    local.get $val1
    local.get $val2
    local.get $cond
    select
  )

  ;; Test function for f64 select operation
  ;; Parameters: val1 (f64), val2 (f64), cond (i32)
  ;; Returns: selected f64 value (val1 if cond != 0, val2 if cond == 0)
  (func (export "test_f64_select") (param $val1 f64) (param $val2 f64) (param $cond i32) (result f64)
    local.get $val1
    local.get $val2
    local.get $cond
    select
  )

  ;; Test function demonstrating select with constant values
  ;; This function tests basic select behavior with hardcoded values
  (func (export "test_select_constants") (param $cond i32) (result i32)
    i32.const 42     ;; val1
    i32.const 99     ;; val2
    local.get $cond  ;; condition
    select
  )

  ;; Test function for nested select operations
  ;; Demonstrates select can be used in compound expressions
  (func (export "test_nested_select") (param $a i32) (param $b i32) (param $c i32) (param $cond1 i32) (param $cond2 i32) (result i32)
    local.get $a
    local.get $b
    local.get $cond1
    select
    local.get $c
    local.get $cond2
    select
  )

  ;; Test function for select with arithmetic operations
  ;; Tests select as part of larger computation chains
  (func (export "test_select_with_arithmetic") (param $x i32) (param $y i32) (param $cond i32) (result i32)
    local.get $x
    i32.const 10
    i32.add
    local.get $y
    i32.const 20
    i32.add
    local.get $cond
    select
  )

  ;; Test function for identity select (same values)
  ;; Tests mathematical property: select(x, x, c) = x
  (func (export "test_identity_select") (param $val i32) (param $cond i32) (result i32)
    local.get $val
    local.get $val  ;; Same value for both operands
    local.get $cond
    select
  )

  ;; Test function for select with zero operands
  ;; Tests behavior when one or both operands are zero
  (func (export "test_select_with_zeros") (param $cond i32) (result i32)
    i32.const 0     ;; First zero
    i32.const 0     ;; Second zero
    local.get $cond
    select
  )

  ;; Test function demonstrating type consistency requirement
  ;; All select operands must be the same type
  (func (export "test_type_consistency") (param $val1 f32) (param $val2 f32) (param $cond i32) (result f32)
    local.get $val1  ;; Both operands must be f32
    local.get $val2  ;; Both operands must be f32
    local.get $cond  ;; Condition is always i32
    select
  )
)