(module
  ;; Enhanced f64.convert_i64_s opcode test module
  ;; This module provides comprehensive test functions for the f64.convert_i64_s WebAssembly instruction
  ;; covering basic functionality, boundary conditions, precision limits, and IEEE 754 compliance

  ;; Basic f64.convert_i64_s test function
  ;; Tests fundamental signed 64-bit integer to double conversion with typical i64 inputs
  ;; Input: i64 signed value to be converted to f64
  ;; Output: f64 IEEE 754 double precision result with sign preservation
  (func $convert_i64_s (export "convert_i64_s") (param $input i64) (result f64)
    ;; Push input parameter onto stack and execute f64.convert_i64_s
    local.get $input      ;; Load i64 parameter onto stack
    f64.convert_i64_s    ;; Execute signed conversion: i64 -> f64 with sign preservation
    ;; Result: f64 value containing IEEE 754 representation of signed i64 input
  )

  ;; Boundary condition testing function
  ;; Tests conversion behavior at i64 boundaries (INT64_MIN, INT64_MAX) and sign transitions
  ;; Validates IEEE 754 representation with expected precision loss for large values
  ;; Input: i64 boundary value for testing (INT64_MIN, INT64_MAX, -1, 0, 1)
  ;; Output: f64 result demonstrating boundary conversion with IEEE 754 compliance
  (func $convert_i64_s_boundary (export "convert_i64_s_boundary") (param $input i64) (result f64)
    ;; Load parameter and perform conversion with boundary-specific validation
    local.get $input      ;; Load i64 boundary test value onto stack
    f64.convert_i64_s    ;; Execute conversion: i64 -> f64 with potential precision loss
    ;; Result: f64 with IEEE 754 compliant representation (may have precision loss)
  )

  ;; Precision limit testing function
  ;; Tests conversion behavior around IEEE 754 double precision mantissa limit (53 bits)
  ;; Validates proper rounding behavior for integers exceeding exact representation
  ;; Input: i64 value for precision testing (around 2^53 boundary)
  ;; Output: f64 result demonstrating IEEE 754 rounding behavior
  (func $convert_i64_s_precision (export "convert_i64_s_precision") (param $value i64) (result f64)
    ;; Test conversion at IEEE 754 double precision boundaries
    local.get $value      ;; Load precision test value onto stack
    f64.convert_i64_s    ;; Execute conversion with IEEE 754 rounding rules
    ;; Result: f64 showing proper rounding for values exceeding mantissa precision
  )

  ;; Large magnitude testing function
  ;; Tests conversion of very large positive and negative i64 values
  ;; Validates that extreme magnitude conversions follow IEEE 754 standards
  ;; Input: i64 extreme magnitude value (close to INT64_MIN or INT64_MAX)
  ;; Output: f64 result demonstrating extreme magnitude conversion with precision loss
  (func $convert_i64_s_extreme (export "convert_i64_s_extreme") (param $extreme i64) (result f64)
    ;; Handle extreme magnitude i64 values with expected precision loss
    local.get $extreme    ;; Load extreme magnitude test value onto stack
    f64.convert_i64_s    ;; Execute conversion on extreme value
    ;; Result: f64 showing IEEE 754 compliant conversion of extreme signed integers
  )

  ;; Sign preservation validation function
  ;; Specifically tests that negative i64 values convert to negative f64 values
  ;; and positive i64 values convert to positive f64 values (with zero special case)
  ;; Input: i64 value for sign preservation testing (positive, negative, or zero)
  ;; Output: f64 result demonstrating correct sign bit handling
  (func $convert_i64_s_sign (export "convert_i64_s_sign") (param $signed_input i64) (result f64)
    ;; Verify sign bit is correctly preserved during i64 -> f64 conversion
    local.get $signed_input ;; Load signed test value onto stack
    f64.convert_i64_s      ;; Execute conversion with sign preservation
    ;; Result: f64 with identical sign to input i64 (negative -> negative, positive -> positive)
  )

  ;; Powers of 2 testing function
  ;; Tests conversion of power-of-2 values which should be exactly representable
  ;; Validates that powers of 2 within IEEE 754 range convert exactly
  ;; Input: i64 power of 2 value for exact representation testing
  ;; Output: f64 result that should be exactly representable
  (func $convert_i64_s_power2 (export "convert_i64_s_power2") (param $power2 i64) (result f64)
    ;; Test conversion of powers of 2 for exact IEEE 754 representation
    local.get $power2     ;; Load power-of-2 test value onto stack
    f64.convert_i64_s    ;; Execute conversion on power-of-2 value
    ;; Result: f64 showing exact representation of powers of 2 within precision limits
  )

  ;; Rounding behavior testing function
  ;; Tests IEEE 754 round-to-nearest-even behavior for values requiring rounding
  ;; Validates proper rounding rules when i64 exceeds f64 mantissa precision
  ;; Input: i64 value designed to test specific rounding scenarios
  ;; Output: f64 result demonstrating IEEE 754 rounding compliance
  (func $convert_i64_s_rounding (export "convert_i64_s_rounding") (param $round_test i64) (result f64)
    ;; Test IEEE 754 rounding behavior for precision-exceeding values
    local.get $round_test ;; Load rounding test value onto stack
    f64.convert_i64_s    ;; Execute conversion with IEEE 754 rounding
    ;; Result: f64 showing proper round-to-nearest-even behavior
  )

  ;; Zero and special values testing function
  ;; Special testing for mathematically significant values: 0, 1, -1, powers of 2
  ;; These values are fundamental for identity operations and precision validation
  ;; Input: i64 special value (0, 1, -1, or specific mathematical constants)
  ;; Output: f64 result for special mathematical values
  (func $convert_i64_s_special (export "convert_i64_s_special") (param $special i64) (result f64)
    ;; Test conversion of mathematically special values
    local.get $special    ;; Load special mathematical value onto stack
    f64.convert_i64_s    ;; Execute conversion on special value
    ;; Result: f64 representing special mathematical constants with sign preservation
  )

  ;; Mathematical identity verification function
  ;; Tests that converted f64 values maintain mathematical equivalence where possible
  ;; Useful for verifying conversion properties within IEEE 754 precision limits
  ;; Input: i64 value for mathematical identity verification
  ;; Output: f64 result that represents the closest IEEE 754 approximation
  (func $convert_i64_s_identity (export "convert_i64_s_identity") (param $math_input i64) (result f64)
    ;; Demonstrate mathematical approximation: f64 closest to i64 input value
    local.get $math_input  ;; Load mathematical test value onto stack
    f64.convert_i64_s     ;; Execute conversion with IEEE 754 approximation
    ;; Result: f64 that is the best IEEE 754 representation of original i64 value
  )
)