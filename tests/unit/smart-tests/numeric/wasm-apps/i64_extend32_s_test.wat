(module
  ;; i64.extend32_s Test Module
  ;;
  ;; This module provides comprehensive test functions for the i64.extend32_s WebAssembly opcode.
  ;; The i64.extend32_s instruction performs sign extension of the lower 32 bits of an i64 value
  ;; to a full 64-bit signed integer, treating the lower 32 bits as a signed 32-bit value.
  ;;
  ;; Functions exported:
  ;; - test_i64_extend32_s: Basic sign extension operation
  ;;
  ;; Sign extension logic:
  ;; - If bit 31 (sign bit) is 0: Upper 32 bits filled with 0s (positive values 0 to 2147483647)
  ;; - If bit 31 (sign bit) is 1: Upper 32 bits filled with 1s (negative values -2147483648 to -1)

  ;; Basic i64.extend32_s operation test function
  ;; Takes an i64 input, applies i64.extend32_s, and returns the result
  ;; Tests the fundamental sign extension behavior across all 32-bit value ranges
  (func (export "test_i64_extend32_s") (param $input i64) (result i64)
    ;; Load input parameter onto stack
    local.get $input

    ;; Apply i64.extend32_s operation
    ;; This instruction:
    ;; 1. Takes the lower 32 bits of the input i64 value
    ;; 2. Treats those 32 bits as a signed 32-bit integer (-2147483648 to +2147483647)
    ;; 3. Sign-extends to full 64-bit signed integer
    ;; 4. Upper 32 bits of input are completely ignored
    i64.extend32_s

    ;; Return the sign-extended result
  )

  ;; Test function with specific positive value
  ;; Returns sign extension of 0x12345678 (305419896 in decimal)
  (func (export "test_positive_value") (result i64)
    i64.const 0x12345678
    i64.extend32_s
  )

  ;; Test function with specific negative value
  ;; Returns sign extension of 0x87654321 (-2023406815 in 32-bit signed)
  (func (export "test_negative_value") (result i64)
    i64.const 0x87654321
    i64.extend32_s
  )

  ;; Test function with INT32_MAX
  ;; Returns sign extension of 0x7FFFFFFF (2147483647)
  (func (export "test_int32_max") (result i64)
    i64.const 0x7FFFFFFF
    i64.extend32_s
  )

  ;; Test function with INT32_MIN
  ;; Returns sign extension of 0x80000000 (-2147483648)
  (func (export "test_int32_min") (result i64)
    i64.const 0x80000000
    i64.extend32_s
  )

  ;; Test function with zero value
  ;; Returns sign extension of 0x00000000 (0)
  (func (export "test_zero") (result i64)
    i64.const 0
    i64.extend32_s
  )

  ;; Test function with -1 in 32-bit representation
  ;; Returns sign extension of 0xFFFFFFFF (-1)
  (func (export "test_minus_one") (result i64)
    i64.const 0xFFFFFFFF
    i64.extend32_s
  )

  ;; Test function with high bits set (should be ignored)
  ;; Returns sign extension of lower 32 bits of 0xFFFFFFFF12345678
  (func (export "test_high_bits_positive") (result i64)
    i64.const 0xFFFFFFFF12345678
    i64.extend32_s
  )

  ;; Test function with high bits set and negative lower 32 bits
  ;; Returns sign extension of lower 32 bits of 0x123456789ABCDEF0
  (func (export "test_high_bits_negative") (result i64)
    i64.const 0x123456789ABCDEF0
    i64.extend32_s
  )
)