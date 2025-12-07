(module
  ;; Test function for i32.trunc_sat_f64_s opcode
  ;; This function takes an f64 parameter and returns the result of
  ;; saturating truncation to a signed i32 value
  (func $test_i32_trunc_sat_f64_s (export "test_i32_trunc_sat_f64_s")
    (param $input f64) (result i32)
    ;; Get the f64 parameter from stack
    local.get $input
    ;; Apply i32.trunc_sat_f64_s saturating conversion
    i32.trunc_sat_f64_s
  )

  ;; Additional test functions for comprehensive coverage

  ;; Test function with hardcoded special values for validation
  (func $test_special_values (export "test_special_values") (result i32)
    ;; Test NaN conversion (should return 0)
    f64.const nan
    i32.trunc_sat_f64_s
    drop

    ;; Test positive infinity (should return INT32_MAX)
    f64.const inf
    i32.trunc_sat_f64_s
    drop

    ;; Test negative infinity (should return INT32_MIN)
    f64.const -inf
    i32.trunc_sat_f64_s
    drop

    ;; Return a simple test result
    f64.const 42.7
    i32.trunc_sat_f64_s
  )

  ;; Test function for boundary value validation
  (func $test_boundary_values (export "test_boundary_values") (result i32)
    ;; Test value just over INT32_MAX (should saturate to INT32_MAX)
    f64.const 2147483648.0  ;; INT32_MAX + 1
    i32.trunc_sat_f64_s
    drop

    ;; Test value just under INT32_MIN (should saturate to INT32_MIN)
    f64.const -2147483649.0  ;; INT32_MIN - 1
    i32.trunc_sat_f64_s
    drop

    ;; Return exact INT32_MAX as result
    f64.const 2147483647.0
    i32.trunc_sat_f64_s
  )

  ;; Test function for fractional truncation behavior
  (func $test_fractional_truncation (export "test_fractional_truncation") (result i32)
    ;; Test positive fractional truncation (toward zero)
    f64.const 123.99999
    i32.trunc_sat_f64_s
    drop

    ;; Test negative fractional truncation (toward zero)
    f64.const -456.99999
    i32.trunc_sat_f64_s
    drop

    ;; Test very small positive fraction (should become 0)
    f64.const 0.9999999
    i32.trunc_sat_f64_s
    drop

    ;; Return result of negative fraction test
    f64.const -0.9999999
    i32.trunc_sat_f64_s
  )

  ;; Test function for zero handling
  (func $test_zero_values (export "test_zero_values") (result i32)
    ;; Test positive zero
    f64.const 0.0
    i32.trunc_sat_f64_s
    drop

    ;; Test negative zero
    f64.const -0.0
    i32.trunc_sat_f64_s
    drop

    ;; Return successful conversion result
    i32.const 0
  )

  ;; Test function for large overflow values
  (func $test_large_values (export "test_large_values") (result i32)
    ;; Test very large positive value (should saturate to INT32_MAX)
    f64.const 1e20
    i32.trunc_sat_f64_s
    drop

    ;; Test very large negative value (should saturate to INT32_MIN)
    f64.const -1e20
    i32.trunc_sat_f64_s

    ;; Return the large negative saturation result
  )
)