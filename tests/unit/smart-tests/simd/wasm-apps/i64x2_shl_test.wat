(module
  ;; Test function for i64x2.shl opcode validation
  ;; Takes two 64-bit values as 4 i32 parameters (2 lanes split into low/high 32-bit parts)
  ;; and one shift count, returns 4 i32 values representing the shifted result lanes

  (func (export "test_i64x2_shl")
    (param $lane0_low i32) (param $lane0_high i32)
    (param $lane1_low i32) (param $lane1_high i32)
    (param $shift_count i32)
    (result i32 i32 i32 i32)

    (local $input_vector v128)
    (local $result_vector v128)

    ;; Construct input vector from lane parameters
    ;; Combine low and high 32-bit parts to form 64-bit values
    (local.set $input_vector
      (i64x2.replace_lane 1
        (i64x2.replace_lane 0
          (v128.const i64x2 0 0)
          (i64.or
            (i64.extend_i32_u (local.get $lane0_low))
            (i64.shl
              (i64.extend_i32_u (local.get $lane0_high))
              (i64.const 32)
            )
          )
        )
        (i64.or
          (i64.extend_i32_u (local.get $lane1_low))
          (i64.shl
            (i64.extend_i32_u (local.get $lane1_high))
            (i64.const 32)
          )
        )
      )
    )

    ;; Perform i64x2.shl operation
    (local.set $result_vector
      (i64x2.shl (local.get $input_vector) (local.get $shift_count))
    )

    ;; Extract results and split back into 32-bit parts
    ;; Lane 0 low 32 bits
    (i32.wrap_i64 (i64x2.extract_lane 0 (local.get $result_vector)))

    ;; Lane 0 high 32 bits
    (i32.wrap_i64
      (i64.shr_u
        (i64x2.extract_lane 0 (local.get $result_vector))
        (i64.const 32)
      )
    )

    ;; Lane 1 low 32 bits
    (i32.wrap_i64 (i64x2.extract_lane 1 (local.get $result_vector)))

    ;; Lane 1 high 32 bits
    (i32.wrap_i64
      (i64.shr_u
        (i64x2.extract_lane 1 (local.get $result_vector))
        (i64.const 32)
      )
    )
  )

  ;; Additional test function for basic i64x2.shl validation
  (func (export "test_basic_shl")
    (param $input v128) (param $shift i32)
    (result v128)
    (i64x2.shl (local.get $input) (local.get $shift))
  )

  ;; Test function for identity operation (shift by 0)
  (func (export "test_identity_shl")
    (result i64)
    ;; Test 0x123456789ABCDEF shifted by 0 should remain unchanged
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x123456789ABCDEF 0xFEDCBA9876543210)
        (i32.const 0)
      )
    )
  )

  ;; Test function for maximum valid shift (63)
  (func (export "test_max_shift")
    (result i64)
    ;; Test 0x0000000000000001 shifted by 63 should equal 0x8000000000000000
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x0000000000000001 0x0000000000000001)
        (i32.const 63)
      )
    )
  )

  ;; Test function for shift masking (shift >= 64)
  (func (export "test_shift_masking")
    (result i64)
    ;; Test 0x0000000000000001 shifted by 64 should equal 0x0000000000000001 (64 & 63 = 0, identity)
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x0000000000000001 0x0000000000000001)
        (i32.const 64)
      )
    )
  )

  ;; Test function for large shift masking (shift by 65)
  (func (export "test_large_shift_masking")
    (result i64)
    ;; Test 0x0000000000000001 shifted by 65 should equal 0x0000000000000002 (65 & 63 = 1)
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x0000000000000001 0x0000000000000001)
        (i32.const 65)
      )
    )
  )

  ;; Test function for overflow behavior
  (func (export "test_overflow")
    (result i64)
    ;; Test 0x8000000000000000 shifted by 1 should equal 0x0000000000000000 (overflow)
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x8000000000000000 0x8000000000000000)
        (i32.const 1)
      )
    )
  )

  ;; Test function for zero vector behavior
  (func (export "test_zero_vector")
    (result i64)
    ;; Test all zeros shifted by any amount should remain zero
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x0000000000000000 0x0000000000000000)
        (i32.const 32)
      )
    )
  )

  ;; Test function for all-ones pattern
  (func (export "test_all_ones")
    (result i64)
    ;; Test 0xFFFFFFFFFFFFFFFF shifted by 1 should equal 0xFFFFFFFFFFFFFFFE
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0xFFFFFFFFFFFFFFFF 0xFFFFFFFFFFFFFFFF)
        (i32.const 1)
      )
    )
  )

  ;; Test function for alternating bit patterns
  (func (export "test_alternating_pattern")
    (result i64)
    ;; Test 0xAAAAAAAAAAAAAAAA shifted by 1 should equal 0x5555555555555554
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0xAAAAAAAAAAAAAAAA 0x5555555555555555)
        (i32.const 1)
      )
    )
  )

  ;; Test function for half-width shift (32)
  (func (export "test_half_width_shift")
    (result i64)
    ;; Test 0x0000000000000001 shifted by 32 should equal 0x0000000100000000
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x0000000000000001 0x0000000000000001)
        (i32.const 32)
      )
    )
  )

  ;; Test function for mixed lane values
  (func (export "test_mixed_lanes")
    (param $shift i32)
    (result i64 i64)
    (local $result v128)

    ;; Test with different values in each lane
    (local.set $result
      (i64x2.shl
        (v128.const i64x2 0x0123456789ABCDEF 0xFEDCBA9876543210)
        (local.get $shift)
      )
    )

    ;; Return both lanes
    (i64x2.extract_lane 0 (local.get $result))
    (i64x2.extract_lane 1 (local.get $result))
  )

  ;; Test function for sign bit behavior
  (func (export "test_sign_bit")
    (result i64)
    ;; Test 0x7FFFFFFFFFFFFFFF shifted by 1 should equal 0xFFFFFFFFFFFFFFFE
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x7FFFFFFFFFFFFFFF 0x7FFFFFFFFFFFFFFF)
        (i32.const 1)
      )
    )
  )

  ;; Test function for power-of-two patterns
  (func (export "test_power_patterns")
    (param $pattern i64) (param $shift i32)
    (result i64)
    (i64x2.extract_lane 0
      (i64x2.shl
        (i64x2.splat (local.get $pattern))
        (local.get $shift)
      )
    )
  )

  ;; Test function for boundary values with specific shift
  (func (export "test_boundary_values")
    (param $shift i32)
    (result i64 i64)
    (local $result v128)

    ;; Test with 1 and maximum signed positive value
    (local.set $result
      (i64x2.shl
        (v128.const i64x2 0x0000000000000001 0x7FFFFFFFFFFFFFFF)
        (local.get $shift)
      )
    )

    ;; Return both lanes
    (i64x2.extract_lane 0 (local.get $result))
    (i64x2.extract_lane 1 (local.get $result))
  )

  ;; Test function for extreme shift masking (shift by 255)
  (func (export "test_extreme_shift_masking")
    (result i64)
    ;; Test 0x0000000000000001 shifted by 255 should equal shift by 63 (255 & 63 = 63)
    (i64x2.extract_lane 0
      (i64x2.shl
        (v128.const i64x2 0x0000000000000001 0x0000000000000001)
        (i32.const 255)
      )
    )
  )
)