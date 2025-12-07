;; WebAssembly Text Format (WAT) for f64.sub opcode testing
;;
;; This module provides comprehensive test functions for the f64.sub operation,
;; which implements IEEE 754 double-precision floating-point subtraction.
;; The f64.sub operation takes two f64 operands (minuend, subtrahend) and returns
;; their difference, handling all special cases according to IEEE 754-2008 standard.
;;
;; Test Functions:
;; - test_f64_sub: Basic f64.sub operation wrapper
;; - test_f64_sub_boundary: Boundary value operations for overflow/underflow testing
;; - test_f64_sub_special: Special IEEE 754 values (NaN, infinity, signed zeros)
;;
;; WebAssembly f64.sub specification:
;; - Stack behavior: [..., f64_minuend, f64_subtrahend] -> [..., f64_difference]
;; - IEEE 754 compliance: Follows IEEE 754-2008 subtraction operation semantics
;; - Never traps: Always produces a result for any f64 input combination
;; - Special handling: Handles overflow to ±∞, signed zero arithmetic, NaN propagation

(module
  ;; Export all test functions for comprehensive f64.sub validation
  (export "test_f64_sub" (func $test_f64_sub))
  (export "test_f64_sub_boundary" (func $test_f64_sub_boundary))
  (export "test_f64_sub_special" (func $test_f64_sub_special))

  ;; Basic f64.sub operation test function
  ;;
  ;; Parameters:
  ;;   $minuend (f64): First operand - value to subtract from
  ;;   $subtrahend (f64): Second operand - value to subtract
  ;;
  ;; Returns:
  ;;   f64: Result of f64.sub($minuend, $subtrahend) = minuend - subtrahend
  ;;
  ;; This function directly exposes the f64.sub WebAssembly instruction
  ;; for testing all combinations of IEEE 754 double-precision values including:
  ;; - Normal finite values (positive/negative combinations)
  ;; - Mixed sign operations: pos-neg, neg-pos, pos-pos, neg-neg
  ;; - Basic arithmetic: 10.5-3.2=7.3, -10.5-(-3.2)=-7.3
  ;; - Identity operations and perfect cancellation scenarios
  (func $test_f64_sub (param $minuend f64) (param $subtrahend f64) (result f64)
    ;; Load both parameters onto the stack
    local.get $minuend       ;; Stack: [minuend]
    local.get $subtrahend    ;; Stack: [minuend, subtrahend]

    ;; Execute f64.sub instruction
    ;; This performs IEEE 754 double-precision subtraction: minuend - subtrahend
    ;; The operation handles all arithmetic cases without trapping:
    ;; - Standard arithmetic: finite values produce expected mathematical result
    ;; - Sign preservation: correct handling of positive/negative operands
    ;; - Perfect cancellation: x - x = +0.0 (always positive zero)
    ;; - Zero operations: x - 0.0 = x, 0.0 - x = -x
    f64.sub                  ;; Stack: [difference]

    ;; Return the difference result
    ;; The result maintains IEEE 754 compliance for all finite input combinations
  )

  ;; Boundary value f64.sub operation test function for overflow/underflow scenarios
  ;;
  ;; Parameters:
  ;;   $minuend (f64): First operand - potentially boundary value (DBL_MAX, DBL_MIN)
  ;;   $subtrahend (f64): Second operand - potentially boundary value for overflow testing
  ;;
  ;; Returns:
  ;;   f64: Result of boundary f64.sub operation with overflow/underflow handling
  ;;
  ;; This function tests f64.sub behavior at representational boundaries including:
  ;; - Overflow scenarios: DBL_MAX - (-DBL_MAX) → +∞
  ;; - Underflow scenarios: (-DBL_MAX) - DBL_MAX → -∞
  ;; - Boundary arithmetic: operations near DBL_MAX and DBL_MIN limits
  ;; - Denormalized results: operations producing subnormal values
  (func $test_f64_sub_boundary (param $minuend f64) (param $subtrahend f64) (result f64)
    ;; Load parameters for boundary testing
    local.get $minuend       ;; Stack: [minuend]
    local.get $subtrahend    ;; Stack: [minuend, subtrahend]

    ;; Execute f64.sub with potential overflow/underflow
    ;; IEEE 754 behavior for boundary conditions:
    ;; - Result > DBL_MAX → +∞ (positive overflow)
    ;; - Result < -DBL_MAX → -∞ (negative overflow/underflow)
    ;; - |Result| < DBL_MIN → denormalized number or signed zero
    ;; - Gradual underflow: smooth transition to denormalized values
    f64.sub                  ;; Stack: [boundary_result]

    ;; Return boundary operation result
    ;; May be finite, ±∞, or denormalized based on input magnitude
  )

  ;; Special value f64.sub operation test function for IEEE 754 edge cases
  ;;
  ;; Parameters:
  ;;   $minuend (f64): First operand - may be NaN, ±∞, ±0.0, or special value
  ;;   $subtrahend (f64): Second operand - may be NaN, ±∞, ±0.0, or special value
  ;;
  ;; Returns:
  ;;   f64: Result of special value f64.sub operation per IEEE 754 rules
  ;;
  ;; This function tests f64.sub behavior with IEEE 754 special values:
  ;; - NaN propagation: any operation with NaN → NaN
  ;; - Infinity arithmetic: ∞ - ∞ → NaN, ∞ - finite → ∞, finite - ∞ → ∞
  ;; - Signed zero handling: (+0.0) - (-0.0) → +0.0, (-0.0) - (+0.0) → -0.0
  ;; - Mixed special cases: combining different special value types
  (func $test_f64_sub_special (param $minuend f64) (param $subtrahend f64) (result f64)
    ;; Load special value parameters
    local.get $minuend       ;; Stack: [minuend]
    local.get $subtrahend    ;; Stack: [minuend, subtrahend]

    ;; Execute f64.sub with special IEEE 754 value handling
    ;; Special case behaviors according to IEEE 754-2008:
    ;; - NaN + anything → NaN (NaN propagation)
    ;; - (+∞) - (+∞) → NaN (indeterminate form)
    ;; - (+∞) - (-∞) → +∞ (infinity minus negative infinity)
    ;; - (-∞) - (+∞) → -∞ (negative infinity minus infinity)
    ;; - (±∞) - finite → ±∞ (infinity dominates)
    ;; - finite - (±∞) → ∓∞ (result opposite of infinity sign)
    ;; - (+0.0) - (-0.0) → +0.0 (signed zero subtraction)
    ;; - (-0.0) - (+0.0) → -0.0 (signed zero subtraction)
    f64.sub                  ;; Stack: [special_result]

    ;; Return special value operation result
    ;; Result follows IEEE 754 special value arithmetic rules
  )

  ;; Memory section (minimal - not needed for f64.sub but included for completeness)
  (memory 1)
)