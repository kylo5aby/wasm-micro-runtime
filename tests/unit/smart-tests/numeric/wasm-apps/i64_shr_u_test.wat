;; i64_shr_u_test.wat - Comprehensive test module for i64.shr_u opcode
;; Tests logical (unsigned) right shift operation for 64-bit integers
;; Validates basic operations, boundary conditions, and edge cases

(module
  ;; Test function: Basic i64.shr_u operation
  ;; Takes two i64 parameters: value to shift and shift count
  ;; Returns i64 result of logical right shift
  (func (export "i64_shr_u_test") (param i64) (param i64) (result i64)
    local.get 0    ;; Load first parameter (value)
    local.get 1    ;; Load second parameter (shift count)
    i64.shr_u      ;; Perform unsigned right shift
  )

  ;; Test function: Identity operation (shift by 0)
  (func (export "i64_shr_u_identity") (param i64) (result i64)
    local.get 0    ;; Load parameter
    i64.const 0    ;; Shift count of 0
    i64.shr_u      ;; Should return original value
  )

  ;; Test function: Maximum shift (63 positions)
  (func (export "i64_shr_u_max_shift") (param i64) (result i64)
    local.get 0    ;; Load parameter
    i64.const 63   ;; Maximum valid shift count
    i64.shr_u      ;; Should leave only LSB for non-zero values
  )

  ;; Test function: Overflow shift behavior (shift by 64)
  (func (export "i64_shr_u_overflow") (param i64) (result i64)
    local.get 0    ;; Load parameter
    i64.const 64   ;; Shift count >= 64 (should behave as modulo 64)
    i64.shr_u      ;; Should behave as shift by 0
  )

  ;; Test function: Large shift count behavior
  (func (export "i64_shr_u_large_shift") (param i64) (result i64)
    local.get 0    ;; Load parameter
    i64.const 128  ;; Large shift count (128 % 64 = 0)
    i64.shr_u      ;; Should behave as shift by 0
  )

  ;; Test function: Zero value with various shift counts
  (func (export "i64_shr_u_zero_value") (param i64) (result i64)
    i64.const 0    ;; Zero value
    local.get 0    ;; Shift count parameter
    i64.shr_u      ;; Should always produce zero
  )

  ;; Test function: Max value (all bits set) shifted
  (func (export "i64_shr_u_max_value") (param i64) (result i64)
    i64.const -1   ;; 0xFFFFFFFFFFFFFFFF (all bits set)
    local.get 0    ;; Shift count parameter
    i64.shr_u      ;; Test zero-fill behavior
  )

  ;; Test function: Single high bit shifted
  (func (export "i64_shr_u_high_bit") (param i64) (result i64)
    i64.const 0x8000000000000000  ;; Single high bit set
    local.get 0    ;; Shift count parameter
    i64.shr_u      ;; Test zero-fill vs sign extension
  )

  ;; Test function: Alternating bit pattern
  (func (export "i64_shr_u_alternating") (param i64) (result i64)
    i64.const 0xAAAAAAAAAAAAAAAA  ;; Alternating bit pattern
    local.get 0    ;; Shift count parameter
    i64.shr_u      ;; Test pattern preservation
  )

  ;; Test function: Mathematical equivalence (division by powers of 2)
  (func (export "i64_shr_u_divide_equivalent") (param i64) (param i64) (result i64)
    local.get 0    ;; Value to shift
    local.get 1    ;; Shift count (should be equivalent to division by 2^shift_count)
    i64.shr_u      ;; Perform shift operation
  )
)