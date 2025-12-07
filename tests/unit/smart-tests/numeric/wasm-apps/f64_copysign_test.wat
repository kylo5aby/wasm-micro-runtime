;; WebAssembly Text Format (WAT) for f64.copysign opcode testing
;;
;; This module provides comprehensive test functions for the f64.copysign operation,
;; which implements IEEE 754 copySign functionality. The copysign operation takes
;; two f64 operands and returns the first operand with the sign bit of the second
;; operand, preserving the magnitude of the first operand.
;;
;; Test Functions:
;; - test_f64_copysign: Basic f64.copysign operation wrapper
;;
;; WebAssembly f64.copysign specification:
;; - Stack behavior: [..., f64_magnitude, f64_sign_source] -> [..., f64_result]
;; - IEEE 754 compliance: Follows IEEE 754-2008 copySign operation semantics
;; - Never traps: Always produces a result for any f64 input combination
;; - Sign manipulation: Copies sign bit from second operand to first operand

(module
  ;; Export the main test function for f64.copysign operation
  (export "test_f64_copysign" (func $test_f64_copysign))

  ;; Basic f64.copysign operation test function
  ;;
  ;; Parameters:
  ;;   $magnitude (f64): First operand - provides the magnitude
  ;;   $sign_source (f64): Second operand - provides the sign bit
  ;;
  ;; Returns:
  ;;   f64: Result of f64.copysign($magnitude, $sign_source)
  ;;
  ;; This function directly exposes the f64.copysign WebAssembly instruction
  ;; for testing all combinations of IEEE 754 double-precision values including:
  ;; - Normal finite values (positive/negative)
  ;; - Special values (NaN, infinity, zero)
  ;; - Boundary conditions (DBL_MAX, DBL_MIN, denormalized)
  ;; - Zero variants (+0.0, -0.0)
  (func $test_f64_copysign (param $magnitude f64) (param $sign_source f64) (result f64)
    ;; Load both parameters onto the stack
    local.get $magnitude     ;; Stack: [magnitude]
    local.get $sign_source   ;; Stack: [magnitude, sign_source]

    ;; Execute f64.copysign instruction
    ;; This copies the sign bit of $sign_source to $magnitude
    ;; while preserving the magnitude and exponent of $magnitude
    f64.copysign            ;; Stack: [result]

    ;; Return the result
    ;; The result has the magnitude of $magnitude with the sign of $sign_source
  )

  ;; Memory section (minimal - not needed for f64.copysign but included for completeness)
  (memory 1)
)