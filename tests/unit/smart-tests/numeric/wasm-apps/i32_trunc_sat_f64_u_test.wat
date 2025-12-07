;; WebAssembly Text Format module for i32.trunc_sat_f64_u opcode testing
;; This module provides comprehensive test functions for saturating truncation
;; from f64 to unsigned i32, covering normal conversions, saturation behavior,
;; special float values, and high-precision scenarios.

(module
  ;; Export the main test function for i32.trunc_sat_f64_u operation
  (func $trunc_sat_f64_u (export "trunc_sat_f64_u") (param $input f64) (result i32)
    ;; Direct application of i32.trunc_sat_f64_u opcode
    ;; Input: f64 value to be converted
    ;; Output: i32 (treated as unsigned) with saturation behavior:
    ;;   - NaN -> 0
    ;;   - values < 0 -> 0
    ;;   - values >= 2^32 -> 0xFFFFFFFF (4294967295)
    ;;   - normal values -> truncated to unsigned integer
    local.get $input
    i32.trunc_sat_f64_u
  )

  ;; Test function for basic conversion scenarios
  (func $test_basic_conversion (export "test_basic_conversion") (result i32)
    ;; Test normal positive value: 100.5 -> 100
    f64.const 100.5
    i32.trunc_sat_f64_u

    ;; Verify result is 100
    i32.const 100
    i32.eq
  )

  ;; Test function for saturation behavior on overflow
  (func $test_overflow_saturation (export "test_overflow_saturation") (result i32)
    ;; Test positive overflow: 4294967296.0 -> 4294967295 (UINT32_MAX)
    f64.const 4294967296.0
    i32.trunc_sat_f64_u

    ;; Verify result is UINT32_MAX (0xFFFFFFFF)
    i32.const 0xFFFFFFFF
    i32.eq
  )

  ;; Test function for saturation behavior on underflow
  (func $test_underflow_saturation (export "test_underflow_saturation") (result i32)
    ;; Test negative value: -1.0 -> 0
    f64.const -1.0
    i32.trunc_sat_f64_u

    ;; Verify result is 0
    i32.const 0
    i32.eq
  )

  ;; Test function for NaN handling
  (func $test_nan_handling (export "test_nan_handling") (result i32)
    ;; Test NaN input: NaN -> 0
    f64.const nan
    i32.trunc_sat_f64_u

    ;; Verify result is 0
    i32.const 0
    i32.eq
  )

  ;; Test function for positive infinity handling
  (func $test_positive_infinity (export "test_positive_infinity") (result i32)
    ;; Test +Infinity: +Inf -> UINT32_MAX
    f64.const inf
    i32.trunc_sat_f64_u

    ;; Verify result is UINT32_MAX
    i32.const 0xFFFFFFFF
    i32.eq
  )

  ;; Test function for negative infinity handling
  (func $test_negative_infinity (export "test_negative_infinity") (result i32)
    ;; Test -Infinity: -Inf -> 0
    f64.const -inf
    i32.trunc_sat_f64_u

    ;; Verify result is 0
    i32.const 0
    i32.eq
  )

  ;; Test function for zero handling
  (func $test_zero_values (export "test_zero_values") (result i32)
    ;; Test positive zero: +0.0 -> 0
    f64.const 0.0
    i32.trunc_sat_f64_u
    i32.const 0
    i32.eq

    ;; Test negative zero: -0.0 -> 0
    f64.const -0.0
    i32.trunc_sat_f64_u
    i32.const 0
    i32.eq

    ;; Both tests must pass
    i32.and
  )

  ;; Test function for boundary values
  (func $test_boundary_values (export "test_boundary_values") (result i32)
    ;; Test maximum valid value: 4294967295.0 -> 4294967295
    f64.const 4294967295.0
    i32.trunc_sat_f64_u
    i32.const 4294967295
    i32.eq

    ;; Test value just above maximum: 4294967295.1 -> 4294967295
    f64.const 4294967295.1
    i32.trunc_sat_f64_u
    i32.const 4294967295
    i32.eq

    ;; Both tests must pass
    i32.and
  )

  ;; Test function for decimal truncation behavior
  (func $test_decimal_truncation (export "test_decimal_truncation") (result i32)
    ;; Test truncation (not rounding): 42.99999 -> 42
    f64.const 42.99999999999999
    i32.trunc_sat_f64_u
    i32.const 42
    i32.eq

    ;; Test small decimal: 0.9 -> 0
    f64.const 0.9
    i32.trunc_sat_f64_u
    i32.const 0
    i32.eq

    ;; Both tests must pass
    i32.and
  )

  ;; Test function for large number handling
  (func $test_large_numbers (export "test_large_numbers") (result i32)
    ;; Test very large positive: 1e20 -> UINT32_MAX
    f64.const 1e20
    i32.trunc_sat_f64_u
    i32.const 0xFFFFFFFF
    i32.eq

    ;; Test large negative: -1e10 -> 0
    f64.const -1e10
    i32.trunc_sat_f64_u
    i32.const 0
    i32.eq

    ;; Both tests must pass
    i32.and
  )
)