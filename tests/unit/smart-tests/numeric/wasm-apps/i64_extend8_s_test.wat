(module
  ;; i64.extend8_s Test Module
  ;;
  ;; This module provides comprehensive test functions for the i64.extend8_s WebAssembly opcode.
  ;; The i64.extend8_s instruction performs sign extension of the lower 8 bits of an i64 value
  ;; to a full 64-bit signed integer, treating the lower 8 bits as a signed 8-bit value.
  ;;
  ;; Functions exported:
  ;; - test_i64_extend8_s: Basic sign extension operation
  ;; - test_i64_extend8_s_double: Double application for idempotent testing
  ;;
  ;; Sign extension logic:
  ;; - If bit 7 (sign bit) is 0: Upper 56 bits filled with 0s (positive values 0-127)
  ;; - If bit 7 (sign bit) is 1: Upper 56 bits filled with 1s (negative values 128-255 as unsigned, -128 to -1 as signed)

  ;; Basic i64.extend8_s operation test function
  ;; Takes an i64 input, applies i64.extend8_s, and returns the result
  ;; Tests the fundamental sign extension behavior across all 8-bit value ranges
  (func (export "test_i64_extend8_s") (param $input i64) (result i64)
    ;; Load input parameter onto stack
    local.get $input

    ;; Apply i64.extend8_s operation
    ;; This instruction:
    ;; 1. Takes the lower 8 bits of the input i64 value
    ;; 2. Treats those 8 bits as a signed 8-bit integer (-128 to +127)
    ;; 3. Sign-extends to full 64-bit signed integer
    ;; 4. Upper 56 bits of input are completely ignored
    i64.extend8_s

    ;; Return the sign-extended result
  )

  ;; Double application test function for idempotent property validation
  ;; Applies i64.extend8_s twice to verify: extend8_s(extend8_s(x)) == extend8_s(x)
  ;; This validates that the operation is idempotent and stable
  (func (export "test_i64_extend8_s_double") (param $input i64) (result i64)
    ;; Load input parameter
    local.get $input

    ;; First application of i64.extend8_s
    i64.extend8_s

    ;; Second application of i64.extend8_s on the already extended result
    ;; This should produce the same result as a single application
    i64.extend8_s

    ;; Return the doubly-extended result
  )

  ;; Test functions for specific boundary conditions and edge cases

  ;; Test positive 8-bit boundary (127) - maximum positive signed 8-bit value
  (func (export "test_boundary_positive_max") (result i64)
    ;; Load 127 (0x7F) - maximum positive signed 8-bit value
    i64.const 127
    i64.extend8_s
  )

  ;; Test negative 8-bit boundary (128) - minimum negative signed 8-bit value (-128)
  (func (export "test_boundary_negative_min") (result i64)
    ;; Load 128 (0x80) - interpreted as -128 in signed 8-bit
    i64.const 128
    i64.extend8_s
  )

  ;; Test maximum unsigned 8-bit value (255) - interpreted as -1 in signed 8-bit
  (func (export "test_boundary_unsigned_max") (result i64)
    ;; Load 255 (0xFF) - interpreted as -1 in signed 8-bit
    i64.const 255
    i64.extend8_s
  )

  ;; Test zero value - should remain unchanged
  (func (export "test_zero_value") (result i64)
    ;; Load 0 - should extend to 0
    i64.const 0
    i64.extend8_s
  )

  ;; Test upper bit masking - large value with specific lower 8 bits
  (func (export "test_upper_bit_masking_positive") (result i64)
    ;; Load large value with lower 8 bits = 0x7F (127, positive)
    ;; Upper bits should be completely ignored
    i64.const 0x123456789ABCDE7F
    i64.extend8_s
  )

  ;; Test upper bit masking - large value with negative lower 8 bits
  (func (export "test_upper_bit_masking_negative") (result i64)
    ;; Load large value with lower 8 bits = 0xBE (190, interpreted as -66)
    ;; Upper bits should be completely ignored
    i64.const 0xDEADBEEFCAFEBABE
    i64.extend8_s
  )

  ;; Test all-ones pattern
  (func (export "test_all_ones_pattern") (result i64)
    ;; Load all-ones value (lower 8 bits = 0xFF = -1)
    i64.const 0xFFFFFFFFFFFFFFFF
    i64.extend8_s
  )

  ;; Test alternating bit pattern
  (func (export "test_alternating_pattern") (result i64)
    ;; Load alternating bit pattern (lower 8 bits = 0xAA = -86)
    i64.const 0xAAAAAAAAAAAAAAAA
    i64.extend8_s
  )

  ;; Simple test function that returns a specific test result based on selector
  (func (export "test_multiple_values") (param $selector i32) (result i64)
    ;; For simplicity, just return the selector value extended
    local.get $selector
    i64.extend_i32_s
    i64.extend8_s
  )
)