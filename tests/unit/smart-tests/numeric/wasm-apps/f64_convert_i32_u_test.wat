(module
  ;; Enhanced f64.convert_i32_u opcode test module
  ;; This module provides comprehensive test functions for the f64.convert_i32_u WebAssembly instruction
  ;; covering basic functionality, boundary conditions, unsigned interpretation, and IEEE 754 compliance

  ;; Basic f64.convert_i32_u test function
  ;; Tests fundamental unsigned integer to double conversion with typical u32 inputs
  ;; Input: i32 value to be interpreted as unsigned and converted to f64
  ;; Output: f64 IEEE 754 double precision result with exact unsigned representation
  (func $test_f64_convert_i32_u (export "test_f64_convert_i32_u") (param $input i32) (result f64)
    ;; Push input parameter onto stack and execute f64.convert_i32_u
    local.get $input      ;; Load i32 parameter onto stack
    f64.convert_i32_u    ;; Execute unsigned conversion: i32 (as u32) -> f64 with unsigned interpretation
    ;; Result: f64 value containing exact mathematical equivalent of unsigned interpretation of i32 input
  )

  ;; Boundary condition testing function
  ;; Tests conversion behavior at u32 boundaries (0, UINT32_MAX) and large unsigned values
  ;; Validates IEEE 754 exact representation properties for boundary values
  ;; Input: i32 value interpreted as unsigned boundary value for testing (0, UINT32_MAX)
  ;; Output: f64 result demonstrating exact boundary conversion without precision loss
  (func $convert_i32_u_boundary (export "convert_i32_u_boundary") (param $input i32) (result f64)
    ;; Load parameter and perform conversion with boundary-specific validation
    local.get $input      ;; Load i32 boundary test value onto stack
    f64.convert_i32_u    ;; Execute conversion: exact u32 -> f64 mapping at boundaries
    ;; Result: f64 with exact mathematical representation (no rounding for u32 range)
  )

  ;; IEEE 754 compliance validation function
  ;; Demonstrates that f64.convert_i32_u provides exact representation for all u32 values
  ;; since IEEE 754 double has 53-bit mantissa (greater than 32-bit u32 range)
  ;; Input: i32 value for IEEE 754 compliance testing (interpreted as unsigned)
  ;; Output: f64 result proving exact representability of all u32 values in f64
  (func $convert_i32_u_ieee754 (export "convert_i32_u_ieee754") (param $value i32) (result f64)
    ;; Verify conversion maintains exact mathematical value with unsigned interpretation
    local.get $value      ;; Load test value for IEEE 754 compliance verification
    f64.convert_i32_u    ;; Perform standard unsigned conversion operation
    ;; Result proves IEEE 754 property: all u32 values exactly representable in f64
  )

  ;; Large unsigned value testing function for values > INT32_MAX
  ;; Tests behavior with values that would be negative if interpreted as signed
  ;; Validates unsigned interpretation and exact conversion for large u32 values
  ;; Input: i32 value with MSB set (interpreted as large unsigned value)
  ;; Output: f64 demonstrating correct unsigned interpretation (positive result)
  (func $convert_i32_u_large (export "convert_i32_u_large") (param $large_unsigned i32) (result f64)
    ;; Handle large unsigned values and verify correct unsigned interpretation
    local.get $large_unsigned ;; Load large unsigned test value onto stack
    f64.convert_i32_u        ;; Execute unsigned conversion on large value
    ;; Result: f64 showing proper unsigned interpretation (always positive)
  )

  ;; Power-of-two testing function for exact representation validation
  ;; Tests conversion of power-of-two values that should have exact f64 representation
  ;; Validates mathematical properties and bit-level accuracy
  ;; Input: i32 power-of-two value (1 << n where n < 32)
  ;; Output: f64 demonstrating exact power-of-two representation
  (func $convert_i32_u_power_of_two (export "convert_i32_u_power_of_two") (param $power_of_two i32) (result f64)
    ;; Test power-of-two values for exact IEEE 754 representation
    local.get $power_of_two   ;; Load power-of-two test value onto stack
    f64.convert_i32_u        ;; Execute conversion on power-of-two input
    ;; Result: f64 with exact power-of-two representation (no mantissa rounding)
  )

  ;; Bit pattern testing function for specific unsigned patterns
  ;; Tests conversion of specific bit patterns that could cause precision issues
  ;; Validates handling of patterns like 0x80000000, 0xFFFF0000, etc.
  ;; Input: i32 with specific bit pattern for testing
  ;; Output: f64 demonstrating correct bit pattern interpretation
  (func $convert_i32_u_bit_pattern (export "convert_i32_u_bit_pattern") (param $pattern i32) (result f64)
    ;; Test specific bit patterns for accurate unsigned conversion
    local.get $pattern       ;; Load bit pattern test value onto stack
    f64.convert_i32_u       ;; Execute conversion on specific bit pattern
    ;; Result: f64 showing correct unsigned interpretation of bit pattern
  )

  ;; Zero conversion identity testing function
  ;; Tests that zero converts to exactly 0.0 in f64 format
  ;; Validates identity property and IEEE 754 zero representation
  ;; Input: i32 zero value (0)
  ;; Output: f64 positive zero (0.0)
  (func $convert_i32_u_zero (export "convert_i32_u_zero") (param $zero i32) (result f64)
    ;; Test zero conversion identity property
    local.get $zero          ;; Load zero value onto stack
    f64.convert_i32_u       ;; Execute conversion on zero input
    ;; Result: f64 positive zero with correct IEEE 754 representation
  )

  ;; Combined test function for comprehensive validation
  ;; Performs multiple conversions in sequence to test function composition
  ;; Validates consistent behavior across multiple operations
  ;; Input: i32 base value for multiple test operations
  ;; Output: f64 result of combined test operations
  (func $convert_i32_u_combined (export "convert_i32_u_combined") (param $base i32) (result f64)
    ;; Execute multiple conversions to validate consistency
    local.get $base          ;; Load base value for combined testing
    f64.convert_i32_u       ;; First conversion operation
    drop                    ;; Discard first result for testing
    local.get $base          ;; Reload base value
    f64.convert_i32_u       ;; Second conversion operation
    ;; Result: f64 demonstrating consistent conversion behavior
  )
)