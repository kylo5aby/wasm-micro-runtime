(module
  ;; f32.max Test Module
  ;;
  ;; This WebAssembly module provides test functions for validating the f32.max opcode.
  ;; The f32.max instruction computes the maximum of two single-precision floating-point
  ;; numbers according to IEEE 754 standard, including proper handling of special values
  ;; such as NaN, infinity, and signed zero.
  ;;
  ;; Functions exported:
  ;;   - test_f32_max: Basic f32.max operation test function
  ;;   - test_f32_max_boundary: Boundary value testing
  ;;   - test_f32_max_special: Special IEEE 754 value testing
  ;;   - test_f32_max_commutative: Commutative property validation
  ;;   - test_stack_underflow: Stack underflow boundary testing

  ;; Basic f32.max test function
  ;; Takes two f32 parameters and returns their maximum value
  ;; Stack effect: [f32 f32] -> [f32]
  (func $test_f32_max (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; Boundary value f32.max test function
  ;; Tests maximum operation with extreme floating-point values
  ;; Includes FLT_MAX, FLT_MIN, denormalized numbers, and infinity
  (func $test_f32_max_boundary (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; Special value f32.max test function
  ;; Tests IEEE 754 special values including NaN, signed zero, and infinity
  ;; Validates proper NaN propagation and signed zero handling
  (func $test_f32_max_special (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; Commutative property test function
  ;; Validates that f32.max(a, b) == f32.max(b, a) for IEEE 754 compliance
  (func $test_f32_max_commutative (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; Multiple operation test function
  ;; Tests chained f32.max operations: max(max(a, b), c)
  (func $test_f32_max_chained (param $a f32) (param $b f32) (param $c f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
    (local.get $c)
    (f32.max)
  )

  ;; Constant folding test function
  ;; Tests f32.max with constant values for optimization validation
  (func $test_f32_max_constants (result f32)
    (f32.const 3.14159)
    (f32.const 2.71828)
    (f32.max)
  )

  ;; Zero comparison test function
  ;; Specifically tests signed zero behavior: max(+0.0, -0.0) = +0.0
  (func $test_f32_max_zero (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; NaN propagation test function
  ;; Tests NaN handling: max(NaN, x) = NaN, max(x, NaN) = NaN
  (func $test_f32_max_nan (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; Infinity test function
  ;; Tests infinity handling: max(inf, x), max(-inf, x), max(inf, -inf)
  (func $test_f32_max_infinity (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; Denormalized number test function
  ;; Tests maximum operation with denormalized floating-point numbers
  (func $test_f32_max_denorm (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; Stack underflow boundary test function
  ;; Tests that f32.max works correctly when stack conditions are at minimum requirements
  ;; This function demonstrates proper stack management and validates the boundary
  ;; between sufficient and insufficient operands for f32.max operation
  (func $test_stack_underflow (result i32)
    ;; Test proper stack usage with exactly 2 operands (minimum required)
    f32.const 5.0   ;; Push first operand
    f32.const 3.0   ;; Push second operand (now stack has exactly what f32.max needs)
    f32.max         ;; Valid: exactly 2 operands available
    drop            ;; Remove the result to clean stack

    ;; Return success indicator - if we reach here, stack management worked
    i32.const 1     ;; Return 1 to indicate successful execution
  )

  ;; Identity test function
  ;; Tests self-comparison: max(x, x) = x
  (func $test_f32_max_identity (param $value f32) (result f32)
    (local.get $value)
    (local.get $value)
    (f32.max)
  )

  ;; Mathematical property test function
  ;; Tests mathematical properties of maximum operation
  (func $test_f32_max_properties (param $a f32) (param $b f32) (param $c f32) (result f32)
    ;; Test associativity: max(max(a, b), c) should equal max(a, max(b, c))
    ;; We'll return max(max(a, b), c) for validation
    (local.get $a)
    (local.get $b)
    (f32.max)
    (local.get $c)
    (f32.max)
  )

  ;; Precision boundary test function
  ;; Tests f32.max at floating-point precision boundaries
  (func $test_f32_max_precision (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.max)
  )

  ;; Export all test functions for C++ test access
  (export "test_f32_max" (func $test_f32_max))
  (export "test_f32_max_boundary" (func $test_f32_max_boundary))
  (export "test_f32_max_special" (func $test_f32_max_special))
  (export "test_f32_max_commutative" (func $test_f32_max_commutative))
  (export "test_f32_max_chained" (func $test_f32_max_chained))
  (export "test_f32_max_constants" (func $test_f32_max_constants))
  (export "test_f32_max_zero" (func $test_f32_max_zero))
  (export "test_f32_max_nan" (func $test_f32_max_nan))
  (export "test_f32_max_infinity" (func $test_f32_max_infinity))
  (export "test_f32_max_denorm" (func $test_f32_max_denorm))
  (export "test_stack_underflow" (func $test_stack_underflow))
  (export "test_f32_max_identity" (func $test_f32_max_identity))
  (export "test_f32_max_properties" (func $test_f32_max_properties))
  (export "test_f32_max_precision" (func $test_f32_max_precision))
)