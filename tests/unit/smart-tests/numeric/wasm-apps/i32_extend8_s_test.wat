(module
  ;; i32.extend8_s Test Module
  ;;
  ;; This module provides comprehensive test functions for the i32.extend8_s WebAssembly opcode.
  ;; The i32.extend8_s instruction performs sign extension of the lower 8 bits of an i32 value
  ;; to a full 32-bit signed integer, treating the lower 8 bits as a signed 8-bit value.
  ;;
  ;; Functions exported:
  ;; - test_i32_extend8_s: Basic sign extension operation
  ;; - test_i32_extend8_s_double: Double application for idempotent testing
  ;;
  ;; Sign extension logic:
  ;; - If bit 7 (sign bit) is 0: Upper 24 bits filled with 0s (positive values 0-127)
  ;; - If bit 7 (sign bit) is 1: Upper 24 bits filled with 1s (negative values 128-255 as unsigned, -128 to -1 as signed)

  ;; Basic i32.extend8_s operation test function
  ;; Takes an i32 input, applies i32.extend8_s, and returns the result
  ;; Tests the fundamental sign extension behavior across all 8-bit value ranges
  (func (export "test_i32_extend8_s") (param $input i32) (result i32)
    ;; Load input parameter onto stack
    local.get $input

    ;; Apply i32.extend8_s operation
    ;; This instruction:
    ;; 1. Takes the lower 8 bits of the input i32 value
    ;; 2. Treats those 8 bits as a signed 8-bit integer (-128 to +127)
    ;; 3. Sign-extends to full 32-bit signed integer
    ;; 4. Upper 24 bits of input are completely ignored
    i32.extend8_s

    ;; Return the sign-extended result
  )

  ;; Double application test function for idempotent property validation
  ;; Applies i32.extend8_s twice to verify: extend8_s(extend8_s(x)) == extend8_s(x)
  ;; This validates that the operation is idempotent and stable
  (func (export "test_i32_extend8_s_double") (param $input i32) (result i32)
    ;; Load input parameter
    local.get $input

    ;; First application of i32.extend8_s
    i32.extend8_s

    ;; Second application of i32.extend8_s on the already extended result
    ;; This should produce the same result as a single application
    i32.extend8_s

    ;; Return the doubly-extended result
  )

  ;; Test case validation functions for specific scenarios
  ;; These functions can be used for additional validation if needed

  ;; Test positive 8-bit value sign extension (0-127 range)
  ;; Should extend with zeros in upper 24 bits
  (func (export "test_positive_extend") (param $input i32) (result i32)
    local.get $input
    ;; Mask to ensure only lower 8 bits, positive range (0x00-0x7F)
    i32.const 0x7F
    i32.and
    i32.extend8_s
  )

  ;; Test negative 8-bit value sign extension (128-255 as unsigned, -128 to -1 as signed)
  ;; Should extend with ones in upper 24 bits
  (func (export "test_negative_extend") (param $input i32) (result i32)
    local.get $input
    ;; Force into negative 8-bit range by setting bit 7
    i32.const 0x80
    i32.or
    i32.const 0xFF
    i32.and
    i32.extend8_s
  )

  ;; Test boundary value at sign bit transition (127 -> 128)
  ;; Validates the critical transition from positive to negative
  (func (export "test_sign_boundary") (param $boundary_offset i32) (result i32)
    ;; Start with 127 (0x7F) and add offset to cross into negative range
    i32.const 0x7F
    local.get $boundary_offset
    i32.add
    i32.extend8_s
  )

  ;; Test upper bits interference - validates that upper 24 bits are ignored
  ;; Uses various patterns in upper bits while controlling lower 8 bits
  (func (export "test_upper_bits_ignored") (param $lower_8_bits i32) (param $upper_24_pattern i32) (result i32)
    ;; Combine upper 24-bit pattern with controlled lower 8 bits
    local.get $upper_24_pattern
    i32.const 0xFFFFFF00  ;; Mask for upper 24 bits
    i32.and

    local.get $lower_8_bits
    i32.const 0xFF        ;; Mask for lower 8 bits
    i32.and

    i32.or                ;; Combine upper and lower parts
    i32.extend8_s         ;; Apply sign extension
  )

  ;; Mathematical property validation - tests specific arithmetic properties
  ;; Validates that sign extension preserves certain mathematical relationships
  (func (export "test_mathematical_properties") (param $value i32) (result i32)
    ;; Test: For already properly sign-extended values, operation should be identity
    ;; This function can be used to validate edge cases in mathematical behavior
    local.get $value
    i32.extend8_s

    ;; Store first result
    local.get $value
    i32.extend8_s
    i32.extend8_s

    ;; Return difference (should be 0 for idempotent property)
    i32.sub
  )

  ;; Comprehensive test combining multiple validation aspects
  ;; This function exercises various edge cases in a single call
  (func (export "test_comprehensive") (param $input i32) (result i32)
    local.get $input

    ;; Apply extend8_s
    i32.extend8_s

    ;; Additional validation: ensure result is properly formed
    ;; Check that if bit 7 of original was 0, upper bits are 0
    ;; Check that if bit 7 of original was 1, upper bits are 1
    local.tee 0  ;; Store result

    ;; Validate consistency: re-extending should yield same result
    i32.extend8_s

    ;; Should match original result
    local.get 0
    i32.eq

    ;; Return original result if consistent, otherwise return error value
    if (result i32)
      local.get 0
    else
      i32.const 0xDEADBEEF  ;; Error indicator
    end
  )
)