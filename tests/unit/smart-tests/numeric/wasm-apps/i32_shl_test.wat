;; WebAssembly Text Format (.wat) for i32.shl Opcode Testing
;; Comprehensive test module for 32-bit integer shift left operation
;;
;; This module provides multiple test functions to validate i32.shl behavior
;; across various scenarios including shift count masking, overflow, and edge cases.

(module
  ;; Primary test function for i32.shl opcode
  ;; Takes value and shift count parameters and returns shifted result
  (func (export "test_i32_shl") (param $value i32) (param $shift_count i32) (result i32)
    ;; Load value onto stack
    local.get $value
    ;; Load shift count onto stack
    local.get $shift_count
    ;; Execute i32.shl opcode
    ;; Stack before: [..., value, shift_count]
    ;; Stack after:  [..., result]
    i32.shl
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for basic shift operations (small values)
  (func (export "test_basic_shift") (result i32)
    i32.const 5      ;; value = 5
    i32.const 2      ;; shift_count = 2
    i32.shl          ;; 5 << 2 = 20
  )

  ;; Test function for zero shift (identity property)
  (func (export "test_zero_shift") (param $x i32) (result i32)
    local.get $x
    i32.const 0      ;; shift by 0 (identity)
    i32.shl
  )

  ;; Test function for zero value shift (zero invariant)
  (func (export "test_shift_zero") (param $shift i32) (result i32)
    i32.const 0      ;; value = 0
    local.get $shift
    i32.shl          ;; 0 << n = 0
  )

  ;; Test function for maximum valid shift (31)
  (func (export "test_max_shift") (result i32)
    i32.const 1      ;; value = 1
    i32.const 31     ;; shift by 31
    i32.shl          ;; 1 << 31 = INT32_MIN (0x80000000)
  )

  ;; Test function for shift count masking (shift by 32)
  (func (export "test_shift_mask_32") (result i32)
    i32.const 5      ;; value = 5
    i32.const 32     ;; shift by 32 (should be masked to 0)
    i32.shl          ;; 5 << 0 = 5 (due to masking)
  )

  ;; Test function for shift count masking (shift by 33)
  (func (export "test_shift_mask_33") (result i32)
    i32.const 7      ;; value = 7
    i32.const 33     ;; shift by 33 (should be masked to 1)
    i32.shl          ;; 7 << 1 = 14 (due to masking)
  )

  ;; Test function for overflow behavior (INT32_MAX shift)
  (func (export "test_overflow") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.const 1           ;; shift by 1
    i32.shl               ;; Should overflow to -2 (0xFFFFFFFE)
  )

  ;; Test function for negative value shift
  (func (export "test_negative_shift") (result i32)
    i32.const -1     ;; value = -1 (0xFFFFFFFF)
    i32.const 4      ;; shift by 4
    i32.shl          ;; -1 << 4 = 0xFFFFFFF0 (-16)
  )

  ;; Test function for power of 2 multiplication property
  (func (export "test_power_of_two") (result i32)
    i32.const 3      ;; value = 3
    i32.const 3      ;; shift by 3
    i32.shl          ;; 3 << 3 = 3 * 8 = 24
  )

  ;; Test function for alternating bit pattern
  (func (export "test_bit_pattern") (result i32)
    i32.const 0x55555555  ;; alternating bit pattern
    i32.const 1           ;; shift by 1
    i32.shl               ;; Should produce 0xAAAAAAAA
  )

  ;; Test function for all-ones pattern shift
  (func (export "test_all_ones") (result i32)
    i32.const 0xFFFFFFFF  ;; all ones (-1)
    i32.const 8           ;; shift by 8
    i32.shl               ;; Should produce 0xFFFFFF00
  )

  ;; Test function for large shift count masking
  (func (export "test_large_shift") (result i32)
    i32.const 42     ;; value = 42
    i32.const 63     ;; shift by 63 (should be masked to 31)
    i32.shl          ;; 42 << 31 should produce a large negative number
  )

  ;; Test function for boundary value at 2^30
  (func (export "test_boundary_2_30") (result i32)
    i32.const 0x40000000  ;; 2^30 (1073741824)
    i32.const 1           ;; shift by 1
    i32.shl               ;; Should produce INT32_MIN (0x80000000)
  )

  ;; Test function for single bit positions
  (func (export "test_single_bit") (result i32)
    i32.const 1      ;; single bit
    i32.const 16     ;; shift to position 16
    i32.shl          ;; Should produce 0x00010000 (65536)
  )

  ;; Test function for mathematical equivalence (x << n = x * 2^n)
  (func (export "test_math_equiv") (result i32)
    i32.const 100    ;; value = 100
    i32.const 4      ;; shift by 4
    i32.shl          ;; 100 << 4 = 100 * 16 = 1600
  )
)