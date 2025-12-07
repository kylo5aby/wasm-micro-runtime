;; WebAssembly Text Format (.wat) for i64.shl Opcode Testing
;; Comprehensive test module for 64-bit integer shift left operation
;;
;; This module provides multiple test functions to validate i64.shl behavior
;; across various scenarios including shift count masking, overflow, and edge cases.
;; All functions test the 64-bit left shift operation with proper zero-fill behavior.

(module
  ;; Primary test function for i64.shl opcode
  ;; Takes value and shift count parameters and returns shifted result
  (func (export "test_i64_shl") (param $value i64) (param $shift_count i64) (result i64)
    ;; Load value onto stack
    local.get $value
    ;; Load shift count onto stack
    local.get $shift_count
    ;; Execute i64.shl opcode
    ;; Stack before: [..., value, shift_count]
    ;; Stack after:  [..., result]
    i64.shl
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for basic shift operations (small values)
  (func (export "test_basic_shift") (result i64)
    i64.const 5      ;; value = 5
    i64.const 3      ;; shift_count = 3
    i64.shl          ;; 5 << 3 = 40
  )

  ;; Test function for zero shift (identity property)
  (func (export "test_zero_shift") (param $x i64) (result i64)
    local.get $x
    i64.const 0      ;; shift by 0 (identity)
    i64.shl
  )

  ;; Test function for zero value shift (zero invariant)
  (func (export "test_shift_zero") (param $shift i64) (result i64)
    i64.const 0      ;; value = 0
    local.get $shift
    i64.shl          ;; 0 << n = 0
  )

  ;; Test function for maximum valid shift (63)
  (func (export "test_max_shift") (result i64)
    i64.const 1      ;; value = 1
    i64.const 63     ;; shift by 63
    i64.shl          ;; 1 << 63 = INT64_MIN (0x8000000000000000)
  )

  ;; Test function for shift count masking (shift by 64)
  (func (export "test_shift_mask_64") (result i64)
    i64.const 42     ;; value = 42
    i64.const 64     ;; shift by 64 (should be masked to 0)
    i64.shl          ;; 42 << 0 = 42 (due to masking)
  )

  ;; Test function for shift count masking (shift by 65)
  (func (export "test_shift_mask_65") (result i64)
    i64.const 100    ;; value = 100
    i64.const 65     ;; shift by 65 (should be masked to 1)
    i64.shl          ;; 100 << 1 = 200 (due to masking)
  )

  ;; Test function for overflow behavior (INT64_MAX shift)
  (func (export "test_overflow") (result i64)
    i64.const 0x7FFFFFFFFFFFFFFF  ;; INT64_MAX
    i64.const 1                   ;; shift by 1
    i64.shl                       ;; Should overflow to 0xFFFFFFFFFFFFFFFE
  )

  ;; Test function for negative value shift
  (func (export "test_negative_shift") (result i64)
    i64.const -1     ;; value = -1 (0xFFFFFFFFFFFFFFFF)
    i64.const 8      ;; shift by 8
    i64.shl          ;; -1 << 8 = 0xFFFFFFFFFFFFFF00
  )

  ;; Test function for power of 2 multiplication property
  (func (export "test_power_of_two") (result i64)
    i64.const 3      ;; value = 3
    i64.const 10     ;; shift by 10
    i64.shl          ;; 3 << 10 = 3 * 1024 = 3072
  )

  ;; Test function for 64-bit specific bit pattern
  (func (export "test_64bit_pattern") (result i64)
    i64.const 0x123456789ABCDEF0  ;; complex 64-bit pattern
    i64.const 4                   ;; shift by 4
    i64.shl                       ;; Should produce 0x23456789ABCDEF00
  )

  ;; Test function for all-ones pattern shift
  (func (export "test_all_ones") (result i64)
    i64.const 0xFFFFFFFFFFFFFFFF  ;; all ones (-1)
    i64.const 1                   ;; shift by 1
    i64.shl                       ;; Should produce 0xFFFFFFFFFFFFFFFE
  )

  ;; Test function for large shift count masking (128)
  (func (export "test_large_shift_128") (result i64)
    i64.const 50     ;; value = 50
    i64.const 128    ;; shift by 128 (should be masked to 0)
    i64.shl          ;; 50 << 0 = 50 (due to masking)
  )

  ;; Test function for large shift count masking (129)
  (func (export "test_large_shift_129") (result i64)
    i64.const 25     ;; value = 25
    i64.const 129    ;; shift by 129 (should be masked to 1)
    i64.shl          ;; 25 << 1 = 50 (due to masking)
  )

  ;; Test function for large shift count masking (192)
  (func (export "test_large_shift_192") (result i64)
    i64.const 75     ;; value = 75
    i64.const 192    ;; shift by 192 (192 & 63 = 0)
    i64.shl          ;; 75 << 0 = 75 (due to masking)
  )

  ;; Test function for negative shift count (treated as large positive)
  (func (export "test_negative_shift_count") (result i64)
    i64.const 1      ;; value = 1
    i64.const -1     ;; shift by -1 (0xFFFFFFFFFFFFFFFF & 63 = 63)
    i64.shl          ;; 1 << 63 = INT64_MIN (0x8000000000000000)
  )

  ;; Test function for INT64_MIN with small shift
  (func (export "test_int64_min_shift") (result i64)
    i64.const 0x8000000000000000  ;; INT64_MIN
    i64.const 1                   ;; shift by 1
    i64.shl                       ;; Should produce 0 (MSB shifted out)
  )

  ;; Test function for high-order bit preservation test
  (func (export "test_high_order_bits") (result i64)
    i64.const 0xF000000000000000  ;; high-order bits set
    i64.const 1                   ;; shift by 1
    i64.shl                       ;; Should produce 0xE000000000000000
  )

  ;; Test function for mathematical equivalence (x << n = x * 2^n)
  (func (export "test_math_equiv") (result i64)
    i64.const 1000   ;; value = 1000
    i64.const 6      ;; shift by 6
    i64.shl          ;; 1000 << 6 = 1000 * 64 = 64000
  )

  ;; Test function for single bit positions in 64-bit range
  (func (export "test_single_bit_32") (result i64)
    i64.const 1      ;; single bit
    i64.const 32     ;; shift to position 32
    i64.shl          ;; Should produce 0x0000000100000000
  )

  ;; Test function for single bit at position 48
  (func (export "test_single_bit_48") (result i64)
    i64.const 1      ;; single bit
    i64.const 48     ;; shift to position 48
    i64.shl          ;; Should produce 0x0001000000000000
  )

  ;; Test function for boundary at 2^62
  (func (export "test_boundary_2_62") (result i64)
    i64.const 0x4000000000000000  ;; 2^62
    i64.const 1                   ;; shift by 1
    i64.shl                       ;; Should produce INT64_MIN (0x8000000000000000)
  )

  ;; Test function for alternating bit pattern in 64-bit
  (func (export "test_alternating_pattern") (result i64)
    i64.const 0x5555555555555555  ;; alternating pattern
    i64.const 1                   ;; shift by 1
    i64.shl                       ;; Should produce 0xAAAAAAAAAAAAAAAA
  )

  ;; Test function for checkerboard pattern
  (func (export "test_checkerboard") (result i64)
    i64.const 0x3333333333333333  ;; checkerboard pattern
    i64.const 2                   ;; shift by 2
    i64.shl                       ;; Should produce 0xCCCCCCCCCCCCCCCC
  )

  ;; Test function for large value with moderate shift
  (func (export "test_large_value") (result i64)
    i64.const 0x123456789ABCDEF   ;; large value (no leading F)
    i64.const 8                   ;; shift by 8
    i64.shl                       ;; Should shift pattern left by 8 bits
  )
)