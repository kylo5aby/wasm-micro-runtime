(module
  ;; Test function for i8x16.shl opcode validation
  ;; Takes 16 individual i32 values representing vector lanes and one shift count
  ;; Returns 16 i32 values representing the shifted result vector lanes

  (func (export "test_i8x16_shl")
    (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
    (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    (param $lane8 i32) (param $lane9 i32) (param $lane10 i32) (param $lane11 i32)
    (param $lane12 i32) (param $lane13 i32) (param $lane14 i32) (param $lane15 i32)
    (param $shift_count i32)
    (result i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)

    (local $result v128)

    ;; Construct input vector from individual lane parameters
    (local.set $result
      (i8x16.replace_lane 15
        (i8x16.replace_lane 14
          (i8x16.replace_lane 13
            (i8x16.replace_lane 12
              (i8x16.replace_lane 11
                (i8x16.replace_lane 10
                  (i8x16.replace_lane 9
                    (i8x16.replace_lane 8
                      (i8x16.replace_lane 7
                        (i8x16.replace_lane 6
                          (i8x16.replace_lane 5
                            (i8x16.replace_lane 4
                              (i8x16.replace_lane 3
                                (i8x16.replace_lane 2
                                  (i8x16.replace_lane 1
                                    (i8x16.splat (local.get $lane0))
                                    (local.get $lane1))
                                  (local.get $lane2))
                                (local.get $lane3))
                              (local.get $lane4))
                            (local.get $lane5))
                          (local.get $lane6))
                        (local.get $lane7))
                      (local.get $lane8))
                    (local.get $lane9))
                  (local.get $lane10))
                (local.get $lane11))
              (local.get $lane12))
            (local.get $lane13))
          (local.get $lane14))
        (local.get $lane15))
    )

    ;; Perform i8x16.shl operation
    (local.set $result
      (i8x16.shl (local.get $result) (local.get $shift_count))
    )

    ;; Extract all 16 lanes from result vector
    (i8x16.extract_lane_s 0 (local.get $result))   ;; lane 0
    (i8x16.extract_lane_s 1 (local.get $result))   ;; lane 1
    (i8x16.extract_lane_s 2 (local.get $result))   ;; lane 2
    (i8x16.extract_lane_s 3 (local.get $result))   ;; lane 3
    (i8x16.extract_lane_s 4 (local.get $result))   ;; lane 4
    (i8x16.extract_lane_s 5 (local.get $result))   ;; lane 5
    (i8x16.extract_lane_s 6 (local.get $result))   ;; lane 6
    (i8x16.extract_lane_s 7 (local.get $result))   ;; lane 7
    (i8x16.extract_lane_s 8 (local.get $result))   ;; lane 8
    (i8x16.extract_lane_s 9 (local.get $result))   ;; lane 9
    (i8x16.extract_lane_s 10 (local.get $result))  ;; lane 10
    (i8x16.extract_lane_s 11 (local.get $result))  ;; lane 11
    (i8x16.extract_lane_s 12 (local.get $result))  ;; lane 12
    (i8x16.extract_lane_s 13 (local.get $result))  ;; lane 13
    (i8x16.extract_lane_s 14 (local.get $result))  ;; lane 14
    (i8x16.extract_lane_s 15 (local.get $result))  ;; lane 15
  )

  ;; Additional test function for basic i8x16.shl validation
  (func (export "test_basic_shl")
    (param $input v128) (param $shift i32)
    (result v128)
    (i8x16.shl (local.get $input) (local.get $shift))
  )

  ;; Test function for boundary shift amounts
  (func (export "test_boundary_shl")
    (result i32)
    ;; Test 0x01 shifted by 7 should equal 0x80
    (i8x16.extract_lane_u 0
      (i8x16.shl
        (i8x16.splat (i32.const 0x01))
        (i32.const 7)
      )
    )
  )

  ;; Test function for shift masking (shift >= 8)
  (func (export "test_shift_masking")
    (result i32)
    ;; Test 0x02 shifted by 8 should equal 0x02 (8 & 7 = 0, identity)
    (i8x16.extract_lane_u 0
      (i8x16.shl
        (i8x16.splat (i32.const 0x02))
        (i32.const 8)
      )
    )
  )

  ;; Test function for overflow behavior
  (func (export "test_overflow")
    (result i32)
    ;; Test 0x80 shifted by 1 should equal 0x00 (overflow)
    (i8x16.extract_lane_u 0
      (i8x16.shl
        (i8x16.splat (i32.const 0x80))
        (i32.const 1)
      )
    )
  )

  ;; Test function for zero vector behavior
  (func (export "test_zero_vector")
    (result i32)
    ;; Test all zeros shifted by any amount should remain zero
    (i8x16.extract_lane_u 0
      (i8x16.shl
        (i8x16.splat (i32.const 0x00))
        (i32.const 5)
      )
    )
  )

  ;; Test function for power-of-two patterns
  (func (export "test_power_patterns")
    (param $pattern i32) (param $shift i32)
    (result i32)
    (i8x16.extract_lane_u 0
      (i8x16.shl
        (i8x16.splat (local.get $pattern))
        (local.get $shift)
      )
    )
  )

  ;; Test function for maximum value handling
  (func (export "test_max_values")
    (result i32)
    ;; Test 0xFF shifted by 1 should equal 0xFE
    (i8x16.extract_lane_u 0
      (i8x16.shl
        (i8x16.splat (i32.const 0xFF))
        (i32.const 1)
      )
    )
  )
)