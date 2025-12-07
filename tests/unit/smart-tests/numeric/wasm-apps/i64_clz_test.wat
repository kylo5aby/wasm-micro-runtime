;; WebAssembly Text Format (WAT) module for i64.clz opcode testing
;; This module provides test functions for validating the i64.clz instruction

(module
  ;; Export test function for i64.clz operation
  ;; Takes one i64 parameter and returns i32 result (leading zero count)
  (func $test_i64_clz (export "test_i64_clz") (param $input i64) (result i32)
    local.get $input    ;; Push input i64 value onto stack
    i64.clz            ;; Execute count leading zeros operation (returns i64)
    i32.wrap_i64       ;; Convert i64 result to i32 (result is always 0-64)
  )

  ;; Export function for boundary value testing - zero
  (func $test_zero_clz (export "test_zero_clz") (result i32)
    i64.const 0
    i64.clz
    i32.wrap_i64
  )

  ;; Export function for boundary value testing - maximum value
  (func $test_max_clz (export "test_max_clz") (result i32)
    i64.const 0x7FFFFFFFFFFFFFFF  ;; INT64_MAX
    i64.clz
    i32.wrap_i64
  )

  ;; Export function for boundary value testing - minimum value
  (func $test_min_clz (export "test_min_clz") (result i32)
    i64.const 0x8000000000000000  ;; INT64_MIN (MSB set)
    i64.clz
    i32.wrap_i64
  )

  ;; Export function for power of 2 testing
  (func $test_power_clz (export "test_power_clz") (param $exponent i32) (result i32)
    i64.const 1
    local.get $exponent
    i64.extend_i32_u
    i64.shl
    i64.clz
    i32.wrap_i64
  )
)