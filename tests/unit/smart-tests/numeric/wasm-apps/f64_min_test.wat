(module
  ;; f64.min Test Module
  ;;
  ;; This WebAssembly module provides test functions for validating the f64.min opcode.
  ;; The f64.min instruction computes the minimum of two double-precision floating-point
  ;; numbers according to IEEE 754 standard, including proper handling of special values
  ;; such as NaN, infinity, and signed zero.
  ;;
  ;; Functions exported:
  ;;   - test_f64_min: Basic f64.min operation test function
  ;;   - test_f64_min_boundary: Boundary value testing
  ;;   - test_f64_min_special: Special IEEE 754 value testing
  ;;   - test_f64_min_identity: Identity operation validation
  ;;   - test_f64_min_subnormal: Subnormal number testing

  ;; Basic f64.min test function
  ;; Takes two f64 parameters and returns their minimum value
  ;; Stack effect: [f64 f64] -> [f64]
  (func $test_f64_min (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Boundary value f64.min test function
  ;; Tests minimum operation with extreme double-precision floating-point values
  ;; Includes DBL_MAX, DBL_MIN, subnormal numbers, and infinity
  (func $test_f64_min_boundary (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Special value f64.min test function
  ;; Tests IEEE 754 special values including NaN, signed zero, and infinity
  ;; Validates proper NaN propagation and signed zero handling
  (func $test_f64_min_special (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Identity operation test function
  ;; Validates that f64.min(x, x) == x for all finite values x
  (func $test_f64_min_identity (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Subnormal number test function
  ;; Tests minimum operation with subnormal double-precision numbers
  ;; Validates proper handling of denormalized values near zero
  (func $test_f64_min_subnormal (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Multiple operation test function
  ;; Tests chained f64.min operations: min(min(a, b), c)
  (func $test_f64_min_chained (param $a f64) (param $b f64) (param $c f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
    (local.get $c)
    (f64.min)
  )

  ;; Constant folding test function
  ;; Tests f64.min with constant values for optimization validation
  (func $test_f64_min_constants (result f64)
    (f64.const 3.141592653589793)
    (f64.const 2.718281828459045)
    (f64.min)
  )

  ;; Zero comparison test function
  ;; Specifically tests signed zero behavior: min(+0.0, -0.0) = -0.0
  (func $test_f64_min_zero (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; NaN propagation test function
  ;; Tests NaN handling: min(NaN, x) = NaN, min(x, NaN) = NaN
  (func $test_f64_min_nan (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Infinity test function
  ;; Tests infinity handling: min(inf, x), min(-inf, x), min(inf, -inf)
  (func $test_f64_min_infinity (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Precision boundary test function
  ;; Tests minimum operation at double precision boundaries
  (func $test_f64_min_precision (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Large magnitude test function
  ;; Tests f64.min with very large magnitude values
  (func $test_f64_min_large (param $a f64) (param $b f64) (result f64)
    (local.get $a)
    (local.get $b)
    (f64.min)
  )

  ;; Export all test functions for C++ test access
  (export "test_f64_min" (func $test_f64_min))
  (export "test_f64_min_boundary" (func $test_f64_min_boundary))
  (export "test_f64_min_special" (func $test_f64_min_special))
  (export "test_f64_min_identity" (func $test_f64_min_identity))
  (export "test_f64_min_subnormal" (func $test_f64_min_subnormal))
  (export "test_f64_min_chained" (func $test_f64_min_chained))
  (export "test_f64_min_constants" (func $test_f64_min_constants))
  (export "test_f64_min_zero" (func $test_f64_min_zero))
  (export "test_f64_min_nan" (func $test_f64_min_nan))
  (export "test_f64_min_infinity" (func $test_f64_min_infinity))
  (export "test_f64_min_precision" (func $test_f64_min_precision))
  (export "test_f64_min_large" (func $test_f64_min_large))
)