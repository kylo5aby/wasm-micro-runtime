(module
  ;; Test function for f32.reinterpret_i32 opcode
  ;; Takes an i32 parameter and reinterprets its bit pattern as f32
  (func (export "f32_reinterpret_i32_test") (param $input i32) (result f32)
    ;; Push the i32 parameter onto stack
    local.get $input
    ;; Apply f32.reinterpret_i32 opcode - reinterpret i32 bits as f32
    f32.reinterpret_i32
    ;; Return the f32 result
  )

  ;; Test function for multiple f32.reinterpret_i32 operations
  ;; Demonstrates batch reinterpretation of multiple values
  (func (export "f32_reinterpret_i32_batch") (param $input1 i32) (param $input2 i32) (result f32 f32)
    ;; First input
    local.get $input1
    f32.reinterpret_i32
    ;; Second input
    local.get $input2
    f32.reinterpret_i32
    ;; Return both results (f32, f32)
  )

  ;; Test function for IEEE 754 special values
  ;; Tests specific bit patterns that produce special IEEE 754 values
  (func (export "f32_reinterpret_i32_special") (param $pattern i32) (result f32)
    local.get $pattern
    f32.reinterpret_i32
  )

  ;; Test function demonstrating round-trip conversion
  ;; f32.reinterpret_i32 followed by i32.reinterpret_f32 should be identity
  (func (export "f32_reinterpret_i32_roundtrip") (param $input i32) (result i32)
    local.get $input
    ;; i32 -> f32 reinterpretation
    f32.reinterpret_i32
    ;; f32 -> i32 reinterpretation (inverse operation)
    i32.reinterpret_f32
  )

  ;; Test function for denormal value handling
  ;; Tests reinterpretation of bit patterns representing denormal floats
  (func (export "f32_reinterpret_i32_denormal") (param $denormal_bits i32) (result f32)
    local.get $denormal_bits
    f32.reinterpret_i32
  )

  ;; Test function for NaN pattern preservation
  ;; Verifies that NaN bit patterns are preserved through reinterpretation
  (func (export "f32_reinterpret_i32_nan") (param $nan_bits i32) (result f32)
    local.get $nan_bits
    f32.reinterpret_i32
  )

  ;; Test function for infinity patterns
  ;; Tests reinterpretation of positive and negative infinity bit patterns
  (func (export "f32_reinterpret_i32_infinity") (param $inf_bits i32) (result f32)
    local.get $inf_bits
    f32.reinterpret_i32
  )

  ;; Test function for zero patterns
  ;; Tests both positive zero (0x00000000) and negative zero (0x80000000)
  (func (export "f32_reinterpret_i32_zero") (param $zero_bits i32) (result f32)
    local.get $zero_bits
    f32.reinterpret_i32
  )

  ;; Comprehensive validation function that tests multiple categories
  (func (export "f32_reinterpret_i32_comprehensive")
        (param $normal i32) (param $special i32) (param $boundary i32) (result f32 f32 f32)
    ;; Normal value
    local.get $normal
    f32.reinterpret_i32

    ;; Special value
    local.get $special
    f32.reinterpret_i32

    ;; Boundary value
    local.get $boundary
    f32.reinterpret_i32
  )
)