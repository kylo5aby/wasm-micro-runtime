;; WebAssembly Text Format (WAT) for f64.div opcode testing
;;
;; This module provides comprehensive test functions for the f64.div operation,
;; which implements IEEE 754 double-precision floating-point division.
;; The f64.div operation takes two f64 operands (dividend, divisor) and returns
;; their quotient, handling all special cases according to IEEE 754-2008 standard.
;;
;; Test Functions:
;; - test_f64_div: Basic f64.div operation wrapper
;;
;; WebAssembly f64.div specification:
;; - Stack behavior: [..., f64_dividend, f64_divisor] -> [..., f64_quotient]
;; - IEEE 754 compliance: Follows IEEE 754-2008 division operation semantics
;; - Never traps: Always produces a result for any f64 input combination
;; - Special handling: Division by zero → infinity, 0/0 → NaN, ∞/∞ → NaN

(module
  ;; Export the main test function for f64.div operation
  (export "test_f64_div" (func $test_f64_div))

  ;; Basic f64.div operation test function
  ;;
  ;; Parameters:
  ;;   $dividend (f64): First operand - numerator value
  ;;   $divisor (f64): Second operand - denominator value
  ;;
  ;; Returns:
  ;;   f64: Result of f64.div($dividend, $divisor)
  ;;
  ;; This function directly exposes the f64.div WebAssembly instruction
  ;; for testing all combinations of IEEE 754 double-precision values including:
  ;; - Normal finite values (positive/negative combinations)
  ;; - Special cases: division by zero (finite/±0.0 → ±∞)
  ;; - Indeterminate forms: 0.0/0.0 → NaN, ∞/∞ → NaN
  ;; - Infinity operations: ∞/finite → ±∞, finite/∞ → ±0.0
  ;; - NaN propagation: any operation with NaN → NaN
  ;; - Boundary conditions: DBL_MAX, DBL_MIN, denormalized values
  ;; - Overflow/underflow: large/small → ∞, small/large → subnormal/0.0
  (func $test_f64_div (param $dividend f64) (param $divisor f64) (result f64)
    ;; Load both parameters onto the stack
    local.get $dividend      ;; Stack: [dividend]
    local.get $divisor       ;; Stack: [dividend, divisor]

    ;; Execute f64.div instruction
    ;; This performs IEEE 754 double-precision division: dividend ÷ divisor
    ;; The operation handles all special cases without trapping:
    ;; - finite/0.0 → ±∞ (sign depends on operand signs)
    ;; - 0.0/0.0 → NaN (indeterminate form)
    ;; - ∞/∞ → NaN (indeterminate form)
    ;; - finite/∞ → ±0.0 (signed zero based on signs)
    ;; - ∞/finite → ±∞ (signed infinity based on signs)
    ;; - NaN operand → NaN (NaN propagation)
    f64.div                  ;; Stack: [quotient]

    ;; Return the quotient result
    ;; The result maintains IEEE 754 compliance for all input combinations
  )

  ;; Memory section (minimal - not needed for f64.div but included for completeness)
  (memory 1)
)