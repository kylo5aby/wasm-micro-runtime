(module
  ;; Test function for i8x16.avgr_u opcode validation
  ;; Takes 32 individual i32 values representing two input vector lanes (16 each)
  ;; Returns 16 i32 values representing the averaged result vector lanes

  (func (export "test_i8x16_avgr_u")
    ;; First vector parameters (vec_a)
    (param $vec_a_lane0 i32) (param $vec_a_lane1 i32) (param $vec_a_lane2 i32) (param $vec_a_lane3 i32)
    (param $vec_a_lane4 i32) (param $vec_a_lane5 i32) (param $vec_a_lane6 i32) (param $vec_a_lane7 i32)
    (param $vec_a_lane8 i32) (param $vec_a_lane9 i32) (param $vec_a_lane10 i32) (param $vec_a_lane11 i32)
    (param $vec_a_lane12 i32) (param $vec_a_lane13 i32) (param $vec_a_lane14 i32) (param $vec_a_lane15 i32)

    ;; Second vector parameters (vec_b)
    (param $vec_b_lane0 i32) (param $vec_b_lane1 i32) (param $vec_b_lane2 i32) (param $vec_b_lane3 i32)
    (param $vec_b_lane4 i32) (param $vec_b_lane5 i32) (param $vec_b_lane6 i32) (param $vec_b_lane7 i32)
    (param $vec_b_lane8 i32) (param $vec_b_lane9 i32) (param $vec_b_lane10 i32) (param $vec_b_lane11 i32)
    (param $vec_b_lane12 i32) (param $vec_b_lane13 i32) (param $vec_b_lane14 i32) (param $vec_b_lane15 i32)

    ;; Unused parameters for compatibility
    (param $unused1 i32) (param $unused2 i32)

    ;; Return 16 i32 values representing result lanes
    (result i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)

    (local $vec_a v128)
    (local $vec_b v128)
    (local $result v128)

    ;; Construct first input vector (vec_a) from individual lane parameters
    (local.set $vec_a
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
                                    (i8x16.splat (local.get $vec_a_lane0))
                                    (local.get $vec_a_lane1))
                                  (local.get $vec_a_lane2))
                                (local.get $vec_a_lane3))
                              (local.get $vec_a_lane4))
                            (local.get $vec_a_lane5))
                          (local.get $vec_a_lane6))
                        (local.get $vec_a_lane7))
                      (local.get $vec_a_lane8))
                    (local.get $vec_a_lane9))
                  (local.get $vec_a_lane10))
                (local.get $vec_a_lane11))
              (local.get $vec_a_lane12))
            (local.get $vec_a_lane13))
          (local.get $vec_a_lane14))
        (local.get $vec_a_lane15))
    )

    ;; Construct second input vector (vec_b) from individual lane parameters
    (local.set $vec_b
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
                                    (i8x16.splat (local.get $vec_b_lane0))
                                    (local.get $vec_b_lane1))
                                  (local.get $vec_b_lane2))
                                (local.get $vec_b_lane3))
                              (local.get $vec_b_lane4))
                            (local.get $vec_b_lane5))
                          (local.get $vec_b_lane6))
                        (local.get $vec_b_lane7))
                      (local.get $vec_b_lane8))
                    (local.get $vec_b_lane9))
                  (local.get $vec_b_lane10))
                (local.get $vec_b_lane11))
              (local.get $vec_b_lane12))
            (local.get $vec_b_lane13))
          (local.get $vec_b_lane14))
        (local.get $vec_b_lane15))
    )

    ;; Perform i8x16.avgr_u operation: (vec_a[i] + vec_b[i] + 1) >> 1
    (local.set $result
      (i8x16.avgr_u (local.get $vec_a) (local.get $vec_b))
    )

    ;; Extract all 16 lanes from result vector using unsigned extraction
    (i8x16.extract_lane_u 0 (local.get $result))   ;; lane 0
    (i8x16.extract_lane_u 1 (local.get $result))   ;; lane 1
    (i8x16.extract_lane_u 2 (local.get $result))   ;; lane 2
    (i8x16.extract_lane_u 3 (local.get $result))   ;; lane 3
    (i8x16.extract_lane_u 4 (local.get $result))   ;; lane 4
    (i8x16.extract_lane_u 5 (local.get $result))   ;; lane 5
    (i8x16.extract_lane_u 6 (local.get $result))   ;; lane 6
    (i8x16.extract_lane_u 7 (local.get $result))   ;; lane 7
    (i8x16.extract_lane_u 8 (local.get $result))   ;; lane 8
    (i8x16.extract_lane_u 9 (local.get $result))   ;; lane 9
    (i8x16.extract_lane_u 10 (local.get $result))  ;; lane 10
    (i8x16.extract_lane_u 11 (local.get $result))  ;; lane 11
    (i8x16.extract_lane_u 12 (local.get $result))  ;; lane 12
    (i8x16.extract_lane_u 13 (local.get $result))  ;; lane 13
    (i8x16.extract_lane_u 14 (local.get $result))  ;; lane 14
    (i8x16.extract_lane_u 15 (local.get $result))  ;; lane 15
  )

  ;; Additional test function for basic i8x16.avgr_u validation
  (func (export "test_basic_avgr_u")
    (param $vec_a v128) (param $vec_b v128)
    (result v128)
    (i8x16.avgr_u (local.get $vec_a) (local.get $vec_b))
  )

  ;; Test function for boundary values (0, 255)
  (func (export "test_boundary_avgr_u")
    (result i32)
    ;; Test average of 0 and 255: (0 + 255 + 1) >> 1 = 128
    (i8x16.extract_lane_u 0
      (i8x16.avgr_u
        (i8x16.splat (i32.const 0))
        (i8x16.splat (i32.const 255))
      )
    )
  )

  ;; Test function for maximum values (255, 255)
  (func (export "test_max_avgr_u")
    (result i32)
    ;; Test average of 255 and 255: (255 + 255 + 1) >> 1 = 255
    (i8x16.extract_lane_u 0
      (i8x16.avgr_u
        (i8x16.splat (i32.const 255))
        (i8x16.splat (i32.const 255))
      )
    )
  )

  ;; Test function for zero values (0, 0)
  (func (export "test_zero_avgr_u")
    (result i32)
    ;; Test average of 0 and 0: (0 + 0 + 1) >> 1 = 0
    (i8x16.extract_lane_u 0
      (i8x16.avgr_u
        (i8x16.splat (i32.const 0))
        (i8x16.splat (i32.const 0))
      )
    )
  )

  ;; Test function for rounding behavior (odd sums)
  (func (export "test_rounding_avgr_u")
    (result i32)
    ;; Test average of 1 and 2: (1 + 2 + 1) >> 1 = 2 (rounds up)
    (i8x16.extract_lane_u 0
      (i8x16.avgr_u
        (i8x16.splat (i32.const 1))
        (i8x16.splat (i32.const 2))
      )
    )
  )

  ;; Test function for typical mid-range values
  (func (export "test_midrange_avgr_u")
    (param $val_a i32) (param $val_b i32)
    (result i32)
    (i8x16.extract_lane_u 0
      (i8x16.avgr_u
        (i8x16.splat (local.get $val_a))
        (i8x16.splat (local.get $val_b))
      )
    )
  )

  ;; Test function for lane independence verification
  (func (export "test_lane_independence")
    (result v128)
    ;; Create alternating pattern to test lane independence
    (i8x16.avgr_u
      ;; vec_a: alternating 100, 50, 100, 50, ...
      (v128.const i8x16 100 50 100 50 100 50 100 50 100 50 100 50 100 50 100 50)
      ;; vec_b: alternating 200, 150, 200, 150, ...
      (v128.const i8x16 200 150 200 150 200 150 200 150 200 150 200 150 200 150 200 150)
    )
  )

  ;; Test function for overflow prevention (large values)
  (func (export "test_overflow_prevention")
    (result i32)
    ;; Test that (254 + 254 + 1) >> 1 = 254, not overflow
    (i8x16.extract_lane_u 0
      (i8x16.avgr_u
        (i8x16.splat (i32.const 254))
        (i8x16.splat (i32.const 254))
      )
    )
  )
)