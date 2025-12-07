;; WebAssembly Text Format (WAT) for f64.convert_i64_u opcode testing
;;
;; This module provides test functions for validating f64.convert_i64_u opcode
;; behavior including basic conversion, boundary cases, precision limits, and
;; IEEE 754 compliance across the full unsigned 64-bit integer range.

(module
  ;; Import memory for potential debugging (optional)
  (memory (export "memory") 1)

  ;; Test function: Basic f64.convert_i64_u operation
  ;;
  ;; Takes an unsigned 64-bit integer (passed as two i32 values for compatibility)
  ;; and converts it to f64 using the f64.convert_i64_u opcode.
  ;; Returns the result as two i32 values representing the f64 bit pattern.
  ;;
  ;; Parameters:
  ;;   - low_bits (i32): Lower 32 bits of the i64 input
  ;;   - high_bits (i32): Upper 32 bits of the i64 input
  ;; Returns:
  ;;   - Two i32 values representing the f64 result bit pattern
  (func (export "test_f64_convert_i64_u")
        (param $low_bits i32) (param $high_bits i32)
        (result i32 i32)
    ;; Reconstruct i64 from two i32 parameters
    ;; Combine high and low 32-bit parts into single i64 value
    (local $input_i64 i64)

    ;; Build i64 from high and low 32-bit components
    ;; input_i64 = (high_bits << 32) | low_bits
    local.get $high_bits
    i64.extend_i32_u          ;; Convert high bits to i64 (zero-extended)
    i64.const 32
    i64.shl                   ;; Shift high bits left by 32 positions

    local.get $low_bits
    i64.extend_i32_u          ;; Convert low bits to i64 (zero-extended)

    i64.or                    ;; Combine high and low parts
    local.set $input_i64      ;; Store reconstructed i64 value

    ;; Perform f64.convert_i64_u conversion
    local.get $input_i64
    f64.convert_i64_u         ;; Convert unsigned i64 to f64 (THE OPCODE UNDER TEST)

    ;; Convert f64 result back to two i32 values for return
    ;; This allows precise bit-level verification of IEEE 754 compliance
    i64.reinterpret_f64       ;; Reinterpret f64 bits as i64

    ;; Extract and return low 32 bits
    i32.wrap_i64              ;; Get low 32 bits as i32

    ;; Extract and return high 32 bits
    local.get $input_i64
    f64.convert_i64_u         ;; Recompute conversion for high bits extraction
    i64.reinterpret_f64       ;; Reinterpret as i64
    i64.const 32
    i64.shr_u                 ;; Shift right to get high 32 bits
    i32.wrap_i64              ;; Convert high bits to i32

    ;; Return stack now contains (low_bits, high_bits) of f64 result
  )

  ;; Test function: Boundary value conversions
  ;;
  ;; Provides specific test cases for critical boundary values including
  ;; zero, maximum values, and precision boundaries around 2^53.
  ;; Each function returns the converted f64 as two i32 bit representations.

  (func (export "test_convert_zero") (result i32 i32)
    ;; Test conversion of zero
    i64.const 0
    f64.convert_i64_u
    i64.reinterpret_f64

    ;; Return low and high 32-bit components
    i32.wrap_i64              ;; Low 32 bits

    i64.const 0
    f64.convert_i64_u
    i64.reinterpret_f64
    i64.const 32
    i64.shr_u
    i32.wrap_i64              ;; High 32 bits
  )

  (func (export "test_convert_max_u64") (result i32 i32)
    ;; Test conversion of UINT64_MAX (0xFFFFFFFFFFFFFFFF)
    i64.const -1              ;; -1 in two's complement = UINT64_MAX unsigned
    f64.convert_i64_u
    i64.reinterpret_f64

    ;; Return bit representation as two i32 values
    i32.wrap_i64              ;; Low 32 bits

    i64.const -1
    f64.convert_i64_u
    i64.reinterpret_f64
    i64.const 32
    i64.shr_u
    i32.wrap_i64              ;; High 32 bits
  )

  (func (export "test_convert_2_pow_53") (result i32 i32)
    ;; Test conversion of 2^53 (precision boundary)
    i64.const 9007199254740992  ;; 2^53
    f64.convert_i64_u
    i64.reinterpret_f64

    i32.wrap_i64              ;; Low 32 bits

    i64.const 9007199254740992
    f64.convert_i64_u
    i64.reinterpret_f64
    i64.const 32
    i64.shr_u
    i32.wrap_i64              ;; High 32 bits
  )

  (func (export "test_convert_2_pow_53_plus_1") (result i32 i32)
    ;; Test conversion of 2^53 + 1 (beyond exact precision)
    i64.const 9007199254740993  ;; 2^53 + 1
    f64.convert_i64_u
    i64.reinterpret_f64

    i32.wrap_i64              ;; Low 32 bits

    i64.const 9007199254740993
    f64.convert_i64_u
    i64.reinterpret_f64
    i64.const 32
    i64.shr_u
    i32.wrap_i64              ;; High 32 bits
  )

  ;; Test function: Power-of-two conversions
  ;;
  ;; Tests various power-of-two values that should convert exactly
  ;; to IEEE 754 double precision format without precision loss.

  (func (export "test_convert_power_of_two") (param $exponent i32) (result i32 i32)
    ;; Convert 2^exponent to f64
    ;; Limited to reasonable exponent values (0-62 for i64 range)

    i64.const 1
    local.get $exponent
    i64.extend_i32_u
    i64.shl                   ;; Calculate 2^exponent

    f64.convert_i64_u         ;; Convert to f64
    i64.reinterpret_f64

    i32.wrap_i64              ;; Low 32 bits

    i64.const 1
    local.get $exponent
    i64.extend_i32_u
    i64.shl
    f64.convert_i64_u
    i64.reinterpret_f64
    i64.const 32
    i64.shr_u
    i32.wrap_i64              ;; High 32 bits
  )

  ;; Test function: Large unsigned values
  ;;
  ;; Tests values in the upper half of u64 range that would be negative
  ;; if interpreted as signed i64, verifying unsigned-specific behavior.

  (func (export "test_convert_large_unsigned") (result i32 i32)
    ;; Test large unsigned value with MSB set (2^63)
    i64.const 0x8000000000000000  ;; 2^63 - would be negative if signed
    f64.convert_i64_u
    i64.reinterpret_f64

    i32.wrap_i64              ;; Low 32 bits

    i64.const 0x8000000000000000
    f64.convert_i64_u
    i64.reinterpret_f64
    i64.const 32
    i64.shr_u
    i32.wrap_i64              ;; High 32 bits
  )

  ;; Test function: Precision loss demonstration
  ;;
  ;; Tests consecutive integers around the precision boundary to demonstrate
  ;; IEEE 754 rounding behavior for values that cannot be exactly represented.

  (func (export "test_precision_loss_sequence") (param $base_offset i32) (result i32 i32)
    ;; Test 2^53 + base_offset to show precision loss patterns
    i64.const 9007199254740992  ;; 2^53
    local.get $base_offset
    i64.extend_i32_s          ;; Allow negative offsets for comprehensive testing
    i64.add

    f64.convert_i64_u
    i64.reinterpret_f64

    i32.wrap_i64              ;; Low 32 bits

    i64.const 9007199254740992
    local.get $base_offset
    i64.extend_i32_s
    i64.add
    f64.convert_i64_u
    i64.reinterpret_f64
    i64.const 32
    i64.shr_u
    i32.wrap_i64              ;; High 32 bits
  )
)