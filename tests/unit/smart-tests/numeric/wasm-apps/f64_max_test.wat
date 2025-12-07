(module
  ;; f64.max Test Module
  ;;
  ;; This WebAssembly module provides test functions for validating the f64.max opcode.
  ;; The f64.max instruction computes the maximum of two double-precision floating-point
  ;; numbers according to IEEE 754 standard, including proper handling of special values
  ;; such as NaN, infinity, and signed zero.
  ;;
  ;; Functions exported:
  ;;   - test_f64_max: Basic f64.max operation test function
  ;;   - test_f64_max_boundary: Boundary value testing
  ;;   - test_f64_max_special: Special IEEE 754 value testing
  ;;   - test_f64_max_identity: Identity operation validation
  ;;   - test_f64_max_subnormal: Subnormal number testing

  ;; Basic f64.max test function
  ;; Takes two f64 parameters and returns their maximum value
  ;; Stack effect: [f64 f64] -> [f64]
  (func $test_f64_max (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Boundary value f64.max test function
  ;; Tests maximum operation with extreme double-precision floating-point values
  ;; Includes DBL_MAX, DBL_MIN, subnormal numbers, and infinity
  (func $test_f64_max_boundary (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Special value f64.max test function
  ;; Tests IEEE 754 special values including NaN, signed zero, and infinity
  ;; Validates proper NaN propagation and signed zero handling
  (func $test_f64_max_special (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Identity operation test function
  ;; Validates that f64.max(x, x) == x for all finite values x
  (func $test_f64_max_identity (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Subnormal number test function
  ;; Tests maximum operation with subnormal double-precision numbers
  ;; Validates proper handling of denormalized values near zero
  (func $test_f64_max_subnormal (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Multiple operation test function
  ;; Tests chained f64.max operations: max(max(a, b), c)
  (func $test_f64_max_chained (param $a f64) (param $b f64) (param $c f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
    (local.get $c)
    (f64.max)
  )

  ;; Constant folding test function
  ;; Tests f64.max with constant values for optimization validation
  (func $test_f64_max_constants (result f64)
    (f64.const 3.141592653589793)
    (f64.const 2.718281828459045)
    (f64.max)
  )

  ;; Zero comparison test function
  ;; Specifically tests signed zero behavior: max(+0.0, -0.0) = +0.0
  (func $test_f64_max_zero (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; NaN propagation test function
  ;; Tests NaN handling: max(NaN, x) = NaN, max(x, NaN) = NaN
  (func $test_f64_max_nan (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Infinity test function
  ;; Tests infinity handling: max(inf, x), max(-inf, x), max(inf, -inf)
  (func $test_f64_max_infinity (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Precision boundary test function
  ;; Tests maximum operation at double precision boundaries
  (func $test_f64_max_precision (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Large magnitude test function
  ;; Tests f64.max with very large magnitude values
  (func $test_f64_max_large (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.max)
  )

  ;; Export all test functions for C++ test access
  (export "test_f64_max" (func $test_f64_max))
  (export "test_f64_max_boundary" (func $test_f64_max_boundary))
  (export "test_f64_max_special" (func $test_f64_max_special))
  (export "test_f64_max_identity" (func $test_f64_max_identity))
  (export "test_f64_max_subnormal" (func $test_f64_max_subnormal))
  (export "test_f64_max_chained" (func $test_f64_max_chained))
  (export "test_f64_max_constants" (func $test_f64_max_constants))
  (export "test_f64_max_zero" (func $test_f64_max_zero))
  (export "test_f64_max_nan" (func $test_f64_max_nan))
  (export "test_f64_max_infinity" (func $test_f64_max_infinity))
  (export "test_f64_max_precision" (func $test_f64_max_precision))
  (export "test_f64_max_large" (func $test_f64_max_large))
)