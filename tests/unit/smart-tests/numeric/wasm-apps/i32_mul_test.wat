;; WebAssembly Text Format (.wat) for i32.mul Opcode Testing
;; Comprehensive test module for 32-bit integer multiplication operation
;;
;; This module provides multiple test functions to validate i32.mul behavior
;; across various scenarios including overflow, boundary conditions, and mathematical properties.

(module
  ;; Primary test function for basic i32.mul opcode
  ;; Takes two i32 parameters and returns their product
  (func (export "i32_multiply") (param $operand1 i32) (param $operand2 i32) (result i32)
    ;; Load first operand onto stack
    local.get $operand1
    ;; Load second operand onto stack
    local.get $operand2
    ;; Execute i32.mul opcode
    ;; Stack before: [..., operand1, operand2]
    ;; Stack after:  [..., result]
    i32.mul
    ;; Return result (automatically consumed from stack)
  )

  ;; Boundary condition test function for i32.mul with overflow scenarios
  ;; Handles INT32_MAX/MIN boundary multiplication cases
  (func (export "i32_multiply_boundary") (param $operand1 i32) (param $operand2 i32) (result i32)
    ;; Load first operand onto stack
    local.get $operand1
    ;; Load second operand onto stack
    local.get $operand2
    ;; Execute i32.mul opcode with potential overflow
    ;; Overflow results in 2's complement wraparound behavior
    i32.mul
    ;; Return wrapped result
  )

  ;; Mathematical properties test function for i32.mul
  ;; Tests zero absorption, identity, and negation properties
  (func (export "i32_multiply_properties") (param $operand1 i32) (param $operand2 i32) (result i32)
    ;; Load first operand onto stack
    local.get $operand1
    ;; Load second operand onto stack
    local.get $operand2
    ;; Execute i32.mul opcode for property validation
    i32.mul
    ;; Return result demonstrating mathematical properties
  )

  ;; Test function for zero multiplication (absorption property)
  (func (export "test_zero_multiply") (param $x i32) (result i32)
    local.get $x
    i32.const 0
    i32.mul  ;; Should always return 0 regardless of $x value
  )

  ;; Test function for identity multiplication (n * 1 = n)
  (func (export "test_identity_multiply") (param $x i32) (result i32)
    local.get $x
    i32.const 1
    i32.mul  ;; Should return original value $x
  )

  ;; Test function for negation through multiplication (n * -1 = -n)
  (func (export "test_negation_multiply") (param $x i32) (result i32)
    local.get $x
    i32.const -1
    i32.mul  ;; Should return negated value of $x
  )

  ;; Test function for INT32_MAX overflow (INT32_MAX * 2)
  (func (export "test_max_overflow") (result i32)
    i32.const 0x7FFFFFFF  ;; INT32_MAX (2147483647)
    i32.const 2
    i32.mul               ;; Should wrap to -2
  )

  ;; Test function for INT32_MIN negation (INT32_MIN * -1)
  (func (export "test_min_negation") (result i32)
    i32.const 0x80000000  ;; INT32_MIN (-2147483648)
    i32.const -1
    i32.mul               ;; Should remain INT32_MIN due to overflow
  )

  ;; Test function for large value multiplication with wraparound
  (func (export "test_large_multiply") (result i32)
    i32.const 46341       ;; Large value that causes overflow when squared
    i32.const 46341
    i32.mul               ;; Should wrap to -2147479015
  )

  ;; Test function for commutative property validation (a * b = b * a)
  (func (export "test_commutative_multiply") (param $a i32) (param $b i32) (result i32)
    ;; Calculate a * b
    local.get $a
    local.get $b
    i32.mul
    ;; Store result temporarily (for validation purposes, return a * b)
    ;; In actual tests, we'll call this function with swapped parameters
    ;; to verify commutative property externally
  )

  ;; Test function for positive-negative multiplication
  (func (export "test_mixed_sign_multiply") (result i32)
    i32.const 15          ;; Positive number
    i32.const -3          ;; Negative number
    i32.mul               ;; Should return -45
  )

  ;; Test function for negative-negative multiplication
  (func (export "test_negative_multiply") (result i32)
    i32.const -8          ;; First negative number
    i32.const -4          ;; Second negative number
    i32.mul               ;; Should return 32 (negative * negative = positive)
  )
)