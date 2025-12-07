(module
  ;; i64.extend16_s Test Module
  ;;
  ;; This module provides comprehensive test functions for the i64.extend16_s WebAssembly opcode.
  ;; The i64.extend16_s instruction performs sign extension of the lower 16 bits of an i64 value
  ;; to a full 64-bit signed integer, treating the lower 16 bits as a signed 16-bit value.
  ;;
  ;; Functions exported:
  ;; - test_i64_extend16_s: Basic sign extension operation
  ;; - test_i64_extend16_s_double: Double application for idempotent testing
  ;; - test_i64_extend16_s_mask: Tests upper bit masking behavior
  ;;
  ;; Sign extension logic:
  ;; - If bit 15 (sign bit) is 0: Upper 48 bits filled with 0s (positive values 0-32767)
  ;; - If bit 15 (sign bit) is 1: Upper 48 bits filled with 1s (negative values 32768-65535 as unsigned, -32768 to -1 as signed)

  ;; Basic i64.extend16_s operation test function
  ;; Takes an i64 input, applies i64.extend16_s, and returns the result
  ;; Tests the fundamental sign extension behavior across all 16-bit value ranges
  (func (export "test_i64_extend16_s") (param $input i64) (result i64)
    ;; Load input parameter onto stack
    local.get $input

    ;; Apply i64.extend16_s operation
    ;; This instruction:
    ;; 1. Takes the lower 16 bits of the input i64 value
    ;; 2. Treats those 16 bits as a signed 16-bit integer (-32768 to +32767)
    ;; 3. Sign-extends to full 64-bit signed integer
    ;; 4. Upper 48 bits of input are completely ignored
    i64.extend16_s

    ;; Return the sign-extended result
  )

  ;; Double application test function for idempotent property validation
  ;; Applies i64.extend16_s twice to verify: extend16_s(extend16_s(x)) == extend16_s(x)
  ;; This validates that the operation is idempotent and stable
  (func (export "test_i64_extend16_s_double") (param $input i64) (result i64)
    ;; Load input parameter
    local.get $input

    ;; First application of i64.extend16_s
    i64.extend16_s

    ;; Second application of i64.extend16_s on the already extended result
    ;; This should produce the same result as a single application
    i64.extend16_s

    ;; Return the doubly-extended result
  )

  ;; Test upper bit masking behavior
  ;; Takes an i64 input with arbitrary upper bits, applies i64.extend16_s
  ;; Verifies that only the lower 16 bits affect the result
  (func (export "test_i64_extend16_s_mask") (param $input i64) (result i64)
    ;; Load input parameter (may have arbitrary upper 48 bits)
    local.get $input

    ;; Apply i64.extend16_s - only lower 16 bits should matter
    i64.extend16_s

    ;; Return result based only on lower 16 bits
  )

  ;; Test functions for specific boundary conditions and edge cases

  ;; Test positive 16-bit boundary (32767) - maximum positive signed 16-bit value
  (func (export "test_boundary_positive_max") (result i64)
    ;; Load 32767 (0x7FFF) - maximum positive signed 16-bit value
    i64.const 32767
    i64.extend16_s
  )

  ;; Test negative 16-bit boundary (-32768) - minimum negative signed 16-bit value
  (func (export "test_boundary_negative_min") (result i64)
    ;; Load -32768 (0x8000) - minimum negative signed 16-bit value
    i64.const -32768
    i64.extend16_s
  )

  ;; Test maximum unsigned 16-bit value (65535) - interpreted as -1 in signed 16-bit
  (func (export "test_boundary_unsigned_max") (result i64)
    ;; Load 65535 (0xFFFF) - interpreted as -1 in signed 16-bit
    i64.const 65535
    i64.extend16_s
  )

  ;; Test zero value - should remain unchanged
  (func (export "test_zero_value") (result i64)
    ;; Load 0 - should extend to 0
    i64.const 0
    i64.extend16_s
  )

  ;; Test upper bit masking - large value with specific lower 16 bits (positive)
  (func (export "test_upper_bit_masking_positive") (result i64)
    ;; Load large value with lower 16 bits = 0x7FFF (32767, positive)
    ;; Upper bits should be completely ignored
    i64.const 0x123456789ABC7FFF
    i64.extend16_s
  )

  ;; Test upper bit masking - large value with negative lower 16 bits
  (func (export "test_upper_bit_masking_negative") (result i64)
    ;; Load large value with lower 16 bits = 0x8000 (-32768)
    ;; Upper bits should be completely ignored
    i64.const 0xDEADBEEFCAFE8000
    i64.extend16_s
  )

  ;; Test all-ones pattern in lower 16 bits
  (func (export "test_all_ones_pattern") (result i64)
    ;; Load all-ones value (lower 16 bits = 0xFFFF = -1)
    i64.const 0xFFFFFFFFFFFFFFFF
    i64.extend16_s
  )

  ;; Test alternating bit pattern in lower 16 bits
  (func (export "test_alternating_pattern") (result i64)
    ;; Load value with lower 16 bits = 0xAAAA (-21846 in signed 16-bit)
    i64.const 0x5555AAAA5555AAAA
    i64.extend16_s
  )

  ;; Test specific 16-bit boundary values
  (func (export "test_boundary_adjacent_positive") (result i64)
    ;; Test 32766 (one below maximum positive)
    i64.const 32766
    i64.extend16_s
  )

  (func (export "test_boundary_adjacent_negative") (result i64)
    ;; Test -32767 (one above minimum negative)
    i64.const -32767
    i64.extend16_s
  )

  ;; Test values with various upper bit patterns
  (func (export "test_upper_bits_pattern1") (result i64)
    ;; Upper bits: 0xFFFF0000FFFF, lower 16 bits: 0x1234 (positive)
    i64.const 0xFFFF0000FFFF1234
    i64.extend16_s
  )

  (func (export "test_upper_bits_pattern2") (result i64)
    ;; Upper bits: 0x12345678ABCD, lower 16 bits: 0x9ABC (negative in 16-bit)
    i64.const 0x12345678ABCD9ABC
    i64.extend16_s
  )

  ;; Test mathematical property: commutativity with other operations
  (func (export "test_property_with_operations") (param $input i64) (result i64)
    ;; Test that extend16_s result is consistent with mathematical operations
    local.get $input
    i64.extend16_s
    ;; Return extended value for verification
  )
)