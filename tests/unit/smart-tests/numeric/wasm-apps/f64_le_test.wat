(module
  ;; Test function for f64.le opcode
  ;; Takes two f64 parameters and returns i32 (1 if a <= b, 0 otherwise)
  (func (export "test_f64_le") (param $a f64) (param $b f64) (result i32)
    local.get $a
    local.get $b
    f64.le
  )

  ;; Additional test functions for comprehensive coverage

  ;; Test f64.le with constant values
  (func (export "test_constants") (result i32)
    f64.const 1.5
    f64.const 2.5
    f64.le
  )

  ;; Test f64.le with infinity values
  (func (export "test_infinity") (result i32)
    f64.const -inf
    f64.const inf
    f64.le
  )

  ;; Test f64.le with NaN values
  (func (export "test_nan") (result i32)
    f64.const nan
    f64.const 1.0
    f64.le
  )

  ;; Test f64.le with zero values
  (func (export "test_zero") (result i32)
    f64.const 0.0
    f64.const -0.0
    f64.le
  )

  ;; Test f64.le with equal values
  (func (export "test_equal") (result i32)
    f64.const 3.14159
    f64.const 3.14159
    f64.le
  )

  ;; Test f64.le with maximum f64 value
  (func (export "test_max") (result i32)
    f64.const 1.7976931348623157e+308  ;; DBL_MAX approximation
    f64.const 1.0
    f64.le
  )

  ;; Test f64.le with minimum positive f64 value
  (func (export "test_min") (result i32)
    f64.const 2.2250738585072014e-308  ;; DBL_MIN approximation
    f64.const 1.0
    f64.le
  )

  ;; Test f64.le with denormalized values
  (func (export "test_denorm") (result i32)
    f64.const 1.112536929253601e-308   ;; Denormalized value (DBL_MIN/2 approx)
    f64.const 2.2250738585072014e-308  ;; DBL_MIN approximation
    f64.le
  )
)