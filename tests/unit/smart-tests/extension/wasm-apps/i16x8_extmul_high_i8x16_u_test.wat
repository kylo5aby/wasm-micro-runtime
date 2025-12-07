(module
  ;; Memory for test data input and output
  (memory (export "memory") 1)

  ;; Test function for i16x8.extmul_high_i8x16_u opcode
  ;; Takes two v128 parameters and returns v128 result
  (func (export "test_i16x8_extmul_high_i8x16_u")
    (param $vec_a v128) (param $vec_b v128) (result v128)

    ;; Perform i16x8.extmul_high_i8x16_u operation
    local.get $vec_a
    local.get $vec_b
    i16x8.extmul_high_i8x16_u
  )

  ;; Test function for boundary values (0 and 255)
  (func (export "test_boundary_values") (result v128)
    ;; Create test vectors with boundary values
    ;; First vector: [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 255, 255, 0, 255, 127, 128]
    ;; Second vector: [0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 254, 100, 50, 2, 2]

    ;; Build first vector
    i32.const 0
    i8x16.splat
    i32.const 0 i8x16.replace_lane 8   ;; lane 8 = 0
    i32.const 1 i8x16.replace_lane 9   ;; lane 9 = 1
    i32.const 255 i8x16.replace_lane 10 ;; lane 10 = 255
    i32.const 255 i8x16.replace_lane 11 ;; lane 11 = 255
    i32.const 0 i8x16.replace_lane 12   ;; lane 12 = 0
    i32.const 255 i8x16.replace_lane 13 ;; lane 13 = 255
    i32.const 127 i8x16.replace_lane 14 ;; lane 14 = 127
    i32.const 128 i8x16.replace_lane 15 ;; lane 15 = 128

    ;; Build second vector
    i32.const 0
    i8x16.splat
    i32.const 255 i8x16.replace_lane 8  ;; lane 8 = 255
    i32.const 255 i8x16.replace_lane 9  ;; lane 9 = 255
    i32.const 255 i8x16.replace_lane 10 ;; lane 10 = 255
    i32.const 254 i8x16.replace_lane 11 ;; lane 11 = 254
    i32.const 100 i8x16.replace_lane 12 ;; lane 12 = 100
    i32.const 50 i8x16.replace_lane 13  ;; lane 13 = 50
    i32.const 2 i8x16.replace_lane 14   ;; lane 14 = 2
    i32.const 2 i8x16.replace_lane 15   ;; lane 15 = 2

    ;; Perform i16x8.extmul_high_i8x16_u operation
    i16x8.extmul_high_i8x16_u
  )

  ;; Test function for zero multiplication patterns
  (func (export "test_zero_multiplication") (result v128)
    ;; Create test vectors with zeros
    ;; First vector high lanes: [0, 0, 0, 0, 255, 255, 255, 255]
    ;; Second vector high lanes: [255, 100, 50, 25, 0, 0, 0, 0]

    i32.const 1
    i8x16.splat
    i32.const 0 i8x16.replace_lane 8    ;; lane 8 = 0
    i32.const 0 i8x16.replace_lane 9    ;; lane 9 = 0
    i32.const 0 i8x16.replace_lane 10   ;; lane 10 = 0
    i32.const 0 i8x16.replace_lane 11   ;; lane 11 = 0
    i32.const 255 i8x16.replace_lane 12 ;; lane 12 = 255
    i32.const 255 i8x16.replace_lane 13 ;; lane 13 = 255
    i32.const 255 i8x16.replace_lane 14 ;; lane 14 = 255
    i32.const 255 i8x16.replace_lane 15 ;; lane 15 = 255

    i32.const 2
    i8x16.splat
    i32.const 255 i8x16.replace_lane 8  ;; lane 8 = 255
    i32.const 100 i8x16.replace_lane 9  ;; lane 9 = 100
    i32.const 50 i8x16.replace_lane 10  ;; lane 10 = 50
    i32.const 25 i8x16.replace_lane 11  ;; lane 11 = 25
    i32.const 0 i8x16.replace_lane 12   ;; lane 12 = 0
    i32.const 0 i8x16.replace_lane 13   ;; lane 13 = 0
    i32.const 0 i8x16.replace_lane 14   ;; lane 14 = 0
    i32.const 0 i8x16.replace_lane 15   ;; lane 15 = 0

    ;; Perform i16x8.extmul_high_i8x16_u operation
    i16x8.extmul_high_i8x16_u
  )

  ;; Test function for identity multiplication (multiply by 1)
  (func (export "test_identity_multiplication") (result v128)
    ;; Create test vectors for identity
    ;; First vector high lanes: [1, 1, 1, 1, 1, 1, 1, 1]
    ;; Second vector high lanes: [37, 89, 123, 200, 255, 128, 64, 15]

    i32.const 0
    i8x16.splat
    i32.const 1 i8x16.replace_lane 8    ;; lane 8 = 1
    i32.const 1 i8x16.replace_lane 9    ;; lane 9 = 1
    i32.const 1 i8x16.replace_lane 10   ;; lane 10 = 1
    i32.const 1 i8x16.replace_lane 11   ;; lane 11 = 1
    i32.const 1 i8x16.replace_lane 12   ;; lane 12 = 1
    i32.const 1 i8x16.replace_lane 13   ;; lane 13 = 1
    i32.const 1 i8x16.replace_lane 14   ;; lane 14 = 1
    i32.const 1 i8x16.replace_lane 15   ;; lane 15 = 1

    i32.const 99
    i8x16.splat
    i32.const 37 i8x16.replace_lane 8   ;; lane 8 = 37
    i32.const 89 i8x16.replace_lane 9   ;; lane 9 = 89
    i32.const 123 i8x16.replace_lane 10 ;; lane 10 = 123
    i32.const 200 i8x16.replace_lane 11 ;; lane 11 = 200
    i32.const 255 i8x16.replace_lane 12 ;; lane 12 = 255
    i32.const 128 i8x16.replace_lane 13 ;; lane 13 = 128
    i32.const 64 i8x16.replace_lane 14  ;; lane 14 = 64
    i32.const 15 i8x16.replace_lane 15  ;; lane 15 = 15

    ;; Perform i16x8.extmul_high_i8x16_u operation
    i16x8.extmul_high_i8x16_u
  )

  ;; Test function for high lane isolation (verifies low lanes are ignored)
  (func (export "test_high_lane_isolation") (result v128)
    ;; Create test vectors with extreme values in low lanes, normal in high lanes
    ;; First vector: low=[255,255,255,255,255,255,255,255], high=[1,2,3,4,5,6,7,8]
    ;; Second vector: low=[127,127,127,127,127,127,127,127], high=[2,3,4,5,6,7,8,9]

    i32.const 255
    i8x16.splat
    i32.const 1 i8x16.replace_lane 8    ;; lane 8 = 1
    i32.const 2 i8x16.replace_lane 9    ;; lane 9 = 2
    i32.const 3 i8x16.replace_lane 10   ;; lane 10 = 3
    i32.const 4 i8x16.replace_lane 11   ;; lane 11 = 4
    i32.const 5 i8x16.replace_lane 12   ;; lane 12 = 5
    i32.const 6 i8x16.replace_lane 13   ;; lane 13 = 6
    i32.const 7 i8x16.replace_lane 14   ;; lane 14 = 7
    i32.const 8 i8x16.replace_lane 15   ;; lane 15 = 8

    i32.const 127
    i8x16.splat
    i32.const 2 i8x16.replace_lane 8    ;; lane 8 = 2
    i32.const 3 i8x16.replace_lane 9    ;; lane 9 = 3
    i32.const 4 i8x16.replace_lane 10   ;; lane 10 = 4
    i32.const 5 i8x16.replace_lane 11   ;; lane 11 = 5
    i32.const 6 i8x16.replace_lane 12   ;; lane 12 = 6
    i32.const 7 i8x16.replace_lane 13   ;; lane 13 = 7
    i32.const 8 i8x16.replace_lane 14   ;; lane 14 = 8
    i32.const 9 i8x16.replace_lane 15   ;; lane 15 = 9

    ;; Perform i16x8.extmul_high_i8x16_u operation
    i16x8.extmul_high_i8x16_u
  )
)