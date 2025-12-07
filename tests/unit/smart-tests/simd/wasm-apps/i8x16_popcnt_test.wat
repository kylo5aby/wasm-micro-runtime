(module
  ;; Import memory for test data
  (memory 1)

  ;; Test function: Mixed population count values
  ;; Tests various bit densities from 0-8 set bits per lane
  (func (export "test_mixed_values") (result i32)
    ;; Input vector: [0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0x55, 0xAA, 0xF0, 0x0F, 0x33, 0x66, 0x99]
    ;; Expected:      [0,    1,    2,    3,    4,    5,    6,    7,    8,    4,    4,    4,    4,    4,    4,    4]
    v128.const i8x16 0x00 0x01 0x03 0x07 0x0F 0x1F 0x3F 0x7F 0xFF 0x55 0xAA 0xF0 0x0F 0x33 0x66 0x99
    i8x16.popcnt
    ;; Validate lane 0 (0x00 -> 0)
    i8x16.extract_lane_u 0
    i32.const 0
    i32.eq
    if (result i32)
      ;; Validate lane 8 (0xFF -> 8)
      v128.const i8x16 0x00 0x01 0x03 0x07 0x0F 0x1F 0x3F 0x7F 0xFF 0x55 0xAA 0xF0 0x0F 0x33 0x66 0x99
      i8x16.popcnt
      i8x16.extract_lane_u 8
      i32.const 8
      i32.eq
      if (result i32)
        ;; Validate lane 4 (0x0F -> 4)
        v128.const i8x16 0x00 0x01 0x03 0x07 0x0F 0x1F 0x3F 0x7F 0xFF 0x55 0xAA 0xF0 0x0F 0x33 0x66 0x99
        i8x16.popcnt
        i8x16.extract_lane_u 4
        i32.const 4
        i32.eq
        if (result i32)
          ;; Validate lane 9 (0x55 -> 4)
          v128.const i8x16 0x00 0x01 0x03 0x07 0x0F 0x1F 0x3F 0x7F 0xFF 0x55 0xAA 0xF0 0x0F 0x33 0x66 0x99
          i8x16.popcnt
          i8x16.extract_lane_u 9
          i32.const 4
          i32.eq
        else
          i32.const 0
        end
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Boundary values (MIN/MAX)
  ;; Tests alternating 0x00 and 0xFF values
  (func (export "test_boundary_values") (result i32)
    ;; Input vector: alternating 0x00 and 0xFF
    ;; Expected:     alternating 0 and 8
    v128.const i8x16 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF
    i8x16.popcnt
    ;; Validate lane 0 (0x00 -> 0)
    i8x16.extract_lane_u 0
    i32.const 0
    i32.eq
    if (result i32)
      ;; Validate lane 1 (0xFF -> 8)
      v128.const i8x16 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF
      i8x16.popcnt
      i8x16.extract_lane_u 1
      i32.const 8
      i32.eq
      if (result i32)
        ;; Validate lane 14 (0x00 -> 0)
        v128.const i8x16 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF
        i8x16.popcnt
        i8x16.extract_lane_u 14
        i32.const 0
        i32.eq
        if (result i32)
          ;; Validate lane 15 (0xFF -> 8)
          v128.const i8x16 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF 0x00 0xFF
          i8x16.popcnt
          i8x16.extract_lane_u 15
          i32.const 8
          i32.eq
        else
          i32.const 0
        end
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: All zeros vector
  ;; Tests edge case with all lanes set to 0x00
  (func (export "test_all_zeros") (result i32)
    ;; Input vector: all 0x00
    ;; Expected:     all 0
    v128.const i8x16 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
    i8x16.popcnt
    ;; Validate multiple lanes are zero
    ;; Check lane 0
    i8x16.extract_lane_u 0
    i32.const 0
    i32.eq
    if (result i32)
      ;; Check lane 7
      v128.const i8x16 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
      i8x16.popcnt
      i8x16.extract_lane_u 7
      i32.const 0
      i32.eq
      if (result i32)
        ;; Check lane 15
        v128.const i8x16 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
        i8x16.popcnt
        i8x16.extract_lane_u 15
        i32.const 0
        i32.eq
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: All ones vector
  ;; Tests edge case with all lanes set to 0xFF
  (func (export "test_all_ones") (result i32)
    ;; Input vector: all 0xFF
    ;; Expected:     all 8
    v128.const i8x16 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
    i8x16.popcnt
    ;; Validate multiple lanes are 8
    ;; Check lane 0
    i8x16.extract_lane_u 0
    i32.const 8
    i32.eq
    if (result i32)
      ;; Check lane 7
      v128.const i8x16 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      i8x16.popcnt
      i8x16.extract_lane_u 7
      i32.const 8
      i32.eq
      if (result i32)
        ;; Check lane 15
        v128.const i8x16 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
        i8x16.popcnt
        i8x16.extract_lane_u 15
        i32.const 8
        i32.eq
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Complement patterns
  ;; Tests mathematical property: popcount(x) + popcount(~x) = 8
  (func (export "test_complement_patterns") (result i32)
    (local $result v128)
    ;; Input vector: complementary pairs [0x33, 0xCC, 0x0F, 0xF0, 0x3C, 0xC3, 0x5A, 0xA5, 0x69, 0x96, 0x71, 0x8E, 0x4B, 0xB4, 0x2D, 0xD2]
    ;; Expected popcount + complement popcount = 8 for each pair
    v128.const i8x16 0x33 0xCC 0x0F 0xF0 0x3C 0xC3 0x5A 0xA5 0x69 0x96 0x71 0x8E 0x4B 0xB4 0x2D 0xD2
    i8x16.popcnt
    ;; Store result for later comparison
    local.set $result

    ;; Validate complement property for first pair (0x33 -> 4, 0xCC -> 4, sum = 8)
    local.get $result
    i8x16.extract_lane_u 0  ;; popcount(0x33) = 4
    local.get $result
    i8x16.extract_lane_u 1  ;; popcount(0xCC) = 4
    i32.add
    i32.const 8
    i32.eq
    if (result i32)
      ;; Validate second pair (0x0F -> 4, 0xF0 -> 4, sum = 8)
      local.get $result
      i8x16.extract_lane_u 2  ;; popcount(0x0F) = 4
      local.get $result
      i8x16.extract_lane_u 3  ;; popcount(0xF0) = 4
      i32.add
      i32.const 8
      i32.eq
      if (result i32)
        ;; Validate third pair (0x3C -> 4, 0xC3 -> 4, sum = 8)
        local.get $result
        i8x16.extract_lane_u 4  ;; popcount(0x3C) = 4
        local.get $result
        i8x16.extract_lane_u 5  ;; popcount(0xC3) = 4
        i32.add
        i32.const 8
        i32.eq
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )

  ;; Test function: Single bit patterns
  ;; Tests power-of-2 values (single bit set)
  (func (export "test_single_bits") (result i32)
    ;; Input vector: single bit patterns [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80]
    ;; Expected:     all lanes should have popcount = 1
    v128.const i8x16 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80
    i8x16.popcnt
    ;; Validate multiple lanes have popcount = 1
    ;; Check lane 0 (0x01 -> 1)
    i8x16.extract_lane_u 0
    i32.const 1
    i32.eq
    if (result i32)
      ;; Check lane 7 (0x80 -> 1)
      v128.const i8x16 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80
      i8x16.popcnt
      i8x16.extract_lane_u 7
      i32.const 1
      i32.eq
      if (result i32)
        ;; Check lane 15 (0x80 -> 1)
        v128.const i8x16 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80
        i8x16.popcnt
        i8x16.extract_lane_u 15
        i32.const 1
        i32.eq
        if (result i32)
          ;; Check lane 4 (0x10 -> 1)
          v128.const i8x16 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80
          i8x16.popcnt
          i8x16.extract_lane_u 4
          i32.const 1
          i32.eq
        else
          i32.const 0
        end
      else
        i32.const 0
      end
    else
      i32.const 0
    end
  )
)