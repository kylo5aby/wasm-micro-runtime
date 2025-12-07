;; WebAssembly Text Format (WAT) for f64.mul opcode testing
;;
;; This module provides comprehensive test functions for the f64.mul operation,
;; which implements IEEE 754 double-precision floating-point multiplication.
;; The f64.mul operation takes two f64 operands (a, b) and returns their product,
;; handling all special cases according to IEEE 754-2008 standard.
;;
;; Test Functions:
;; - test_f64_mul: Basic f64.mul operation wrapper
;;
;; WebAssembly f64.mul specification:
;; - Stack behavior: [..., f64_a, f64_b] -> [..., f64_product]
;; - IEEE 754 compliance: Follows IEEE 754-2008 multiplication operation semantics
;; - Never traps: Always produces a result for any f64 input combination
;; - Special handling: 0×∞ → NaN, ∞×finite → ±∞, NaN propagation

(module
  ;; Export the main test function for f64.mul operation
  (export "test_f64_mul" (func $test_f64_mul))

  ;; Basic f64.mul operation test function
  ;;
  ;; Parameters:
  ;;   $a (f64): First operand - left multiplicand
  ;;   $b (f64): Second operand - right multiplicand
  ;;
  ;; Returns:
  ;;   f64: Result of f64.mul($a, $b)
  ;;
  ;; This function directly exposes the f64.mul WebAssembly instruction
  ;; for testing all combinations of IEEE 754 double-precision values including:
  ;; - Normal finite values (positive/negative combinations)
  ;; - Special cases: multiplication by zero (finite×±0.0 → ±0.0)
  ;; - Indeterminate forms: 0.0×∞ → NaN, ∞×0.0 → NaN
  ;; - Infinity operations: ∞×finite → ±∞, ∞×∞ → ±∞
  ;; - NaN propagation: any operation with NaN → NaN
  ;; - Identity cases: value×1.0 → value, value×-1.0 → -value
  ;; - Boundary conditions: DBL_MAX, DBL_MIN, denormalized values
  ;; - Overflow/underflow: large×large → ∞, small×small → subnormal/0.0
  (func $test_f64_mul (param $a f64) (param $b f64) (result f64)
    ;; Load both parameters onto the stack
    local.get $a             ;; Stack: [a]
    local.get $b             ;; Stack: [a, b]

    ;; Execute f64.mul instruction
    ;; This performs IEEE 754 double-precision multiplication: a × b
    ;; The operation handles all special cases without trapping:
    ;; - finite×0.0 → ±0.0 (sign depends on operand signs)
    ;; - 0.0×∞ → NaN (indeterminate form)
    ;; - ∞×0.0 → NaN (indeterminate form)
    ;; - finite×∞ → ±∞ (signed infinity based on signs)
    ;; - ∞×finite → ±∞ (signed infinity based on signs)
    ;; - ∞×∞ → ±∞ (signed infinity based on signs)
    ;; - NaN operand → NaN (NaN propagation)
    ;; - Overflow: large×large → ±∞
    ;; - Underflow: small×small → subnormal or ±0.0
    f64.mul                  ;; Stack: [product]

    ;; Return the product result
    ;; The result maintains IEEE 754 compliance for all input combinations
  )

  ;; Memory section (minimal - not needed for f64.mul but included for completeness)
  (memory 1)
)