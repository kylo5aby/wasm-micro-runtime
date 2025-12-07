(module
  ;; Import SIMD support from the environment
  (import "env" "memory" (memory 1))

  ;; Test i8x16.all_true with all lanes set to 1 (non-zero)
  ;; Expected result: 1 (true)
  (func (export "test_i8x16_all_true_all_ones") (result i32)
    ;; Create v128 with all lanes = 1
    i32.const 1
    i8x16.splat
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test i8x16.all_true with mixed positive and negative non-zero values
  ;; Expected result: 1 (true)
  (func (export "test_i8x16_all_true_mixed_signs") (result i32)
    ;; Create v128 with alternating positive/negative non-zero values
    ;; Lanes: [1, -1, 2, -2, 3, -3, 4, -4, 5, -5, 6, -6, 7, -7, 8, -8]
    v128.const i8x16
      0x01 0xFF 0x02 0xFE 0x03 0xFD 0x04 0xFC
      0x05 0xFB 0x06 0xFA 0x07 0xF9 0x08 0xF8
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test i8x16.all_true with MIN_VALUE (-128) and MAX_VALUE (+127)
  ;; Expected result: 1 (true)
  (func (export "test_i8x16_all_true_min_max") (result i32)
    ;; Create v128 with alternating MIN_VALUE and MAX_VALUE
    ;; Lanes: [-128, 127, -128, 127, -128, 127, -128, 127, -128, 127, -128, 127, -128, 127, -128, 127]
    v128.const i8x16
      0x80 0x7F 0x80 0x7F 0x80 0x7F 0x80 0x7F
      0x80 0x7F 0x80 0x7F 0x80 0x7F 0x80 0x7F
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test i8x16.all_true with single zero lane among non-zero lanes
  ;; Expected result: 0 (false)
  (func (export "test_i8x16_all_true_single_zero") (result i32)
    ;; Create v128 with 15 non-zero lanes and 1 zero lane at position 8
    ;; Lanes: [1, 2, 3, 4, 5, 6, 7, 8, 0, 9, 10, 11, 12, 13, 14, 15]
    v128.const i8x16
      0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08
      0x00 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test i8x16.all_true with all lanes set to zero
  ;; Expected result: 0 (false)
  (func (export "test_i8x16_all_true_all_zeros") (result i32)
    ;; Create v128 with all lanes = 0
    i32.const 0
    i8x16.splat
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test i8x16.all_true with alternating zero/non-zero pattern
  ;; Expected result: 0 (false)
  (func (export "test_i8x16_all_true_alternating") (result i32)
    ;; Create v128 with alternating zero and non-zero values
    ;; Lanes: [1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0]
    v128.const i8x16
      0x01 0x00 0x02 0x00 0x03 0x00 0x04 0x00
      0x05 0x00 0x06 0x00 0x07 0x00 0x08 0x00
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Additional comprehensive test functions for edge cases

  ;; Test with all negative ones (-1 in all lanes)
  ;; Expected result: 1 (true)
  (func (export "test_i8x16_all_true_negative_ones") (result i32)
    ;; Create v128 with all lanes = -1 (0xFF)
    i32.const -1
    i8x16.splat
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test with zero in first lane only
  ;; Expected result: 0 (false)
  (func (export "test_i8x16_all_true_zero_first_lane") (result i32)
    ;; Create v128 with zero in first lane, non-zero in remaining lanes
    ;; Lanes: [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
    v128.const i8x16
      0x00 0x01 0x01 0x01 0x01 0x01 0x01 0x01
      0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test with zero in last lane only
  ;; Expected result: 0 (false)
  (func (export "test_i8x16_all_true_zero_last_lane") (result i32)
    ;; Create v128 with non-zero in first 15 lanes, zero in last lane
    ;; Lanes: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0]
    v128.const i8x16
      0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01
      0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x00
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test with random non-zero values across all lanes
  ;; Expected result: 1 (true)
  (func (export "test_i8x16_all_true_random_nonzero") (result i32)
    ;; Create v128 with diverse non-zero values
    ;; Lanes: [42, -17, 100, -50, 23, -99, 77, -33, 55, -88, 11, -22, 66, -44, 99, -11]
    v128.const i8x16
      0x2A 0xEF 0x64 0xCE 0x17 0x9D 0x4D 0xDF
      0x37 0xA8 0x0B 0xEA 0x42 0xD4 0x63 0xF5
    ;; Apply i8x16.all_true
    i8x16.all_true
  )

  ;; Test with multiple zeros scattered throughout
  ;; Expected result: 0 (false)
  (func (export "test_i8x16_all_true_multiple_zeros") (result i32)
    ;; Create v128 with zeros at positions 2, 5, 8, 11, 14
    ;; Lanes: [1, 2, 0, 4, 5, 0, 7, 8, 0, 10, 11, 0, 13, 14, 0, 16]
    v128.const i8x16
      0x01 0x02 0x00 0x04 0x05 0x00 0x07 0x08
      0x00 0x0A 0x0B 0x00 0x0D 0x0E 0x00 0x10
    ;; Apply i8x16.all_true
    i8x16.all_true
  )
)