(module
  ;; i64.lt_u Test Module
  ;;
  ;; This module provides comprehensive test functions for the i64.lt_u WebAssembly opcode.
  ;; The i64.lt_u instruction performs unsigned 64-bit integer less-than comparison.
  ;; It pops two i64 values from the stack and pushes 1 (i32) if first < second (unsigned),
  ;; 0 (i32) otherwise.
  ;;
  ;; Functions exported:
  ;; - i64_lt_u_test: Basic unsigned less-than comparison
  ;; - test_zero_comparisons: Specialized tests for zero boundary cases
  ;; - test_boundary_values: Tests for UINT64_MAX and boundary conditions
  ;; - test_signed_unsigned_diff: Tests values that differ in signed vs unsigned interpretation
  ;;
  ;; Unsigned comparison logic:
  ;; - All values treated as positive unsigned integers (0 to UINT64_MAX)
  ;; - 0x8000000000000000 to 0xFFFFFFFFFFFFFFFF are large positive values, not negative
  ;; - Comparison: first < second returns 1 if true, 0 if false

  ;; Basic i64.lt_u operation test function
  ;; Takes two i64 inputs, applies i64.lt_u, and returns the comparison result
  ;; Tests the fundamental unsigned less-than behavior
  (func (export "i64_lt_u_test") (param $first i64) (param $second i64) (result i32)
    ;; Load first parameter onto stack
    local.get $first

    ;; Load second parameter onto stack
    local.get $second

    ;; Apply i64.lt_u operation
    ;; This instruction:
    ;; 1. Pops second operand from stack (top of stack)
    ;; 2. Pops first operand from stack (second from top)
    ;; 3. Compares first < second using unsigned integer semantics
    ;; 4. Pushes 1 (i32) if comparison is true, 0 (i32) if false
    i64.lt_u

    ;; Return the comparison result (i32: 1 for true, 0 for false)
  )

  ;; Zero boundary comparison tests
  ;; Tests critical cases involving zero as operand
  (func (export "test_zero_comparisons") (param $selector i32) (result i32)
    (if (result i32) (i32.eq (local.get $selector) (i32.const 0))
      (then
        ;; Test: 0 < 1 (should be true = 1)
        i64.const 0
        i64.const 1
        i64.lt_u
      )
      (else
        (if (result i32) (i32.eq (local.get $selector) (i32.const 1))
          (then
            ;; Test: 1 < 0 (should be false = 0)
            i64.const 1
            i64.const 0
            i64.lt_u
          )
          (else
            (if (result i32) (i32.eq (local.get $selector) (i32.const 2))
              (then
                ;; Test: 0 < 0 (should be false = 0)
                i64.const 0
                i64.const 0
                i64.lt_u
              )
              (else
                (if (result i32) (i32.eq (local.get $selector) (i32.const 3))
                  (then
                    ;; Test: 0 < UINT64_MAX (should be true = 1)
                    i64.const 0
                    i64.const 0xFFFFFFFFFFFFFFFF
                    i64.lt_u
                  )
                  (else
                    ;; Default case
                    i32.const 0
                  )
                )
              )
            )
          )
        )
      )
    )
  )

  ;; Boundary value comparison tests
  ;; Tests maximum values and critical boundaries
  (func (export "test_boundary_values") (param $selector i32) (result i32)
    (if (result i32) (i32.eq (local.get $selector) (i32.const 0))
      (then
        ;; Test: (UINT64_MAX - 1) < UINT64_MAX (should be true = 1)
        i64.const 0xFFFFFFFFFFFFFFFE
        i64.const 0xFFFFFFFFFFFFFFFF
        i64.lt_u
      )
      (else
        (if (result i32) (i32.eq (local.get $selector) (i32.const 1))
          (then
            ;; Test: UINT64_MAX < (UINT64_MAX - 1) (should be false = 0)
            i64.const 0xFFFFFFFFFFFFFFFF
            i64.const 0xFFFFFFFFFFFFFFFE
            i64.lt_u
          )
          (else
            (if (result i32) (i32.eq (local.get $selector) (i32.const 2))
              (then
                ;; Test: UINT64_MAX < UINT64_MAX (should be false = 0)
                i64.const 0xFFFFFFFFFFFFFFFF
                i64.const 0xFFFFFFFFFFFFFFFF
                i64.lt_u
              )
              (else
                (if (result i32) (i32.eq (local.get $selector) (i32.const 3))
                  (then
                    ;; Test: 0x7FFFFFFFFFFFFFFF < 0x8000000000000000 (should be true = 1 in unsigned)
                    i64.const 0x7FFFFFFFFFFFFFFF
                    i64.const 0x8000000000000000
                    i64.lt_u
                  )
                  (else
                    ;; Default case
                    i32.const 0
                  )
                )
              )
            )
          )
        )
      )
    )
  )

  ;; Signed vs unsigned interpretation difference tests
  ;; Tests values that would behave differently in signed comparison
  (func (export "test_signed_unsigned_diff") (param $selector i32) (result i32)
    (if (result i32) (i32.eq (local.get $selector) (i32.const 0))
      (then
        ;; Test: 0xFFFFFFFFFFFFFFFF < 1 (unsigned: false = 0, signed would be true)
        i64.const 0xFFFFFFFFFFFFFFFF
        i64.const 1
        i64.lt_u
      )
      (else
        (if (result i32) (i32.eq (local.get $selector) (i32.const 1))
          (then
            ;; Test: 0xFFFFFFFFFFFFFFFE < 0xFFFFFFFFFFFFFFFF (unsigned: true = 1)
            i64.const 0xFFFFFFFFFFFFFFFE
            i64.const 0xFFFFFFFFFFFFFFFF
            i64.lt_u
          )
          (else
            (if (result i32) (i32.eq (local.get $selector) (i32.const 2))
              (then
                ;; Test: 0x8000000000000000 < 0x7FFFFFFFFFFFFFFF (unsigned: false = 0, signed would be true)
                i64.const 0x8000000000000000
                i64.const 0x7FFFFFFFFFFFFFFF
                i64.lt_u
              )
              (else
                (if (result i32) (i32.eq (local.get $selector) (i32.const 3))
                  (then
                    ;; Test: 0x8000000000000001 < 0xFFFFFFFFFFFFFFFF (unsigned: true = 1)
                    i64.const 0x8000000000000001
                    i64.const 0xFFFFFFFFFFFFFFFF
                    i64.lt_u
                  )
                  (else
                    ;; Default case
                    i32.const 0
                  )
                )
              )
            )
          )
        )
      )
    )
  )

  ;; Anti-reflexive property test function
  ;; Tests that any value compared with itself returns false
  (func (export "test_anti_reflexive") (param $value i64) (result i32)
    ;; Load value twice for self-comparison
    local.get $value
    local.get $value

    ;; Apply i64.lt_u - should always return 0 for self-comparison
    i64.lt_u
  )

  ;; Transitive property validation helper
  ;; Tests transitivity: if a < b and b < c, then a < c
  (func (export "test_transitivity") (param $a i64) (param $b i64) (param $c i64) (result i32)
    ;; We expect: a < b, b < c, therefore a < c
    ;; This function tests the conclusion: a < c
    local.get $a
    local.get $c
    i64.lt_u
  )

  ;; Basic arithmetic sequence test
  ;; Tests comparison with simple incremental values
  (func (export "test_arithmetic_sequence") (param $selector i32) (result i32)
    (if (result i32) (i32.eq (local.get $selector) (i32.const 0))
      (then
        ;; Test: 100 < 200 (should be true = 1)
        i64.const 100
        i64.const 200
        i64.lt_u
      )
      (else
        (if (result i32) (i32.eq (local.get $selector) (i32.const 1))
          (then
            ;; Test: 200 < 100 (should be false = 0)
            i64.const 200
            i64.const 100
            i64.lt_u
          )
          (else
            (if (result i32) (i32.eq (local.get $selector) (i32.const 2))
              (then
                ;; Test: 1000 < 2000 (should be true = 1)
                i64.const 1000
                i64.const 2000
                i64.lt_u
              )
              (else
                (if (result i32) (i32.eq (local.get $selector) (i32.const 3))
                  (then
                    ;; Test: 0x8000000000000000 < 0xFFFFFFFFFFFFFFFF (should be true = 1)
                    i64.const 0x8000000000000000
                    i64.const 0xFFFFFFFFFFFFFFFF
                    i64.lt_u
                  )
                  (else
                    ;; Default case
                    i32.const 0
                  )
                )
              )
            )
          )
        )
      )
    )
  )
)