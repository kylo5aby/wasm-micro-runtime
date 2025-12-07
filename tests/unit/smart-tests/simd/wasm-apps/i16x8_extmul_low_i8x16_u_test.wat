(module
  ;; i16x8.extmul_low_i8x16_u comprehensive test module
  ;; Tests extended multiplication on low eight i8 lanes (0-7) of two v128 vectors,
  ;; treating i8 values as unsigned integers and producing eight i16 results.

  ;; Basic extended multiplication test
  ;; Tests sequential values: [1,2,3,4,5,6,7,8] × [2,3,4,5,6,7,8,9]
  ;; Expected results: [2,6,12,20,30,42,56,72] in lanes 0-7
  (func (export "extmul_low_basic") (result v128)
    ;; First operand: [1,2,3,4,5,6,7,8, 255,254,253,252,251,250,249,248]
    ;; Only lanes 0-7 matter for low operation
    v128.const i8x16 1 2 3 4 5 6 7 8 255 254 253 252 251 250 249 248
    ;; Second operand: [2,3,4,5,6,7,8,9, 100,101,102,103,104,105,106,107]
    ;; Only lanes 0-7 matter for low operation
    v128.const i8x16 2 3 4 5 6 7 8 9 100 101 102 103 104 105 106 107
    ;; Perform extended multiplication on low lanes (0-7)
    i16x8.extmul_low_i8x16_u
  )

  ;; Maximum boundary values test
  ;; Tests maximum unsigned i8 values: [255,255,...] × [255,255,...]
  ;; Expected results: [65025,65025,...] (255×255) in lanes 0-7
  (func (export "extmul_low_max") (result v128)
    ;; First operand: all 255 in lanes 0-7
    v128.const i8x16 255 255 255 255 255 255 255 255 0 0 0 0 0 0 0 0
    ;; Second operand: all 255 in lanes 0-7
    v128.const i8x16 255 255 255 255 255 255 255 255 0 0 0 0 0 0 0 0
    ;; Perform extended multiplication: 255 × 255 = 65025 in each lane
    i16x8.extmul_low_i8x16_u
  )

  ;; Zero operand test
  ;; Tests zero multiplication: [0,0,0,...] × any values = [0,0,0,...]
  ;; Expected results: all zeros in lanes 0-7
  (func (export "extmul_low_zero") (result v128)
    ;; First operand: all zeros in lanes 0-7
    v128.const i8x16 0 0 0 0 0 0 0 0 255 254 253 252 251 250 249 248
    ;; Second operand: various values in lanes 0-7
    v128.const i8x16 1 2 3 4 5 6 7 8 100 101 102 103 104 105 106 107
    ;; Perform extended multiplication: 0 × any = 0 in each lane
    i16x8.extmul_low_i8x16_u
  )

  ;; Identity multiplication test
  ;; Tests multiplication by 1: [a,b,c,...] × [1,1,1,...] = [a,b,c,...]
  ;; Expected results: original values preserved in lanes 0-7
  (func (export "extmul_low_identity") (result v128)
    ;; First operand: test values in lanes 0-7
    v128.const i8x16 10 20 30 40 50 60 70 80 200 201 202 203 204 205 206 207
    ;; Second operand: identity (1) in lanes 0-7
    v128.const i8x16 1 1 1 1 1 1 1 1 100 101 102 103 104 105 106 107
    ;; Perform extended multiplication: value × 1 = value in each lane
    i16x8.extmul_low_i8x16_u
  )

  ;; High lane isolation test
  ;; Verifies that lanes 8-15 in input vectors don't affect result
  ;; Only lanes 0-7 should be processed, high lanes should be ignored
  (func (export "extmul_low_isolation") (result v128)
    ;; First operand: [5,10,15,20,25,30,35,40] in low lanes, different values in high
    v128.const i8x16 5 10 15 20 25 30 35 40 100 101 102 103 104 105 106 107
    ;; Second operand: [2,2,2,2,2,2,2,2] in low lanes, different values in high
    v128.const i8x16 2 2 2 2 2 2 2 2 200 201 202 203 204 205 206 207
    ;; Result should be [10,20,30,40,50,60,70,80] regardless of high lane values
    i16x8.extmul_low_i8x16_u
  )

  ;; Power-of-two values test
  ;; Tests power-of-two bit patterns: [1,2,4,8,16,32,64,128] × [2,2,2,...]
  ;; Expected results: [2,4,8,16,32,64,128,256] in lanes 0-7
  (func (export "extmul_low_powers") (result v128)
    ;; First operand: power-of-two values in lanes 0-7
    v128.const i8x16 1 2 4 8 16 32 64 128 255 254 253 252 251 250 249 248
    ;; Second operand: multiply by 2 in lanes 0-7
    v128.const i8x16 2 2 2 2 2 2 2 2 100 101 102 103 104 105 106 107
    ;; Result: each power-of-two × 2 = next power-of-two
    i16x8.extmul_low_i8x16_u
  )

  ;; Mixed value test for comprehensive validation
  ;; Tests various unsigned i8 combinations and edge cases
  (func (export "extmul_low_mixed") (result v128)
    ;; First operand: mixed values
    v128.const i8x16 0 1 127 128 254 255 17 34 200 201 202 203 204 205 206 207
    ;; Second operand: mixed multipliers
    v128.const i8x16 255 128 2 2 1 1 3 7 100 101 102 103 104 105 106 107
    ;; Result: [0, 128, 254, 256, 254, 255, 51, 238]
    i16x8.extmul_low_i8x16_u
  )

  ;; Boundary combination test
  ;; Tests combinations of min (0), max (255), and mid-range values
  (func (export "extmul_low_boundary") (result v128)
    ;; First operand: boundary values
    v128.const i8x16 0 255 0 255 128 1 254 127 100 101 102 103 104 105 106 107
    ;; Second operand: boundary multipliers
    v128.const i8x16 255 0 1 1 2 255 1 2 200 201 202 203 204 205 206 207
    ;; Result: [0, 0, 0, 255, 256, 255, 254, 254]
    i16x8.extmul_low_i8x16_u
  )
)