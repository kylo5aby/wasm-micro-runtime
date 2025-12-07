(module
  ;; i32.rotl test module - Comprehensive testing of 32-bit rotate left operation
  ;; Tests basic functionality, boundary conditions, and mathematical properties

  ;; Main i32.rotl test function
  ;; Parameters: (value: i32, count: i32) -> (result: i32)
  ;; Performs i32.rotl operation and returns the result
  (func $i32_rotl_test (export "i32_rotl_test") (param $value i32) (param $count i32) (result i32)
    local.get $value    ;; Push value onto stack
    local.get $count    ;; Push rotation count onto stack
    i32.rotl           ;; Perform rotate left operation: rotl(value, count)
  )

  ;; Test function for basic rotation scenarios
  ;; Pre-configured test cases for validation
  (func $basic_rotation_test (export "basic_rotation_test") (result i32)
    ;; Test: rotl(0x12345678, 4) should return 0x23456781
    i32.const 0x12345678
    i32.const 4
    i32.rotl
  )

  ;; Test function for boundary rotation scenarios
  ;; Tests rotation by 0, 32, and other boundary values
  (func $boundary_rotation_test (export "boundary_rotation_test") (result i32)
    ;; Test: rotl(0x12345678, 0) should return 0x12345678 (identity)
    i32.const 0x12345678
    i32.const 0
    i32.rotl
  )

  ;; Test function for full rotation (32 positions)
  ;; Should return original value
  (func $full_rotation_test (export "full_rotation_test") (result i32)
    ;; Test: rotl(0x12345678, 32) should return 0x12345678
    i32.const 0x12345678
    i32.const 32
    i32.rotl
  )

  ;; Test function for modulo arithmetic behavior
  ;; Tests that rotation counts > 32 are properly handled
  (func $modulo_rotation_test (export "modulo_rotation_test") (result i32)
    ;; Test: rotl(0x12345678, 33) should equal rotl(0x12345678, 1)
    i32.const 0x12345678
    i32.const 33
    i32.rotl
  )

  ;; Test function for special bit patterns
  ;; Tests all zeros, all ones, and alternating patterns
  (func $special_pattern_test (export "special_pattern_test") (result i32)
    ;; Test: rotl(0xAAAAAAAA, 1) should return 0x55555555
    i32.const 0xAAAAAAAA
    i32.const 1
    i32.rotl
  )

  ;; Test function for all zeros pattern
  ;; All zeros should remain all zeros regardless of rotation count
  (func $zero_pattern_test (export "zero_pattern_test") (result i32)
    ;; Test: rotl(0x00000000, 15) should return 0x00000000
    i32.const 0x00000000
    i32.const 15
    i32.rotl
  )

  ;; Test function for all ones pattern
  ;; All ones should remain all ones regardless of rotation count
  (func $ones_pattern_test (export "ones_pattern_test") (result i32)
    ;; Test: rotl(0xFFFFFFFF, 7) should return 0xFFFFFFFF
    i32.const 0xFFFFFFFF
    i32.const 7
    i32.rotl
  )

  ;; Test function for high bit rotation
  ;; Tests behavior when high bit is set
  (func $high_bit_test (export "high_bit_test") (result i32)
    ;; Test: rotl(0x80000001, 1) should return 0x00000003
    i32.const 0x80000001
    i32.const 1
    i32.rotl
  )

  ;; Test function for maximum rotation count (31)
  ;; Tests largest meaningful rotation without full rotation
  (func $max_rotation_test (export "max_rotation_test") (result i32)
    ;; Test: rotl(0x12345678, 31) should return 0x091A2B3C
    i32.const 0x12345678
    i32.const 31
    i32.rotl
  )

  ;; Test function for large rotation count modulo behavior
  ;; Tests rotation by 64 (should equal rotation by 0)
  (func $large_count_test (export "large_count_test") (result i32)
    ;; Test: rotl(0xABCDEF01, 64) should return 0xABCDEF01
    i32.const 0xABCDEF01
    i32.const 64
    i32.rotl
  )

  ;; Test function for very large rotation count
  ;; Tests rotation by 100 (should equal rotation by 4: 100 % 32 = 4)
  (func $very_large_count_test (export "very_large_count_test") (result i32)
    ;; Test: rotl(0xABCDEF01, 100) equivalent to rotl(0xABCDEF01, 4)
    i32.const 0xABCDEF01
    i32.const 100
    i32.rotl
  )

  ;; Test function for distributive property validation
  ;; Tests that rotl(rotl(x, a), b) equals rotl(x, a+b)
  (func $distributive_test (export "distributive_test") (param $value i32) (param $count1 i32) (param $count2 i32) (result i32)
    local.get $value
    local.get $count1
    i32.rotl           ;; First rotation
    local.get $count2
    i32.rotl           ;; Second rotation
  )

  ;; Test function for single combined rotation (for distributive property)
  (func $combined_rotation_test (export "combined_rotation_test") (param $value i32) (param $total_count i32) (result i32)
    local.get $value
    local.get $total_count
    i32.rotl
  )

  ;; Test function for mixed bit pattern
  ;; Tests complex bit pattern preservation
  (func $mixed_pattern_test (export "mixed_pattern_test") (result i32)
    ;; Test: rotl(0xF0F0F0F0, 2)
    i32.const 0xF0F0F0F0
    i32.const 2
    i32.rotl
  )

  ;; Test function for single bit rotation
  ;; Tests rotation of single bit pattern
  (func $single_bit_test (export "single_bit_test") (result i32)
    ;; Test: rotl(0x00000001, 16) should return 0x00010000
    i32.const 0x00000001
    i32.const 16
    i32.rotl
  )

  ;; Test function for inverse rotation property
  ;; Helper function for testing rotl(rotl(x, n), 32-n) = x
  (func $inverse_rotation_test (export "inverse_rotation_test") (param $value i32) (param $count i32) (result i32)
    local.get $value
    local.get $count
    i32.rotl           ;; Forward rotation

    ;; Calculate inverse rotation count (32 - count)
    i32.const 32
    local.get $count
    i32.sub
    i32.rotl           ;; Inverse rotation
  )

  ;; Memory section for advanced testing (if needed)
  (memory $mem 1)

  ;; Data section with test patterns (if needed for future extensions)
  (data (i32.const 0) "\78\56\34\12")  ;; 0x12345678 in little-endian format
)