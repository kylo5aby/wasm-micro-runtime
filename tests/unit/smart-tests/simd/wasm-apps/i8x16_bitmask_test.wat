(module
  ;; Memory declaration for SIMD operations
  (memory 1)

  ;; Helper function to create i8x16 vector from 16 individual i8 lane values
  ;; Parameters: 16 i32 values (each representing an i8 lane value)
  ;; Returns: v128 vector constructed from the i8 lane values
  (func $make_i8x16_vector
    (param $l0 i32) (param $l1 i32) (param $l2 i32) (param $l3 i32)
    (param $l4 i32) (param $l5 i32) (param $l6 i32) (param $l7 i32)
    (param $l8 i32) (param $l9 i32) (param $l10 i32) (param $l11 i32)
    (param $l12 i32) (param $l13 i32) (param $l14 i32) (param $l15 i32)
    (result v128)

    ;; Create i8x16 vector by splatting first lane, then replacing each lane
    local.get $l0
    i8x16.splat

    local.get $l1
    i8x16.replace_lane 1

    local.get $l2
    i8x16.replace_lane 2

    local.get $l3
    i8x16.replace_lane 3

    local.get $l4
    i8x16.replace_lane 4

    local.get $l5
    i8x16.replace_lane 5

    local.get $l6
    i8x16.replace_lane 6

    local.get $l7
    i8x16.replace_lane 7

    local.get $l8
    i8x16.replace_lane 8

    local.get $l9
    i8x16.replace_lane 9

    local.get $l10
    i8x16.replace_lane 10

    local.get $l11
    i8x16.replace_lane 11

    local.get $l12
    i8x16.replace_lane 12

    local.get $l13
    i8x16.replace_lane 13

    local.get $l14
    i8x16.replace_lane 14

    local.get $l15
    i8x16.replace_lane 15
  )

  ;; Main test function: i8x16.bitmask with individual lane values
  ;; Creates vector from 16 lane values and extracts MSB bitmask
  ;; Parameters: 16 i32 values representing i8 lanes
  ;; Returns: i32 bitmask (16-bit pattern in lower bits)
  (func (export "test_i8x16_bitmask_lanes")
    (param $l0 i32) (param $l1 i32) (param $l2 i32) (param $l3 i32)
    (param $l4 i32) (param $l5 i32) (param $l6 i32) (param $l7 i32)
    (param $l8 i32) (param $l9 i32) (param $l10 i32) (param $l11 i32)
    (param $l12 i32) (param $l13 i32) (param $l14 i32) (param $l15 i32)
    (result i32)

    ;; Create i8x16 vector from individual lane values
    local.get $l0
    local.get $l1
    local.get $l2
    local.get $l3
    local.get $l4
    local.get $l5
    local.get $l6
    local.get $l7
    local.get $l8
    local.get $l9
    local.get $l10
    local.get $l11
    local.get $l12
    local.get $l13
    local.get $l14
    local.get $l15
    call $make_i8x16_vector

    ;; Extract MSB bitmask using i8x16.bitmask
    i8x16.bitmask
  )

  ;; Alternative test function: i8x16.bitmask with v128 input (four i32 values)
  ;; Parameters: four 32-bit parts representing v128 vector (little-endian)
  ;; Returns: i32 bitmask result
  (func (export "test_i8x16_bitmask_v128") (param $lo_lo i32) (param $lo_hi i32) (param $hi_lo i32) (param $hi_hi i32) (result i32)
    (local $low_i64 i64)
    (local $high_i64 i64)

    ;; Construct low i64 from two i32 values
    local.get $lo_hi
    i64.extend_i32_u
    i64.const 32
    i64.shl
    local.get $lo_lo
    i64.extend_i32_u
    i64.or
    local.set $low_i64

    ;; Construct high i64 from two i32 values
    local.get $hi_hi
    i64.extend_i32_u
    i64.const 32
    i64.shl
    local.get $hi_lo
    i64.extend_i32_u
    i64.or
    local.set $high_i64

    ;; Create v128 from two i64 values and extract bitmask
    local.get $low_i64
    i64x2.splat
    local.get $high_i64
    i64x2.replace_lane 1
    i8x16.bitmask
  )

  ;; Test function for specific known patterns
  ;; Returns bitmask for alternating positive/negative pattern
  (func (export "test_alternating_pattern") (result i32)
    ;; Create alternating pattern: [1, -1, 2, -2, 3, -3, 4, -4, 5, -5, 6, -6, 7, -7, 8, -8]
    ;; Expected result: 0xAAAA (bits 1,3,5,7,9,11,13,15 set)
    i32.const 1    ;; lane 0: positive (MSB = 0)
    i32.const -1   ;; lane 1: negative (MSB = 1) -> bit 1
    i32.const 2    ;; lane 2: positive (MSB = 0)
    i32.const -2   ;; lane 3: negative (MSB = 1) -> bit 3
    i32.const 3    ;; lane 4: positive (MSB = 0)
    i32.const -3   ;; lane 5: negative (MSB = 1) -> bit 5
    i32.const 4    ;; lane 6: positive (MSB = 0)
    i32.const -4   ;; lane 7: negative (MSB = 1) -> bit 7
    i32.const 5    ;; lane 8: positive (MSB = 0)
    i32.const -5   ;; lane 9: negative (MSB = 1) -> bit 9
    i32.const 6    ;; lane 10: positive (MSB = 0)
    i32.const -6   ;; lane 11: negative (MSB = 1) -> bit 11
    i32.const 7    ;; lane 12: positive (MSB = 0)
    i32.const -7   ;; lane 13: negative (MSB = 1) -> bit 13
    i32.const 8    ;; lane 14: positive (MSB = 0)
    i32.const -8   ;; lane 15: negative (MSB = 1) -> bit 15
    call $make_i8x16_vector
    i8x16.bitmask
  )

  ;; Test function for all MIN_VALUE (-128) pattern
  ;; Returns bitmask for all minimum i8 values
  (func (export "test_all_min_pattern") (result i32)
    ;; All lanes = -128 (MIN_VALUE): all MSBs = 1
    ;; Expected result: 0xFFFF (all 16 bits set)
    i32.const -128
    i8x16.splat
    i8x16.bitmask
  )

  ;; Test function for all MAX_VALUE (127) pattern
  ;; Returns bitmask for all maximum i8 values
  (func (export "test_all_max_pattern") (result i32)
    ;; All lanes = 127 (MAX_VALUE): all MSBs = 0
    ;; Expected result: 0x0000 (no bits set)
    i32.const 127
    i8x16.splat
    i8x16.bitmask
  )

  ;; Test function for all zero pattern
  ;; Returns bitmask for all zero values
  (func (export "test_all_zero_pattern") (result i32)
    ;; All lanes = 0: all MSBs = 0
    ;; Expected result: 0x0000 (no bits set)
    i32.const 0
    i8x16.splat
    i8x16.bitmask
  )

  ;; Test function for all -1 pattern
  ;; Returns bitmask for all -1 values
  (func (export "test_all_neg_one_pattern") (result i32)
    ;; All lanes = -1: all MSBs = 1
    ;; Expected result: 0xFFFF (all 16 bits set)
    i32.const -1
    i8x16.splat
    i8x16.bitmask
  )

  ;; Test function for single lane patterns (power of 2 results)
  ;; Returns bitmask for single negative lane at specified position
  (func (export "test_single_lane_0") (result i32)
    ;; Only lane 0 = -1, rest = 1
    ;; Expected result: 0x0001 (bit 0 set)
    i32.const -1   ;; lane 0: negative -> bit 0
    i32.const 1    ;; lanes 1-15: positive
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    call $make_i8x16_vector
    i8x16.bitmask
  )

  (func (export "test_single_lane_15") (result i32)
    ;; Only lane 15 = -1, rest = 1
    ;; Expected result: 0x8000 (bit 15 set)
    i32.const 1    ;; lanes 0-14: positive
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const 1
    i32.const -1   ;; lane 15: negative -> bit 15
    call $make_i8x16_vector
    i8x16.bitmask
  )

  ;; Test function for symmetric pattern: first half positive, second half negative
  (func (export "test_symmetric_pattern") (result i32)
    ;; First 8 lanes positive, last 8 lanes negative
    ;; Expected result: 0xFF00 (bits 8-15 set)
    i32.const 10   ;; lanes 0-7: positive
    i32.const 20
    i32.const 30
    i32.const 40
    i32.const 50
    i32.const 60
    i32.const 70
    i32.const 80
    i32.const -10  ;; lanes 8-15: negative -> bits 8-15
    i32.const -20
    i32.const -30
    i32.const -40
    i32.const -50
    i32.const -60
    i32.const -70
    i32.const -80
    call $make_i8x16_vector
    i8x16.bitmask
  )

  ;; Test function to validate MSB extraction correctness
  ;; Uses known bit pattern to verify each lane contributes correctly
  (func (export "test_msb_validation") (result i32)
    ;; Mixed pattern with predictable MSB positions
    ;; Pattern: lanes 4,5,6,7 negative, rest positive
    ;; Expected result: 0x00F0 (bits 4,5,6,7 set)
    i32.const 10   ;; lanes 0-3: positive
    i32.const 20
    i32.const 30
    i32.const 40
    i32.const -50  ;; lanes 4-7: negative -> bits 4-7
    i32.const -60
    i32.const -70
    i32.const -80
    i32.const 90   ;; lanes 8-15: positive
    i32.const 100
    i32.const 110
    i32.const 120
    i32.const 13
    i32.const 14
    i32.const 15
    i32.const 16
    call $make_i8x16_vector
    i8x16.bitmask
  )

  ;; Simple test function for basic compilation verification
  (func (export "test_simple") (result i32)
    i32.const 42
  )
)