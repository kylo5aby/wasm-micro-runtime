(module
  ;; f32.min Test Module
  ;;
  ;; This WebAssembly module provides test functions for validating the f32.min opcode.
  ;; The f32.min instruction computes the minimum of two single-precision floating-point
  ;; numbers according to IEEE 754 standard, including proper handling of special values
  ;; such as NaN, infinity, and signed zero.
  ;;
  ;; Functions exported:
  ;;   - test_f32_min: Basic f32.min operation test function
  ;;   - test_f32_min_boundary: Boundary value testing
  ;;   - test_f32_min_special: Special IEEE 754 value testing
  ;;   - test_f32_min_commutative: Commutative property validation

  ;; Basic f32.min test function
  ;; Takes two f32 parameters and returns their minimum value
  ;; Stack effect: [f32 f32] -> [f32]
  (func $test_f32_min (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
  )

  ;; Boundary value f32.min test function
  ;; Tests minimum operation with extreme floating-point values
  ;; Includes FLT_MAX, FLT_MIN, denormalized numbers, and infinity
  (func $test_f32_min_boundary (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
  )

  ;; Special value f32.min test function
  ;; Tests IEEE 754 special values including NaN, signed zero, and infinity
  ;; Validates proper NaN propagation and signed zero handling
  (func $test_f32_min_special (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
  )

  ;; Commutative property test function
  ;; Validates that f32.min(a, b) == f32.min(b, a) for IEEE 754 compliance
  (func $test_f32_min_commutative (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
  )

  ;; Multiple operation test function
  ;; Tests chained f32.min operations: min(min(a, b), c)
  (func $test_f32_min_chained (param $a f32) (param $b f32) (param $c f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
    (local.get $c)
    (f32.min)
  )

  ;; Constant folding test function
  ;; Tests f32.min with constant values for optimization validation
  (func $test_f32_min_constants (result f32)
    (f32.const 3.14159)
    (f32.const 2.71828)
    (f32.min)
  )

  ;; Zero comparison test function
  ;; Specifically tests signed zero behavior: min(+0.0, -0.0) = -0.0
  (func $test_f32_min_zero (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
  )

  ;; NaN propagation test function
  ;; Tests NaN handling: min(NaN, x) = NaN, min(x, NaN) = NaN
  (func $test_f32_min_nan (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
  )

  ;; Infinity test function
  ;; Tests infinity handling: min(inf, x), min(-inf, x), min(inf, -inf)
  (func $test_f32_min_infinity (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
  )

  ;; Denormalized number test function
  ;; Tests minimum operation with denormalized floating-point numbers
  (func $test_f32_min_denorm (param $a f32) (param $b f32) (result f32)
    (local.get $a)
    (local.get $b)
    (f32.min)
  )

  ;; Export all test functions for C++ test access
  (export "test_f32_min" (func $test_f32_min))
  (export "test_f32_min_boundary" (func $test_f32_min_boundary))
  (export "test_f32_min_special" (func $test_f32_min_special))
  (export "test_f32_min_commutative" (func $test_f32_min_commutative))
  (export "test_f32_min_chained" (func $test_f32_min_chained))
  (export "test_f32_min_constants" (func $test_f32_min_constants))
  (export "test_f32_min_zero" (func $test_f32_min_zero))
  (export "test_f32_min_nan" (func $test_f32_min_nan))
  (export "test_f32_min_infinity" (func $test_f32_min_infinity))
  (export "test_f32_min_denorm" (func $test_f32_min_denorm))
)