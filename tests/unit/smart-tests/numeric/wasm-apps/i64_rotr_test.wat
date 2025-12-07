(module
  ;; Primary i64.rotr test function - takes two i64 parameters and returns rotated result
  (func (export "i64_rotr_test") (param $value i64) (param $count i64) (result i64)
    local.get $value
    local.get $count
    i64.rotr)

  ;; Basic rotation test function for 4-bit shifts
  (func (export "test_basic_rotr") (param $input i64) (result i64)
    local.get $input
    i64.const 4  ;; Fixed 4-bit rotation for basic testing
    i64.rotr)

  ;; Zero rotation test - should return original value
  (func (export "test_zero_rotation") (param $input i64) (result i64)
    local.get $input
    i64.const 0
    i64.rotr)

  ;; Full rotation test - rotate by 64 should return original value
  (func (export "test_full_rotation") (param $input i64) (result i64)
    local.get $input
    i64.const 64
    i64.rotr)

  ;; Boundary rotation test - rotate by 63 (maximum meaningful rotation)
  (func (export "test_boundary_rotation") (param $input i64) (result i64)
    local.get $input
    i64.const 63
    i64.rotr)

  ;; Large count rotation test - test modulo 64 behavior
  (func (export "test_large_count") (param $input i64) (result i64)
    local.get $input
    i64.const 65  ;; Should be equivalent to rotation by 1 (65 mod 64 = 1)
    i64.rotr)

  ;; Extreme large count test - test with very large rotation count
  (func (export "test_extreme_count") (param $input i64) (result i64)
    local.get $input
    i64.const 0x7FFFFFFFFFFFFFFF  ;; Large count to test modulo behavior
    i64.rotr)

  ;; Byte rotation test - rotate by 8 bits
  (func (export "test_byte_rotation") (param $input i64) (result i64)
    local.get $input
    i64.const 8
    i64.rotr)

  ;; Half rotation test - rotate by 32 bits
  (func (export "test_half_rotation") (param $input i64) (result i64)
    local.get $input
    i64.const 32
    i64.rotr)

  ;; Single bit MSB test - tests MSB rotation
  (func (export "test_msb_rotation") (result i64)
    i64.const 0x8000000000000000  ;; MSB set
    i64.const 1                   ;; Rotate right by 1
    i64.rotr)

  ;; All ones pattern test
  (func (export "test_all_ones") (result i64)
    i64.const 0xFFFFFFFFFFFFFFFF  ;; All bits set
    i64.const 1                   ;; Rotate right by 1
    i64.rotr)

  ;; Mathematical property test - double rotation for inverse testing
  (func (export "test_double_rotation") (param $value i64) (param $count1 i64) (param $count2 i64) (result i64)
    local.get $value
    local.get $count1
    i64.rotr
    local.get $count2
    i64.rotr)

  ;; Alternating bit pattern test
  (func (export "test_alternating_pattern") (result i64)
    i64.const 0xAAAAAAAAAAAAAAAA  ;; Alternating pattern
    i64.const 1                    ;; Rotate right by 1
    i64.rotr)

  ;; Powers of 2 test function - single bit rotation
  (func (export "test_power_of_2") (result i64)
    i64.const 1    ;; Single LSB bit
    i64.const 1    ;; Rotate right by 1
    i64.rotr)

  ;; Identity test with zero count
  (func (export "test_identity") (param $input i64) (result i64)
    local.get $input
    i64.const 0  ;; Zero rotation count
    i64.rotr)
)