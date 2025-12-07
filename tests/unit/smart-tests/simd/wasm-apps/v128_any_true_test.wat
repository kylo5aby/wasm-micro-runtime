;; v128_any_true_test.wat - WebAssembly Text Format for v128.any_true opcode testing

(module
  ;; Function: test_v128_any_true
  ;; Description: Performs v128.any_true operation on input v128 vector
  ;; Parameters: Four i32 values representing one v128 input
  ;; Returns: One i32 value (0 or 1) representing boolean result
  (func $test_v128_any_true (export "test_v128_any_true")
    (param $in_0 i32) (param $in_1 i32) (param $in_2 i32) (param $in_3 i32)
    (result i32)
    (local $vector v128)

    ;; Create v128 vector from four i32 values
    (v128.const i32x4 0 0 0 0)
    (local.get $in_0)
    (i32x4.replace_lane 0)
    (local.get $in_1)
    (i32x4.replace_lane 1)
    (local.get $in_2)
    (i32x4.replace_lane 2)
    (local.get $in_3)
    (i32x4.replace_lane 3)
    (local.set $vector)

    ;; Apply v128.any_true boolean reduction operation
    (local.get $vector)
    (v128.any_true)
    ;; Result is already i32 (0 or 1), return directly
  )

  ;; Additional test functions for specific bit patterns

  ;; Function: test_v128_any_true_all_zeros
  ;; Description: Tests v128.any_true with all zeros vector
  ;; Returns: Should return 0
  (func $test_v128_any_true_all_zeros (export "test_v128_any_true_all_zeros")
    (result i32)
    ;; Create all zeros vector
    (v128.const i32x4 0 0 0 0)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_all_ones
  ;; Description: Tests v128.any_true with all ones vector
  ;; Returns: Should return 1
  (func $test_v128_any_true_all_ones (export "test_v128_any_true_all_ones")
    (result i32)
    ;; Create all ones vector using 0xFFFFFFFF for each i32 lane
    (v128.const i32x4 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_single_bit
  ;; Description: Tests v128.any_true with single bit set
  ;; Returns: Should return 1
  (func $test_v128_any_true_single_bit (export "test_v128_any_true_single_bit")
    (result i32)
    ;; Create vector with single bit set (0x00000001 in first lane)
    (v128.const i32x4 0x00000001 0 0 0)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_high_bit
  ;; Description: Tests v128.any_true with highest bit set
  ;; Returns: Should return 1
  (func $test_v128_any_true_high_bit (export "test_v128_any_true_high_bit")
    (result i32)
    ;; Create vector with highest bit set (0x80000000 in last lane)
    (v128.const i32x4 0 0 0 0x80000000)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_alternating
  ;; Description: Tests v128.any_true with alternating bit pattern
  ;; Returns: Should return 1
  (func $test_v128_any_true_alternating (export "test_v128_any_true_alternating")
    (result i32)
    ;; Create vector with alternating pattern (0x55555555 = 01010101...)
    (v128.const i32x4 0x55555555 0x55555555 0x55555555 0x55555555)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_nibble_pattern
  ;; Description: Tests v128.any_true with nibble alternating pattern
  ;; Returns: Should return 1
  (func $test_v128_any_true_nibble_pattern (export "test_v128_any_true_nibble_pattern")
    (result i32)
    ;; Create vector with nibble alternating pattern (0x0F0F0F0F)
    (v128.const i32x4 0x0F0F0F0F 0xF0F0F0F0 0x0F0F0F0F 0xF0F0F0F0)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_sparse_bits
  ;; Description: Tests v128.any_true with sparse bit pattern
  ;; Returns: Should return 1
  (func $test_v128_any_true_sparse_bits (export "test_v128_any_true_sparse_bits")
    (result i32)
    ;; Create vector with sparse bits (bits at positions 0, 32, 64, 96)
    (v128.const i32x4 0x00000001 0x00000001 0x00000001 0x00000001)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_boundary_bits
  ;; Description: Tests v128.any_true with bits at lane boundaries
  ;; Returns: Should return 1
  (func $test_v128_any_true_boundary_bits (export "test_v128_any_true_boundary_bits")
    (result i32)
    ;; Create vector with bits at 32-bit lane boundaries
    (v128.const i32x4 0x80000000 0x00000001 0x80000000 0x00000001)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_middle_lanes
  ;; Description: Tests v128.any_true with bits only in middle lanes
  ;; Returns: Should return 1
  (func $test_v128_any_true_middle_lanes (export "test_v128_any_true_middle_lanes")
    (result i32)
    ;; Create vector with bits only in lanes 1 and 2
    (v128.const i32x4 0 0x12345678 0x9ABCDEF0 0)
    (v128.any_true)
  )

  ;; Function: test_v128_any_true_edge_positions
  ;; Description: Tests v128.any_true with bits at extreme edge positions
  ;; Returns: Should return 1
  (func $test_v128_any_true_edge_positions (export "test_v128_any_true_edge_positions")
    (result i32)
    ;; Create vector with bit 0 and bit 127 set (first and last possible positions)
    (v128.const i32x4 0x00000001 0 0 0x80000000)
    (v128.any_true)
  )
)