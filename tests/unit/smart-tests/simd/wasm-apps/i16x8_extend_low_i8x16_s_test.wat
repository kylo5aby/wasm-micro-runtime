(module
  ;; Test function for i16x8.extend_low_i8x16_s operation
  ;; Takes 16 individual i32 parameters representing i8 values and returns 8 i32 values representing i16 results
  (func (export "test_extend_low")
    (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
    (param $b4 i32) (param $b5 i32) (param $b6 i32) (param $b7 i32)
    (param $b8 i32) (param $b9 i32) (param $b10 i32) (param $b11 i32)
    (param $b12 i32) (param $b13 i32) (param $b14 i32) (param $b15 i32)
    (result i32) (result i32) (result i32) (result i32)
    (result i32) (result i32) (result i32) (result i32)

    (local $input_v128 v128)
    (local $result_v128 v128)

    ;; Build i8x16 vector from parameters
    (local.set $input_v128
      (i8x16.replace_lane 15 (local.get $b15)
        (i8x16.replace_lane 14 (local.get $b14)
          (i8x16.replace_lane 13 (local.get $b13)
            (i8x16.replace_lane 12 (local.get $b12)
              (i8x16.replace_lane 11 (local.get $b11)
                (i8x16.replace_lane 10 (local.get $b10)
                  (i8x16.replace_lane 9 (local.get $b9)
                    (i8x16.replace_lane 8 (local.get $b8)
                      (i8x16.replace_lane 7 (local.get $b7)
                        (i8x16.replace_lane 6 (local.get $b6)
                          (i8x16.replace_lane 5 (local.get $b5)
                            (i8x16.replace_lane 4 (local.get $b4)
                              (i8x16.replace_lane 3 (local.get $b3)
                                (i8x16.replace_lane 2 (local.get $b2)
                                  (i8x16.replace_lane 1 (local.get $b1)
                                    (i8x16.replace_lane 0 (local.get $b0)
                                      (i8x16.splat (i32.const 0))))))))))))))))))

    ;; Perform i16x8.extend_low_i8x16_s operation
    (local.set $result_v128
      (i16x8.extend_low_i8x16_s (local.get $input_v128)))

    ;; Extract and return the 8 i16 results as i32 values
    (i16x8.extract_lane_s 0 (local.get $result_v128))
    (i16x8.extract_lane_s 1 (local.get $result_v128))
    (i16x8.extract_lane_s 2 (local.get $result_v128))
    (i16x8.extract_lane_s 3 (local.get $result_v128))
    (i16x8.extract_lane_s 4 (local.get $result_v128))
    (i16x8.extract_lane_s 5 (local.get $result_v128))
    (i16x8.extract_lane_s 6 (local.get $result_v128))
    (i16x8.extract_lane_s 7 (local.get $result_v128))
  )
)