(module
  ;; f32.abs comprehensive test module
  ;; Tests all f32.abs functionality including basic operations,
  ;; boundary conditions, special values, and sign bit manipulation

  ;; Basic absolute value test function
  ;; Tests typical mathematical absolute value operations
  (func (export "abs_test") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Special values test function
  ;; Tests IEEE 754 special values: ±0, ±∞, NaN
  (func (export "abs_special") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Boundary values test function
  ;; Tests floating-point boundary conditions and extreme values
  (func (export "abs_boundary") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Mathematical property validation function
  ;; Tests mathematical properties like abs(abs(x)) = abs(x)
  (func (export "abs_math_property") (param $x f32) (result f32)
    local.get $x
    f32.abs        ;; abs(x)
    f32.abs        ;; abs(abs(x)) - should equal abs(x)
  )

  ;; Identity test function for positive values
  ;; Tests that abs(positive) = positive (identity property)
  (func (export "abs_identity_positive") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Symmetry test function
  ;; Tests that abs(x) = abs(-x) for all x
  (func (export "abs_symmetry") (param $x f32) (result f32)
    local.get $x
    f32.neg        ;; -x
    f32.abs        ;; abs(-x)
  )

  ;; Sign bit manipulation test function
  ;; Tests that only sign bit is affected by abs operation
  (func (export "abs_sign_bit") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Zero handling test function
  ;; Tests abs behavior with positive and negative zero
  (func (export "abs_zero") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Composed operation test function
  ;; Tests f32.abs in combination with other operations
  (func (export "abs_composed") (param $a f32) (param $b f32) (result f32)
    local.get $a
    local.get $b
    f32.sub        ;; a - b
    f32.abs        ;; abs(a - b)
  )

  ;; Non-negativity test function
  ;; Tests that abs(x) >= 0 for all finite x
  (func (export "abs_non_negative") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Magnitude preservation test function
  ;; Tests that abs preserves magnitude (distance from zero)
  (func (export "abs_magnitude") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Subnormal handling test function
  ;; Tests abs behavior with denormalized floating-point values
  (func (export "abs_subnormal") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Infinity handling test function
  ;; Tests abs behavior with positive and negative infinity
  (func (export "abs_infinity") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; NaN preservation test function
  ;; Tests that abs preserves NaN while clearing sign bit
  (func (export "abs_nan") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )

  ;; Range validation function
  ;; Tests abs across different input ranges and magnitudes
  (func (export "abs_range") (param $x f32) (result f32)
    local.get $x
    f32.abs
  )
)