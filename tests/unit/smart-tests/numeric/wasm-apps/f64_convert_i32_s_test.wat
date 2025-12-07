(module
  ;; Enhanced f64.convert_i32_s opcode test module
  ;; This module provides comprehensive test functions for the f64.convert_i32_s WebAssembly instruction
  ;; covering basic functionality, boundary conditions, sign preservation, and IEEE 754 compliance

  ;; Basic f64.convert_i32_s test function
  ;; Tests fundamental signed integer to double conversion with typical i32 inputs
  ;; Input: i32 signed value to be converted to f64
  ;; Output: f64 IEEE 754 double precision result with exact representation
  (func $convert_i32_s (export "convert_i32_s") (param $input i32) (result f64)
    ;; Push input parameter onto stack and execute f64.convert_i32_s
    local.get $input      ;; Load i32 parameter onto stack
    f64.convert_i32_s    ;; Execute signed conversion: i32 -> f64 with sign preservation
    ;; Result: f64 value containing exact mathematical equivalent of signed i32 input
  )

  ;; Boundary condition testing function
  ;; Tests conversion behavior at i32 boundaries (INT32_MIN, INT32_MAX) and sign transitions
  ;; Validates IEEE 754 exact representation properties for boundary values
  ;; Input: i32 boundary value for testing (INT32_MIN, INT32_MAX, -1, 0, 1)
  ;; Output: f64 result demonstrating exact boundary conversion without precision loss
  (func $convert_i32_s_boundary (export "convert_i32_s_boundary") (param $input i32) (result f64)
    ;; Load parameter and perform conversion with boundary-specific validation
    local.get $input      ;; Load i32 boundary test value onto stack
    f64.convert_i32_s    ;; Execute conversion: exact i32 -> f64 mapping at boundaries
    ;; Result: f64 with exact mathematical representation (no rounding for i32 range)
  )

  ;; IEEE 754 compliance validation function
  ;; Demonstrates that f64.convert_i32_s provides exact representation for all i32 values
  ;; since IEEE 754 double has 53-bit mantissa (greater than 32-bit i32 range)
  ;; Input: i32 value for IEEE 754 compliance testing
  ;; Output: f64 result proving exact representability of all i32 values in f64
  (func $convert_i32_s_ieee754 (export "convert_i32_s_ieee754") (param $value i32) (result f64)
    ;; Verify conversion maintains exact mathematical value with sign preservation
    local.get $value      ;; Load test value for IEEE 754 compliance verification
    f64.convert_i32_s    ;; Perform standard signed conversion operation
    ;; Result proves IEEE 754 property: all i32 values exactly representable in f64
  )

  ;; Extreme value testing function for maximum magnitude integers
  ;; Tests behavior with INT32_MIN, INT32_MAX, powers of 2, and special bit patterns
  ;; Validates sign preservation and exact conversion for extreme i32 values
  ;; Input: i32 extreme value (like INT32_MIN, INT32_MAX, large powers of 2)
  ;; Output: f64 demonstrating correct extreme value conversion with sign handling
  (func $convert_i32_s_extreme (export "convert_i32_s_extreme") (param $extreme i32) (result f64)
    ;; Handle extreme i32 values and verify correct sign-preserving conversion
    local.get $extreme    ;; Load extreme test value onto stack
    f64.convert_i32_s    ;; Execute signed conversion on extreme value
    ;; Result: f64 showing proper handling of extreme signed i32 inputs
  )

  ;; Sign preservation validation function
  ;; Specifically tests that negative i32 values convert to negative f64 values
  ;; and positive i32 values convert to positive f64 values (with zero special case)
  ;; Input: i32 value for sign preservation testing (positive, negative, or zero)
  ;; Output: f64 result demonstrating correct sign bit handling
  (func $convert_i32_s_sign (export "convert_i32_s_sign") (param $signed_input i32) (result f64)
    ;; Verify sign bit is correctly preserved during i32 -> f64 conversion
    local.get $signed_input ;; Load signed test value onto stack
    f64.convert_i32_s      ;; Execute conversion with sign preservation
    ;; Result: f64 with identical sign to input i32 (negative -> negative, positive -> positive)
  )

  ;; Mathematical identity verification function
  ;; Tests that converted f64 values maintain mathematical equivalence to original i32
  ;; Useful for verifying round-trip conversion properties where applicable
  ;; Input: i32 value for mathematical identity verification
  ;; Output: f64 result that should be mathematically equivalent to input
  (func $convert_i32_s_identity (export "convert_i32_s_identity") (param $math_input i32) (result f64)
    ;; Demonstrate mathematical identity: (double)(int32_t)x == (double)x for all i32 x
    local.get $math_input  ;; Load mathematical test value onto stack
    f64.convert_i32_s     ;; Execute conversion maintaining mathematical identity
    ;; Result: f64 that is mathematically identical to original i32 value
  )

  ;; Zero and unity testing function
  ;; Special testing for mathematically significant values: 0, 1, -1
  ;; These values are fundamental for identity operations and sign validation
  ;; Input: i32 special value (0, 1, or -1)
  ;; Output: f64 result for special mathematical values
  (func $convert_i32_s_special (export "convert_i32_s_special") (param $special i32) (result f64)
    ;; Test conversion of mathematically special values
    local.get $special    ;; Load special mathematical value (0, ±1)
    f64.convert_i32_s    ;; Execute conversion on special value
    ;; Result: f64 representing special mathematical constants (0.0, 1.0, -1.0)
  )

  ;; Large magnitude testing function
  ;; Tests conversion of large positive and negative i32 values near boundaries
  ;; Verifies that large magnitude conversions maintain precision and sign
  ;; Input: i32 large magnitude value (close to INT32_MIN or INT32_MAX)
  ;; Output: f64 result demonstrating large magnitude conversion accuracy
  (func $convert_i32_s_large (export "convert_i32_s_large") (param $large i32) (result f64)
    ;; Handle large magnitude i32 values and verify precision preservation
    local.get $large      ;; Load large magnitude test value onto stack
    f64.convert_i32_s    ;; Execute conversion on large magnitude value
    ;; Result: f64 showing exact conversion of large magnitude signed integers
  )
)