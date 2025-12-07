;; WebAssembly Text Format (WAT) module for i32.clz opcode testing
;; This module provides test functions for validating the i32.clz instruction

(module
  ;; Export test function for i32.clz operation
  ;; Takes one i32 parameter and returns i32 result (leading zero count)
  (func $test_i32_clz (export "test_i32_clz") (param $input i32) (result i32)
    local.get $input    ;; Push input i32 value onto stack
    i32.clz            ;; Execute count leading zeros operation (returns i32)
  )

  ;; Export function for boundary value testing - zero
  (func $test_zero_clz (export "test_zero_clz") (result i32)
    i32.const 0
    i32.clz
  )

  ;; Export function for boundary value testing - maximum value
  (func $test_max_clz (export "test_max_clz") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX
    i32.clz
  )

  ;; Export function for boundary value testing - minimum value
  (func $test_min_clz (export "test_min_clz") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (MSB set)
    i32.clz
  )

  ;; Export function for power of 2 testing
  (func $test_power_clz (export "test_power_clz") (param $exponent i32) (result i32)
    i32.const 1
    local.get $exponent
    i32.shl
    i32.clz
  )

  ;; Export function for all bits set testing
  (func $test_all_bits_clz (export "test_all_bits_clz") (result i32)
    i32.const 0xFFFFFFFF  ;; All bits set
    i32.clz
  )

  ;; Export function for single bit pattern testing
  (func $test_single_bit_clz (export "test_single_bit_clz") (param $position i32) (result i32)
    i32.const 1
    local.get $position
    i32.shl
    i32.clz
  )
)