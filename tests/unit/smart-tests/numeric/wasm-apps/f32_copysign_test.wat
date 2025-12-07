(module
  ;; WebAssembly Text Format (WAT) module for f32.copysign opcode testing
  ;; This module provides comprehensive test functions for validating the IEEE 754
  ;; copysign operation implementation in WAMR runtime environments.
  ;;
  ;; The f32.copysign instruction takes two f32 operands from the stack:
  ;; - First operand (magnitude source): provides the absolute value component
  ;; - Second operand (sign source): provides the sign bit for the result
  ;; Returns an f32 with the magnitude of the first and sign of the second operand

  ;; Basic f32.copysign test function
  ;; Takes two f32 parameters and returns copysign(magnitude, sign)
  (func (export "test_f32_copysign") (param $magnitude f32) (param $sign f32) (result f32)
    local.get $magnitude  ;; Load magnitude operand onto stack
    local.get $sign       ;; Load sign operand onto stack
    f32.copysign         ;; Execute copysign: result = copysign(magnitude, sign)
  )

  ;; Stack underflow test function
  ;; Attempts to execute f32.copysign with insufficient operands to test error handling
  (func (export "test_stack_underflow") (result f32)
    ;; This function intentionally provides no operands for f32.copysign
    ;; In a valid WASM module, this would be caught during validation
    ;; This serves as a placeholder for runtime error testing
    f32.const 0.0  ;; Provide a default return value
  )

  ;; Identity operation test: copysign(x, 1.0) should return |x|
  (func (export "test_positive_identity") (param $value f32) (result f32)
    local.get $value
    f32.const 1.0        ;; Positive sign source
    f32.copysign
  )

  ;; Negative identity test: copysign(x, -1.0) should return -|x|
  (func (export "test_negative_identity") (param $value f32) (result f32)
    local.get $value
    f32.const -1.0       ;; Negative sign source
    f32.copysign
  )

  ;; Zero sign manipulation test functions
  (func (export "test_zero_positive_sign") (result f32)
    f32.const 0.0        ;; +0.0 magnitude
    f32.const 1.0        ;; Positive sign
    f32.copysign         ;; Should return +0.0
  )

  (func (export "test_zero_negative_sign") (result f32)
    f32.const 0.0        ;; +0.0 magnitude
    f32.const -1.0       ;; Negative sign
    f32.copysign         ;; Should return -0.0
  )

  ;; Infinity sign manipulation test functions
  (func (export "test_infinity_positive_sign") (result f32)
    f32.const inf        ;; +∞ magnitude
    f32.const 1.0        ;; Positive sign
    f32.copysign         ;; Should return +∞
  )

  (func (export "test_infinity_negative_sign") (result f32)
    f32.const inf        ;; +∞ magnitude
    f32.const -1.0       ;; Negative sign
    f32.copysign         ;; Should return -∞
  )

  ;; NaN sign manipulation test function
  (func (export "test_nan_negative_sign") (result f32)
    f32.const nan        ;; NaN magnitude
    f32.const -1.0       ;; Negative sign
    f32.copysign         ;; Should return NaN with negative sign bit
  )

  ;; Boundary value test functions
  (func (export "test_max_finite_negative_sign") (result f32)
    f32.const 0x1.fffffep+127  ;; F32_MAX (3.4028235e+38)
    f32.const -1.0             ;; Negative sign
    f32.copysign               ;; Should return -F32_MAX
  )

  (func (export "test_min_positive_negative_sign") (result f32)
    f32.const 0x1p-126         ;; F32_MIN_POSITIVE (1.175494e-38)
    f32.const -1.0             ;; Negative sign
    f32.copysign               ;; Should return -F32_MIN_POSITIVE
  )

  ;; Cross-operand sign tests with special values
  (func (export "test_normal_from_zero_sign") (param $magnitude f32) (result f32)
    local.get $magnitude
    f32.const -0.0       ;; Sign from negative zero
    f32.copysign         ;; Should copy sign bit from -0.0
  )

  (func (export "test_normal_from_infinity_sign") (param $magnitude f32) (result f32)
    local.get $magnitude
    f32.const -inf       ;; Sign from negative infinity
    f32.copysign         ;; Should copy sign bit from -∞
  )

  ;; Denormal (subnormal) value test function
  (func (export "test_denormal_negative_sign") (result f32)
    f32.const 0x0.000002p-126  ;; Small denormal number
    f32.const -1.0             ;; Negative sign
    f32.copysign               ;; Should return negative denormal
  )
)