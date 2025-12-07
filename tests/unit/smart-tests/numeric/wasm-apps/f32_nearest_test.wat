(module
  ;; WebAssembly Text Format (WAT) module for f32.nearest opcode testing
  ;; This module provides comprehensive test functions for validating the IEEE 754
  ;; banker's rounding (round half to even) operation implementation in WAMR runtime environments.
  ;;
  ;; The f32.nearest instruction takes one f32 operand from the stack and returns
  ;; the rounded value to the nearest integer using banker's rounding rules:
  ;; - Round half-values (x.5) to the nearest even integer
  ;; - Preserve special IEEE 754 values (NaN, infinity, signed zeros)

  ;; Basic f32.nearest test function
  ;; Takes one f32 parameter and returns nearest(input) using banker's rounding
  (func (export "test_f32_nearest") (param $input f32) (result f32)
    local.get $input     ;; Load input operand onto stack
    f32.nearest         ;; Execute nearest: result = nearest(input)
  )

  ;; Stack underflow test function
  ;; Simulates a condition that would cause stack underflow if operands were consumed elsewhere
  ;; Returns a default value since direct stack underflow cannot be represented in valid WASM
  (func (export "test_f32_nearest_underflow") (result f32)
    ;; This function represents a scenario where the stack might be insufficient
    ;; In practice, actual underflow testing is handled at the runtime level
    f32.const 0.0       ;; Return default value for underflow simulation
  )

  ;; Banker's rounding validation functions
  ;; Test specific half-value cases to validate round-half-to-even behavior

  ;; Test 0.5 -> 0 (round to even)
  (func (export "test_half_to_zero") (result f32)
    f32.const 0.5       ;; Half-value that should round to 0 (even)
    f32.nearest
  )

  ;; Test 1.5 -> 2 (round to even)
  (func (export "test_half_to_two") (result f32)
    f32.const 1.5       ;; Half-value that should round to 2 (even)
    f32.nearest
  )

  ;; Test 2.5 -> 2 (round to even)
  (func (export "test_half_stays_two") (result f32)
    f32.const 2.5       ;; Half-value that should round to 2 (even)
    f32.nearest
  )

  ;; Test 3.5 -> 4 (round to even)
  (func (export "test_half_to_four") (result f32)
    f32.const 3.5       ;; Half-value that should round to 4 (even)
    f32.nearest
  )

  ;; Negative banker's rounding tests
  ;; Test -1.5 -> -2 (round to even)
  (func (export "test_negative_half_to_negative_two") (result f32)
    f32.const -1.5      ;; Negative half-value that should round to -2 (even)
    f32.nearest
  )

  ;; Test -2.5 -> -2 (round to even)
  (func (export "test_negative_half_stays_negative_two") (result f32)
    f32.const -2.5      ;; Negative half-value that should round to -2 (even)
    f32.nearest
  )

  ;; Special value test functions
  ;; IEEE 754 special values should be preserved

  ;; NaN preservation test
  (func (export "test_nan_preservation") (result f32)
    f32.const nan       ;; Not-a-Number input
    f32.nearest         ;; Should preserve NaN
  )

  ;; Positive infinity preservation test
  (func (export "test_positive_infinity_preservation") (result f32)
    f32.const inf       ;; Positive infinity input
    f32.nearest         ;; Should preserve +∞
  )

  ;; Negative infinity preservation test
  (func (export "test_negative_infinity_preservation") (result f32)
    f32.const -inf      ;; Negative infinity input
    f32.nearest         ;; Should preserve -∞
  )

  ;; Signed zero preservation tests
  (func (export "test_positive_zero_preservation") (result f32)
    f32.const 0.0       ;; Positive zero input
    f32.nearest         ;; Should preserve +0.0
  )

  (func (export "test_negative_zero_preservation") (result f32)
    f32.const -0.0      ;; Negative zero input
    f32.nearest         ;; Should preserve -0.0
  )

  ;; Integer identity operation tests
  ;; Integer values should remain unchanged

  (func (export "test_positive_integer_identity") (result f32)
    f32.const 42.0      ;; Positive integer
    f32.nearest         ;; Should remain 42.0
  )

  (func (export "test_negative_integer_identity") (result f32)
    f32.const -100.0    ;; Negative integer
    f32.nearest         ;; Should remain -100.0
  )

  ;; Precision boundary tests
  ;; Test values at the limits of f32 precision

  ;; Test large value without fractional part
  (func (export "test_large_integer_no_fraction") (result f32)
    f32.const 8388608.0 ;; 2^23 - precision boundary for f32
    f32.nearest         ;; Should remain unchanged
  )

  ;; Basic fractional rounding tests
  (func (export "test_small_positive_fraction") (result f32)
    f32.const 2.3       ;; Small positive fraction
    f32.nearest         ;; Should round to 2.0
  )

  (func (export "test_small_negative_fraction") (result f32)
    f32.const -2.3      ;; Small negative fraction
    f32.nearest         ;; Should round to -2.0
  )

  ;; Near-precision boundary tests
  (func (export "test_just_below_half") (result f32)
    f32.const 0.49999999 ;; Just below 0.5
    f32.nearest          ;; Should round to 0.0
  )

  (func (export "test_just_above_half") (result f32)
    f32.const 0.50000001 ;; Just above 0.5
    f32.nearest          ;; Should round to 1.0
  )
)