;; WebAssembly Text Format (.wat) for i32.sub Opcode Testing
;; Comprehensive test module for 32-bit integer subtraction operation
;;
;; This module provides multiple test functions to validate i32.sub behavior
;; across various scenarios including underflow, overflow, and edge cases.
;;
;; Test Coverage:
;; - Basic subtraction with positive/negative operands
;; - Boundary value subtraction near INT32_MIN/INT32_MAX
;; - Wrap-around behavior for underflow/overflow scenarios
;; - Zero operand handling and identity properties
;; - Mathematical property validation (non-commutativity)

(module
  ;; Primary test function for i32.sub opcode
  ;; Takes two i32 parameters (minuend, subtrahend) and returns their difference
  (func (export "test_i32_sub") (param $minuend i32) (param $subtrahend i32) (result i32)
    ;; Load first operand (minuend) onto stack
    local.get $minuend
    ;; Load second operand (subtrahend) onto stack
    local.get $subtrahend
    ;; Execute i32.sub opcode: minuend - subtrahend
    ;; Stack before: [..., minuend, subtrahend]
    ;; Stack after:  [..., result]
    i32.sub
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for zero subtraction (identity property: x - 0 = x)
  (func (export "test_zero_sub_identity") (param $x i32) (result i32)
    local.get $x
    i32.const 0
    i32.sub
  )

  ;; Test function for subtraction from zero (negation property: 0 - x = -x)
  (func (export "test_zero_sub_negation") (param $x i32) (result i32)
    i32.const 0
    local.get $x
    i32.sub
  )

  ;; Test function for self-subtraction (x - x = 0)
  (func (export "test_self_sub") (param $x i32) (result i32)
    local.get $x
    local.get $x
    i32.sub
  )

  ;; Test function for underflow behavior (INT32_MIN - 1)
  ;; Expected: -2147483648 - 1 = 2147483647 (wraps to INT32_MAX)
  (func (export "test_underflow") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.const 1
    i32.sub               ;; Should wrap to INT32_MAX (2147483647)
  )

  ;; Test function for extreme underflow (INT32_MIN - INT32_MAX)
  ;; Expected: -2147483648 - 2147483647 = 1 (wraps around)
  (func (export "test_extreme_underflow") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.sub               ;; Should wrap to 1
  )

  ;; Test function for overflow behavior (INT32_MAX - INT32_MIN)
  ;; Expected: 2147483647 - (-2147483648) = -1 (overflows and wraps)
  (func (export "test_overflow") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.sub               ;; Should wrap to -1
  )

  ;; Test function for boundary subtraction near maximum
  ;; Expected: INT32_MAX - 1 = 2147483646 (no overflow)
  (func (export "test_boundary_max") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.const 1
    i32.sub               ;; Should equal 2147483646
  )

  ;; Test function for boundary subtraction near minimum
  ;; Expected: INT32_MIN - (-1) = INT32_MIN + 1 = -2147483647 (no underflow)
  (func (export "test_boundary_min") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.const -1          ;; -1
    i32.sub               ;; Should equal -2147483647
  )

  ;; Test function for large positive minus large negative
  ;; Expected: Large positive - large negative might cause overflow
  (func (export "test_large_pos_minus_neg") (result i32)
    i32.const 1000000000  ;; Large positive
    i32.const -1000000000 ;; Large negative
    i32.sub               ;; Should equal 2000000000 (within range)
  )

  ;; Test function for large negative minus large positive
  ;; Expected: Large negative - large positive causes underflow
  (func (export "test_large_neg_minus_pos") (result i32)
    i32.const -1500000000 ;; Large negative
    i32.const 1000000000  ;; Large positive
    i32.sub               ;; Should wrap around due to underflow
  )

  ;; Test function for non-commutativity validation
  ;; Returns array-like results for a - b and b - a comparison
  (func (export "test_non_commutative_1") (result i32)
    i32.const 15
    i32.const 7
    i32.sub               ;; 15 - 7 = 8
  )

  (func (export "test_non_commutative_2") (result i32)
    i32.const 7
    i32.const 15
    i32.sub               ;; 7 - 15 = -8
  )

  ;; Test function for unit subtraction (decrement operation)
  (func (export "test_unit_sub") (param $x i32) (result i32)
    local.get $x
    i32.const 1
    i32.sub
  )

  ;; Test function for zero boundary crossing
  ;; Tests positive to negative transition
  (func (export "test_zero_crossing_pos") (result i32)
    i32.const 5
    i32.const 10
    i32.sub               ;; 5 - 10 = -5 (crosses zero)
  )

  ;; Test function for zero boundary crossing
  ;; Tests negative to positive transition
  (func (export "test_zero_crossing_neg") (result i32)
    i32.const -5
    i32.const -10
    i32.sub               ;; -5 - (-10) = 5 (crosses zero)
  )

  ;; Test function for medium range subtraction
  (func (export "test_medium_range") (result i32)
    i32.const 1000000
    i32.const 300000
    i32.sub               ;; 1000000 - 300000 = 700000
  )

  ;; Test function simulating inverse addition property: (a + b) - b = a
  ;; We test: 22 - 7 = 15 (where 22 could be result of 15 + 7)
  (func (export "test_inverse_addition") (result i32)
    i32.const 22          ;; Simulated sum result
    i32.const 7           ;; Original second operand
    i32.sub               ;; Should return original first operand: 15
  )
)