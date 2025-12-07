(module
  ;; f32.sqrt comprehensive test module
  ;; Tests all f32.sqrt functionality including basic operations,
  ;; boundary conditions, special values, and error scenarios

  ;; Basic square root test function
  ;; Tests typical mathematical square root operations
  (func (export "sqrt_test") (param $x f32) (result f32)
    local.get $x
    f32.sqrt
  )

  ;; Special values test function
  ;; Tests IEEE 754 special values: ±0, ±∞, NaN, negative numbers
  (func (export "sqrt_special") (param $x f32) (result f32)
    local.get $x
    f32.sqrt
  )

  ;; Boundary values test function
  ;; Tests floating-point boundary conditions and extreme values
  (func (export "sqrt_boundary") (param $x f32) (result f32)
    local.get $x
    f32.sqrt
  )

  ;; Mathematical property validation function
  ;; Tests mathematical properties like sqrt(x*x) ≈ x for x >= 0
  (func (export "sqrt_math_property") (param $x f32) (result f32)
    local.get $x
    local.get $x
    f32.mul        ;; x * x
    f32.sqrt       ;; sqrt(x * x) should ≈ |x|
  )

  ;; Composed operation test function
  ;; Tests f32.sqrt in combination with other operations
  (func (export "sqrt_composed") (param $a f32) (param $b f32) (result f32)
    local.get $a
    local.get $b
    f32.add        ;; a + b
    f32.sqrt       ;; sqrt(a + b)
  )

  ;; Identity test function
  ;; Tests sqrt(1.0) = 1.0 identity property
  (func (export "sqrt_identity") (result f32)
    f32.const 1.0
    f32.sqrt
  )

  ;; Zero test function
  ;; Tests sqrt of positive and negative zero
  (func (export "sqrt_zero") (param $x f32) (result f32)
    local.get $x
    f32.sqrt
  )

  ;; Perfect square validation function
  ;; Tests that perfect squares return exact integer results
  (func (export "sqrt_perfect") (param $x f32) (result f32)
    local.get $x
    f32.sqrt
  )

  ;; Range validation function
  ;; Tests sqrt across different input ranges
  (func (export "sqrt_range") (param $x f32) (result f32)
    local.get $x
    f32.sqrt
  )

  ;; Precision test function
  ;; Tests sqrt precision with values near floating-point limits
  (func (export "sqrt_precision") (param $x f32) (result f32)
    local.get $x
    f32.sqrt
  )
)