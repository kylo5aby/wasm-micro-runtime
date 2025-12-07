(module
  ;; Test function for i64x2.extend_high_i32x4 operation
  ;; Takes 4 individual i32 parameters representing i32x4 vector and returns 4 i32 values representing 2 i64 results
  ;; The operation extends the high 2 i32 values (lanes 2,3) to i64 values with sign extension
  (func (export "test_extend_high")
    (param $i0 i32) (param $i1 i32) (param $i2 i32) (param $i3 i32)
    (result i32) (result i32) (result i32) (result i32)

    (local $input_v128 v128)
    (local $result_v128 v128)

    ;; Build i32x4 vector from parameters
    ;; Lane order: i0=lane0, i1=lane1, i2=lane2, i3=lane3
    (local.set $input_v128
      (i32x4.replace_lane 3
        (i32x4.replace_lane 2
          (i32x4.replace_lane 1
            (i32x4.replace_lane 0
              (i32x4.splat (i32.const 0))
              (local.get $i0))
            (local.get $i1))
          (local.get $i2))
        (local.get $i3)))

    ;; Perform i64x2.extend_high_i32x4 operation
    ;; This extends lanes 2 and 3 (high lanes) to i64 values with sign extension
    (local.set $result_v128
      (i64x2.extend_high_i32x4_s (local.get $input_v128)))

    ;; Extract and return the 2 i64 results as 4 i32 values
    ;; Each i64 is returned as two i32 values: low 32 bits, high 32 bits
    ;; Result lane 0: i64 from input lane 2 (i2) -> low32, high32
    ;; Result lane 1: i64 from input lane 3 (i3) -> low32, high32
    (i64x2.extract_lane 0 (local.get $result_v128))
    (i32.wrap_i64)  ;; Low 32 bits of first i64

    (i64x2.extract_lane 0 (local.get $result_v128))
    (i64.const 32)
    (i64.shr_s)
    (i32.wrap_i64)  ;; High 32 bits of first i64

    (i64x2.extract_lane 1 (local.get $result_v128))
    (i32.wrap_i64)  ;; Low 32 bits of second i64

    (i64x2.extract_lane 1 (local.get $result_v128))
    (i64.const 32)
    (i64.shr_s)
    (i32.wrap_i64)  ;; High 32 bits of second i64
  )
)