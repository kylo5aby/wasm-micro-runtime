(module
  ;; Test functions for i16x8.extmul_high_i8x16_s opcode
  ;; This module provides comprehensive test coverage for extended multiplication
  ;; of high lanes (8-15) from two i8x16 vectors producing i16x8 results

  ;; Basic positive multiplication test
  ;; Tests: 2×3=6, 4×5=20, 1×7=7, 6×8=48 in high lanes (8-15)
  (func (export "test_basic_positive") (result v128)
    ;; Create first vector: low lanes don't matter, high lanes: [2, 4, 1, 6, 0, 0, 0, 0]
    ;; Positions:                                    lanes:  [8, 9, 10,11,12,13,14,15]
    (v128.const i8x16 99 99 99 99 99 99 99 99 2 4 1 6 0 0 0 0)

    ;; Create second vector: low lanes don't matter, high lanes: [3, 5, 7, 8, 0, 0, 0, 0]
    ;; Positions:                                     lanes:  [8, 9, 10,11,12,13,14,15]
    (v128.const i8x16 88 88 88 88 88 88 88 88 3 5 7 8 0 0 0 0)

    ;; Perform extended multiplication on high lanes (8-15)
    ;; Result should be: [6, 20, 7, 48, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Mixed-sign multiplication test
  ;; Tests: 5×(-3)=-15, (-2)×4=-8, 7×(-1)=-7, (-6)×3=-18 in high lanes
  (func (export "test_mixed_signs") (result v128)
    ;; First vector high lanes: [5, -2, 7, -6, 0, 0, 0, 0]
    (v128.const i8x16 11 22 33 44 55 66 77 88 5 254 7 250 0 0 0 0)

    ;; Second vector high lanes: [-3, 4, -1, 3, 0, 0, 0, 0]
    ;; Note: -3 = 253, -1 = 255 in unsigned representation
    (v128.const i8x16 10 20 30 40 50 60 70 80 253 4 255 3 0 0 0 0)

    ;; Result: [-15, -8, -7, -18, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Maximum positive boundary test: INT8_MAX × INT8_MAX = 127 × 127 = 16129
  (func (export "test_max_positive") (result v128)
    ;; Both vectors have 127 in first high lane, others are 0
    (v128.const i8x16 1 2 3 4 5 6 7 8 127 0 0 0 0 0 0 0)
    (v128.const i8x16 9 8 7 6 5 4 3 2 127 0 0 0 0 0 0 0)

    ;; Result: [16129, 0, 0, 0, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Minimum × Maximum boundary test: INT8_MIN × INT8_MAX = (-128) × 127 = -16256
  (func (export "test_min_times_max") (result v128)
    ;; First vector: -128 in first high lane (represented as 128)
    (v128.const i8x16 15 25 35 45 55 65 75 85 128 0 0 0 0 0 0 0)

    ;; Second vector: 127 in first high lane
    (v128.const i8x16 91 81 71 61 51 41 31 21 127 0 0 0 0 0 0 0)

    ;; Result: [-16256, 0, 0, 0, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Minimum × Minimum boundary test: INT8_MIN × INT8_MIN = (-128) × (-128) = 16384
  (func (export "test_min_times_min") (result v128)
    ;; Both vectors have -128 in first high lane (represented as 128)
    (v128.const i8x16 100 101 102 103 104 105 106 107 128 0 0 0 0 0 0 0)
    (v128.const i8x16 200 201 202 203 204 205 206 207 128 0 0 0 0 0 0 0)

    ;; Result: [16384, 0, 0, 0, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; All-zero high lanes test
  (func (export "test_all_zero_high") (result v128)
    ;; Both vectors have all zeros in high lanes (8-15)
    (v128.const i8x16 50 60 70 80 90 100 110 120 0 0 0 0 0 0 0 0)
    (v128.const i8x16 1 2 3 4 5 6 7 8 0 0 0 0 0 0 0 0)

    ;; Result: [0, 0, 0, 0, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Partial zero test: some high lanes zero, others non-zero
  ;; Tests: 0×5=0, 3×0=0, 2×4=8, 0×7=0 in first 4 high lanes
  (func (export "test_partial_zero") (result v128)
    ;; First vector high lanes: [0, 3, 2, 0, 1, 1, 1, 1]
    (v128.const i8x16 11 12 13 14 15 16 17 18 0 3 2 0 1 1 1 1)

    ;; Second vector high lanes: [5, 0, 4, 7, 2, 2, 2, 2]
    (v128.const i8x16 21 22 23 24 25 26 27 28 5 0 4 7 2 2 2 2)

    ;; Result: [0, 0, 8, 0, 2, 2, 2, 2] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Identity multiplication test: any_value × 1 = any_value
  ;; Tests: 5×1=5, (-3)×1=-3, 7×1=7, (-12)×1=-12
  (func (export "test_identity_multiply") (result v128)
    ;; First vector high lanes: [5, -3, 7, -12, 0, 0, 0, 0]
    ;; -3 = 253, -12 = 244 in unsigned representation
    (v128.const i8x16 31 32 33 34 35 36 37 38 5 253 7 244 0 0 0 0)

    ;; Second vector high lanes: [1, 1, 1, 1, 0, 0, 0, 0]
    (v128.const i8x16 41 42 43 44 45 46 47 48 1 1 1 1 0 0 0 0)

    ;; Result: [5, -3, 7, -12, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Sign inversion test: any_value × (-1) = -any_value
  ;; Tests: 8×(-1)=-8, (-5)×(-1)=5, 15×(-1)=-15, (-20)×(-1)=20
  (func (export "test_sign_inversion") (result v128)
    ;; First vector high lanes: [8, -5, 15, -20, 0, 0, 0, 0]
    ;; -5 = 251, -20 = 236 in unsigned representation
    (v128.const i8x16 51 52 53 54 55 56 57 58 8 251 15 236 0 0 0 0)

    ;; Second vector high lanes: [-1, -1, -1, -1, 0, 0, 0, 0]
    ;; -1 = 255 in unsigned representation
    (v128.const i8x16 61 62 63 64 65 66 67 68 255 255 255 255 0 0 0 0)

    ;; Result: [-8, 5, -15, 20, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Lane isolation test - verify only high lanes matter, low lanes ignored
  ;; High lanes: 2×3=6, 4×1=4, rest zeros
  (func (export "test_lane_isolation") (result v128)
    ;; First vector: specific values in low lanes, test values in high lanes
    ;; Low lanes: [10, 20, 30, 40, 50, 60, 70, 80], High lanes: [2, 4, 0, 0, 0, 0, 0, 0]
    (v128.const i8x16 10 20 30 40 50 60 70 80 2 4 0 0 0 0 0 0)

    ;; Second vector: different values in low lanes, test values in high lanes
    ;; Low lanes: [11, 21, 31, 41, 51, 61, 71, 81], High lanes: [3, 1, 0, 0, 0, 0, 0, 0]
    (v128.const i8x16 11 21 31 41 51 61 71 81 3 1 0 0 0 0 0 0)

    ;; Result should be: [6, 4, 0, 0, 0, 0, 0, 0] as i16x8 (only depends on high lanes)
    i16x8.extmul_high_i8x16_s
  )

  ;; Lane isolation variant - same high lanes, completely different low lanes
  (func (export "test_lane_isolation_variant") (result v128)
    ;; First vector: different low lane values, SAME high lanes as above
    ;; Low lanes: [99, 88, 77, 66, 55, 44, 33, 22], High lanes: [2, 4, 0, 0, 0, 0, 0, 0]
    (v128.const i8x16 99 88 77 66 55 44 33 22 2 4 0 0 0 0 0 0)

    ;; Second vector: different low lane values, SAME high lanes as above
    ;; Low lanes: [111, 222, 100, 200, 150, 175, 125, 225], High lanes: [3, 1, 0, 0, 0, 0, 0, 0]
    (v128.const i8x16 111 222 100 200 150 175 125 225 3 1 0 0 0 0 0 0)

    ;; Result should be IDENTICAL: [6, 4, 0, 0, 0, 0, 0, 0] as i16x8
    i16x8.extmul_high_i8x16_s
  )

  ;; Additional comprehensive test combining multiple scenarios
  (func (export "test_comprehensive_mixed") (result v128)
    ;; Test all 8 high lanes with varied scenarios:
    ;; Lane 8: 10×5=50, Lane 9: -1×7=-7, Lane 10: 3×(-2)=-6, Lane 11: 0×anything=0
    ;; Lane 12: 127×1=127, Lane 13: (-128)×(-1)=128, Lane 14: 25×(-4)=-100, Lane 15: 1×1=1
    ;; -1=255, -2=254, -128=128, -4=252 in unsigned representation
    (v128.const i8x16 5 6 7 8 9 10 11 12 10 255 3 0 127 128 25 1)
    (v128.const i8x16 1 2 3 4 13 14 15 16 5 7 254 100 1 255 252 1)

    ;; Expected result: [50, -7, -6, 0, 127, 128, -100, 1] as i16x8
    i16x8.extmul_high_i8x16_s
  )
)