(module
  ;; Test function for i8x16.narrow_i16x8_s opcode validation
  ;; Takes 16 individual i32 values representing two input i16x8 vectors (8 each)
  ;; Returns 16 i32 values representing the narrowed i8x16 result vector lanes

  (func (export "test_basic_narrowing")
    ;; First vector parameters (i16x8)
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec1_lane4 i32) (param $vec1_lane5 i32) (param $vec1_lane6 i32) (param $vec1_lane7 i32)

    ;; Second vector parameters (i16x8)
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (param $vec2_lane4 i32) (param $vec2_lane5 i32) (param $vec2_lane6 i32) (param $vec2_lane7 i32)

    ;; Return 16 i32 values representing result i8x16 lanes
    (result i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)

    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Construct first input vector (i16x8) from individual lane parameters
    (local.set $vec1
      (i16x8.replace_lane 7
        (i16x8.replace_lane 6
          (i16x8.replace_lane 5
            (i16x8.replace_lane 4
              (i16x8.replace_lane 3
                (i16x8.replace_lane 2
                  (i16x8.replace_lane 1
                    (i16x8.splat (local.get $vec1_lane0))
                    (local.get $vec1_lane1))
                  (local.get $vec1_lane2))
                (local.get $vec1_lane3))
              (local.get $vec1_lane4))
            (local.get $vec1_lane5))
          (local.get $vec1_lane6))
        (local.get $vec1_lane7))
    )

    ;; Construct second input vector (i16x8) from individual lane parameters
    (local.set $vec2
      (i16x8.replace_lane 7
        (i16x8.replace_lane 6
          (i16x8.replace_lane 5
            (i16x8.replace_lane 4
              (i16x8.replace_lane 3
                (i16x8.replace_lane 2
                  (i16x8.replace_lane 1
                    (i16x8.splat (local.get $vec2_lane0))
                    (local.get $vec2_lane1))
                  (local.get $vec2_lane2))
                (local.get $vec2_lane3))
              (local.get $vec2_lane4))
            (local.get $vec2_lane5))
          (local.get $vec2_lane6))
        (local.get $vec2_lane7))
    )

    ;; Perform i8x16.narrow_i16x8_s operation
    (local.set $result
      (i8x16.narrow_i16x8_s (local.get $vec1) (local.get $vec2))
    )

    ;; Extract and return all 16 result lanes as individual i32 values
    (i8x16.extract_lane_s 0 (local.get $result))
    (i8x16.extract_lane_s 1 (local.get $result))
    (i8x16.extract_lane_s 2 (local.get $result))
    (i8x16.extract_lane_s 3 (local.get $result))
    (i8x16.extract_lane_s 4 (local.get $result))
    (i8x16.extract_lane_s 5 (local.get $result))
    (i8x16.extract_lane_s 6 (local.get $result))
    (i8x16.extract_lane_s 7 (local.get $result))
    (i8x16.extract_lane_s 8 (local.get $result))
    (i8x16.extract_lane_s 9 (local.get $result))
    (i8x16.extract_lane_s 10 (local.get $result))
    (i8x16.extract_lane_s 11 (local.get $result))
    (i8x16.extract_lane_s 12 (local.get $result))
    (i8x16.extract_lane_s 13 (local.get $result))
    (i8x16.extract_lane_s 14 (local.get $result))
    (i8x16.extract_lane_s 15 (local.get $result))
  )

  (func (export "test_saturation_behavior")
    ;; First vector parameters (i16x8) - values requiring saturation
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec1_lane4 i32) (param $vec1_lane5 i32) (param $vec1_lane6 i32) (param $vec1_lane7 i32)

    ;; Second vector parameters (i16x8) - values requiring saturation
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (param $vec2_lane4 i32) (param $vec2_lane5 i32) (param $vec2_lane6 i32) (param $vec2_lane7 i32)

    ;; Return 16 i32 values representing result i8x16 lanes
    (result i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)

    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Construct first input vector with saturation test values
    (local.set $vec1
      (i16x8.replace_lane 7
        (i16x8.replace_lane 6
          (i16x8.replace_lane 5
            (i16x8.replace_lane 4
              (i16x8.replace_lane 3
                (i16x8.replace_lane 2
                  (i16x8.replace_lane 1
                    (i16x8.splat (local.get $vec1_lane0))
                    (local.get $vec1_lane1))
                  (local.get $vec1_lane2))
                (local.get $vec1_lane3))
              (local.get $vec1_lane4))
            (local.get $vec1_lane5))
          (local.get $vec1_lane6))
        (local.get $vec1_lane7))
    )

    ;; Construct second input vector with saturation test values
    (local.set $vec2
      (i16x8.replace_lane 7
        (i16x8.replace_lane 6
          (i16x8.replace_lane 5
            (i16x8.replace_lane 4
              (i16x8.replace_lane 3
                (i16x8.replace_lane 2
                  (i16x8.replace_lane 1
                    (i16x8.splat (local.get $vec2_lane0))
                    (local.get $vec2_lane1))
                  (local.get $vec2_lane2))
                (local.get $vec2_lane3))
              (local.get $vec2_lane4))
            (local.get $vec2_lane5))
          (local.get $vec2_lane6))
        (local.get $vec2_lane7))
    )

    ;; Perform i8x16.narrow_i16x8_s operation with saturation
    (local.set $result
      (i8x16.narrow_i16x8_s (local.get $vec1) (local.get $vec2))
    )

    ;; Extract and return all 16 result lanes
    (i8x16.extract_lane_s 0 (local.get $result))
    (i8x16.extract_lane_s 1 (local.get $result))
    (i8x16.extract_lane_s 2 (local.get $result))
    (i8x16.extract_lane_s 3 (local.get $result))
    (i8x16.extract_lane_s 4 (local.get $result))
    (i8x16.extract_lane_s 5 (local.get $result))
    (i8x16.extract_lane_s 6 (local.get $result))
    (i8x16.extract_lane_s 7 (local.get $result))
    (i8x16.extract_lane_s 8 (local.get $result))
    (i8x16.extract_lane_s 9 (local.get $result))
    (i8x16.extract_lane_s 10 (local.get $result))
    (i8x16.extract_lane_s 11 (local.get $result))
    (i8x16.extract_lane_s 12 (local.get $result))
    (i8x16.extract_lane_s 13 (local.get $result))
    (i8x16.extract_lane_s 14 (local.get $result))
    (i8x16.extract_lane_s 15 (local.get $result))
  )

  (func (export "test_zero_values")
    ;; First vector parameters (i16x8) - all zeros
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec1_lane4 i32) (param $vec1_lane5 i32) (param $vec1_lane6 i32) (param $vec1_lane7 i32)

    ;; Second vector parameters (i16x8) - all zeros
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (param $vec2_lane4 i32) (param $vec2_lane5 i32) (param $vec2_lane6 i32) (param $vec2_lane7 i32)

    ;; Return 16 i32 values representing result i8x16 lanes
    (result i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)

    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Create zero vectors
    (local.set $vec1 (i16x8.splat (i32.const 0)))
    (local.set $vec2 (i16x8.splat (i32.const 0)))

    ;; Perform i8x16.narrow_i16x8_s operation on zero vectors
    (local.set $result
      (i8x16.narrow_i16x8_s (local.get $vec1) (local.get $vec2))
    )

    ;; Extract and return all 16 result lanes
    (i8x16.extract_lane_s 0 (local.get $result))
    (i8x16.extract_lane_s 1 (local.get $result))
    (i8x16.extract_lane_s 2 (local.get $result))
    (i8x16.extract_lane_s 3 (local.get $result))
    (i8x16.extract_lane_s 4 (local.get $result))
    (i8x16.extract_lane_s 5 (local.get $result))
    (i8x16.extract_lane_s 6 (local.get $result))
    (i8x16.extract_lane_s 7 (local.get $result))
    (i8x16.extract_lane_s 8 (local.get $result))
    (i8x16.extract_lane_s 9 (local.get $result))
    (i8x16.extract_lane_s 10 (local.get $result))
    (i8x16.extract_lane_s 11 (local.get $result))
    (i8x16.extract_lane_s 12 (local.get $result))
    (i8x16.extract_lane_s 13 (local.get $result))
    (i8x16.extract_lane_s 14 (local.get $result))
    (i8x16.extract_lane_s 15 (local.get $result))
  )

  (func (export "test_boundary_values")
    ;; First vector parameters (i16x8) - boundary values
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec1_lane4 i32) (param $vec1_lane5 i32) (param $vec1_lane6 i32) (param $vec1_lane7 i32)

    ;; Second vector parameters (i16x8) - boundary values
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (param $vec2_lane4 i32) (param $vec2_lane5 i32) (param $vec2_lane6 i32) (param $vec2_lane7 i32)

    ;; Return 16 i32 values representing result i8x16 lanes
    (result i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)

    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Construct first input vector with exact i8 boundary values
    (local.set $vec1
      (i16x8.replace_lane 7
        (i16x8.replace_lane 6
          (i16x8.replace_lane 5
            (i16x8.replace_lane 4
              (i16x8.replace_lane 3
                (i16x8.replace_lane 2
                  (i16x8.replace_lane 1
                    (i16x8.splat (local.get $vec1_lane0))
                    (local.get $vec1_lane1))
                  (local.get $vec1_lane2))
                (local.get $vec1_lane3))
              (local.get $vec1_lane4))
            (local.get $vec1_lane5))
          (local.get $vec1_lane6))
        (local.get $vec1_lane7))
    )

    ;; Construct second input vector with values just outside i8 range
    (local.set $vec2
      (i16x8.replace_lane 7
        (i16x8.replace_lane 6
          (i16x8.replace_lane 5
            (i16x8.replace_lane 4
              (i16x8.replace_lane 3
                (i16x8.replace_lane 2
                  (i16x8.replace_lane 1
                    (i16x8.splat (local.get $vec2_lane0))
                    (local.get $vec2_lane1))
                  (local.get $vec2_lane2))
                (local.get $vec2_lane3))
              (local.get $vec2_lane4))
            (local.get $vec2_lane5))
          (local.get $vec2_lane6))
        (local.get $vec2_lane7))
    )

    ;; Perform i8x16.narrow_i16x8_s operation
    (local.set $result
      (i8x16.narrow_i16x8_s (local.get $vec1) (local.get $vec2))
    )

    ;; Extract and return all 16 result lanes
    (i8x16.extract_lane_s 0 (local.get $result))
    (i8x16.extract_lane_s 1 (local.get $result))
    (i8x16.extract_lane_s 2 (local.get $result))
    (i8x16.extract_lane_s 3 (local.get $result))
    (i8x16.extract_lane_s 4 (local.get $result))
    (i8x16.extract_lane_s 5 (local.get $result))
    (i8x16.extract_lane_s 6 (local.get $result))
    (i8x16.extract_lane_s 7 (local.get $result))
    (i8x16.extract_lane_s 8 (local.get $result))
    (i8x16.extract_lane_s 9 (local.get $result))
    (i8x16.extract_lane_s 10 (local.get $result))
    (i8x16.extract_lane_s 11 (local.get $result))
    (i8x16.extract_lane_s 12 (local.get $result))
    (i8x16.extract_lane_s 13 (local.get $result))
    (i8x16.extract_lane_s 14 (local.get $result))
    (i8x16.extract_lane_s 15 (local.get $result))
  )

  (func (export "test_lane_ordering")
    ;; First vector parameters (i16x8) - sequential values 1-8
    (param $vec1_lane0 i32) (param $vec1_lane1 i32) (param $vec1_lane2 i32) (param $vec1_lane3 i32)
    (param $vec1_lane4 i32) (param $vec1_lane5 i32) (param $vec1_lane6 i32) (param $vec1_lane7 i32)

    ;; Second vector parameters (i16x8) - sequential values 11-18
    (param $vec2_lane0 i32) (param $vec2_lane1 i32) (param $vec2_lane2 i32) (param $vec2_lane3 i32)
    (param $vec2_lane4 i32) (param $vec2_lane5 i32) (param $vec2_lane6 i32) (param $vec2_lane7 i32)

    ;; Return 16 i32 values representing result i8x16 lanes
    (result i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)

    (local $vec1 v128)
    (local $vec2 v128)
    (local $result v128)

    ;; Construct first input vector with sequential values 1-8
    (local.set $vec1
      (i16x8.replace_lane 7
        (i16x8.replace_lane 6
          (i16x8.replace_lane 5
            (i16x8.replace_lane 4
              (i16x8.replace_lane 3
                (i16x8.replace_lane 2
                  (i16x8.replace_lane 1
                    (i16x8.splat (i32.const 1))
                    (i32.const 2))
                  (i32.const 3))
                (i32.const 4))
              (i32.const 5))
            (i32.const 6))
          (i32.const 7))
        (i32.const 8))
    )

    ;; Construct second input vector with sequential values 11-18
    (local.set $vec2
      (i16x8.replace_lane 7
        (i16x8.replace_lane 6
          (i16x8.replace_lane 5
            (i16x8.replace_lane 4
              (i16x8.replace_lane 3
                (i16x8.replace_lane 2
                  (i16x8.replace_lane 1
                    (i16x8.splat (i32.const 11))
                    (i32.const 12))
                  (i32.const 13))
                (i32.const 14))
              (i32.const 15))
            (i32.const 16))
          (i32.const 17))
        (i32.const 18))
    )

    ;; Perform i8x16.narrow_i16x8_s operation
    ;; Result should be: [1,2,3,4,5,6,7,8,11,12,13,14,15,16,17,18]
    (local.set $result
      (i8x16.narrow_i16x8_s (local.get $vec1) (local.get $vec2))
    )

    ;; Extract and return all 16 result lanes
    (i8x16.extract_lane_s 0 (local.get $result))
    (i8x16.extract_lane_s 1 (local.get $result))
    (i8x16.extract_lane_s 2 (local.get $result))
    (i8x16.extract_lane_s 3 (local.get $result))
    (i8x16.extract_lane_s 4 (local.get $result))
    (i8x16.extract_lane_s 5 (local.get $result))
    (i8x16.extract_lane_s 6 (local.get $result))
    (i8x16.extract_lane_s 7 (local.get $result))
    (i8x16.extract_lane_s 8 (local.get $result))
    (i8x16.extract_lane_s 9 (local.get $result))
    (i8x16.extract_lane_s 10 (local.get $result))
    (i8x16.extract_lane_s 11 (local.get $result))
    (i8x16.extract_lane_s 12 (local.get $result))
    (i8x16.extract_lane_s 13 (local.get $result))
    (i8x16.extract_lane_s 14 (local.get $result))
    (i8x16.extract_lane_s 15 (local.get $result))
  )
)